#ifndef CAP__NuclearDistribution
#define CAP__NuclearDistribution
#include "MathConstants.hpp"
#include "Profile.hpp"
#include <vector>

namespace CAP
{
  //!
    //! @brief Radial nuclear density distribution used to place the nucleons: type 0 exponential, 1 three-parameter Fermi
    //! (3pF), 2 three-parameter Gaussian (3pG), 3 and 4 Hulthen (deuteron), 5 ellipsoid, 8 deformed (TF2), 9 and 10
    //! proton Gaussian and double Gaussian (6 and 7 not implemented). random() draws a radius from it.
    //!
  //!Class implements the notion of nuclear density distribution.
  //!
  //!From Improved Monte Carlo Glauber predictions at present and future nuclear colliders
  //!by Loizides, Kamin, d'Enterria
  //!
  //!The implementation of the MCG calculation is described in  detail in [6, 7].
  //!It consists of two steps: first, constructing the nuclei and, second, colliding the nuclei.
  //!To construct a nucleus, the position of each nucleon is determined according to a
  //!probability density function usually   taken from measurements of the charge density
  //!distribution of   the nucleus [2, 3]. For spherical nuclei, the nucleon positions
  //!can be determined in polar coordinates with a uniform distribution for the azimuthal and polar angles,
  //!coupled with a 2pF   distribution in the radial direction
  //![6] B. Alver, M. Baker, C. Loizides, and P. Steinberg, “The  PHOBOS Glauber Monte Carlo,” arXiv:0805.4411   [nucl-ex].
  //![7] C. Loizides, J. Nagle, and P. Steinberg, “Improved version of the PHOBOS Glauber Monte Carlo,” SoftwareX 1-2 (2015)  13–18, arXiv:1408.2549 [nucl-ex].
  //!
  //!Several implmentation and profiles are used -- based on the original code by Loizides and Steinberg.
  //!
  class NuclearDistribution
  {
  public:

  //! @brief Default constructor.
  NuclearDistribution();
  //! @brief Copy constructor. @param src object to copy
  NuclearDistribution(const NuclearDistribution & src);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  NuclearDistribution & operator=(const NuclearDistribution & rhs);
  //! @brief Destructor.
  virtual ~NuclearDistribution(){ delete _radiusGen; }
  //! @brief Density at a radius and polar angle. @param x radius (fm) @param theta polar angle (deformed nuclei) @return density
  virtual double evaluate(double x, double theta=0.0);
  //! @brief Random radius drawn from the distribution (from the cumulative sums of the radius histogram). @return radius (fm)
  virtual double random();
  //! @brief Reset the parameters.
  virtual void reset();
  //! @brief Create the radius histogram used by random().
  virtual void createHistogram();

  //! @brief Set the type of distribution (see the class description). @param value value
  void setType(int value)      {  _type   = value; }
  //! @brief Set the radius parameter (fm). @param value value
  void setRadius(double value) {  _radius = value; }
  //! @brief Set the skin depth (fm). @param value value
  void setSkin(double value)   {  _skin   = value; }
  //! @brief Set the w parameter (3pF, 3pG). @param value value
  void setW(double value)      {  _w = value;     }
  //! @brief Set the parameter a. @param value value
  void setA(double value)      {  _a = value;     }
  //! @brief Set the parameter b. @param value value
  void setB(double value)      {  _b = value;     }
  //! @brief Set the quadrupole deformation. @param value value
  void setBeta2(double value)  {  _beta2 = value; }
  //! @brief Set the hexadecapole deformation. @param value value
  void setBeta4(double value)  {  _beta4 = value; }
  //! @brief Set the parameter r0. @param value value
  void setR0(double value)     {  _r0 = value;    }
  //! @brief Set the parameter r1. @param value value
  void setR1(double value)     {  _r1 = value;    }
  //! @brief Set the parameter r2. @param value value
  void setR2(double value)     {  _r2 = value;    }
  //! @brief Set the maximum radius (fm). @param value value
  void setMaxRadius(double value) { _maxRadius = value; }

  //! @brief Type of distribution (see the class description). @return value
  int type() const       { return _type;  }
  //! @brief Radius parameter (fm). @return value
  double radius() const  { return _radius; }
  //! @brief Skin depth (fm). @return value
  double skin() const    { return _skin; }
  //! @brief w parameter (3pF, 3pG). @return value
  double w() const       { return _w;    }
  //! @brief Parameter a. @return value
  double a() const       { return _a;    }
  //! @brief Parameter b. @return value
  double b() const       { return _b;    }
  //! @brief Quadrupole deformation. @return value
  double beta2() const   { return _beta2;}
  //! @brief Hexadecapole deformation. @return value
  double beta4() const   { return _beta4;}
  //! @brief Parameter r0. @return value
  double r0() const      { return _r0;   }
  //! @brief Parameter r1. @return value
  double r1() const      { return _r1;   }
  //! @brief Parameter r2. @return value
  double r2() const      { return _r2;   }
  //! @brief Maximum radius (fm). @return value
  double maxRadius() const { return _maxRadius; }

  

  protected:
  
  //! Type of distribution (see the class description).
  int    _type;
  //! Radius parameter (fm).
  double _radius;
  //! Skin depth (fm).
  double _skin;
  //! w parameter (3pF, 3pG).
  double _w;
  //! Parameter a.
  double _a;
  //! Parameter b.
  double _b;
  //! Quadrupole deformation.
  double _beta2;
  //! Hexadecapole deformation.
  double _beta4;
  //! Parameter r0.
  double _r0;
  //! Parameter r1.
  double _r1;
  //! Parameter r2.
  double _r2;
  //! Maximum radius (fm).
  double _maxRadius;
  Histogram *  _radiusGen;            //!< nuclear density vs radius (owned)
  std::vector<double> _cumulative;    //!< cumulative sums of _radiusGen, used by random()

  //! Number of instances (used to name the histograms).
  static int _instances;

  };

} // namespace CAP

#endif // !CAP__NuclearDistribution



