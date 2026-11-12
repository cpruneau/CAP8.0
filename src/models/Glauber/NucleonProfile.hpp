/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,  Nov 10, 2026
 * Based on TGlauber.C written originally by B. Alver, M. Baker, 
 * C. Loizides, P. Steinberg, [arXiv:0805.4411](https://arxiv.org/abs/0805.4411) [nucl-ex]
 *
 * *********************************************************************/
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



