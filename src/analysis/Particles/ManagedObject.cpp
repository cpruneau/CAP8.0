/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ManagedObject.hpp"

ClassImp(CAP::ManagedObject);

namespace CAP
{

  ManagedObject::ManagedObject()
  :
  Object()
  { /* no ops */}

  ManagedObject::ManagedObject(const ManagedObject & source)
  :
  Object(source)
  { /* no ops */}

  ManagedObject & ManagedObject::operator=(const ManagedObject & rhs)
  {
  if (this!=&rhs)
    {
    Object::operator=(rhs);
    }
  return *this;
  }

} // namespace CAP
