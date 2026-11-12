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
#include "GlauberExceptions.hpp"
#include "SmearingProfile.hpp"
using namespace std;
using CAP::MATH::pi;
using CAP::MATH::twoPi;

namespace CAP
{
  SmearingProfile::SmearingProfile()
  :
  _nucleonSigma(0.0)
  {  }
  
  SmearingProfile::SmearingProfile(const SmearingProfile & src)
  :
  _nucleonSigma(src._nucleonSigma)
  {  }
  
  SmearingProfile & SmearingProfile::operator=(const SmearingProfile & rhs)
  {
  if (this!=&rhs)
    {
    _nucleonSigma = rhs._nucleonSigma;
    _twoNucleonSigmaSq = rhs._twoNucleonSigmaSq;
    }
  return *this;
  }
  
  void SmearingProfile::setNucleonSigma(double value)
  {
  if (value<=0.0)
    throw GlauberException("nucleonSigma<=0",__FUNCTION__);
  _nucleonSigma = value;
  _twoNucleonSigmaSq = 2.0*_nucleonSigma*_nucleonSigma;
  }
  
  void SmearingProfile::reset()
  {
  _nucleonSigma = 0.0;
  _twoNucleonSigmaSq = 0.0;
  }
  
  double SmearingProfile::evaluate(double x)
  {
  return x*std::exp(-x*x/_twoNucleonSigmaSq);
  }
  
}  // namespace CAP



