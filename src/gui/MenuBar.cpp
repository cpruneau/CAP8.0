/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file MenuBar.cpp
//! @brief Implementation of MenuBar.
#include <cctype>
#include <string>
#include "imgui.h"
#include "MenuBar.hpp"

namespace CAP
{
  namespace GUI
  {

  MenuBar::MenuBar()
  :
  _menus(),
  _listeners()
  { }

  Menu & MenuBar::addMenu(const String & name, const String & description)
  {
  _menus.emplace_back(new Menu(name, description));
  return *_menus.back();
  }

  Menu * MenuBar::findMenu(const String & name)
  {
  for (auto & menu : _menus) if (menu->name() == name) return menu.get();
  return nullptr;
  }

  MenuItem * MenuBar::findItem(const String & command)
  {
  for (auto & menu : _menus) if (MenuItem * item = menu->findItem(command)) return item;
  return nullptr;
  }

  bool MenuBar::fire(const String & command)
  {
  MenuItem * item = findItem(command);
  if (!item) return false;
  item->fire();
  return true;
  }

  void MenuBar::addListener(ActionListener * listener)
  {
  _listeners.push_back(listener);
  }

  String MenuBar::displayShortcut(const String & shortcut)
  {
#ifdef __APPLE__
  String s = shortcut;
  s.replaceAll("Ctrl+", "Cmd+");
  return s;
#else
  return shortcut;
#endif
  }

  int MenuBar::keyChord(const String & shortcut)
  {
  if (shortcut.isNull()) return 0;
  ImGuiKeyChord chord = 0;
  std::string s = shortcut.str();
  size_t start = 0;
  while (start <= s.size())
    {
    size_t plus = s.find('+', start);
    if (plus == start) plus = s.find('+', start+1);              // a "+" key
    std::string token = s.substr(start, plus == std::string::npos ? std::string::npos : plus-start);
    start = (plus == std::string::npos) ? s.size()+1 : plus+1;
    std::string t;
    for (char c : token) t += char(std::tolower((unsigned char)c));
    if      (t == "ctrl" || t == "cmd") chord |= ImGuiMod_Ctrl;
    else if (t == "shift") chord |= ImGuiMod_Shift;
    else if (t == "alt" || t == "option") chord |= ImGuiMod_Alt;
    else if (t.size() == 1 && t[0] >= 'a' && t[0] <= 'z') chord |= ImGuiKey_A + (t[0]-'a');
    else if (t.size() == 1 && t[0] >= '0' && t[0] <= '9') chord |= ImGuiKey_0 + (t[0]-'0');
    else if (t.size() >= 2 && t[0] == 'f' && std::isdigit((unsigned char)t[1]))
      {
      int n = std::stoi(t.substr(1));
      if (n >= 1 && n <= 12) chord |= ImGuiKey_F1 + (n-1);
      }
    else if (t == "delete" || t == "del") chord |= ImGuiKey_Delete;
    else if (t == "backspace") chord |= ImGuiKey_Backspace;
    else if (t == "escape" || t == "esc") chord |= ImGuiKey_Escape;
    else if (t == "enter" || t == "return") chord |= ImGuiKey_Enter;
    else if (t == "tab") chord |= ImGuiKey_Tab;
    else if (t == "space") chord |= ImGuiKey_Space;
    else if (t == "," || t == "comma") chord |= ImGuiKey_Comma;
    else if (t == "." || t == "period") chord |= ImGuiKey_Period;
    else if (t == "=" || t == "plus") chord |= ImGuiKey_Equal;
    else if (t == "-" || t == "minus") chord |= ImGuiKey_Minus;
    else return 0;
    }
  return (chord & ~ImGuiMod_Mask_) ? int(chord) : 0;
  }

  const MenuItem * MenuBar::render()
  {
  // listeners registered on the bar reach every item
  for (auto & menu : _menus)
    {
    std::vector<MenuItem*> items;
    menu->collectItems(items);
    for (auto item : items) for (auto listener : _listeners) item->addListener(listener);
    }

  const MenuItem * hovered = nullptr;
  if (ImGui::BeginMainMenuBar())
    {
    for (auto & menu : _menus)
      {
      if (ImGui::BeginMenu(menu->name().data(), menu->isEnabled()))
        {
        menu->renderContents(hovered);
        ImGui::EndMenu();
        }
      }
    ImGui::EndMainMenuBar();
    }

  // shortcuts: fired even when the menus are closed, unless a text field has the keyboard
  if (!ImGui::GetIO().WantTextInput)
    {
    for (auto & menu : _menus)
      {
      std::vector<MenuItem*> items;
      menu->collectItems(items);
      for (auto item : items)
        {
        int chord = keyChord(item->shortcut());
        if (chord && item->isEnabled() && ImGui::Shortcut(ImGuiKeyChord(chord), ImGuiInputFlags_RouteGlobal))
          item->fire();
        }
      }
    }
  return hovered;
  }

  } // namespace GUI
} // namespace CAP
