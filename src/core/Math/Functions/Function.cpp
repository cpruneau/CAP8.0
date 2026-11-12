#include "Function.hpp"
#include <cmath>
#include <string>
#include "MathExceptions.hpp"
#include "SimpsonIntegrator.hpp"
#include "Differentiation.hpp"


namespace CAP
{
  namespace MATH
  {

  Function::Function(unsigned int rank)
  :
  Object("Function","Function","Function"),
  _rank(1)
  {
  setRank(rank);
  }

  Function::Function(const String & name, const String & title, const String & className, unsigned int rank)
  :
  Object(name,title,className),
  _rank(1)
  {
  setRank(rank);
  }

  Function::Function(const Function & source)
  :
  Object(source),
  _rank(source._rank),
  _fits(source._fits)
  {}

  Function & Function::operator=(const Function & rhs)
  {
  if (this != &rhs)
    {
    Object::operator=(rhs);
    _rank = rhs._rank;
    _fits = rhs._fits;
    }
  return *this;
  }

  void Function::setRank(unsigned int rank)
  {
  if (rank==0) throw MathException("Rank of a function must be at least 1",__FUNCTION__);
  _rank = rank;
  }

  void Function::checkRank(unsigned int nArguments, const char * source) const
  {
  if (nArguments!=_rank)
    {
    String message = "Function of rank ";
    message += std::to_string(_rank).c_str();
    message += " called with ";
    message += std::to_string(nArguments).c_str();
    message += " argument(s)";
    throw MathException(message,source);
    }
  }

  double Function::evaluateAt(double x) const
  {
  checkRank(1,__FUNCTION__);
  return evaluateAt(std::vector<double>{x});
  }

  double Function::evaluateAt(double x1, double x2) const
  {
  checkRank(2,__FUNCTION__);
  return evaluateAt(std::vector<double>{x1,x2});
  }

  double Function::evaluateAt(double x1, double x2, double x3) const
  {
  checkRank(3,__FUNCTION__);
  return evaluateAt(std::vector<double>{x1,x2,x3});
  }

  // ---------------------------------------------------------------- derivatives (numerical defaults)

  double Function::derivativeAt(double x) const
  {
  checkRank(1,__FUNCTION__);
  // the derivative of the polynomial of degree 4 through five points around x: step 1e-3 max(1, |x|), error h^4
  static const Differentiation differentiation(Differentiation::Polynomial, 4);
  return differentiation.derivative(*this, x);
  }

  VectorND Function::gradientAt(const std::vector<double> & x) const
  {
  checkRank(x.size(),__FUNCTION__);
  if (_rank==1) return VectorND(1,derivativeAt(x[0]));
  static const Differentiation differentiation(Differentiation::Polynomial, 4);
  return differentiation.gradient(*this, x);
  }

  VectorND Function::gradientAt(double x) const
  {
  checkRank(1,__FUNCTION__);
  return gradientAt(std::vector<double>{x});
  }

  VectorND Function::gradientAt(double x1, double x2) const
  {
  checkRank(2,__FUNCTION__);
  return gradientAt(std::vector<double>{x1,x2});
  }

  VectorND Function::gradientAt(double x1, double x2, double x3) const
  {
  checkRank(3,__FUNCTION__);
  return gradientAt(std::vector<double>{x1,x2,x3});
  }

  // ---------------------------------------------------------------- integrals (numerical defaults)

  double Function::integralInRange(double low, double high) const
  {
  checkRank(1,__FUNCTION__);
  static const SimpsonIntegrator integrator(1.0E-10, 40, 8);
  return integrator.integral(Integrand1D([this](double x) { return evaluateAt(x); }), low, high);
  }

  double Function::integralInRange(double x1Low, double x1High, double x2Low, double x2High) const
  {
  checkRank(2,__FUNCTION__);
  return integralInRange(std::vector<double>{x1Low,x2Low},std::vector<double>{x1High,x2High});
  }

  double Function::integralInRange(double x1Low, double x1High, double x2Low, double x2High, double x3Low, double x3High) const
  {
  checkRank(3,__FUNCTION__);
  return integralInRange(std::vector<double>{x1Low,x2Low,x3Low},std::vector<double>{x1High,x2High,x3High});
  }

  double Function::integralInRange(const std::vector<double> & low, const std::vector<double> & high) const
  {
  checkRank(low.size(),__FUNCTION__);
  checkRank(high.size(),__FUNCTION__);
  if (_rank==1) return integralInRange(low[0],high[0]);
  static const SimpsonIntegrator integrator(1.0E-8, 20, 8);   // one variable at a time
  return integrator.integral(IntegrandND([this](const std::vector<double> & x) { return evaluateAt(x); }), low, high);
  }

  void Function::checkPowerRange(double low, double high, int power, const char * source)
  {
  if (power<0 && std::min(low,high)<=0.0 && std::max(low,high)>=0.0)
    throw MathException("Negative power of x is singular at x=0, which is in the range",source);
  }

  double Function::expectationValue(double low, double high, int power) const
  {
  checkRank(1,__FUNCTION__);
  if (power==1) return expectationValue(low,high);
  checkPowerRange(low,high,power,__FUNCTION__);
  double norm = Function::integralInRange(low,high);
  if (norm==0.0) throw MathException("Integral of the function over the range is zero",__FUNCTION__);
  if (power==0) return 1.0;
  static const SimpsonIntegrator integrator(1.0E-10, 40, 8);
  return integrator.integral(Integrand1D([this,power](double x) { return std::pow(x,power)*evaluateAt(x); }), low, high)/norm;
  }

  double Function::expectationValue(double low, double high) const
  {
  checkRank(1,__FUNCTION__);
  double norm = Function::integralInRange(low,high);
  if (norm==0.0) throw MathException("Integral of the function over the range is zero",__FUNCTION__);
  static const SimpsonIntegrator integrator(1.0E-10, 40, 8);
  return integrator.integral(Integrand1D([this](double x) { return x*evaluateAt(x); }), low, high)/norm;
  }

  } // namespace MATH
} // namespace CAP
