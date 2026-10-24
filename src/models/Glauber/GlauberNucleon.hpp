#ifndef CAP__GlauberNucleon
#define CAP__GlauberNucleon
#include "GlauberExceptions.hpp"
#include "MathConstants.hpp"
#include "String.hpp"
//#include "TF1.h"
//#include "TF2.h"
#include "VectorLorentz.hpp"
#include "NuclearDistribution.hpp"

using CAP::MATH::VectorLorentz;

namespace CAP
{

//!
//! @brief Nucleon of a Monte Carlo Glauber simulation: type (proton or neutron), position, numbers of binary and hard
//! collisions, and cross section (when nucleon sizes fluctuate).
//!
//! Class defines  a nucleon for the purpose of MC Glauber simulations
//!
class GlauberNucleon
{
protected:
  int _nBinary;  //! Number of binary collisions
  int _nHard;    //! Number of hard  collisions
  int _type;     //! 0= proton, 1=neutron
  VectorLorentz _position;  ///< position
  double _xSect; // used when nucleon size fluctuate

public:

  //! @brief Default constructor.
  GlauberNucleon();
  //! @brief Copy constructor. @param nucleon object to copy
  GlauberNucleon(const GlauberNucleon & nucleon);
  //! @brief Copy assignment. @param nucleon object to copy @return this object
  GlauberNucleon & operator=(const GlauberNucleon & nucleon);
  //! @brief Destructor.
  virtual ~GlauberNucleon() {}

  //! @brief Set the position and type. @param newPosition position @param type 0 for a proton, 1 for a neutron
  void  set(const VectorLorentz & newPosition, int type);
  //! @brief Set the position. @param newPosition position
  void  setPosition(const VectorLorentz & newPosition);
  //! @brief Set the type. @param type 0 for a proton, 1 for a neutron
  void  setNucleonType(int type)       { _type = type;}
  //! @brief Make the nucleon a proton.
  void  setTypeToProton()              { _type = 0; }
  //! @brief Make the nucleon a neutron.
  void  setTypeToNeutron()             { _type = 1; }
  //! @brief Set the nucleon-nucleon cross section of this nucleon (fluctuating sizes). @param v cross section
  void  setXSect(double v)             { _xSect= v; }

  //! @brief Count a binary collision.
  void  collide()                      { ++_nBinary;}
  //! @brief Count a hard collision.
  void  collideHard()                  { ++_nHard;}
  double get2CWeight(double x) const { return 2.*(0.5*(1-x)+0.5*x*_nBinary);}

  //! @brief position. @return value
  VectorLorentz & position()              { return _position; }
  //! @brief position. @return value
  const VectorLorentz & position() const  { return _position; }
  int   type() const                      { return _type;}
  int   nCollisions() const               { return _nBinary;}
  int   nHardCollisions() const           { return _nHard;}
  bool  isProton() const                  { return _type==0; }
  bool  isNeutron() const                 { return _type==1; }
  bool  isSpectator() const               { return _nBinary==0;}
  bool  isWounded() const                 { return _nBinary>0;}
  bool  isWoundedHard() const             { return _nHard>0;}
  double xSect() const                    { return _xSect;  }

  //! @brief Rotate the position about the x axis. @param phi angle
  void  rotateX(double phi);
  //! @brief Rotate the position about the y axis. @param phi angle
  void  rotateY(double phi);
  //! @brief Rotate the position about the z axis. @param phi angle
  void  rotateZ(double phi);
  //! @brief Rotate the position about the x, y and z axes. @param psiX angle about x @param psiY angle about y @param psiZ angle about z
  void  rotateXYZ(double psiX, double psiY, double psiZ);
  //! @brief Shift the position. @param shift shift
  void  shiftPosition(const VectorLorentz & shift);
  //! @brief Set the numbers of collisions to 0.
  void  reset();
  //! @brief Set the numbers of collisions and the position to 0.
  void  clear();
  //! @name Distances (3D, or in the transverse plane for the 2D versions)
  //! @{

  //! @brief Distance to a point. @return distance
  double distanceTo(double x, double y, double z) const;
  //! @brief Distance to another nucleon. @return distance
  double distanceTo(const GlauberNucleon & nucleon) const;
  //! @brief Transverse distance to a point. @return distance
  double distanceTo2D(double x, double y) const;
  //! @brief Transverse distance to another nucleon. @return distance
  double distanceTo2D(const GlauberNucleon & nucleon) const;
  //! @brief Squared distance to a point. @return squared distance
  double squareDistanceTo(double x, double y, double z) const;
  //! @brief Squared distance to another nucleon. @return squared distance
  double squareDistanceTo(const GlauberNucleon & nucleon) const;
  //! @brief Squared transverse distance to a point. @return squared distance
  double square2DDistanceTo(double x, double y) const;
  //! @brief Squared transverse distance to another nucleon. @return squared distance
  double square2DDistanceTo(const GlauberNucleon & nucleon) const;
  //! @brief Whether the squared distance to a point is below a maximum. @return true if so
  bool   isCloseTo(double x, double y, double z, double maxDistanceSq) const;
  bool   isCloseTo(const VectorLorentz & _position, double maxDistanceSq) const;
  bool   isCloseTo(const GlauberNucleon & nucleon, double maxDistanceSq) const;
  //! @brief Whether the squared transverse distance to a point is below a maximum. @return true if so
  bool   isCloseTo2D(double x, double y, double maxDistanceSq) const;
  bool   isCloseTo2D(const VectorLorentz & _position, double maxDistanceSq) const;
  bool   isCloseTo2D(const GlauberNucleon & nucleon, double maxDistanceSq) const;

  //! @}

  //! @brief Random position uniform in a cube. @param halfSize half size of the cube
  void   setRndmPositionInCube(double halfSize=20.0);
  //! @brief Random position uniform in a sphere. @param radius radius
  void   setRndmPositionInSphere(double radius=400.0);
  //! @brief Random position drawn from a nuclear density distribution. @param d distribution
  void   setRndmPositionInSphere(NuclearDistribution & d);
  //! @brief Print the object.
  //! @brief Print the nucleon.
  void   print() const;

};

}  // namespace CAP


#endif



