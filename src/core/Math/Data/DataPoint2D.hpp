#ifndef CAP_MATH_DataPoint2D
#define CAP_MATH_DataPoint2D
#include <cmath>
#include <ostream>
#include <vector>
#include "String.hpp"
#include "BinaryIO.hpp"
#include "DataPoint.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Data point of rank 1: one independent variable x and one dependent value y, with uncertainties.
  //!
  //!  - x : independent (control) variable, with uncertainty xError
  //!  - y : dependent value (DataPoint::value()), with statistical uncertainty yStatError (DataPoint::stat())
  //!        and systematic uncertainty ySysError (DataPoint::syst())
  //!  - xyCovariance : statistical covariance between x and y
  //!
  //! The statistical uncertainties can be set as a whole from a 2 by 2 error (covariance) matrix
  //! @code
  //!   | var(x)    cov(x,y) |   =   | xError^2             xyCovariance |
  //!   | cov(x,y)  var(y)   |       | xyCovariance   yStatError^2       |
  //! @endcode
  //! with setErrorMatrix(), and read back with errorMatrix(). The systematic uncertainty on y is
  //! not part of the matrix.
  //!
  //! <b>Arithmetic.</b> Operations between two points act on y and require compatible x values
  //! (see DataPoint::compatibleX()). The result has the mean x and the mean x uncertainty of the operands.
  //! Statistical and systematic y uncertainties are propagated separately, to first order,
  //! assuming the two points are independent. The x-y covariance is propagated to first order
  //! as well. Operations with a number (e.g., p*2.0, p+1.0) act on y only. The binary operators
  //! (+, -, *, /, unary -) are the DataPoint templates.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::DataPoint2D p(1.0, 0.1, 5.0, 0.4, 0.2);  // x, xError, y, yStatError, ySysError
  //!   p.setErrorMatrix(0.01, 0.02, 0.16);                 // var(x), cov(x,y), var(y)
  //!   double rho = p.correlation();                       // 0.5
  //! @endcode
  //!
  class DataPoint2D : public DataPoint
  {
  protected:

  float _x;             //!< x value
  float _xError;        //!< uncertainty on x
  float _xyCovariance;  //!< statistical covariance between x and y

  public:

  //! @name Construction and assignment
  //! @{

  //! @brief Default constructor: all values and uncertainties are zero.
  DataPoint2D();
  //! @brief Point without uncertainties. @param x x value @param y y value
  DataPoint2D(float x, float y);
  //! @brief Point with a statistical uncertainty on y. @param x x value @param y y value @param yStatError statistical uncertainty on y
  DataPoint2D(float x, float y, float yStatError);
  //! @brief Point with statistical and systematic uncertainties on y.
  //! @param x x value @param y y value @param yStatError statistical uncertainty on y @param ySysError systematic uncertainty on y
  DataPoint2D(float x, float y, float yStatError, float ySysError);
  //! @brief Point with uncertainties on x and y. Note the argument order: x, xError, y, yStatError, ySysError.
  //! @param x x value @param xError uncertainty on x @param y y value
  //! @param yStatError statistical uncertainty on y @param ySysError systematic uncertainty on y
  DataPoint2D(float x, float xError, float y, float yStatError, float ySysError);
  //! @brief Copy constructor. @param source point to copy
  DataPoint2D(const DataPoint2D & source);
  //! @brief Destructor.
  virtual ~DataPoint2D() {}
  //! @brief Assignment operator. @param rhs point to copy @return reference to this point
  DataPoint2D & operator= (const DataPoint2D & rhs);
  //! @brief Polymorphic copy. @return new DataPoint2D (owned by the caller)
  DataPoint2D * clone() const override { return new DataPoint2D(*this); }

  //! @}

  //! @name DataPoint interface
  //! @{

  //! @brief Number of independent variables. @return 1
  unsigned int rank() const override { return 1; }
  //! @brief x value. @param index must be 0 @return x
  float x(unsigned int index) const override;
  //! @brief Uncertainty on x. @param index must be 0 @return xError
  float xError(unsigned int index) const override;
  //! @brief Set x. @param index must be 0 @param value x value
  void setX(unsigned int index, double value) override;
  //! @brief Set the uncertainty on x. @param index must be 0 @param error uncertainty (>= 0)
  void setXError(unsigned int index, double error) override;

  //! @}

  //! @name Values and uncertainties
  //! @{

  //! @brief x value. @return x
  float x() const { return _x; }
  //! @brief Uncertainty on x. @return xError
  float xError() const { return _xError; }
  //! @brief y value. @return y
  float y() const { return _value; }
  //! @brief Statistical uncertainty on y. @return yStatError
  float yStatError() const { return _stat; }
  //! @brief Systematic uncertainty on y. @return ySysError
  float ySysError() const { return _syst; }
  //! @brief Systematic uncertainty on y (same as ySysError()). @return ySysError
  float ySystError() const { return _syst; }
  //! @brief Total uncertainty on y, statistical and systematic added in quadrature. @return sqrt(yStatError^2 + ySysError^2)
  float yError() const { return error(); }
  //! @brief Statistical covariance between x and y. @return cov(x,y)
  float xyCovariance() const { return _xyCovariance; }
  //! @brief Statistical correlation coefficient between x and y. @return cov(x,y)/(xError*yStatError), or 0 if either uncertainty is zero
  float correlation() const;

  //!
  //! @brief Set all values and uncertainties; the x-y covariance is reset to zero.
  //! @param x x value @param xError uncertainty on x @param y y value
  //! @param yStatError statistical uncertainty on y @param ySysError systematic uncertainty on y
  //!
  void set(float x, float xError, float y, float yStatError, float ySysError=0.0);

  //! @brief Set x and y; all uncertainties and the covariance are reset to zero. @param x x value @param y y value
  void setPoint(double x, double y);
  //! @brief Set x, y and the statistical uncertainty on y; other uncertainties and the covariance are reset to zero.
  //! @param x x value @param y y value @param yStatError statistical uncertainty on y
  void setPoint(double x, double y, double yStatError);
  //! @brief Set x, y and both uncertainties on y; the x uncertainty and the covariance are reset to zero.
  //! @param x x value @param y y value @param yStatError statistical uncertainty on y @param ySysError systematic uncertainty on y
  void setPoint(double x, double y, double yStatError, double ySysError);

  //! @brief Set the uncertainty on x (the covariance is kept). @param xError uncertainty on x (must be >= 0)
  //! @throws MathException if xError<0
  void setXError(double xError);
  //! @brief Set the statistical uncertainty on y (the covariance is kept). @param yStatError statistical uncertainty on y (must be >= 0)
  //! @throws MathException if yStatError<0
  void setYStatError(double yStatError) { setStat(yStatError); }
  //! @brief Set the systematic uncertainty on y. @param ySysError systematic uncertainty on y (must be >= 0)
  //! @throws MathException if ySysError<0
  void setYSysError(double ySysError) { setSyst(ySysError); }

  //! @}

  //! @name Error matrix
  //! The 2 by 2 statistical error (covariance) matrix of (x, y):
  //! element [0][0] is var(x), [1][1] is var(y), [0][1] and [1][0] are cov(x,y).
  //! A valid matrix is symmetric, has non-negative diagonal elements and satisfies
  //! cov(x,y)^2 <= var(x)*var(y) (positive semi-definite). Invalid matrices throw MathException.
  //! @{

  //!
  //! @brief Set the statistical uncertainties from the elements of the error matrix.
  //!
  //! Sets xError = sqrt(varX), yStatError = sqrt(varY) and xyCovariance = covXY. ySysError is unchanged.
  //! @param varX variance of x
  //! @param covXY covariance of x and y
  //! @param varY variance of y
  //! @throws MathException if the matrix is not a valid covariance matrix
  //!
  void setErrorMatrix(double varX, double covXY, double varY);

  //!
  //! @brief Set the statistical uncertainties from a 2 by 2 error matrix given as an array.
  //! @param matrix error matrix; matrix[0][1] and matrix[1][0] must be equal (relative tolerance 1e-6)
  //! @throws MathException if the matrix is not symmetric or not a valid covariance matrix
  //!
  void setErrorMatrix(const double matrix[2][2]);

  //!
  //! @brief Set the statistical uncertainties from a 2 by 2 error matrix given as nested vectors.
  //! @param matrix error matrix with 2 rows of 2 elements; matrix[0][1] and matrix[1][0] must be equal (relative tolerance 1e-6)
  //! @throws MathException if the matrix is not 2 by 2, not symmetric, or not a valid covariance matrix
  //!
  void setErrorMatrix(const std::vector< std::vector<double> > & matrix);

  //!
  //! @brief Set the statistical uncertainties from the two uncertainties and their correlation coefficient.
  //! @param xError uncertainty on x (>= 0)
  //! @param yStatError statistical uncertainty on y (>= 0)
  //! @param correlation correlation coefficient between x and y, in [-1, 1]
  //! @throws MathException if an uncertainty is negative or |correlation|>1
  //!
  void setErrors(double xError, double yStatError, double correlation=0.0);

  //!
  //! @brief Copy the 2 by 2 statistical error matrix into an array.
  //! @param matrix [out] error matrix
  //!
  void errorMatrix(double matrix[2][2]) const;

  //!
  //! @brief Return the 2 by 2 statistical error matrix as nested vectors.
  //! @return error matrix (2 rows of 2 elements)
  //!
  std::vector< std::vector<double> > errorMatrix() const;

  //! @}

  //! @name Arithmetic
  //! Operations between points require compatible x values (MathException otherwise); see the class description.
  //! @{

  //! @brief y = y + rhs.y. @param rhs point to add @return reference to this point
  DataPoint2D & operator+= (const DataPoint2D & rhs) { add(rhs);      return *this; }
  //! @brief y = y - rhs.y. @param rhs point to subtract @return reference to this point
  DataPoint2D & operator-= (const DataPoint2D & rhs) { subtract(rhs); return *this; }
  //! @brief y = y * rhs.y. @param rhs point to multiply by @return reference to this point
  DataPoint2D & operator*= (const DataPoint2D & rhs) { multiply(rhs); return *this; }
  //! @brief y = y / rhs.y. @param rhs point to divide by @return reference to this point @throws MathException if rhs.y==0
  DataPoint2D & operator/= (const DataPoint2D & rhs) { divide(rhs);   return *this; }
  //! @brief Add a constant to y (uncertainties unchanged). @param rhs constant @return reference to this point
  DataPoint2D & operator+= (double rhs) { shift(rhs);  return *this; }
  //! @brief Subtract a constant from y (uncertainties unchanged). @param rhs constant @return reference to this point
  DataPoint2D & operator-= (double rhs) { shift(-rhs); return *this; }
  //! @brief Multiply y, its uncertainties and the covariance by a constant (see scale()). @param rhs constant @return reference to this point
  DataPoint2D & operator*= (double rhs) { scale(rhs);  return *this; }
  //! @brief Divide y, its uncertainties and the covariance by a constant. @param rhs constant @return reference to this point
  //! @throws MathException if rhs==0
  DataPoint2D & operator/= (double rhs);

  //! @brief Multiply y by coefficient, its uncertainties by |coefficient| and the covariance by coefficient. @param coefficient scale factor
  void scale(double coefficient) override;
  //! @brief Replace y by 1/y; uncertainties become e/y^2 and the covariance -cov/y^2. @throws MathException if y==0
  void inverse() override;

  //!
  //! @brief Set this point to coefficient1*p1 + coefficient2*p2.
  //! @param p1 first point @param p2 second point (compatible x)
  //! @param coefficient1 coefficient of p1 @param coefficient2 coefficient of p2
  //!
  void setAsSumOf(const DataPoint2D & p1, const DataPoint2D & p2,
                  double coefficient1=1.0, double coefficient2=1.0);

  //!
  //! @brief Set this point to the sum of points[i]*coefficients[i].
  //! @param points points to sum (non-null, compatible x) @param coefficients coefficient of each point
  //! @throws MathException if the list is empty, the sizes differ or a pointer is null
  //!
  void setAsSumOf(const std::vector<const DataPoint2D *> & points,
                  const std::vector<double> & coefficients);

  //!
  //! @brief Set this point to (coefficient1*p1)*(coefficient2*p2).
  //! @param p1 first point @param p2 second point (compatible x)
  //! @param coefficient1 coefficient of p1 @param coefficient2 coefficient of p2
  //!
  void setAsProductOf(const DataPoint2D & p1, const DataPoint2D & p2,
                      double coefficient1=1.0, double coefficient2=1.0);

  //!
  //! @brief Set this point to the product of points[i]*coefficients[i].
  //! @param points points to multiply (non-null, compatible x) @param coefficients coefficient of each point
  //! @throws MathException if the list is empty, the sizes differ or a pointer is null
  //!
  void setAsProductOf(const std::vector<const DataPoint2D *> & points,
                      const std::vector<double> & coefficients);

  //!
  //! @brief Set this point to (coefficient1*p1)/(coefficient2*p2).
  //! @param p1 numerator point @param p2 denominator point (compatible x)
  //! @param coefficient1 coefficient of p1 @param coefficient2 coefficient of p2
  //! @throws MathException if coefficient2*p2.y==0
  //!
  void setAsRatioOf(const DataPoint2D & p1, const DataPoint2D & p2,
                    double coefficient1=1.0, double coefficient2=1.0);

  //! @}

  //! @brief Reset all values, uncertainties and the covariance to zero.
  void clear() override;

  using DataPoint::print;
  //! @brief Write the point as x +- xError, y +- yStatError (stat) +- ySysError (syst), cov to a stream. @param out output stream
  void print(std::ostream & out) const override;

  //! @name Binary encoding (used by DataPointsVector::encodeBinary())
  //! A mask byte tells which of the six numbers (x, xError, y, yStatError, ySysError, xyCovariance) are non-zero;
  //! only those are written (float32). Restored exactly.
  //! @{
  //! @brief Write this point. @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Read this point. @param r reader @throws Exception if the data is invalid
  void decodeBinary(BinaryReader & r);
  //! @}


  //! @name Binary files (independent of ROOT)
  //! Saved as "CAPP" records in the CAP binary format (see BinaryIO.hpp): compact, portable, checksummed, and restored
  //! exactly. Records of other classes in the same file are skipped.
  //! @{

  //! @brief Tag of DataPoint2D records in binary files. @return "CAPP"
  static const char * binaryTag() { return "CAPP"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 1; }
  //! @brief Name stored in binary records. @return ""
  String binaryName() const { return ""; }
  //! @brief Rebuild this object from a binary payload (used by readBinary()). @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write this data point as one binary record to a stream (open in binary mode). @param out stream @throws FileException if writing fails
  void writeBinary(std::ostream & out) const;
  //! @brief Read the next DataPoint2D record of a stream into this object, skipping records of other classes.
  //! @param in stream @return false at the end of the stream (unchanged) @throws FileException if the data is invalid (unchanged)
  bool readBinary(std::istream & in);
  //! @brief Save this data point to a binary file. @param fileName file name @param append add at the end of the file instead of replacing it
  //! @throws FileException if the file cannot be written
  void saveBinary(const String & fileName, bool append=false) const;
  //! @brief Load from a binary file the data point at position index among the objects of this class (0 = the first one).
  //! @param fileName file name @param index position
  //! @throws FileException if the file cannot be read, is corrupted, or holds fewer objects (this one is then unchanged)
  void loadBinary(const String & fileName, unsigned int index=0);
  //! @brief Save several objects to one binary file (replacing it). @param fileName file name @param objects objects to save, in order
  //! @throws FileException if the file cannot be written
  static void saveBinary(const String & fileName, const std::vector<DataPoint2D> & objects);
  //! @brief Load all the DataPoint2D objects of a binary file, in order. @param fileName file name @return objects
  //! @throws FileException if the file cannot be read or is corrupted
  static std::vector<DataPoint2D> loadAllBinary(const String & fileName);

  //! @}

  protected:

  //! @brief Extends DataPoint::combine() with first-order propagation of the x-y covariance (mean of d1*cov1 and d2*cov2).
  void combine(const DataPoint & p1, const DataPoint & p2, double value, double d1, double d2) override;


  };

  } // namespace MATH
} // namespace CAP

#endif
