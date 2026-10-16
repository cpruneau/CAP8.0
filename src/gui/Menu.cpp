#include "imgui.h"
#include "Menu.hpp"
#include "MenuBar.hpp"

namespace CAP
{
  namespace GUI
  {

  Menu::Menu(const String & name, const String & description)
  :
  _name(name),
  _description(description),
  _enabled(true),
  _entries()
  { }

  MenuItem & Menu::addItem(const String & name, const String & command, const String & shortcut, const String & description)
  {
  Entry entry;
  entry.item.reset(new MenuItem(name, command, shortcut, description));
  _entries.push_back(std::move(entry));
  return *_entries.back().item;
  }

  void Menu::addSeparator()
  {
  _entries.push_back(Entry());
  }

  Menu & Menu::addMenu(const String & name, const String & description)
  {
  Entry entry;
  entry.menu.reset(new Menu(name, description));
  _entries.push_back(std::move(entry));
  return *_entries.back().menu;
  }

  MenuItem * Menu::findItem(const String & command)
  {
  for (auto & entry : _entries)
    {
    if (entry.item && entry.item->command() == command) return entry.item.get();
    if (entry.menu)
      if (MenuItem * item = entry.menu->findItem(command)) return item;
    }
  return nullptr;
  }

  Menu * Menu::findMenu(const String & name)
  {
  for (auto & entry : _entries) if (entry.menu && entry.menu->name() == name) return entry.menu.get();
  return nullptr;
  }

  void Menu::collectItems(std::vector<MenuItem*> & items)
  {
  for (auto & entry : _entries)
    {
    if (entry.item) items.push_back(entry.item.get());
    if (entry.menu) entry.menu->collectItems(items);
    }
  }

  void Menu::renderContents(const MenuItem *& hovered)
  {
  for (auto & entry : _entries)
    {
    if (entry.menu)
      {
      if (ImGui::BeginMenu(entry.menu->name().data(), entry.menu->isEnabled()))
        {
        entry.menu->renderContents(hovered);
        ImGui::EndMenu();
        }
      continue;
      }
    if (!entry.item)
      {
      ImGui::Separator();
      continue;
      }
    MenuItem & item = *entry.item;
    const MenuItemStyle & style = item.style();
    bool colored = style.alpha > 0;
    if (colored) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(style.red, style.green, style.blue, style.alpha));
    String label = style.icon.isNull() ? item.name() : style.icon + String("  ") + item.name();
    label += "##"; label += item.command();                                   // unique ImGui id
    String shortcut = MenuBar::displayShortcut(item.shortcut());
    bool selected = ImGui::MenuItem(label.data(), shortcut.isNull() ? nullptr : shortcut.data(),
                                    item.isCheckable() && item.isChecked(), item.isEnabled());
    if (colored) ImGui::PopStyleColor();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
      {
      hovered = &item;
      if (!item.description().isNull()) ImGui::SetItemTooltip("%s", item.description().data());
      }
    if (selected) item.fire();
    }
  }

  } // namespace GUI
} // namespace CAP
