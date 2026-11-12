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
#ifndef CAP__GlauberGenerator
#define CAP__GlauberGenerator
#include "GlauberDb.hpp"
#include "GlauberEvent.hpp"
#include "GlauberEventFilter.hpp"
#include "NNProfile.hpp"
#include "NNXSectVsSqrtS.hpp"
#include "PTotProfile.hpp"
#include "ManagedObjects.hpp"
#include "Task.hpp"

namespace CAP
{
using namespace std;

//!
//! @brief Monte Carlo Glauber event generator: for each event, places the nucleons of the two nuclei, draws the impact
//! parameter (uniform in b^2 between the minimum and maximum), optionally rotates the event plane, and determines the
//! nucleon-nucleon collisions from their transverse distances and the nucleon-nucleon collision profile.
//!
//! @image html GlauberClasses.png "Classes of the Glauber model (models/Glauber)" width=100%
//! @image latex GlauberClasses.pdf "Classes of the Glauber model (models/Glauber)" width=\textwidth
//!
//! Main keys (\<name\>:GENERATION:...), with the defaults of TGlauberMC v3.2:
//!  - NUCLEUS_A:NAME (same keys for NUCLEUS_B), NTRIALS (1000), RECENTERING (true), RECENTERING_MODE (1: see
//!    GlauberNucleus::recenterNucleus()), ALLOW_OVERLAP (false), MIN_INTER (hard core, 0.4 fm), LATTICE (false),
//!    LATTICE_TYPE (0), NODE_DISTANCE (fm), SMEARING (0), MAX_CENTER_SHIFT (not positive: 0.1 fm for the reweighted
//!    types, 99 otherwise);
//!  - INTERACTION:xSECT_OPTION (1: sigma from SQRT_S with SQRT_S_OPTION, see NNXSectVsSqrtS; 2: xSECTION, mb);
//!  - INTERACTION:NNCOLLISION_OPTION (1: fixed cross section; 2: cross section drawn for each event; 3: drawn for each
//!    nucleon; 2 and 3 from the distribution of TGlauberMC with width xSECTION_SIGMA, see PTotProfile);
//!  - INTERACTION:xSECTION_OMEGA (0: hard sphere; 0 < omega <= 1: gamma profile, see NNProfile), xSECTION_G;
//!  - INTERACTION:IMPACT_MIN, IMPACT_MAX, RANDOMIZE_EP, DISTANCE_MAX (largest transverse distance tested, 4 fm),
//!    HARD_DISTANCE_MAX (not positive: sqrt(HARD_FRACTION sigma/(10 pi)), HARD_FRACTION 0.65).
//! The event loop keys are \<name\>:EVENT:REQUESTED:N and EVENT:REPORT:N.
//!
class GlauberGenerator
  :
  public Task
  {
   public:

  //! @brief Default constructor.
  GlauberGenerator();
  //! @brief Copy constructor. @param src object to copy
  GlauberGenerator(const GlauberGenerator & src);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  GlauberGenerator & operator=(const GlauberGenerator & rhs);
  //! @brief Destructor.
  virtual ~GlauberGenerator() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Configure the task. @param configuration configuration
  virtual void configure(const Configuration & configuration);
  //! @brief Initialize the task.
  virtual void initialize();
  //! @brief Execute the task (process the current event).
  virtual void execute();
  //! @brief Finalize the task.
  virtual void finalize();
  //! @brief Reset the task and its objects.
  virtual void reset();
  //! @brief Clear the task and its objects.
  virtual void clear();
  //! @brief Print the object.
  virtual void print() const;
  //! @brief Number of events. @return number
  unsigned int nEvents() const  { return _managedEvents.size();  }
  //! @brief Number of event filters. @return number
  unsigned int nEventFilters() const  { return _managedEventFilters.size();  }
  //! @brief Number of nucleus databases. @return number
  unsigned int nDbs() const  { return _managedDb.size();  }
  //! @brief First nucleus database. @return database
  const GlauberDb & db() const;
  //! @brief Nucleus databases. @return databases
  const std::vector<GlauberDb*>   & dbs() const;
  //! @brief Current event. @return event
  const GlauberEvent & event() const;
  //! @brief Events. @return events
  const std::vector<GlauberEvent*> & events() const;
  //! @brief Event filters. @return filters
  const std::vector<GlauberEventFilter*> & eventFilters() const;
  //! @brief First nucleus database. @return database
  GlauberDb & db();
  //! @brief Nucleus databases. @return databases
  std::vector<GlauberDb*> & dbs();
  //! @brief Current event. @return event
  GlauberEvent & event();
  //! @brief Events. @return events
  std::vector<GlauberEvent*> & events();

  //! @brief Parameterization of the nucleon-nucleon cross section vs sqrt(s). @return parameterization
  NNXSectVsSqrtS  * nnXSectVsSqrt()   { return _nnXSectVsSqrtS;  }
  //! @brief Nucleon-nucleon collision profile. @return profile
  NNProfile       * nnProfile()       { return _nnXSectProfile;  }
  //SmearingProfile * smearingProfile() { return _smearingProfile; }
  //! @brief Distribution of the fluctuating nucleon-nucleon cross section. @return distribution
  PTotProfile     * ptotProfile()     { return _ptotProfile;     }
  //NucleonProfile  * nucleonProfile()  { return _nucleonProfile;  }

  protected:

  //! Nucleus databases.
  ManagedObjects<GlauberDb>          _managedDb;
  //! Events.
  ManagedObjects<GlauberEvent>       _managedEvents;
  //! Event filters.
  ManagedObjects<GlauberEventFilter> _managedEventFilters;

  //! Nucleon-nucleon collision option (see the class description).
  int    _nnCollisionOption;
  //! Whether the event plane is rotated randomly.
  bool   _randomizeEP;
  //! Cross section option.
  int    _xSectOption;
  //! Option of the cross section vs sqrt(s) parameterization.
  int    _xSectVsSOption;
  //! Nucleon-nucleon collision energy sqrt(s) (GeV).
  double _sqrtS;
  //! Nucleon-nucleon inelastic cross section (mb).
  double _xSection;
  //! Normalized variance of the cross section fluctuations.
  double _xSectionOmega;
  //! Normalization of the collision profile.
  double _xSectionG;
  //! Width of the cross section distribution.
  double _xSectionSigma;
  //! Minimum impact parameter (fm).
  double _minImpact;
  //! Maximum impact parameter (fm).
  double _maxImpact;
  //! Maximum transverse distance of an interaction (fm).
  double _intDistanceMax;
  //! Maximum transverse distance of a hard interaction (fm); not positive: from _hardFraction.
  double _hardIntDistMax;
  //! Fraction of the cross section whose disk makes hard collisions (TGlauberMC fHardFrac, default 0.65).
  double _hardFraction;

  //! Squares of the above.
  double _minImpactSq;
  double _maxImpactSq;
  double _intDistanceMaxSq;
  double _hardIntDistMaxSq;

  //! Cross section vs sqrt(s) parameterization (owned).
  NNXSectVsSqrtS  *_nnXSectVsSqrtS;
  //! Nucleon-nucleon collision profile (owned).
  NNProfile       *_nnXSectProfile;
  //! Fluctuating cross section distribution (owned).
  PTotProfile     *_ptotProfile;
  //! Collision profile of the current event (NNCOLLISION_OPTION 2).
  NNProfile        _eventProfile;

};

//  _smearingProfile = new SmearingProfile();
//  _smearingProfile->setNucleonSigma(nucleonSigma);
//  NucleonProfile  *_nucleonProfile;

  //  _nucleonProfile = new NucleonProfile();
//  _nucleonProfile->setNucleonRadius(nucleonRadius);




} // namespace CAP

#endif


