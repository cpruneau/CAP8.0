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
#include "NucleonProfile.hpp"
using namespace std;
using CAP::MATH::pi;
using CAP::MATH::twoPi;

namespace CAP
{
  NucleonProfile::NucleonProfile()
  :
  _nucleonRadius(0)
  { }

  NucleonProfile::NucleonProfile(const NucleonProfile & src)
  :
  _nucleonRadius(src._nucleonRadius)
  { }


  NucleonProfile & NucleonProfile::operator=(const NucleonProfile & rhs)
  {
  if (this!=&rhs)
    {
    _nucleonRadius = rhs._nucleonRadius;
    }
  return *this;
  }

  void NucleonProfile::setNucleonRadius(double value)
  {
  if (value<=0.0)
    throw GlauberException("nucleonRadius<=0",__FUNCTION__);
  _nucleonRadius = value;
  }

  double NucleonProfile::evaluate(double x)
  {
  return x*x*std::exp(-x/_nucleonRadius);
  }

}  // namespace CAP



