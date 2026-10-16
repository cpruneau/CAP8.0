#include "CompositeFunction.hpp"
#include <string>
#include "ParameterizedFunction.hpp"
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::CompositeFunction);

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  // Presents a plain Function as a ParameterizedFunction, so that it gets the numerical
  // derivativeAt() and integralInRange() of ParameterizedFunction.
  class FunctionWrapper : public ParameterizedFunction
  {
  public:
  FunctionWrapper(const Function & f) : ParameterizedFunction(f.rank()), _f(f) {}
  double evaluateAt(const std::vector<double> & x) const override { return _f.evaluateAt(x); }
  using ParameterizedFunction::evaluateAt;
  private:
  const Function & _f;
  };

  // Derivative of one component of rank 1: its own derivativeAt() if it has one, numerical otherwise.
  double componentDerivative(const Function & f, double x)
  {
  if (const ParameterizedFunction * pf = dynamic_cast<const ParameterizedFunction*>(&f)) return pf->derivativeAt(x);
  return FunctionWrapper(f).derivativeAt(x);
  }

  // Integral of one ParameterizedFunction over a box, through the version matching its rank,
  // so that analytical overrides of any version are used.
  double rankIntegral(const ParameterizedFunction & f, const std::vector<double> & low, const std::vector<double> & high)
  {
  switch (f.rank())
    {
    case 1:  return f.integralInRange(low[0],high[0]);
    case 2:  return f.integralInRange(low[0],high[0],low[1],high[1]);
    case 3:  return f.integralInRange(low[0],high[0],low[1],high[1],low[2],high[2]);
    default: return f.integralInRange(low,high);
    }
  }

  // Integral of one component over a box: its own integralInRange() if it has one, numerical otherwise.
  double componentIntegral(const Function & f, const std::vector<double> & low, const std::vector<double> & high)
  {
  if (const ParameterizedFunction * pf = dynamic_cast<const ParameterizedFunction*>(&f)) return rankIntegral(*pf,low,high);
  return rankIntegral(FunctionWrapper(f),low,high);
  }
  } // anonymous namespace

  CompositeFunction::CompositeFunction(unsigned int rank)
  :
  Function(rank),
  _functions()
  {}

  CompositeFunction::CompositeFunction(const std::vector<Function*> & functions)
  :
  Function(1),
  _functions()
  {
  setFunctions(functions);
  }

  CompositeFunction::CompositeFunction(const CompositeFunction & source)
  :
  Function(source),
  _functions(source._functions)
  {}

  CompositeFunction & CompositeFunction::operator=(const CompositeFunction & rhs)
  {
  if (this != &rhs)
    {
    Function::operator=(rhs);
    _functions = rhs._functions;
    }
  return *this;
  }

  void CompositeFunction::checkFunction(const Function * function, unsigned int rank, const char * source)
  {
  if (!function) throw MathException("Null pointer given in lieu of a function",source);
  if (function->rank() != rank)
    {
    String message = "Function of rank ";
    message += std::to_string(function->rank()).c_str();
    message += " cannot be combined with functions of rank ";
    message += std::to_string(rank).c_str();
    throw MathException(message,source);
    }
  }

  void CompositeFunction::setRank(unsigned int rank)
  {
  if (!_functions.empty() && rank != _rank)
    throw MathException("Cannot change the rank of a composite that holds functions",__FUNCTION__);
  Function::setRank(rank);
  }

  void CompositeFunction::addFunction(Function * function)
  {
  if (!function) throw MathException("Null pointer given in lieu of a function",__FUNCTION__);
  if (_functions.empty()) Function::setRank(function->rank());
  checkFunction(function,_rank,__FUNCTION__);
  _functions.push_back(function);
  }

  void CompositeFunction::setFunctions(const std::vector<Function*> & functions)
  {
  // validate everything before changing anything
  if (!functions.empty())
    {
    if (!functions[0]) throw MathException("Null pointer given in lieu of a function",__FUNCTION__);
    for (auto f : functions) checkFunction(f,functions[0]->rank(),__FUNCTION__);
    Function::setRank(functions[0]->rank());
    }
  _functions = functions;
  }

  void CompositeFunction::setFunction(unsigned int index, Function * function)
  {
  if (index >= _functions.size()) throw MathException("Function index out of range",__FUNCTION__);
  checkFunction(function,_rank,__FUNCTION__);
  _functions[index] = function;
  }

  Function * CompositeFunction::function(unsigned int index)
  {
  if (index >= _functions.size()) throw MathException("Function index out of range",__FUNCTION__);
  return _functions[index];
  }

  const Function * CompositeFunction::function(unsigned int index) const
  {
  if (index >= _functions.size()) throw MathException("Function index out of range",__FUNCTION__);
  return _functions[index];
  }

  double CompositeFunction::evaluateAt(double x) const
  {
  checkRank(1,__FUNCTION__);
  double sum = 0.0;
  for (auto f : _functions) sum += f->evaluateAt(x);
  return sum;
  }

  double CompositeFunction::evaluateAt(double x1, double x2) const
  {
  checkRank(2,__FUNCTION__);
  double sum = 0.0;
  for (auto f : _functions) sum += f->evaluateAt(x1,x2);
  return sum;
  }

  double CompositeFunction::evaluateAt(double x1, double x2, double x3) const
  {
  checkRank(3,__FUNCTION__);
  double sum = 0.0;
  for (auto f : _functions) sum += f->evaluateAt(x1,x2,x3);
  return sum;
  }

  double CompositeFunction::evaluateAt(const std::vector<double> & x) const
  {
  checkRank(x.size(),__FUNCTION__);
  double sum = 0.0;
  for (auto f : _functions) sum += f->evaluateAt(x);
  return sum;
  }

  double CompositeFunction::derivativeAt(double x) const
  {
  checkRank(1,__FUNCTION__);
  double sum = 0.0;
  for (auto f : _functions) sum += componentDerivative(*f,x);
  return sum;
  }

  double CompositeFunction::integralInRange(double low, double high) const
  {
  checkRank(1,__FUNCTION__);
  return integralInRange(std::vector<double>{low},std::vector<double>{high});
  }

  double CompositeFunction::integralInRange(double x1Low, double x1High, double x2Low, double x2High) const
  {
  checkRank(2,__FUNCTION__);
  return integralInRange(std::vector<double>{x1Low,x2Low},std::vector<double>{x1High,x2High});
  }

  double CompositeFunction::integralInRange(double x1Low, double x1High, double x2Low, double x2High, double x3Low, double x3High) const
  {
  checkRank(3,__FUNCTION__);
  return integralInRange(std::vector<double>{x1Low,x2Low,x3Low},std::vector<double>{x1High,x2High,x3High});
  }

  double CompositeFunction::integralInRange(const std::vector<double> & low, const std::vector<double> & high) const
  {
  checkRank(low.size(),__FUNCTION__);
  checkRank(high.size(),__FUNCTION__);
  double sum = 0.0;
  for (auto f : _functions) sum += componentIntegral(*f,low,high);
  return sum;
  }

  } // namespace MATH
} // namespace CAP
