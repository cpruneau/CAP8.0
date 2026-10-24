/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * Copyright (C) 2016, Chun Shen
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticleDb
#define CAP__ParticleDb
#include "Configuration.hpp"
#include "ManagedObject.hpp"
#include "Object.hpp"
#include "ParticleType.hpp"

namespace CAP
{
  class ParticleType;

  //!
  //! @brief Database of particle types (ParticleType) and their decay modes.
  //!
  //! The database is loaded from an XML file holding the particles and their decay modes (loadFromXml(), see
  //! ParticleDbXml; DB/ParticleData/ParticleDb.xml), and can be written as XML (exportXml()). After loading, the particles
  //! and decays are enabled as configured, the types are sorted by mass, and the antiparticle indices are mapped.
  //! The database owns its particle types.
  //!
  //! Types can be looked up by name, PDG code, private code or index, and particles or decays enabled or disabled by
  //! category (neutral, charged, weak decays, lifetime).
  //!
  class ParticleDb
  :
  public ManagedObject
  {
  public:

  // const String& _name, bool _ownership, Severity logLevel
  //! @brief Default constructor: empty database.
  ParticleDb();
  //! @brief Copy constructor. @param analyzer database to copy
  ParticleDb(const ParticleDb & analyzer);
  //! @brief Copy assignment. @param analyzer database to copy @return this database
  ParticleDb & operator=(const ParticleDb & analyzer);

  //! @brief Destructor.
  virtual ~ParticleDb() {}
  //! @brief Sort the types by increasing mass.
  void sortByMass();
  //! @brief New database (not owning its types) with the types of a category.
  //! @param option 0: stable (photon excluded), 1: decay disabled (photon excluded), 2: unstable, 3: decay enabled,
  //! 4: mesons, 5: baryons, 6: leptons, 7: fermions, 8: bosons @return new database (owned by the caller)
  ParticleDb * extractList(int option);
  //! @brief Index of a type. @param type type @return index, or -1 if absent
  int findIndexForType(ParticleType * type) const;
  //! @brief Index of the type of a given name. @param name name @return index, or -1 if absent
  int findIndexForName(const String & name) const;
  //! @brief Index of the type of a given PDG code. @param pdgCode PDG code @return index, or -1 if absent
  int findIndexForPdgCode(int pdgCode) const;
  //! @brief Index of the type of a given private code. @param privateCode code @return index, or -1 if absent
  int findIndexForPrivateCode(int privateCode) const;
  //! @brief Whether the database has a type of a given name. @param aName name @return true if so
  bool containsTypeNamed(const String & aName) const;

  //! @brief Number of types. @return number of types
  unsigned int size() { return _particleTypes.size();}
  //! @brief Types (owned by the database). @return types
  std::vector<ParticleType*> & particleTypes()   { return _particleTypes; }
  //! @brief Types (owned by the database). @return types
  const std::vector<ParticleType*> & particleTypes() const   { return _particleTypes; }

  //! @brief Resolve the product types of the decay modes from their PDG codes (currently does nothing).
  void resolveTypes();
  //! @brief Set the antiparticle index of each type: index of the type of opposite PDG code, or its own index.
  void mapAntiParticleIndices();
  //! @brief Set up the random selection of the decay modes of the types that have decay modes.
  void setupDecayGenerator();
  //! @brief Type of a given PDG code. @param pdgCode PDG code @return type
  //! @throws InvalidIndexParticleTypeException if absent
  ParticleType * findPdgCode(int pdgCode) const;
  //! @brief Type of a given private code. @param privateCode code @return type
  ParticleType * findPrivateCode(int privateCode) const;
  //! @brief Type of a given name. @param aName name @return type @throws UndefinedParticleTypeException if absent
  ParticleType * findTypeNamed(const String & aName) const;
  //  ParticleType * findPdgCode(int pdgCode);
  //  ParticleType * findPrivateCode(int privateCode);
  //  ParticleType * findTypeNamed(const String & aName);
  //! @brief Build the list of weakly decaying hadrons (K0, Lambda, Sigma, Xi, Omega and antiparticles).
  //! Identified by PDG code; codes absent from the database are skipped. The list is rebuilt at each call. @return list
  std::vector<ParticleType*> & findWeakDecays();

  //! @brief Add a type, which the database then owns. @param particleType type created with new
  void append(ParticleType * particleType);
  //! @brief Type of a given name. @param name name @return type @throws UndefinedParticleTypeException if absent
  ParticleType * particleType(const String & name);
  //! @brief Type at an index. @param index index @return type @throws IndexOutOfBoundException if index>=size()
  ParticleType * particleType(unsigned int index);
  //! @brief Delete the types.
  virtual void clear();
  //! @brief PDG codes of the types. @return codes
  std::vector<int> listOfPdgCodes();

  //! @brief Print the types (index, PDG code, name, mass, ...).
  virtual void print() const;
  //! @brief Print the decay modes of the types.
  virtual void printDecays() const;

  //  static ParticleDb & list();


  //! @brief Configure the database from keys \<taskName\>:\<objectType\>_\<index\>:... (_dbEnableAllParticles,
  //! _dbDisableNeutralParticles, _dbEnableAllDecays, _dbDisableWeakDecays, MaxLifeTime, ...).
  //! @param taskName name of the owning task @param objectType object type (PARTICLE_DB)
  //! @param configuration configuration @param index index of the database
  virtual void configure(const String & taskName,
                         const String & objectType,
                         const Configuration & configuration,
                         unsigned int index);


  //! @brief Load the database from an XML file (particles and decays in one file, see ParticleDbXml).
  //! @param fileName file name
  virtual void loadFromXml(const String & fileName);
  //! @brief Write the database (particles and decays) to the XML file path/name(.xml) (see ParticleDbXml).
  //! @param path directory @param name file name
  virtual void exportXml(const String & path, const String & name) const;

  //virtual void dbAnalyzer();
  //! @name Enabling and disabling particles and decays
  //! @{

  //! @brief Enable all particle types.
  virtual void enableAllParticles();
  //! @brief Enable the neutral particle types.
  virtual void enableNeutralParticles();
  //! @brief Enable the charged particle types.
  virtual void enableChargedParticles();
  //! @brief Enable selected particle types. @param selection types
  virtual void enableSelectedParticles(std::vector<ParticleType*> & selection);
  //! @brief Disable all particle types.
  virtual void disableAllParticles();
  //! @brief Disable the neutral particle types.
  virtual void disableNeutralParticles();
  //! @brief Disable the charged particle types.
  virtual void disableChargedParticles();
  //! @brief Disable selected particle types. @param selection types
  virtual void disableSelectedParticles(std::vector<ParticleType*> & selection);

  //! @brief Disable the decays of all types.
  virtual void disableAllDecays();
  //! @brief Disable the decays of the neutral types.
  virtual void disableNeutralDecays();
  //! @brief Disable the decays of the charged types.
  virtual void disableChargedDecays();
  //! @brief Disable the weak decays (see findWeakDecays()).
  virtual void disableWeakDecays();
  //! @brief Disable the decays of the types whose lifetime exceeds a value. @param lifeTime lifetime
  virtual void disableDecaysTauGreatThan(double lifeTime);
  //! @brief Disable the decays of the types whose lifetime is shorter than a value. @param lifeTime lifetime
  virtual void disableDecaysTauShorterThan(double lifeTime);
  //! @brief Disable the decays of selected types. @param selection types
  virtual void disableSelectedDecays(std::vector<ParticleType*> & selection);

  //! @brief Enable the decays of all types.
  virtual void enableAllDecays();
  //! @brief Enable the decays of the neutral types.
  virtual void enableNeutralDecays();
  //! @brief Enable the decays of the charged types.
  virtual void enableChargedDecays();
  //! @brief Enable the weak decays (see findWeakDecays()).
  virtual void enableWeakDecays();
  //! @brief Enable the decays of the types whose lifetime exceeds a value. @param lifeTime lifetime
  virtual void enableDecaysTauGreaterThan(double lifeTime);
  //! @brief Enable the decays of the types whose lifetime is shorter than a value. @param lifeTime lifetime
  virtual void enableDecaysTauShorterThan(double lifeTime);
  //! @brief Enable the decays of selected types. @param selection types
  virtual void enableSelectedDecays(std::vector<ParticleType*> & selection);
  //! @brief Print the enabled/decay status of each type.
  //! @}
  virtual void listParticleStatus();

  protected:
  //! @brief Copy the types of another database. @param db database to copy
  void cloneAll(const ParticleDb & db);
  //! @brief End of loadFromXml(): enable the particles and decays as configured, sort by
  //! mass and map the antiparticles.
  void finishLoad();

  //! Particle types (owned).
  std::vector<ParticleType*> _particleTypes;
  //! Weakly decaying types (see findWeakDecays()).
  std::vector<ParticleType*> _weakDecays;

  //! Flags applied after loading (see configure()).
  bool _dbDisableNeutralParticles;
  bool _dbDisableChargedParticles;
  bool _dbDisableSelected;

  bool _dbEnableAllParticles;
  bool _dbEnableNeutralParticles;
  bool _dbEnableChargedParticles;
  bool _dbEnableSelected;

  bool _dbDisableNeutralDecays;
  bool _dbDisableChargedDecays;
  bool _dbDisableSelectedDecays;
  bool _dbDisableWeakDecays;
  bool _dbDisableLongLivedDecays;
  bool _dbDisableShortLivedDecays;

  bool _dbEnableAllDecays;
  bool _dbEnableNeutralDecays;
  bool _dbEnableChargedDecays;
  bool _dbEnableSelectedDecays;
  bool _dbEnableWeakDecays;
  bool _dbEnableLongLivedDecays;
  bool _dbEnableShortLivedDecays;
  //! Lifetime threshold of the lifetime-based selections of decays.
  double _dbMaxLifeTime;

  };

} //namespace CAP

#endif  // CAP__ParticleDb

