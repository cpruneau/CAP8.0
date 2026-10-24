#ifndef CAP_MATH_Function
#define CAP_MATH_Function
#include <vector>
#include "String.hpp"
#include "Object.hpp"
#include "FitRecord.hpp"

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

  unsigned int _rank;  //!< number of independent variables
  FitList _fits;   //!<! fits attached (not streamed by ROOT; shared by copies)

  };

  } // namespace MATH
} // namespace CAP

#endif
