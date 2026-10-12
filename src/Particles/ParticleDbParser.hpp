/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau
 * All rights reserved.
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   05/16/2024
 * *********************************************************************/
#ifndef CAP__ParticleDbParser
#define CAP__ParticleDbParser
#include <fstream>
#include <vector>
#include "PropertiesParser.hpp"
#include "ParticleDb.hpp"

namespace CAP
{

//!
//! @brief Reader of the ASCII particle database: particles.data and decays.data (see DB/ParticleData).
//!
//! particles.data has one particle per line (14 columns):
//! @code
//!   name title pdgCode charge mass massUncertainty width widthUncertainty spin isospin isospin3 quarks antiquarks leptons
//! @endcode
//! decays.data has one decay mode per line (7 columns; "none" for absent products):
//! @code
//!   parent child1 child2 child3 child4 branchingRatio clebschGordan
//! @endcode
//! Lines that are empty or begin with '#' are skipped. When clebschGordan is 1, the branching ratio used to generate
//! decays is the declared one multiplied by the square of the isospin Clebsch-Gordan coefficient (and by 2 for two
//! products of same spin and mass, different isospin3 and same strangeness and charm), see addDecayMode().
//!
//! The XML format of the database (both files in one) is handled by ParticleDbXml, which uses addDecayMode() so that
//! both formats give the same database.
//!
class ParticleDbParser : public PropertiesParser
{
public:

  //! @brief Default constructor.
  ParticleDbParser();
  //! @brief Destructor.
  virtual ~ParticleDbParser() {}
  //!
  //! @brief Read the particles and the decay modes into a database, then map the antiparticles and set up the decay
  //! generators (see finishDb()).
  //! @param particleDb database filled @param inputFileParticles particles.data @param inputFileDecays decays.data
  //! (closed when read) @param verbose report the decay modes whose products are heavier than the parent
  //! @throws Exception for a line that cannot be parsed; UndefinedParticleTypeException for an unknown particle name
  //!
  virtual void readDb(ParticleDb & particleDb,
                      std::ifstream & inputFileParticles,
                      std::ifstream & inputFileDecays,
                      bool verbose=false);
  //!
  //! @brief Add a decay mode to a particle type. The declared branching ratio and the Clebsch-Gordan flag are kept in
  //! the decay mode (so that the database can be written back unchanged); the ratio used to generate decays is the
  //! declared one, completed by the Clebsch-Gordan coefficient if clebschGordan is true (see the class description).
  //! @param parent decaying particle type @param children products (2 to 4) @param declaredBranchingRatio ratio as
  //! given in the database @param clebschGordan whether to complete the ratio by the Clebsch-Gordan coefficient
  //! @param verbose report the modes whose products are heavier than the parent
  //! @throws Exception for fewer than 2 or more than 4 products
  //!
  static void addDecayMode(ParticleType * parent,
                           const std::vector<ParticleType*> & children,
                           double declaredBranchingRatio,
                           bool clebschGordan,
                           bool verbose=false);
  //! @brief Map the antiparticle indices and set up the decay generators of a database just read. @param particleDb database
  static void finishDb(ParticleDb & particleDb);
  //! @brief Clebsch-Gordan coefficient <j1 m1 j2 m2 | J M>.
  static double clebschGordan(double aJot,  double aEm,
                              double aJot1, double aEm1,
                              double aJot2, double aEm2);
  //! @brief Triangle coefficient used by clebschGordan().
  static double deltaJ(double aJot1, double aJot2, double aJot);

  ClassDef(ParticleDbParser,0)
};

} // namespace CAP

#endif /* CAP__ParticleDbParser */
