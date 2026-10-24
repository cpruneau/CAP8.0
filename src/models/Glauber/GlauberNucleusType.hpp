#ifndef CAP_GlauberNucleusType
#define CAP_GlauberNucleusType

#include "GlauberExceptions.hpp"
#include "ClassDef.hpp"

namespace CAP
{
using namespace std;

//!
//! @brief Type of nucleus for Monte Carlo Glauber simulations: name, numbers of protons and neutrons, and parameters
//! of the proton and neutron density distributions (radius, skin depth, w, deformations beta2 and beta4, ...).
//! See NuclearDistribution for the radial distribution types.
//!
class GlauberNucleusType
{
public:
  friend class GlauberNucleus;
  //! @brief Default constructor.
  GlauberNucleusType();
  //! @brief Copy constructor. @param src object to copy
  GlauberNucleusType(const GlauberNucleusType & src);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  GlauberNucleusType operator=(const GlauberNucleusType & rhs);
  //! @brief Destructor.
  virtual ~GlauberNucleusType() { }
  //! @brief Print the object.
  //! @brief Print the type.
  virtual void print();

  //! @brief name (e.g., Pb). @return value
  String name() const           { return _name;      }
  //! @brief type code. @return value
  int    type() const           { return _type;      }
  //! @brief number of protons. @return value
  int    nProtons() const       { return _nProtons;  }
  //! @brief number of neutrons. @return value
  int    nNeutrons() const      { return _nNeutrons; }
  int    nNucleons() const      { return _nProtons + _nNeutrons; }
  //! @brief radius of the proton distribution (fm). @return value
  double radiusProton()const    { return _radiusProton; }
  //! @brief skin depth of the proton distribution (fm). @return value
  double skinProton() const     { return _skinProton; }
  //! @brief w parameter of the proton distribution. @return value
  double wProton() const        { return _wProton;    }
  //! @brief radius of the neutron distribution (fm). @return value
  double radiusNeutron() const  { return _radiusNeutron; }
  //! @brief skin depth of the neutron distribution (fm). @return value
  double skinNeutron() const    { return _skinNeutron; }
  //! @brief w parameter of the neutron distribution. @return value
  double wNeutron() const       { return _wNeutron;    }
  //! @brief quadrupole deformation. @return value
  double beta2() const          { return _beta2; }
  //! @brief hexadecapole deformation. @return value
  double beta4() const          { return _beta4; }
  //! @brief maximum radius (fm). @return value
  double maxRadius() const      { return _maxRadius; }
  //! @brief type of radial distribution (see NuclearDistribution). @return value
  int    radialDistType() const { return _radialDistType;}
  //! @brief parameter a. @return value
  double a()    { return _a;   }
  //! @brief parameter b. @return value
  double b()    { return _b;   }
  //! @brief parameter r0. @return value
  double r0()   { return _r0;  }
  //! @brief parameter r1. @return value
  double r1()   { return _r1;  }
  //! @brief parameter r2. @return value
  double r2()   { return _r2;  }

  //! @brief Set the name (e.g., Pb, Au). @param name name
  void setName(const String name)  { _name       = name; }
  //! @brief Set the type code. @param type code
  void setType(int type)           { _type       = type; }
  //! @brief Set the number of protons. @param n number
  void setNProtons(int n)          { _nProtons   = n;  }
  //! @brief Set the number of neutrons. @param n number
  void setNNeutrons(int n)         { _nNeutrons  = n; }
  //! @brief Set whether the Hulthen distribution is used (deuteron). @param v flag
  void setUseHulten(bool v)        { _useHulthen = v; }
  //! @brief Set the radius of the proton distribution. @param v radius (fm)
  void setRadiusProton(double v)   { _radiusProton  = v; }
  //! @brief Set the skin depth of the proton distribution. @param v skin depth (fm)
  void setSkinProton(double v)     { _skinProton    = v; }
  //! @brief Set the w parameter of the proton distribution. @param v w
  void setWProton(double v)        { _wProton = v;};
  //! @brief Set the radius of the neutron distribution. @param v radius (fm)
  void setRadiusNeutron(double v)  { _radiusNeutron = v; }
  //! @brief Set the skin depth of the neutron distribution. @param v skin depth (fm)
  void setSkinNeutron(double v)    { _skinNeutron   = v; }
  //! @brief Set the w parameter of the neutron distribution. @param v w
  void setWNeutron(double v)       { _wNeutron = v;};
  //! @brief Set the quadrupole deformation. @param v beta2
  void setBeta2(double v)          { _beta2 = v; }
  //! @brief Set the hexadecapole deformation. @param v beta4
  void setBeta4(double v)          { _beta4 = v; }
  //! @brief Set the parameter a of the distribution. @param v value
  void setA(double v)            { _a = v;  }
  //! @brief Set the parameter b of the distribution. @param v value
  void setB(double v)            { _b = v;  }
  //! @brief Set the parameter r0 of the distribution. @param v value
  void setR0(double v)           { _r0 = v; }
  //! @brief Set the parameter r1 of the distribution. @param v value
  void setR1(double v)           { _r1 = v; }
  //! @brief Set the parameter r2 of the distribution. @param v value
  void setR2(double v)           { _r2 = v; }
  //! @brief Set the maximum radius at which nucleons are generated. @param v radius (fm)
  void setMaxRadius(double v)    { _maxRadius = v; }
  //! @brief Set the type of radial distribution (see NuclearDistribution). @param v type
  void setRadialDistType(int v)  { _radialDistType = v;}

protected:

  String _name;  ///< name (e.g., Pb)
  int    _type;  ///< type code
  int    _nProtons;  ///< number of protons
  int    _nNeutrons;  ///< number of neutrons
  bool   _useHulthen;  ///< whether the Hulthen distribution is used (deuteron)
  double _radiusProton;  ///< radius of the proton distribution (fm)
  double _skinProton;  ///< skin depth of the proton distribution (fm)
  double _wProton;  ///< w parameter of the proton distribution
  double _radiusNeutron;  ///< radius of the neutron distribution (fm)
  double _skinNeutron;  ///< skin depth of the neutron distribution (fm)
  double _wNeutron;  ///< w parameter of the neutron distribution
  double _beta2;  ///< quadrupole deformation
  double _beta4;  ///< hexadecapole deformation
  double _a;  ///< parameter a
  double _b;  ///< parameter b
  double _r0;  ///< parameter r0
  double _r1;  ///< parameter r1
  double _r2;  ///< parameter r2
  double _maxRadius;  ///< maximum radius (fm)
  int    _radialDistType;  ///< type of radial distribution (see NuclearDistribution)

  ClassDef(GlauberNucleusType,1) // GlauberNucleus class
};

}  // namespace CAP

#endif // !CAP_GlauberNucleusType



