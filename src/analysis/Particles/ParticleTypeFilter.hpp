#ifndef CAP__ParticleTypeFilter
#define CAP__ParticleTypeFilter
#include "ParticleType.hpp"
#include "Filter.hpp"
#include "ManagedObject.hpp"
#include "Configuration.hpp"

namespace CAP
{

  //!
  //! @brief Filter selecting particle types (ParticleType) by their properties: stability, PDG code, mass, charge,
  //! strangeness, charm, bottom, baryon number, lepton numbers, kind (lepton, neutrino, photon, gluon, weak boson,
  //! Higgs), lifetime, width.
  //!
  //! A type is accepted when it satisfies all the conditions (none: all accepted). The conditions are read by configure()
  //! from the keys \<owner\>:\<type\>_\<index\>:CONDITION:N and \<owner\>:\<type\>_\<index\>:CONDITION_\<k\>:TYPE, SUBTYPE, MINIMUM,
  //! MAXIMUM (MINIMUM2 and MAXIMUM2 for DOUBLE_DUAL_RANGE).
  //!
  //! @note ParticleTypeFilter.cpp is not compiled (it is not listed in Particles/CMakeLists.txt) and uses enumerators
  //! (kQuark, kLeptonElectron, ...) that are not declared here.
  //!
  class ParticleTypeFilter
  :
  public Filter<ParticleType>,
  public ManagedObject
  {
  public:

  //! @brief Properties a condition can test.
  enum ParticleTypeFilterType
  { kStable, kEnabled, kPDG, kMass, kCharge, kStrange, kCharm, kBottom, kBaryon, kLepton, kNeutrino, kPhoton, kGluon, kWeakBoson, kHiggs, kLifeTime, kWidth };

  //! @brief Default constructor: no conditions (all types accepted).
  ParticleTypeFilter();
  //! @brief Copy constructor (the conditions are cloned). @param source filter to copy
  ParticleTypeFilter(const ParticleTypeFilter & source);
  //! @brief Copy assignment (the conditions are cloned). @param rhs filter to copy @return this filter
  ParticleTypeFilter & operator=(const ParticleTypeFilter & rhs);
  //! @brief Destructor.
  virtual ~ParticleTypeFilter() {}
  //! @brief Whether a particle type satisfies all the conditions. @param particleType type @return true if accepted
  virtual bool accept(const ParticleType & particleType) const;
  //! @brief Read the conditions from a configuration (see the class description).
  //! @param ownerName name of the owner (task) @param typeName object type (e.g. PARTICLE_TYPE_FILTER)
  //! @param configuration configuration @param index index of the filter
  virtual void configure(const String & ownerName,
                               const String & typeName,
                               const Configuration & configuration,
                               unsigned int index);

  };

}

#endif /* CAP__ParticleTypeFilter */

