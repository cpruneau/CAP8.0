/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau
 * All rights reserved.
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#ifndef CAP__ParticleDbXml
#define CAP__ParticleDbXml
#include "ParticleDb.hpp"
#include "XmlDocument.hpp"

namespace CAP
{

//!
//! @brief XML format of the particle database (DB/ParticleData/ParticleDb.xml): the particles and their decay modes in
//! one file, the decay modes of a particle nested in its PARTICLE element.
//!
//! @code
//!   <CAP version="1" type="particleDb">
//!     <PARTICLE_DB>
//!       <PARTICLE name="Lambda1115zero" title="#Lambda" pdg="3122" charge="0" mass="1.115683" massUncertainty="0"
//!                 width="0" widthUncertainty="0" spin="0.5" isospin="0" isospin3="0" quarks="111" antiquarks="0" leptons="0"/>
//!       <PARTICLE name="Sigma1385plus" title="#Sigma^{+}(1385)" pdg="3224" ... >
//!         <DECAY branchingRatio="0.88" clebschGordan="false" products="Lambda1115zero pion0139plus"/>
//!         <DECAY branchingRatio="0.06" clebschGordan="false" products="Sigma1189plus pion0135zero"/>
//!       </PARTICLE>
//!     </PARTICLE_DB>
//!   </CAP>
//! @endcode
//! - massUncertainty, width and widthUncertainty default to 0, clebschGordan to false; the other attributes are required.
//! - branchingRatio is the declared ratio: with clebschGordan="true" the ratio used to generate decays is completed by
//!   the isospin Clebsch-Gordan coefficient when read (see addDecayMode()).
//! - products lists 2 to 4 particle names separated by spaces; a product may be defined anywhere in the file.
//! - Unknown elements or attributes, duplicate particle names and unknown products are errors.
//!
//! Reading appends the particles to the database, then maps the antiparticles and sets up the decay generators.
//! Writing a database reproduces the content of the file it was read from.
//!
class ParticleDbXml
{
public:

  //! @brief Read a database from an XML file. @param particleDb database filled @param fileName file name
  //! @throws FileException, XmlException for an unreadable file or invalid content
  static void read(ParticleDb & particleDb, const String & fileName);
  //! @brief Read a database from the XML file path/name(.xml). @param particleDb database filled @param path directory @param name file name
  static void read(ParticleDb & particleDb, const String & path, const String & name);
  //! @brief Read a database from a PARTICLE_DB element. @param particleDb database filled @param element element
  //! @throws XmlException for invalid content
  static void readElement(ParticleDb & particleDb, const XmlElement & element);

  //! @brief PARTICLE_DB element of a database. @param particleDb database @return element
  static XmlElement createElement(const ParticleDb & particleDb);
  //! @brief Write a database to an XML file. @param particleDb database @param fileName file name
  //! @throws FileException if the file cannot be written
  static void write(const ParticleDb & particleDb, const String & fileName);
  //! @brief Write a database to the XML file path/name(.xml). @param particleDb database @param path directory @param name file name
  static void write(const ParticleDb & particleDb, const String & path, const String & name);

  //! @brief Whether a file name designates an XML database (extension .xml). @param fileName file name @return true if so
  static bool isXmlFileName(const String & fileName);

  //!
  //! @brief Add a decay mode to a particle type. The declared branching ratio and the Clebsch-Gordan flag are kept in
  //! the decay mode (so that the database is written back unchanged); the ratio used to generate decays is the
  //! declared one or, if clebschGordan is true, the declared one multiplied by the square of the isospin
  //! Clebsch-Gordan coefficient (and by 2 for two products of same spin and mass, different isospin3 and same
  //! strangeness and charm). @param parent decaying particle type @param children products (2 to 4)
  //! @param declaredBranchingRatio ratio as given in the database @param clebschGordan whether to complete the ratio
  //! @throws Exception for fewer than 2 or more than 4 products
  //!
  static void addDecayMode(ParticleType * parent, const std::vector<ParticleType*> & children,
                           double declaredBranchingRatio, bool clebschGordan);
  //! @brief Clebsch-Gordan coefficient <j1 m1 j2 m2 | J M>.
  static double clebschGordan(double aJot,  double aEm,
                              double aJot1, double aEm1,
                              double aJot2, double aEm2);
  //! @brief Triangle coefficient used by clebschGordan().
  static double deltaJ(double aJot1, double aJot2, double aJot);

private:
  static XmlDocument createDocument(const ParticleDb & particleDb);
};

} // namespace CAP

#endif /* CAP__ParticleDbXml */
