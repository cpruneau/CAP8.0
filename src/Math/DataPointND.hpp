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
#ifndef CAP_MATH_DataPointND
#define CAP_MATH_DataPointND
#include <ostream>
#include <vector>
#include "Aliases.hpp"
#include "BinaryIO.hpp"
#include "DataPoint.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Data point of rank n>2: n independent variables x[0..n-1] and one dependent value, with uncertainties.
  //!
  //! The rank is set at construction (minimum 3; use DataPoint2D for rank 1 and DataPoint3D for rank 2). It changes
  //! only through operator= or set(), which take the rank of their argument.
  //! Uncertainties on the independent variables are treated as independent.
  //!
  //! Arithmetic follows DataPoint: operations between points require the same rank and compatible independent
  //! variables, and act on the dependent value. The binary operators (+, -, *, /, unary -) are the DataPoint templates.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::DataPointND p({1.0, 2.0, 3.0}, {0.1, 0.1, 0.1}, 5.0, 0.4, 0.3);  // x, xErrors, value, stat, syst
  //!   p.setX(2, 3.5);
  //! @endcode
  //!
  class DataPointND : public DataPoint
  {
  protected:

  std::vector<float> _x;       //!< independent variables
  std::vector<float> _xError;  //!< uncertainties on the independent variables

  public:

  //! @name Construction and assignment
  //! @{

  //! @brief Default constructor: rank 3, all values and uncertainties zero.
  DataPointND();
  //! @brief Point of the given rank, all values and uncertainties zero. @param rank number of independent variables (>= 3)
  //! @throws MathException if rank<3
  explicit DataPointND(unsigned int rank);
  //! @brief Point without uncertainties on the independent variables.
  //! @param x independent variables (size >= 3) @param value dependent value @param stat statistical uncertainty
  //! @param syst systematic uncertainty @throws MathException if x.size()<3 or an uncertainty is negative
  DataPointND(const std::vector<double> & x, double value, double stat=0.0, double syst=0.0);
  //! @brief Point with uncertainties on the independent variables.
  //! @param x independent variables (size >= 3) @param xError their uncertainties (same size, >= 0)
  //! @param value dependent value @param stat statistical uncertainty @param syst systematic uncertainty
  //! @throws MathException if the sizes differ, x.size()<3, or an uncertainty is negative
  DataPointND(const std::vector<double> & x, const std::vector<double> & xError, double value, double stat=0.0, double syst=0.0);
  //! @brief Copy constructor. @param source point to copy
  DataPointND(const DataPointND & source);
  //! @brief Destructor.
  virtual ~DataPointND() {}
  //! @brief Assignment operator (the rank is copied too). @param rhs point to copy @return reference to this point
  DataPointND & operator=(const DataPointND & rhs);
  //! @brief Polymorphic copy. @return new DataPointND (owned by the caller)
  DataPointND * clone() const override { return new DataPointND(*this); }

  //! @}

  //! @name DataPoint interface
  //! @{

  //! @brief Number of independent variables. @return rank (>= 3)
  unsigned int rank() const override { return (unsigned int)_x.size(); }
  //! @brief Independent variable index. @param index 0 <= index < rank() @return value
  float x(unsigned int index) const override;
  //! @brief Uncertainty on independent variable index. @param index 0 <= index < rank() @return uncertainty
  float xError(unsigned int index) const override;
  //! @brief Set independent variable index. @param index 0 <= index < rank() @param value new value
  void setX(unsigned int index, double value) override;
  //! @brief Set the uncertainty on independent variable index. @param index 0 <= index < rank() @param error uncertainty (>= 0)
  void setXError(unsigned int index, double error) override;

  //! @}

  //! @brief Set all values and uncertainties; the rank becomes x.size().
  //! @param x independent variables (size >= 3) @param xError their uncertainties (same size, >= 0)
  //! @param value dependent value @param stat statistical uncertainty @param syst systematic uncertainty
  //! @throws MathException if the sizes differ, x.size()<3, or an uncertainty is negative (the point is then unchanged)
  void set(const std::vector<double> & x, const std::vector<double> & xError, double value, double stat=0.0, double syst=0.0);

  //! @name Arithmetic
  //! Operations between points require the same rank and compatible independent variables (MathException otherwise).
  //! @{

  //! @brief value = value + rhs.value. @param rhs point @return reference to this point
  DataPointND & operator+=(const DataPointND & rhs) { add(rhs);      return *this; }
  //! @brief value = value - rhs.value. @param rhs point @return reference to this point
  DataPointND & operator-=(const DataPointND & rhs) { subtract(rhs); return *this; }
  //! @brief value = value * rhs.value. @param rhs point @return reference to this point
  DataPointND & operator*=(const DataPointND & rhs) { multiply(rhs); return *this; }
  //! @brief value = value / rhs.value. @param rhs point @return reference to this point @throws MathException if rhs.value==0
  DataPointND & operator/=(const DataPointND & rhs) { divide(rhs);   return *this; }
  //! @brief Add a constant to the value. @param rhs constant @return reference to this point
  DataPointND & operator+=(double rhs) { shift(rhs);  return *this; }
  //! @brief Subtract a constant from the value. @param rhs constant @return reference to this point
  DataPointND & operator-=(double rhs) { shift(-rhs); return *this; }
  //! @brief Multiply the value and its uncertainties by a constant. @param rhs constant @return reference to this point
  DataPointND & operator*=(double rhs) { scale(rhs);  return *this; }
  //! @brief Divide the value and its uncertainties by a constant. @param rhs constant @return reference to this point @throws MathException if rhs==0
  DataPointND & operator/=(double rhs);

  //! @}

  //! @name Binary encoding
  //! The rank (varint), then x[0], xError[0], ..., x[n-1], xError[n-1], value, stat, syst as float32. Restored exactly.
  //! @{
  //! @brief Write this point. @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Read this point. @param r reader @throws Exception if the data is invalid
  void decodeBinary(BinaryReader & r);
  //! @}

  //! @name Binary files (independent of ROOT)
  //! Saved as "CAPN" records in the CAP binary format (see BinaryIO.hpp).
  //! @{

  //! @brief Tag of DataPointND records in binary files. @return "CAPN"
  static const char * binaryTag() { return "CAPN"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 1; }
  //! @brief Name stored in binary records. @return ""
  String binaryName() const { return ""; }
  //! @brief Rebuild this object from a binary payload. @param r reader @param name stored name @param version format version
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write this point as one binary record to a stream. @param out stream @throws FileException if writing fails
  void writeBinary(std::ostream & out) const;
  //! @brief Read the next DataPointND record of a stream into this object. @param in stream @return false at the end of the stream
  bool readBinary(std::istream & in);
  //! @brief Save this point to a binary file. @param fileName file name @param append add at the end of the file instead of replacing it
  void saveBinary(const String & fileName, bool append=false) const;
  //! @brief Load the point at position index among the DataPointND objects of a binary file. @param fileName file name @param index position
  void loadBinary(const String & fileName, unsigned int index=0);
  //! @brief Save several points to one binary file (replacing it). @param fileName file name @param objects points to save
  static void saveBinary(const String & fileName, const std::vector<DataPointND> & objects);
  //! @brief Load all the DataPointND objects of a binary file, in order. @param fileName file name @return points
  static std::vector<DataPointND> loadAllBinary(const String & fileName);

  //! @}

  ClassDefOverride(DataPointND,1)
  };

  } // namespace MATH
} // namespace CAP

#endif
