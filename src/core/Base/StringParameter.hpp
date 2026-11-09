/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__StringParameter
#define CAP__StringParameter
#include "Object.hpp"
#include "String.hpp"

namespace CAP
{

class BinaryWriter;
class BinaryReader;

//!
//! @brief A named text value saved in CAP files (e.g. the settings of the kinematic maps of an efficiency or smearer:
//! source, variables, distribution, species). Stored as "CAPS" records of the CAP binary format (see BinaryIO.hpp); the
//! class is registered with Folder, as LongParameter.
//!
class StringParameter : public Object
{
public:

  //! @brief Constructor. @param name name of the parameter @param value value
  StringParameter(const String & name="StringParameter", const String & value="");
  //! @brief Copy constructor. @param source parameter to copy
  StringParameter(const StringParameter & source) = default;
  //! @brief Copy assignment. @param source parameter to copy @return this parameter
  StringParameter & operator=(const StringParameter & source) = default;
  //! @brief Destructor.
  virtual ~StringParameter() {}

  //! @brief Value of the parameter. @return value
  const String & value() const { return _value; }
  //! @brief Set the value of the parameter. @param value new value
  void setValue(const String & value) { _value = value; }

  //! @brief Print the name and value of the parameter.
  virtual void print() const override;

  //! @name Binary files (independent of ROOT)
  //! @{
  //! @brief Tag of the records of this class. @return "CAPS"
  static const char * binaryTag() { return "CAPS"; }
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

  String _value; //!< value of the parameter

};

} // namespace CAP

#endif
