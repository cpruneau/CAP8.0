/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetSettings.hpp
//! @brief JetSettings: the definition of a jet finder (algorithm, radius R, recombination scheme, pT floor);
//! JetSettingsCreator: the task creating the jet settings of a job (JET_SETTINGS).
#ifndef DATUM__JetSettings
#define DATUM__JetSettings
#include "ManagedObject.hpp"
#include "ObjectCreator.hpp"
#include "Configuration.hpp"
#include "String.hpp"

namespace Datum
{

//!
//! @brief Definition of a jet finder: the algorithm, the radius R, the recombination scheme and the pT floor of
//! FastJet. It selects nothing: the particles clustered are those of a particle filter, the jets kept for the
//! histograms those of a jet filter.
//!
//! The pT floor is the technical minimum of FastJet (ClusterSequence::inclusive_jets(ptFloor)): the jets below it are
//! not kept, which saves the many very soft jets of an event. It does not change the rank of the jets above it (the rank
//! is the order in pT of all the jets found with these settings: 1 for the leading jet).
//!
//! The jet settings of a job are a list of named objects, as the filters: defined by the keys
//! \<owner\>:JET_SETTINGS_\<k\>:NAME, ALGORITHM (ANTIKT, KT or CA), R, RECOMBINATION (E or PT) and PT_FLOOR (GeV/c),
//! made by JetSettingsCreator (OWNER true) and used by name by the jet analyzers (OWNER false). Each analyzer finds the
//! jets of each particle filter for each of its jet settings, so that results found with different settings can be
//! compared in one run.
//!
class JetSettings : public ManagedObject
{
public:

  //! @brief Algorithms of FastJet.
  enum Algorithm { AntiKt, Kt, CambridgeAachen };
  //! @brief Recombination schemes: E (four-momenta added), Pt (pT-weighted, massless jets).
  enum Recombination { EScheme, PtScheme };

  //! @brief Default settings: anti-kt, R = 0.4, E scheme, pT floor 1 GeV/c.
  JetSettings();
  //! @brief Copy constructor. @param source settings
  JetSettings(const JetSettings & source) = default;
  //! @brief Copy assignment. @param rhs settings @return these settings
  JetSettings & operator=(const JetSettings & rhs) = default;
  //! @brief Destructor.
  virtual ~JetSettings() {}

  //! @brief Read the settings: \<owner\>:\<type\>_\<index\>:ALGORITHM, R, RECOMBINATION, PT_FLOOR.
  //! @param ownerName owner @param typeName JET_SETTINGS @param configuration configuration @param index index
  //! @throws ConfigurationException if a value is invalid
  virtual void configure(const String & ownerName, const String & typeName, const Configuration & configuration,
                         unsigned int index) override;

  //! @brief Whether two settings define the same jet finder (their names aside). @param other settings @return true if so
  bool sameDefinition(const JetSettings & other) const
  {
  return algorithm==other.algorithm && radius==other.radius && recombination==other.recombination
         && ptFloor==other.ptFloor;
  }
  //! @brief Description, e.g. "anti-kt R = 0.4, E scheme, pT > 1 GeV/c". @return description
  String description() const;

  //! @brief Algorithm from its name (ANTIKT, KT, CA). @param name name @return algorithm
  //! @throws Exception if the name is unknown
  static Algorithm algorithmOf(const String & name);
  //! @brief Recombination scheme from its name (E, PT). @param name name @return scheme @throws Exception if unknown
  static Recombination recombinationOf(const String & name);
  //! @brief Name of an algorithm (ANTIKT, KT, CA). @param algorithm algorithm @return name
  static String algorithmName(Algorithm algorithm);
  //! @brief Name of a recombination scheme (E, PT). @param recombination scheme @return name
  static String recombinationName(Recombination recombination);
  //! @brief Label of a radius: R0p4 for 0.4. @param radius radius @return label
  static String radiusLabel(double radius);

  Algorithm algorithm = AntiKt;           //!< algorithm
  double radius = 0.4;                    //!< radius R
  Recombination recombination = EScheme;  //!< recombination scheme
  double ptFloor = 1.0;                   //!< pT floor of FastJet (GeV/c)
};

//!
//! @brief Task creating the jet settings of a job (JET_SETTINGS), see ObjectCreator; a subtask of FilterCreator.
//!
class JetSettingsCreator : public ObjectCreator<JetSettings>
{
public:
  //! @brief Constructor.
  JetSettingsCreator() : ObjectCreator<JetSettings>("JET_SETTINGS")
  { appendClassName("JetSettingsCreator"); setName("JetSettingsCreator"); }
  //! @brief Copy. @return new task
  Task * clone() override { return new JetSettingsCreator(*this); }
};

} // namespace Datum

#endif /* DATUM__JetSettings */
