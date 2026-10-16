#ifndef CAP__GUI__MenuBar
#define CAP__GUI__MenuBar
#include <memory>
#include <vector>
#include "Menu.hpp"

namespace CAP
{
  namespace GUI
  {

  //!
  //! @brief Menu bar of the main window: an ordered list of menus, with the keyboard shortcuts of their items.
  //!
  //! render() draws the bar, fires the item selected with the mouse, and fires the items whose shortcut was pressed
  //! (whether or not a menu is open). addListener() registers a listener on every item, present and future.
  //!
  class MenuBar
  {
  public:

  MenuBar();

  //! @brief Add a menu at the end. @return the new menu (owned by the bar)
  Menu & addMenu(const String & name, const String & description="");
  //! @brief Menu with the given name. @return menu or nullptr
  Menu * findMenu(const String & name);
  //! @brief Item with the given command, in any menu. @return item or nullptr
  MenuItem * findItem(const String & command);
  //! @brief Fire the item with the given command (as if selected). @return false if there is no such item
  bool fire(const String & command);

  //! @brief Register a listener on all the items (including those added later). Not owned.
  void addListener(ActionListener * listener);

  //! @brief Draw the main menu bar and process the shortcuts. @return the item under the mouse, or nullptr
  const MenuItem * render();

  //! @brief Shortcut as shown in menus: "Ctrl" becomes "Cmd" on macOS. @param shortcut e.g. "Ctrl+O"
  static String displayShortcut(const String & shortcut);
  //! @brief ImGui key chord of a shortcut ("Ctrl+Shift+S", "Delete", "F1"; Ctrl means Cmd on macOS, as in
  //! ImGui). @return chord, or 0 if the shortcut is empty or not understood
  static int keyChord(const String & shortcut);

  protected:

  std::vector<std::unique_ptr<Menu>> _menus;
  std::vector<ActionListener*> _listeners;
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__MenuBar
