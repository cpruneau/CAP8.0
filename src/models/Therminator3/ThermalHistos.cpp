#include "ThermalHistos.hpp"
#include "RootConversion.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "TCanvas.h"
#include "TLine.h"
#include "TArrow.h"
#include "TFrame.h"
#include "TStyle.h"
#include "ParticleDb.hpp"
#include "ThermalHadron.hpp"
#include "ThermalHadronGas.hpp"

ClassImp(CAP::ThermalHistos);

namespace CAP
{
  TLine * createLine(float x1, float y1, float x2, float y2, int style, int color, int width, bool doDraw)
  {
  TLine *line = new TLine(x1,y1,x2,y2);
  line->SetLineStyle(style);
  line->SetLineColor(color);
  line->SetLineWidth(width);
  if (doDraw) line->Draw();
  return line;
  }
  
  //!
  //!Create Arrow Line
  //!
  TArrow * createArrow(float x1, float y1, float x2, float y2, float arrowSize, Option_t* option, int style, int color, int width, bool doDraw)
  {
  TArrow *line = new TArrow(x1,y1,x2,y2,arrowSize,option);
  line->SetLineStyle(style);
  line->SetLineColor(color);
  line->SetLineWidth(width);
  line->SetFillColor(color);
  
  if (doDraw) line->Draw();
  return line;
  }
  
  
  
  ThermalHistos::ThermalHistos()
  :
  HistogramFolder()
  {
  appendClassName("ThermalHistos");
  setMinimumReportLevel(Object::kInfo);
  setName("ThermalHistos");
  setTitle("ThermalHistos");
  }
  
  ThermalHistos::ThermalHistos(const ThermalHistos & source __attribute__((unused)))
  {   }
  
  ThermalHistos ThermalHistos::operator=(const ThermalHistos & rhs __attribute__((unused)))
  {
  return *this;
  }
  
  
  
  void ThermalHistos::configure(const String & taskName,
                                const String & objectType,
                                const Configuration & configuration,
                                unsigned int index  __attribute__((unused)))
  {
  _histogramBaseName = configuration.valueString(createKey(taskName,objectType,"BASE_NAME"));
  index_fill  = configuration.valueBool(createKey(taskName,objectType,"index_fill"));
  index_nbins = configuration.valueInt(createKey(taskName,objectType,"index_nbins"));
  index_min   = configuration.valueDouble(createKey(taskName,objectType,"index_min"));
  index_max   = configuration.valueDouble(createKey(taskName,objectType,"index_max"));
  mass_fill   = configuration.valueBool(createKey(taskName,objectType,"mass_fill"));
  mass_nbins  = configuration.valueInt(createKey(taskName,objectType,"mass_nbins"));
  mass_min    = configuration.valueDouble(createKey(taskName,objectType,"mass_min"));
  mass_max    = configuration.valueDouble(createKey(taskName,objectType,"mass_max"));
  }
  
  void ThermalHistos::create()
  {
  if (reportStart(__FUNCTION__)) { printCR(); }
  //stables
  unsigned int n1 = 15;
  double n1Min = -0.001;
  double n1Max = double(n1)-0.001;
  //int n2 = n1*n1;
  //double n2Min = -0.5;
  //double n2Max = double(n2)-0.5;
  String yTitle  = "#rho_{2}^{#alpha|#beta}";
  String yTitle2 = "#rho_{2}^{#alpha|#beta}/#int #rho_{2}^{#alpha|#beta}";
  
  h_massVsIndex = createHistogram(createName(_histogramBaseName,"_mass"),  index_nbins,index_min,index_max,"#alpha","m (GeV/c2)");
  diag          = createHistogram(createName(_histogramBaseName,"mass_spectrum"),  332,0.0,333.0,"","Mass (GeV/c^{2})");
  
  
  if (index_fill)
    {
    h_numberDensityVsIndex    = createHistogram(createName(_histogramBaseName,"_nDensity"),  index_nbins,index_min,index_max,"#alpha","n_{1}");
    h_energyDensityVsIndex    = createHistogram(createName(_histogramBaseName,"_eDensity"),  index_nbins,index_min,index_max,"#alpha","e");
    h_entropyDensityVsIndex   = createHistogram(createName(_histogramBaseName,"_sDensity"),  index_nbins,index_min,index_max,"#alpha","s");
    h_pressureVsIndex         = createHistogram(createName(_histogramBaseName,"_pressure"),       index_nbins,index_min,index_max,"#alpha","p");
    h_densityWFDVsIndex       = createHistogram(createName(_histogramBaseName,"densityWFeed"),   index_nbins,index_min,index_max,"#alpha","nfeed");
    h_netBaryonDensityVsIndex = createHistogram(createName(_histogramBaseName,"netBaryonDensity"), index_nbins,index_min,index_max,"#alpha","Baryon");
    h_netChargeDensityVsIndex = createHistogram(createName(_histogramBaseName,"netChargeDensity"), index_nbins,index_min,index_max,"#alpha","Q");
    h_netStrangeDensityVsIndex= createHistogram(createName(_histogramBaseName,"netStrangeDensity"),index_nbins,index_min,index_max,"#alpha","S");
    h_charmDensityVsIndex  = createHistogram(createName(_histogramBaseName,"charmDensity"),  index_nbins,index_min,index_max,"#alpha","C");
    h_bottomDensityVsIndex = createHistogram(createName(_histogramBaseName,"bottomDensity"), index_nbins,index_min,index_max,"#alpha","B");
    h_topDensityVsIndex    = createHistogram(createName(_histogramBaseName,"topDensity"),    index_nbins,index_min,index_max,"#alpha","T");
    
    for (unsigned int iStable=0; iStable<n1-1; iStable++)
      {
      String name;
      name = "pair_"; name += iStable;
      h_pairDensityVsIndex.push_back(createHistogram(createName(_histogramBaseName,name),  n1+2,n1Min,n1Max+2,"#alpha",yTitle));
      name = "normPair_"; name += iStable;
      h_normPairDensityVsIndex.push_back(createHistogram(createName(_histogramBaseName,name),  n1,n1Min,n1Max,"#alpha",yTitle2));
      }
    
    h_relNumberDensityVsIndex    = createHistogram(createName(_histogramBaseName,"relNumberDensity"),  index_nbins,index_min,index_max,"#alpha","#_rho(#alpha)/#_rho(#pi)");
    //    h_relEnergyDensityVsIndex    = createHistogram(createName(_histogramBaseName,"relEnergyDensity"),  index_nbins,index_min,index_max,"#alpha","e");
    //    h_relEntropyDensityVsIndex   = createHistogram(createName(_histogramBaseName,"relEntropyDensity"), index_nbins,index_min,index_max,"#alpha","s");
    
    //stable
    h_singleDensityVsIndex        = createHistogram(createName(_histogramBaseName,"singleDensityVsIndex"),     n1,n1Min,n1Max,"Stable Index","#rho_{1}");
    h_singleDensityWFDVsIndex     = createHistogram(createName(_histogramBaseName,"singleDensityFDVsIndex"),   n1,n1Min,n1Max,"Stable Index","#rho_{1}");
    h_singleDensityRatioVsIndex   = createHistogram(createName(_histogramBaseName,"singleDensityRatioVsIndex"),n1,n1Min,n1Max,"#alpha","#_rho^{TH+FD}(#alpha)/#_rho^{TH}(#alpha)");
    h_relDensityWFDVsIndex        = createHistogram(createName(_histogramBaseName,"relWtoPiVsIndex"),          n1,n1Min,n1Max,"#alpha","#_rho(#alpha)/#_rho(#pi)");
    }
  
  if (mass_fill)
    {
    h_numberDensityVsMass     = createProfile(createName(_histogramBaseName,"numberDensityVsM"),  mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","n");
    h_energyDensityVsMass     = createProfile(createName(_histogramBaseName,"energyDensityVsM"),  mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","e");
    h_entropyDensityVsMass    = createProfile(createName(_histogramBaseName,"entropyDensityVsM"), mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","s");
    h_pressureVsMass          = createProfile(createName(_histogramBaseName,"pressureVsM"),       mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","p");
    h_densityWFDVsMass        = createProfile(createName(_histogramBaseName,"densityWFeedVsM"),     mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","D");
    h_netBaryonDensityVsMass  = createProfile(createName(_histogramBaseName,"netBaryonDensityVsM"), mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","Baryon");
    h_netChargeDensityVsMass  = createProfile(createName(_histogramBaseName,"netChargeDensityVsM"), mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","Q");
    h_netStrangeDensityVsMass = createProfile(createName(_histogramBaseName,"netStrangeDensityVsM"),mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","S");
    h_charmDensityVsMass   = createProfile(createName(_histogramBaseName,"charmDensityVsM"),  mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","C");
    h_bottomDensityVsMass  = createProfile(createName(_histogramBaseName,"bottomDensityVsM"), mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","B");
    h_topDensityVsMass     = createProfile(createName(_histogramBaseName,"topDensityVsM"),    mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","T");
    
    h_relNumberDensityVsMass  = createProfile(createName(_histogramBaseName,"relNumberDensityVsM"), mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","#_rho(#alpha)/#_rho(#pi)");
    h_relDensityWFDVsMass     = createProfile(createName(_histogramBaseName,"relWtoPiVsM"),         mass_nbins,mass_min,mass_max,"M (GeV/c^{2})","#_rho^{FD}(#alpha)/#_rho(#pi)");
    }
  }
  
  // Fix me...
  void ThermalHistos::loadFrom(const Folder & inputFile __attribute__((unused)))
  {
  
  }
  
  void ThermalHistos::cloneB(const ThermalHistos & source)
  {
  h_numberDensityVsIndex     = histogramLike(source.h_numberDensityVsIndex);
  h_energyDensityVsIndex     = histogramLike(source.h_energyDensityVsIndex);
  h_entropyDensityVsIndex    = histogramLike(source.h_entropyDensityVsIndex);
  h_pressureVsIndex          = histogramLike(source.h_pressureVsIndex);
  h_densityWFDVsIndex      = histogramLike(source.h_densityWFDVsIndex);
  h_netBaryonDensityVsIndex  = histogramLike(source.h_netBaryonDensityVsIndex);
  h_netChargeDensityVsIndex  = histogramLike(source.h_netChargeDensityVsIndex);
  h_netStrangeDensityVsIndex = histogramLike(source.h_netStrangeDensityVsIndex);
  h_charmDensityVsIndex   = histogramLike(source.h_charmDensityVsIndex);
  h_bottomDensityVsIndex  = histogramLike(source.h_bottomDensityVsIndex);
  h_topDensityVsIndex     = histogramLike(source.h_topDensityVsIndex);
  
  h_numberDensityVsMass      = histogramLike(source.h_numberDensityVsMass);
  h_energyDensityVsMass      = histogramLike(source.h_energyDensityVsMass);
  h_entropyDensityVsMass     = histogramLike(source.h_entropyDensityVsMass);
  h_pressureVsMass           = histogramLike(source.h_pressureVsMass);
  h_densityWFDVsMass       = histogramLike(source.h_densityWFDVsMass);
  h_netBaryonDensityVsMass   = histogramLike(source.h_netBaryonDensityVsMass);
  h_netChargeDensityVsMass   = histogramLike(source.h_netChargeDensityVsMass);
  h_netStrangeDensityVsMass  = histogramLike(source.h_netStrangeDensityVsMass);
  h_charmDensityVsMass    = histogramLike(source.h_charmDensityVsMass);
  h_bottomDensityVsMass   = histogramLike(source.h_bottomDensityVsMass);
  h_topDensityVsMass      = histogramLike(source.h_topDensityVsMass);
  }
  
  double NoInf(double num,double denom1,double denom2=1.0)
  {
  if (denom1 == 0.0) return 0.0;
  return num/(denom1*denom2);
  }
  
  void ThermalHistos::fill(const ThermalHadronGasFD & hadronGas)
  {
  if (reportInfo(__FUNCTION__)) { printCR(); }
  //double  zero = 0.0;
  const vector<ThermalHadron> & hadrons        = hadronGas.hadrons();
  const vector<ThermalHadron*> & stableHadrons = hadronGas.stableHadrons();
  const vector<double> & stableHadronPairs     = hadronGas.stableHadronPairs();
  //unsigned int nHadrons           = hadrons.size();
  unsigned int nStableHadrons     = stableHadrons.size();
  //unsigned int nStableHadronPairs = stableHadronPairs.size();
  // thermal particle (undecayed)
  unsigned int iHadron = 1;
  const ThermalHadron & pion = hadrons[1];
  double pionSpinFactor = pion.type().spinFactor();
  double zero = 0.0;
  for (auto & hadron : hadrons)
    {
    double _mass  = hadron.type().mass();
    double width = hadron.type().width();
    double gspin = hadron.type().spinFactor()/pionSpinFactor;
    h_massVsIndex->setBinValueAt(unsigned((iHadron)-1),_mass);
    h_massVsIndex->setBinUncertaintyAt(unsigned((iHadron)-1),width);
    //double _gSpin = hadron.type().spinFactor();
    if (index_fill)
      {
      h_numberDensityVsIndex    ->setBinValueAt(unsigned((iHadron)-1),hadron.numberDensity());
      h_energyDensityVsIndex    ->setBinValueAt(unsigned((iHadron)-1),hadron.energyDensity());
      h_entropyDensityVsIndex   ->setBinValueAt(unsigned((iHadron)-1),hadron.entropyDensity());
      h_pressureVsIndex         ->setBinValueAt(unsigned((iHadron)-1),hadron.partialPressure());
      h_netBaryonDensityVsIndex ->setBinValueAt(unsigned((iHadron)-1),hadron.baryonDensity());
      h_netChargeDensityVsIndex ->setBinValueAt(unsigned((iHadron)-1),hadron.chargeDensity());
      h_netStrangeDensityVsIndex->setBinValueAt(unsigned((iHadron)-1),hadron.strangeDensity());
      h_charmDensityVsIndex     ->setBinValueAt(unsigned((iHadron)-1),hadron.charmDensity());
      h_bottomDensityVsIndex    ->setBinValueAt(unsigned((iHadron)-1),hadron.bottomDensity());
      h_topDensityVsIndex       ->setBinValueAt(unsigned((iHadron)-1),hadron.topDensity());
      h_densityWFDVsIndex       ->setBinValueAt(unsigned((iHadron)-1),hadron.numberDensityWFD());
      h_relNumberDensityVsIndex ->setBinValueAt(unsigned((iHadron)-1),NoInf(hadron.numberDensity(),pion.numberDensity(),gspin));
      h_relDensityWFDVsIndex    ->setBinValueAt(unsigned((iHadron)-1),NoInf(hadron.numberDensityWFD(),pion.numberDensityWFD(),gspin));
      
      h_numberDensityVsIndex    ->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      h_energyDensityVsIndex    ->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      h_entropyDensityVsIndex   ->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      h_pressureVsIndex         ->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      h_netBaryonDensityVsIndex ->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      h_netChargeDensityVsIndex ->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      h_netStrangeDensityVsIndex->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      h_charmDensityVsIndex     ->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      h_bottomDensityVsIndex    ->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      h_topDensityVsIndex       ->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      h_densityWFDVsIndex       ->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      h_relNumberDensityVsIndex ->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      h_relDensityWFDVsIndex    ->setBinUncertaintyAt(unsigned((iHadron)-1),zero);
      
      iHadron++;
      }
    
    if (mass_fill)
      {
      h_numberDensityVsMass    ->fill(_mass,hadron.numberDensity()/gspin);
      h_energyDensityVsMass    ->fill(_mass,hadron.energyDensity());
      h_entropyDensityVsMass   ->fill(_mass,hadron.entropyDensity());
      h_pressureVsMass         ->fill(_mass,hadron.partialPressure());
      h_densityWFDVsMass       ->fill(_mass,hadron.numberDensityWFD());
      h_netBaryonDensityVsMass ->fill(_mass,hadron.baryonDensity());
      h_netChargeDensityVsMass ->fill(_mass,hadron.chargeDensity());
      h_netStrangeDensityVsMass->fill(_mass,hadron.strangeDensity());
      h_charmDensityVsMass     ->fill(_mass,hadron.charmDensity());
      h_bottomDensityVsMass    ->fill(_mass,hadron.bottomDensity());
      h_topDensityVsMass       ->fill(_mass,hadron.topDensity());
      h_relNumberDensityVsMass ->fill(_mass,NoInf(hadron.numberDensity(),pion.numberDensity(),gspin));
      h_relDensityWFDVsMass    ->fill(_mass,NoInf(hadron.numberDensityWFD(),pion.numberDensityWFD(),gspin));
      }
    }
  
  // Stable particles
  for (unsigned int iHadron = 1; iHadron<nStableHadrons; iHadron++)
    {
    unsigned int iHadron1 = iHadron + 1;
    h_singleDensityVsIndex->setBinValueAt(unsigned((iHadron1)-1),stableHadrons[iHadron]->numberDensity());
    h_singleDensityVsIndex->setBinUncertaintyAt(unsigned((iHadron1)-1),0.0);
    h_singleDensityWFDVsIndex->setBinValueAt(unsigned((iHadron1)-1),stableHadrons[iHadron]->numberDensityWFD());
    h_singleDensityWFDVsIndex->setBinUncertaintyAt(unsigned((iHadron1)-1),0.0);
    }
  h_singleDensityRatioVsIndex->setAsRatioOf(*h_singleDensityWFDVsIndex,*h_singleDensityVsIndex);
  
  for (unsigned int iHadron1 = 0; iHadron1<nStableHadrons; iHadron1++)
    {
    Histogram * hPair = h_pairDensityVsIndex[iHadron1];
    Histogram * hNormPair= h_normPairDensityVsIndex[iHadron1];
    //unsigned int iHadron1P = iHadron1+1;
    double sum = 0;
    for (unsigned int iHadron2 = 1; iHadron2<nStableHadrons; iHadron2++)
      {
      unsigned int iHadron2P = iHadron2+1;
      unsigned int aIndex = iHadron1*nStableHadrons+iHadron2;
      double stableYield = stableHadronPairs[aIndex];
      sum += stableYield;
      hPair->setBinValueAt(unsigned((iHadron2P)-1),stableYield);
      hPair->setBinUncertaintyAt(unsigned((iHadron2P)-1),0.0);
      hNormPair->setBinValueAt(unsigned((iHadron2P)-1),stableYield);
      hNormPair->setBinUncertaintyAt(unsigned((iHadron2P)-1),0.0);
      }
    if (sum>0.0)
      {
      hPair->setBinValueAt(unsigned((nStableHadrons+2)-1),sum);
      hNormPair->scale(1.0/sum);
      }
    //    else
    //      {
    //      printValue("iHadron1",iHadron1);
    //      printValue("sum",sum);
    //      }
    }
  if (reportEnd(__FUNCTION__)) { printCR(); }
  }
  
  void ThermalHistos::setLabels(vector<ThermalHadron> & hadrons,
                                vector<ThermalHadron*> & stableHadrons)
  {
  if (reportInfo(__FUNCTION__)) { printCR(); }
  unsigned int nHadrons = hadrons.size();
  unsigned int nStable  = stableHadrons.size();
  printValue("nHadrons",nHadrons);
  printValue("nStable",nStable);
  
  //  for (unsigned int iSpecies=0; iSpecies<nHadrons; iSpecies++)
  //    {
  //    int bin = iSpecies+1;
  //    String label = hadrons[iSpecies].type().title();
  //    h_numberDensityVsIndex  ->setBinLabel((bin)-1,label,0);
  //    }
  String base = "#rho_{2}(#alpha|";
  String label;
  String title;
  for (unsigned int iSpecies=0; iSpecies<nStable; iSpecies++)
    {
    label = stableHadrons[iSpecies]->type().title();
    title = base + label + ")";
    h_pairDensityVsIndex[iSpecies]->setTitle(title);
    h_normPairDensityVsIndex[iSpecies]->setTitle(title);
    }
  
  
  for (unsigned int iSpecies2=0; iSpecies2<nStable; iSpecies2++)
    {
    int binLabel = iSpecies2+1;
    if (iSpecies2==0)
      label = " ";
    else
      label = stableHadrons[iSpecies2]->type().title();

    h_singleDensityVsIndex->setBinLabel((binLabel)-1,label,0);
    h_singleDensityWFDVsIndex->setBinLabel((binLabel)-1,label,0);
    h_singleDensityRatioVsIndex->setBinLabel((binLabel)-1,label,0);
    for (unsigned int iSpecies1=0; iSpecies1<nStable; iSpecies1++)
      {
      //      std::cout << binLabel <<  " LABEL= " << label <<  std::endl;
      h_pairDensityVsIndex[iSpecies1]->setBinLabel((binLabel)-1,label,0);
      h_normPairDensityVsIndex[iSpecies1]->setBinLabel((binLabel)-1,label,0);
      h_pairDensityVsIndex[iSpecies1]->setBinLabel((nStable+2)-1,"#Sigma_{#alpha}",0);
      }
    }
  }
  
  void ThermalHistos::drawMassSpectrum(const ThermalHadronGas & thermalGas)
  {
  gStyle->SetPalette(1,0);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetOptDate(0);
  gStyle->SetOptTitle(0);
  gStyle->SetPadBorderMode(0);
  Double_t w = 4000;
  Double_t h = 4000;
  auto c = new TCanvas("c", "c", w, h);
  c->SetCanvasSize(4000, 4000);
  //c->SetWindowSize(2000, 500);
  double parentMass;
  double daughterMass;
  TH1 * diagRoot = toRootHistogram(*diag);   // ROOT copy, drawn as the frame of the diagram
  diagRoot->SetMinimum(0.0);
  diagRoot->SetMaximum(2.2);
//  diagRoot->GetXaxis()->SetTicklength(0); // Removes tick marks
//  diagRoot->GetXaxis()->SetLabelOffset(999); //
//  diagRoot->GetYaxis()->SetTitleOffset(1.3); //
  diagRoot->Draw();
  double _position = 0.5;
  const std::vector<ThermalHadron> & hadrons = thermalGas.hadrons();
  
  unsigned int k = 0;
  for (auto & hadron : hadrons)
    {
    _position = 6.0*(double(k)-200.0) + 2.5;
    parentMass = hadron.type().mass();
    createLine(0.0, parentMass, 333.0, parentMass, 1, 1, 1, true);
    if (hadron.type().isStable() || parentMass>2.0) continue;
    k++;
    if (k<200 || k>250) continue;
    const std::vector<ParticleDecayMode> & decayModes = hadron.type().decayModes();
    
    for (auto & decayMode : decayModes)
      {
      const vector<int> & childrenCodes = decayMode.childrenPdgCodes();
      for (auto & childCode : childrenCodes)
        {
        if (childCode == 22) continue; // dont care about photons
        const ThermalHadron & childHadron = thermalGas.findHadronFor(childCode);
        daughterMass = childHadron.type().mass();
        double massDif = parentMass - daughterMass;
        if (massDif>0.1  &&  daughterMass>0)
          {
          //createArrow(float x1, float y1, float x2, float y2, float arrowSize, Option_t* option, int style, int color, int width, bool doDraw)
          if (massDif<0.5) createArrow(_position, parentMass, _position, daughterMass, 0.004, "|>", 1, 1, 1,true);
          else if (massDif<1.0) createArrow(_position, parentMass, _position, daughterMass, 0.004, "|>", 1, 2, 1,true);
          else if (massDif<1.5) createArrow(_position, parentMass, _position, daughterMass, 0.004, "|>", 2, 4, 1,true);
          else if (massDif<2.0) createArrow(_position, parentMass, _position, daughterMass, 0.004, "|>", 2, 6, 1,true);
          else  createArrow(_position, parentMass, _position, daughterMass, 0.004, "|>", 1, 7, 1,true);
          }
        _position += 1.0;
        }
      }
    }
  c->Print("./MassDecay.pdf");
  }
  
  
} // namespace CAP
