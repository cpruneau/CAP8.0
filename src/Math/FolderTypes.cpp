/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//!
//! Registers the Math classes with binary support in the type registry of CAP::Folder (Base), so that folders
//! saved with these objects can be loaded. Runs when the Math library is loaded.
//!
#include "Folder.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "DataPoint2D.hpp"
#include "DataPointsVector.hpp"
#include "DataPointColumns.hpp"
#include "ScalarField.hpp"
#include "Matrix.hpp"
#include "VectorND.hpp"
#include "Vector2D.hpp"
#include "Vector3D.hpp"
#include "VectorLorentz.hpp"
#include "Parameter.hpp"
#include "Table2D.hpp"

namespace CAP
{
  namespace MATH
  {

  //! @brief Register the Math classes in the Folder type registry (called automatically when the library is loaded;
  //! calling it again is harmless). @return true
  bool registerFolderTypes()
  {
  Folder::registerType<Histogram>();
  Folder::registerType<Profile>();
  Folder::registerType<DataPointsVector>();
  Folder::registerType<DataPointColumns>();
  Folder::registerType<DataPoint2D>();
  Folder::registerType<ScalarField>();
  Folder::registerType<Matrix>();
  Folder::registerType<VectorND>();
  Folder::registerType<Vector2D>();
  Folder::registerType<Vector3D>();
  Folder::registerType<VectorLorentz>();
  Folder::registerType<Parameter>();
  Folder::registerType<Table2D>();
  return true;
  }

  namespace
  {
  [[maybe_unused]] const bool folderTypesRegistered = registerFolderTypes();
  }

  } // namespace MATH
} // namespace CAP
