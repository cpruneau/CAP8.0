/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ParticleSingleDerivedHistos.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "Task.hpp"
#include "NameHelpers.hpp"
#include "Configuration.hpp"


namespace CAP
{

ParticleSingleDerivedHistos::ParticleSingleDerivedHistos()
:
ParticleSingleHistos(),
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
  appendClassName("ParticleSingleDerivedHistos");
  setName("ParticleSingleDerivedHistos");
  setTitle("ParticleSingleDerivedHistos");
}

ParticleSingleDerivedHistos::ParticleSingleDerivedHistos(const ParticleSingleDerivedHistos & source)
:
ParticleSingleHistos(source),
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
  cloneD(source);
}

ParticleSingleDerivedHistos & ParticleSingleDerivedHistos::operator=(const ParticleSingleDerivedHistos & source)
{
  if (this!=&source)
    {
    ParticleSingleHistos::operator=(source);
    cloneD(source);
    }
  return *this;
}

void ParticleSingleDerivedHistos::cloneD(const ParticleSingleDerivedHistos & source)
{
  // The base-class histograms were cloned by the ParticleSingleHistos copy constructor or assignment;
  // clone (and register in the group) the derived ones only.
  if (reportStart(__FUNCTION__)) { printCR(); }
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
}

void ParticleSingleDerivedHistos::configure(const String & taskName,
                                                  const String & objectType,
                                                  const Configuration & configuration,
                                                  unsigned int index)
{
  ParticleSingleHistos::configure(taskName,objectType,configuration,index);
}

void ParticleSingleDerivedHistos::create()
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_n1_phi = createHistogram(createName(_histogramBaseName,"n1_phi"), phi_nbins, phi_min, phi_max, "#varphi","#rho_{1}(#varphi)");
  if (p2_fill)
    {
    h_spt_phi = createHistogram(createName(_histogramBaseName,"sum_pt_phi"), phi_nbins, phi_min, phi_max, "#varphi","#sum p_{T}");
    h_avg_pt_phi = createHistogram(createName(_histogramBaseName,"avg_pt_phi"),     phi_nbins, phi_min, phi_max, "#varphi","<p_{T}>");
    }

  if (eta_fill)
    {
    h_n1_eta = createHistogram(createName(_histogramBaseName,"n1_eta"),     eta_nbins, eta_min, eta_max, "#eta",   "#rho_{1}(#eta)");
    if (p2_fill)
      {
      h_spt_eta     = createHistogram(createName(_histogramBaseName,"sum_pt_eta"),  eta_nbins, eta_min, eta_max, "#eta",   "#sum p_{T}");
      h_avg_pt_eta     = createHistogram(createName(_histogramBaseName,"avg_pt_eta"),     eta_nbins, eta_min, eta_max, "#eta",   "<p_{T}>");
      h_avg_pt_phiEta  = createHistogram(createName(_histogramBaseName,"avg_pt_phiEta"),  eta_nbins, eta_min, eta_max, phi_nbins, phi_min, phi_max, "#eta", "#varphi","N");
      }
    }
  if (rapidity_fill)
    {
    h_n1_y = createHistogram(createName(_histogramBaseName,"n1_y"),     rapidity_nbins, rapidity_minimum,   rapidity_maximum, "y",   "#rho_{1}(y)");
    if (p2_fill)
      {
      h_spt_y         = createHistogram(createName(_histogramBaseName,"sum_pt_y"),     rapidity_nbins, rapidity_minimum,   rapidity_maximum, "y",   "#sum p_{T}");
      h_avg_pt_y      = createHistogram(createName(_histogramBaseName,"avg_pt_y"),     rapidity_nbins, rapidity_minimum,   rapidity_maximum, "y",   "<p_{T}>");
      h_avg_pt_phiY   = createHistogram(createName(_histogramBaseName,"avg_pt_phiY"),  rapidity_nbins, rapidity_minimum,   rapidity_maximum, phi_nbins, phi_min, phi_max, "y", "#varphi","N");
      }
    }
}

void ParticleSingleDerivedHistos::loadFrom(const Folder & inputFile)
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_n1_phi      = importHistogram(inputFile,  createName(_histogramBaseName,"n1_phi"));
  if (p2_fill)
    {
    h_spt_phi     = importHistogram(inputFile,  createName(_histogramBaseName,"sum_pt_phi"));
    h_avg_pt_phi  = importHistogram(inputFile,  createName(_histogramBaseName,"avg_pt_phi"));
    }

  if (eta_fill)
    {
    h_n1_eta     = importHistogram(inputFile,  createName(_histogramBaseName,"n1_eta"));
    if (p2_fill)
      {
      h_spt_eta         = importHistogram(inputFile,  createName(_histogramBaseName,"sum_pt_eta"));
      h_avg_pt_eta      = importHistogram(inputFile,  createName(_histogramBaseName,"avg_pt_eta"));
      h_avg_pt_phiEta   = importHistogram(inputFile,  createName(_histogramBaseName,"avg_pt_phiEta"));
      }
    }
  if (rapidity_fill)
    {
    h_n1_y     = importHistogram(inputFile,  createName(_histogramBaseName,"n1_y"));
    if (p2_fill)
      {
      h_spt_y         = importHistogram(inputFile,  createName(_histogramBaseName,"sum_pt_y"));
      h_avg_pt_y      = importHistogram(inputFile,  createName(_histogramBaseName,"avg_pt_y"));
      h_avg_pt_phiY   = importHistogram(inputFile,  createName(_histogramBaseName,"avg_pt_phiY"));
      }
    }
}

//!
//! Calculates derived histograms. This is currently limited to pt averages vs eta and phi.
//!
void ParticleSingleDerivedHistos::calculateDerivedHistograms(ParticleSingleHistos & baseHistos)
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  Histogram* hTemp;
  if (baseHistos.h_n1_phiEta)
    {
    hTemp = new Histogram(baseHistos.h_n1_phiEta->project2DOnto1D(1));
    h_n1_phi->add(*hTemp);
    delete hTemp;

    hTemp = new Histogram(baseHistos.h_n1_phiEta->project2DOnto1D(0));
    h_n1_eta->add(*hTemp);
    delete hTemp;

    if (p2_fill  &&  baseHistos.h_spt_phiEta)
      {
      hTemp = new Histogram(baseHistos.h_spt_phiEta->project2DOnto1D(1));
      h_spt_phi->add(*hTemp);
      delete hTemp;
      hTemp = new Histogram(baseHistos.h_spt_phiEta->project2DOnto1D(0));
      h_spt_eta->add(*hTemp);
      delete hTemp;

      calculateInclusiveAvg(h_spt_phi,h_n1_phi,h_avg_pt_phi);
      calculateInclusiveAvg(h_spt_eta,h_n1_eta,h_avg_pt_eta);
      calculateInclusiveAvgH2(baseHistos.h_spt_phiEta,baseHistos.h_n1_phiEta,h_avg_pt_phiEta);
      }
    }

  if (baseHistos.h_n1_phiY)
    {
    hTemp = new Histogram(baseHistos.h_n1_phiY->project2DOnto1D(1));
    h_n1_phi->reset();
    h_n1_phi->add(*hTemp);
    delete hTemp;

    hTemp = new Histogram(baseHistos.h_n1_phiY->project2DOnto1D(0));
    h_n1_y->add(*hTemp);
    delete hTemp;

    if (p2_fill  &&  baseHistos.h_spt_phiY)
      {
      hTemp = new Histogram(baseHistos.h_spt_phiY->project2DOnto1D(1));
      h_spt_phi->reset();   // like h_n1_phi above: phi projections are recomputed from the y histograms
      h_spt_phi->add(*hTemp);
      delete hTemp;
      hTemp = new Histogram(baseHistos.h_spt_phiY->project2DOnto1D(0));
      h_spt_y->add(*hTemp);
      delete hTemp;

      calculateInclusiveAvg(h_spt_phi,h_n1_phi,h_avg_pt_phi);
      calculateInclusiveAvg(h_spt_y,h_n1_y,h_avg_pt_y);
      calculateInclusiveAvgH2(baseHistos.h_spt_phiY,baseHistos.h_n1_phiY,h_avg_pt_phiY);
      }
    }
}

} // namespace cap

