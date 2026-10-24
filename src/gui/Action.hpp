/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Action.hpp
//! @brief ActionEvent and ActionListener: actions of the menus of CapGui.
#ifndef CAP__GUI__Action
#define CAP__GUI__Action
#include "String.hpp"

namespace CAP
{
  namespace GUI
  {

  class MenuItem;

  //!
  //! @brief Event sent when a menu item (or a button, a shortcut, ...) is activated.
  //!
  //! command identifies the action ("file.open", "edit.undo", ...); source is the menu item that fired it, or nullptr
  //! when the action comes from elsewhere (a toolbar button, a drop, a script).
  //!
  struct ActionEvent
  {
  String     command;   //!< command identifying the action, e.g. "file.open"
  MenuItem * source;   //!< menu item that fired the action, or nullptr
  };

  //!
  //! @brief Receiver of actions: implement actionPerformed() and register with MenuItem::addListener() (or
  //! MenuBar::addListener() for all the items of a menu bar).
  //!
  //! CapApp is the main listener: every menu command goes through it, so that the application (not the window)
  //! decides what each command does. A menu item can also carry its own callback (MenuItem::setCallback()), handy
  //! for one-off items; the callback runs first, then the listeners.
  //!
  class ActionListener
  {
  public:
  //! @brief Destructor.
  virtual ~ActionListener() { }
  //! @brief Called when an action happens. @param event the action
  virtual void actionPerformed(const ActionEvent & event) = 0;
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__Action
