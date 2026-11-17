/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetGroomingAnalyzer.hpp"

namespace Datum
{

JetGroomingAnalyzer::JetGroomingAnalyzer()
:
JetTask<JetGroomingHistos>(),
_managedGroomers()
{
  appendClassName("JetGroomingAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("JetGroomingAnalyzer");
  setTitle("JetGroomingAnalyzer");
}

JetGroomingAnalyzer::JetGroomingAnalyzer(const JetGroomingAnalyzer & source)
:
JetTask<JetGroomingHistos>(source),
_managedGroomers(source._managedGroomers)
{ }

JetGroomingAnalyzer & JetGroomingAnalyzer::operator=(const JetGroomingAnalyzer & rhs)
{
  if (this!=&rhs)
    {
    JetTask<JetGroomingHistos>::operator=(rhs);
    _managedGroomers = rhs._managedGroomers;
    }
  return *this;
}

void JetGroomingAnalyzer::setDefaultConfiguration()
{
  JetTask<JetGroomingHistos>::setDefaultConfiguration();
  _managedGroomers.setDefaultObjectConfiguration(name(),"JET_GROOMER",_configuration);
}

void JetGroomingAnalyzer::configureSelections(const Configuration & configuration)
{
  _managedGroomers.configureObjects(configuration);
  if (_managedGroomers.size()==0)
    throw ConfigurationException(createKey(name(),"JET_GROOMER","N"),"no jet groomer chosen",__FUNCTION__);
}

void JetGroomingAnalyzer::initializeSelections()
{
  _managedGroomers.initialize();
}

void JetGroomingAnalyzer::finalizeSelections()
{
  _managedGroomers.finalize();
}

void JetGroomingAnalyzer::fill(unsigned int first, const std::vector<Jet> & jets __attribute__((unused)),
                               const std::vector<const Jet*> & accepted,
                               const std::vector<Particle*> & particles __attribute__((unused)),
                               const JetSettings & settings __attribute__((unused)))
{
  const std::vector<JetGroomer*> & list = groomers();
  for (const Jet * jet : accepted)
    for (unsigned int x=0; x<list.size(); x++) _folders[first+x]->fill(list[x]->groom(*jet));
}

} // namespace Datum
