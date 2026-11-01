/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "ParticlePair3DBFHistos.hpp"
#include "Exceptions.hpp"
#include "NameHelpers.hpp"

namespace CAP
{

  ParticlePair3DBFHistos::ParticlePair3DBFHistos(const String & name)
  :
  HistogramFolder(name)
  {
  appendClassName("ParticlePair3DBFHistos");
  for (int v=0; v<kNVariables; v++)
    for (int q=0; q<kNQuantities; q++) _h[v][q] = nullptr;
  }

  ParticlePair3DBFHistos::ParticlePair3DBFHistos(const ParticlePair3DBFHistos & source)
  :
  HistogramFolder(source)
  {
  relink();
  }

  ParticlePair3DBFHistos & ParticlePair3DBFHistos::operator=(const ParticlePair3DBFHistos & rhs)
  {
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    relink();
    }
  return *this;
  }

  const char * ParticlePair3DBFHistos::variableName(Variables v)
  {
  static const char * names[kNVariables] = { "Qinv", "DeltaP", "DeltaPs", "DeltaPo", "DeltaPl" };
  return names[v];
  }

  const char * ParticlePair3DBFHistos::quantityName(Quantities q)
  {
  static const char * names[kNQuantities] = { "R2CI", "R2CD", "B_a_bB", "B_aB_b", "B" };
  return names[q];
  }

  void ParticlePair3DBFHistos::relink()
  {
  for (int v=0; v<kNVariables; v++)
    for (int q=0; q<kNQuantities; q++)
      _h[v][q] = histogram(String(quantityName(Quantities(q))) + "_" + variableName(Variables(v)));
  }

  const Histogram * ParticlePair3DBFHistos::of(const ParticlePair3DHistos & pair, const char * prefix, Variables v)
  {
  return pair.histogram(createName(pair.histogramBaseName(), String(prefix) + "_" + variableName(v)));
  }

  void ParticlePair3DBFHistos::createFrom(const ParticlePair3DHistos & pair)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  clear();
  for (int v=0; v<kNVariables; v++)
    for (int q=0; q<kNQuantities; q++)
      {
      _h[v][q] = nullptr;
      const Histogram * r2 = of(pair,"r2",Variables(v));
      if (!r2) continue;
      String name = String(quantityName(Quantities(q))) + "_" + variableName(Variables(v));
      Histogram * h = new Histogram(*r2);
      h->reset();
      h->setName(name);
      h->setTitle(name);
      _h[v][q] = append(h);
      }
  }

  void ParticlePair3DBFHistos::calculate(const ParticlePair3DHistos & ab,  const ParticlePair3DHistos & abB,
                                         const ParticlePair3DHistos & aBb, const ParticlePair3DHistos & aBbB,
                                         const ParticleSingleHistos & a, const ParticleSingleHistos & aB,
                                         const ParticleSingleHistos & /*b*/, const ParticleSingleHistos & /*bB*/)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  if (!a.h_n1_ptY || !aB.h_n1_ptY)
    throw HistogramException("n1_ptY","missing in a single folder (FillPtvsY)",__FUNCTION__);
  double nA = a.h_n1_ptY->integral(), nAB = aB.h_n1_ptY->integral();
  double invA = nA>0.0 ? 1.0/nA : 0.0, invAB = nAB>0.0 ? 1.0/nAB : 0.0;
  for (int iv=0; iv<kNVariables; iv++)
    {
    Variables v = Variables(iv);
    if (!_h[v][kB]) continue;
    const Histogram * r_ab   = of(ab,"r2",v);
    const Histogram * r_abB  = of(abB,"r2",v);
    const Histogram * r_aBb  = of(aBb,"r2",v);
    const Histogram * r_aBbB = of(aBbB,"r2",v);
    const Histogram * n_ab   = of(ab,"n2",v);
    const Histogram * n_abB  = of(abB,"n2",v);
    const Histogram * n_aBb  = of(aBb,"n2",v);
    const Histogram * n_aBbB = of(aBbB,"n2",v);
    if (!r_ab || !r_abB || !r_aBb || !r_aBbB)
      throw HistogramException(String("r2_")+variableName(v),"missing in a pair folder (derived histograms not computed?)",__FUNCTION__);
    if (!n_ab || !n_abB || !n_aBb || !n_aBbB)
      throw HistogramException(String("n2_")+variableName(v),"missing in a pair folder",__FUNCTION__);

    // charge independent and charge dependent combinations
    _h[v][kR2CI]->setAsSumOf({ r_ab, r_abB, r_aBb, r_aBbB }, { 0.25, 0.25, 0.25, 0.25 });
    _h[v][kR2CD]->setAsSumOf({ r_abB, r_aBb, r_ab, r_aBbB }, { 0.5, 0.5, -0.5, -0.5 });

    // balance functions per trigger particle
    _h[v][kBabB]->setAsSumOf(*n_abB, *n_aBbB, invA,  -invAB);
    _h[v][kBaBb]->setAsSumOf(*n_aBb, *n_ab,   invAB, -invA);
    _h[v][kB]   ->setAsSumOf(*_h[v][kBabB], *_h[v][kBaBb], 0.5, 0.5);
    }
  }

} // namespace CAP
