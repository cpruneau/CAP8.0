/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticleSmear
#define CAP__ParticleSmear
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include "KinematicMapFolder.hpp"
#include "MersenneTwister.hpp"
#include "Particle.hpp"

namespace CAP
{

//!
//! @brief Smearing of the momentum of generator-level particles, simulating the resolution of a detector: smear() makes
//! the detector-level (reconstructed) copy of a particle.
//!
//! Each observable (pT, eta, phi) is smeared independently, by its resolution sigma and, optionally, its bias:
//!     smeared = generated + bias + sigma * deviate
//! where the deviate is drawn from the distribution of the observable (Gaussian by default, see Distribution). The
//! resolution and the bias of each observable are kinematic maps (see KinematicMap) of the generator-level pT, eta and
//! phi: none, a constant, a function (MATH::Function) or a histogram (MATH::Histogram) of rank 1 to 3. An observable
//! without resolution (none: the default) is not smeared; without bias, it is not shifted. Resolutions are >= 0
//! (histograms with a negative value are refused, functions clamped); outside the range of a histogram, the resolution
//! and the bias are 0 (no smearing). The resolution of pT is absolute (GeV/c): a relative resolution is a function of
//! pT, e.g. sigma(pT) = 0.01 pT.
//!
//! Distributions (the deviate in units of sigma): kGaussian (rms 1), kUniform (in [-sqrt 3, sqrt 3): rms 1), kLaplace
//! (double exponential, rms 1), kCauchy (Lorentzian: no rms, sigma is its half width at half maximum), or a function of
//! the user (setDeviate(): kCustom). A smeared pT that is not positive is drawn again (up to 100 times; then the
//! generator-level pT is kept): its distribution is truncated at 0.
//!
//! The smeared particle is a copy of the generated one (type, position, live status, pid) with the smeared momentum at
//! the same mass, and the generated particle as its truth particle (Particle::truth()). The random numbers come from the
//! generator of the object (CAP's MersenneTwister, seeded with setSeed()) or from a generator given to smear().
//!
//! As a managed object (PARTICLE_SMEARER, see ManagedObjects and KinematicMapFolder), the keys of object k are, for
//! each map PT, ETA, PHI (resolutions) and PT_BIAS, ETA_BIAS, PHI_BIAS (biases): \<task\>:PARTICLE_SMEARER_\<k\>:\<MAP\>:
//! MODEL, VALUE, FUNCTION, PARAMETERS, VARIABLES; for each observable PT:DISTRIBUTION, ETA:DISTRIBUTION,
//! PHI:DISTRIBUTION (GAUSSIAN, UNIFORM, LAPLACE or CAUCHY); and the binning keys of the created histograms.
//!
//! Example:
//! @code
//!   ParticleSmear smear;
//!   smear.setResolution(ParticleSmear::kEta,0.01);                                   // sigma(eta) = 0.01
//!   smear.setResolution(ParticleSmear::kPt,std::make_shared<MATH::Polynomial>(std::vector<double>{0.0,0.01}));
//!   smear.setDistribution(ParticleSmear::kPt,ParticleSmear::kCauchy);
//!   smear.smear(generated,reconstructed);
//! @endcode
//!
class ParticleSmear : public KinematicMapFolder
{
public:

  //! Smeared observables.
  enum Observable { kPt=0, kEta, kPhi };
  //! Distributions of the deviates.
  enum Distribution { kGaussian=0, kUniform, kLaplace, kCauchy, kCustom };
  //! Variables of the resolutions and biases.
  typedef KinematicMap::Variable Variable;
  //! A deviate drawn from a generator, in units of the resolution.
  typedef std::function<double(MATH::RandomGenerator &)> Deviate;

  //! @brief No smearing (no resolution).
  ParticleSmear();
  //! @brief Copy constructor (the histograms copied, the functions shared). @param source object to copy
  ParticleSmear(const ParticleSmear & source) = default;
  //! @brief Copy assignment. @param rhs object to copy @return this object
  ParticleSmear & operator=(const ParticleSmear & rhs) = default;
  //! @brief Destructor.
  virtual ~ParticleSmear() {}

  //! @name Resolutions and biases
  //! @{
  //! @brief No smearing of an observable. @param o observable
  void clearResolution(Observable o) { setMapNone(o); }
  //! @brief Constant resolution. @param o observable @param sigma resolution (>= 0) @throws Exception if negative
  void setResolution(Observable o, double sigma) { setMapConstant(o,sigma); }
  //! @brief Resolution given by a function. @param o observable @param function function of rank 1 to 3 (shared)
  //! @param variables its variables, in order (empty: the default of its rank) @throws Exception (see KinematicMap)
  void setResolution(Observable o, std::shared_ptr<const MATH::Function> function, const std::vector<Variable> & variables = {})
  { setMapFunction(o,function,variables); }
  //! @brief Resolution given by a histogram (a copy is kept in this folder). @param o observable @param histogram
  //! histogram of rank 1 to 3 @param variables its variables, in order @throws Exception if a value is negative
  void setResolution(Observable o, const MATH::Histogram & histogram, const std::vector<Variable> & variables = {})
  { setMapHistogram(o,histogram,variables); }
  //! @brief No bias of an observable. @param o observable
  void clearBias(Observable o) { setMapNone(3+o); }
  //! @brief Constant bias. @param o observable @param bias bias
  void setBias(Observable o, double bias) { setMapConstant(3+o,bias); }
  //! @brief Bias given by a function. @param o observable @param function function (shared) @param variables its variables
  void setBias(Observable o, std::shared_ptr<const MATH::Function> function, const std::vector<Variable> & variables = {})
  { setMapFunction(3+o,function,variables); }
  //! @brief Bias given by a histogram (copied). @param o observable @param histogram histogram @param variables its variables
  void setBias(Observable o, const MATH::Histogram & histogram, const std::vector<Variable> & variables = {})
  { setMapHistogram(3+o,histogram,variables); }
  //! @brief Kinematic map of the resolution of an observable. @param o observable @return map
  const KinematicMap & resolutionMap(Observable o) const { return map(o); }
  //! @brief Kinematic map of the bias of an observable. @param o observable @return map
  const KinematicMap & biasMap(Observable o) const { return map(3+o); }
  //! @brief Whether an observable is smeared (it has a resolution). @param o observable @return true if it is
  bool smears(Observable o) const { return !map(o).isNone(); }
  //! @brief Resolution of an observable at a point (0: not smeared). @param o observable @param pt pT
  //! @param eta pseudorapidity @param phi azimuth @return resolution (>= 0)
  double resolution(Observable o, double pt, double eta, double phi) const { return mapValue(o,pt,eta,phi); }
  //! @brief Bias of an observable at a point. @param o observable @param pt pT @param eta pseudorapidity
  //! @param phi azimuth @return bias
  double bias(Observable o, double pt, double eta, double phi) const { return mapValue(3+o,pt,eta,phi); }
  //! @}

  //! @name Distributions
  //! @{
  //! @brief Distribution of the deviates of an observable. @param o observable @param d distribution (not kCustom: use
  //! setDeviate()) @throws Exception for kCustom
  void setDistribution(Observable o, Distribution d);
  //! @brief Deviates of an observable drawn by a function of the user (distribution kCustom), in units of the resolution.
  //! @param o observable @param deviate function @throws Exception if it is empty
  void setDeviate(Observable o, Deviate deviate);
  //! @brief Distribution of the deviates of an observable. @param o observable @return distribution
  Distribution distribution(Observable o) const { return _distribution[o]; }
  //! @brief A deviate of a distribution (not kCustom), in units of the resolution. @param d distribution
  //! @param random generator @return deviate
  static double deviate(Distribution d, MATH::RandomGenerator & random);
  //! @brief Distribution from a text (GAUSSIAN, UNIFORM, LAPLACE, CAUCHY; case ignored). @param text text
  //! @return distribution @throws Exception for an unknown distribution
  static Distribution parseDistribution(const String & text);
  //! @brief Text of a distribution. @param d distribution @return text
  static const char * distributionName(Distribution d);
  //! @}

  //! @name Smearing
  //! @{
  //! @brief Make smeared the detector-level copy of generated (see the class description), with the generator of this
  //! object. Every property of smeared is set: it may be a reused object of Particle::factory().
  //! @param generated generator-level particle (not modified; it becomes the truth particle of smeared)
  //! @param smeared detector-level particle
  void smear(Particle & generated, Particle & smeared) { smear(generated,smeared,_random); }
  //! @brief Same as smear(generated, smeared), the random numbers drawn from a given generator.
  //! @param generated generator-level particle @param smeared detector-level particle @param random generator
  void smear(Particle & generated, Particle & smeared, MATH::RandomGenerator & random) const;
  //! @brief Seed the generator of this object (0: a seed of its own, different at each call). @param seed seed
  void setSeed(std::uint64_t seed) { _random.setSeed(seed); }
  //! @}

  //! @brief Configure the resolutions, biases and distributions (see the class description and KinematicMapFolder).
  //! @param taskName name of the task @param typeName object type @param configuration configuration @param index index
  virtual void configure(const String & taskName, const String & typeName, const Configuration & configuration,
                         unsigned int index) override;
  //! @brief Kind written in the files. @return PARTICLE_SMEARER
  virtual String kindName() const override { return "PARTICLE_SMEARER"; }

protected:

  //! @brief Save the distributions (\<BASE_NAME\>_PT_DISTRIBUTION, ...; a custom one as CUSTOM). @param outputFile folder
  virtual void saveSettings(Folder & outputFile) const override;
  //! @brief Load the distributions, if the folder has them (CUSTOM: the configured distribution is kept, the function
  //! of the user cannot be saved). @param inputFile folder
  virtual void loadSettings(const Folder & inputFile) override;
  //! @brief A deviate of an observable, in units of its resolution. @param o observable @param random generator
  //! @return deviate
  double draw(Observable o, MATH::RandomGenerator & random) const;

  Distribution _distribution[3];      //!< distribution of the deviates of each observable
  Deviate      _deviate[3];           //!< deviates of the user (kCustom)
  MATH::MersenneTwister _random;      //!< generator of smear(generated, smeared)
};

} // namespace CAP

#endif /* CAP__ParticleSmear */
