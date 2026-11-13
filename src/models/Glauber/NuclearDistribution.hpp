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
#ifndef CAP__NuclearDistribution
#define CAP__NuclearDistribution
#include <vector>

namespace CAP
{
  //!
  //! @brief Density of the nucleons of a nucleus, and the random positions drawn from it (TGlauberMC v3.2 types).
  //!
  //! Types (the parameters are those of GlauberNucleusType, named as in NucleusDb.xml; R = radius, a = skin):
  //!  - 0 proton, exponential: r^2 exp(-r/R)
  //!  - 1 three-parameter Fermi (3pF): r^2 (1 + w (r/R)^2) / (1 + exp((r-R)/a))
  //!  - 2 three-parameter Gaussian (3pG): r^2 (1 + w (r/R)^2) / (1 + exp((r^2-R^2)/a^2))
  //!  - 3, 4 Hulthen (deuteron; R and a are its two inverse lengths): r^2 ((exp(-R r) - exp(-a r))/r)^2; the
  //!    distance between the two nucleons is drawn from it, each nucleon at half of it (type 4: opposite)
  //!  - 5 ellipsoid (uranium): deformed, 1/(1 + exp((r - R(theta))/a)) with R(theta) = R + w cos^2(theta)
  //!  - 6 nucleon configurations read from a file (no density: see GlauberNucleus)
  //!  - 7 deformed: R(theta) = R (1 + beta2 Y20(theta) + beta4 Y40(theta))
  //!  - 8 deformed, as 7 with the coefficients 0.315 and 0.105 of TGlauberMC
  //!  - 9 proton Gaussian: r^2 exp(-r^2/(2R^2))
  //!  - 10 proton double Gaussian: r^2 (0.5/R^3 exp(-r^2/R^2) + 0.5/(0.4R)^3 exp(-r^2/(0.4R)^2))
  //!  - 11 3pF, with distinct proton and neutron parameters (one distribution for each)
  //!  - 12 3pF reweighted: divided by (r0 + r1 r + r2 r^2)
  //!  - 13 as 11 reweighted (r0, r1, r2 = 1.00866, -0.000461484, -0.000203571 when not given)
  //!  - 14 as 8 reweighted
  //!  - 15 harmonic oscillator: r^2 (1 + R r^2/a^2) exp(-r^2/a^2)
  //!
  //! The spherical types are drawn by inversion of their radial distribution on a fine grid (0 to maxRadius;
  //! 14 fm at least for the Hulthen types, as in TGlauberMC), the deformed ones (5, 7, 8, 14) in a box with
  //! acceptance proportional to the density, along the symmetry axis z (the nucleus is then rotated by
  //! GlauberNucleus).
  //!
  class NuclearDistribution
  {
  public:

  NuclearDistribution();
  NuclearDistribution(const NuclearDistribution & src) = default;
  NuclearDistribution & operator=(const NuclearDistribution & rhs) = default;
  virtual ~NuclearDistribution() {}
  //! @brief Radial distribution r^2 rho(r, theta) (theta: angle to the symmetry axis, deformed types only).
  //! @param x radius (fm) @param theta polar angle @return value (not normalized)
  virtual double evaluate(double x, double theta=0.0);
  //! @brief Density rho(r, cos theta), without the r^2 of the volume element. @param r radius (fm)
  //! @param cosTheta cosine of the angle to the symmetry axis @return value (not normalized)
  double density(double r, double cosTheta=0.0) const;
  //! @brief A radius drawn from the radial distribution (spherical types). @return radius (fm)
  virtual double random();
  //! @brief A position drawn from the density (any type but 6), the symmetry axis along z.
  //! @param x x (fm) @param y y (fm) @param z z (fm)
  void randomPosition(double & x, double & y, double & z);
  //! @brief Whether the type is deformed (5, 7, 8, 14). @return true if it is
  bool isDeformed() const { return _type==5 || _type==7 || _type==8 || _type==14; }
  //! @brief Whether the type is a Hulthen distribution of the deuteron (3, 4). @return true if it is
  bool isHulthen() const { return _type==3 || _type==4; }
  //! @brief Largest radius drawn (fm). @return value
  double rangeMax() const;
  virtual void reset();
  //! @brief Build the tables used by random() and randomPosition() (done on first use).
  virtual void createHistogram();

  void setType(int value)      {  _type   = value; _ready = false; }
  void setRadius(double value) {  _radius = value; _ready = false; }
  void setSkin(double value)   {  _skin   = value; _ready = false; }
  void setW(double value)      {  _w = value;     _ready = false; }
  void setA(double value)      {  _a = value;     _ready = false; }
  void setB(double value)      {  _b = value;     _ready = false; }
  void setBeta2(double value)  {  _beta2 = value; _ready = false; }
  void setBeta4(double value)  {  _beta4 = value; _ready = false; }
  void setR0(double value)     {  _r0 = value;    _ready = false; }
  void setR1(double value)     {  _r1 = value;    _ready = false; }
  void setR2(double value)     {  _r2 = value;    _ready = false; }
  void setMaxRadius(double value) { _maxRadius = value; _ready = false; }

  int type() const       { return _type;  }
  double radius() const  { return _radius; }
  double skin() const    { return _skin; }
  double w() const       { return _w;    }
  double a() const       { return _a;    }
  double b() const       { return _b;    }
  double beta2() const   { return _beta2;}
  double beta4() const   { return _beta4;}
  double r0() const      { return _r0;   }
  double r1() const      { return _r1;   }
  double r2() const      { return _r2;   }
  double maxRadius() const { return _maxRadius; }

  protected:

  //! @brief Radius of the deformed surface in a direction. @param cosTheta cosine of the angle to the axis
  //! @return radius (fm)
  double deformedRadius(double cosTheta) const;
  //! @brief Reweighting factor of the types 12, 13, 14 (1 for the others). @param r radius (fm) @return factor
  double reweight(double r) const;

  int    _type;
  double _radius;
  double _skin;
  double _w;
  double _a;
  double _b;
  double _beta2;
  double _beta4;
  double _r0;
  double _r1;
  double _r2;
  double _maxRadius;
  bool   _ready;                      //!< tables built for the current parameters
  std::vector<double> _grid;          //!< radii of the table of the radial distribution
  std::vector<double> _cumulative;    //!< normalized cumulative distribution at _grid (spherical types)
  double _boxHalfSize;                //!< half size of the box of the deformed types (fm)
  double _densityMax;                 //!< upper bound of the density in the box (deformed types)
  };

} // namespace CAP

#endif // !CAP__NuclearDistribution
