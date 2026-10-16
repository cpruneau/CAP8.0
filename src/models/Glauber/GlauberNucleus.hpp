#ifndef CAP__GlauberNucleus
#define CAP__GlauberNucleus
#include "GlauberExceptions.hpp"
#include "GlauberNucleon.hpp"
#include "GlauberNucleusType.hpp"
#include "NuclearDistribution.hpp"

namespace CAP
{
  //!
  //! @brief Nucleus of a Monte Carlo Glauber simulation: its type and nucleons, and the options used to place the
  //! nucleons (overlap allowed or minimum inter-nucleon distance, lattice, number of trials, recentering).
  //!
  //! The nucleons are created by generateNucleons() and placed by positionNucleons() according to the proton and
  //! neutron density distributions of the type (special cases for the proton and the deuteron).
  //!
  class GlauberNucleus
  {
  public:
  
  //! @brief Default constructor.
  GlauberNucleus();
  //! @brief Copy constructor. @param nucleus object to copy
  GlauberNucleus(const GlauberNucleus & nucleus);
  //! @brief Copy assignment. @param nucleus object to copy @return this object
  GlauberNucleus operator=(const GlauberNucleus & nucleus);
  //! @brief Destructor.
  virtual ~GlauberNucleus();
  
  //!
  //!Returns the type of this nucleus
  //!
  const GlauberNucleusType & type() const
  {
  return _nucleusType;
  }
  
  //!
  //!Returns the type of this nucleus
  //!
  GlauberNucleusType & type()
  {
  return _nucleusType;
  }
  
  //!
  //!Returns a constant vector of all the nucleons of this nucleus
  //!
  const vector<GlauberNucleon> & allNucleons() const
  {
  return _nucleons;
  }
  
  //!
  //!Returns a mutable vector of all the nucleons of this nucleus
  //!
  vector<GlauberNucleon> & allNucleons()
  {
  return _nucleons;
  }
  
  //! @brief Name of the nucleus type. @return name
  String name() const      { return _nucleusType.name(); }
  //! @brief Number of protons. @return number
  int nProtons() const     { return _nucleusType.nProtons(); }
  //! @brief Number of neutrons. @return number
  int nNeutrons() const    { return _nucleusType.nNeutrons(); }
  //! @brief Number of nucleons. @return number
  int nNucleons() const    { return _nucleusType.nNucleons(); }
  //! @name Participants, collisions and spectators
  //! @{

  //! @brief Number of wounded nucleons (at least one collision). @return number
  int nWoundedNucleons() const;
  //! @brief Number of wounded protons. @return number
  int nWoundedProtons() const;
  //! @brief Number of wounded neutrons. @return number
  int nWoundedNeutrons() const;
  //! @brief Number of nucleons with at least one hard collision. @return number
  int nWoundedHardNucleons() const;
  //! @brief Number of protons with at least one hard collision. @return number
  int nWoundedHardProtons() const;
  //! @brief Number of neutrons with at least one hard collision. @return number
  int nWoundedHardNeutrons() const;

  //! @brief Number of binary collisions of the nucleons. @return number
  int nCollisions() const;
  //! @brief Number of binary collisions of the protons. @return number
  int nPCollisions() const;
  //! @brief Number of binary collisions of the neutrons. @return number
  int nNCollisions() const;
  //! @brief Number of hard collisions of the nucleons. @return number
  int nHardCollisions() const;
  //! @brief Number of hard collisions of the protons. @return number
  int nPHardCollisions() const;
  //! @brief Number of hard collisions of the neutrons. @return number
  int nNHardCollisions() const;

  //! @brief Number of spectator nucleons (no collision). @return number
  int nSpectatorNucleons() const;
  //! @brief Number of spectator protons. @return number
  int nSpectatorProtons() const;
  //! @brief Number of spectator neutrons. @return number
  //! @}
  int nSpectatorNeutrons() const;

  //! @brief Whether nucleons may overlap. @return value
  bool   allowOverlap() const    { return _allowOverlap;}
  //! @brief Whether nucleons are placed on a lattice. @return value
  bool   useLattice() const      { return _useLattice;}
  //! @brief Type of lattice. @return value
  int    latticeType() const     { return _latticeType;}
  //! @brief Maximum number of trials to place a nucleon. @return value
  int    nTrialsMax() const      { return _nTrialsMax;}
  //! @brief Minimum distance between nucleons (fm). @return value
  double nnDistanceMin() const   { return _nnDistanceMin;}
  //! @brief Squared minimum distance between nucleons. @return value
  double nnDistanceMinSq() const { return _nnDistanceMinSq;}
  //! @brief Distance between lattice nodes (fm). @return value
  double nodeDistance() const    { return _nodeDistance;}
  //! @brief Recentering mode. @return value
  int    recenteringMode() const { return _recenteringMode;}
  //! @brief Overlap mode. @return value
  int    overlapMode() const     { return _overlapMode;}
  //! @brief Whether a user cross section is used. @return value
  bool   userSigmaMode() const   { return _userSigmaMode;}

  //! @brief Create the nucleons (protons and neutrons) of the nucleus type.
  virtual void generateNucleons();
  //! @brief Place the nucleons according to the density distributions (and the overlap or minimum distance options).
  virtual void positionNucleons();
  //! @brief Shift the nucleons so that their center is at the origin.
  virtual void recenterNucleus();
  //! @brief Center of the nucleons. @param center center (set)
  virtual void findNucleusCenter(VectorLorentz & center);
  //! @brief Shift all the nucleons. @param shift shift
  virtual void shiftNucleus(VectorLorentz & shift);
  //! @brief Rotate the nucleus about the x axis. @param phi angle
  virtual void rotateX(double phi);
  //! @brief Rotate the nucleus about the y axis. @param phi angle
  virtual void rotateY(double phi);
  //! @brief Rotate the nucleus about the z axis. @param phi angle
  virtual void rotateZ(double phi);
  //! @brief Rotate the nucleus about the x, y and z axes. @param psiX angle about x @param psiY angle about y @param psiZ angle about z
  virtual void rotateXYZ(double psiX, double psiY, double psiZ);
  //! @brief Reset the collision counters of the nucleons.
  virtual void resetNucleons();
  //! @brief Delete the nucleons.
  virtual void clearNucleons();
  //! @brief Reset the nucleus.
  virtual void reset();
  //! @brief Clear the nucleus.
  virtual void clear();
  //! @brief Print the object.
  virtual void print() const;

  //! @brief Whether a nucleon is closer to a point than a distance. @param x x @param y y @param z z
  //! @param dSqMax squared distance @return true if so
  bool hasNucleonCloseTo(double x, double y, double z, double dSqMax) const;
  //! @brief Whether a nucleon is closer to a position than a distance. @param reference position @param dSqMax squared distance @return true if so
  bool hasNucleonCloseTo(const VectorLorentz & reference, double dSqMax) const;
  //! @brief Whether a nucleon is closer to another nucleon than a distance. @param reference nucleon @param dSqMax squared distance @return true if so
  bool hasNucleonCloseTo(const GlauberNucleon & reference, double dSqMax) const;

  //! @brief Set up the proton and neutron distributions from the nucleus type and generate the nucleons.
  void setNucleus();
  //! @brief Set: whether nucleons may overlap. @param v value
  void setAllowOverlap(bool v)    { _allowOverlap = v;}
  //! @brief Set: whether nucleons are placed on a lattice. @param v value
  void setUseLattice(bool v)      { _useLattice = v;}
  //! @brief Set the maximum number of trials to place a nucleon. @param v value
  void setMaxNumberTrials(int v)  { _nTrialsMax = v;}
  //! @brief Set the type of lattice. @param v value
  void setLatticeType(int v)      { _latticeType = v;}
  //! @brief Set the minimum distance between nucleons (when overlap is not allowed). @param v distance (fm)
  void setMinInterNucleonDistance(double v)
  {
  _nnDistanceMin = v;
  _nnDistanceMinSq = v*v;
  }
  //! @brief Set the distance between lattice nodes (fm). @param v value
  void setNodeDistance(double v)     { _nodeDistance = v;}
  //! @brief Set the recentering mode. @param v value
  void setRecenteringMode(double v)  { _recenteringMode = v; }
  //! @brief Set: whether a user cross section is used. @param v value
  void setUserSigmaMode(bool v)      { _userSigmaMode = v;}

  protected:
  
  //! Type of the nucleus.
  GlauberNucleusType _nucleusType;
  //! Nucleons.
  vector<GlauberNucleon> _nucleons;
  
  //! Whether nucleons may overlap.
  bool   _allowOverlap;
  //! Whether nucleons are placed on a lattice.
  bool   _useLattice;
  //! Type of lattice.
  int    _latticeType;
  //! Maximum number of trials to place a nucleon.
  int    _nTrialsMax;
  //! Minimum distance between nucleons (fm).
  double _nnDistanceMin;
  //! Squared minimum distance between nucleons.
  double _nnDistanceMinSq;
  //! Distance between lattice nodes (fm).
  double _nodeDistance;
  //! Recentering mode.
  int    _recenteringMode;
  //! Overlap mode.
  int    _overlapMode;
  //! Whether a user cross section is used.
  bool   _userSigmaMode;

  //! Density distribution of the protons.
  NuclearDistribution _protonDistribution;
  //! Density distribution of the neutrons.
  NuclearDistribution _neutronDistribution;

  ClassDef(GlauberNucleus,1) // GlauberNucleus class
  };
  
} // namespace CAP

#endif



