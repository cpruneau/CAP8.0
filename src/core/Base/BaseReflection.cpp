/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file BaseReflection.cpp
//! @brief Classes of the Base library described for the run-time reflection (see Reflection.hpp).
#include "Reflection.hpp"
#include "Object.hpp"
#include "TaskRegistration.hpp"

DATUM_REFLECTION(Base)
{
  using namespace Datum;
  using namespace Datum::REFLECTION;
  ClassBuilder<Object>("Datum::Object")
    .property("name", &Object::name, &Object::setName)
    .property("title", &Object::title, &Object::setTitle)
    .method("print", &Object::print);
  registerTask<Task>("Datum::Task");
}
