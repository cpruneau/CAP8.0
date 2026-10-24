/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "Exceptions.hpp"
#include "Task.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "ParticleEfficiency.hpp"

ClassImp(CAP::ParticleEfficiency);

namespace CAP
{

  ParticleEfficiency::ParticleEfficiency()
  :
  HistogramFolder(),
  h_efficiency_etaPt(nullptr),
  h_weight_etaPt(nullptr),
  pt_nbins(0),
  eta_nbins(0),
  phi_nbins(0),
  pt_min(0),
  pt_max(0),
  eta_min(0),
  eta_max(0),
  phi_min(0),
  phi_max(0)
  { /* no ops */}

  ParticleEfficiency::ParticleEfficiency(const ParticleEfficiency & source)
  :
  HistogramFolder(source),
  h_efficiency_etaPt(nullptr),
  h_weight_etaPt(nullptr),
  pt_nbins(source.pt_nbins),
  eta_nbins(source.eta_nbins),
  phi_nbins(source.phi_nbins),
  pt_min(source.pt_min),
  pt_max(source.pt_max),
  eta_min(source.eta_min),
  eta_max(source.eta_max),
  phi_min(source.phi_min),
  phi_max(source.phi_max)
  {
  cloneB(source);
  }

  ParticleEfficiency & ParticleEfficiency::operator=(const ParticleEfficiency & rhs)
  {
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    cloneB(rhs);
    pt_nbins   = rhs.pt_nbins;
    eta_nbins  = rhs.eta_nbins;
    phi_nbins  = rhs.phi_nbins;
    pt_min     = rhs.pt_min;
    pt_max     = rhs.pt_max;
    eta_min    = rhs.eta_min;
    eta_max    = rhs.eta_max;
    phi_min    = rhs.phi_min;
    phi_max    = rhs.phi_max;
    }
  return *this;
  }

  double ParticleEfficiency::efficiencyAt(const VectorLorentz & momentum)
  {
  int k = h_efficiency_etaPt->binIndex(momentum.pseudorapidity(),momentum.perp());
  if (k<0) return 0.0;   // outside the (eta,pt) range of the histogram
  return h_efficiency_etaPt->binValueAtGlobal(k);
  }

  double ParticleEfficiency::weightAt(const VectorLorentz & momentum)
  {
  if (!h_weight_etaPt) calculateWeights();
  int k = h_weight_etaPt->binIndex(momentum.pseudorapidity(),momentum.perp());
  if (k<0) return 0.0;   // outside the (eta,pt) range of the histogram
  return h_weight_etaPt->binValueAtGlobal(k);
  }

  bool ParticleEfficiency::accept(const Particle & particle)
  {
  return gRandom->Rndm()<=efficiencyAt(particle.momentum());
  }

  bool ParticleEfficiency::accept(const VectorLorentz & momentum)
  {
  return gRandom->Rndm()<=efficiencyAt(momentum);
  }

  void ParticleEfficiency::calculateWeights()
  {
  if (h_weight_etaPt) delete h_weight_etaPt;
  if (h_efficiency_etaPt)
    {
    h_weight_etaPt = h_efficiency_etaPt->clone();
    // weight = 1/efficiency in bins with an efficiency in (0,1]; 0 elsewhere
    for (unsigned int k=0; k<h_weight_etaPt->nBinsTotal(); k++)
      {
      double v  = h_weight_etaPt->binValueAtGlobal(k);
      double ev = h_weight_etaPt->binUncertaintyAtGlobal(k);
      if (v>0.0  &&  v<=1.0)
        {
        double w  = 1.0/v;
        double ew = w*ev/v;
        h_weight_etaPt->setBinAtGlobal(k,1,w,ew);
        }
      else
        {
        h_weight_etaPt->setBinAtGlobal(k,0,0.0,0.0);
        }
      }
    }
  else
    {
    throw Exception("Missing histogram h_efficiency_etaPt",__FUNCTION__);
    }
  }



  void ParticleEfficiency::cloneB(const ParticleEfficiency & source)
  {
  // the efficiency histogram belongs to the group (copied with it); the weight histogram is owned by this object
  h_efficiency_etaPt = histogramLike(source.h_efficiency_etaPt);
  delete h_weight_etaPt;
  h_weight_etaPt     = safeClone(source.h_weight_etaPt);
  }

  void ParticleEfficiency::configure(const String & ownerName,
                                     const String & typeName,
                                     const Configuration & configuration,
                                     unsigned int index)
  {
  HistogramFolder::configure(ownerName,typeName,configuration,index);
  int pt_nbins   = configuration.valueInt(   createKey(ownerName,typeName,index,"pt_nbins"));
  double pt_min  = configuration.valueDouble(createKey(ownerName,typeName,index,"pt_min"));
  double pt_max  = configuration.valueDouble(createKey(ownerName,typeName,index,"pt_max"));
  int eta_nbins  = configuration.valueInt(   createKey(ownerName,typeName,index,"eta_nbins"));
  double eta_min = configuration.valueDouble(createKey(ownerName,typeName,index,"eta_min"));
  double eta_max = configuration.valueDouble(createKey(ownerName,typeName,index,"eta_max"));
  //  int phi_nbins  = configuration.valueInt(createKey(ownerName,typeName,index,"phi_nbins"));
  //  double phi_min = configuration.valueDouble(createKey(ownerName,typeName,index,"phi_min"));
  //  double phi_max = configuration.valueDouble(createKey(ownerName,typeName,index,"phi_max"));
  if (reportDebug(__FUNCTION__))
    {
    printCR();
    printLine();
    printValue("Histo Base Name",baseName());
    printValue("pt_nbins",   pt_nbins);
    printValue("pt_min",     pt_min);
    printValue("pt_max",     pt_max);
    printValue("eta_nbins",  eta_nbins);
    printValue("eta_min",    eta_min);
    printValue("eta_max",    eta_max);
    printCR();
    }
  }
  
  void ParticleEfficiency::create()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_efficiency_etaPt  = createHistogram(createName(baseName(),"efficiency_etaPt"),eta_nbins,eta_min,eta_max,pt_nbins,pt_min,pt_max,"#eta","p_{T}","#epsilon",2);

  }

  void ParticleEfficiency::loadFrom(const Folder & inputFile)
  {
  String name = "PART_EFF_ALL_ALL_etaPt";
  printValue(name,name);
  h_efficiency_etaPt  = importHistogram(inputFile,name);
  }

} // namespace CAP
