/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetHistos.hpp
//! @brief JetHistos: the histograms of the jets of one combination of an event filter, a particle filter, a jet radius
//! and a jet filter: per-event jet variables, single-jet variables and constituent variables.
#ifndef CAP__JetHistos
#define CAP__JetHistos
#include <vector>
#include "HistogramFolder.hpp"
#include "Configuration.hpp"
#include "Jet.hpp"

namespace CAP
{

//!
//! @brief Histograms of the jets accepted by a jet filter (JetAnalyzer), for one event filter, particle filter (the
//! particles clustered) and jet radius.
//!
//! Per event (every event accepted by the event filter, also those without a jet):
//!  - nJets: number of jets accepted; sumPt: scalar sum of their pT.
//! Per jet:
//!  - pt, eta, phi, etaPhi, mass; mult (constituents), multPos, multNeg, mult0 (by charge sign); netQ (net charge);
//!    jetCharge (Q_kappa = sum q_i (pT_i/pT_jet)^kappa, kappa = jetChargeKappa);
//!  - profiles vs jet pT (suffix _vsPt): mult, multPos, multNeg, mult0, netQ, mass, jetCharge.
//! Per constituent, all and by charge sign (prefix c_, cPos_, cNeg_): z = pT/pT_jet, jt (momentum perpendicular to the
//! jet axis), theta (angle to the jet axis), r (distance to the axis in rapidity and azimuth); and the transverse
//! momentum profile rho (r), each constituent weighted by pT/pT_jet.
//!
//! Normalization (normalize(), called by JetAnalyzer::finalize()): the per-event histograms (nJets, sumPt) are divided
//! by the number of events accepted by the event filter; the per-jet and per-constituent histograms characterize the
//! jets themselves and are divided by the number of jets filled in this folder (this event filter, particle filter,
//! radius and jet filter): a constituent histogram is then a distribution per jet. The profiles are averages per jet.
//!
//! Binning (keys \<task\>:HISTOGRAM:\<name\>, see addDefaultConfiguration()): nJets_nbins/max, sumPt_nbins/max,
//! pt_nbins/min/max, eta_nbins/min/max, phi_nbins, mass_nbins/max, mult_nbins/max, netQ_nbins/max (symmetric),
//! jetCharge_nbins/max (symmetric), jetChargeKappa, z_nbins, jt_nbins/max, theta_nbins/max, r_nbins/max.
//!
class JetHistos : public HistogramFolder
{
public:

  //! @brief Default constructor.
  JetHistos();
  //! @brief Copy constructor. @param source folder
  JetHistos(const JetHistos & source);
  //! @brief Copy assignment. @param rhs folder @return this folder
  JetHistos & operator=(const JetHistos & rhs);
  //! @brief Destructor.
  virtual ~JetHistos() {}
  //! @brief Add the default binning (keys \<taskName\>:HISTOGRAM:\<name\>). @param configuration configuration
  //! @param taskName task
  static void addDefaultConfiguration(Configuration & configuration, const String & taskName);
  //! @brief Read the binning. @param taskName task @param objectType HISTOGRAM @param configuration configuration
  //! @param index index (not used: one binning for all the folders)
  virtual void configure(const String & taskName, const String & objectType, const Configuration & configuration,
                         unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a folder. @param inputFile folder @throws FileException if one is missing
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Fill the histograms with the jets of an event accepted by the jet filter. @param jets jets accepted
  void fill(const std::vector<const Jet*> & jets);
  //! @brief Normalize: the per-event histograms by a factor (1/number of events), the per-jet and per-constituent
  //! histograms by the number of jets filled (nothing if none); the profiles are not changed. @param eventFactor factor
  void normalize(double eventFactor);
  //! @brief Scale the per-event histograms only (the jet histograms are normalized per jet by normalize()).
  //! @param factor factor
  virtual void scaleObject(double factor);
  //! @brief Number of jets filled. @return number
  double jetCount() const { return _jetCount; }

protected:

  //! @brief Set the pointers to the histograms of another folder. @param source folder
  void cloneB(const JetHistos & source);
  //! @brief Fill the constituent histograms of one charge class. @param k class (0 all, 1 positive, 2 negative)
  //! @param z z @param jt jT @param theta theta @param r r
  void fillConstituent(unsigned int k, double z, double jt, double theta, double r);

  // binning
  int nJets_nbins;    double nJets_max;          //!< jets per event
  int sumPt_nbins;    double sumPt_max;          //!< pT sum of the jets
  int pt_nbins;       double pt_min, pt_max;     //!< jet pT
  int eta_nbins;      double eta_min, eta_max;   //!< jet eta
  int phi_nbins;                                 //!< jet phi, [0, 2 pi)
  int mass_nbins;     double mass_max;           //!< jet mass
  int mult_nbins;     double mult_max;           //!< constituents
  int netQ_nbins;     double netQ_max;           //!< net charge, [-max, max)
  int jetCharge_nbins; double jetCharge_max;     //!< jet charge, [-max, max)
  double jetChargeKappa;                         //!< kappa of the jet charge
  int z_nbins;                                   //!< z, [0, 1]
  int jt_nbins;       double jt_max;             //!< jT
  int theta_nbins;    double theta_max;          //!< theta
  int r_nbins;        double r_max;              //!< r

  double _jetCount;      //!< jets filled
  // per event
  Histogram * h_nJets;   //!< jets per event
  Histogram * h_sumPt;   //!< pT sum per event
  // per jet
  Histogram * h_pt;  Histogram * h_eta;  Histogram * h_phi;  Histogram * h_etaPhi;  Histogram * h_mass;
  Histogram * h_mult;  Histogram * h_multPos;  Histogram * h_multNeg;  Histogram * h_mult0;
  Histogram * h_netQ;  Histogram * h_jetCharge;
  Profile * p_mult;  Profile * p_multPos;  Profile * p_multNeg;  Profile * p_mult0;  Profile * p_netQ;
  Profile * p_mass;  Profile * p_jetCharge;
  // per constituent: all, positive, negative
  std::vector<Histogram*> h_z, h_jt, h_theta, h_r, h_rho;
};

} // namespace CAP

#endif /* CAP__JetHistos */
