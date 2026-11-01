/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "AnalysisTask.hpp"

namespace CAP
{

  AnalysisTask::AnalysisTask()
  :
  Task(),
  _managedEventFilters(),
  _managedParticleFilters(),
  _eventFilterNames(),
  _particleFilterNames()
  {
  appendClassName("AnalysisTask");
  setMinimumReportLevel(Object::kInfo);
  setName("AnalysisTask");
  setTitle("AnalysisTask");
  }

  AnalysisTask::AnalysisTask(const AnalysisTask & source)
  :
  Task(source),
  _managedEventFilters(source._managedEventFilters),
  _managedParticleFilters(source._managedParticleFilters),
  _eventFilterNames(source._eventFilterNames),
  _particleFilterNames(source._particleFilterNames)
  {   }

  AnalysisTask & AnalysisTask::operator=(const AnalysisTask & rhs)
  {
  if (this!=&rhs)
    {
    Task::operator=(rhs);
    _managedEventFilters    = rhs._managedEventFilters;
    _managedParticleFilters = rhs._managedParticleFilters;
    _eventFilterNames       = rhs._eventFilterNames;
    _particleFilterNames    = rhs._particleFilterNames;
    }
  return *this;
  }

  void AnalysisTask::setDefaultConfiguration()
  {
  Task::setDefaultConfiguration();
  const String & taskName = Task::name();
  _managedEventFilters.setDefaultObjectConfiguration(taskName,"EVENT_FILTER", _configuration);
  _managedParticleFilters.setDefaultObjectConfiguration(taskName,"PARTICLE_FILTER", _configuration);
  }

  void AnalysisTask::configure(const Configuration & configuration)
  {
  if (reportDebug(__FUNCTION__)) { printCR();  }
  Task::configure(configuration);
  _managedEventFilters.configureObjects(configuration);
  _managedParticleFilters.configureObjects(configuration);
  recordFilterNames();
  }

  void AnalysisTask::recordFilterNames()
  {
  _eventFilterNames.clear();
  _particleFilterNames.clear();
  for (auto filter : eventFilters())    _eventFilterNames.push_back(filter->name());
  for (auto filter : particleFilters()) _particleFilterNames.push_back(filter->name());
  }

  void AnalysisTask::initialize()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  _timer.start();
  _taskExecuted.initialize("taskExecuted",1);
  _managedEventFilters.initialize();
  _managedParticleFilters.initialize();
  if (hasSubTasks()) initializeSubTasks();
  }

  void AnalysisTask::finalize()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  if (hasSubTasks()) finalizeSubTasks();
  _managedEventFilters.finalize();
  _managedParticleFilters.finalize();
  _taskExecuted.clear();
  }

  void AnalysisTask::reset()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  _managedEventFilters.reset();
  _managedParticleFilters.reset();
  }

  void AnalysisTask::clear()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  _managedEventFilters.clear();
  _managedParticleFilters.clear();
  }

  void AnalysisTask::print() const
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  _managedEventFilters.print();
  _managedParticleFilters.print();
  }

  String AnalysisTask::combinationName(unsigned int index, unsigned int nParticles) const
  {
  unsigned int nE = _eventFilterNames.size(), nP = _particleFilterNames.size(), nCombinations = 1;
  for (unsigned int i=0; i<nParticles; i++) nCombinations *= nP;
  if (nE==0 || nP==0 || index>=nE*nCombinations) return String("Folder_")+String(std::to_string(index).c_str());
  String name = _eventFilterNames[index/nCombinations];
  unsigned int rest = index%nCombinations;
  std::vector<unsigned int> particles(nParticles);
  for (unsigned int i=nParticles; i>0; i--) { particles[i-1] = rest%nP; rest /= nP; }
  for (unsigned int p : particles) { name += "_"; name += _particleFilterNames[p]; }
  return name;
  }

  void AnalysisTask::scaleHistograms(double factor __attribute__((unused)))
  {
  throw NoImplementationException("AnalysisTask",__FUNCTION__);
  }

  void AnalysisTask::scaleHistograms(const std::vector<double> & factors __attribute__((unused)))
  {
  throw NoImplementationException("AnalysisTask",__FUNCTION__);
  }

} // namespace CAP
