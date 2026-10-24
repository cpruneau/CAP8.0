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
#ifndef CAP__LongParameter
#define CAP__LongParameter
#include "Object.hpp"
#include "BinaryIO.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Named integer value (64 bits) that can be stored in folders and CAP binary files.
//!
//! LongParameter replaces TParameter<Long64_t> for the parameters (event counts, accountant entries, ...) saved in
//! CapFile objects: see CapFile::exportParameter() and CapFile::importParameter(). Stored as "CAPI" records of the CAP
//! binary format (see BinaryIO.hpp); the class is registered with Folder.
//!
class LongParameter : public Object
{
public:

  //! @brief Constructor. @param name name of the parameter @param value value
  LongParameter(const String & name="LongParameter", long value=0);
  //! @brief Copy constructor. @param source parameter to copy
  LongParameter(const LongParameter & source) = default;
  //! @brief Copy assignment. @param source parameter to copy @return this parameter
  LongParameter & operator=(const LongParameter & source) = default;
  //! @brief Destructor.
  virtual ~LongParameter() {}

  //! @brief Value of the parameter. @return value
  long value() const { return _value; }
  //! @brief Set the value of the parameter. @param value new value
  void setValue(long value) { _value = value; }

  //! @brief Print the name and value of the parameter.
  virtual void print() const override;

  //! @name Binary files (independent of ROOT)
  //! @{
  //! @brief Tag of the records of this class. @return "CAPI"
  static const char * binaryTag() { return "CAPI"; }
  //! @brief Version of the record format. @return version
  static unsigned int binaryVersion() { return 1; }
  //! @brief Name stored in the record. @return name
  String binaryName() const { return name(); }
  //! @brief Encode the value. @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Decode the value. @param r reader @param name name of the record @param version version of the record
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @}

protected:

  long _value; //!< value of the parameter

  ClassDefOverride(LongParameter,1)
};

} // namespace CAP

#endif
