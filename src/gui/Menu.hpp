#ifndef CAP__GUI__Menu
#define CAP__GUI__Menu
#include <memory>
#include <vector>
#include "MenuItem.hpp"

namespace CAP
{
  namespace GUI
  {

  //!
  //! @brief A menu: a named list of items, separators and submenus, drawn in order.
  //!
  class Menu
  {
  public:

  //! @brief Menu. @param name text shown in the menu bar (or in the parent menu) @param description tooltip
  Menu(const String & name, const String & description="");

  const String & name() const        { return _name; }
  const String & description() const { return _description; }
  bool isEnabled() const  { return _enabled; }
  void setEnabled(bool v) { _enabled = v; }

  //! @brief Add an item at the end (see MenuItem). @return the new item (owned by the menu)
  MenuItem & addItem(const String & name, const String & command="", const String & shortcut="",
                     const String & description="");
  //! @brief Add a separator line at the end.
  void addSeparator();
  //! @brief Add a submenu at the end. @return the new submenu (owned by the menu)
  Menu & addMenu(const String & name, const String & description="");

  //! @brief Item with the given command, searched in this menu and its submenus. @return item or nullptr
  MenuItem * findItem(const String & command);
  //! @brief Submenu with the given name (this menu only). @return submenu or nullptr
  Menu * findMenu(const String & name);
  //! @brief All the items of this menu and its submenus. @param items list the items are appended to
  void collectItems(std::vector<MenuItem*> & items);

  //! @brief Draw the menu contents (inside an open ImGui menu) and fire the selected item.
  //! @param hovered set to the item under the mouse, for the status bar (unchanged if none)
  void renderContents(const MenuItem *& hovered);

  protected:

  //! One entry: an item, a separator (both null) or a submenu.
  struct Entry
  {
  std::unique_ptr<MenuItem> item;
  std::unique_ptr<Menu>     menu;
  };

  String _name;
  String _description;
  bool   _enabled;
  std::vector<Entry> _entries;
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__Menu
