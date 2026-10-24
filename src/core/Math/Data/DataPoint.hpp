/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP_MATH_DataPoint
#define CAP_MATH_DataPoint
#include <cmath>
#include <iostream>
#include <ostream>
#include <type_traits>
#include <vector>
#include "String.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Abstract data point: an arbitrary number of independent variables and one dependent value.
  //!
  //!  - rank()            : number of independent variables (1 for DataPoint2D, 2 for DataPoint3D, n>2 for DataPointND)
  //!  - x(i), xError(i)   : value and uncertainty of independent variable i, 0 <= i < rank()
  //!  - value()           : dependent value
  //!  - stat(), syst()    : statistical and systematic uncertainties on the dependent value (members _stat and _syst)
  //!
  //! Storage of the independent variables is left to the derived classes. Their uncertainties are, for now,
  //! treated as mutually independent: no covariance between independent variables is stored.
  //!
  //! <b>Arithmetic.</b> add(), subtract(), multiply() and divide() act on the dependent value of
  //! two points of the same rank whose independent variables are compatible (see compatibleX()). The result takes the
  //! mean of each independent variable and the mean of its uncertainty. Statistical and systematic uncertainties are
  //! propagated separately, to first order, assuming the two points are independent. Derived classes expose these
  //! as operators (+=, -=, *=, /=); the binary operators (+, -, *, /, unary -) are provided for every derived class
  //! by the templates at the end of this file.
  //!
  class DataPoint
  {
  protected:

  float _value;  //!< dependent value
  float _stat;   //!< statistical uncertainty on the dependent value
  float _syst;   //!< systematic uncertainty on the dependent value

  public:

  //! @name Construction and assignment
  //! @{

  //! @brief Default constructor: value and uncertainties are zero.
  DataPoint();
  //! @brief Constructor. @param value dependent value @param stat statistical uncertainty @param syst systematic uncertainty
  DataPoint(double value, double stat=0.0, double syst=0.0);
  //! @brief Copy constructor. @param source point to copy
  DataPoint(const DataPoint & source);
  //! @brief Destructor.
  virtual ~DataPoint() {}
  //! @brief Polymorphic copy. @return new point of the same class (owned by the caller)
  virtual DataPoint * clone() const = 0;

  //! @}

  //! @name Independent variables
  //! Index arguments must satisfy 0 <= index < rank(); IndexOutOfBoundException otherwise.
  //! @{

  //! @brief Number of independent variables. @return rank
  virtual unsigned int rank() const = 0;
  //! @brief Value of independent variable index. @param index variable index @return value
  virtual float x(unsigned int index) const = 0;
  //! @brief Uncertainty on independent variable index. @param index variable index @return uncertainty
  virtual float xError(unsigned int index) const = 0;
  //! @brief Set independent variable index. @param index variable index @param value new value
  virtual void setX(unsigned int index, double value) = 0;
  //! @brief Set the uncertainty on independent variable index. @param index variable index @param error uncertainty (>= 0)
  //! @throws MathException if error<0
  virtual void setXError(unsigned int index, double error) = 0;
  //! @brief All independent variables. @return vector of rank() values
  std::vector<double> xValues() const;
  //! @brief Uncertainties on all independent variables. @return vector of rank() uncertainties
  std::vector<double> xErrors() const;

  //! @}

  //! @name Dependent value
  //! @{

  //! @brief Dependent value. @return value
  float value() const { return _value; }
  //! @brief Statistical uncertainty on the dependent value. @return stat
  float stat()  const { return _stat; }
  //! @brief Systematic uncertainty on the dependent value. @return syst
  float syst()  const { return _syst; }
  //! @brief Total uncertainty on the dependent value, stat and syst added in quadrature. @return sqrt(stat^2+syst^2)
  float error() const { return std::sqrt(double(_stat)*_stat + double(_syst)*_syst); }
  //! @brief Set the dependent value (uncertainties unchanged). @param value new value
  void setValue(double value) { _value = value; }
  //! @brief Set the dependent value and its uncertainties. @param value new value @param stat statistical uncertainty (>= 0)
  //! @param syst systematic uncertainty (>= 0) @throws MathException if an uncertainty is negative
  void setValue(double value, double stat, double syst=0.0);
  //! @brief Set the statistical uncertainty. @param stat uncertainty (>= 0) @throws MathException if stat<0
  void setStat(double stat);
  //! @brief Set the systematic uncertainty. @param syst uncertainty (>= 0) @throws MathException if syst<0
  void setSyst(double syst);

  //! @}

  //! @name Comparison
  //! @{

  //!
  //! @brief Check whether the independent variables of two points are compatible.
  //!
  //! Compatible means same rank and, for every i, |x1(i)-x2(i)| <= nSigma*sqrt(xError1(i)^2 + xError2(i)^2),
  //! with a small relative tolerance (1e-6) so that equal values computed with rounding are accepted.
  //! @param other other point @param nSigma number of standard deviations allowed @return true if compatible
  //!
  bool compatibleX(const DataPoint & other, double nSigma=2.0) const;
  //! @brief Statistical equality: compatible independent variables and dependent values within two combined total
  //! uncertainties. @param rhs point to compare with @return true if the points agree
  bool operator==(const DataPoint & rhs) const;
  //! @brief Negation of operator==. @param rhs point to compare with @return true if the points disagree
  bool operator!=(const DataPoint & rhs) const { return !operator==(rhs); }

  //! @}

  //! @name Operations on the dependent value
  //! @{

  //! @brief Add a constant to the dependent value (uncertainties unchanged). @param constant value added
  void shift(double constant) { _value += constant; }
  //! @brief Multiply the dependent value by coefficient and its uncertainties by |coefficient|. @param coefficient scale factor
  virtual void scale(double coefficient);
  //! @brief Replace the value v by 1/v; uncertainties become e/v^2. @throws MathException if the value is zero
  virtual void inverse();
  //! @brief value = value + rhs.value, with uncertainties propagated (see the class description).
  //! @param rhs point of the same rank with compatible independent variables @throws MathException otherwise
  void add(const DataPoint & rhs);
  //! @brief value = value - rhs.value. @param rhs point of the same rank with compatible independent variables
  //! @throws MathException otherwise
  void subtract(const DataPoint & rhs);
  //! @brief value = value * rhs.value. @param rhs point of the same rank with compatible independent variables
  //! @throws MathException otherwise
  void multiply(const DataPoint & rhs);
  //! @brief value = value / rhs.value. @param rhs point of the same rank with compatible independent variables
  //! @throws MathException otherwise, or if rhs.value==0
  void divide(const DataPoint & rhs);

  //! @}

  //! @brief Reset the dependent value, its uncertainties and the independent variables to zero (the rank is kept).
  virtual void clear();
  //! @brief Same as clear().
  void reset() { clear(); }
  //! @brief Write the point to a stream (no end of line). @param out output stream
  virtual void print(std::ostream & out) const;
  //! @brief Print the point to std::cout, followed by an end of line.
  void print() const;

  protected:

  //! @brief Assignment of the dependent value and its uncertainties, for use by derived classes' assignment operators.
  //! Protected so that assigning through a DataPoint reference (which would copy only part of a point) does not compile.
  //! @param rhs point to copy @return reference to this point
  DataPoint & operator=(const DataPoint & rhs);

  //! @brief Throw unless rhs has the same rank and compatible independent variables. @param rhs other point @param source caller name
  void requireCompatible(const DataPoint & rhs, const char * source) const;
  //! @brief Throw unless index < rank(). @param index variable index @param source caller name
  void checkIndex(unsigned int index, const char * source) const;

  //!
  //! @brief Set this point to the result of a two-operand operation f(v1,v2).
  //!
  //! Each independent variable (and its uncertainty) is set to the mean of the two operands. Uncertainties are
  //! propagated to first order: stat = sqrt((d1*stat1)^2 + (d2*stat2)^2), and likewise for syst.
  //! p1 or p2 may be this point. Derived classes holding more data (e.g., a covariance) override and extend it.
  //! @param p1 first operand @param p2 second operand (same rank) @param value f(v1,v2)
  //! @param d1 partial derivative df/dv1 @param d2 partial derivative df/dv2
  //!
  virtual void combine(const DataPoint & p1, const DataPoint & p2, double value, double d1, double d2);


  ClassDef(DataPoint,1)
  };

  //! @relates DataPoint
  //! @brief Write a point to a stream (calls print(out)). @param out output stream @param p point @return the output stream
  std::ostream & operator<<(std::ostream & out, const DataPoint & p);

  //! @name Binary operators for all concrete data point classes
  //! P is any class derived from DataPoint that provides a copy constructor and the compound operators.
  //! Operations between two points act on the dependent value; operations with a number act on the dependent value only.
  //! @{

  //! @brief Enables a template for concrete classes derived from DataPoint only.
  template <class P>
  using EnableIfDataPoint = typename std::enable_if<std::is_base_of<DataPoint,P>::value && !std::is_same<DataPoint,P>::value, P>::type;

  template <class P> EnableIfDataPoint<P> operator+(const P & left, const P & right) { P r(left); r += right; return r; }
  template <class P> EnableIfDataPoint<P> operator-(const P & left, const P & right) { P r(left); r -= right; return r; }
  template <class P> EnableIfDataPoint<P> operator*(const P & left, const P & right) { P r(left); r *= right; return r; }
  template <class P> EnableIfDataPoint<P> operator/(const P & left, const P & right) { P r(left); r /= right; return r; }
  template <class P> EnableIfDataPoint<P> operator+(const P & left, double right)    { P r(left);  r += right; return r; }
  template <class P> EnableIfDataPoint<P> operator+(double left, const P & right)    { P r(right); r += left;  return r; }
  template <class P> EnableIfDataPoint<P> operator-(const P & left, double right)    { P r(left);  r -= right; return r; }
  template <class P> EnableIfDataPoint<P> operator-(double left, const P & right)    { P r(right); r.scale(-1.0); r += left; return r; }
  template <class P> EnableIfDataPoint<P> operator*(const P & left, double right)    { P r(left);  r.scale(right); return r; }
  template <class P> EnableIfDataPoint<P> operator*(double left, const P & right)    { P r(right); r.scale(left);  return r; }
  template <class P> EnableIfDataPoint<P> operator/(const P & left, double right)    { P r(left);  r /= right; return r; }
  template <class P> EnableIfDataPoint<P> operator/(double left, const P & right)    { P r(right); r.inverse(); r.scale(left); return r; }
  template <class P> EnableIfDataPoint<P> operator+(const P & right)                 { return P(right); }
  template <class P> EnableIfDataPoint<P> operator-(const P & right)                 { P r(right); r.scale(-1.0); return r; }

  //! @}

  } // namespace MATH
} // namespace CAP

#endif
