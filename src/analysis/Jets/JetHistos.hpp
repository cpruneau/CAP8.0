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
//! @brief JetHistos: the histograms of the jets of one combination of an event filter, a particle filter, jet settings
//! and a jet filter: per-event jet variables, single-jet variables, moments of the multiplicities within the jets and
//! in the cones of the underlying event, and constituent variables.
#ifndef DATUM__JetHistos
#define DATUM__JetHistos
#include <vector>
#include "HistogramFolder.hpp"
#include "Configuration.hpp"
#include "Jet.hpp"
#include "JetUnderlyingEvent.hpp"

namespace Datum
{

//!
//! @brief Histograms of the jets accepted by a jet filter (JetAnalyzer), for one event filter, particle filter (the
//! particles clustered) and jet settings.
//!
//! Per event (every event accepted by the event filter, also those without a jet):
//!  - nJets: number of jets accepted; sumPt: scalar sum of their pT; nCones: number of cones of the underlying event.
//! Per jet:
//!  - pt, eta, phi, etaPhi, mass; mult (N, constituents), multPos, multNeg, mult0 (N+, N-, N0, by charge sign); netQ
//!    (net charge Q, the sum of the charges of the constituents); jetCharge (Q_kappa = sum q_i (pT_i/pT_jet)^kappa,
//!    kappa = jetChargeKappa).
//! Moments within the jets, profiles (averages per jet) versus the jet pT (suffix _vsPt) and versus the number of
//! constituents N of the jet (suffix _vsMult):
//!  - multPos, multNeg, mult0: first moments <N+>, <N->, <N0>; f2Pos, f2Neg, f20: second factorial moments
//!    <N+(N+ - 1)>, <N-(N- - 1)>, <N0(N0 - 1)>; posNeg: the mixed moment <N+ N->;
//!  - netQ, netQf2: <Q> and the second factorial moment <Q(Q - 1)>;
//!  - jetCharge, jetCharge2: <Q_kappa> and <Q_kappa^2> (Q_kappa is not an integer: its second moment);
//!  - versus pT only (versus N they are N and N(N - 1)): mult <N>, f2 <N(N - 1)>, and mass <m_jet>.
//! Underlying event (prefix ue_): the multiplicities of the particles in the cones of the radius of the jets transverse
//! to the leading jet of the same particle filter and jet settings (UnderlyingEventCones, set by JetAnalyzer), kept if
//! within the acceptance and away from every jet: ue_mult, ue_multPos, ue_multNeg, ue_mult0 (first moments) and
//! ue_f2, ue_f2Pos, ue_f2Neg (second factorial moments N(N - 1), N+(N+ - 1), N-(N- - 1)), ue_posNeg (the mixed
//! moment N+ N-), averages per cone versus the
//! pT and versus N of the jets of the event (a jet of an event without a cone does not fill them).
//! Per constituent, all and by charge sign (prefix c_, cPos_, cNeg_): z = pT/pT_jet, jt (momentum perpendicular to the
//! jet axis), theta (angle to the jet axis), r (distance to the axis in rapidity and azimuth); and the transverse
//! momentum profile rho (r), each constituent weighted by pT/pT_jet.
//!
//! Normalization (normalize(), called by JetAnalyzer::finalize()): the per-event histograms (nJets, sumPt, nCones) are
//! divided by the number of events accepted by the event filter; the per-jet and per-constituent histograms
//! characterize the jets themselves and are divided by the number of jets filled in this folder (this event filter,
//! particle filter, jet settings and jet filter): a constituent histogram is then a distribution per jet. The profiles
//! are averages per jet (per cone for the underlying event) and are not scaled.
//!
//! Binning (keys \<task\>:HISTOGRAM:\<name\>, see addDefaultConfiguration()): nJets_nbins/max, sumPt_nbins/max,
//! nCones_nbins/max, pt_nbins/min/max, eta_nbins/min/max, phi_nbins, mass_nbins/max, mult_nbins/max (also the axis of
//! the profiles versus N), netQ_nbins/max (symmetric), jetCharge_nbins/max (symmetric), jetChargeKappa, z_nbins,
//! jt_nbins/max, theta_nbins/max, r_nbins/max.
//!
class JetHistos : public HistogramFolder
{
public:

  //! @brief Quantities of the profiles of the jets (see the class description): index in profiles().
  enum Moment { kMult, kMultPos, kMultNeg, kMult0, kF2, kF2Pos, kF2Neg, kF20, kPosNeg, kNetQ, kNetQF2, kJetCharge, kJetCharge2,
                kMass, kMoments };
  //! @brief Quantities of the profiles of the underlying event: index in ueProfiles().
  enum UeMoment { kUeMult, kUeMultPos, kUeMultNeg, kUeMult0, kUeF2, kUeF2Pos, kUeF2Neg, kUePosNeg, kUeMoments };
  //! @brief Axes of the profiles: the jet pT, the jet multiplicity N.
  enum Axis { kVsPt, kVsMult, kAxes };

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
  //! @brief Fill the histograms with the jets of an event accepted by the jet filter and the cones of the underlying
  //! event of the event. @param jets jets accepted @param cones multiplicities of the cones (none: no underlying event)
  void fill(const std::vector<const Jet*> & jets, const std::vector<Multiplicities> & cones);
  //! @brief Fill the histograms with the jets of an event, without cones. @param jets jets accepted
  void fill(const std::vector<const Jet*> & jets) { fill(jets,std::vector<Multiplicities>()); }
  //! @brief Normalize: the per-event histograms by a factor (1/number of events), the per-jet and per-constituent
  //! histograms by the number of jets filled (nothing if none); the profiles are not changed. @param eventFactor factor
  void normalize(double eventFactor);
  //! @brief Scale the per-event histograms only (the jet histograms are normalized per jet by normalize()).
  //! @param factor factor
  virtual void scaleObject(double factor);
  //! @brief Number of jets filled. @return number
  double jetCount() const { return _jetCount; }
  //! @brief Name of a quantity of the jets (e.g. "f2Pos"). @param m quantity @return name
  static const char * momentName(unsigned int m);
  //! @brief Name of a quantity of the underlying event (e.g. "ue_multPos"). @param m quantity @return name
  static const char * ueMomentName(unsigned int m);
  //! @brief Whether a quantity of the jets has a profile versus an axis (not N and N(N - 1) and the mass versus N).
  //! @param m quantity @param axis axis @return true if so
  static bool hasProfile(unsigned int m, unsigned int axis) { return axis==kVsPt || !(m==kMult || m==kF2 || m==kMass); }
  //! @brief Profile of a quantity of the jets versus an axis. @param m quantity @param axis axis @return profile, or
  //! null if it has none
  Profile * profile(unsigned int m, unsigned int axis) const { return _profiles[axis*kMoments+m]; }
  //! @brief Profile of a quantity of the underlying event versus an axis. @param m quantity @param axis axis @return profile
  Profile * ueProfile(unsigned int m, unsigned int axis) const { return _ueProfiles[axis*kUeMoments+m]; }

protected:

  //! @brief Set the pointers to the histograms of another folder. @param source folder
  void cloneB(const JetHistos & source);
  //! @brief Fill the constituent histograms of one charge class. @param k class (0 all, 1 positive, 2 negative)
  //! @param z z @param jt jT @param theta theta @param r r
  void fillConstituent(unsigned int k, double z, double jt, double theta, double r);
  //! @brief Name of a profile: quantity and axis, e.g. "f2Pos_vsMult". @param name quantity @param axis axis @return name
  static String profileName(const char * name, unsigned int axis);

  // binning
  int nJets_nbins;    double nJets_max;          //!< jets per event
  int sumPt_nbins;    double sumPt_max;          //!< pT sum of the jets
  int nCones_nbins;   double nCones_max;         //!< cones of the underlying event per event
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
  Histogram * h_nCones;  //!< cones of the underlying event per event
  // per jet
  Histogram * h_pt;  Histogram * h_eta;  Histogram * h_phi;  Histogram * h_etaPhi;  Histogram * h_mass;
  Histogram * h_mult;  Histogram * h_multPos;  Histogram * h_multNeg;  Histogram * h_mult0;
  Histogram * h_netQ;  Histogram * h_jetCharge;
  std::vector<Profile*> _profiles;     //!< moments of the jets, [axis*kMoments + m] (null: no such profile)
  std::vector<Profile*> _ueProfiles;   //!< multiplicities of the cones, [axis*kUeMoments + m]
  // per constituent: all, positive, negative
  std::vector<Histogram*> h_z, h_jt, h_theta, h_r, h_rho;
};

} // namespace Datum

#endif /* DATUM__JetHistos */
