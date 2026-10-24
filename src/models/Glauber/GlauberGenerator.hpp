#ifndef CAP__GlauberGenerator
#define CAP__GlauberGenerator
#include "GlauberDb.hpp"
#include "GlauberEvent.hpp"
#include "GlauberEventFilter.hpp"
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
//! Main keys (\<name\>:GENERATION:...): NUCLEUS_A:NAME, NUCLEUS_A:NTRIALS, NUCLEUS_A:RECENTERING,
//! NUCLEUS_A:ALLOW_OVERLAP, NUCLEUS_A:MIN_INTER (same for NUCLEUS_B), INTERACTION:NNCOLLISION_OPTION (1: profile
//! NNProfile; 2 and 3, fluctuating cross sections, are not written yet and throw NoImplementationException), INTERACTION:RANDOMIZE_EP, INTERACTION:SQRT_S_OPTION,
//! INTERACTION:DISTANCE_MAX, INTERACTION:HARD_DISTANCE_MAX; the event loop keys are \<name\>:EVENT:REQUESTED:N and
//! EVENT:REPORT:N.
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
  //! Maximum transverse distance of a hard interaction (fm).
  double _hardIntDistMax;

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

};

//  _smearingProfile = new SmearingProfile();
//  _smearingProfile->setNucleonSigma(nucleonSigma);
//  NucleonProfile  *_nucleonProfile;

  //  _nucleonProfile = new NucleonProfile();
//  _nucleonProfile->setNucleonRadius(nucleonRadius);




} // namespace CAP

#endif


