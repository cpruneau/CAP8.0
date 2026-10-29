/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#include "ParticlePairEventFilterHistos.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Configuration.hpp"

namespace CAP
{

ParticlePairEventFilterHistos::ParticlePairEventFilterHistos()
:
HistogramFolder(),
single1(),
single2(),
pair11(),
pair12(),
pair21(),
pair22(),
_name1(),
_name2()
{
  appendClassName("ParticlePairEventFilterHistos");
  setName("ParticlePairEventFilterHistos");
  setTitle("ParticlePairEventFilterHistos");
}

ParticlePairEventFilterHistos::ParticlePairEventFilterHistos(const ParticlePairEventFilterHistos & source)
:
HistogramFolder(source),
single1(source.single1),
single2(source.single2),
pair11(source.pair11),
pair12(source.pair12),
pair21(source.pair21),
pair22(source.pair22),
_name1(source._name1),
_name2(source._name2)
{   }

ParticlePairEventFilterHistos & ParticlePairEventFilterHistos::operator=(const ParticlePairEventFilterHistos & rhs)
{
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    single1 = rhs.single1;
    single2 = rhs.single2;
    pair11  = rhs.pair11;
    pair12  = rhs.pair12;
    pair21  = rhs.pair21;
    pair22  = rhs.pair22;
    _name1  = rhs._name1;
    _name2  = rhs._name2;
    }
  return *this;
}

std::vector<HistogramFolder*> ParticlePairEventFilterHistos::groups()
{
  return { &single1, &single2, &pair11, &pair12, &pair21, &pair22 };
}

std::vector<const HistogramFolder*> ParticlePairEventFilterHistos::groups() const
{
  return { &single1, &single2, &pair11, &pair12, &pair21, &pair22 };
}

void ParticlePairEventFilterHistos::configure(const String & taskName, const String & objectType,
                                              const Configuration & configuration, unsigned int index)
{
  HistogramFolder::configure(taskName,objectType,configuration,index);
  _name1 = configuration.valueString(createKey(taskName,"PARTICLE_FILTER",0,"NAME"));
  _name2 = configuration.valueString(createKey(taskName,"PARTICLE_FILTER",1,"NAME"));
  if (_name1==_name2)
    throw Exception(String("The two particle filters of a pair analysis must differ (both are ")+_name1+")",__FUNCTION__);
  for (HistogramFolder * group : groups()) group->configure(taskName,objectType,configuration,index);
  single1.setHistogramBaseName(_name1);
  single2.setHistogramBaseName(_name2);
  pair11.setHistogramBaseName(createName(_name1,_name1));
  pair12.setHistogramBaseName(createName(_name1,_name2));
  pair21.setHistogramBaseName(createName(_name2,_name1));
  pair22.setHistogramBaseName(createName(_name2,_name2));
}

void ParticlePairEventFilterHistos::create()
{
  for (HistogramFolder * group : groups()) group->create();
}

void ParticlePairEventFilterHistos::createDerived()
{
  single1.createDerived();
  single2.createDerived();
  for (ParticlePairHistos * pair : { &pair11, &pair12, &pair21, &pair22 }) pair->createDerived();
}

void ParticlePairEventFilterHistos::calculateDerived()
{
  if (!hasDerived()) throw Exception("The derived histograms were not created (createDerived())",__FUNCTION__);
  single1.calculateDerived();
  single2.calculateDerived();
  pair11.calculateDerived(single1,single1);
  pair12.calculateDerived(single1,single2);
  pair21.calculateDerived(single2,single1);
  pair22.calculateDerived(single2,single2);
}

void ParticlePairEventFilterHistos::loadFrom(const Folder & file)
{
  for (HistogramFolder * group : groups()) group->loadFrom(file);
}

void ParticlePairEventFilterHistos::saveTo(Folder & file) const
{
  for (const HistogramFolder * group : groups()) group->saveTo(file);
}

void ParticlePairEventFilterHistos::scaleHistograms(double factor)
{
  for (HistogramFolder * group : groups()) group->scaleHistograms(factor);
}

void ParticlePairEventFilterHistos::reset()
{
  for (HistogramFolder * group : groups()) group->reset();
}

} // namespace CAP
