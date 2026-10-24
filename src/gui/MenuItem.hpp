/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file MenuItem.hpp
//! @brief MenuItem and MenuItemStyle: entry of a menu of CapGui.
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
  float  red   = 0.0f;   //!< red component of the text color, in [0,1]
  float  green = 0.0f;   //!< green component of the text color
  float  blue  = 0.0f;   //!< blue component of the text color
  float  alpha = 0.0f;   //!< opacity of the text color (0: the color of the theme)
  String icon;   //!< text drawn before the name (e.g. a symbol), or empty
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

  using Callback = std::function<void(MenuItem &)>;   //!< Function run when the item fires

  //! @brief Item. @param name text shown @param command action identifier (the name if empty)
  //! @param shortcut key chord, e.g. "Ctrl+O", "Ctrl+Shift+S", "Delete", "F1" (Ctrl is Cmd on macOS)
  //! @param description tooltip and status bar text
  MenuItem(const String & name, const String & command="", const String & shortcut="", const String & description="");

  const String & name() const        { return _name; }   //!< Text shown
  const String & command() const     { return _command; }   //!< Action identifier sent to the listeners
  const String & shortcut() const    { return _shortcut; }   //!< Keyboard shortcut, e.g. "Ctrl+O", or empty
  const String & description() const { return _description; }   //!< Tooltip and status bar text
  void setName(const String & name)               { _name = name; }   //!< Set the text shown
  void setShortcut(const String & shortcut)       { _shortcut = shortcut; }   //!< Set the keyboard shortcut
  void setDescription(const String & description) { _description = description; }   //!< Set the tooltip and status bar text

  bool isEnabled() const   { return _enabled; }   //!< Whether the item can be selected
  bool isCheckable() const { return _checkable; }   //!< Whether the item shows a check mark
  bool isChecked() const   { return _checked; }   //!< Whether the check mark is shown
  void setEnabled(bool v)  { _enabled = v; }   //!< Enable or disable the item
  //! @brief A checkable item shows a check mark and toggles isChecked() when selected.
  void setCheckable(bool v) { _checkable = v; }
  void setChecked(bool v)   { _checked = v; }   //!< Show or hide the check mark

  MenuItemStyle & style() { return _style; }   //!< Color and icon of the item
  const MenuItemStyle & style() const { return _style; }   //!< Color and icon of the item

  //! @brief Function run when the item fires (before the listeners). @return this item
  MenuItem & setCallback(const Callback & callback) { _callback = callback; return *this; }
  //! @brief Add a listener (not owned). @return this item
  MenuItem & addListener(ActionListener * listener);
  //! @brief Remove a listener (nothing if it is not registered). @param listener listener
  void removeListener(ActionListener * listener);

  //! @brief Toggle a checkable item, run the callback, notify the listeners. Does nothing if disabled.
  void fire();

  protected:

  String _name;   //!< text shown
  String _command;   //!< action identifier
  String _shortcut;   //!< keyboard shortcut
  String _description;   //!< tooltip and status bar text
  bool   _enabled;   //!< the item can be selected
  bool   _checkable;   //!< the item shows a check mark
  bool   _checked;   //!< the check mark is shown
  MenuItemStyle _style;   //!< color and icon
  Callback _callback;   //!< function run when the item fires (may be empty)
  std::vector<ActionListener*> _listeners;   //!< listeners notified when the item fires (not owned)
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__MenuItem
