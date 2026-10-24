/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP_MATH_DataPoint3D
#define CAP_MATH_DataPoint3D
#include <ostream>
#include <vector>
#include "Aliases.hpp"
#include "BinaryIO.hpp"
#include "DataPoint.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Data point of rank 2: independent variables x and y, dependent value z, with uncertainties.
  //!
  //!  - x, y : independent variables (indices 0 and 1), with uncertainties xError and yError, treated as independent
  //!  - z    : dependent value (DataPoint::value()), with statistical uncertainty zStatError (DataPoint::stat())
  //!           and systematic uncertainty zSysError (DataPoint::syst())
  //!
  //! Arithmetic follows DataPoint: operations between points act on z and require compatible x and y;
  //! operations with a number act on z only. The binary operators (+, -, *, /, unary -) are the DataPoint templates.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::DataPoint3D p(1.0, 0.1, 2.0, 0.2, 5.0, 0.4, 0.3);  // x, xError, y, yError, z, zStatError, zSysError
  //!   CAP::MATH::DataPoint3D q = 2.0*p;                              // z=10, zStatError=0.8, zSysError=0.6
  //! @endcode
  //!
  class DataPoint3D : public DataPoint
  {
  protected:

  float _x;       //!< x value
  float _xError;  //!< uncertainty on x
  float _y;       //!< y value
  float _yError;  //!< uncertainty on y

  public:

  //! @name Construction and assignment
  //! @{

  //! @brief Default constructor: all values and uncertainties are zero.
  DataPoint3D();
  //! @brief Point without uncertainties. @param x x value @param y y value @param z dependent value
  DataPoint3D(double x, double y, double z);
  //! @brief Point with uncertainties on z only.
  //! @param x x value @param y y value @param z dependent value @param zStatError statistical uncertainty on z
  //! @param zSysError systematic uncertainty on z
  DataPoint3D(double x, double y, double z, double zStatError, double zSysError=0.0);
  //! @brief Point with all uncertainties. Note the argument order: x, xError, y, yError, z, zStatError, zSysError.
  //! @param x x value @param xError uncertainty on x @param y y value @param yError uncertainty on y
  //! @param z dependent value @param zStatError statistical uncertainty on z @param zSysError systematic uncertainty on z
  //! @throws MathException if an uncertainty is negative
  DataPoint3D(double x, double xError, double y, double yError, double z, double zStatError, double zSysError);
  //! @brief Copy constructor. @param source point to copy
  DataPoint3D(const DataPoint3D & source);
  //! @brief Destructor.
  virtual ~DataPoint3D() {}
  //! @brief Assignment operator. @param rhs point to copy @return reference to this point
  DataPoint3D & operator=(const DataPoint3D & rhs);
  //! @brief Polymorphic copy. @return new DataPoint3D (owned by the caller)
  DataPoint3D * clone() const override { return new DataPoint3D(*this); }

  //! @}

  //! @name DataPoint interface
  //! @{

  //! @brief Number of independent variables. @return 2
  unsigned int rank() const override { return 2; }
  //! @brief Independent variable: index 0 is x, 1 is y. @param index 0 or 1 @return value
  float x(unsigned int index) const override;
  //! @brief Uncertainty on an independent variable. @param index 0 or 1 @return uncertainty
  float xError(unsigned int index) const override;
  //! @brief Set an independent variable. @param index 0 or 1 @param value new value
  void setX(unsigned int index, double value) override;
  //! @brief Set the uncertainty on an independent variable. @param index 0 or 1 @param error uncertainty (>= 0)
  void setXError(unsigned int index, double error) override;

  //! @}

  //! @name Values and uncertainties
  //! @{

  //! @brief x value. @return x
  float x() const { return _x; }
  //! @brief Uncertainty on x. @return xError
  float xError() const { return _xError; }
  //! @brief y value. @return y
  float y() const { return _y; }
  //! @brief Uncertainty on y. @return yError
  float yError() const { return _yError; }
  //! @brief Dependent value z. @return z
  float z() const { return _value; }
  //! @brief Statistical uncertainty on z. @return zStatError
  float zStatError() const { return _stat; }
  //! @brief Systematic uncertainty on z. @return zSysError
  float zSysError() const { return _syst; }
  //! @brief Total uncertainty on z, statistical and systematic added in quadrature. @return sqrt(zStatError^2+zSysError^2)
  float zError() const { return error(); }

  //! @brief Set all values and uncertainties.
  //! @param x x value @param xError uncertainty on x @param y y value @param yError uncertainty on y
  //! @param z dependent value @param zStatError statistical uncertainty on z @param zSysError systematic uncertainty on z
  //! @throws MathException if an uncertainty is negative
  void set(double x, double xError, double y, double yError, double z, double zStatError, double zSysError=0.0);
  //! @brief Set x, y and z; all uncertainties are reset to zero. @param x x value @param y y value @param z dependent value
  void setPoint(double x, double y, double z);
  //! @brief Set the uncertainty on x. @param xError uncertainty (>= 0) @throws MathException if xError<0
  void setXError(double xError);
  //! @brief Set the uncertainty on y. @param yError uncertainty (>= 0) @throws MathException if yError<0
  void setYError(double yError);

  //! @}

  //! @name Arithmetic
  //! Operations between points require compatible x and y (MathException otherwise).
  //! @{

  //! @brief z = z + rhs.z. @param rhs point @return reference to this point
  DataPoint3D & operator+=(const DataPoint3D & rhs) { add(rhs);      return *this; }
  //! @brief z = z - rhs.z. @param rhs point @return reference to this point
  DataPoint3D & operator-=(const DataPoint3D & rhs) { subtract(rhs); return *this; }
  //! @brief z = z * rhs.z. @param rhs point @return reference to this point
  DataPoint3D & operator*=(const DataPoint3D & rhs) { multiply(rhs); return *this; }
  //! @brief z = z / rhs.z. @param rhs point @return reference to this point @throws MathException if rhs.z==0
  DataPoint3D & operator/=(const DataPoint3D & rhs) { divide(rhs);   return *this; }
  //! @brief Add a constant to z. @param rhs constant @return reference to this point
  DataPoint3D & operator+=(double rhs) { shift(rhs);  return *this; }
  //! @brief Subtract a constant from z. @param rhs constant @return reference to this point
  DataPoint3D & operator-=(double rhs) { shift(-rhs); return *this; }
  //! @brief Multiply z and its uncertainties by a constant (see DataPoint::scale()). @param rhs constant @return reference to this point
  DataPoint3D & operator*=(double rhs) { scale(rhs);  return *this; }
  //! @brief Divide z and its uncertainties by a constant. @param rhs constant @return reference to this point @throws MathException if rhs==0
  DataPoint3D & operator/=(double rhs);

  //! @}

  using DataPoint::print;
  //! @brief Write the point as x +- xError, y +- yError, z +- zStatError (stat) +- zSysError (syst). @param out output stream
  void print(std::ostream & out) const override;

  //! @name Binary encoding
  //! A mask byte tells which of the seven numbers (x, xError, y, yError, z, zStatError, zSysError) are non-zero;
  //! only those are written (float32). Restored exactly.
  //! @{
  //! @brief Write this point. @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Read this point. @param r reader @throws Exception if the data is invalid
  void decodeBinary(BinaryReader & r);
  //! @}

  //! @name Binary files (independent of ROOT)
  //! Saved as "CAPT" records in the CAP binary format (see BinaryIO.hpp).
  //! @{

  //! @brief Tag of DataPoint3D records in binary files. @return "CAPT"
  static const char * binaryTag() { return "CAPT"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 1; }
  //! @brief Name stored in binary records. @return ""
  String binaryName() const { return ""; }
  //! @brief Rebuild this object from a binary payload. @param r reader @param name stored name @param version format version
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write this point as one binary record to a stream. @param out stream @throws FileException if writing fails
  void writeBinary(std::ostream & out) const;
  //! @brief Read the next DataPoint3D record of a stream into this object. @param in stream @return false at the end of the stream
  bool readBinary(std::istream & in);
  //! @brief Save this point to a binary file. @param fileName file name @param append add at the end of the file instead of replacing it
  void saveBinary(const String & fileName, bool append=false) const;
  //! @brief Load the point at position index among the DataPoint3D objects of a binary file. @param fileName file name @param index position
  void loadBinary(const String & fileName, unsigned int index=0);
  //! @brief Save several points to one binary file (replacing it). @param fileName file name @param objects points to save
  static void saveBinary(const String & fileName, const std::vector<DataPoint3D> & objects);
  //! @brief Load all the DataPoint3D objects of a binary file, in order. @param fileName file name @return points
  static std::vector<DataPoint3D> loadAllBinary(const String & fileName);

  //! @}

  ClassDefOverride(DataPoint3D,1)
  };

  } // namespace MATH
} // namespace CAP

#endif
