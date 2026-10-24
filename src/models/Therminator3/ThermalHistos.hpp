// Author: Claude Pruneau   09/25/2019

/***********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 **********************************************************************/
#ifndef CAP_ThermalHistos
#define CAP_ThermalHistos
#include "HistogramFolder.hpp"
#include "ThermalHadronGasFD.hpp"

namespace CAP
{
  
  
  //!
  //!Histograming class designed to ampke plots of the  properties of the  hadron gas (HG)  particle species partial quantities as
  //!a function of species index or particle _mass.
  //!Quantities of interest for each of the (therampl) particle species included in the HG calculation include
  //!- number density
  //!- _entropy density
  //!- partial _pressure
  //!Quantities of interest for each of the  stable particle species included in the HG calculation include
  //!- number density
  //!
  //!
  class ThermalHistos
  :
  public HistogramFolder
  {
  public:
  
  ThermalHistos();
  ThermalHistos(const ThermalHistos & source);
  ThermalHistos operator=(const ThermalHistos & rhe);
  virtual ~ThermalHistos() {}
  virtual void configure(const String & taskName,
                         const String & objectType,
                         const Configuration & configuration,
                         unsigned int index);
  virtual void create();
  virtual void loadFrom(const Folder & inputFile);
  virtual void fill(const ThermalHadronGasFD & hadronGas);
  //virtual void fillRho1VsP(ThermalHadronGas & hadronGas, double _volume);
  
  void setLabels(vector<ThermalHadron> & hadrons,
                 vector<ThermalHadron*> & stableHadrons);
  
  void drawMassSpectrum(const ThermalHadronGas & thermalGas);
  
  protected:
  
  virtual void cloneB(const ThermalHistos & source);
  
  Histogram * h_massVsIndex;
  Histogram * h_numberDensityVsIndex;
  Histogram * h_energyDensityVsIndex;
  Histogram * h_entropyDensityVsIndex;
  Histogram * h_pressureVsIndex;
  Histogram * h_netBaryonDensityVsIndex;
  Histogram * h_netChargeDensityVsIndex;
  Histogram * h_netStrangeDensityVsIndex;
  Histogram * h_charmDensityVsIndex;
  Histogram * h_bottomDensityVsIndex;
  Histogram * h_topDensityVsIndex;
  Histogram * h_densityWFDVsIndex;
  
  // stable particles only
  Histogram * diag;
  Histogram * h_singleDensityVsIndex;
  Histogram * h_singleDensityWFDVsIndex;
  Histogram * h_singleDensityRatioVsIndex;
  vector<Histogram*> h_pairDensityVsIndex;
  vector<Histogram*> h_normPairDensityVsIndex;
  
  
  Histogram * h_relNumberDensityVsIndex;
  //  Histogram * h_relEnergyDensityVsIndex;
  //  Histogram * h_relEntropyDensityVsIndex;
  //  Histogram * h_relPressureVsIndex;
  
  // stable only
  Histogram * h_relDensityWFDVsIndex;
  //  Histogram * h_ratioDensityWtoWOFDVsIndex;
  //  Histogram * h_ratioDensityWtoWOFDVsMass;
  
  Profile * h_numberDensityVsMass;
  Profile * h_energyDensityVsMass;
  Profile * h_entropyDensityVsMass;
  Profile * h_pressureVsMass;
  Profile * h_netBaryonDensityVsMass;
  Profile * h_netChargeDensityVsMass;
  Profile * h_netStrangeDensityVsMass;
  Profile * h_charmDensityVsMass;
  Profile * h_bottomDensityVsMass;
  Profile * h_topDensityVsMass;
  Profile * h_densityWFDVsMass;
  
  Profile * h_relNumberDensityVsMass;
  Profile * h_relEnergyDensityVsMass;
  Profile * h_relEntropyDensityVsMass;
  Profile * h_relPressureVsMass;
  Profile * h_relDensityWFDVsMass;
  Profile * h_relNetBaryonDensityVsMass;
  Profile * h_relNetChargeDensityVsMass;
  Profile * h_relNetStrangeDensityVsMass;
  Profile * h_relcharmDensityVsMass;
  Profile * h_relbottomDensityVsMass;
  Profile * h_reltopDensityVsMass;
  
  
  bool    index_fill;
  int     index_nbins;
  double  index_min;
  double  index_max;
  bool    mass_fill;
  int     mass_nbins;
  double  mass_min;
  double  mass_max;
  
  
  };
  
} // namespace CAP

#endif // !CAP_ThermalHistos



