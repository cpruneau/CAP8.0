/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ParticleFlowHistos.hpp
//! @brief ParticleFlowHistos: the histograms of the analysis of anisotropic flow by multi-particle cumulants and the
//! scalar product method, for one event filter and one reference particle filter.
#ifndef CAP__ParticleFlowHistos
#define CAP__ParticleFlowHistos
#include <vector>
#include "HistogramFolder.hpp"
#include "DerivedCalculator.hpp"
#include "Configuration.hpp"
#include "FlowCorrelator.hpp"

namespace CAP
{

class Particle;

//!
//! @brief Histograms of the flow analysis (ParticleFlowAnalyzer) for one event filter and one reference particle
//! filter: the event-averaged multi-particle correlators (base histograms, profiles) and, computed from them, the
//! cumulants and flow coefficients (derived histograms, ParticleFlowDerivedCalculator).
//!
//! Two sets of particles enter: the reference particles (RF), selected by the particle filter of the folder, and the
//! particles of interest (POI), selected by each of the POI filters of the analyzer (POI_FILTER). For each harmonic
//! n = 1..nHarmonics and each order 2k = 2, 4, ..., maxOrder:
//!
//! Integral (reference) flow, profiles vs the event class variable (evtName):
//! - Corr<2k>_n<n>: <<2k>>_n, the 2k-particle correlator of the RF, every pair of the 2k particles distinct, harmonics
//!   n (k particles) and -n (k particles), weighted by the number of 2k-tuples (generic framework, FlowCorrelator);
//! - CorrGap<2k>_n<n> (gap on): the same with k particles in the sub-event A (x < -gap/2) and k in the sub-event B
//!   (x > gap/2), x the pseudorapidity (gapVariable ETA) or rapidity (Y): a rapidity gap of at least gap between the
//!   two halves (2-sub-event method, Jia et al., PRC 96, 034906 (2017));
//! - SPQaQb_n<n> (scalarProduct on, needs gap): <Q_A Q_B*>, the scalar product of the flow vectors of the sub-events
//!   (not normalized), per event.
//!
//! Differential flow, for each POI filter j (POI<j>), profiles vs the event class (integrated over the POI) and vs the
//! event class and the POI transverse momentum (suffix _pt):
//! - POI<j>_Corr<2k>_n<n>: <<2k'>>_n, a POI with harmonic n and 2k-1 RF, all distinct (a particle both POI and RF
//!   counted once: q vectors);
//! - POI<j>_CorrGap<2k>_n<n>: a POI and k-1 RF in one sub-event and k RF in the other (both sides averaged);
//! - POI<j>_SPuQ_n<n>: <u Q*>, the unit vector u = exp(i n phi) of a POI times the flow vector of the opposite sub-event.
//!
//! Derived histograms (calculateDerived(), FlowCumulants): Cum<2k>_n<n> = c_n{2k} and V<2k>_n<n> = v_n{2k};
//! CumGap, VGap (with the gap); POI<j>_DCum<2k>_n<n> = d_n{2k}, POI<j>_V<2k>_n<n> = v'_n{2k} (and _pt, and Gap);
//! POI<j>_VSP_n<n> = v_n{SP} = <u Q_B*>/sqrt(<Q_A Q_B*>) (and _pt). A coefficient that is not defined (c_n{2k}/A_k
//! not positive) is left empty (0, no entries).
//!
//! Configuration (keys \<task\>:HISTOGRAM:\<param\>, addDefaultConfiguration()): evtName, evt_nbins, evt_min, evt_max
//! (the event class axis); nHarmonics (n = 1..nHarmonics); maxOrder (2k up to maxOrder, even); pt_nbins, pt_min,
//! pt_max (the POI axis); gap (true: the gapped correlators), gapSize (its width), gapVariable (ETA or Y);
//! scalarProduct (true: the scalar product method, with the sub-events of the gap).
//! Folders read from a file take their sizes (harmonics, orders, POI, gap, scalar product) and binning from the
//! histograms in the file.
//!
//! Particle weights (1/efficiency, see ParticleFlowAnalyzer) correct non-uniform acceptance in the flow vectors; a
//! particle has the same weight as RF and as POI.
//!
class ParticleFlowHistos : public HistogramFolder
{
public:

  //! @brief Default constructor.
  ParticleFlowHistos();
  //! @brief Copy constructor. @param source group to copy
  ParticleFlowHistos(const ParticleFlowHistos & source);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  ParticleFlowHistos & operator=(const ParticleFlowHistos & rhs);
  //! @brief Destructor.
  virtual ~ParticleFlowHistos() { }
  //! @brief Add the default settings (keys \<taskName\>:HISTOGRAM:\<param\>) to a configuration: those of every task
  //! working on these histograms (ParticleFlowAnalyzer, ParticleFlowDerivedCalculator). @param configuration
  //! configuration @param taskName name of the task
  static void addDefaultConfiguration(Configuration & configuration, const String & taskName);
  //! @brief Name of the task computing the derived histograms. @return name
  static const char * derivedCalculatorName() { return "ParticleFlowDerivedCalculator"; }
  //! @brief Default name of the analyzer task filling these folders. @return name
  static const char * analyzerName() { return "ParticleFlowAnalyzer"; }
  //! @brief Read the settings. @param taskName task @param objectType object type @param configuration configuration
  //! @param index index @throws ConfigurationException if a setting is invalid
  virtual void configure(const String & taskName, const String & objectType, const Configuration & configuration,
                         unsigned int index);
  //! @brief Set the names of the POI filters (before create()). @param names names (one per POI filter)
  void setPoiNames(const std::vector<String> & names) { _poiNames = names; }
  //! @brief Create the base histograms.
  virtual void create();
  //! @brief Load the base histograms from a folder of a file (their sizes found in it), and the derived histograms if
  //! the folder has them. @param inputFile folder @throws FileException if a base histogram is missing
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Create the derived histograms, empty; see calculateDerived().
  virtual void createDerived();
  //! @brief Whether the derived histograms exist. @return true if they do
  bool hasDerived() const { return !_cum.empty() && _cum[0]!=nullptr; }
  //! @brief Load the derived histograms if the folder has all of them. @param inputFile folder
  virtual void loadDerivedFrom(const Folder & inputFile);
  //! @brief The derived histograms (members and names). @return list
  HistogramList derivedHistograms();
  //! @brief Compute the derived histograms (createDerived() first). @throws Exception if they were not created
  virtual void calculateDerived();
  //! @brief Scale the event class distribution (the profiles are averages). @param factor factor
  virtual void scaleObject(double factor);
  //!
  //! @brief Fill the correlators of an event. @param evt event class value @param ref reference particles
  //! @param refWeights their weights @param poi particles of interest of each POI filter @param poiWeights their
  //! weights @throws Exception if the numbers of POI lists differ from nPoi()
  //!
  void fill(double evt, const std::vector<Particle*> & ref, const std::vector<double> & refWeights,
            const std::vector<const std::vector<Particle*>*> & poi, const std::vector<std::vector<double>> & poiWeights);

  //! @name Settings
  //! @{
  //! @brief Highest harmonic. @return nHarmonics
  unsigned int nHarmonics() const { return _nHarmonics; }
  //! @brief Highest order (even). @return maxOrder
  unsigned int maxOrder() const { return _maxOrder; }
  //! @brief Number of POI filters. @return number
  unsigned int nPoi() const { return _poiNames.size(); }
  //! @brief Whether the gapped correlators are computed. @return true if so
  bool gap() const { return _gap; }
  //! @brief Width of the gap. @return width
  double gapSize() const { return _gapSize; }
  //! @brief Whether the scalar product method is used. @return true if so
  bool scalarProduct() const { return _scalarProduct; }
  //! @}

  //! @name Names of the histograms
  //! @{
  //! @brief Name of a histogram of the integral flow. @param what Corr, CorrGap, Cum, CumGap, V, VGap @param order 2k
  //! @param n harmonic @return name
  String integralName(const char * what, unsigned int order, unsigned int n) const;
  //! @brief Name of a histogram of the differential flow. @param poi POI filter @param what Corr, CorrGap, DCum,
  //! DCumGap, V, VGap, SPuQ, VSP @param order 2k (0 for SPuQ, VSP) @param n harmonic @param pt true: vs pT
  //! @return name
  String poiName(unsigned int poi, const char * what, unsigned int order, unsigned int n, bool pt) const;
  //! @}

protected:

  //! @brief Set the pointers to the histograms of another group. @param source group
  void cloneB(const ParticleFlowHistos & source);
  //! @brief Make the work vectors (sizes from the settings).
  void prepareWork();
  //! @brief Number of orders K (maxOrder/2). @return K
  unsigned int nOrders() const { return _maxOrder/2; }
  //! @brief Index of (n, k) in the integral lists. @param n harmonic @param k order/2 @return index
  unsigned int at(unsigned int n, unsigned int k) const { return (n-1)*nOrders()+(k-1); }
  //! @brief Index of (poi, n, k) in the POI lists. @param j POI @param n harmonic @param k order/2 @return index
  unsigned int at(unsigned int j, unsigned int n, unsigned int k) const { return (j*_nHarmonics+(n-1))*nOrders()+(k-1); }
  //! @brief Index of (poi, n) in the SP lists. @param j POI @param n harmonic @return index
  unsigned int atSP(unsigned int j, unsigned int n) const { return j*_nHarmonics+(n-1); }
  //! @brief Sub-event of a particle: 0 (A, x < -gap/2), 1 (B, x > gap/2) or -1 (in the gap). @param particle
  //! particle @return sub-event
  int subEvent(const Particle & particle) const;
  //! @brief pT bin of a particle (-1 outside). @param pt transverse momentum @return bin
  int ptBin(double pt) const;

  // settings
  String evtName;        //!< name of the event class variable
  int    evt_nbins;      //!< bins of the event class axis
  double evt_min;        //!< lower edge of the event class axis
  double evt_max;        //!< upper edge of the event class axis
  int    pt_nbins;       //!< bins of the POI pT axis
  double pt_min;         //!< lower edge of the POI pT axis
  double pt_max;         //!< upper edge of the POI pT axis
  unsigned int _nHarmonics;      //!< highest harmonic
  unsigned int _maxOrder;        //!< highest order (even)
  bool   _gap;                   //!< gapped correlators
  double _gapSize;               //!< width of the gap
  bool   _gapRapidity;           //!< gap in rapidity (true) or pseudorapidity (false)
  bool   _scalarProduct;         //!< scalar product method
  std::vector<String> _poiNames; //!< names of the POI filters

  // base histograms
  Histogram * h_evt;                    //!< event class distribution
  std::vector<Profile*> _corr;          //!< <<2k>>_n vs evt, at(n,k)
  std::vector<Profile*> _corrGap;       //!< gapped <<2k>>_n vs evt
  std::vector<Profile*> _spQaQb;        //!< <Q_A Q_B*>_n vs evt, n-1
  std::vector<Profile*> _poiCorr;       //!< <<2k'>>_n vs evt, at(j,n,k)
  std::vector<Profile*> _poiCorrPt;     //!< <<2k'>>_n vs evt and pT
  std::vector<Profile*> _poiCorrGap;    //!< gapped <<2k'>>_n vs evt
  std::vector<Profile*> _poiCorrGapPt;  //!< gapped <<2k'>>_n vs evt and pT
  std::vector<Profile*> _poiSP;         //!< <u Q*>_n vs evt, atSP(j,n)
  std::vector<Profile*> _poiSPPt;       //!< <u Q*>_n vs evt and pT

  // derived histograms
  std::vector<Histogram*> _cum, _v, _cumGap, _vGap;                     //!< c_n{2k}, v_n{2k}, at(n,k)
  std::vector<Histogram*> _dcum, _vp, _dcumPt, _vpPt;                   //!< d_n{2k}, v'_n{2k}, at(j,n,k)
  std::vector<Histogram*> _dcumGap, _vpGap, _dcumGapPt, _vpGapPt;       //!< gapped
  std::vector<Histogram*> _vSP, _vSPPt;                                 //!< v_n{SP}, atSP(j,n)

  // work (per event)
  FlowVector _q, _qA, _qB;                          //!< RF: all, sub-events A and B
  //! POI vectors (p) and POI-and-RF vectors (q), all and per sub-event, of each POI filter j and pT bin b (the last
  //! bin: all the POI), at j*(pt_nbins+1)+b
  std::vector<FlowVector> _p, _pq, _pA, _pqA, _pB, _pqB;
};

//!
//! @brief Derived histograms of the flow analysis (second phase): DerivedCalculator with the folders
//! ParticleFlowHistos, registered as the task class CAP::ParticleFlowDerivedCalculator.
//!
using ParticleFlowDerivedCalculator = DerivedCalculator<ParticleFlowHistos,1>;

} // namespace CAP

#endif /* CAP__ParticleFlowHistos */
