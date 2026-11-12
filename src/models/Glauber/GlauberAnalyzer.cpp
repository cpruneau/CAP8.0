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
#include <algorithm>
#include "GlauberAnalyzer.hpp"
#include <initializer_list>
#include "GlauberHistos.hpp"
#include "MathBasicFunctions.hpp"
#include "NuclearDistribution.hpp"
#include "MersenneTwister.hpp"


namespace CAP
{

  using CAP::MATH::pi;
  using CAP::MATH::twoPi;


  GlauberAnalyzer::GlauberAnalyzer()
  :
  Task(),
  _managedEvents(),
  _managedEventFilters(),
  _managedHistograms(),
  _acceptedEventFilters(),
  _eventAccepted(),
  _useRadialProfile(false)
  {
  appendClassName("GlauberAnalyzer");
  setName("GlauberAnalyzer");
  setTitle("GlauberAnalyzer");
  }

  GlauberAnalyzer::GlauberAnalyzer(const GlauberAnalyzer & src)
  :
  Task(src),
  _managedEvents(src._managedEvents),
  _managedEventFilters(src._managedEventFilters),
  _managedHistograms(src._managedHistograms),
  _eventFilterNames(src._eventFilterNames),
  _acceptedEventFilters(src._acceptedEventFilters),
  _eventAccepted(src._eventAccepted),
  _useRadialProfile(src._useRadialProfile)
  {   }

  GlauberAnalyzer & GlauberAnalyzer::operator=(const GlauberAnalyzer & rhs)
  {
  if (this!=&rhs)
    {
    Task::operator=(rhs);
    _managedEvents = rhs._managedEvents;
    _managedEventFilters = rhs._managedEventFilters;
    _managedHistograms = rhs._managedHistograms;
    _eventFilterNames = rhs._eventFilterNames;
    _acceptedEventFilters = rhs._acceptedEventFilters;
    _eventAccepted = rhs._eventAccepted;
    _useRadialProfile = rhs._useRadialProfile;
    }
  return *this;
  }

  void GlauberAnalyzer::setDefaultConfiguration()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  Task::setDefaultConfiguration();
  const String & taskName = Task::name();
  // the defaults of the lists first: the values below (EVENT:USE, EVENT:N, ...) replace theirs
  _managedEvents.setDefaultObjectConfiguration(taskName,"EVENT", _configuration);
  _managedEventFilters.setDefaultObjectConfiguration(taskName,"EVENT_FILTER", _configuration);

  _configuration.addProperty(createKey(taskName,"SEVERITY"),         "TRACE");
  _configuration.addProperty(createKey(taskName,"SUBTASK:N"),        0);
  _configuration.addProperty(createKey(taskName,"EVENT:USE"),        true);
  _configuration.addProperty(createKey(taskName,"EVENT:REQUESTED:N"),1000);
  _configuration.addProperty(createKey(taskName,"EVENT:REPORT:N"),   100);
  _configuration.addProperty(createKey(taskName,"EVENT:N"),          1);
  _configuration.addProperty(createKey(taskName,"EVENT_0:NAME"),     "Glauber");
  _configuration.addProperty(createKey(taskName,"EVENT_0:OWNER"),    false);

  String objectType = "HISTOGRAM";
  _configuration.addProperty(createKey(taskName,objectType,"fillBasics"),  true);
  _configuration.addProperty(createKey(taskName,objectType,"fillMoments"), false);
  _configuration.addProperty(createKey(taskName,objectType,"fillSmeared"), false);
  _configuration.addProperty(createKey(taskName,objectType,"fillArea"),    false);
  _configuration.addProperty(createKey(taskName,objectType,"drawEvents"),  false);
  _configuration.addProperty(createKey(taskName,objectType,"drawEvents:N"),20);
  _configuration.addProperty(createKey(taskName,objectType,"nOrders"),4);

  // binning of the histograms (the keys GlauberHistos::configure() reads): <variable>_nBins, _min, _max
  _configuration.addProperty(createKey(taskName,objectType,"nParts_nBins"),100);
  _configuration.addProperty(createKey(taskName,objectType,"nParts_min"),0.0);
  _configuration.addProperty(createKey(taskName,objectType,"nParts_max"),400.0);
  _configuration.addProperty(createKey(taskName,objectType,"nBinaries_nBins"),100);
  _configuration.addProperty(createKey(taskName,objectType,"nBinaries_min"),0.0);
  _configuration.addProperty(createKey(taskName,objectType,"nBinaries_max"),1000.0);
  _configuration.addProperty(createKey(taskName,objectType,"nSpect_nBins"),100);
  _configuration.addProperty(createKey(taskName,objectType,"nSpect_min"),0.0);
  _configuration.addProperty(createKey(taskName,objectType,"nSpect_max"),400.0);
  _configuration.addProperty(createKey(taskName,objectType,"impact_nBins"),50);
  _configuration.addProperty(createKey(taskName,objectType,"impact_min"),0.0);
  _configuration.addProperty(createKey(taskName,objectType,"impact_max"),14.0);
  _configuration.addProperty(createKey(taskName,objectType,"cs_nBins"),20);
  _configuration.addProperty(createKey(taskName,objectType,"cs_min"),-1.0);
  _configuration.addProperty(createKey(taskName,objectType,"cs_max"),1.0);
  _configuration.addProperty(createKey(taskName,objectType,"psi_nBins"),20);
  _configuration.addProperty(createKey(taskName,objectType,"psi_min"),0.0);
  _configuration.addProperty(createKey(taskName,objectType,"psi_max"),pi());
  _configuration.addProperty(createKey(taskName,objectType,"ecc_nBins"),20);
  _configuration.addProperty(createKey(taskName,objectType,"ecc_min"),-1.0);
  _configuration.addProperty(createKey(taskName,objectType,"ecc_max"),1.0);
  _configuration.addProperty(createKey(taskName,objectType,"rn_nBins"),20);
  _configuration.addProperty(createKey(taskName,objectType,"rn_min"),0.0);
  _configuration.addProperty(createKey(taskName,objectType,"rn_max"),12.0);
  _configuration.addProperty(createKey(taskName,objectType,"psiN_nBins"),20);
  _configuration.addProperty(createKey(taskName,objectType,"psiN_min"),0.0);
  _configuration.addProperty(createKey(taskName,objectType,"psiN_max"),twoPi());
  _configuration.addProperty(createKey(taskName,objectType,"eccN_nBins"),20);
  _configuration.addProperty(createKey(taskName,objectType,"eccN_min"),0.0);
  _configuration.addProperty(createKey(taskName,objectType,"eccN_max"),1.0);
  _configuration.addProperty(createKey(taskName,objectType,"area_nBins"),50);
  _configuration.addProperty(createKey(taskName,objectType,"area_min"),0.0);
  _configuration.addProperty(createKey(taskName,objectType,"area_max"),14.0);
  // for RunCAP: one folder of histograms per event filter, no particle filter (not read here)
  _configuration.addProperty(createKey(taskName,objectType,"FOLDER_RANK"),0);

  _managedHistograms.setDefaultObjectConfiguration(taskName,objectType, _configuration);


  //  TF1 *radialProfile = new TF1("radialProfile","x*TMath::Exp(-x*x/(2.*[0]*[0]))",0.0,3.0*sigma);
  //  radialProfile->SetParameter(0,sigma);
  }

  namespace
  {
  //! Folder of histogram group k: named after event filter k.
  String filterFolderName(const std::vector<String> & names, unsigned int k)
  {
  return k<names.size() ? names[k] : String("Folder_")+String(std::to_string(k).c_str());
  }
  }

  void GlauberAnalyzer::configure(const Configuration & configuration)
  {
  if (reportDebug(__FUNCTION__)) { printCR();  }
  Task::configure(configuration);
  _managedEvents.configureObjects(configuration);
  _managedEventFilters.configureObjects(configuration);
  _managedHistograms.configureObjects(configuration);
  _eventFilterNames.clear();
  for (auto filter : eventFilters()) _eventFilterNames.push_back(filter->name());
  // one group of histograms per event filter (one if there is none): the groups the configuration does not list
  // (HISTOGRAM:N smaller, e.g. 0 when written without filters) are made here, configured as those it lists
  if (configuration.valueBool(createKey(name(),"HISTOGRAM","USE"), true, true))
    {
    unsigned int needed = std::max(1u, (unsigned int) _eventFilterNames.size());
    for (unsigned int k=_managedHistograms.size(); k<needed; k++)
      {
      GlauberHistos * histos = _managedHistograms.instantiateObject(name()+String("_")+filterFolderName(_eventFilterNames,k));
      histos->configure(name(),"HISTOGRAM",configuration,k);
      }
    }
  }

  void GlauberAnalyzer::initialize()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  _timer.start();
  _taskExecuted.initialize("taskExecuted",1);
  _managedEvents.initialize();
  _managedEventFilters.initialize();
  _managedHistograms.initialize();
  GlauberEvent & ev = event();
  GlauberEventMoments & momentsA      = ev.momentsNucleusA();
  GlauberEventMoments & momentsB      = ev.momentsNucleusB();
  GlauberEventMoments & momentsSystem = ev.momentsSystem();
  momentsA.initialize();
  momentsB.initialize();
  momentsSystem.initialize();

  int nef = nEventFilters();
  if (nef<1) nef = 1;
  _eventAccepted.initialize("EventAccepted",nef);
  _acceptedEventFilters.assign(nef,false);
  }

  void GlauberAnalyzer::saveHistograms(Folder & top)
  {
  if (_managedHistograms.size()>0)
    _managedHistograms.saveToFolders(*top.addFolder(name()),[this](unsigned int k) { return filterFolderName(_eventFilterNames,k); });
  Task::saveHistograms(top);
  }

  void GlauberAnalyzer::loadHistograms(const Folder & top)
  {
  if (_managedHistograms.imports() && _managedHistograms.size()>0)
    {
    Folder * folder = top.folder(name());
    if (!folder) throw Exception(String("No folder ")+name()+" in the input file",__FUNCTION__);
    _managedHistograms.loadFromFolders(*folder,[this](unsigned int k) { return filterFolderName(_eventFilterNames,k); });
    }
  Task::loadHistograms(top);
  }

  void GlauberAnalyzer::finalize()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  scaleHistograms();
  _managedEvents.finalize();
  _managedEventFilters.finalize();
  _managedHistograms.finalize();
  _taskExecuted.clear();
  _eventAccepted.clear();
  }

  void GlauberAnalyzer::reset()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  _managedEvents.reset();
  _managedEventFilters.reset();
  _managedHistograms.reset();
  _taskExecuted.reset();
  _eventAccepted.reset();
  }

  void GlauberAnalyzer::clear()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  _managedEvents.clear();
  _managedEventFilters.clear();
  _managedHistograms.clear();
  _taskExecuted.clear();
  _eventAccepted.clear();
  }

  void GlauberAnalyzer::print() const
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  _managedEvents.print();
  _managedEventFilters.print();
  _managedHistograms.print();
  _taskExecuted.print();
  _eventAccepted.print();
  }

  void GlauberAnalyzer::fillMoments(std::vector<GlauberNucleon> & nucleonsA,
                                    std::vector<GlauberNucleon> & nucleonsB,
                                    GlauberEventMoments & moments)
  {
  // the positions of the wounded nucleons (smeared once, when a radial profile is used), then the two passes over them
  _positions.clear();
  for (auto * nucleons : {&nucleonsA, &nucleonsB})
    for (auto & nucleon : *nucleons)
      {
      if (!nucleon.isWounded()) continue;
      const VectorLorentz & position = nucleon.position();
      double x = position.x();
      double y = position.y();
      if (_useRadialProfile)
        {
        double smearRadius = 0.5;// fix me radialProfile->GetRandom();
        double smearPhi    = twoPi() * CAP::MATH::randomGenerator().random();
        x += smearRadius*std::cos(smearPhi);
        y += smearRadius*std::sin(smearPhi);
        }
      _positions.push_back(x);
      _positions.push_back(y);
      }
  moments.resetPass();
  for (int pass=0; pass<2; pass++)
    {
    moments.reset();
    for (size_t k=0; k+1<_positions.size(); k+=2) moments.fill(_positions[k],_positions[k+1]);
    moments.calculateAverages();
    }
  }

  void GlauberAnalyzer::execute()
  {
  //if (reportInfo(__FUNCTION__)) { printCR();}
  GlauberEvent & ev = event();
  GlauberNucleus & nucleusA = ev.nucleusA();
  GlauberNucleus & nucleusB = ev.nucleusB();
  std::vector<GlauberNucleon> & nucleonsA = nucleusA.allNucleons();
  std::vector<GlauberNucleon> & nucleonsB = nucleusB.allNucleons();
  GlauberEventMoments & momentsSystem = ev.momentsSystem();
  fillMoments(nucleonsA,nucleonsB,momentsSystem);

  std::vector<GlauberEventFilter*> & filters = eventFilters();
  std::vector<GlauberHistos*> & glauberHistos = histograms();
  if (momentsSystem._sum>0) // fill histos only for events having wounded nucleons.
    {
    if (filters.size()==0)
      {
      if (!glauberHistos.empty()) glauberHistos[0]->fill(ev,1.0);
      _eventAccepted.increment(0);
      }
    else
      {
      unsigned int iEventFilter = 0;
      for (auto & filter : filters)
        {
        if (filter->accept(ev))
          {
          if (iEventFilter<glauberHistos.size()) glauberHistos[iEventFilter]->fill(ev,1.0);
          _eventAccepted.increment(iEventFilter);
          }
        iEventFilter++;
        }
      }
    }
  incrementTaskExecuted();
  }

  //!
  //! Scale (all) the histograms held in all the histogram groups owned by this Task instance. This operation is executed automatically within the finalize() method call if
  //! and only if the HistogramsScale parameter of the Configuration instance controlling this Task is set to "true".
  //!
  void GlauberAnalyzer::scaleHistograms()
  {
  if (reportInfo(__FUNCTION__)) { printCR(); };

  std::vector<long> & counts = _eventAccepted.values();
  std::vector<double> factors;
  factors.clear();
  for (auto count : counts)
    {
    if (count>0)
      {
      factors.push_back(1.0/double(count));
      }
    else
      factors.push_back(1.0);
    }

  scaleHistograms(factors);
  if (reportEnd(__FUNCTION__)) { /* no ops */ };
  }

  void GlauberAnalyzer::scaleHistograms(const std::vector<double> & factors)
  {
  std::vector<GlauberHistos*> & histos = histograms();

  for (unsigned int k=0; k<histos.size() && k<factors.size(); k++)
    {
    if (reportInfo(__FUNCTION__)) { printValue("k",k); printValue("scale",factors[k]); }
    histos[k]->scaleHistograms(factors[k]);
    }
  }
  


} // namespace CAP
