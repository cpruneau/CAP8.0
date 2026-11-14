/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetGroomingHistos.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"

namespace CAP
{

JetGroomingHistos::JetGroomingHistos()
:
HistogramFolder(),
zg_nbins(50),
rg_nbins(40), rg_max(0.8),
ktg_nbins(50), ktg_max(10.0),
nSD_nbins(15), nSD_max(15.0),
pt_nbins(100), pt_min(0.0), pt_max(100.0),
mass_nbins(50), mass_max(50.0),
mult_nbins(50), mult_max(50.0),
ptLoss_nbins(50),
_jetCount(0.0),
h_status(nullptr),
h_zg(nullptr), h_rg(nullptr), h_ktg(nullptr), h_nSD(nullptr),
h_pt(nullptr), h_mass(nullptr), h_mult(nullptr), h_ptLoss(nullptr), h_massVsMass(nullptr)
{
  appendClassName("JetGroomingHistos");
  setName("JetGroomingHistos");
  setTitle("JetGroomingHistos");
  setHistogramBaseName("JetGroomed");
}

JetGroomingHistos::JetGroomingHistos(const JetGroomingHistos & source)
:
HistogramFolder(source)
{
  *this = source;
}

JetGroomingHistos & JetGroomingHistos::operator=(const JetGroomingHistos & rhs)
{
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    zg_nbins = rhs.zg_nbins;
    rg_nbins = rhs.rg_nbins;  rg_max = rhs.rg_max;
    ktg_nbins = rhs.ktg_nbins;  ktg_max = rhs.ktg_max;
    nSD_nbins = rhs.nSD_nbins;  nSD_max = rhs.nSD_max;
    pt_nbins = rhs.pt_nbins;  pt_min = rhs.pt_min;  pt_max = rhs.pt_max;
    mass_nbins = rhs.mass_nbins;  mass_max = rhs.mass_max;
    mult_nbins = rhs.mult_nbins;  mult_max = rhs.mult_max;
    ptLoss_nbins = rhs.ptLoss_nbins;
    _jetCount = rhs._jetCount;
    cloneB(rhs);
    }
  return *this;
}

void JetGroomingHistos::cloneB(const JetGroomingHistos & source)
{
  h_status = histogramLike(source.h_status);
  h_zg = histogramLike(source.h_zg);  h_rg = histogramLike(source.h_rg);  h_ktg = histogramLike(source.h_ktg);
  h_nSD = histogramLike(source.h_nSD);
  h_pt = histogramLike(source.h_pt);  h_mass = histogramLike(source.h_mass);  h_mult = histogramLike(source.h_mult);
  h_ptLoss = histogramLike(source.h_ptLoss);  h_massVsMass = histogramLike(source.h_massVsMass);
}

void JetGroomingHistos::addDefaultConfiguration(Configuration & configuration, const String & taskName)
{
  auto add = [&](const char * name, auto value) { configuration.addProperty(createKey(taskName,"HISTOGRAM",name),value); };
  add("zg_nbins",50);
  add("rg_nbins",40);      add("rg_max",0.8);
  add("ktg_nbins",50);     add("ktg_max",10.0);
  add("nSD_nbins",15);     add("nSD_max",15.0);
  add("pt_nbins",100);     add("pt_min",0.0);     add("pt_max",100.0);
  add("mass_nbins",50);    add("mass_max",50.0);
  add("mult_nbins",50);    add("mult_max",50.0);
  add("ptLoss_nbins",50);
}

void JetGroomingHistos::configure(const String & taskName, const String & objectType, const Configuration & configuration,
                                  unsigned int index __attribute__((unused)))
{
  String type = objectType.beginsWith("HISTOGRAM") ? String("HISTOGRAM") : objectType;
  auto i = [&](const char * name) { return configuration.valueInt(createKey(taskName,type,name)); };
  auto d = [&](const char * name) { return configuration.valueDouble(createKey(taskName,type,name)); };
  zg_nbins = i("zg_nbins");
  rg_nbins = i("rg_nbins");  rg_max = d("rg_max");
  ktg_nbins = i("ktg_nbins");  ktg_max = d("ktg_max");
  nSD_nbins = i("nSD_nbins");  nSD_max = d("nSD_max");
  pt_nbins = i("pt_nbins");  pt_min = d("pt_min");  pt_max = d("pt_max");
  mass_nbins = i("mass_nbins");  mass_max = d("mass_max");
  mult_nbins = i("mult_nbins");  mult_max = d("mult_max");
  ptLoss_nbins = i("ptLoss_nbins");
}

void JetGroomingHistos::create()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  auto name = [this](const char * n) { return createName(_histogramBaseName,n); };
  const double oneUp = 1.0+1.0E-9;
  h_status    = createHistogram(name("status"),3,-0.5,2.5,"0 groomed, 1 no splitting, 2 failed","Jets");
  h_zg        = createHistogram(name("zg"),zg_nbins,0.0,0.5+1.0E-9,"z_{g}","Jets");
  h_rg        = createHistogram(name("rg"),rg_nbins,0.0,rg_max,"R_{g}","Jets");
  h_ktg       = createHistogram(name("ktg"),ktg_nbins,0.0,ktg_max,"k_{T,g} (GeV/c)","Jets");
  h_nSD       = createHistogram(name("nSD"),nSD_nbins,0.0,nSD_max,"n_{SD}","Jets");
  h_pt        = createHistogram(name("pt"),pt_nbins,pt_min,pt_max,"p_{T,g} (GeV/c)","Jets");
  h_mass      = createHistogram(name("mass"),mass_nbins,0.0,mass_max,"m_{g} (GeV/c^{2})","Jets");
  h_mult      = createHistogram(name("mult"),mult_nbins,0.0,mult_max,"N_{constituents,g}","Jets");
  h_ptLoss    = createHistogram(name("ptLoss"),ptLoss_nbins,0.0,oneUp,"1 - p_{T,g}/p_{T}","Jets");
  h_massVsMass = createHistogram(name("massVsMass"),mass_nbins,0.0,mass_max,mass_nbins,0.0,mass_max,
                                 "m (GeV/c^{2})","m_{g} (GeV/c^{2})","Jets");
}

void JetGroomingHistos::loadFrom(const Folder & inputFile)
{
  auto h = [&](const char * n) { return importHistogram(inputFile,createName(_histogramBaseName,n)); };
  h_status = h("status");
  h_zg = h("zg");  h_rg = h("rg");  h_ktg = h("ktg");  h_nSD = h("nSD");
  h_pt = h("pt");  h_mass = h("mass");  h_mult = h("mult");  h_ptLoss = h("ptLoss");  h_massVsMass = h("massVsMass");
}

void JetGroomingHistos::fill(const GroomedJet & groomed)
{
  _jetCount += 1.0;
  h_status->fill(double(int(groomed.status)));
  if (groomed.hasSplitting())
    {
    h_zg->fill(groomed.zg);
    h_rg->fill(groomed.rg);
    h_ktg->fill(groomed.ktg);
    }
  if (groomed.nSD>=0) h_nSD->fill(double(groomed.nSD));
  if (!groomed.kept()) return;
  const double pt = groomed.pt(), ptJet = groomed.jet!=nullptr ? groomed.jet->pt() : 0.0;
  h_pt->fill(pt);
  h_mass->fill(groomed.mass());
  h_mult->fill(double(groomed.constituents.size()));
  if (ptJet>0.0) h_ptLoss->fill(1.0-pt/ptJet);
  if (groomed.jet!=nullptr) h_massVsMass->fill(groomed.jet->mass(),groomed.mass(),1.0);
}

std::vector<HistogramFolder::Histogram*> JetGroomingHistos::allHistograms() const
{
  return {h_status, h_zg, h_rg, h_ktg, h_nSD, h_pt, h_mass, h_mult, h_ptLoss, h_massVsMass};
}

void JetGroomingHistos::scaleObject(double factor __attribute__((unused)))
{
}

void JetGroomingHistos::normalize(double eventFactor __attribute__((unused)))
{
  if (!(_jetCount>0.0)) return;
  for (Histogram * h : allHistograms()) if (h) h->scale(1.0/_jetCount);
}

} // namespace CAP
