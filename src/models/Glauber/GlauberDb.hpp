/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,  Nov 10, 2026
 * Based on TGlauber.C written originally by B. Alver, M. Baker, 
 * C. Loizides, P. Steinberg, [arXiv:0805.4411](https://arxiv.org/abs/0805.4411) [nucl-ex]
 *
 * *********************************************************************/
#ifndef CAP__GlauberDb
#define CAP__GlauberDb
#include <ostream>
#include <vector>
#include "Configuration.hpp"
#include "GlauberNucleus.hpp"
#include "GlauberNucleusType.hpp"
#include "ManagedObject.hpp"
#include "MathConstants.hpp"
#include "NNXSectVsSqrtS.hpp"
#include "NNProfile.hpp"
#include "NucleonProfile.hpp"
#include "PTotProfile.hpp"
#include "SmearingProfile.hpp"
#include "XmlDocument.hpp"

using namespace std;
using CAP::MATH::pi;
using CAP::MATH::twoPi;

namespace CAP
{
  //!
  //! @brief Database of the nucleus types of the Glauber simulations: numbers of nucleons and protons, and parameters
  //! of the nuclear densities of the common nuclei.
  //!
  //! The nuclei are read from an XML file, DB/Glauber/NucleusDb.xml by default, where they can be added or changed:
  //! @code
  //!   <CAP version="1" type="glauberNucleusDb">
  //!     <NUCLEUS_DB>
  //!       <NUCLEUS name="Pb" A="208" Z="82" radialType="1" radius="6.62" skin="0.546" w="0"/>
  //!       <NUCLEUS name="Pbpn" A="208" Z="82" radialType="11" radius="6.68" skin="0.447" w="0"
  //!                neutronRadius="6.69" neutronSkin="0.56" neutronW="0"/>
  //!       <NUCLEUS name="U2" A="238" Z="92" radialType="8" radius="6.67" skin="0.44" w="0" beta2="0.28" beta4="0.093"/>
  //!     </NUCLEUS_DB>
  //!   </CAP>
  //! @endcode
  //! (attributes: see Nucleus and the comment at the top of the file; NucleusDbEditor.py edits and checks it). The file is that of the key
  //! \<task\>:DB_\<k\>:FILE (default Glauber/NucleusDb.xml; relative paths are taken in $CAP_DATABASE_PATH; NONE: the
  //! built-in table). When the file cannot be found, the built-in table, the same nuclei as the file delivered with
  //! CAP, is used.
  //!
  class GlauberDb
  :
  public ManagedObject
  {
  public:

  //! A nucleus type of the database (lengths in fm)
  struct Nucleus
  {
  String name;              //!< name (e.g. Pb, Au2rw)
  int    A = 0;             //!< number of nucleons
  int    Z = 0;             //!< number of protons
  int    radialType = 0;    //!< radial density (see NuclearDistribution)
  double radius = 0;        //!< radius of the proton density
  double skin = 0;          //!< skin depth of the proton density
  double w = 0;             //!< w parameter of the proton density
  bool   neutron = false;   //!< the neutron density has parameters of its own (otherwise those of the protons)
  double neutronRadius = 0; //!< radius of the neutron density
  double neutronSkin = 0;   //!< skin depth of the neutron density
  double neutronW = 0;      //!< w parameter of the neutron density
  double beta2 = 0;         //!< quadrupole deformation
  double beta4 = 0;         //!< hexadecapole deformation
  double r0 = 0;            //!< reweighting parameter r0 (types 12, 13, 14)
  double r1 = 0;            //!< reweighting parameter r1
  double r2 = 0;            //!< reweighting parameter r2
  double a = 0;             //!< parameter a of special profiles
  double b = 0;             //!< parameter b of special profiles
  double maxRadius = 0;     //!< largest radius of the nucleons (0: 3 x 1.2 A^(1/3))
  String comment;           //!< source of the parameters
  };

  //! @brief Default constructor: the default file, read when a nucleus is first looked for.
  GlauberDb();
  //! @brief Copy constructor. @param src object to copy
  GlauberDb(const GlauberDb & src) = default;
  //! @brief Copy assignment. @param rhs object to copy @return this object
  GlauberDb & operator=(const GlauberDb & rhs) = default;
  //! @brief Destructor.
  virtual ~GlauberDb() {}

  //! @brief Set a nucleus type from its name. @param name name (e.g., Pb, Au, Cu, p, d) @param nucleus type set
  //! @throws NucleusNotFoundGlauberException if the database has no nucleus of that name
  void findNucleus(const String & name, GlauberNucleusType & nucleus);
  //! @brief Nucleus of a name. @param name name @return nucleus, or null
  const Nucleus * find(const String & name);
  //! @brief Nuclei of the database (read first if needed). @return nuclei
  const std::vector<Nucleus> & nuclei();

  //! @brief Configure the database: its file, \<taskName\>:\<objectType\>_\<index\>:FILE (see the class description).
  //! @param taskName task @param objectType type (DB) @param configuration configuration @param index index
  virtual void configure(const String & taskName, const String & objectType, const Configuration & configuration,
                         unsigned int index) override;
  //! @brief Set the file of the database (read when a nucleus is first looked for). @param fileName file ("NONE":
  //! the built-in table; relative paths are taken in $CAP_DATABASE_PATH)
  void setFileName(const String & fileName) { _fileName = fileName; _loaded = false; }
  //! @brief File of the database. @return file name, as given
  const String & fileName() const { return _fileName; }

  //! @brief Read the nuclei of an XML file, replacing those held. @param fileName file
  //! @throws Exception if the file cannot be read or a nucleus is malformed
  void loadFromXml(const String & fileName) override;
  //! @brief Read the nuclei of an XML document, replacing those held. @param document document
  //! @throws Exception if a nucleus is malformed
  void loadFromXml(const XmlDocument & document);
  //! @brief The nuclei held as an XML document (the format read). @return document
  XmlDocument toXml() const;
  //! @brief Write the nuclei held to an XML file. @param fileName file
  void exportXml(const String & fileName) const { toXml().write(fileName); }
  //! @brief Use the built-in table, the nuclei of the file delivered with CAP.
  void useBuiltIn();
  //! @brief The built-in table. @return nuclei
  static std::vector<Nucleus> builtIn();

  protected:
  //! @brief Read the file if not yet done (the built-in table if it cannot be found).
  void ensureLoaded();
  String _fileName;                //!< file of the database
  bool _loaded = false;            //!< nuclei read
  String _source;                  //!< where the nuclei come from (file, or "built-in")
  std::vector<Nucleus> _nuclei;    //!< nuclei
  };

} // namespace CAP

#endif
