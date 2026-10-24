#ifndef CAP_MATH_ParameterizedFunction
#define CAP_MATH_ParameterizedFunction
#include <vector>
#include "String.hpp"
#include "Parameter.hpp"
#include "Function.hpp"
#include "VectorND.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Abstract base class for functions of rank() real variables with a list of parameters (see Parameter), f(x; p).
  //!
  //! The rank (number of independent variables) is handled by the base class Function; it is 1 by default.
  //! The function can be evaluated with evaluateAt(x), evaluateAt(x1,x2), evaluateAt(x1,x2,x3) or
  //! evaluateAt(std::vector<double>); each throws MathException if the number of arguments differs from the rank.
  //!
  //! Derived classes implement either the evaluateAt() version matching their rank (rank 1, 2 or 3),
  //! or the vector version (any rank): the base class forwards each version to the other. A derived class
  //! that overrides some versions should add <tt>using ParameterizedFunction::evaluateAt;</tt> so that the
  //! others remain visible.
  //!
  //! For functions of any rank, gradientAt() returns the gradient and integralInRange() the integral over a
  //! rectangular box, computed numerically by default (derived classes may override them with analytical expressions).
  //!
  //! For functions of rank 1, f(x) is the same as evaluateAt(x), and derivativeAt(x) returns f'(x), integralInRange(low,high) the integral of f from low to high,
  //! and expectationValue(low,high) the mean of x weighted by f over that range: by default
  //! these are computed numerically, and derived classes override them with analytical
  //! expressions whenever these are available.
  //!
  //! Example:
  //! @code
  //!   class Parabola : public CAP::MATH::ParameterizedFunction
  //!   {
  //!   public:
  //!     double evaluateAt(double x) const override { return x*x; }
  //!     using ParameterizedFunction::evaluateAt;
  //!   };
  //!
  //!   Parabola f;
  //!   double y1 = f.evaluateAt(3.0);   // 9
  //!   double y2 = f(3.0);              // 9
  //!   double d  = f.derivativeAt(3.0); // 6 (numerical, since Parabola does not override it)
  //!   unsigned int r = f.rank();       // 1
  //!   double y3 = f.evaluateAt(std::vector<double>{3.0});  // 9
  //!   f.evaluateAt(1.0, 2.0);          // throws MathException: rank is 1
  //! @endcode
  //!
  class ParameterizedFunction : public Function
  {
  public:

  //! @brief Default constructor: no parameters. @param rank number of independent variables (>= 1)
  //! @throws MathException if rank is 0
  explicit ParameterizedFunction(unsigned int rank=1);
  //! @brief Full constructor. @param parameters parameters of the function @param rank number of independent variables (>= 1)
  //! @throws MathException if rank is 0
  ParameterizedFunction(const std::vector<Parameter> & parameters, unsigned int rank=1);
  //! @brief Copy constructor. @param source function to copy
  ParameterizedFunction(const ParameterizedFunction & source);
  //! @brief Assignment operator. @param rhs function to copy @return reference to this function
  ParameterizedFunction & operator=(const ParameterizedFunction & rhs);
  //! @brief Destructor (pure virtual: the class is abstract).
  virtual ~ParameterizedFunction() = 0;
  //!
  //! @brief Copy of the function, of its own class (e.g. to keep the result of a fit, see FitRecord). The functions of
  //! CAP implement it; a class of a user should too, as @c MyFunction * clone() const override { return new MyFunction(*this); }.
  //! @return new function (the caller owns it)
  //! @throws MathException if the class does not implement it
  //!
  virtual ParameterizedFunction * clone() const;

  //! @brief Append a parameter. @param parameter parameter to add
  void addParameter(const Parameter & parameter);

  //! @brief All parameters of the function. @return parameters
  const std::vector<Parameter> & parameters() const { return _parameters; }
  //! @brief Number of parameters. @return number of parameters
  unsigned int nParameters() const { return _parameters.size(); }
  //!
  //! @brief Parameter at the given index.
  //! @param index index of the parameter
  //! @return parameter at index
  //! @throws MathException if index is out of range
  //!
  const Parameter & parameter(unsigned int index) const;

  //! @name Setting the parameters (e.g. by a fit)
  //! @{

  //! @brief Set the value of a parameter. @param index index of the parameter @param value new value
  //! @throws MathException if index is out of range; MathDomainException if the value is outside its bounds
  void setParameterValue(unsigned int index, double value);
  //! @brief Set the values of all the parameters. @param values values, one per parameter
  //! @throws MathException if the number of values differs from nParameters(); MathDomainException as setParameterValue()
  void setParameterValues(const std::vector<double> & values);
  //! @brief Values of all the parameters. @return values
  std::vector<double> parameterValues() const;
  //! @brief Indices of the parameters that are not Fixed. @return indices
  std::vector<unsigned int> freeParameters() const;
  //!
  //! @brief Fix a parameter at its current value, or release it (its type then follows its bounds: Free, Low, High or
  //! LowHigh). @param index index of the parameter @param fixed fix it, or release it
  //! @throws MathException if index is out of range
  //!
  void fixParameter(unsigned int index, bool fixed=true);
  //!
  //! @brief Set the bounds of a parameter (its type becomes Low, High or LowHigh unless it is Fixed); its value is
  //! moved within them if needed. @param index index of the parameter @param minimum lower bound @param maximum upper
  //! bound @throws MathException if index is out of range or minimum > maximum
  //!
  void setParameterLimits(unsigned int index, double minimum, double maximum);
  //! @brief Replace a parameter (name, value, type and bounds; e.g. when a function is read from a file).
  //! @param index index of the parameter @param parameter new parameter @throws MathException if index is out of range
  void setParameter(unsigned int index, const Parameter & parameter);
  //! @}

  //! @name Fits
  //! @{

  //!
  //! @brief Adapt the function to the range of x of a fit, before it is fitted there (rank 1). By default nothing;
  //! functions whose variable is mapped onto a standard interval (e.g. OrthogonalSeries) set their mapping
  //! parameters, if those are fixed. Called by Fitter::estimate() and by the Fit window of CapGui.
  //! @param low lower end of the range @param high upper end of the range
  //!
  virtual void adaptToRange(double low, double high) { (void)low; (void)high; }
  //!
  //! @brief Adapt the function to a box of its variables (any rank): by default, for rank 1, adaptToRange(low[0],
  //! high[0]); functions of several variables mapped onto a standard box (e.g. Polynomial2D) set their mapping.
  //! @param low lower ends, one per variable @param high upper ends
  //!
  virtual void adaptToRange(const std::vector<double> & low, const std::vector<double> & high)
  { if (low.size() == 1 && high.size() == 1) adaptToRange(low[0], high[0]); }
  //!
  //! @brief Name under which the function itself was registered in FunctionRegistry (e.g. a composite registered by
  //! FunctionRegistry::registerComposite()), or empty when its class is what identifies it. @return name
  //!
  virtual String registryName() const { return String(); }
  //! @}

  //!
  //! @brief Value of a function of rank 1. Implemented by derived classes of rank 1; by default, forwards to the vector version.
  //! @param x value of the independent variable
  //! @return f(x)
  //! @throws MathException if the rank is not 1, or if the derived class implements neither this nor the vector version
  //!
  double evaluateAt(double x) const override;

  //!
  //! @brief Value of a function of rank 2. Implemented by derived classes of rank 2; by default, forwards to the vector version.
  //! @param x1 first variable @param x2 second variable
  //! @return f(x1, x2)
  //! @throws MathException if the rank is not 2, or if the derived class implements neither this nor the vector version
  //!
  double evaluateAt(double x1, double x2) const override;

  //!
  //! @brief Value of a function of rank 3. Implemented by derived classes of rank 3; by default, forwards to the vector version.
  //! @param x1 first variable @param x2 second variable @param x3 third variable
  //! @return f(x1, x2, x3)
  //! @throws MathException if the rank is not 3, or if the derived class implements neither this nor the vector version
  //!
  double evaluateAt(double x1, double x2, double x3) const override;

  //!
  //! @brief Value of the function at the point x, for any rank.
  //!
  //! Implemented by derived classes of rank > 3. By default, for rank 1, 2 or 3, forwards to the
  //! evaluateAt() version with that number of arguments.
  //! @param x values of the rank() independent variables
  //! @return f(x)
  //! @throws MathException if x.size() differs from the rank, or if the derived class implements no evaluateAt() version
  //!
  double evaluateAt(const std::vector<double> & x) const override;

  //!
  //! @brief First derivative of the function at x.
  //!
  //! The default implementation is numerical: a five-point central difference,
  //! f'(x) = [f(x-2h) - 8 f(x-h) + 8 f(x+h) - f(x+2h)] / (12 h), with step
  //! h = 1e-3 max(1, |x|), accurate to order h^4 for smooth functions (typically 1e-10 relative).
  //! Derived classes should override it with the analytical derivative when one is known.
  //! @param x value of the independent variable
  //! @return f'(x)
  //! @throws MathException (default implementation) if the rank is not 1
  //!
  virtual double derivativeAt(double x) const;

  //!
  //! @brief Gradient of the function at x, for any rank.
  //!
  //! The default implementation uses derivativeAt() for rank 1 (analytical when the derived class provides it),
  //! and otherwise a five-point central difference along each axis, with step h = 1e-3 max(1, |x_i|).
  //! Derived classes may override it with the analytical gradient.
  //! @param x values of the rank() independent variables
  //! @return vector of the rank() partial derivatives
  //! @throws MathException if x.size() differs from the rank
  //!
  virtual VectorND gradientAt(const std::vector<double> & x) const;
  //! @brief Gradient of a function of rank 1 (same as gradientAt({x})). @param x variable @return gradient @throws MathException if the rank is not 1
  VectorND gradientAt(double x) const;
  //! @brief Gradient of a function of rank 2. @param x1 first variable @param x2 second variable @return gradient @throws MathException if the rank is not 2
  VectorND gradientAt(double x1, double x2) const;
  //! @brief Gradient of a function of rank 3. @param x1 first variable @param x2 second variable @param x3 third variable @return gradient @throws MathException if the rank is not 3
  VectorND gradientAt(double x1, double x2, double x3) const;

  //!
  //! @brief Integral of a function of rank 2 over the rectangle [x1Low, x1High] x [x2Low, x2High].
  //!
  //! By default, forwards to the vector version. Reversed bounds change the sign of the result.
  //! @param x1Low lower bound of x1 @param x1High upper bound of x1
  //! @param x2Low lower bound of x2 @param x2High upper bound of x2
  //! @return integral of f dx1 dx2
  //! @throws MathException if the rank is not 2, or (default implementation) if a bound is not finite
  //!
  virtual double integralInRange(double x1Low, double x1High, double x2Low, double x2High) const;

  //!
  //! @brief Integral of a function of rank 3 over the box [x1Low, x1High] x [x2Low, x2High] x [x3Low, x3High].
  //!
  //! By default, forwards to the vector version. Reversed bounds change the sign of the result.
  //! @param x1Low lower bound of x1 @param x1High upper bound of x1
  //! @param x2Low lower bound of x2 @param x2High upper bound of x2
  //! @param x3Low lower bound of x3 @param x3High upper bound of x3
  //! @return integral of f dx1 dx2 dx3
  //! @throws MathException if the rank is not 3, or (default implementation) if a bound is not finite
  //!
  virtual double integralInRange(double x1Low, double x1High, double x2Low, double x2High, double x3Low, double x3High) const;

  //!
  //! @brief Integral of the function over the box [low[0], high[0]] x ... x [low[n-1], high[n-1]], for any rank n.
  //!
  //! For rank 1, forwards to integralInRange(low, high). Otherwise the default implementation is numerical:
  //! iterated adaptive Simpson integration, one axis at a time, with a relative tolerance of about 1e-8.
  //! Its cost grows quickly with the rank (typically 1e2 to 1e3 evaluations per axis, multiplied across axes),
  //! so derived classes should override it with an analytical expression whenever possible.
  //! Reversed bounds change the sign of the result.
  //! @param low lower bounds, one per variable @param high upper bounds, one per variable
  //! @return integral of f over the box
  //! @throws MathException if low.size() or high.size() differs from the rank, or (default implementation) if a bound is not finite
  //!
  virtual double integralInRange(const std::vector<double> & low, const std::vector<double> & high) const;

  //!
  //! @brief Integral of the function from low to high.
  //!
  //! If low > high the result is the negative of the integral from high to low; it is zero if low == high.
  //! The default implementation is numerical: adaptive Simpson integration with a relative
  //! tolerance of about 1e-10. It requires finite bounds and a function without singularities
  //! in [low, high]. Derived classes should override it with the analytical integral when one is known.
  //! @param low lower bound of integration
  //! @param high upper bound of integration
  //! @return integral of f(x) dx from low to high
  //! @throws MathException (default implementation) if a bound is not finite
  //! @throws MathException (default implementation) if the rank is not 1
  //!
  virtual double integralInRange(double low, double high) const;

  //!
  //! @brief Expectation value of x over [low, high], using the function as a (not necessarily normalized) weight.
  //!
  //! E[x] = integral of x f(x) dx / integral of f(x) dx, both from low to high. The result is
  //! the same whatever the order of the bounds. It is the mean of x for a distribution f
  //! restricted to [low, high], and is meaningful when f does not change sign in the range.
  //! The default implementation is numerical (adaptive Simpson, see integralInRange()).
  //! Derived classes should override it with the analytical expression when one is known.
  //! @param low lower bound of the range
  //! @param high upper bound of the range
  //! @return expectation value of x
  //! @throws MathException if the integral of f over the range is zero, or (default implementation) if a bound is not finite
  //! @throws MathException (default implementation) if the rank is not 1
  //!
  virtual double expectationValue(double low, double high) const;

  //!
  //! @brief Expectation value of x^power over [low, high], using the function as a (not necessarily normalized) weight.
  //!
  //! E[x^n] = integral of x^n f(x) dx / integral of f(x) dx, both from low to high. power = 1 gives
  //! expectationValue(low, high) and power = 0 gives 1. The result does not depend on the order of the bounds.
  //! Negative powers are allowed only if 0 is outside [low, high].
  //! The default implementation is numerical (adaptive Simpson, see integralInRange()).
  //! Derived classes should override it with the analytical expression when one is known.
  //! @param low lower bound of the range
  //! @param high upper bound of the range
  //! @param power power n of x
  //! @return expectation value of x^power
  //! @throws MathException if the integral of f over the range is zero, if power < 0 and 0 is in the range,
  //! or (default implementation) if a bound is not finite
  //! @throws MathException (default implementation) if the rank is not 1
  //!
  virtual double expectationValue(double low, double high, int power) const;

  protected:

  std::vector<Parameter> _parameters; //!< parameters of the function

  //! @brief Value of the parameter at index, without range check (for use by derived classes). @param index index of the parameter @return value of the parameter
  double parameterValue(unsigned int index) const { return _parameters[index].value(); }

  //!
  //! @brief Throw if power < 0 and the range [low, high] (in either order) contains 0, where x^power is singular.
  //! @param low lower bound @param high upper bound @param power power of x @param source name of the calling function
  //! @throws MathException if x^power is singular in the range
  //!
  static void checkPowerRange(double low, double high, int power, const char * source);

  public:

  //!
  //! @brief Value of the function at x (same as evaluateAt()).
  //! @param x value of the independent variable
  //! @return f(x)
  //!
  double operator()(double x) const { return evaluateAt(x); }

  };

  } // namespace MATH
} // namespace CAP

#endif
