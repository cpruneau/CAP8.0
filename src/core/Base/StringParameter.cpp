/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "StringParameter.hpp"
#include "BinaryIO.hpp"
#include "PrintHelpers.hpp"

namespace CAP
{

  StringParameter::StringParameter(const String & name, const String & value)
  :
  Object(name,name,"StringParameter"),
  _value(value)
  { }

  void StringParameter::print() const
  {
  printValue(name(),_value);
  }

  void StringParameter::encodeBinary(BinaryWriter & w) const
  {
  w.writeString(_value);
  }

  void StringParameter::decodeBinary(BinaryReader & r, const String & name, unsigned int /*version*/)
  {
  String value = r.readString();
  setName(name);
  setTitle(name);
  _value = value;
  }

} // namespace CAP
