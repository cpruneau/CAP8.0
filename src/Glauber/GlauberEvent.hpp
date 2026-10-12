#ifndef CAP__GlauberEvent
#define CAP__GlauberEvent
#include <TObject.h>
#include "GlauberPoint.hpp"
#include "GlauberNucleus.hpp"
#include "GlauberEventMoments.hpp"
#include "ManagedObject.hpp"
#include "Configuration.hpp"

namespace CAP
{

  //!
  //! @brief Event of a Monte Carlo Glauber simulation: the two nuclei, the impact parameter, the numbers of binary and
  //! hard nucleon-nucleon collisions (by type of pair), the interaction positions, and the eccentricity moments of each
  //! nucleus and of the system.
  //!
  class GlauberEvent
  :
  public ManagedObject
  {
  protected:

  //! Impact parameter (fm).
  double   _impactParameter;
  //! Nucleon-nucleon cross section used for the event.
  double   _eventXSection;
  //! Event plane angle.
  double   _eventPlaneAngle;
  double   _abDistance;  ///< distance between the centers of the nuclei
  double   _areaOverlap;  ///< overlap area
  double   _areaAll;  ///< total area
  double   _length;  ///< characteristic length of the overlap region
  //! Number of binary collisions (then by pair type: pp, pn, np, nn).
  int      _nCollisions;
  int      _nPPCollisions;  ///< number of proton-proton collisions
  int      _nPNCollisions;  ///< number of proton-neutron collisions
  int      _nNPCollisions;  ///< number of neutron-proton collisions
  int      _nNNCollisions;  ///< number of neutron-neutron collisions
  //! Number of hard collisions (then by pair type).
  int      _nHard;
  int      _nPPHardCollisions;  ///< number of hard proton-proton collisions
  int      _nPNHardCollisions;  ///< number of hard proton-neutron collisions
  int      _nNPHardCollisions;  ///< number of hard neutron-proton collisions
  int      _nNNHardCollisions;  ///< number of hard neutron-neutron collisions

  //! First nucleus.
  GlauberNucleus _nucleusA;
  //! Second nucleus.
  GlauberNucleus _nucleusB;
  //! Moments of the participants of nucleus A.
  GlauberEventMoments _momentsNucleusA;
  //! Moments of the participants of nucleus B.
  GlauberEventMoments _momentsNucleusB;
  //! Moments of all participants.
  GlauberEventMoments _momentsSystem;
  //! Transverse positions of the nucleon-nucleon interactions.
  std::vector<GlauberPoint> _interactionPositions;

  public:

  //! @brief Default constructor.
  GlauberEvent();
  //! @brief Copy constructor. @param source object to copy
  GlauberEvent(const GlauberEvent & source);
  //! @brief Copy assignment. @param source object to copy @return this object
  GlauberEvent operator=(const GlauberEvent & source);
  //! @brief Destructor.
  virtual ~GlauberEvent();
  //! @brief Reset the event for a new collision.
  virtual void reset();
  //! @brief Clear the event.
  virtual void clear();
  //! @brief Print the object.
  virtual void print() const;

  //! @brief Count a collision between two nucleons (and mark them as wounded).
  //! @param nucleonA nucleon of nucleus A @param nucleonB nucleon of nucleus B @param isHard whether the collision is hard
  virtual void incrementColls(GlauberNucleon & nucleonA, GlauberNucleon & nucleonB, bool isHard=0);

  //! @brief First nucleus. @return value
  const GlauberNucleus & nucleusA() const { return _nucleusA; }
  //! @brief Second nucleus. @return value
  const GlauberNucleus & nucleusB() const { return _nucleusB; }
  const std::vector<GlauberNucleon> & allNucleonsA() const { return _nucleusA.allNucleons(); }
  const std::vector<GlauberNucleon> & allNucleonsB() const { return _nucleusB.allNucleons(); }
  //! @brief Transverse positions of the nucleon-nucleon interactions. @return value
  const std::vector<GlauberPoint> & interactionPositions() const { return _interactionPositions; }
  //! @brief Moments of the participants of nucleus A. @return value
  const GlauberEventMoments & momentsNucleusA() const { return _momentsNucleusA; }
  //! @brief Moments of the participants of nucleus B. @return value
  const GlauberEventMoments & momentsNucleusB() const { return _momentsNucleusB; }
  //! @brief Moments of all participants. @return value
  const GlauberEventMoments & momentsSystem() const   { return _momentsSystem;   }

  //! @brief First nucleus. @return value
  GlauberNucleus & nucleusA() { return _nucleusA; }
  //! @brief Second nucleus. @return value
  GlauberNucleus & nucleusB() { return _nucleusB; }
  std::vector<GlauberNucleon> & allNucleonsA()   { return _nucleusA.allNucleons(); }
  std::vector<GlauberNucleon> & allNucleonsB()   { return _nucleusB.allNucleons(); }
  //! @brief Transverse positions of the nucleon-nucleon interactions. @return value
  std::vector<GlauberPoint> & interactionPositions()  { return _interactionPositions; }
  //! @brief Moments of the participants of nucleus A. @return value
  GlauberEventMoments & momentsNucleusA()         { return _momentsNucleusA; }
  //! @brief Moments of the participants of nucleus B. @return value
  GlauberEventMoments & momentsNucleusB()         { return _momentsNucleusB; }
  //! @brief Moments of all participants. @return value
  GlauberEventMoments & momentsSystem()           { return _momentsSystem;   }

  //! @brief Set the impact parameter. @param v impact parameter (fm)
  void setImpactParameter(double v) { _impactParameter = v; }
  //! @brief Set the nucleon-nucleon cross section of the event. @param v cross section
  void setEventXSection(double v)   { _eventXSection = v; }
  //! @brief Set the overlap area. @param v area
  void setAreaOverlap(double v)     { _areaOverlap = v; }
  //! @brief Set the total area. @param v area
  void setAreaAll(double v)         { _areaAll = v; }

  //! @brief Impact parameter (fm). @return value
  double   impactParameter() const {  return _impactParameter; };
  //! @brief Nucleon-nucleon cross section used for the event. @return value
  double   eventXSection() const   {  return _eventXSection; };
  //! @brief overlap area. @return value
  double   areaOverlap() const     {  return _areaOverlap; }
  //! @brief total area. @return value
  double   areaAll() const         {  return _areaAll; }

  //! @brief Number of participants (wounded nucleons of both nuclei). @return number
  int nParticipants() const
  {
  int na = _nucleusA.nWoundedNucleons();
  int nb = _nucleusB.nWoundedNucleons();
  return na+nb;
  }

  //! @brief Number of participating protons. @return number
  int nProtonParticipants() const
  {
  int na = _nucleusA.nWoundedProtons();
  int nb = _nucleusB.nWoundedProtons();
  return na+nb;
  }

  //! @brief Number of participating neutrons. @return number
  int nNeutronParticipants() const
  {
  int na = _nucleusA.nWoundedNeutrons();
  int nb = _nucleusB.nWoundedNeutrons();
  return na+nb;
  }


  int nParticipantsA() const  {  return _nucleusA.nWoundedNucleons();  }
  int nParticipantsB() const  {  return _nucleusB.nWoundedNucleons();  }
  int nProtonParticipantsA() const   { return _nucleusA.nWoundedProtons();  }
  int nProtonParticipantsB() const   { return _nucleusB.nWoundedProtons();  }
  int nNeutronParticipantsA() const  { return  _nucleusA.nWoundedNeutrons();}
  int nNeutronParticipantsB() const  { return _nucleusB.nWoundedNeutrons(); }

  //! @brief Number of binary collisions (then by pair type: pp, pn, np, nn). @return value
  int nCollisions() const     {  return _nCollisions;  }
  //! @brief number of proton-proton collisions. @return value
  int nPPCollisions() const   {  return _nPPCollisions;  }
  //! @brief number of proton-neutron collisions. @return value
  int nPNCollisions() const   {  return _nPNCollisions;  }
  //! @brief number of neutron-proton collisions. @return value
  int nNPCollisions() const   {  return _nNPCollisions;  }
  //! @brief number of neutron-neutron collisions. @return value
  int nNNCollisions() const   {  return _nNNCollisions;  }
  int nCollisionsA() const    {  return _nucleusA.nCollisions();  }
  int nCollisionsB() const    {  return _nucleusB.nCollisions();  }

  //! @brief Number of hard collisions (then by pair type). @return value
  int nHardCollisions() const    {  return _nHard; }
  //! @brief number of hard proton-proton collisions. @return value
  int nPPHardCollisions() const  {  return _nPPHardCollisions; }
  //! @brief number of hard proton-neutron collisions. @return value
  int nPNHardCollisions() const  {  return _nPNHardCollisions; }
  //! @brief number of hard neutron-proton collisions. @return value
  int nNPHardCollisions() const  {  return _nNPHardCollisions; }
  //! @brief number of hard neutron-neutron collisions. @return value
  int nNNHardCollisions() const  {  return _nNNHardCollisions; }
  int nHardCollisionsA() const   {  return _nucleusA.nHardCollisions(); }
  int nHardCollisionsB() const   {  return _nucleusB.nHardCollisions(); }

  int nSpectatorsA() const  {  return _nucleusA.nSpectatorNucleons(); }
  int nSpectatorsB() const  {  return _nucleusB.nSpectatorNucleons(); }
  int nProtonSpectatorsA() const  {  return _nucleusA.nSpectatorProtons();  }
  int nProtonSpectatorsB() const  {  return _nucleusB.nSpectatorProtons();  }
  int nNeutronSpectatorsA() const  {  return _nucleusA.nSpectatorNeutrons();  }
  int nNeutronSpectatorsB() const  {  return _nucleusB.nSpectatorNeutrons();  }

  //! @brief Number of spectator nucleons. @return number
  int nSpectators() const
  {
  int na = _nucleusA.nSpectatorNucleons();
  int nb = _nucleusB.nSpectatorNucleons();
  return na+nb;
  }

  //! @brief Number of spectator protons. @return number
  int nProtonSpectators() const
  {
  int na = _nucleusA.nSpectatorProtons();
  int nb = _nucleusB.nSpectatorProtons();
  return na+nb;
  }

  //! @brief Number of spectator neutrons. @return number
  int nNeutronSpectators() const
  {
  int na = _nucleusA.nSpectatorNeutrons();
  int nb = _nucleusB.nSpectatorNeutrons();
  return na+nb;
  }



  //  double   calcDensity(TF1 & prof, double xval, double yval) const;
  //  double   calcArea();
  //  double   calc_length();
  //  double   calcEvent(double bgen);
  //  double   calcResult(double bgen);

  ClassDef(GlauberEvent, 1)
  };

} // namespace CAP

#endif



