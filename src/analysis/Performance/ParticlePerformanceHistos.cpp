/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ParticlePerformanceHistos.hpp"
#include "Exceptions.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "Task.hpp"
#include <cmath>

namespace CAP
{

  namespace
  {
    //! Configuration keys of the axes and of the differences: <prefix>_nbins, <prefix>_min, <prefix>_max.
    const char * const axisKey[ParticlePerformanceHistos::nAxes]        = { "pt", "eta", "phi", "rapidity" };
    const char * const diffKey[ParticlePerformanceHistos::nQuantities]  = { "dpt", "Deta", "Dphi", "Drapidity" };
  }

  ParticlePerformanceHistos::ParticlePerformanceHistos()
  :
  HistogramFolder(),
  eta_fill(true),
  rapidity_fill(true),
  phi_fill(false),
  ptEta_fill(true),
  h_gen2(nullptr),
  h_rec2(nullptr),
  h_eff2(nullptr)
  {
  appendClassName("ParticlePerformanceHistos");
  setName("ParticlePerformanceHistos");
  setTitle("ParticlePerformanceHistos");
  for (int a=0; a<nAxes; a++)       { axis_nbins[a] = 0; axis_min[a] = 0.0; axis_max[a] = 0.0; h_gen[a] = h_rec[a] = h_eff[a] = nullptr; }
  for (int q=0; q<nQuantities; q++)
    {
    diff_nbins[q] = 0; diff_min[q] = 0.0; diff_max[q] = 0.0; h_n1_d[q] = nullptr; p_d2[q] = nullptr; h_rms2[q] = nullptr;
    for (int a=0; a<kY; a++) { p_d[q][a] = nullptr; h_rms[q][a] = nullptr; }
    }
  ptEta_nbins[0] = ptEta_nbins[1] = 0;
  }

  ParticlePerformanceHistos::ParticlePerformanceHistos(const ParticlePerformanceHistos & source)
  :
  HistogramFolder(source),
  eta_fill(source.eta_fill),
  rapidity_fill(source.rapidity_fill),
  phi_fill(source.phi_fill),
  ptEta_fill(source.ptEta_fill)
  {
  ptEta_nbins[0] = source.ptEta_nbins[0]; ptEta_nbins[1] = source.ptEta_nbins[1];
  for (int a=0; a<nAxes; a++)       { axis_nbins[a] = source.axis_nbins[a]; axis_min[a] = source.axis_min[a]; axis_max[a] = source.axis_max[a]; }
  for (int q=0; q<nQuantities; q++) { diff_nbins[q] = source.diff_nbins[q]; diff_min[q] = source.diff_min[q]; diff_max[q] = source.diff_max[q]; }
  cloneB(source);
  }

  ParticlePerformanceHistos & ParticlePerformanceHistos::operator=(const ParticlePerformanceHistos & rhs)
  {
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    eta_fill      = rhs.eta_fill;
    rapidity_fill = rhs.rapidity_fill;
    phi_fill      = rhs.phi_fill;
    ptEta_fill    = rhs.ptEta_fill;
    ptEta_nbins[0] = rhs.ptEta_nbins[0]; ptEta_nbins[1] = rhs.ptEta_nbins[1];
    for (int a=0; a<nAxes; a++)       { axis_nbins[a] = rhs.axis_nbins[a]; axis_min[a] = rhs.axis_min[a]; axis_max[a] = rhs.axis_max[a]; }
    for (int q=0; q<nQuantities; q++) { diff_nbins[q] = rhs.diff_nbins[q]; diff_min[q] = rhs.diff_min[q]; diff_max[q] = rhs.diff_max[q]; }
    cloneB(rhs);
    }
  return *this;
  }

  void ParticlePerformanceHistos::cloneB(const ParticlePerformanceHistos & source)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  for (int a=0; a<nAxes; a++)
    {
    h_gen[a] = histogramLike(source.h_gen[a]);
    h_rec[a] = histogramLike(source.h_rec[a]);
    h_eff[a] = histogramLike(source.h_eff[a]);
    }
  h_gen2 = histogramLike(source.h_gen2);
  h_rec2 = histogramLike(source.h_rec2);
  h_eff2 = histogramLike(source.h_eff2);
  for (int q=0; q<nQuantities; q++)
    {
    h_n1_d[q] = histogramLike(source.h_n1_d[q]);
    p_d2[q]   = histogramLike(source.p_d2[q]);
    h_rms2[q] = histogramLike(source.h_rms2[q]);
    for (int a=0; a<kY; a++)
      {
      p_d[q][a]   = histogramLike(source.p_d[q][a]);
      h_rms[q][a] = histogramLike(source.h_rms[q][a]);
      }
    }
  }

  void ParticlePerformanceHistos::addDefaultConfiguration(Configuration & configuration, const String & taskName)
  {
  const String type = "HISTOGRAM";
  configuration.addProperty(createKey(taskName,type,"pt_nbins"),      100);
  configuration.addProperty(createKey(taskName,type,"pt_min"),        0.0);
  configuration.addProperty(createKey(taskName,type,"pt_max"),        5.0);
  configuration.addProperty(createKey(taskName,type,"dpt_nbins"),     100);
  configuration.addProperty(createKey(taskName,type,"dpt_min"),       -1.0);
  configuration.addProperty(createKey(taskName,type,"dpt_max"),       1.0);
  configuration.addProperty(createKey(taskName,type,"phi_nbins"),     36);
  configuration.addProperty(createKey(taskName,type,"phi_min"),       0.0);
  configuration.addProperty(createKey(taskName,type,"phi_max"),       CAP::MATH::twoPi());
  configuration.addProperty(createKey(taskName,type,"Dphi_nbins"),    100);
  configuration.addProperty(createKey(taskName,type,"Dphi_min"),      -1.0);
  configuration.addProperty(createKey(taskName,type,"Dphi_max"),      1.0);
  configuration.addProperty(createKey(taskName,type,"eta_nbins"),     100);
  configuration.addProperty(createKey(taskName,type,"eta_min"),       -1.0);
  configuration.addProperty(createKey(taskName,type,"eta_max"),       1.0);
  configuration.addProperty(createKey(taskName,type,"Deta_nbins"),    100);
  configuration.addProperty(createKey(taskName,type,"Deta_min"),      -1.0);
  configuration.addProperty(createKey(taskName,type,"Deta_max"),      1.0);
  configuration.addProperty(createKey(taskName,type,"rapidity_nbins"), 100);
  configuration.addProperty(createKey(taskName,type,"rapidity_min"),  -1.0);
  configuration.addProperty(createKey(taskName,type,"rapidity_max"),  1.0);
  configuration.addProperty(createKey(taskName,type,"Drapidity_nbins"), 100);
  configuration.addProperty(createKey(taskName,type,"Drapidity_min"), -1.0);
  configuration.addProperty(createKey(taskName,type,"Drapidity_max"), 1.0);
  configuration.addProperty(createKey(taskName,type,"eta_fill"),      true);
  configuration.addProperty(createKey(taskName,type,"rapidity_fill"), true);
  configuration.addProperty(createKey(taskName,type,"phi_fill"),      false);
  configuration.addProperty(createKey(taskName,type,"ptEta_fill"),    true);
  configuration.addProperty(createKey(taskName,type,"ptEta_pt_nbins"),  20);
  configuration.addProperty(createKey(taskName,type,"ptEta_eta_nbins"), 10);
  }

  void ParticlePerformanceHistos::configure(const String & taskName,
                                            const String & objectType,
                                            const Configuration & configuration,
                                            unsigned int index)
  {
  HistogramFolder::configure(taskName,objectType,configuration,index);
  // the binning is in the HISTOGRAM group of the task, whatever the list of the folder (HISTOGRAM_1, ...)
  String type = objectType.contains("HISTOGRAM") ? String("HISTOGRAM") : objectType;
  eta_fill      = configuration.valueBool(createKey(taskName,type,"eta_fill"));
  rapidity_fill = configuration.valueBool(createKey(taskName,type,"rapidity_fill"));
  phi_fill      = configuration.valueBool(createKey(taskName,type,"phi_fill"));
  ptEta_fill    = configuration.valueBool(createKey(taskName,type,"ptEta_fill"));
  ptEta_nbins[0] = configuration.valueInt(createKey(taskName,type,"ptEta_pt_nbins"));
  ptEta_nbins[1] = configuration.valueInt(createKey(taskName,type,"ptEta_eta_nbins"));
  for (int a=0; a<nAxes; a++)
    {
    String key = axisKey[a];
    axis_nbins[a] = configuration.valueInt(   createKey(taskName,type,key+"_nbins"));
    axis_min[a]   = configuration.valueDouble(createKey(taskName,type,key+"_min"));
    axis_max[a]   = configuration.valueDouble(createKey(taskName,type,key+"_max"));
    }
  for (int q=0; q<nQuantities; q++)
    {
    String key = diffKey[q];
    diff_nbins[q] = configuration.valueInt(   createKey(taskName,type,key+"_nbins"));
    diff_min[q]   = configuration.valueDouble(createKey(taskName,type,key+"_min"));
    diff_max[q]   = configuration.valueDouble(createKey(taskName,type,key+"_max"));
    }
  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printValue(createKey(taskName,type,"eta_fill"),eta_fill);
    printValue(createKey(taskName,type,"rapidity_fill"),rapidity_fill);
    printValue(createKey(taskName,type,"phi_fill"),phi_fill);
    printValue(createKey(taskName,type,"ptEta_fill"),ptEta_fill);
    printValue(createKey(taskName,type,"ptEta_pt_nbins"),ptEta_nbins[0]);
    printValue(createKey(taskName,type,"ptEta_eta_nbins"),ptEta_nbins[1]);
    for (int a=0; a<nAxes; a++)
      {
      String key = axisKey[a];
      printValue(createKey(taskName,type,key+"_nbins"),axis_nbins[a]);
      printValue(createKey(taskName,type,key+"_min"),axis_min[a]);
      printValue(createKey(taskName,type,key+"_max"),axis_max[a]);
      }
    for (int q=0; q<nQuantities; q++)
      {
      String key = diffKey[q];
      printValue(createKey(taskName,type,key+"_nbins"),diff_nbins[q]);
      printValue(createKey(taskName,type,key+"_min"),diff_min[q]);
      printValue(createKey(taskName,type,key+"_max"),diff_max[q]);
      }
    printLine();
    printCR();
    }
  }

  const char * ParticlePerformanceHistos::quantityName(Quantity q)
  {
  static const char * const names[nQuantities] = { "dpt", "deta", "dphi", "dy" };
  return names[q];
  }

  const char * ParticlePerformanceHistos::axisName(Axis a)
  {
  static const char * const names[nAxes] = { "Pt", "Eta", "Phi", "Y" };
  return names[a];
  }

  const char * ParticlePerformanceHistos::quantityTitle(Quantity q)
  {
  static const char * const titles[nQuantities] = { "#Delta p_{T}", "#Delta#eta", "#Delta#varphi", "#Delta y" };
  return titles[q];
  }

  const char * ParticlePerformanceHistos::axisTitle(Axis a)
  {
  static const char * const titles[nAxes] = { "p_{T}", "#eta", "#varphi", "y" };
  return titles[a];
  }

  String ParticlePerformanceHistos::differenceName(Quantity q) const
  {
  return createName(_histogramBaseName,String("n1_")+quantityName(q));
  }

  String ParticlePerformanceHistos::profileName(Quantity q, Axis a, bool rms) const
  {
  String name = String(rms ? "rms_" : "")+quantityName(q)+"Vs"+axisName(a);
  return createName(_histogramBaseName,name);
  }

  String ParticlePerformanceHistos::countName(Axis a, int kind) const
  {
  static const char * const prefixes[3] = { "n1_gen", "n1_rec", "effVs" };
  return createName(_histogramBaseName,String(prefixes[kind])+axisName(a));
  }

  String ParticlePerformanceHistos::profileNamePtEta(Quantity q, bool rms) const
  {
  return createName(_histogramBaseName,String(rms ? "rms_" : "")+quantityName(q)+"VsPtEta");
  }

  String ParticlePerformanceHistos::countNamePtEta(int kind) const
  {
  static const char * const prefixes[3] = { "n1_gen", "n1_rec", "effVs" };
  return createName(_histogramBaseName,String(prefixes[kind])+"PtEta");
  }

  double ParticlePerformanceHistos::axisValue(Axis a, const VectorLorentz & p) const
  {
  switch (a)
    {
    case kPt:  return p.perp();
    case kEta: return p.pseudorapidity();
    case kY:   return p.rapidity();
    case kPhi:
      {
      double phi = p.phi();                                 // (-pi, pi]: mapped onto [phi_min, phi_min + 2pi)
      while (phi <  axis_min[kPhi])                    phi += MATH::twoPi();
      while (phi >= axis_min[kPhi] + MATH::twoPi())    phi -= MATH::twoPi();
      return phi;
      }
    default: return 0.0;
    }
  }

  void ParticlePerformanceHistos::create()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  for (int iq=0; iq<nQuantities; iq++)
    {
    Quantity q = Quantity(iq);
    if (!uses(q)) continue;
    h_n1_d[q] = createHistogram(differenceName(q),diff_nbins[q],diff_min[q],diff_max[q],quantityTitle(q),"N");
    for (int ia=0; ia<kY; ia++)
      {
      Axis a = Axis(ia);
      if (!profilesVs(a)) continue;
      p_d[q][a] = createProfile(profileName(q,a),axis_nbins[a],axis_min[a],axis_max[a],axisTitle(a),quantityTitle(q));
      }
    }
  for (int ia=0; ia<nAxes; ia++)
    {
    Axis a = Axis(ia);
    if (!countsVs(a)) continue;
    h_gen[a] = createHistogram(countName(a,0),axis_nbins[a],axis_min[a],axis_max[a],axisTitle(a),"N_{gen}");
    h_rec[a] = createHistogram(countName(a,1),axis_nbins[a],axis_min[a],axis_max[a],axisTitle(a),"N_{rec}");
    }
  if (!ptEta_fill) return;
  for (int iq=0; iq<nQuantities; iq++)
    {
    Quantity q = Quantity(iq);
    if (!uses(q)) continue;
    p_d2[q] = createProfile(profileNamePtEta(q),ptEta_nbins[0],axis_min[kPt],axis_max[kPt],
                            ptEta_nbins[1],axis_min[kEta],axis_max[kEta],axisTitle(kPt),axisTitle(kEta),quantityTitle(q));
    }
  h_gen2 = createHistogram(countNamePtEta(0),ptEta_nbins[0],axis_min[kPt],axis_max[kPt],
                           ptEta_nbins[1],axis_min[kEta],axis_max[kEta],axisTitle(kPt),axisTitle(kEta),"N_{gen}");
  h_rec2 = createHistogram(countNamePtEta(1),ptEta_nbins[0],axis_min[kPt],axis_max[kPt],
                           ptEta_nbins[1],axis_min[kEta],axis_max[kEta],axisTitle(kPt),axisTitle(kEta),"N_{rec}");
  }

  void ParticlePerformanceHistos::createDerived()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  for (int iq=0; iq<nQuantities; iq++)
    {
    Quantity q = Quantity(iq);
    if (!uses(q)) continue;
    for (int ia=0; ia<kY; ia++)
      {
      Axis a = Axis(ia);
      if (!profilesVs(a)) continue;
      h_rms[q][a] = createHistogram(profileName(q,a,true),axis_nbins[a],axis_min[a],axis_max[a],axisTitle(a),
                                    String("#sigma(")+quantityTitle(q)+")");
      }
    }
  for (int ia=0; ia<nAxes; ia++)
    {
    Axis a = Axis(ia);
    if (!countsVs(a)) continue;
    h_eff[a] = createHistogram(countName(a,2),axis_nbins[a],axis_min[a],axis_max[a],axisTitle(a),"#varepsilon");
    }
  if (!ptEta_fill) return;
  for (int iq=0; iq<nQuantities; iq++)
    {
    Quantity q = Quantity(iq);
    if (!uses(q)) continue;
    h_rms2[q] = createHistogram(profileNamePtEta(q,true),ptEta_nbins[0],axis_min[kPt],axis_max[kPt],
                                ptEta_nbins[1],axis_min[kEta],axis_max[kEta],axisTitle(kPt),axisTitle(kEta),
                                String("#sigma(")+quantityTitle(q)+")");
    }
  h_eff2 = createHistogram(countNamePtEta(2),ptEta_nbins[0],axis_min[kPt],axis_max[kPt],
                           ptEta_nbins[1],axis_min[kEta],axis_max[kEta],axisTitle(kPt),axisTitle(kEta),"#varepsilon");
  }

  void ParticlePerformanceHistos::loadFrom(const Folder & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  for (int iq=0; iq<nQuantities; iq++)
    {
    Quantity q = Quantity(iq);
    if (!uses(q)) continue;
    h_n1_d[q] = importHistogram(inputFile,differenceName(q));
    for (int ia=0; ia<kY; ia++)
      {
      Axis a = Axis(ia);
      if (profilesVs(a)) p_d[q][a] = importProfile(inputFile,profileName(q,a));
      }
    }
  for (int ia=0; ia<nAxes; ia++)
    {
    Axis a = Axis(ia);
    if (!countsVs(a)) continue;
    h_gen[a] = importHistogram(inputFile,countName(a,0));
    h_rec[a] = importHistogram(inputFile,countName(a,1));
    }
  if (ptEta_fill)
    {
    for (int iq=0; iq<nQuantities; iq++)
      if (uses(Quantity(iq))) p_d2[iq] = importProfile(inputFile,profileNamePtEta(Quantity(iq)));
    h_gen2 = importHistogram(inputFile,countNamePtEta(0));
    h_rec2 = importHistogram(inputFile,countNamePtEta(1));
    }
  loadDerivedFrom(inputFile);   // the derived histograms, if the file has them (all)
  }

  void ParticlePerformanceHistos::loadDerivedFrom(const Folder & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  importAllOrNone(inputFile,derivedHistograms());
  }

  HistogramFolder::HistogramList ParticlePerformanceHistos::derivedHistograms()
  {
  HistogramList list;
  for (int iq=0; iq<nQuantities; iq++)
    {
    Quantity q = Quantity(iq);
    if (!uses(q)) continue;
    for (int ia=0; ia<kY; ia++)
      {
      Axis a = Axis(ia);
      if (profilesVs(a)) list.push_back({&h_rms[q][a],profileName(q,a,true)});
      }
    }
  for (int ia=0; ia<nAxes; ia++)
    {
    Axis a = Axis(ia);
    if (countsVs(a)) list.push_back({&h_eff[a],countName(a,2)});
    }
  if (ptEta_fill)
    {
    for (int iq=0; iq<nQuantities; iq++)
      if (uses(Quantity(iq))) list.push_back({&h_rms2[iq],profileNamePtEta(Quantity(iq),true)});
    list.push_back({&h_eff2,countNamePtEta(2)});
    }
  return list;
  }

  double ParticlePerformanceHistos::deltaPhi(double a, double b)
  {
  double d = std::fmod(a-b+MATH::pi(),MATH::twoPi());
  if (d<0.0) d += MATH::twoPi();
  return d-MATH::pi();
  }

  //!
  //! Fill the histograms with the differences between a detector-level particle and its truth particle, and count it
  //! at the generator-level values (efficiency numerator).
  //!
  void ParticlePerformanceHistos::fill(Particle & recoParticle, double weight)
  {
  if (!recoParticle.hasTruth())
    throw Exception("A particle has no truth particle: the performance histograms need detector-level particles made "
                    "by MeasurementSimulator",__FUNCTION__);
  const VectorLorentz & reco = recoParticle.momentum();
  const VectorLorentz & gen  = recoParticle.truth()->momentum();
  double d[nQuantities];
  d[kDpt]  = reco.perp()           - gen.perp();
  d[kDeta] = reco.pseudorapidity() - gen.pseudorapidity();
  d[kDphi] = deltaPhi(reco.phi(),gen.phi());
  d[kDy]   = rapidity_fill ? reco.rapidity() - gen.rapidity() : 0.0;
  double x[nAxes];
  for (int ia=0; ia<nAxes; ia++) x[ia] = countsVs(Axis(ia)) ? axisValue(Axis(ia),gen) : 0.0;
  for (int iq=0; iq<nQuantities; iq++)
    {
    if (!h_n1_d[iq]) continue;
    h_n1_d[iq]->fill(d[iq],weight);
    for (int ia=0; ia<kY; ia++) if (p_d[iq][ia]) p_d[iq][ia]->fill(x[ia],d[iq],weight);
    }
  for (int ia=0; ia<nAxes; ia++) if (h_rec[ia]) h_rec[ia]->fill(x[ia],weight);
  if (!h_rec2) return;
  double pt = gen.perp(), eta = gen.pseudorapidity();
  for (int iq=0; iq<nQuantities; iq++) if (p_d2[iq]) p_d2[iq]->fill(pt,eta,d[iq],weight);
  h_rec2->fill(pt,eta,weight);
  }

  void ParticlePerformanceHistos::fillGenerated(const Particle & genParticle, double weight)
  {
  const VectorLorentz & momentum = genParticle.momentum();
  for (int ia=0; ia<nAxes; ia++) if (h_gen[ia]) h_gen[ia]->fill(axisValue(Axis(ia),momentum),weight);
  if (h_gen2) h_gen2->fill(momentum.perp(),momentum.pseudorapidity(),weight);
  }

  void ParticlePerformanceHistos::scaleObject(double factor)
  {
  for (int iq=0; iq<nQuantities; iq++) if (h_n1_d[iq]) h_n1_d[iq]->scale(factor);
  for (int ia=0; ia<nAxes; ia++)
    {
    if (h_gen[ia]) h_gen[ia]->scale(factor);
    if (h_rec[ia]) h_rec[ia]->scale(factor);
    }
  if (h_gen2) h_gen2->scale(factor);
  if (h_rec2) h_rec2->scale(factor);
  }

  void ParticlePerformanceHistos::calculateRms(const Profile & profile, Histogram & rms)
  {
  rms.reset();
  unsigned int nBins = profile.nBinsTotal();
  for (unsigned int k=0; k<nBins; k++)
    {
    double sw  = profile.sumWeights(k);
    double sw2 = profile.sumWeights2(k);
    if (profile.nEntries(k)<2 || sw<=0.0) continue;
    double mean     = profile.sumWeightedValues(k)/sw;
    double variance = profile.sumWeightedValues2(k)/sw - mean*mean;
    if (variance<0.0) variance = 0.0;
    double sigma = std::sqrt(variance);
    double nEff  = sw2>0.0 ? sw*sw/sw2 : 0.0;
    rms.setBinValueAtGlobal(k,sigma);
    rms.setBinUncertaintyAtGlobal(k,nEff>1.0 ? sigma/std::sqrt(2.0*(nEff-1.0)) : 0.0);
    }
  }

  void ParticlePerformanceHistos::calculateEfficiency(const Histogram & passed, const Histogram & total, Histogram & eff)
  {
  eff.reset();
  unsigned int nBins = total.nBinsTotal();
  for (unsigned int k=0; k<nBins; k++)
    {
    double n = total.binValueAtGlobal(k);
    if (n<=0.0) continue;
    double e = passed.binValueAtGlobal(k)/n;
    double p = e<0.0 ? 0.0 : (e>1.0 ? 1.0 : e);       // binomial uncertainty; an efficiency above 1 is migration
    eff.setBinValueAtGlobal(k,e);
    eff.setBinUncertaintyAtGlobal(k,std::sqrt(p*(1.0-p)/n));
    }
  }

  //!
  //! Resolutions: rms of reco - gen in each bin of the profiles (the profiles themselves give the biases).
  //! Efficiencies: detector-level / generator-level counts vs each axis.
  //!
  void ParticlePerformanceHistos::calculateDerived()
  {
  if (!hasDerived()) throw Exception("The derived histograms were not created (createDerived())",__FUNCTION__);
  for (int iq=0; iq<nQuantities; iq++)
    for (int ia=0; ia<kY; ia++)
      {
      if (!h_rms[iq][ia]) continue;
      if (!p_d[iq][ia]) throw Exception(String("No profile ")+profileName(Quantity(iq),Axis(ia)),__FUNCTION__);
      calculateRms(*p_d[iq][ia],*h_rms[iq][ia]);
      }
  for (int ia=0; ia<nAxes; ia++)
    {
    if (!h_eff[ia]) continue;
    if (!h_gen[ia] || !h_rec[ia]) throw Exception(String("No counts ")+countName(Axis(ia),0),__FUNCTION__);
    calculateEfficiency(*h_rec[ia],*h_gen[ia],*h_eff[ia]);
    }
  for (int iq=0; iq<nQuantities; iq++)
    {
    if (!h_rms2[iq]) continue;
    if (!p_d2[iq]) throw Exception(String("No profile ")+profileNamePtEta(Quantity(iq)),__FUNCTION__);
    calculateRms(*p_d2[iq],*h_rms2[iq]);
    }
  if (h_eff2)
    {
    if (!h_gen2 || !h_rec2) throw Exception(String("No counts ")+countNamePtEta(0),__FUNCTION__);
    calculateEfficiency(*h_rec2,*h_gen2,*h_eff2);
    }
  }

} // namespace CAP
