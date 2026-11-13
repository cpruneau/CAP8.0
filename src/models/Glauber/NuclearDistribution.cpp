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
#include "NuclearDistribution.hpp"
#include "MathConstants.hpp"
#include "MersenneTwister.hpp"
#include <algorithm>
#include <cmath>

namespace CAP
{
  namespace
  {
  //! Coefficients of the spherical harmonics Y20 = c2 (3 cos^2 - 1) and Y40 = c4 (35 cos^4 - 30 cos^2 + 3).
  const double kY20 = 0.31539156525252005;   // sqrt(5/(16 pi))
  const double kY40 = 0.10578554691520431;   // 3/(16 sqrt(pi))
  //! Reweighting of type 13 when r0, r1, r2 are not given (TGlauberMC, Pbpnrw).
  const double kR0Type13 = 1.00866, kR1Type13 = -0.000461484, kR2Type13 = -0.000203571;
  }

  NuclearDistribution::NuclearDistribution()
  :
  _type(0),
  _radius(0),
  _skin(0),
  _w(0),
  _a(0),
  _b(0),
  _beta2(0),
  _beta4(0),
  _r0(0),
  _r1(0),
  _r2(0),
  _maxRadius(0),
  _ready(false),
  _grid(),
  _cumulative(),
  _boxHalfSize(0),
  _densityMax(1.0)
  { }

  double NuclearDistribution::reweight(double r) const
  {
  double r0 = _r0, r1 = _r1, r2 = _r2;
  if (_type==13 && r0==0.0 && r1==0.0 && r2==0.0) { r0 = kR0Type13; r1 = kR1Type13; r2 = kR2Type13; }
  if (_type!=12 && _type!=13 && _type!=14) return 1.0;
  double d = r0 + r1*r + r2*r*r;
  return d>0.0 ? 1.0/d : 0.0;
  }

  double NuclearDistribution::deformedRadius(double c) const
  {
  double c2 = c*c;
  switch (_type)
    {
      case 5:  return _radius + _w*c2;
      case 7:  return _radius*(1.0 + _beta2*kY20*(3.0*c2-1.0) + _beta4*kY40*(35.0*c2*c2-30.0*c2+3.0));
      case 8:
      case 14: return _radius*(1.0 + _beta2*0.315*(3.0*c2-1.0) + _beta4*0.105*(35.0*c2*c2-30.0*c2+3.0));
      default: return _radius;
    }
  }

  double NuclearDistribution::density(double r, double cosTheta) const
  {
  switch (_type)
    {
      case 0:  return std::exp(-r/_radius);
      case 1:
      case 11:
      case 12:
      case 13:
      {
      double t = r/_radius;
      return (1.0 + _w*t*t)/(1.0 + std::exp((r-_radius)/_skin))*reweight(r);
      }
      case 2:
      {
      double t = r/_radius;
      return (1.0 + _w*t*t)/(1.0 + std::exp((r*r-_radius*_radius)/(_skin*_skin)));
      }
      case 3:
      case 4:
      {
      // Hulthen wave function squared; radius and skin are its two inverse lengths (fm^-1)
      double a = _radius, b = _skin;
      double norm = a*b*(a+b)/(CAP::MATH::twoPi()*(a-b)*(a-b));
      double f = (r>1.0E-9) ? (std::exp(-a*r)-std::exp(-b*r))/r : (b-a);
      return norm*f*f;
      }
      case 5:
      case 7:
      case 8:
      case 14:
      return 1.0/(1.0 + std::exp((r-deformedRadius(cosTheta))/_skin))*reweight(r);
      case 9:
      {
      double t = r/_radius;
      return std::exp(-0.5*t*t);
      }
      case 10:
      {
      double R = _radius, s = 0.4*_radius;
      return 0.5/(R*R*R)*std::exp(-r*r/(R*R)) + 0.5/(s*s*s)*std::exp(-r*r/(s*s));
      }
      case 15:
      {
      double t2 = r*r/(_skin*_skin);
      return (1.0 + _radius*t2)*std::exp(-t2);
      }
      case 6:
      throw RadialTypeGlauberException(_type,"Type 6: nucleon configurations are read from a file, not drawn from a density",
                                       __FUNCTION__);
      default:
      throw RadialTypeGlauberException(_type,"Radial type not found", __FUNCTION__);
    }
  }

  double NuclearDistribution::evaluate(double x, double theta)
  {
  return x*x*density(x, std::cos(theta));
  }

  double NuclearDistribution::rangeMax() const
  {
  double range = _maxRadius>0.0 ? _maxRadius : 14.0;
  if (isHulthen()) range = std::max(range, 14.0);   // the Hulthen tail is long (TGlauberMC: 0 to 14 fm)
  return range;
  }

  void NuclearDistribution::createHistogram()
  {
  _grid.clear();
  _cumulative.clear();
  if (_type==6) throw RadialTypeGlauberException(_type,"Type 6 has no density",__FUNCTION__);
  if (!isDeformed())
    {
    // radial distribution on a fine grid; random() inverts its cumulative distribution (linear inside a step)
    const double range = rangeMax();
    const int n = std::max(4000, int(1000.0*range));
    _grid.resize(n+1);
    _cumulative.assign(n+1,0.0);
    double previous = 0.0;
    for (int k=0; k<=n; k++)
      {
      double r = range*double(k)/double(n);
      double f = std::max(0.0, r*r*density(r));
      _grid[k] = r;
      if (k>0) _cumulative[k] = _cumulative[k-1] + 0.5*(f+previous)*(r-_grid[k-1]);
      previous = f;
      }
    if (!(_cumulative[n]>0.0)) throw Exception("Empty nuclear density distribution",__FUNCTION__);
    for (double & c : _cumulative) c /= _cumulative[n];
    }
  else
    {
    // box around the deformed nucleus: the largest radius of the surface plus 12 skin depths
    double surface = 0.0;
    for (int k=0; k<=1000; k++) surface = std::max(surface, deformedRadius(double(k)/1000.0));
    _boxHalfSize = std::min(rangeMax(), surface + 12.0*_skin);
    // upper bound of the density in the box (1, but for the reweighting of type 14)
    _densityMax = 0.0;
    const double rMax = _boxHalfSize*std::sqrt(3.0);
    for (int k=0; k<=2000; k++)
      {
      double r = rMax*double(k)/2000.0;
      for (int j=0; j<=20; j++) _densityMax = std::max(_densityMax, density(r, double(j)/20.0));
      }
    _densityMax *= 1.01;
    if (!(_densityMax>0.0)) throw Exception("Empty nuclear density distribution",__FUNCTION__);
    }
  _ready = true;
  }

  double NuclearDistribution::random()
  {
  if (!_ready) createHistogram();
  if (isDeformed())
    {
    double x, y, z;
    randomPosition(x,y,z);
    return std::sqrt(x*x+y*y+z*z);
    }
  double u = CAP::MATH::randomGenerator().random();
  unsigned int k = std::upper_bound(_cumulative.begin(),_cumulative.end(),u) - _cumulative.begin();
  if (k<1) k = 1;
  if (k>=_cumulative.size()) k = _cumulative.size()-1;
  double c0 = _cumulative[k-1], c1 = _cumulative[k];
  double f  = (c1>c0) ? (u-c0)/(c1-c0) : 0.5;
  return _grid[k-1] + f*(_grid[k]-_grid[k-1]);
  }

  void NuclearDistribution::randomPosition(double & x, double & y, double & z)
  {
  if (!_ready) createHistogram();
  CAP::MATH::RandomGenerator & generator = CAP::MATH::randomGenerator();
  if (!isDeformed())
    {
    double r      = random();
    double phi    = CAP::MATH::twoPi()*generator.random();
    double cosTh  = 2.0*generator.random()-1.0;
    double sinTh  = std::sqrt(std::max(0.0,1.0-cosTh*cosTh));
    x = r*sinTh*std::cos(phi);
    y = r*sinTh*std::sin(phi);
    z = r*cosTh;
    return;
    }
  // deformed: uniform in the box, kept with a probability proportional to the density (TGlauberMC, box method)
  const double h = _boxHalfSize;
  for (long trial=0; trial<100000000L; trial++)
    {
    x = h*(2.0*generator.random()-1.0);
    y = h*(2.0*generator.random()-1.0);
    z = h*(2.0*generator.random()-1.0);
    double r = std::sqrt(x*x+y*y+z*z);
    double c = r>0.0 ? z/r : 1.0;
    if (generator.random()*_densityMax < density(r,c)) return;
    }
  throw Exception("No position accepted in the box of a deformed nucleus",__FUNCTION__);
  }

  void NuclearDistribution::reset()
  {
  _type = 0;
  _radius = 0;
  _skin = 0;
  _w = 0;
  _a = 0;
  _b = 0;
  _beta2 = 0;
  _beta4 = 0;
  _r0 = 0;
  _r1 = 0;
  _r2 = 0;
  _maxRadius = 0;
  _ready = false;
  _grid.clear();
  _cumulative.clear();
  _boxHalfSize = 0;
  _densityMax = 1.0;
  }

} // namespace CAP
