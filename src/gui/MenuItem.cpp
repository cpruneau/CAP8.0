/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file MenuItem.cpp
//! @brief Implementation of MenuItem.
#include <algorithm>
#include "MenuItem.hpp"

namespace CAP
{
  namespace GUI
  {

  MenuItem::MenuItem(const String & name, const String & command, const String & shortcut, const String & description)
  :
  _name(name),
  _command(command.isNull() ? name : command),
  _shortcut(shortcut),
  _description(description),
  _enabled(true),
  _checkable(false),
  _checked(false),
  _style(),
  _callback(),
  _listeners()
  { }

  MenuItem & MenuItem::addListener(ActionListener * listener)
  {
  if (listener && std::find(_listeners.begin(), _listeners.end(), listener) == _listeners.end())
    _listeners.push_back(listener);
  return *this;
  }

  void MenuItem::removeListener(ActionListener * listener)
  {
  _listeners.erase(std::remove(_listeners.begin(), _listeners.end(), listener), _listeners.end());
  }

  void MenuItem::fire()
  {
  if (!_enabled) return;
  if (_checkable) _checked = !_checked;
  if (_callback) _callback(*this);
  ActionEvent event{_command, this};
  std::vector<ActionListener*> listeners = _listeners;   // a listener may change the list
  for (auto listener : listeners) listener->actionPerformed(event);
  }

  } // namespace GUI
} // namespace CAP
