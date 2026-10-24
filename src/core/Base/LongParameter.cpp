/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "LongParameter.hpp"
#include "PrintHelpers.hpp"

ClassImp(CAP::LongParameter);

namespace CAP
{

  LongParameter::LongParameter(const String & name, long value)
  :
  Object(name,name,"LongParameter"),
  _value(value)
  { }

  void LongParameter::print() const
  {
  printValue(name(),_value);
  }

  void LongParameter::encodeBinary(BinaryWriter & w) const
  {
  w.writeVarInt(int64_t(_value));
  }

  void LongParameter::decodeBinary(BinaryReader & r, const String & name, unsigned int /*version*/)
  {
  long value = long(r.readVarInt());
  setName(name);
  setTitle(name);
  _value = value;
  }

} // namespace CAP
