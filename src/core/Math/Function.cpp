#include "Function.hpp"
#include <string>
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::Function);

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

  } // namespace MATH
} // namespace CAP
