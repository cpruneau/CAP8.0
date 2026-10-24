/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "RootConversion.hpp"
#include <cmath>
#include "TH2D.h"
#include "TH3D.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TAxis.h"
#include "Exceptions.hpp"

namespace CAP
{

  namespace
  {
  void copyAxis(const MATH::Histogram & h, unsigned int d, TAxis * axis)
  {
  axis->SetTitle(h.dimensionTitle(d));
  for (const auto & label : h.binLabels(d)) axis->SetBinLabel(int(label.first)+1,label.second);
  }

  void checkFixedBins(const TAxis * axis, const char * source)
  {
  if (axis->GetXbins()->GetSize()>0) throw Exception("Variable-width bins cannot be converted",source);
  }
  }

  TH1 * toRootHistogram(const MATH::Histogram & h, const String & name)
  {
  String rootName = name.length()>0 ? name : h.name();
  if (const MATH::Profile * p = dynamic_cast<const MATH::Profile*>(&h)) return toRootProfile(*p,rootName);
  TH1 * r = nullptr;
  bool addStatus = TH1::AddDirectoryStatus();
  TH1::AddDirectory(false);
  switch (h.rank())
    {
    case 1:
      r = new TH1D(rootName,h.title(),h.nBins(0),h.minimum(0),h.maximum(0));
      break;
    case 2:
      r = new TH2D(rootName,h.title(),h.nBins(0),h.minimum(0),h.maximum(0),h.nBins(1),h.minimum(1),h.maximum(1));
      break;
    case 3:
      r = new TH3D(rootName,h.title(),h.nBins(0),h.minimum(0),h.maximum(0),h.nBins(1),h.minimum(1),h.maximum(1),
                   h.nBins(2),h.minimum(2),h.maximum(2));
      break;
    default:
      TH1::AddDirectory(addStatus);
      throw Exception("Only histograms of rank 1, 2 or 3 can be converted to ROOT",__FUNCTION__);
    }
  TH1::AddDirectory(addStatus);
  r->Sumw2();
  copyAxis(h,0,r->GetXaxis());
  if (h.rank()>1) copyAxis(h,1,r->GetYaxis());
  if (h.rank()>2) copyAxis(h,2,r->GetZaxis());
  unsigned int n0 = h.nBins(0);
  unsigned int n1 = h.rank()>1 ? h.nBins(1) : 1;
  double entries = 0.0;
  for (unsigned int k=0; k<h.nBinsTotal(); k++)
    {
    unsigned int i0 = k % n0;
    unsigned int i1 = (k/n0) % n1;
    unsigned int i2 = k/(n0*n1);
    int bin = (h.rank()==1) ? r->GetBin(i0+1) : (h.rank()==2) ? r->GetBin(i0+1,i1+1) : r->GetBin(i0+1,i1+1,i2+1);
    r->SetBinContent(bin,h.binValueAtGlobal(k));
    r->SetBinError(bin,h.binUncertaintyAtGlobal(k));
    entries += h.binEntriesAtGlobal(k);
    }
  r->SetEntries(entries);
  return r;
  }

  TH1 * toRootProfile(const MATH::Profile & p, const String & name)
  {
  if (p.rank()<1 || p.rank()>2) throw Exception("Only profiles of rank 1 or 2 can be converted to ROOT",__FUNCTION__);
  String rootName = name.length()>0 ? name : p.name();
  bool addStatus = TH1::AddDirectoryStatus();
  TH1::AddDirectory(false);
  TProfile * p1 = nullptr;
  TProfile2D * p2 = nullptr;
  if (p.rank()==1) p1 = new TProfile(rootName,p.title(),p.nBins(0),p.minimum(0),p.maximum(0));
  else             p2 = new TProfile2D(rootName,p.title(),p.nBins(0),p.minimum(0),p.maximum(0),p.nBins(1),p.minimum(1),p.maximum(1));
  TH1::AddDirectory(addStatus);
  TH1 * r = p1 ? static_cast<TH1*>(p1) : static_cast<TH1*>(p2);
  r->Sumw2();   // sums of w^2 per bin kept (weighted fills)
  copyAxis(p,0,r->GetXaxis());
  if (p.rank()>1) copyAxis(p,1,r->GetYaxis());
  unsigned int n0 = p.nBins(0);
  double entries = 0.0;
  // ROOT's arrays of a profile: fArray = sum w*y, fSumw2 = sum w*y^2, fBinEntries = sum w, fBinSumw2 = sum w^2
  TArrayD * sumWY2 = r->GetSumw2();
  TArrayD * sumW2  = p1 ? p1->GetBinSumw2() : p2->GetBinSumw2();
  for (unsigned int k=0; k<p.nBinsTotal(); k++)
    {
    int bin = p.rank()==1 ? r->GetBin(int(k)+1) : r->GetBin(int(k%n0)+1,int(k/n0)+1);
    if (p1) p1->SetBinEntries(bin,p.sumWeights(k)); else p2->SetBinEntries(bin,p.sumWeights(k));
    (p1 ? p1->GetArray() : p2->GetArray())[bin] = p.sumWeightedValues(k);
    sumWY2->fArray[bin] = p.sumWeightedValues2(k);
    if (sumW2->GetSize()>bin) sumW2->fArray[bin] = p.sumWeights2(k);
    entries += p.nEntries(k);
    }
  r->SetEntries(entries);
  return r;
  }

  MATH::Histogram * fromRootHistogram(const TH1 & r)
  {
  if (r.InheritsFrom(TProfile::Class()) || r.InheritsFrom(TProfile2D::Class())) return fromRootProfile(r);
  int dimension = r.GetDimension();
  if (dimension<1 || dimension>3) throw Exception("Only ROOT histograms of dimension 1, 2 or 3 can be converted",__FUNCTION__);
  MATH::Partition partition;
  const TAxis * axes[3] = { r.GetXaxis(), r.GetYaxis(), r.GetZaxis() };
  for (int d=0; d<dimension; d++)
    {
    checkFixedBins(axes[d],__FUNCTION__);
    partition.addDimension(axes[d]->GetNbins(),axes[d]->GetXmin(),axes[d]->GetXmax(),axes[d]->GetTitle());
    }
  MATH::Histogram * h = new MATH::Histogram(partition);
  h->setPrecision(MATH::Histogram::kDouble);
  h->setName(r.GetName());
  h->setTitle(r.GetTitle());
  for (int d=0; d<dimension; d++)
    for (int i=1; i<=axes[d]->GetNbins(); i++)
      {
      const char * label = axes[d]->GetBinLabel(i);
      if (label && label[0]) h->setBinLabel(i-1,label,d);
      }
  unsigned int n0 = h->nBins(0);
  unsigned int n1 = dimension>1 ? h->nBins(1) : 1;
  for (unsigned int k=0; k<h->nBinsTotal(); k++)
    {
    unsigned int i0 = k % n0;
    unsigned int i1 = (k/n0) % n1;
    unsigned int i2 = k/(n0*n1);
    int bin = (dimension==1) ? r.GetBin(i0+1) : (dimension==2) ? r.GetBin(i0+1,i1+1) : r.GetBin(i0+1,i1+1,i2+1);
    double value = r.GetBinContent(bin);
    double error = r.GetBinError(bin);
    if (value==0.0 && error==0.0) continue;
    // unweighted counts (error^2 = content, a whole number): as many entries; otherwise one
    long entries = 1;
    double rounded = std::round(value);
    if (value>0.0 && rounded==value && std::fabs(error*error-value)<=1e-9*value) entries = long(rounded);
    h->setBinAtGlobal(k,entries,value,error);
    }
  return h;
  }

  MATH::Profile * fromRootProfile(const TH1 & r)
  {
  const TProfile * p1 = dynamic_cast<const TProfile*>(&r);
  const TProfile2D * p2 = dynamic_cast<const TProfile2D*>(&r);
  if (!p1 && !p2) throw Exception("Not a TProfile or TProfile2D",__FUNCTION__);
  int dimension = p1 ? 1 : 2;
  const TAxis * axes[2] = { r.GetXaxis(), r.GetYaxis() };
  MATH::Partition partition;
  for (int d=0; d<dimension; d++)
    {
    checkFixedBins(axes[d],__FUNCTION__);
    partition.addDimension(axes[d]->GetNbins(),axes[d]->GetXmin(),axes[d]->GetXmax(),axes[d]->GetTitle());
    }
  MATH::Profile * p = new MATH::Profile(partition);
  p->setName(r.GetName());
  p->setTitle(r.GetTitle());
  for (int d=0; d<dimension; d++)
    for (int i=1; i<=axes[d]->GetNbins(); i++)
      {
      const char * label = axes[d]->GetBinLabel(i);
      if (label && label[0]) p->setBinLabel(i-1,label,d);
      }
  const TArrayD * sumWY2 = const_cast<TH1&>(r).GetSumw2();
  const TArrayD * sumW2  = p1 ? const_cast<TProfile*>(p1)->GetBinSumw2() : const_cast<TProfile2D*>(p2)->GetBinSumw2();
  const double * sumWY = p1 ? p1->GetArray() : p2->GetArray();
  unsigned int n0 = p->nBins(0);
  for (unsigned int k=0; k<p->nBinsTotal(); k++)
    {
    int bin = dimension==1 ? r.GetBin(int(k)+1) : r.GetBin(int(k%n0)+1,int(k/n0)+1);
    double sw = p1 ? p1->GetBinEntries(bin) : p2->GetBinEntries(bin);
    if (sw==0.0) continue;
    double sw2 = sumW2->GetSize()>bin ? sumW2->fArray[bin] : sw;   // no sums of w^2: unweighted fills
    double effective = p1 ? p1->GetBinEffectiveEntries(bin) : const_cast<TProfile2D*>(p2)->GetBinEffectiveEntries(bin);
    long entries = long(std::round(sw2==sw ? sw : effective));
    if (entries<1) entries = 1;
    p->setSums(k,entries,sw,sw2,sumWY[bin],sumWY2->GetSize()>bin ? sumWY2->fArray[bin] : 0.0);
    }
  return p;
  }

} // namespace CAP
