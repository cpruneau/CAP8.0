/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "EventProcessor.hpp"


namespace CAP
{
  EventProcessor::EventProcessor()
  :
  AnalysisTask(),
  _managedEvents(),
  _managedEventEfficiencies(),
  _managedEventSmearers(),
  _managedParticleDbs(),
  _managedParticleEfficiencies(),
  _managedParticleSmearers(),
  _acceptedEventFilters(),
  _acceptedParticleFilters(),
  _particleSlots(),
  _eventSlots(),
  _selections(),
  _ownSelections(),
  _ptMomentOrder(2),
  _eventAccepted(),
  _evtType()
  {
  appendClassName("EventProcessor");
  setMinimumReportLevel(Object::kInfo);
  setName("EventProcessor");
  setTitle("EventProcessor");
  }

  EventProcessor::EventProcessor(const EventProcessor & source)
  :
  AnalysisTask(source),
  _managedEvents(source._managedEvents),
  _managedEventEfficiencies(source._managedEventEfficiencies),
  _managedEventSmearers(source._managedEventSmearers),
  _managedParticleDbs(source._managedParticleDbs),
  _managedParticleEfficiencies(source._managedParticleEfficiencies),
  _managedParticleSmearers(source._managedParticleSmearers),
  _acceptedEventFilters(source._acceptedEventFilters),
  _acceptedParticleFilters(source._acceptedParticleFilters),
  _particleSlots(source._particleSlots),
  _eventSlots(source._eventSlots),
  _selections(),
  _ownSelections(),
  _ptMomentOrder(source._ptMomentOrder),
  _eventAccepted(source._eventAccepted),
  _evtType(source._evtType)
  {   }

  EventProcessor & EventProcessor::operator=(const EventProcessor & rhs)
  {
  if (this!=&rhs)
    {
    AnalysisTask::operator=(rhs);
    _managedEvents               = rhs._managedEvents;
    _managedEventEfficiencies    = rhs._managedEventEfficiencies;
    _managedEventSmearers        = rhs._managedEventSmearers;
    _managedParticleDbs          = rhs._managedParticleDbs;
    _managedParticleEfficiencies = rhs._managedParticleEfficiencies;
    _managedParticleSmearers     = rhs._managedParticleSmearers;
    _acceptedEventFilters        = rhs._acceptedEventFilters;
    _acceptedParticleFilters= rhs._acceptedParticleFilters;
    _particleSlots = rhs._particleSlots;
    _eventSlots    = rhs._eventSlots;
    _selections.clear();
    _ownSelections.clear();
    _ptMomentOrder = rhs._ptMomentOrder;
    _eventAccepted = rhs._eventAccepted;
    _evtType = rhs._evtType;
    }
  return *this;
  }

  void EventProcessor::setDefaultConfiguration()
  {
  Task::setDefaultConfiguration();
  const String & taskName = Task::name();
  _managedEvents.setDefaultObjectConfiguration(taskName,"EVENT", _configuration);
  _managedEventFilters.setDefaultObjectConfiguration(taskName,"EVENT_FILTER", _configuration);
  _managedEventEfficiencies.setDefaultObjectConfiguration(taskName,"EVENT_EFFICIENCY", _configuration);
  _managedEventSmearers.setDefaultObjectConfiguration(taskName,"EVENT_SMEARER", _configuration);
  _managedParticleDbs.setDefaultObjectConfiguration(taskName,"PARTICLE_DB", _configuration);
  _managedParticleFilters.setDefaultObjectConfiguration(taskName,"PARTICLE_FILTER", _configuration);
  _managedParticleEfficiencies.setDefaultObjectConfiguration(taskName,"PARTICLE_EFFICIENCY", _configuration);
  _managedParticleSmearers.setDefaultObjectConfiguration(taskName,"PARTICLE_SMEARER", _configuration);
  }

  void EventProcessor::configure(const Configuration & configuration)
  {
  if (reportDebug(__FUNCTION__)) { printCR();  }
  Task::configure(configuration);
  _managedEvents.configureObjects(configuration);
  _managedEventFilters.configureObjects(configuration);
  _managedEventEfficiencies.configureObjects(configuration);
  _managedEventSmearers.configureObjects(configuration);
  _managedParticleDbs.configureObjects(configuration);
  _managedParticleFilters.configureObjects(configuration);
  _managedParticleEfficiencies.configureObjects(configuration);
  _managedParticleSmearers.configureObjects(configuration);
  recordFilterNames();
  }

  void EventProcessor::initialize()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  _timer.start();
  _taskExecuted.initialize("taskExecuted",1);

  _managedEvents.initialize();
  _managedEventFilters.initialize();
  _managedEventEfficiencies.initialize();
  _managedEventSmearers.initialize();
  _managedParticleDbs.initialize();
  _managedParticleFilters.initialize();
  _managedParticleEfficiencies.initialize();
  _managedParticleSmearers.initialize();

  _eventAccepted.initialize("EventAccepted",nEventFilters());
  _acceptedEventFilters.assign(nEventFilters(),false);
  _acceptedParticleFilters.assign(nParticleFilters(),false);
  assignSlots();
  if (hasSubTasks()) initializeSubTasks();
  }

  void EventProcessor::finalize()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  if (hasSubTasks()) finalizeSubTasks();
 _managedEventFilters.finalize();
  _managedEventEfficiencies.finalize();
  _managedEventSmearers.finalize();
  _managedParticleDbs.finalize();
  _managedParticleFilters.finalize();
  _managedParticleEfficiencies.finalize();
  _managedParticleSmearers.finalize();
  _taskExecuted.clear();
  _eventAccepted.clear();
  }


  void EventProcessor::reset()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  _managedEventFilters.reset();
  _managedEventEfficiencies.reset();
  _managedEventSmearers.reset();
  _managedParticleFilters.reset();
  _managedParticleEfficiencies.reset();
  _managedParticleSmearers.reset();
  }

  void EventProcessor::clear()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  _managedEventFilters.clear();
  _managedEventEfficiencies.clear();
  _managedEventSmearers.clear();
  _managedParticleDbs.clear();
  _managedParticleFilters.clear();
  _managedParticleEfficiencies.clear();
  _managedParticleSmearers.clear();
  }

  void EventProcessor::print() const
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  _managedEventFilters.print();
  _managedEventEfficiencies.print();
  _managedEventSmearers.print();
  _managedParticleDbs.print();
  _managedParticleFilters.print();
  _managedParticleEfficiencies.print();
  _managedParticleSmearers.print();
  }

  const ParticleSelection & EventProcessor::selection(unsigned int index) const
  {
  if (index>=_selections.size() || !_selections[index])
    throw MemoryException("no selection for this particle filter (filterParticles() not called?)",__FUNCTION__);
  return *_selections[index];
  }

  const std::vector<Particle*> & EventProcessor::acceptedParticles(unsigned int index) const
  {
  return selection(index).particles();
  }

  std::vector<Particle*> & EventProcessor::acceptedParticles(unsigned int index)
  {
  return const_cast<ParticleSelection &>(selection(index)).particles();
  }

  std::vector<double> & EventProcessor::moments(unsigned int index)
  {
  return const_cast<ParticleSelection &>(selection(index)).ptMoments();
  }

  void EventProcessor::assignSlots()
  {
  std::vector<ParticleFilter*> & pf = particleFilters();
  std::vector<EventFilter*> & ef = eventFilters();
  _particleSlots.resize(pf.size());
  for (unsigned int k=0; k<pf.size(); k++) _particleSlots[k] = ParticleSelection::slot(pf[k], _ptMomentOrder);
  _eventSlots.resize(ef.size());
  for (unsigned int k=0; k<ef.size(); k++) _eventSlots[k] = ParticleSelection::slot(ef[k]);
  _selections.assign(pf.size(), nullptr);
  }

  double EventProcessor::getEvtClassValue() const
  {
  const Event & ev = event();
  switch (_evtType)
    {
      case EventFilter::kAll:    return ev.mult(0); break;
      case EventFilter::kMult0:  return ev.mult(0); break;
      case EventFilter::kMult1:  return ev.mult(1); break;
      case EventFilter::kMult2:  return ev.mult(2); break;
      case EventFilter::kMult3:  return ev.mult(3); break;
      case EventFilter::kMult4:  return ev.mult(4); break;
      case EventFilter::kMult5:  return ev.mult(5); break;
      case EventFilter::kEnergy0:  return ev.energy(0); break;
      case EventFilter::kEnergy1:  return ev.energy(1); break;
      case EventFilter::kEnergy2:  return ev.energy(2); break;
      case EventFilter::kEnergy3:  return ev.energy(3); break;
      case EventFilter::kEnergy4:  return ev.energy(4); break;
      case EventFilter::kEnergy5:  return ev.energy(5); break;
      case EventFilter::kSphero0:  return ev.spherocity(0); break;
      case EventFilter::kSphero1:  return ev.spherocity(1); break;
      case EventFilter::kSphero2:  return ev.spherocity(2); break;
      case EventFilter::kSphero3:  return ev.spherocity(3); break;
      case EventFilter::kSphero4:  return ev.spherocity(4); break;
      case EventFilter::kSphero5:  return ev.spherocity(5); break;
      default: throw Exception("Unknown evtClassType",__FUNCTION__);
    }
  return 0;
  }

  void EventProcessor::scaleHistograms()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  std::vector<double> factors;
  factors.clear();
  double f;
  unsigned int nef = nEventFilters();
  for (unsigned int iEventFilter=0; iEventFilter<nef; iEventFilter++)
    {
    long count = _eventAccepted.valueAt(iEventFilter);
    if (count>0)
      f = 1.0/double(count);
    else
      f = 1.0;
    factors.push_back(f);
    if (reportDebug(__FUNCTION__))
      {
      printCR();
      printValue("iEventFilter",iEventFilter);
      printValue("counts",count);
      printValue("scaling factor",f);
      }
    }
  scaleHistograms(factors);
  }

  void EventProcessor::execute()
  {
  throw NoImplementationException("EventProcessor",__FUNCTION__);
  }

  void EventProcessor::postProcess()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  calculateDerived();
  }

  void EventProcessor::calculateDerived()
  {
  throw NoImplementationException("EventProcessor",__FUNCTION__);
  }

  void EventProcessor::calculateBf()
  {
  throw NoImplementationException("EventProcessor",__FUNCTION__);
  }

  bool EventProcessor::filterEvent(Event & event)
  {
  bool analyzeThisEvent = false;
  std::vector<EventFilter*> & ef = eventFilters();
  if (_eventSlots.size()!=ef.size() || _acceptedEventFilters.size()!=ef.size()) assignSlots();
  if (_acceptedEventFilters.size()!=ef.size()) _acceptedEventFilters.assign(ef.size(),false);
  for (unsigned int iEventFilter=0; iEventFilter<ef.size();iEventFilter++)
    {
    bool accepted = event.accepted(*ef[iEventFilter],_eventSlots[iEventFilter]);
    _acceptedEventFilters[iEventFilter] = accepted;
    if (accepted)
      {
      _eventAccepted.increment(iEventFilter);
      analyzeThisEvent = true;
      }
    }
  return analyzeThisEvent;
  }

  bool EventProcessor::filterParticles(Event & event)
  {
  bool analyzeThisEvent = false;
  unsigned int npf = nParticleFilters();
  if (_particleSlots.size()!=npf || _selections.size()!=npf) assignSlots();
  if (_acceptedParticleFilters.size()!=npf) _acceptedParticleFilters.assign(npf,false);
  for (unsigned int iParticleFilter = 0;iParticleFilter<npf;iParticleFilter++)
    {
    ParticleSelection & s = event.selection(_particleSlots[iParticleFilter]);
    _selections[iParticleFilter] = &s;
    bool useThisFilter = !s.empty();     // at least one particle accepted
    _acceptedParticleFilters[iParticleFilter] = useThisFilter;
    if (useThisFilter) analyzeThisEvent = true;
    }
  return analyzeThisEvent;
  }

  bool EventProcessor::filterParticles(std::vector<Particle*> & particles)
  {
  bool analyzeThisEvent = false;
  std::vector<ParticleFilter*> & pf = particleFilters();
  if (_ownSelections.size()!=pf.size())
    {
    _ownSelections.clear();
    for (unsigned int k=0; k<pf.size(); k++) _ownSelections.emplace_back(new ParticleSelection(pf[k],k));
    }
  if (_selections.size()!=pf.size()) _selections.assign(pf.size(),nullptr);
  if (_acceptedParticleFilters.size()!=pf.size()) _acceptedParticleFilters.assign(pf.size(),false);
  for (unsigned int iParticleFilter = 0;iParticleFilter<pf.size();iParticleFilter++)
    {
    ParticleSelection & s = *_ownSelections[iParticleFilter];
    s.fill(particles,_ptMomentOrder);
    _selections[iParticleFilter] = &s;
    bool useThisFilter = !s.empty();
    _acceptedParticleFilters[iParticleFilter] = useThisFilter;
    if (useThisFilter) analyzeThisEvent = true;
    }
  return analyzeThisEvent;
  }

} // namespace CAP

