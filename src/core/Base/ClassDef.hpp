/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ClassDef.hpp
//! @brief ROOT's ClassDef and ClassImp macros, for the classes that still have ROOT dictionaries.
//!
//! Only the headers of classes declared with ClassDef include it: the other CAP headers do not see ROOT. ClassDef
//! adds virtual functions to a class, so it cannot be defined as nothing while the dictionaries exist; when a library
//! drops its dictionary, its classes drop ClassDef and this include.
#ifndef CAP__ClassDef
#define CAP__ClassDef
#include "Rtypes.h"
#endif // !CAP__ClassDef
