/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <cmath>
#include "ParticlePairBFHistos.hpp"
#include "Exceptions.hpp"

namespace CAP
{

  ParticlePairBFHistos::ParticlePairBFHistos(const String & name)
  :
  HistogramFolder(name),
  _shiftBins(0)
  {
  appendClassName("ParticlePairBFHistos");
  for (int v=0; v<kNVariables; v++)
    for (int q=0; q<kNQuantities; q++) { _h[v][q] = nullptr; _hShifted[v][q] = nullptr; }
  }

  ParticlePairBFHistos::ParticlePairBFHistos(const ParticlePairBFHistos & source)
  :
  HistogramFolder(source),
  _shiftBins(source._shiftBins)
  {
  relink();
  }

  ParticlePairBFHistos & ParticlePairBFHistos::operator=(const ParticlePairBFHistos & rhs)
  {
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    _shiftBins = rhs._shiftBins;
    relink();
    }
  return *this;
  }

  const char * ParticlePairBFHistos::variablesName(Variables v)
  {
  static const char * names[kNVariables] = { "ptpt", "phiphi", "etaeta", "yY", "DetaDphi", "DyDphi" };
  return names[v];
  }

  const char * ParticlePairBFHistos::quantityName(Quantities q)
  {
  static const char * names[kNQuantities] = { "R2CI", "R2CD", "B_a_bB", "B_aB_b", "B" };
  return names[q];
  }

  void ParticlePairBFHistos::relink()
  {
  for (int v=0; v<kNVariables; v++)
    for (int q=0; q<kNQuantities; q++)
      {
      String name = String(quantityName(Quantities(q))) + "_" + variablesName(Variables(v));
      _h[v][q]        = histogram(name);
      _hShifted[v][q] = histogram(name + "_shft");
      }
  }

  const Histogram * ParticlePairBFHistos::r2Of(const ParticlePairHistos & pair, Variables v)
  {
  switch (v)
    {
    case kPtPt:     return pair.h_R2_ptpt;
    case kPhiPhi:   return pair.h_R2_phiphi;
    case kEtaEta:   return pair.eta_fill      ? pair.h_R2_etaeta   : nullptr;
    case kYY:       return pair.rapidity_fill ? pair.h_R2_yY       : nullptr;
    case kDetaDphi: return pair.eta_fill      ? pair.h_R2_DetaDphi : nullptr;
    case kDyDphi:   return pair.rapidity_fill ? pair.h_R2_DyDphi   : nullptr;
    default:        return nullptr;
    }
  }

  const Histogram * ParticlePairBFHistos::shiftedR2Of(const ParticlePairHistos & pair, Variables v)
  {
  if (v==kDetaDphi) return pair.eta_fill      ? pair.h_R2_DetaDphi_shft : nullptr;
  if (v==kDyDphi)   return pair.rapidity_fill ? pair.h_R2_DyDphi_shft   : nullptr;
  return nullptr;
  }

  void ParticlePairBFHistos::createFrom(const ParticlePairHistos & pair)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  clear();
  _shiftBins = pair.Dphi_nbins_shft;
  for (int v=0; v<kNVariables; v++)
    for (int q=0; q<kNQuantities; q++)
      {
      _h[v][q] = nullptr;
      _hShifted[v][q] = nullptr;
      const Histogram * r2 = r2Of(pair,Variables(v));
      if (!r2) continue;
      String name = String(quantityName(Quantities(q))) + "_" + variablesName(Variables(v));
      Histogram * h = new Histogram(*r2);
      h->reset();
      h->setName(name);
      h->setTitle(name);
      _h[v][q] = append(h);
      const Histogram * shifted = shiftedR2Of(pair,Variables(v));
      if (shifted && _shiftBins>0)
        {
        Histogram * hs = new Histogram(*shifted);
        hs->reset();
        hs->setName(name + "_shft");
        hs->setTitle(name + "_shft");
        _hShifted[v][q] = append(hs);
        }
      }
  }

  void ParticlePairBFHistos::multiplyByAssociatedDensity(Histogram & h, const Histogram & rho1)
  {
  if (h.rank()!=2 || rho1.rank()!=1 || h.nBins(1)!=rho1.nBins(0))
    throw HistogramException(h.name(),String("binning incompatible with the density ")+rho1.name(),__FUNCTION__);
  unsigned int n0 = h.nBins(0), n1 = h.nBins(1);
  for (unsigned int j=0; j<n1; j++)
    {
    double rho = rho1.binValueAtGlobal(j), eRho = rho1.binUncertaintyAtGlobal(j);
    for (unsigned int i=0; i<n0; i++)
      {
      unsigned int k = i + n0*j;
      double v = h.binValueAtGlobal(k), e = h.binUncertaintyAtGlobal(k);
      h.setBinValueAtGlobal(k, v*rho);
      h.setBinUncertaintyAtGlobal(k, std::sqrt(e*e*rho*rho + v*v*eRho*eRho));
      }
    }
  }

  void ParticlePairBFHistos::calculate(const ParticlePairHistos & ab,  const ParticlePairHistos & abB,
                                       const ParticlePairHistos & aBb, const ParticlePairHistos & aBbB,
                                       const ParticleSingleHistos & /*a*/, const ParticleSingleHistos & /*aB*/,
                                       const ParticleSingleHistos & b, const ParticleSingleHistos & bB)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  for (int iv=0; iv<kNVariables; iv++)
    {
    Variables v = Variables(iv);
    if (!_h[v][kB]) continue;
    const Histogram * r_ab   = r2Of(ab,v);
    const Histogram * r_abB  = r2Of(abB,v);
    const Histogram * r_aBb  = r2Of(aBb,v);
    const Histogram * r_aBbB = r2Of(aBbB,v);
    if (!r_ab || !r_abB || !r_aBb || !r_aBbB)
      throw HistogramException(String("R2_")+variablesName(v),"missing in a pair folder (derived histograms not computed?)",__FUNCTION__);

    // charge independent and charge dependent combinations
    _h[v][kR2CI]->setAsSumOf({ r_ab, r_abB, r_aBb, r_aBbB }, { 0.25, 0.25, 0.25, 0.25 });
    _h[v][kR2CD]->setAsSumOf({ r_abB, r_aBb, r_ab, r_aBbB }, { 0.5, 0.5, -0.5, -0.5 });

    // balance functions B(a|b̄) and B(ā|b)
    Histogram & babB = *_h[v][kBabB];
    Histogram & baBb = *_h[v][kBaBb];
    babB.setAsSumOf(*r_abB, *r_aBbB, 1.0, -1.0);
    baBb.setAsSumOf(*r_aBb, *r_ab,   1.0, -1.0);
    if (v==kDetaDphi || v==kDyDphi)
      {
      // mean density of the associated particles per bin of (Delta, Delta phi) [Eq. (BF) of the paper]
      const Histogram * d_b  = v==kDetaDphi ? b.h_n1_phiEta  : b.h_n1_phiY;
      const Histogram * d_bB = v==kDetaDphi ? bB.h_n1_phiEta : bB.h_n1_phiY;
      if (!d_b || !d_bB) throw HistogramException(v==kDetaDphi ? "n1_phiEta" : "n1_phiY","missing in a single folder",__FUNCTION__);
      double acceptance = (d_b->maximum(0)-d_b->minimum(0))*(d_b->maximum(1)-d_b->minimum(1));
      double binArea    = babB.binWidth(0,0)*babB.binWidth(0,1);
      double factor     = acceptance>0.0 ? binArea/acceptance : 0.0;
      babB.scale(factor*d_bB->integral());
      baBb.scale(factor*d_b->integral());
      }
    else
      {
      const Histogram * rho_b  = nullptr;
      const Histogram * rho_bB = nullptr;
      switch (v)
        {
        case kPtPt:   rho_b = b.h_n1_pt;  rho_bB = bB.h_n1_pt;  break;
        case kPhiPhi: rho_b = b.h_n1_phi; rho_bB = bB.h_n1_phi; break;
        case kEtaEta: rho_b = b.h_n1_eta; rho_bB = bB.h_n1_eta; break;
        case kYY:     rho_b = b.h_n1_y;   rho_bB = bB.h_n1_y;   break;
        default: break;
        }
      if (!rho_b || !rho_bB) throw HistogramException(String("n1 of ")+variablesName(v),"missing in a single folder",__FUNCTION__);
      multiplyByAssociatedDensity(babB,*rho_bB);
      multiplyByAssociatedDensity(baBb,*rho_b);
      }
    _h[v][kB]->setAsSumOf(babB, baBb, 0.5, 0.5);

    if (_shiftBins>0)
      for (int q=0; q<kNQuantities; q++)
        if (_hShifted[v][q]) shiftY(*_h[v][q], *_hShifted[v][q], _shiftBins);
    }
  }

} // namespace CAP
