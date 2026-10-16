#ifndef CAP_MATH_CompositeFunction
#define CAP_MATH_CompositeFunction
#include <vector>
#include "Aliases.hpp"
#include "Function.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Sum of functions: f(x) = f_0(x) + f_1(x) + ... + f_{n-1}(x).
  //!
  //! All component functions must have the same rank, which is the rank of the composite.
  //! The first function added sets the rank of an empty composite; adding a function of a
  //! different rank afterwards throws MathException.
  //!
  //! Components are not owned: the caller keeps them alive as long as they are in the composite.
  //! Copying a composite copies the pointers, so both copies share the same components.
  //!
  //! derivativeAt() and integralInRange() are the sums of the components' derivatives and integrals.
  //! Components derived from ParameterizedFunction contribute their own derivativeAt() and
  //! integralInRange() (analytical when they provide them); other components are differentiated and
  //! integrated numerically with the ParameterizedFunction default methods.
  //! An empty composite is the zero function.
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
  //! @endcode
  //!
  class CompositeFunction : public Function
  {
  public:

  //!
  //! @brief Empty composite (the zero function).
  //! @param rank number of independent variables (>= 1); replaced by the rank of the first function added
  //! @throws MathException if rank is 0
  //!
  CompositeFunction(unsigned int rank=1);
  //!
  //! @brief Composite of the given functions.
  //! @param functions component functions (not owned), all of the same rank
  //! @throws MathException if a pointer is null or the ranks differ
  //!
  CompositeFunction(const std::vector<Function*> & functions);
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
  //! @brief Append a function to the sum.
  //! @param function function to add (not owned)
  //! @throws MathException if function is null, or if its rank differs from the rank of a non-empty composite
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
  //! @throws MathException if index is out of range, function is null, or its rank differs from the composite's
  //!
  void setFunction(unsigned int index, Function * function);
  //! @brief Remove all functions (the composite becomes the zero function). The functions are not deleted.
  void clearFunctions() { _functions.clear(); }

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

  std::vector<Function*> _functions;  //!< component functions (not owned)

  ClassDef(CompositeFunction,1)
  };

  } // namespace MATH
} // namespace CAP

#endif
