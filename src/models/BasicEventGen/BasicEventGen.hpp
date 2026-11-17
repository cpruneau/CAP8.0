/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef DATUM__BasicEventGen
#define DATUM__BasicEventGen
#include <memory>
#include <vector>
#include "EventProcessor.hpp"
#include "ParticleDb.hpp"
#include "MersenneTwister.hpp"
#include "ParameterizedFunction.hpp"
#include "VectorLorentz.hpp"

namespace Datum
{

//!
//! @brief Simple event generator: independent particles of a few types, without correlations, emitted at the origin.
//!
//! For each event and each selected particle type (\<name\>:PdgSelected, a list of PDG codes separated by commas or
//! spaces, e.g. "211, -211, 321, -321"), the number of particles is drawn from the multiplicity distribution, the same
//! for every type and drawn independently for each, set by \<name\>:Mult:Distribution:
//! - "UNIFORM" (the default): uniform between MultMin and MultMax (inclusive);
//! - "FUNCTION": P(n) proportional to f(n) for the integers n in [MultMin, MultMax], f a function of the function
//!   registry of the Math library, named by \<name\>:Mult:Function:Name, with the parameters of
//!   Mult:Function:Parameters (empty: the defaults of the class), e.g. "Exponential" or a registered composite
//!   (a negative or non-finite f(n) counts as 0);
//! - "HISTOGRAM": P(n) proportional to the contents of a histogram of one variable, n = the integers of each bin
//!   (equally; the integer nearest to the centre of a bin holding none), read from the Datum file Mult:Histogram:File
//!   (".datum" may be omitted) of the directory Mult:Histogram:Path ("DEFAULT": $DATUM_HISTOS_IMPORT_PATH), at the path
//!   Mult:Histogram:Name in it (e.g. "ParticleSingleAnalyzer/ALL_ALL/n1"); MultMin and MultMax are not used
//!   (negative bins and contents count as 0).
//!
//! The momentum of each particle is drawn according to the style of generation, \<name\>:EventGenType:
//!
//! - 0, cylindrical (kCylindrical): rapidity uniform in [RapidityMin, RapidityMax], azimuth uniform in [PhiMin, PhiMax],
//!   and transverse momentum drawn from the momentum distribution. The momenta are restricted either to a range of
//!   transverse momentum, [PtMin, PtMax] (MomentumRange = "PT", the default), or to a range of momentum, [PMin, PMax]
//!   (MomentumRange = "P": pT is drawn in [0, PMax] and the particle kept if its momentum is in [PMin, PMax]).
//! - 1, spherical (kSpherical): momentum drawn from the momentum distribution in [PMin, PMax], polar angle theta
//!   isotropic (cos(theta) uniform) in [ThetaMin, ThetaMax], and azimuth uniform in [PhiMin, PhiMax] (radians).
//!
//! The momentum distribution, dN/dx of x = pT (cylindrical) or p (spherical), is set by \<name\>:Distribution:
//! - "EXPONENTIAL": exp(-x/Slope) (the default, Slope = 0.5 GeV/c);
//! - "GAUSSIAN": exp(-x^2/(2 Sigma^2)), a Gaussian centred at x = 0;
//! - "FUNCTION": a function of the function registry of the Math library, named by \<name\>:Function:Name (a class
//!   such as "Exponential" or "Polynomial", or a registered composite), with the parameters of
//!   \<name\>:Function:Parameters (values separated by commas or spaces; empty: the defaults of the class). The function
//!   must be non-negative on the range; it is tabulated on the range when the task is initialized.
//!
//! Anisotropic flow (for tests of flow analyses): \<name\>:Flow:Vn, a list of coefficients v_1, v_2, ... (empty, the
//! default: none), modulates the azimuthal distribution about a reaction plane Psi drawn uniformly in [0, 2 pi) for each
//! event: dN/dphi proportional to 1 + 2 sum_n v_n(pT) cos(n (phi - Psi)), with v_n(pT) = v_n (Flow:PtScale = 0, the
//! default) or v_n min(pT/PtScale, 1) (a linear rise up to PtScale; pT = p sin(theta) in the spherical style). The
//! azimuth is drawn by rejection in [PhiMin, PhiMax]; the sum of the |v_n| must be below 1/2.
//!
//! The random numbers are drawn by a Mersenne Twister seeded with \<name\>:Random:Seed if Random:SetSeed is true, and
//! with a seed of its own otherwise. A particle is added to the event if the task has no particle filter or if one of
//! its particle filters accepts it.
//!
class BasicEventGen
:
public EventProcessor
{
public:

  //! Styles of generation (\<name\>:EventGenType).
  enum EventGenType { kCylindrical=0, kSpherical=1 };
  //! Momentum distributions (\<name\>:Distribution).
  enum DistributionType { kExponential=0, kGaussian=1, kFunction=2 };
  //! Multiplicity distributions (\<name\>:Mult:Distribution).
  enum MultiplicityType { kMultUniform=0, kMultFunction=1, kMultHistogram=2 };

  //! @brief Default constructor.
  BasicEventGen();
  //! @brief Copy constructor. @param source object to copy
  BasicEventGen(const BasicEventGen & source);
  //! @brief Destructor.
  virtual ~BasicEventGen() {}
  //! @brief Copy assignment. @param rhs object to copy @return this object
  BasicEventGen & operator=(const BasicEventGen & rhs);
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration() override;
  //!
  //! @brief Initialize the task: read the configuration, find the particle types, prepare the momentum distribution.
  //! @throws ConfigurationException if a setting is invalid or a PDG code is not in the particle database
  //!
  virtual void initialize() override;
  //! @brief Execute the task: add the particles of one event.
  virtual void execute() override;

  //! @brief Parse a list of numbers separated by commas, semicolons or spaces. @param text list @param key key (for
  //! the messages) @return numbers @throws ConfigurationException if an entry is not a number
  static std::vector<double> parseNumbers(const String & text, const String & key);
  //!
  //! @brief A function of the function registry (a class with its default or given parameters, or a registered
  //! composite) of one variable. @param name name @param parameters parameter values (empty: the defaults)
  //! @param nameKey key of the name, @param parametersKey key of the parameters (for the messages)
  //! @return the function (owned by the caller) @throws ConfigurationException if there is no such function
  //!
  static MATH::ParameterizedFunction * makeFunction(const String & name, const String & parameters,
                                                    const String & nameKey, const String & parametersKey);

protected:

  //! @brief Read the settings of the generation and prepare the momentum distribution (no particle types).
  //! @throws ConfigurationException if a setting is invalid
  void readSettings();
  //! @brief Find the selected particle types (PdgSelected) in a particle database. @param particleDb database
  //! @throws ConfigurationException if no type is selected or a PDG code is not in the database
  void selectTypes(const ParticleDb & particleDb);
  //! @brief Draw a value of the momentum distribution in [low, high]. @param low lower edge @param high upper edge
  //! @return value
  double drawMomentum(double low, double high);
  //! @brief Tabulate the cumulative of the Gaussian or user distribution on [low, high], for drawMomentum().
  //! @param low lower edge @param high upper edge @throws ConfigurationException if the distribution is not positive
  void tabulate(double low, double high);
  //! @brief Density of the distribution at x (Gaussian or user function). @param x value @return density
  double density(double x) const;
  //! @brief Azimuth of a particle of transverse momentum pT (with the flow, if any). @param pt transverse momentum
  //! @return azimuth
  double drawPhi(double pt);
  //! @brief Reaction plane of the current event (flow). @return Psi
  double reactionPlane() const { return _psi; }
  //! @brief Read the multiplicity distribution and tabulate its cumulative (Mult:Distribution).
  //! @throws ConfigurationException if a setting is invalid, the histogram is missing or the distribution is zero
  void readMultiplicity();
  //! @brief Set the multiplicity table from values and their weights (cumulative of the positive weights).
  //! @param values multiplicities @param weights weights @param key key (for the message) @throws ConfigurationException
  //! if all weights are zero
  void setMultiplicityTable(const std::vector<long> & values, const std::vector<double> & weights, const String & key);
  //! @brief Number of particles of a type in an event. @return multiplicity
  long drawMultiplicity();
  //! @brief Momentum of one particle of a type, in the cylindrical style. @param m mass @param momentum momentum set
  void generateCylindrical(double m, MATH::VectorLorentz & momentum);
  //! @brief Momentum of one particle of a type, in the spherical style. @param m mass @param momentum momentum set
  void generateSpherical(double m, MATH::VectorLorentz & momentum);

  //! Style of generation.
  int    _eventGenType;
  //! PDG codes of the generated particle types.
  std::vector<int> _pdgSelected;
  //! Minimum number of particles of each type per event.
  int    _multMin;
  //! Maximum number of particles of each type per event.
  int    _multMax;
  //! Multiplicity distribution.
  int    _multDistribution;
  //! Multiplicities and cumulative of their probabilities (FUNCTION, HISTOGRAM).
  std::vector<long> _multValues;
  std::vector<double> _multCdf;
  //! Rapidity range (cylindrical).
  double _rapidityMin, _rapidityMax;
  //! Whether the cylindrical momenta are restricted in momentum (true) or in transverse momentum (false).
  bool   _useMomentumRange;
  //! Transverse momentum range (cylindrical).
  double _ptMin, _ptMax;
  //! Momentum range.
  double _pMin, _pMax;
  //! Polar angle range (spherical), radians.
  double _thetaMin, _thetaMax;
  //! Azimuth range, radians.
  double _phiMin, _phiMax;
  //! Momentum distribution.
  int    _distribution;
  //! Slope (mean) of the exponential distribution.
  double _slope;
  //! Width of the Gaussian distribution.
  double _sigma;
  //! User function (distribution "FUNCTION").
  std::shared_ptr<MATH::ParameterizedFunction> _function;
  //! Abscissae and cumulative of the tabulated distribution.
  std::vector<double> _tableX, _tableCdf;
  //! Whether the random seed is set.
  bool   _setSeed;
  //! Random seed.
  long   _seedValue;
  //! Random number generator.
  MATH::MersenneTwister _random;
  //! Masses of the generated types.
  std::vector<double> _mass;
  //! Generated types.
  std::vector<ParticleType*> _types;
  //! Flow coefficients v_1, v_2, ... (empty: no flow).
  std::vector<double> _vn;
  //! Transverse momentum of saturation of the flow coefficients (0: constant).
  double _flowPtScale;
  //! Reaction plane of the current event.
  double _psi;

};

}

#endif /* DATUM__BasicEventGen */
