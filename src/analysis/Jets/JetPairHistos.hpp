/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetPairHistos.hpp
//! @brief JetPairHistos: the histograms of the jet pairs of one combination of an event filter, a particle filter, a
//! jet radius, a jet filter and a jet pair filter.
#ifndef CAP__JetPairHistos
#define CAP__JetPairHistos
#include <vector>
#include "HistogramFolder.hpp"
#include "Configuration.hpp"
#include "JetPair.hpp"

namespace CAP
{

//!
//! @brief Histograms of the jet pairs accepted by a jet pair filter (JetPairAnalyzer).
//!
//! Per event (every event accepted by the event filter, also those without a pair): nPairs, the number of pairs.
//! Per pair (the first jet of higher rank): pt1, pt2, pt1pt2; eta1, eta2; dphi (phi1 - phi2 in [-pi/2, 3 pi/2)),
//! acoplanarity (pi - |phi1 - phi2|), deta (eta1 - eta2); xJ (pT2/pT1), aJ ((pT1 - pT2)/(pT1 + pT2)); mass, pt and y
//! of the pair; dphiXJ (dphi vs xJ); profiles vs pT1 (suffix _vsPt1): xJ, aJ, acoplanarity.
//!
//! Normalization (normalize()): nPairs per event (factor of the event filter); the pair histograms per pair (divided by
//! the number of pairs of the folder); the profiles are averages per pair.
//!
//! Binning (keys \<task\>:HISTOGRAM:\<name\>): nPairs_nbins/max, pt_nbins/min/max, eta_nbins/min/max, dphi_nbins,
//! acop_nbins/max, deta_nbins/max, xJ_nbins, mass_nbins/max, pairPt_nbins/max, y_nbins/max.
//!
class JetPairHistos : public HistogramFolder
{
public:

  //! @brief Default constructor.
  JetPairHistos();
  //! @brief Copy constructor. @param source folder
  JetPairHistos(const JetPairHistos & source);
  //! @brief Copy assignment. @param rhs folder @return this folder
  JetPairHistos & operator=(const JetPairHistos & rhs);
  //! @brief Destructor.
  virtual ~JetPairHistos() {}
  //! @brief Add the default binning. @param configuration configuration @param taskName task
  static void addDefaultConfiguration(Configuration & configuration, const String & taskName);
  //! @brief Read the binning. @param taskName task @param objectType HISTOGRAM @param configuration configuration
  //! @param index index (not used)
  virtual void configure(const String & taskName, const String & objectType, const Configuration & configuration,
                         unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a folder. @param inputFile folder
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Fill the histograms with the pairs of an event accepted by the pair filter. @param pairs pairs
  void fill(const std::vector<JetPair> & pairs);
  //! @brief Normalize: nPairs per event, the pair histograms per pair; not the profiles. @param eventFactor factor
  void normalize(double eventFactor);
  //! @brief Scale the per-event histogram only. @param factor factor
  virtual void scaleObject(double factor);
  //! @brief Number of pairs filled. @return number
  double pairCount() const { return _pairCount; }

protected:

  //! @brief Set the pointers to the histograms of another folder. @param source folder
  void cloneB(const JetPairHistos & source);
  //! @brief The pair histograms (not the profiles, not nPairs). @return histograms
  std::vector<Histogram*> pairHistograms() const;

  int nPairs_nbins;  double nPairs_max;           //!< pairs per event
  int pt_nbins;      double pt_min, pt_max;       //!< jet pT
  int eta_nbins;     double eta_min, eta_max;     //!< jet eta
  int dphi_nbins;                                 //!< dphi in [-pi/2, 3 pi/2)
  int acop_nbins;    double acop_max;             //!< acoplanarity
  int deta_nbins;    double deta_max;             //!< deta in [-max, max)
  int xJ_nbins;                                   //!< xJ and aJ in [0, 1]
  int mass_nbins;    double mass_max;             //!< pair mass
  int pairPt_nbins;  double pairPt_max;           //!< pair pT
  int y_nbins;       double y_max;                //!< pair rapidity in [-max, max)

  double _pairCount;   //!< pairs filled
  Histogram * h_nPairs;
  Histogram * h_pt1;  Histogram * h_pt2;  Histogram * h_pt1pt2;  Histogram * h_eta1;  Histogram * h_eta2;
  Histogram * h_dphi;  Histogram * h_acop;  Histogram * h_deta;  Histogram * h_xJ;  Histogram * h_aJ;
  Histogram * h_mass;  Histogram * h_pairPt;  Histogram * h_y;  Histogram * h_dphiXJ;
  Profile * p_xJ;  Profile * p_aJ;  Profile * p_acop;
};

} // namespace CAP

#endif /* CAP__JetPairHistos */
