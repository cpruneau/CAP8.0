/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticleEfficiency
#define CAP__ParticleEfficiency
#include <cstdint>
#include <memory>
#include <vector>
#include "KinematicMapFolder.hpp"
#include "MersenneTwister.hpp"
#include "Particle.hpp"

namespace CAP
{

//!
//! @brief Detection efficiency of generator-level particles: decides at random whether each one is detected (accept()),
//! and gives the weight correcting for the losses (weight(), 1/efficiency).
//!
//! The efficiency, in [0, 1], is a kinematic map (see KinematicMap) of pT, eta and phi:
//! - a constant (the default: 1, a perfect detector);
//! - a function (MATH::Function) of rank 1 to 3, its values clamped to [0, 1];
//! - a histogram (MATH::Histogram) of rank 1 to 3, whose values must all be in [0, 1]; outside its range the
//!   efficiency is 0 (outside the acceptance).
//!
//! The efficiency can be restricted to particle species (setSpecies(): PDG codes, with their sign); a particle of
//! another species, or without type, has efficiency 0. With no species given, it applies to all particles.
//!
//! accept() draws a random number r uniformly in [0, 1) and accepts the particle if r < efficiency: always for an
//! efficiency of 1, never for 0. The random numbers come from the generator of the object (CAP's MersenneTwister,
//! seeded with setSeed()) or from a generator given to accept() (e.g. that of the task, seeded by its configuration).
//!
//! As a managed object (PARTICLE_EFFICIENCY, see ManagedObjects and KinematicMapFolder), the keys of object k are
//! \<task\>:PARTICLE_EFFICIENCY_\<k\>:EFFICIENCY:MODEL, VALUE, FUNCTION, PARAMETERS, VARIABLES, the binning keys of the
//! created histograms, and SPECIES (PDG codes, e.g. "211,-211"; empty: all).
//!
//! Example:
//! @code
//!   ParticleEfficiency eff(histogramPtEta);           // rank 2: pT and eta
//!   eff.setSpecies({211,-211});                       // pions only
//!   eff.setSeed(12345);
//!   if (eff.accept(particle)) ...                     // detected
//! @endcode
//!
class ParticleEfficiency : public KinematicMapFolder
{
public:

  //! Variables of the efficiency.
  typedef KinematicMap::Variable Variable;
  static constexpr Variable kPt  = KinematicMap::kPt;   //!< pT
  static constexpr Variable kEta = KinematicMap::kEta;  //!< pseudorapidity
  static constexpr Variable kPhi = KinematicMap::kPhi;  //!< azimuth

  //! @brief Constant efficiency. @param efficiency efficiency in [0, 1] @throws Exception if it is not in [0, 1]
  explicit ParticleEfficiency(double efficiency=1.0);
  //! @brief Efficiency given by a function. @param function function of rank 1 to 3 (shared, not copied)
  //! @param variables its variables, in order (default: see KinematicMap) @throws Exception (see setFunction())
  ParticleEfficiency(std::shared_ptr<const MATH::Function> function, const std::vector<Variable> & variables = {});
  //! @brief Efficiency given by a histogram. @param histogram histogram of rank 1 to 3 (copied) @param variables its
  //! variables, in order (default: see KinematicMap) @throws Exception (see setHistogram())
  ParticleEfficiency(const MATH::Histogram & histogram, const std::vector<Variable> & variables = {});
  //! @brief Copy constructor (the histogram copied, the function shared). @param source efficiency to copy
  ParticleEfficiency(const ParticleEfficiency & source) = default;
  //! @brief Copy assignment. @param rhs efficiency to copy @return this efficiency
  ParticleEfficiency & operator=(const ParticleEfficiency & rhs) = default;
  //! @brief Destructor.
  virtual ~ParticleEfficiency() {}

  //! @name Source of the efficiency
  //! @{
  //! @brief Use a constant efficiency. @param efficiency efficiency in [0, 1] @throws Exception if it is not in [0, 1]
  void setConstant(double efficiency) { setMapConstant(0,efficiency); }
  //! @brief Use a function. @param function function of rank 1 to 3 (shared) @param variables its variables, in order
  //! (empty: the default of its rank) @throws Exception if the function is null, or its rank or the variables do not
  //! match
  void setFunction(std::shared_ptr<const MATH::Function> function, const std::vector<Variable> & variables = {})
  { setMapFunction(0,function,variables); }
  //! @brief Use a histogram (a copy is kept in this folder). @param histogram histogram of rank 1 to 3
  //! @param variables its variables, in order (empty: the default of its rank) @throws Exception if a bin value is not
  //! in [0, 1], or its rank or the variables do not match
  void setHistogram(const MATH::Histogram & histogram, const std::vector<Variable> & variables = {})
  { setMapHistogram(0,histogram,variables); }
  //! @brief The kinematic map of the efficiency. @return map
  const KinematicMap & efficiencyMap() const { return map(0); }
  //! @brief Whether the efficiency is a constant. @return true if it is
  bool isConstant() const { return map(0).model()==KinematicMap::kConstant; }
  //! @brief The constant efficiency (if isConstant()). @return efficiency
  double constant() const { return map(0).constant(); }
  //! @brief The function (null if none). @return function
  const MATH::Function * function() const { return map(0).function(); }
  //! @brief The histogram (null if none). @return histogram
  const MATH::Histogram * histogram() const { return map(0).histogram(); }
  //! @brief Variables of the function or histogram, in order (empty for a constant). @return variables
  const std::vector<Variable> & variables() const { return map(0).variables(); }
  //! @brief Lower edge of the phi range (phi taken in [phiMinimum(), phiMinimum() + 2pi)). @return edge
  double phiMinimum() const { return map(0).phiMinimum(); }
  //! @brief Set the lower edge of the phi range (set by setHistogram() from its phi axis). @param phiMinimum edge
  void setPhiMinimum(double phiMinimum) { _maps[0].map.setPhiMinimum(phiMinimum); }
  //! @}

  //! @name Species
  //! @{
  //! @brief Restrict the efficiency to species. @param pdgCodes PDG codes (with their sign; empty: all species)
  void setSpecies(const std::vector<int> & pdgCodes) { _species = pdgCodes; }
  //! @brief Species of the efficiency (empty: all). @return PDG codes
  const std::vector<int> & species() const { return _species; }
  //! @brief Whether the efficiency applies to a particle (its species is one of species(), or species() is empty; a
  //! particle without type is of no species). @param particle particle @return true if it applies
  bool appliesTo(const Particle & particle) const;
  //! @}

  //! @name Efficiency, acceptance and correction weight
  //! @{
  //! @brief Efficiency at a point (whatever the species). @param pt pT @param eta pseudorapidity @param phi azimuth
  //! @return efficiency in [0, 1]
  double efficiency(double pt, double eta, double phi) const { return mapValue(0,pt,eta,phi); }
  //! @brief Efficiency of a particle: 0 if appliesTo() is false, else at its pT, eta and phi.
  //! @param particle particle @return efficiency in [0, 1]
  double efficiency(const Particle & particle) const;
  //! @brief Weight correcting for the losses: 1/efficiency(particle), 0 where the efficiency is 0.
  //! @param particle particle @return weight
  double weight(const Particle & particle) const;
  //! @brief Whether a particle is detected: a random number in [0, 1) below efficiency(particle), drawn from the
  //! generator of this object. @param particle generator-level particle @return true if accepted
  bool accept(const Particle & particle) { return accept(particle,_random); }
  //! @brief Same as accept(particle), the random number drawn from a given generator.
  //! @param particle generator-level particle @param random generator @return true if accepted
  bool accept(const Particle & particle, MATH::RandomGenerator & random) const;
  //! @brief Seed the generator of this object (0: a seed of its own, different at each call). @param seed seed
  void setSeed(std::uint64_t seed) { _random.setSeed(seed); }
  //! @}

  //! @brief Configure the efficiency (see the class description and KinematicMapFolder).
  //! @param taskName name of the task @param typeName object type @param configuration configuration @param index index
  virtual void configure(const String & taskName, const String & typeName, const Configuration & configuration,
                         unsigned int index) override;
  //! @brief Kind written in the files. @return PARTICLE_EFFICIENCY
  virtual String kindName() const override { return "PARTICLE_EFFICIENCY"; }

protected:

  //! @brief Save the species (\<BASE_NAME\>_SPECIES). @param outputFile folder
  virtual void saveSettings(Folder & outputFile) const override;
  //! @brief Load the species, if the folder has them. @param inputFile folder
  virtual void loadSettings(const Folder & inputFile) override;

  std::vector<int> _species;          //!< PDG codes of the species (empty: all)
  MATH::MersenneTwister _random;      //!< generator of accept(particle)
};

} // namespace CAP

#endif /* CAP__ParticleEfficiency */
