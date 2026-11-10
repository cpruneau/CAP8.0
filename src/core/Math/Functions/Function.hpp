#ifndef CAP_MATH_Function
#define CAP_MATH_Function
#include <vector>
#include "String.hpp"
#include "Object.hpp"
#include "FitRecord.hpp"
#include "VectorND.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Abstract base class for continuous functions of n real variables to one real value, f(x1, ..., xn), i.e., scalar fields.
  //!
  //! The number n of independent variables is the rank of the function. evaluateAt() can be called with
  //! one, two or three arguments, or with a vector of arguments; each throws MathException if the number
  //! of arguments differs from the rank.
  //!
  //! Derived classes must implement evaluateAt(const std::vector<double> &). The one, two and three argument
  //! versions check the rank and, by default, forward to the vector version; derived classes may override them
  //! for efficiency, and should then call checkRank() first.
  //!
  //! The classes derived from Function, and the classes they depend on (the source of the figure is
  //! docs/paper/figures/FunctionClasses.tex; see also ParameterizedFunction, FunctionRegistry and Fitter):
  //!
  //! @image html FunctionClasses.png "Classes of the functions (core/Math/Functions)" width=100%
  //! @image latex FunctionClasses.pdf "Classes of the functions (core/Math/Functions)" width=\textwidth
  //!
  //! Open triangles point to the base class; abstract classes are in italics. A box with a header in bold lists the
  //! classes derived from it; a box with a header in italics groups classes derived from the class it points to.
  //! Dashed lines are dependencies: the diamond marks the owner of the objects held.
  //!
  //! Example:
  //! @code
  //!   class Paraboloid : public CAP::MATH::Function
  //!   {
  //!   public:
  //!     Paraboloid() : Function(2) {}
  //!     double evaluateAt(const std::vector<double> & x) const override
  //!     {
  //!       checkRank(x.size(),__FUNCTION__);
  //!       return x[0]*x[0] + x[1]*x[1];
  //!     }
  //!     using Function::evaluateAt;
  //!   };
  //!
  //!   Paraboloid f;
  //!   double z = f.evaluateAt(1.0, 2.0);   // 5
  //!   f.evaluateAt(1.0);                   // throws MathException: rank is 2
  //! @endcode
  //!
  class Function : public Object
  {
  public:

  //!
  //! @brief Default constructor.
  //! @param rank number of independent variables (>= 1)
  //! @throws MathException if rank is 0
  //!
  Function(unsigned int rank=1);
  //! @brief Constructor with a name. @param name object name @param title object title @param className class name
  //! @param rank number of independent variables (>= 1) @throws MathException if rank==0
  Function(const String & name, const String & title, const String & className, unsigned int rank=1);
  //! @brief Copy constructor. @param source function to copy
  Function(const Function & source);
  //! @brief Assignment operator. @param rhs function to copy @return reference to this function
  Function & operator=(const Function & rhs);
  //! @brief Destructor.
  virtual ~Function() {}

  //! @brief Number of independent variables. @return rank
  unsigned int rank() const { return _rank; }
  //!
  //! @brief Set the number of independent variables.
  //! @param rank new rank (>= 1)
  //! @throws MathException if rank is 0
  //!
  virtual void setRank(unsigned int rank);

  //!
  //! @brief Value of a function of rank 1.
  //! @param x value of the independent variable
  //! @return f(x)
  //! @throws MathException if the rank is not 1
  //!
  virtual double evaluateAt(double x) const;

  //!
  //! @brief Value of a function of rank 2.
  //! @param x1 first variable @param x2 second variable
  //! @return f(x1, x2)
  //! @throws MathException if the rank is not 2
  //!
  virtual double evaluateAt(double x1, double x2) const;

  //!
  //! @brief Value of a function of rank 3.
  //! @param x1 first variable @param x2 second variable @param x3 third variable
  //! @return f(x1, x2, x3)
  //! @throws MathException if the rank is not 3
  //!
  virtual double evaluateAt(double x1, double x2, double x3) const;

  //!
  //! @brief Value of the function at the point x. Must be implemented by derived classes.
  //! @param x values of the rank() independent variables
  //! @return f(x)
  //! @throws MathException (implementations should call checkRank()) if x.size() differs from the rank
  //!
  virtual double evaluateAt(const std::vector<double> & x) const = 0;

  //! @brief Value of a function of rank 1 (same as evaluateAt()). @param x variable @return f(x)
  double operator()(double x) const { return evaluateAt(x); }

  //! @name Derivatives
  //! The defaults are numerical (Differentiation: the derivative of the polynomial of degree 4 through five points
  //! around x, step 1e-3 max(1, |x|), error of order h^4); derived classes override them with analytical expressions
  //! when they are known, and then say so with hasAnalyticalDerivative(). derivativeFunction() gives the derivative
  //! as a function for the classes closed under differentiation (polynomials, exponentials, splines, series).
  //! Derivative (a function) and Differentiation compute derivatives of any order, analytical when possible.
  //! @{
  //!
  //! @brief First derivative of a function of rank 1.
  //! @param x value of the variable @return f'(x)
  //! @throws MathException if the rank is not 1
  //!
  virtual double derivativeAt(double x) const;
  //!
  //! @brief Gradient at x, for any rank: derivativeAt() for rank 1; by default, numerical partial derivatives otherwise.
  //! @param x values of the rank() variables @return the rank() partial derivatives
  //! @throws MathException if x.size() differs from the rank
  //!
  virtual VectorND gradientAt(const std::vector<double> & x) const;
  //! @brief Gradient of a function of rank 1. @param x variable @return gradient @throws MathException if the rank is not 1
  VectorND gradientAt(double x) const;
  //! @brief Gradient of a function of rank 2. @param x1 first variable @param x2 second variable @return gradient @throws MathException if the rank is not 2
  VectorND gradientAt(double x1, double x2) const;
  //! @brief Gradient of a function of rank 3. @param x1 first variable @param x2 second variable @param x3 third variable @return gradient @throws MathException if the rank is not 3
  VectorND gradientAt(double x1, double x2, double x3) const;
  //! @brief Whether derivativeAt() (and gradientAt()) are analytical (false by default). @return true if so
  virtual bool hasAnalyticalDerivative() const { return false; }
  //!
  //! @brief Derivative along an axis as a new function, for the classes whose derivatives are of the same kind
  //! (polynomials, exponentials, orthogonal series, Fourier decompositions, piecewise polynomials).
  //! @param axis axis (0 for rank 1) @return new function (owned by the caller), or null (the default) if the class
  //! does not provide it
  //!
  virtual Function * derivativeFunction(unsigned int axis=0) const { (void) axis; return nullptr; }
  //! @}

  //! @name Integrals
  //! The defaults are numerical: adaptive Simpson integration (SimpsonIntegrator) with a relative tolerance of 1e-10
  //! for one variable, and one variable at a time with 1e-8 for a box. They require finite bounds and a function
  //! without singularities. Derived classes override them with analytical expressions when they are known. Reversed
  //! bounds change the sign. Integration and the integrators (Integral folder) offer other methods.
  //! @{
  //!
  //! @brief Integral of a function of rank 1 from low to high.
  //! @param low lower bound @param high upper bound @return integral
  //! @throws MathException if the rank is not 1, or (default) if a bound is not finite
  //!
  virtual double integralInRange(double low, double high) const;
  //!
  //! @brief Integral of a function of rank 2 over [x1Low, x1High] x [x2Low, x2High] (by default, the vector version).
  //! @param x1Low lower bound of x1 @param x1High upper bound of x1 @param x2Low lower bound of x2 @param x2High upper bound of x2
  //! @return integral @throws MathException if the rank is not 2
  //!
  virtual double integralInRange(double x1Low, double x1High, double x2Low, double x2High) const;
  //!
  //! @brief Integral of a function of rank 3 over a box (by default, the vector version).
  //! @param x1Low lower bound of x1 @param x1High upper bound of x1 @param x2Low lower bound of x2
  //! @param x2High upper bound of x2 @param x3Low lower bound of x3 @param x3High upper bound of x3
  //! @return integral @throws MathException if the rank is not 3
  //!
  virtual double integralInRange(double x1Low, double x1High, double x2Low, double x2High, double x3Low, double x3High) const;
  //!
  //! @brief Integral over the box [low[0], high[0]] x ... x [low[n-1], high[n-1]], for any rank n (rank 1: the
  //! version with two bounds). @param low lower bounds @param high upper bounds @return integral
  //! @throws MathException if the sizes differ from the rank, or (default) if a bound is not finite
  //!
  virtual double integralInRange(const std::vector<double> & low, const std::vector<double> & high) const;
  //!
  //! @brief Expectation value of x over [low, high], the function as a (not necessarily normalized) weight:
  //! integral of x f(x) / integral of f(x). @param low lower bound @param high upper bound @return E[x]
  //! @throws MathException if the rank is not 1 or the integral of f is zero
  //!
  virtual double expectationValue(double low, double high) const;
  //!
  //! @brief Expectation value of x^power over [low, high] (power 0 gives 1; negative powers only if 0 is outside the
  //! range). @param low lower bound @param high upper bound @param power power @return E[x^power]
  //! @throws MathException if the rank is not 1, the integral of f is zero, or x^power is singular in the range
  //!
  virtual double expectationValue(double low, double high, int power) const;
  //! @}

  //! @name Fits attached (see FitRecord, Fitter::record())
  //! @{

  //! @brief Attach a copy of a fit. @param record record
  void addFit(const FitRecord & record) { _fits.add(record); }
  //! @brief Attach a fit (shared). @param record record
  void addFit(const std::shared_ptr<const FitRecord> & record) { _fits.add(record); }
  //! @brief Number of fits attached. @return number
  unsigned int nFits() const { return _fits.size(); }
  //! @brief Fit k. @param k index @return record @throws MathException if k is out of range
  const FitRecord & fit(unsigned int k) const { return _fits.at(k); }
  //! @brief Fit k, shared. @param k index @return record @throws MathException if k is out of range
  std::shared_ptr<const FitRecord> fitPointer(unsigned int k) const { return _fits.pointer(k); }
  //! @brief Detach fit k. @param k index @throws MathException if k is out of range
  void removeFit(unsigned int k) { _fits.remove(k); }
  //! @brief Detach all the fits.
  void clearFits() { _fits.clear(); }
  //! @}

  protected:

  //!
  //! @brief Throw if the number of arguments differs from the rank.
  //! @param nArguments number of arguments given @param source name of the calling function
  //! @throws MathException if nArguments != rank()
  //!
  void checkRank(unsigned int nArguments, const char * source) const;
  //!
  //! @brief Throw if power < 0 and the range [low, high] (in either order) contains 0, where x^power is singular.
  //! @param low lower bound @param high upper bound @param power power of x @param source name of the calling function
  //! @throws MathException if x^power is singular in the range
  //!
  static void checkPowerRange(double low, double high, int power, const char * source);

  unsigned int _rank;  //!< number of independent variables
  FitList _fits;   //!<! fits attached (not streamed by ROOT; shared by copies)

  };

  } // namespace MATH
} // namespace CAP

#endif
