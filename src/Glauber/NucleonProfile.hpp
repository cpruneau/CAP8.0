#ifndef CAP__NucleonProfile
#define CAP__NucleonProfile
//#include <ostream>
#include "MathConstants.hpp"


namespace CAP
{
  //!
  //! @brief Radial profile of a nucleon: r^2 exp(-r/R).
  //!
  class NucleonProfile
  {
  public:

  //! @brief Default constructor.
  NucleonProfile();
  //! @brief Copy constructor. @param src object to copy
  NucleonProfile(const NucleonProfile & src);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  NucleonProfile & operator=(const NucleonProfile & rhs);
  //! @brief Destructor.
  virtual ~NucleonProfile(){}
  //! @brief Profile at a radius. @param x radius @return value
  virtual double evaluate(double x);
  //! @brief Set the nucleon radius R. @param value radius (fm)
  void setNucleonRadius(double value);
  //! @brief Nucleon radius. @return radius (fm)
  double nucleonRadius() { return _nucleonRadius; }
  //! @brief Set the radius to 0.
  virtual void reset() { _nucleonRadius = 0.0; }

  protected:

  double _nucleonRadius;
  };

} // namespace CAP

#endif // !CAP__NucleonProfile



