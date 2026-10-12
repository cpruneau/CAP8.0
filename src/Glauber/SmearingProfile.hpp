#ifndef CAP__SmearingProfile
#define CAP__SmearingProfile
#include "MathConstants.hpp"

namespace CAP
{
  //!
  //! @brief Gaussian smearing profile of a nucleon: r exp(-r^2/(2 sigma^2)).
  //!
  class SmearingProfile
  {
  public:

  //! @brief Default constructor.
  SmearingProfile();
  //! @brief Copy constructor. @param src object to copy
  SmearingProfile(const SmearingProfile & src);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  SmearingProfile & operator=(const SmearingProfile & rhs);
  //! @brief Destructor.
  virtual ~SmearingProfile(){}
  //! @brief Profile at a radius. @param x radius @return value
  virtual double evaluate(double x);

  //! @brief Set the width sigma. @param value sigma (fm)
  void setNucleonSigma(double value);
  //! @brief Width sigma. @return sigma (fm)
  double nucleonSigma() { return _nucleonSigma; }
  //! @brief Reset the width.
  void reset();

  protected:

  double _nucleonSigma;
  double _twoNucleonSigmaSq;
  };

} // namespace CAP

#endif // !CAP__SmearingProfile



