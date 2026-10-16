#ifndef CAP__GUI__MenuItem
#define CAP__GUI__MenuItem
#include <functional>
#include <vector>
#include "String.hpp"
#include "Action.hpp"

namespace CAP
{
  namespace GUI
  {

  //!
  //! @brief How a menu item is drawn: text color (alpha 0 keeps the theme color) and an optional icon (UTF-8 text
  //! drawn before the name).
  //!
  struct MenuItemStyle
  {
  float  red   = 0.0f;
  float  green = 0.0f;
  float  blue  = 0.0f;
  float  alpha = 0.0f;
  String icon;
  };

  //!
  //! @brief Entry of a menu: name shown, command sent to the listeners, description (tooltip and status bar),
  //! keyboard shortcut, enabled and checked states, style, and an optional callback.
  //!
  //! When the item is selected (clicked, or its shortcut pressed), fire() runs the callback, if any, then calls
  //! actionPerformed() on every listener with an ActionEvent{command, this}.
  //!
  //! @code
  //!   MenuItem & open = fileMenu.addItem("Open...", "file.open", "Ctrl+O", "Open a CAP file");
  //!   open.addListener(&app);                                 // the application handles "file.open"
  //!   MenuItem & about = helpMenu.addItem("About CAP", "help.about");
  //!   about.setCallback([](MenuItem &) { showAbout(); });     // or a one-off callback
  //! @endcode
  //!
  class MenuItem
  {
  public:

  using Callback = std::function<void(MenuItem &)>;

  //! @brief Item. @param name text shown @param command action identifier (the name if empty)
  //! @param shortcut key chord, e.g. "Ctrl+O", "Ctrl+Shift+S", "Delete", "F1" (Ctrl is Cmd on macOS)
  //! @param description tooltip and status bar text
  MenuItem(const String & name, const String & command="", const String & shortcut="", const String & description="");

  const String & name() const        { return _name; }
  const String & command() const     { return _command; }
  const String & shortcut() const    { return _shortcut; }
  const String & description() const { return _description; }
  void setName(const String & name)               { _name = name; }
  void setShortcut(const String & shortcut)       { _shortcut = shortcut; }
  void setDescription(const String & description) { _description = description; }

  bool isEnabled() const   { return _enabled; }
  bool isCheckable() const { return _checkable; }
  bool isChecked() const   { return _checked; }
  void setEnabled(bool v)  { _enabled = v; }
  //! @brief A checkable item shows a check mark and toggles isChecked() when selected.
  void setCheckable(bool v) { _checkable = v; }
  void setChecked(bool v)   { _checked = v; }

  MenuItemStyle & style() { return _style; }
  const MenuItemStyle & style() const { return _style; }

  //! @brief Function run when the item fires (before the listeners). @return this item
  MenuItem & setCallback(const Callback & callback) { _callback = callback; return *this; }
  //! @brief Add a listener (not owned). @return this item
  MenuItem & addListener(ActionListener * listener);
  void removeListener(ActionListener * listener);

  //! @brief Toggle a checkable item, run the callback, notify the listeners. Does nothing if disabled.
  void fire();

  protected:

  String _name;
  String _command;
  String _shortcut;
  String _description;
  bool   _enabled;
  bool   _checkable;
  bool   _checked;
  MenuItemStyle _style;
  Callback _callback;
  std::vector<ActionListener*> _listeners;
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__MenuItem
