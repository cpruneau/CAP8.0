/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ParticleSingleHistos.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Task.hpp"
#include "Configuration.hpp"
#include "NameHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"


namespace CAP
{

  ParticleSingleHistos::ParticleSingleHistos()
  :
  HistogramFolder(),
  eta_fill(0),
  rapidity_fill(0),
  p2_fill(0),
  ptVsRapidity_fill(0),
  pid_fill(0),
  n1_nbins(0),
  pt_nbins(0),
  phi_nbins(0),
  eta_nbins(0),
  rapidity_nbins(0),
  n1_min(0),
  n1_max(0),
  pt_min(0),
  pt_max(0),
  phi_min(0),
  phi_max(6.28319),
  eta_min(0),
  eta_max(0),
  rapidity_minimum(0),
  rapidity_maximum(0),
  h_n1(nullptr),
  h_n1_eTotal(nullptr),
  h_n1_pt(nullptr),
  h_n1_ptXS(nullptr),
  h_n1_phiEta(nullptr),
  h_spt_phiEta(nullptr),
  h_n1_phiY(nullptr),
  h_spt_phiY(nullptr),
  h_n1_ptY(nullptr),
  h_pdgId(nullptr),
  h_n1_phi(nullptr),
  h_n1_eta(nullptr),
  h_n1_y(nullptr),
  h_spt_phi(nullptr),
  h_spt_eta(nullptr),
  h_spt_y(nullptr),
  h_avg_pt_phi(nullptr),
  h_avg_pt_eta(nullptr),
  h_avg_pt_phiEta(nullptr),
  h_avg_pt_y(nullptr),
  h_avg_pt_phiY(nullptr)
  {
  appendClassName("ParticleSingleHistos");
  setName("ParticleSingleHistos");
  setTitle("ParticleSingleHistos");
  }

  ParticleSingleHistos::ParticleSingleHistos(const ParticleSingleHistos & source )
  :
  HistogramFolder(source ),
  eta_fill(source.eta_fill ),
  rapidity_fill(source.rapidity_fill ),
  p2_fill(source.p2_fill ),
  ptVsRapidity_fill(source.ptVsRapidity_fill ),
  pid_fill(source.pid_fill ),
  n1_nbins(source.n1_nbins),
  pt_nbins(source.pt_nbins),
  phi_nbins(source.phi_nbins),
  eta_nbins(source.eta_nbins),
  rapidity_nbins(source.rapidity_nbins),
  n1_min(source.n1_min),
  n1_max(source.n1_max),
  pt_min(source.pt_min),
  pt_max(source.pt_max),
  phi_min(source.phi_min),
  phi_max(source.phi_max),
  eta_min(source.eta_min),
  eta_max(source.eta_max),
  rapidity_minimum(source.rapidity_minimum),
  rapidity_maximum(source.rapidity_maximum),
  h_n1(nullptr),
  h_n1_eTotal(nullptr),
  h_n1_pt(nullptr),
  h_n1_ptXS(nullptr),
  h_n1_phiEta(nullptr),
  h_spt_phiEta(nullptr),
  h_n1_phiY(nullptr),
  h_spt_phiY(nullptr),
  h_n1_ptY(nullptr),
  h_pdgId(nullptr),
  h_n1_phi(nullptr),
  h_n1_eta(nullptr),
  h_n1_y(nullptr),
  h_spt_phi(nullptr),
  h_spt_eta(nullptr),
  h_spt_y(nullptr),
  h_avg_pt_phi(nullptr),
  h_avg_pt_eta(nullptr),
  h_avg_pt_phiEta(nullptr),
  h_avg_pt_y(nullptr),
  h_avg_pt_phiY(nullptr)
  {
  cloneB(source );
  }

  ParticleSingleHistos & ParticleSingleHistos::operator=(const ParticleSingleHistos & rhs)
  {
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    eta_fill           = rhs.eta_fill;
    rapidity_fill      = rhs.rapidity_fill;
    p2_fill            = rhs.p2_fill;
    ptVsRapidity_fill  = rhs.ptVsRapidity_fill;
    pid_fill           = rhs.pid_fill;
    n1_nbins           = rhs.n1_nbins;
    pt_nbins           = rhs.pt_nbins;
    phi_nbins          = rhs.phi_nbins;
    eta_nbins          = rhs.eta_nbins;
    rapidity_nbins     = rhs.rapidity_nbins;
    n1_min             = rhs.n1_min;
    n1_max             = rhs.n1_max;
    pt_min             = rhs.pt_min;
    pt_max             = rhs.pt_max;
    phi_min            = rhs.phi_min;
    phi_max            = rhs.phi_max;
    eta_min            = rhs.eta_min;
    eta_max            = rhs.eta_max;
    rapidity_minimum   = rhs.rapidity_minimum;
    rapidity_maximum   = rhs.rapidity_maximum;
    cloneB(rhs);
    }
  return *this;
  }

  void ParticleSingleHistos::cloneB(const ParticleSingleHistos & source )
  {
  if (reportStart(__FUNCTION__)) { printCR(); }
  h_n1         = histogramLike(source.h_n1);
  h_n1_eTotal  = histogramLike(source.h_n1_eTotal);
  h_n1_pt      = histogramLike(source.h_n1_pt);
  h_n1_ptXS    = histogramLike(source.h_n1_ptXS);
  h_n1_phiEta  = histogramLike(source.h_n1_phiEta);
  h_spt_phiEta = histogramLike(source.h_spt_phiEta);
  h_n1_phiY    = histogramLike(source.h_n1_phiY);
  h_spt_phiY   = histogramLike(source.h_spt_phiY);
  h_n1_ptY     = histogramLike(source.h_n1_ptY);
  h_pdgId      = histogramLike(source.h_pdgId);
  h_n1_phi        = histogramLike(source.h_n1_phi);
  h_n1_eta        = histogramLike(source.h_n1_eta);
  h_n1_y          = histogramLike(source.h_n1_y);
  h_spt_phi       = histogramLike(source.h_spt_phi);
  h_spt_eta       = histogramLike(source.h_spt_eta);
  h_spt_y         = histogramLike(source.h_spt_y);
  h_avg_pt_phi    = histogramLike(source.h_avg_pt_phi);
  h_avg_pt_eta    = histogramLike(source.h_avg_pt_eta);
  h_avg_pt_phiEta = histogramLike(source.h_avg_pt_phiEta);
  h_avg_pt_y      = histogramLike(source.h_avg_pt_y);
  h_avg_pt_phiY   = histogramLike(source.h_avg_pt_phiY);
  if (reportEnd(__FUNCTION__)) { printCR(); }
  }

  void ParticleSingleHistos::addDefaultConfiguration(Configuration & configuration, const String & taskName)
  {
  const String type = "HISTOGRAM";
  configuration.addProperty(createKey(taskName,type,"n1_nbins"),       100);
  configuration.addProperty(createKey(taskName,type,"n1_min"),         0.0);
  configuration.addProperty(createKey(taskName,type,"n1_max"),       100.0);
  configuration.addProperty(createKey(taskName,type,"etot_nbins"),     100);
  configuration.addProperty(createKey(taskName,type,"etot_min"),       0.0);
  configuration.addProperty(createKey(taskName,type,"etot_max"),     100.0);
  configuration.addProperty(createKey(taskName,type,"pt_nbins"),       100);
  configuration.addProperty(createKey(taskName,type,"pt_min"),         0.0);
  configuration.addProperty(createKey(taskName,type,"pt_max"),       100.0);
  configuration.addProperty(createKey(taskName,type,"eta_nbins"),       20);
  configuration.addProperty(createKey(taskName,type,"eta_min"),       -1.0);
  configuration.addProperty(createKey(taskName,type,"eta_max"),        1.0);
  configuration.addProperty(createKey(taskName,type,"rapidity_nbins"),  20);
  configuration.addProperty(createKey(taskName,type,"rapidity_min"),  -1.0);
  configuration.addProperty(createKey(taskName,type,"rapidity_max"),   1.0);
  configuration.addProperty(createKey(taskName,type,"phi_nbins"),       36);
  configuration.addProperty(createKey(taskName,type,"phi_min"),        0.0);
  configuration.addProperty(createKey(taskName,type,"phi_max"),        CAP::MATH::twoPi());
  configuration.addProperty(createKey(taskName,type,"phiEta_nbins"),   720);
  configuration.addProperty(createKey(taskName,type,"phiEtaPt_nbins"), 7200);
  configuration.addProperty(createKey(taskName,type,"phiY_nbins"),     720);
  configuration.addProperty(createKey(taskName,type,"phiYPt_nbins"),   7200);
  configuration.addProperty(createKey(taskName,type,"eta_fill"),       true);
  configuration.addProperty(createKey(taskName,type,"rapidity_fill"),  false);
  configuration.addProperty(createKey(taskName,type,"p2_fill"),        false);
  configuration.addProperty(createKey(taskName,type,"FillPtvsY"),      false);
  configuration.addProperty(createKey(taskName,type,"FillPid"),        false);
  }

  void ParticleSingleHistos::configure(const String & taskName,
                                       const String & objectType,
                                       const Configuration & configuration,
                                       unsigned int index)
  {
  HistogramFolder::configure(taskName,objectType,configuration,index);
  String type;
  if (objectType.contains("HISTOGRAM"))
    type = "HISTOGRAM";
  else
    type = objectType;
  eta_fill          = configuration.valueBool(   createKey(taskName,type,"eta_fill"));
  rapidity_fill     = configuration.valueBool(   createKey(taskName,type,"rapidity_fill"));
  p2_fill           = configuration.valueBool(   createKey(taskName,type,"p2_fill"));
  pid_fill          = configuration.valueBool(   createKey(taskName,type,"FillPid"));
  ptVsRapidity_fill = configuration.valueBool(   createKey(taskName,type,"FillPtvsY"));

  n1_nbins          = configuration.valueInt(    createKey(taskName,type,"n1_nbins"));
  pt_nbins          = configuration.valueInt(    createKey(taskName,type,"pt_nbins"));
  phi_nbins         = configuration.valueInt(    createKey(taskName,type,"phi_nbins"));
  eta_nbins         = configuration.valueInt(    createKey(taskName,type,"eta_nbins"));
  rapidity_nbins    = configuration.valueInt(    createKey(taskName,type,"rapidity_nbins"));

  n1_min   = configuration.valueDouble(   createKey(taskName,type,"n1_min"));
  n1_max   = configuration.valueDouble(   createKey(taskName,type,"n1_max"));
  pt_min   = configuration.valueDouble(   createKey(taskName,type,"pt_min"));
  pt_max   = configuration.valueDouble(   createKey(taskName,type,"pt_max"));
  phi_min  = configuration.valueDouble(   createKey(taskName,type,"phi_min"));
  phi_max  = configuration.valueDouble(   createKey(taskName,type,"phi_max"));
  eta_min  = configuration.valueDouble(   createKey(taskName,type,"eta_min"));
  eta_max  = configuration.valueDouble(   createKey(taskName,type,"eta_max"));
  rapidity_minimum    = configuration.valueDouble(   createKey(taskName,type,"rapidity_min"));
  rapidity_maximum    = configuration.valueDouble(   createKey(taskName,type,"rapidity_max"));


  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printValue(createKey(taskName,type,"eta_fill"),eta_fill);
    printValue(createKey(taskName,type,"rapidity_fill"),rapidity_fill  );
    printValue(createKey(taskName,type,"p2_fill"),p2_fill );
    printValue(createKey(taskName,type,"FillPtvsY"),ptVsRapidity_fill );
    printValue(createKey(taskName,type,"n1_nbins"),n1_nbins);
    printValue(createKey(taskName,type,"n1_min"),n1_min);
    printValue(createKey(taskName,type,"n1_max"),n1_max);
    printValue(createKey(taskName,type,"pt_nbins"),pt_nbins);
    printValue(createKey(taskName,type,"pt_min"),pt_min);
    printValue(createKey(taskName,type,"pt_max"),pt_max);
    printValue(createKey(taskName,type,"phi_nbins"),phi_nbins);
    printValue(createKey(taskName,type,"phi_min"),phi_min);
    printValue(createKey(taskName,type,"phi_max"),phi_max);
    printValue(createKey(taskName,type,"eta_nbins"),eta_nbins);
    printValue(createKey(taskName,type,"eta_min"),eta_min);
    printValue(createKey(taskName,type,"eta_max"),eta_max);
    printValue(createKey(taskName,type,"rapidity_nbins"),rapidity_nbins);
    printValue(createKey(taskName,type,"rapidity_min"),rapidity_minimum);
    printValue(createKey(taskName,type,"rapidity_max"),rapidity_maximum);
    printLine();
    printCR();
    }
  }

  // for now use the same boundaries for eta and y histogram
  void ParticleSingleHistos::create()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_n1         = createHistogram(createName(_histogramBaseName,"n1"),           n1_nbins,  n1_min,  n1_max,  "n_1","N",2);
  h_n1_eTotal  = createHistogram(createName(_histogramBaseName,"n1_eTotal"),    n1_nbins,  n1_min,  10.0*n1_max,  "n1_eTotal","N",2);
  h_n1_pt      = createHistogram(createName(_histogramBaseName,"n1_pt"),        pt_nbins,  pt_min,  pt_max,  "p_{T}","N",2);
  h_n1_ptXS    = createHistogram(createName(_histogramBaseName,"n1_ptXS"),      pt_nbins,  pt_min,  pt_max,  "p_{T}","1/p_{T} dN/p_{T}",2);
  if (eta_fill)
    {
    h_n1_phiEta  = createHistogram(createName(_histogramBaseName,"n1_phiEta"),  eta_nbins, eta_min, eta_max, phi_nbins, phi_min, phi_max, "#eta", "#varphi","N",2);
    if (p2_fill)
      {
      h_spt_phiEta  = createHistogram(createName(_histogramBaseName,"spt_phiEta"),  eta_nbins, eta_min, eta_max, phi_nbins, phi_min, phi_max, "#eta", "#varphi","N",2);
      }
    }
  if (rapidity_fill)
    {
    h_n1_phiY  = createHistogram(createName(_histogramBaseName,"n1_phiY"),     rapidity_nbins,   rapidity_minimum,   rapidity_maximum, phi_nbins, phi_min, phi_max, "y", "#varphi","N",2);
    if (p2_fill)
      {
      h_spt_phiY  = createHistogram(createName(_histogramBaseName,"spt_phiY"),  rapidity_nbins,   rapidity_minimum,   rapidity_maximum, phi_nbins, phi_min, phi_max, "y", "#varphi","N",2);
      }
    }
  if (ptVsRapidity_fill)
    {
    h_n1_ptY  = createHistogram(createName(_histogramBaseName,"n1_ptY"),     rapidity_nbins,   rapidity_minimum,   rapidity_maximum, pt_nbins, pt_min, pt_max, "y", "p_{T}","N",2);
    }
  if (pid_fill)
    h_pdgId  = createHistogram(createName(_histogramBaseName,"n1_indexId"),  6000,  -3000.5, 2999.5, "Index", "N");
  if (reportDebug(__FUNCTION__)) { printCR(); }
  }

  void ParticleSingleHistos::loadFrom(const Folder & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_n1         = importHistogram(inputFile,  createName(_histogramBaseName,"n1"));
  h_n1_eTotal  = importHistogram(inputFile,  createName(_histogramBaseName,"n1_eTotal"));
  h_n1_pt      = importHistogram(inputFile,  createName(_histogramBaseName,"n1_pt"));
  h_n1_ptXS    = importHistogram(inputFile,  createName(_histogramBaseName,"n1_ptXS"));

  if (eta_fill)
    {
    h_n1_phiEta  = importHistogram(inputFile,createName(_histogramBaseName,"n1_phiEta"));
    if (p2_fill)
      {
      h_spt_phiEta  = importHistogram(inputFile,createName(_histogramBaseName,"spt_phiEta"));
      }
    }
  if (rapidity_fill)
    {
    h_n1_phiY  = importHistogram(inputFile,createName(_histogramBaseName,"n1_phiY"));
    if (p2_fill)
      {
      h_spt_phiY  = importHistogram(inputFile,  createName(_histogramBaseName,"spt_phiY"));
      }
    }
  if (ptVsRapidity_fill)
    {
    h_n1_ptY  = importHistogram(inputFile,  createName(_histogramBaseName,"n1_ptY"));
    }

  if (pid_fill)
    h_pdgId  = importHistogram(inputFile,  createName(_histogramBaseName,"n1_indexId"));
  // derived histograms: present once ParticleSingleDerivedCalculator has run
  if (inputFile.contains(createName(_histogramBaseName,"n1_phi"))) loadDerivedFrom(inputFile);
  }

  void ParticleSingleHistos::loadDerivedFrom(const Folder & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_n1_phi = importHistogram(inputFile, createName(_histogramBaseName,"n1_phi"));
  if (p2_fill)
    {
    h_spt_phi    = importHistogram(inputFile, createName(_histogramBaseName,"sum_pt_phi"));
    h_avg_pt_phi = importHistogram(inputFile, createName(_histogramBaseName,"avg_pt_phi"));
    }
  if (eta_fill)
    {
    h_n1_eta = importHistogram(inputFile, createName(_histogramBaseName,"n1_eta"));
    if (p2_fill)
      {
      h_spt_eta       = importHistogram(inputFile, createName(_histogramBaseName,"sum_pt_eta"));
      h_avg_pt_eta    = importHistogram(inputFile, createName(_histogramBaseName,"avg_pt_eta"));
      h_avg_pt_phiEta = importHistogram(inputFile, createName(_histogramBaseName,"avg_pt_phiEta"));
      }
    }
  if (rapidity_fill)
    {
    h_n1_y = importHistogram(inputFile, createName(_histogramBaseName,"n1_y"));
    if (p2_fill)
      {
      h_spt_y       = importHistogram(inputFile, createName(_histogramBaseName,"sum_pt_y"));
      h_avg_pt_y    = importHistogram(inputFile, createName(_histogramBaseName,"avg_pt_y"));
      h_avg_pt_phiY = importHistogram(inputFile, createName(_histogramBaseName,"avg_pt_phiY"));
      }
    }
  }

  void ParticleSingleHistos::createDerived()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_n1_phi = createHistogram(createName(_histogramBaseName,"n1_phi"), phi_nbins, phi_min, phi_max, "#varphi","#rho_{1}(#varphi)");
  if (p2_fill)
    {
    h_spt_phi    = createHistogram(createName(_histogramBaseName,"sum_pt_phi"), phi_nbins, phi_min, phi_max, "#varphi","#sum p_{T}");
    h_avg_pt_phi = createHistogram(createName(_histogramBaseName,"avg_pt_phi"), phi_nbins, phi_min, phi_max, "#varphi","<p_{T}>");
    }
  if (eta_fill)
    {
    h_n1_eta = createHistogram(createName(_histogramBaseName,"n1_eta"), eta_nbins, eta_min, eta_max, "#eta", "#rho_{1}(#eta)");
    if (p2_fill)
      {
      h_spt_eta       = createHistogram(createName(_histogramBaseName,"sum_pt_eta"),    eta_nbins, eta_min, eta_max, "#eta", "#sum p_{T}");
      h_avg_pt_eta    = createHistogram(createName(_histogramBaseName,"avg_pt_eta"),    eta_nbins, eta_min, eta_max, "#eta", "<p_{T}>");
      h_avg_pt_phiEta = createHistogram(createName(_histogramBaseName,"avg_pt_phiEta"), eta_nbins, eta_min, eta_max, phi_nbins, phi_min, phi_max, "#eta", "#varphi", "<p_{T}>");
      }
    }
  if (rapidity_fill)
    {
    h_n1_y = createHistogram(createName(_histogramBaseName,"n1_y"), rapidity_nbins, rapidity_minimum, rapidity_maximum, "y", "#rho_{1}(y)");
    if (p2_fill)
      {
      h_spt_y       = createHistogram(createName(_histogramBaseName,"sum_pt_y"),    rapidity_nbins, rapidity_minimum, rapidity_maximum, "y", "#sum p_{T}");
      h_avg_pt_y    = createHistogram(createName(_histogramBaseName,"avg_pt_y"),    rapidity_nbins, rapidity_minimum, rapidity_maximum, "y", "<p_{T}>");
      h_avg_pt_phiY = createHistogram(createName(_histogramBaseName,"avg_pt_phiY"), rapidity_nbins, rapidity_minimum, rapidity_maximum, phi_nbins, phi_min, phi_max, "y", "#varphi", "<p_{T}>");
      }
    }
  }

  //!
  //! Projections of the (eta, phi) and (y, phi) densities and pT sums, and pT averages. The phi projections are taken
  //! from the (y, phi) histograms when both are filled (as before). The derived histograms are recomputed from scratch,
  //! so calling this function twice gives the same result.
  //!
  void ParticleSingleHistos::calculateDerived()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  if (!h_n1_phi) throw Exception("The derived histograms were not created (createDerived())",__FUNCTION__);
  auto projectOnto = [](const Histogram * source, unsigned int axis, Histogram * target)
    {
    target->reset();
    target->add(source->project2DOnto1D(axis));
    };
  if (eta_fill && h_n1_phiEta)
    {
    projectOnto(h_n1_phiEta,1,h_n1_phi);
    projectOnto(h_n1_phiEta,0,h_n1_eta);
    if (p2_fill && h_spt_phiEta)
      {
      projectOnto(h_spt_phiEta,1,h_spt_phi);
      projectOnto(h_spt_phiEta,0,h_spt_eta);
      calculateInclusiveAvg(h_spt_phi,h_n1_phi,h_avg_pt_phi);
      calculateInclusiveAvg(h_spt_eta,h_n1_eta,h_avg_pt_eta);
      calculateInclusiveAvgH2(h_spt_phiEta,h_n1_phiEta,h_avg_pt_phiEta);
      }
    }
  if (rapidity_fill && h_n1_phiY)
    {
    projectOnto(h_n1_phiY,1,h_n1_phi);
    projectOnto(h_n1_phiY,0,h_n1_y);
    if (p2_fill && h_spt_phiY)
      {
      projectOnto(h_spt_phiY,1,h_spt_phi);
      projectOnto(h_spt_phiY,0,h_spt_y);
      calculateInclusiveAvg(h_spt_phi,h_n1_phi,h_avg_pt_phi);
      calculateInclusiveAvg(h_spt_y,h_n1_y,h_avg_pt_y);
      calculateInclusiveAvgH2(h_spt_phiY,h_n1_phiY,h_avg_pt_phiY);
      }
    }
  }

  //!
  //! Fiil  single particle histograms of this class with the particles contained in the given list.
  //!
  void ParticleSingleHistos::fill(Particle & particle, double weight)
  {
  VectorLorentz & momentum = particle.momentum();
  float pt   = momentum.perp();
  float eta  = momentum.pseudorapidity();
  float phi  = momentum.phi();
  if (phi<0) phi += CAP::MATH::twoPi();
  float rapidity = momentum.rapidity();
  h_n1_pt  ->fill(pt,weight);
  if (pt>0) h_n1_ptXS->fill(pt,weight/pt);
  if (eta_fill)
    {
    h_n1_phiEta->fill(eta,phi,weight);
    if (p2_fill) h_spt_phiEta->fill(eta,phi,weight*pt);
    }
  if (rapidity_fill)
    {
    h_n1_phiY->fill(rapidity,phi,weight);
    if (p2_fill) h_spt_phiY->fill(rapidity,phi,weight*pt);
    }
  if (ptVsRapidity_fill)
    {
    h_n1_ptY->fill(rapidity,pt,weight);
    }
  if (pid_fill)
    {
    int pdgIndex = particle.type().pdgCode();
    h_pdgId->fill(pdgIndex,weight);
    }
  }

  //!
  //! Fiil the global histogram for the multiplicity and the total energy of an event.
  //! Call this function only once per event.
  //!
  void ParticleSingleHistos::fillMultiplicity(double nAccepted, double totalEnergy, double weight)
  {
  h_n1->fill(nAccepted, weight);
  h_n1_eTotal->fill(totalEnergy, weight);
  }

  void ParticleSingleHistos::scaleObject(double factor)
  {
  h_n1->scale(factor);
  h_n1_eTotal->scale(factor);
  h_n1_pt->scale(factor);
  h_n1_ptXS->scale(factor);  // 1/pt dN/dptdy
  if (eta_fill)
    {
    h_n1_phiEta->scale(factor);
    if (p2_fill) h_spt_phiEta->scale(factor);
    }
  if (rapidity_fill)
    {
    h_n1_phiY->scale(factor);
    if (p2_fill) h_spt_phiY->scale(factor);
    }
  if (ptVsRapidity_fill)
    {
    h_n1_ptY->scale(factor);
    }
  if (pid_fill)
    {
    h_pdgId->scale(factor);
    }


  }


} // namespace CAP
