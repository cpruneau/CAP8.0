#ifndef CAP_MATH_CompositeFunction
#define CAP_MATH_CompositeFunction
#include <vector>
#include "String.hpp"
#include "Function.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Sum or product of functions.
  //!
  //! The operation (operation(), chosen when the composite is made, or with setOperation()) is:
  //!  - kSum (the default): f(x) = f_0(x) + f_1(x) + ... + f_{n-1}(x);
  //!  - kProduct: a product in which each component is a function of its own variables, taken in the order the
  //!    components were added: f(x, y) = f_0(x) f_1(y) for two functions of one variable, and in general the rank of
  //!    the composite is the sum of those of its components (as ParameterizedComposite::kProduct). This makes
  //!    functions of two (or more) variables of functions of one, e.g. a Gaussian in x times an exponential in y;
  //!  - kPointwiseProduct: f(x) = f_0(x) f_1(x) ... f_{n-1}(x), all the components of the same variables.
  //!
  //! For a sum or a pointwise product, all component functions must have the same rank, which is the rank of the
  //! composite: the first function added sets the rank of an empty composite, and adding a function of a different
  //! rank afterwards throws MathException.
  //!
  //! Components are not owned: the caller keeps them alive as long as they are in the composite.
  //! Copying a composite copies the pointers, so both copies share the same components.
  //!
  //! derivativeAt() is the sum of the components' derivatives, or follows the product rule; integralInRange() over a
  //! box is the sum of the components' integrals, the product of their integrals over their own variables (kProduct),
  //! or numerical (kPointwiseProduct). Components derived from ParameterizedFunction contribute their own
  //! derivativeAt() and integralInRange() (analytical when they provide them); other components are differentiated
  //! and integrated numerically with the ParameterizedFunction default methods.
  //! An empty sum is the zero function, an empty product the constant 1.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Linear   line(1.0, 2.0);   // 1 + 2x
  //!   CAP::MATH::Constant offset(3.0);      // 3
  //!   CAP::MATH::CompositeFunction f;
  //!   f.addFunction(&line);
  //!   f.addFunction(&offset);
  //!   double y = f.evaluateAt(2.0);             // 8
  //!   double d = f.derivativeAt(2.0);           // 2
  //!   double s = f.integralInRange(0.0, 1.0);   // 5
  //!
  //!   CAP::MATH::Normal      gx(1.0, 0.0, 1.0);   // exp(-x^2/2)
  //!   CAP::MATH::Exponential ey(2.0, -0.5);       // 2 exp(-y/2)
  //!   CAP::MATH::CompositeFunction g = CAP::MATH::CompositeFunction::product(&gx, &ey);   // rank 2
  //!   double v = g.evaluateAt(1.0, 2.0);           // exp(-1/2) 2 exp(-1)
  //!   double w = g.integralInRange(-1.0, 1.0, 0.0, 4.0);   // integral of gx times integral of ey
  //! @endcode
  //!
  class CompositeFunction : public Function
  {
  public:

  //! @brief How the functions are combined (see the class).
  enum Operation { kSum, kProduct, kPointwiseProduct };

  //!
  //! @brief Empty composite (the zero function).
  //! @param rank number of independent variables (>= 1); replaced by the rank of the first function added
  //! @throws MathException if rank is 0
  //!
  CompositeFunction(unsigned int rank=1);
  //!
  //! @brief Empty composite of an operation (of rank 1 until a function is added).
  //! @param operation sum, product (each component of its own variables) or pointwise product
  //!
  explicit CompositeFunction(Operation operation);
  //!
  //! @brief Composite of the given functions.
  //! @param functions component functions (not owned), all of the same rank
  //! @throws MathException if a pointer is null or the ranks differ
  //!
  CompositeFunction(const std::vector<Function*> & functions);
  //!
  //! @brief Composite of the given functions, with an operation.
  //! @param functions component functions (not owned) @param operation sum, product or pointwise product
  //! @throws MathException if a pointer is null or the ranks differ (sum and pointwise product)
  //!
  CompositeFunction(const std::vector<Function*> & functions, Operation operation);
  //!
  //! @brief Product f(x) g(y) (kProduct) of two functions of their own variables (not owned).
  //! @param f first function (of the first variables) @param g second function (of the next ones)
  //! @return composite whose rank is the sum of theirs @throws MathException if a pointer is null
  //!
  static CompositeFunction product(Function * f, Function * g);
  //! @brief Copy constructor (the components are shared, not copied). @param source composite to copy
  CompositeFunction(const CompositeFunction & source);
  //! @brief Assignment operator (the components are shared, not copied). @param rhs composite to copy @return reference to this composite
  CompositeFunction & operator=(const CompositeFunction & rhs);
  //! @brief Destructor. The components are not deleted.
  virtual ~CompositeFunction() {}

  //!
  //! @brief Set the number of independent variables.
  //! @param rank new rank (>= 1)
  //! @throws MathException if rank is 0, or if the composite holds functions of a different rank
  //!
  void setRank(unsigned int rank) override;

  //!
  //! @brief Append a function to the sum or product (in a product, a function of the next variables).
  //! @param function function to add (not owned)
  //! @throws MathException if function is null, or if, for a sum or a pointwise product, its rank differs from the
  //! rank of a non-empty composite
  //!
  void addFunction(Function * function);
  //!
  //! @brief Replace all the functions.
  //! @param functions new component functions (not owned), all of the same rank
  //! @throws MathException if a pointer is null or the ranks differ; the composite is then unchanged
  //!
  void setFunctions(const std::vector<Function*> & functions);
  //!
  //! @brief Replace the function at the given index.
  //! @param index index of the function to replace @param function new function (not owned)
  //! @throws MathException if index is out of range, function is null, or its rank differs from the composite's (from
  //! that of the function replaced, in a product)
  //!
  void setFunction(unsigned int index, Function * function);
  //! @brief Remove all functions (a sum becomes the zero function, a product the constant 1, of rank 1). The
  //! functions are not deleted.
  void clearFunctions() { _functions.clear(); if (_operation == kProduct) Function::setRank(1); }

  //!
  //! @brief Function at the given index.
  //! @param index index of the function
  //! @return pointer to the function
  //! @throws MathException if index is out of range
  //!
  Function * function(unsigned int index);
  //! @brief Function at the given index (const version). @param index index of the function @return pointer to the function @throws MathException if index is out of range
  const Function * function(unsigned int index) const;
  //! @brief All component functions. @return functions
  const std::vector<Function*> & functions() const { return _functions; }
  //! @brief Number of component functions. @return size of the sum
  unsigned int nFunctions() const { return _functions.size(); }
  //! @brief How the functions are combined. @return sum, product or pointwise product
  Operation operation() const { return _operation; }
  //!
  //! @brief Change how the functions are combined (the rank of a product is then the sum of those of its components).
  //! @param operation operation @throws MathException if the functions held do not allow it (different ranks for a
  //! sum or a pointwise product); the composite is then unchanged
  //!
  void setOperation(Operation operation);

  //! @brief Value of a composite of rank 1. @param x variable @return sum of f_i(x) @throws MathException if the rank is not 1
  double evaluateAt(double x) const override;
  //! @brief Value of a composite of rank 2. @param x1 first variable @param x2 second variable @return sum of f_i(x1,x2) @throws MathException if the rank is not 2
  double evaluateAt(double x1, double x2) const override;
  //! @brief Value of a composite of rank 3. @param x1 first variable @param x2 second variable @param x3 third variable @return sum of f_i(x1,x2,x3) @throws MathException if the rank is not 3
  double evaluateAt(double x1, double x2, double x3) const override;
  //! @brief Value of the composite at x, for any rank. @param x values of the rank() variables @return sum of f_i(x) @throws MathException if x.size() differs from the rank
  double evaluateAt(const std::vector<double> & x) const override;

  //!
  //! @brief First derivative of a composite of rank 1: sum of the components' derivatives.
  //! @param x value of the independent variable
  //! @return sum of f_i'(x)
  //! @throws MathException if the rank is not 1
  //!
  virtual double derivativeAt(double x) const;

  //!
  //! @brief Integral of a composite of rank 1 from low to high: sum of the components' integrals.
  //! @param low lower bound @param high upper bound
  //! @return integral of f(x) dx from low to high
  //! @throws MathException if the rank is not 1, or (numerical integration) if a bound is not finite
  //!
  virtual double integralInRange(double low, double high) const;
  //!
  //! @brief Integral of a composite of rank 2 over [x1Low, x1High] x [x2Low, x2High].
  //! @param x1Low lower bound of x1 @param x1High upper bound of x1 @param x2Low lower bound of x2 @param x2High upper bound of x2
  //! @return integral of f dx1 dx2
  //! @throws MathException if the rank is not 2, or (numerical integration) if a bound is not finite
  //!
  virtual double integralInRange(double x1Low, double x1High, double x2Low, double x2High) const;
  //!
  //! @brief Integral of a composite of rank 3 over [x1Low, x1High] x [x2Low, x2High] x [x3Low, x3High].
  //! @param x1Low lower bound of x1 @param x1High upper bound of x1 @param x2Low lower bound of x2
  //! @param x2High upper bound of x2 @param x3Low lower bound of x3 @param x3High upper bound of x3
  //! @return integral of f dx1 dx2 dx3
  //! @throws MathException if the rank is not 3, or (numerical integration) if a bound is not finite
  //!
  virtual double integralInRange(double x1Low, double x1High, double x2Low, double x2High, double x3Low, double x3High) const;
  //!
  //! @brief Integral of the composite over the box [low[0], high[0]] x ... x [low[n-1], high[n-1]], for any rank n.
  //! @param low lower bounds, one per variable @param high upper bounds, one per variable
  //! @return integral of f over the box
  //! @throws MathException if low.size() or high.size() differs from the rank, or (numerical integration) if a bound is not finite
  //!
  virtual double integralInRange(const std::vector<double> & low, const std::vector<double> & high) const;

  protected:

  //! @brief Throw unless function is non-null and has the given rank. @param function function to check @param rank expected rank @param source name of the calling function
  static void checkFunction(const Function * function, unsigned int rank, const char * source);
  //! @brief Value of a component at its variables. @param f component @param x first of its variables @return value
  static double componentValue(const Function & f, const double * x);
  //! @brief Value at x (rank() values), by the operation. @param x variables @return value
  double combine(const double * x) const;
  //! @brief Recompute the rank of a product (sum of those of its components).
  void updateProductRank();

  std::vector<Function*> _functions;  //!< component functions (not owned)
  Operation _operation;  //!< how the functions are combined

  };

  } // namespace MATH
} // namespace CAP

#endif
