#ifndef CAP__GlauberAnalyzer
#define CAP__GlauberAnalyzer
#include "Accountant.hpp"
#include "Task.hpp"
#include "GlauberDb.hpp"
#include "GlauberEvent.hpp"
#include "GlauberEventFilter.hpp"
#include "GlauberHistos.hpp"

namespace CAP
{

  //!
  //! @brief Task analyzing Glauber events: computes the participant moments of each event (two passes) and fills a
  //! GlauberHistos for each event filter that accepts it (events with wounded nucleons only).
  //!
  class GlauberAnalyzer
  :
  public Task
  {
  protected:

  //! Events (the first one is analyzed).
  ManagedObjects<GlauberEvent>         _managedEvents;
  //! Event filters.
  ManagedObjects<GlauberEventFilter>   _managedEventFilters;
  //! Histogram groups, one per event filter.
  ManagedObjects<GlauberHistos>        _managedHistograms;
  //! Names of the event filters, kept from the configuration for the folders of the histograms (see saveHistograms()).
  std::vector<String> _eventFilterNames;   //!
  //! Whether each event filter accepted the current event.
  std::vector<bool>                    _acceptedEventFilters;
  //! Number of events accepted by each event filter.
  Accountant _eventAccepted;
  //! Whether the radial profile is used.
  bool _useRadialProfile;
  //bool _fillEvents;
  //radialProfile;

  public:

  //! @brief Default constructor.
  GlauberAnalyzer();
  //! @brief Copy constructor. @param analyzer object to copy
  GlauberAnalyzer(const GlauberAnalyzer & analyzer);
  //! @brief Copy assignment. @param analyzer object to copy @return this object
  GlauberAnalyzer & operator=(const GlauberAnalyzer & analyzer);
  //! @brief Destructor.
  virtual ~GlauberAnalyzer() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration() override;
  //! @brief Configure the task. @param configuration configuration
  virtual void configure(const Configuration & configuration) override;
  //! @brief Initialize the task.
  virtual void initialize() override;
  //! @brief Execute the task (process the current event).
  virtual void execute() override;
  //! @brief Finalize the task.
  virtual void finalize() override;
  //! @brief Add the histograms to the folder of the job: a folder named after the task, holding a folder per event
  //! filter. @param top folder of the job
  void saveHistograms(Folder & top) override;
  //! @brief Read the histograms imported (IMPORT) from the folder of the job (same folders as saveHistograms()).
  //! @param top folder of the job @throws Exception if a folder is missing
  void loadHistograms(const Folder & top) override;
  //! @brief Reset the task and its objects.
  virtual void reset() override;
  //! @brief Clear the task and its objects.
  virtual void clear() override;
  //! @brief Print the object.
  virtual void print() const override;
  //! @brief Add the wounded nucleons of both nuclei to the moments. @param nucleonsA nucleons of nucleus A
  //! @param nucleonsB nucleons of nucleus B @param moments moments filled
  virtual void fillMoments(std::vector<GlauberNucleon> & nucleonsA,
                           std::vector<GlauberNucleon> & nucleonsB,
                           GlauberEventMoments & moments);
  //! @brief Number of events. @return number
  unsigned int nEvents() const  { return _managedEvents.size();  }
  //! @brief Number of event filters. @return number
  unsigned int nEventFilters() const  { return _managedEventFilters.size();  }
  //! @brief Number of histogram groups. @return number
  unsigned int nHistograms() const  { return _managedHistograms.size();  }
  //! @brief Current event. @return event
  const GlauberEvent & event() const                                  { return *_managedEvents.objectAt(0); }
  //! @brief Events. @return events
  const std::vector<GlauberEvent*> & events() const                   { return _managedEvents.getObjects(); }
  //! @brief Event filters. @return filters
  const std::vector<GlauberEventFilter*> & eventFilters() const       { return _managedEventFilters.getObjects(); }
  //! @brief Histogram groups. @return groups
  const std::vector<GlauberHistos*> & histograms() const              { return _managedHistograms.getObjects(); }
  //! @brief Current event. @return event
  GlauberEvent & event()                                              { return *_managedEvents.objectAt(0); }
  //! @brief Events. @return events
  std::vector<GlauberEvent*> & events()                               { return _managedEvents.getObjects(); }
  //! @brief Event filters. @return filters
  std::vector<GlauberEventFilter*> & eventFilters()                   { return _managedEventFilters.getObjects(); }
  //! @brief Histogram groups. @return groups
  std::vector<GlauberHistos*> & histograms()                          { return _managedHistograms.getObjects(); }
  //! @brief Whether each event filter accepted the current event. @return flags
  const std::vector<bool> & acceptedEventFilters() const { return _acceptedEventFilters; }
  //! @brief Number of events accepted by each event filter. @return counter
  Accountant & eventAccepted() { return _eventAccepted; }

  //! @brief Divide the histograms of each event filter by its number of accepted events.
  virtual void scaleHistograms();
  //! @brief Scale the histograms of each event filter by its factor. @param factors factors
  virtual void scaleHistograms(const std::vector<double> & factors);

//  double calculateDensity(NuclearDistribution & dist, double x, double y);
//  void   calculateArea(double xSectionEvent,
//                       const GlauberEventMoments & momentsSystem,
//                       double & areaOverlap,
//                       double & areaAll);
//  double calculatelength();

  };

} // namespace CAP

#endif // !CAP__GlauberAnalyzer
