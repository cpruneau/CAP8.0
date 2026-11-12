/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetPairAnalyzer.hpp"

namespace CAP
{

JetPairAnalyzer::JetPairAnalyzer()
:
JetTask<JetPairHistos>(),
_managedPairFilters(),
_pairs(),
_selected()
{
  appendClassName("JetPairAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("JetPairAnalyzer");
  setTitle("JetPairAnalyzer");
}

JetPairAnalyzer::JetPairAnalyzer(const JetPairAnalyzer & source)
:
JetTask<JetPairHistos>(source),
_managedPairFilters(source._managedPairFilters),
_pairs(),
_selected()
{ }

JetPairAnalyzer & JetPairAnalyzer::operator=(const JetPairAnalyzer & rhs)
{
  if (this!=&rhs)
    {
    JetTask<JetPairHistos>::operator=(rhs);
    _managedPairFilters = rhs._managedPairFilters;
    }
  return *this;
}

void JetPairAnalyzer::setDefaultConfiguration()
{
  JetTask<JetPairHistos>::setDefaultConfiguration();
  _managedPairFilters.setDefaultObjectConfiguration(name(),"JET_PAIR_FILTER",_configuration);
}

void JetPairAnalyzer::configureSelections(const Configuration & configuration)
{
  _managedPairFilters.configureObjects(configuration);
}

void JetPairAnalyzer::initializeSelections()
{
  _managedPairFilters.initialize();
}

void JetPairAnalyzer::finalizeSelections()
{
  _managedPairFilters.finalize();
}

void JetPairAnalyzer::fill(unsigned int first, const std::vector<Jet> & jets __attribute__((unused)),
                           const std::vector<const Jet*> & accepted,
                           const std::vector<Particle*> & particles __attribute__((unused)),
                           const JetSettings & settings __attribute__((unused)))
{
  _pairs.clear();
  for (unsigned int a=0; a<accepted.size(); a++)
    for (unsigned int b=a+1; b<accepted.size(); b++) _pairs.emplace_back(accepted[a],accepted[b]);
  const std::vector<JetPairFilter*> & filters = pairFilters();
  for (unsigned int x=0; x<filters.size(); x++)
    {
    _selected.clear();
    for (const JetPair & pair : _pairs) if (filters[x]->accept(pair)) _selected.push_back(pair);
    _folders[first+x]->fill(_selected);
    }
}

} // namespace CAP
