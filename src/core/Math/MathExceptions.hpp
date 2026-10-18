#ifndef CAP_MathExceptions
#define CAP_MathExceptions
#include "Exceptions.hpp"
namespace CAP
{
  namespace MATH
  {

  //! @brief Exception thrown by the mathematical classes; the base of the more specific math exceptions.
  class MathException : public Exception
  {
  public:
  //! @brief Exception. @param message what happened @param source function that throws
  MathException(const String & message, const String & source);
  //! @brief Destructor.
  virtual ~MathException() {}

  protected:
  //! @brief Exception of a derived class. @param message what happened @param source function that throws
  //! @param exceptionName name of the derived class
  MathException(const String & message, const String & source, const String & exceptionName);
  };

  //! @brief A value outside its domain [min, max] (e.g. a parameter outside its bounds); a MathException.
  class MathDomainException : public MathException
  {
  public:
  MathDomainException(double min, double max, double x, const String & message, const String & source);
  virtual ~MathDomainException() {}
  virtual void print() const;

  protected:
  double _min;
  double _max;
  double _x;
  };

  class DivByZeroException : public Exception
  {
  public:
  DivByZeroException(const String varName, const String & message, const String & source);
  virtual ~DivByZeroException() {}
  virtual void print() const;

  protected:
  const String  _varName;
  };

  class InvalidBoundsException : public Exception
  {
  public:
  InvalidBoundsException(double min, double max, const String & message, const String & source);
  virtual ~InvalidBoundsException() {}
  virtual void print() const;

  protected:
  double  _min;
  double  _max;
  };




  // MathIterationException

  } // namespace MATH
} // namespace CAP

#endif
