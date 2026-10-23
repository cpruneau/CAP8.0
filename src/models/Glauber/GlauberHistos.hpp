/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez   
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__GlauberHistos
#define CAP__GlauberHistos
#include "TObject.h"
#include "Task.hpp"
#include "Exceptions.hpp"
#include "ManagedObjects.hpp"
#include "GlauberEvent.hpp"
#include "HistogramFolder.hpp"
#include "MathConstants.hpp"

namespace CAP
{

//!
//! @brief Histograms of Glauber events for one event filter: impact parameter, numbers of participants, binary
//! collisions and spectators (total, per nucleus, protons and neutrons), collisions by pair type, their averages vs
//! impact parameter, participant moments (means, variances, eccentricities, participant planes of each order), and
//! transverse areas.
//!
class GlauberHistos : public HistogramFolder
{
public:
  //! @brief Default constructor.
  GlauberHistos();
  //! @brief Copy constructor. @param group object to copy
  GlauberHistos(const GlauberHistos & group);
  //! @brief Copy assignment. @param group object to copy @return this object
  GlauberHistos & operator=(const GlauberHistos & group);
  //! @brief Destructor.
  virtual ~GlauberHistos(){ }
  //! @brief Copy the content of another group. @param group group to copy
  //! @brief Copy the content of another group. @param group group to copy
  virtual void cloneAll(const GlauberHistos & group);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Fill the histograms with an event. @param event event @param weight weight
  virtual void fill(GlauberEvent & event, double weight=1.0);
  //! @brief Configure the object from keys \<taskName\>:\<objectType\>:... (binning, options).
  //! @param taskName name of the task @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                         const String & objectType,
                         const Configuration & configuration,
                         unsigned int index);

  //! @brief Characteristic length of the overlap region. @return length
  double calculatelength();
  //! @brief Density of the nucleons of a nucleus at a transverse point. @param nucleus nucleus @param x x @param y y
  //! @param rMaxSq squared radius of the neighborhood @return density
  double calculateDensityAt(const GlauberNucleus & nucleus, double x, double y, double rMaxSq);
  //! @brief Value of a nuclear distribution at a transverse point. @param dist distribution @param x x @param y y @return density
  double calculateDensity(NuclearDistribution & dist, double x, double y);
  //! @brief Compute the transverse areas of the nuclei and of their overlap for an event. @param event event
  void   calculateArea(GlauberEvent & event);
  //! @brief Fill the area histogram of a nucleus around a point. @param nucleus nucleus @param area histogram
  //! @param mx x of the point @param my y of the point @param rMaxSq squared radius of the neighborhood
  void   calculateAreaAt(const GlauberNucleus & nucleus, Histogram* area, double mx, double my, double rMaxSq);

  //! @brief Draw an event (positions of the nucleons). @param event event @param evName name of the drawing
  void   drawEvent(GlauberEvent & event, const String & evName);
protected:

  bool _fillBasics;
  bool _fillMoments;
  bool _fillSmeared;
  bool _fillArea;
  bool _drawEvents;
  int  _drawEventsN;
  int  _drawEventsIndex;
  int  _nOrders;
  
  int _nParts_nBins;
  int _nBinaries_nBins;
  int _nSpect_nBins;
  int _impact_nBins;
  int _cs_nBins;
  int _psi_nBins;
  int _ecc_nBins;
  int _rN_nBins;
  int _psiN_nBins;
  int _eccN_nBins;
  int _area_nBins;

  float _nParts_min;  ///< lower edge of the _nParts axis
  float _nBinaries_min;  ///< lower edge of the _nBinaries axis
  float _nSpect_min;  ///< lower edge of the _nSpect axis
  float _impact_min;  ///< lower edge of the _impact axis
  float _cs_min;  ///< lower edge of the _cs axis
  float _psi_min;  ///< lower edge of the _psi axis
  float _ecc_min;  ///< lower edge of the _ecc axis
  float _rN_min;  ///< lower edge of the _rN axis
  float _psiN_min;  ///< lower edge of the _psiN axis
  float _eccN_min;  ///< lower edge of the _eccN axis
  float _area_min;  ///< lower edge of the _area axis

  float _nParts_max;  ///< upper edge of the _nParts axis
  float _nBinaries_max;  ///< upper edge of the _nBinaries axis
  float _nSpect_max;  ///< upper edge of the _nSpect axis
  float _impact_max;  ///< upper edge of the _impact axis
  float _cs_max;  ///< upper edge of the _cs axis
  float _psi_max;  ///< upper edge of the _psi axis
  float _ecc_max;  ///< upper edge of the _ecc axis
  float _rN_max;  ///< upper edge of the _rN axis
  float _psiN_max;  ///< upper edge of the _psiN axis
  float _eccN_max;  ///< upper edge of the _eccN axis
  float _area_max;  ///< upper edge of the _area axis




  Histogram * h_impactPar;  ///< distribution of the impact parameter
  Histogram * h_xSection;  ///< distribution of the nucleon-nucleon cross section
  Histogram * h_nParts;  ///< distribution of the number of participants
  Histogram * h_nPartsA;  ///< distribution of the number of participants (nucleus A)
  Histogram * h_nPartsB;  ///< distribution of the number of participants (nucleus B)
  Histogram * h_nPartsProton;  ///< distribution of the number of participating protons
  Histogram * h_nPartsProtonA;  ///< distribution of the number of participating protons (nucleus A)
  Histogram * h_nPartsProtonB;  ///< distribution of the number of participating protons (nucleus B)
  Histogram * h_nPartsNeutron;  ///< distribution of the number of participating neutrons
  Histogram * h_nPartsNeutronA;  ///< distribution of the number of participating neutrons (nucleus A)
  Histogram * h_nPartsNeutronB;  ///< distribution of the number of participating neutrons (nucleus B)

  Histogram * h_nBinaries;  ///< distribution of the number of binary collisions
  Histogram * h_nBinariesA;  ///< distribution of the number of binary collisions (nucleus A)
  Histogram * h_nBinariesB;  ///< distribution of the number of binary collisions (nucleus B)

  Histogram * h_nSpect;  ///< distribution of the number of spectators
  Histogram * h_nSpectA;  ///< distribution of the number of spectators (nucleus A)
  Histogram * h_nSpectB;  ///< distribution of the number of spectators (nucleus B)
  Histogram * h_nSpectProton;  ///< distribution of the number of spectator protons
  Histogram * h_nSpectProtonA;  ///< distribution of the number of spectator protons (nucleus A)
  Histogram * h_nSpectProtonB;  ///< distribution of the number of spectator protons (nucleus B)
  Histogram * h_nSpectNeutron;  ///< distribution of the number of spectator neutrons
  Histogram * h_nSpectNeutronA;  ///< distribution of the number of spectator neutrons (nucleus A)
  Histogram * h_nSpectNeutronB;  ///< distribution of the number of spectator neutrons (nucleus B)

  Histogram * h_nPP;  ///< distribution of the number of proton-proton collisions
  Histogram * h_nPN;  ///< distribution of the number of proton-neutron collisions
  Histogram * h_nNP;  ///< distribution of the number of neutron-proton collisions
  Histogram * h_nNN;  ///< distribution of the number of neutron-neutron collisions
  Histogram * h_nHard;  ///< distribution of the number of hard collisions

  Histogram * h_nPartsVsImpact;  ///< number of participants vs impact parameter
  Histogram * h_nBinariesVsImpact;  ///< number of binary collisions vs impact parameter
  Histogram * h_nBinariesVsNParts;  ///< number of binary collisions vs number of participants

  Profile * h_nPartsAvgVsImpact;  ///< average number of participants vs impact parameter
  Profile * h_nPartsAAvgVsImpact;  ///< average number of participants (nucleus A) vs impact parameter
  Profile * h_nPartsBAvgVsImpact;  ///< average number of participants (nucleus B) vs impact parameter
  Profile * h_nPartsProtonAvgVsImpact;  ///< average number of participating protons vs impact parameter
  Profile * h_nPartsProtonAAvgVsImpact;  ///< average number of participating protons (nucleus A) vs impact parameter
  Profile * h_nPartsProtonBAvgVsImpact;  ///< average number of participating protons (nucleus B) vs impact parameter
  Profile * h_nPartsNeutronAvgVsImpact;  ///< average number of participating neutrons vs impact parameter
  Profile * h_nPartsNeutronAAvgVsImpact;  ///< average number of participating neutrons (nucleus A) vs impact parameter
  Profile * h_nPartsNeutronBAvgVsImpact;  ///< average number of participating neutrons (nucleus B) vs impact parameter

  Profile * h_nBinariesAvgVsImpact;  ///< average number of binary collisions vs impact parameter
  Profile * h_nBinariesAAvgVsImpact;  ///< average number of binary collisions (nucleus A) vs impact parameter
  Profile * h_nBinariesBAvgVsImpact;  ///< average number of binary collisions (nucleus B) vs impact parameter

  Profile * h_nSpectAvgVsImpact;  ///< average number of spectators vs impact parameter
  Profile * h_nSpectAAvgVsImpact;  ///< average number of spectators (nucleus A) vs impact parameter
  Profile * h_nSpectBAvgVsImpact;  ///< average number of spectators (nucleus B) vs impact parameter
  Profile * h_nSpectProtonAvgVsImpact;  ///< average number of spectator protons vs impact parameter
  Profile * h_nSpectProtonAAvgVsImpact;  ///< average number of spectator protons (nucleus A) vs impact parameter
  Profile * h_nSpectProtonBAvgVsImpact;  ///< average number of spectator protons (nucleus B) vs impact parameter
  Profile * h_nSpectNeutronAvgVsImpact;  ///< average number of spectator neutrons vs impact parameter
  Profile * h_nSpectNeutronAAvgVsImpact;  ///< average number of spectator neutrons (nucleus A) vs impact parameter
  Profile * h_nSpectNeutronBAvgVsImpact;  ///< average number of spectator neutrons (nucleus B) vs impact parameter

  Profile * h_nPPAvgVsImpact;  ///< average number of proton-proton collisions vs impact parameter
  Profile * h_nPNAvgVsImpact;  ///< average number of proton-neutron collisions vs impact parameter
  Profile * h_nNPAvgVsImpact;  ///< average number of neutron-proton collisions vs impact parameter
  Profile * h_nNNAvgVsImpact;  ///< average number of neutron-neutron collisions vs impact parameter
  Profile * h_nHardAvgVsImpact;  ///< average number of hard collisions vs impact parameter


// sys == system == participants

  Histogram * h_sysMeanX;  ///< mean x of the participants
  Histogram * h_sysMeanY;  ///< mean y of the participants
  Histogram * h_sysVarX;  ///< variance of x of the participants
  Histogram * h_sysVarY;  ///< variance of y of the participants
  Histogram * h_sysVarXY;  ///< covariance of x and y of the participants
  Histogram * h_sysRmsX;  ///< rms of x of the participants
  Histogram * h_sysRmsY;  ///< rms of y of the participants
  Histogram * h_sysEccX;  ///< eccentricity (x) of the participants
  Histogram * h_sysEccY;  ///< eccentricity (y) of the participants
  Histogram * h_sysEcc;  ///< participant eccentricity
  Histogram * h_sysPsi;  ///< participant-plane angle

  Profile * h_sysMeanXVsImpact;  ///< mean x of the participants vs impact parameter
  Profile * h_sysMeanYVsImpact;  ///< mean y of the participants vs impact parameter
  Profile * h_sysVarXVsImpact;  ///< variance of x of the participants vs impact parameter
  Profile * h_sysVarYVsImpact;  ///< variance of y of the participants vs impact parameter
  Profile * h_sysVarXYVsImpact;  ///< covariance of x and y of the participants vs impact parameter
  Profile * h_sysRmsXVsImpact;  ///< rms of x of the participants vs impact parameter
  Profile * h_sysRmsYVsImpact;  ///< rms of y of the participants vs impact parameter
  Profile * h_sysEccXVsImpact;  ///< eccentricity (x) of the participants vs impact parameter
  Profile * h_sysEccYVsImpact;  ///< eccentricity (y) of the participants vs impact parameter
  Profile * h_sysEccVsImpact;  ///< participant eccentricity vs impact parameter
  Profile * h_sysPsiVsImpact;  ///< participant-plane angle vs impact parameter

  std::vector<Histogram*> h_sysCosPhiN;  ///< <cos(n phi)> of the participants, one histogram per order n
  std::vector<Histogram*> h_sysSinPhiN;  ///< <sin(n phi)> of the participants, one histogram per order n
  std::vector<Histogram*> h_sysRnN;  ///< <r^n> of the participants, one histogram per order n
  std::vector<Histogram*> h_sysPsiN;  ///< participant-plane angle of order n, one histogram per order n
  std::vector<Histogram*> h_sysEccN;  ///< eccentricity of order n, one histogram per order n
  std::vector<Profile*> h_sysCosPhiNVsImpact;  ///< <cos(n phi)> of the participants vs impact parameter, one histogram per order n
  std::vector<Profile*> h_sysSinPhiNVsImpact;  ///< <sin(n phi)> of the participants vs impact parameter, one histogram per order n
  std::vector<Profile*> h_sysRnNVsImpact;  ///< <r^n> of the participants vs impact parameter, one histogram per order n
  std::vector<Profile*> h_sysPsiNVsImpact;  ///< participant-plane angle of order n vs impact parameter, one histogram per order n
  std::vector<Profile*> h_sysEccNVsImpact;  ///< eccentricity of order n vs impact parameter, one histogram per order n

  Histogram * h_areaA;  ///< transverse area of nucleus A
  Histogram * h_areaB;  ///< transverse area of nucleus B
  Histogram * h_areaOverlap;  ///< overlap area
  Histogram * h_areaTotal;  ///< total area
  Profile * h_areaAVsImpact;  ///< transverse area of nucleus A vs impact parameter
  Profile * h_areaBVsImpact;  ///< transverse area of nucleus B vs impact parameter
  Profile * h_areaOverlapVsImpact;  ///< overlap area vs impact parameter
  Profile * h_areaTotalVsImpact;  ///< total area vs impact parameter

  std::vector<Histogram*> h_events;  ///< event displays


  ClassDef(GlauberHistos,0)
};

} // namespace CAP

#endif /* CAP__GlauberHistos  */



