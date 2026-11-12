/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "ParticleFlowAnalyzer.hpp"
#include "Exceptions.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"

namespace CAP
{

ParticleFlowAnalyzer::ParticleFlowAnalyzer()
:
Base(),
_managedPoiFilters(),
_poiSlots()
{
  appendClassName("ParticleFlowAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("ParticleFlowAnalyzer");
  setTitle("ParticleFlowAnalyzer");
}

ParticleFlowAnalyzer::ParticleFlowAnalyzer(const ParticleFlowAnalyzer & source)
:
Base(source),
_managedPoiFilters(source._managedPoiFilters),
_poiSlots(source._poiSlots)
{ }

ParticleFlowAnalyzer & ParticleFlowAnalyzer::operator=(const ParticleFlowAnalyzer & rhs)
{
  if (this!=&rhs)
    {
    Base::operator=(rhs);
    _managedPoiFilters = rhs._managedPoiFilters;
    _poiSlots          = rhs._poiSlots;
    }
  return *this;
}

void ParticleFlowAnalyzer::setDefaultConfiguration()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  Base::setDefaultConfiguration();
  _managedPoiFilters.setDefaultObjectConfiguration(name(),"POI_FILTER",_configuration);
  ParticleFlowHistos::addDefaultConfiguration(_configuration,name());
}

void ParticleFlowAnalyzer::configure(const Configuration & configuration)
{
  Base::configure(configuration);
  _managedPoiFilters.configureObjects(configuration);
  std::vector<String> names;
  for (ParticleFilter * filter : _managedPoiFilters.getObjects()) names.push_back(filter->name());
  for (ParticleFlowHistos * folder : histograms()) folder->setPoiNames(names);
}

void ParticleFlowAnalyzer::initialize()
{
  _managedPoiFilters.initialize();
  Base::initialize();
  _poiSlots.clear();
  for (ParticleFilter * filter : _managedPoiFilters.getObjects())
    _poiSlots.push_back(ParticleSelection::slot(filter,ptOrder()));
}

void ParticleFlowAnalyzer::finalize()
{
  Base::finalize();
  _managedPoiFilters.finalize();
}

void ParticleFlowAnalyzer::execute()
{
  _taskExecuted.increment();
  Event & theEvent = event();
  if (!filterEvent(theEvent)) return;
  if (!filterParticles(theEvent)) return;
  const double evt = getEvtClassValue();
  const unsigned int nef = nEventFilters(), npf = nParticleFilters(), npoi = _poiSlots.size();
  std::vector<ParticleFlowHistos*> & folders = histograms();
  if (folders.size()<nCombinations()) throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  std::vector<const std::vector<Particle*>*> poi(npoi);
  for (unsigned int j=0; j<npoi; j++) poi[j] = &theEvent.selection(_poiSlots[j]).particles();
  std::vector<ParticleEfficiency*> & efficiencies = particleEfficiencies();
  std::vector<double> refWeights;
  std::vector<std::vector<double>> poiWeights(npoi);
  for (unsigned int e=0; e<nef; e++)
    {
    if (!_acceptedEventFilters[e]) continue;
    for (unsigned int r=0; r<npf; r++)
      {
      if (!_acceptedParticleFilters[r]) continue;
      const std::vector<Particle*> & ref = acceptedParticles(r);
      ParticleEfficiency * efficiency = objectFor(efficiencies,e,nef,r,npf,"particle efficiencies");
      refWeights.resize(ref.size());
      for (unsigned int i=0; i<ref.size(); i++) refWeights[i] = efficiency ? efficiency->weight(*ref[i]) : 1.0;
      for (unsigned int j=0; j<npoi; j++)
        {
        poiWeights[j].resize(poi[j]->size());
        for (unsigned int i=0; i<poi[j]->size(); i++)
          poiWeights[j][i] = efficiency ? efficiency->weight(*(*poi[j])[i]) : 1.0;
        }
      folders[e*npf+r]->fill(evt,ref,refWeights,poi,poiWeights);
      }
    }
}

} // namespace CAP
