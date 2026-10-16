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
#include "TH2D.h"
#include "TH3D.h"
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

  MATH::Histogram * fromRootHistogram(const TH1 & r)
  {
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
    if (value!=0.0 || error!=0.0) h->setBinAtGlobal(k,1,value,error);
    }
  return h;
  }

} // namespace CAP
