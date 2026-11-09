/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticlePerformanceHistos
#define CAP__ParticlePerformanceHistos
#include "HistogramFolder.hpp"
#include "DerivedCalculator.hpp"
#include "Particle.hpp"
#include "Configuration.hpp"
#include "MathConstants.hpp"

namespace CAP
{

//!
//! @brief Detector performance histograms of one (event filter, particle filter) combination: the base histograms
//! filled event by event (PerformanceAnalyzer) and the histograms derived from them (PerformanceDerivedCalculator), in
//! one folder.
//!
//! Filled from the detector-level particles, each compared with its truth particle (Particle::truth(), see
//! MeasurementSimulator), and, when the analyzer is given the generator-level event, from the generator-level particles.
//! The differences reco - gen are those of pT (dpt), eta (deta), phi (dphi, in [-pi, pi)) and rapidity (dy); the axes
//! are the generator-level pT (Pt), eta (Eta), phi (Phi) and rapidity (Y).
//!
//! Base histograms, always filled: n1_dpt, n1_deta, n1_dphi (distributions of the differences); dptVsPt, detaVsPt,
//! dphiVsPt (profiles of the differences: the biases); n1_genPt (generator-level particles accepted by the filter, the
//! efficiency denominator) and n1_recPt (detector-level particles accepted by the filter, at the pT of their truth
//! particle: the numerator). Optional:
//! - eta_fill (default true): the profiles vs Eta, n1_genEta and n1_recEta;
//! - rapidity_fill (default true): n1_dy and the dy profiles; n1_genY and n1_recY;
//! - phi_fill (default false): the profiles vs Phi, n1_genPhi and n1_recPhi;
//! - ptEta_fill (default true): the maps vs (Pt, Eta), with their own, coarser binning (ptEta_pt_nbins,
//!   ptEta_eta_nbins; edges pt_min, pt_max, eta_min, eta_max): the rank 2 profiles dptVsPtEta, detaVsPtEta,
//!   dphiVsPtEta (and dyVsPtEta with rapidity_fill), n1_genPtEta and n1_recPtEta.
//!
//! Derived histograms (createDerived(), calculateDerived()), according to the same options: rms_<profile> for each
//! profile (e.g. rms_dptVsPt: the resolution, the rms of the difference in each bin) and eff<Axis> for each pair of
//! counts (e.g. effVsPt = n1_recPt / n1_genPt, binomial uncertainties; empty where nothing was generated); with
//! ptEta_fill, rms_<q>VsPtEta and effVsPtEta, the maps of the resolutions, biases (the profiles) and efficiency used
//! to calibrate a detector (CalibrationProducer). The numerator
//! counts the particles accepted at detector level, at their generator-level kinematics: near the edges of a filter,
//! migration can give an efficiency above 1. Histogram names begin with the base name of the group.
//!
class ParticlePerformanceHistos : public HistogramFolder
{
public:

  //! Differences reco - gen.
  enum Quantity { kDpt=0, kDeta, kDphi, kDy, nQuantities };
  //! Axes (generator-level quantities) of the profiles and of the counts.
  enum Axis { kPt=0, kEta, kPhi, kY, nAxes };

  //! @brief Default constructor.
  ParticlePerformanceHistos();
  //! @brief Copy constructor. @param source group to copy
  ParticlePerformanceHistos(const ParticlePerformanceHistos & source);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  ParticlePerformanceHistos & operator=(const ParticlePerformanceHistos & rhs);
  //! @brief Destructor.
  virtual ~ParticlePerformanceHistos() {}
  //! @brief Point the histogram members to the histograms of this group that have the names of those of source
  //! (after a copy or an assignment). @param source group copied
  virtual void cloneB(const ParticlePerformanceHistos & source);

  //!
  //! @brief Add the default binning and fill options (keys \<taskName\>:HISTOGRAM:pt_nbins, pt_min, pt_max, eta_nbins,
  //! eta_min, eta_max, phi_nbins, phi_min, phi_max, rapidity_nbins, rapidity_min, rapidity_max, dpt_nbins, dpt_min,
  //! dpt_max, Deta_nbins, Deta_min, Deta_max, Dphi_nbins, Dphi_min, Dphi_max, Drapidity_nbins, Drapidity_min,
  //! Drapidity_max, eta_fill, rapidity_fill, phi_fill, ptEta_fill, ptEta_pt_nbins, ptEta_eta_nbins) to a configuration: those of every task working on these
  //! histograms (PerformanceAnalyzer, PerformanceDerivedCalculator). @param configuration configuration
  //! @param taskName name of the task
  //!
  static void addDefaultConfiguration(Configuration & configuration, const String & taskName);
  //! @brief Name of the task computing the derived histograms of these folders (see DerivedCalculator). @return name
  static const char * derivedCalculatorName() { return "PerformanceDerivedCalculator"; }
  //! @brief Default name of the analyzer task filling these folders. @return name
  static const char * analyzerName() { return "PerformanceAnalyzer"; }

  //! @brief Read the binning and fill options from keys \<taskName\>:HISTOGRAM:... (see the class description).
  //! @param taskName name of the task @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String& taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the base histograms (according to the fill options).
  virtual void create();
  //! @brief Create the derived histograms (according to the fill options), empty; see calculateDerived().
  virtual void createDerived();
  //! @brief Whether the derived histograms exist (createDerived(), or read from a file). @return true if they do
  bool hasDerived() const { return h_eff[kPt]!=nullptr; }
  //! @brief Load the base histograms from a folder of a file, and the derived histograms if the folder has them.
  //! @param inputFile folder
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Load the derived histograms from a folder of a file, if it has all of them (they are there once a derived
  //! calculator computed them); otherwise none is loaded, and no exception is thrown. @param inputFile folder
  virtual void loadDerivedFrom(const Folder & inputFile);
  //! @brief The derived histograms (according to the fill options): the members pointing to them and their names,
  //! as createDerived() makes them. @return list
  HistogramList derivedHistograms();
  //! @brief Compute the resolutions and the efficiencies from the base histograms (createDerived() first).
  //! @throws Exception if the derived histograms were not created
  virtual void calculateDerived();

  //! @brief Fill the histograms with a detector-level particle (compared with its truth particle).
  //! @param particle particle @param weight weight @throws Exception if the particle has no truth particle
  virtual void fill(Particle & particle, double weight);
  //! @brief Count a generator-level particle (efficiency denominator). @param particle particle @param weight weight
  virtual void fillGenerated(const Particle & particle, double weight);
  //! @brief Scale the histograms of counts (differences, generated and detected particles). The profiles (averages),
  //! the resolutions and the efficiencies are intensive: not scaled. @param factor factor
  virtual void scaleObject(double factor) override;

  //! @brief Difference of two azimuths, mapped onto [-pi, pi). @param a azimuth @param b azimuth @return a - b
  static double deltaPhi(double a, double b);

  //! @name Histograms (nullptr when not created: fill options, or not yet created or loaded)
  //! @{
  //! @brief Distribution of a difference. @param q difference @return histogram
  Histogram * difference(Quantity q) const { return h_n1_d[q]; }
  //! @brief Profile of a difference vs an axis (the bias; none vs Y). @param q difference @param a axis @return profile
  Profile * profile(Quantity q, Axis a) const { return a==kY ? nullptr : p_d[q][a]; }
  //! @brief Resolution: rms of a difference vs an axis (derived; none vs Y). @param q difference @param a axis
  //! @return histogram
  Histogram * resolution(Quantity q, Axis a) const { return a==kY ? nullptr : h_rms[q][a]; }
  //! @brief Generator-level particles vs an axis. @param a axis @return histogram
  Histogram * generated(Axis a) const { return h_gen[a]; }
  //! @brief Detector-level particles vs an axis (at their generator-level values). @param a axis @return histogram
  Histogram * detected(Axis a) const { return h_rec[a]; }
  //! @brief Efficiency vs an axis (derived). @param a axis @return histogram
  Histogram * efficiency(Axis a) const { return h_eff[a]; }
  //! @brief Rank 2 profile of a difference vs (Pt, Eta): the bias map (ptEta_fill). @param q difference @return profile
  Profile * profilePtEta(Quantity q) const { return p_d2[q]; }
  //! @brief Resolution map: rms of a difference vs (Pt, Eta) (derived, ptEta_fill). @param q difference @return histogram
  Histogram * resolutionPtEta(Quantity q) const { return h_rms2[q]; }
  //! @brief Generator-level particles vs (Pt, Eta) (ptEta_fill). @return histogram
  Histogram * generatedPtEta() const { return h_gen2; }
  //! @brief Detector-level particles vs (Pt, Eta), at their generator-level values (ptEta_fill). @return histogram
  Histogram * detectedPtEta() const { return h_rec2; }
  //! @brief Efficiency map vs (Pt, Eta) (derived, ptEta_fill). @return histogram
  Histogram * efficiencyPtEta() const { return h_eff2; }
  //! @}

  //! @brief Whether a difference is filled (dy: rapidity_fill). @param q difference @return true if it is
  bool uses(Quantity q) const { return q!=kDy || rapidity_fill; }
  //! @brief Whether the profiles are filled vs an axis (Pt always, Eta: eta_fill, Phi: phi_fill, Y: never).
  //! @param a axis @return true if they are
  bool profilesVs(Axis a) const { return a==kPt || (a==kEta && eta_fill) || (a==kPhi && phi_fill); }
  //! @brief Whether the particles are counted vs an axis (Pt always, Eta: eta_fill, Phi: phi_fill, Y: rapidity_fill).
  //! @param a axis @return true if they are
  bool countsVs(Axis a) const { return profilesVs(a) || (a==kY && rapidity_fill); }
  //! @brief Whether the maps vs (Pt, Eta) are filled (ptEta_fill). @return true if they are
  bool mapsVsPtEta() const { return ptEta_fill; }

protected:

  //! @brief Short name of a difference (dpt, deta, dphi, dy). @param q difference @return name
  static const char * quantityName(Quantity q);
  //! @brief Short name of an axis (Pt, Eta, Phi, Y). @param a axis @return name
  static const char * axisName(Axis a);
  //! @brief Title of a difference (#Delta p_{T}, ...). @param q difference @return title
  static const char * quantityTitle(Quantity q);
  //! @brief Title of an axis (p_{T}, #eta, ...). @param a axis @return title
  static const char * axisTitle(Axis a);
  //! @brief Name of the distribution of a difference (n1_dpt, ...). @param q difference @return name
  String differenceName(Quantity q) const;
  //! @brief Name of a profile (dptVsPt, ...) or of its resolution (rms_dptVsPt, ...). @param q difference @param a axis
  //! @param rms resolution @return name
  String profileName(Quantity q, Axis a, bool rms=false) const;
  //! @brief Name of the generator-level counts (n1_genPt, ...), detector-level counts (n1_recPt, ...) or efficiency
  //! (effVsPt, ...). @param a axis @param kind 0: generated, 1: detected, 2: efficiency @return name
  String countName(Axis a, int kind) const;
  //! @brief Name of a rank 2 profile (dptVsPtEta, ...) or of its resolution (rms_dptVsPtEta, ...). @param q difference
  //! @param rms resolution @return name
  String profileNamePtEta(Quantity q, bool rms=false) const;
  //! @brief Name of the counts vs (Pt, Eta) (n1_genPtEta, n1_recPtEta) or of the efficiency map (effVsPtEta).
  //! @param kind 0: generated, 1: detected, 2: efficiency @return name
  String countNamePtEta(int kind) const;
  //! @brief Value of an axis for a momentum (phi in [phi_min, phi_min + 2pi)). @param a axis @param p momentum @return value
  double axisValue(Axis a, const VectorLorentz & p) const;

  //! @brief Set rms to the rms of the values in each bin of a profile (any rank, same binning), with the uncertainty
  //! rms/sqrt(2(nEff-1)) (empty bins where fewer than two entries). @param profile profile @param rms histogram
  static void calculateRms(const Profile & profile, Histogram & rms);
  //! @brief Set eff to the ratio passed/total, bin by bin (any rank, same binning), with binomial uncertainties (empty
  //! where total is 0).
  //! @param passed numerator @param total denominator @param eff efficiency
  static void calculateEfficiency(const Histogram & passed, const Histogram & total, Histogram & eff);

public:

  ////////////////////////////////////////////////////////////////////////////
  // Data Members - HistogramFolder
  ////////////////////////////////////////////////////////////////////////////
  //! Whether the profiles and counts vs eta are filled.
  bool eta_fill;
  //! Whether the rapidity differences and the counts vs rapidity are filled.
  bool rapidity_fill;
  //! Whether the profiles and counts vs phi are filled.
  bool phi_fill;
  //! Whether the maps vs (Pt, Eta) are filled.
  bool ptEta_fill;
  int ptEta_nbins[2];        ///< number of bins of the maps vs (Pt, Eta): ptEta_pt_nbins, ptEta_eta_nbins

  int    axis_nbins[nAxes];  ///< number of bins of each axis (pt_nbins, eta_nbins, phi_nbins, rapidity_nbins)
  double axis_min[nAxes];    ///< lower edge of each axis
  double axis_max[nAxes];    ///< upper edge of each axis
  int    diff_nbins[nQuantities];  ///< number of bins of each difference (dpt_nbins, Deta_nbins, Dphi_nbins, Drapidity_nbins)
  double diff_min[nQuantities];    ///< lower edge of each difference
  double diff_max[nQuantities];    ///< upper edge of each difference

protected:

  Histogram * h_n1_d[nQuantities];        //!< distributions of the differences
  Profile   * p_d[nQuantities][kY];       //!< profiles of the differences vs Pt, Eta, Phi
  Histogram * h_gen[nAxes];               //!< generator-level counts
  Histogram * h_rec[nAxes];               //!< detector-level counts
  Histogram * h_rms[nQuantities][kY];     //!< resolutions vs Pt, Eta, Phi (derived)
  Histogram * h_eff[nAxes];               //!< efficiencies (derived)
  Profile   * p_d2[nQuantities];          //!< profiles of the differences vs (Pt, Eta)
  Histogram * h_gen2;                     //!< generator-level counts vs (Pt, Eta)
  Histogram * h_rec2;                     //!< detector-level counts vs (Pt, Eta)
  Histogram * h_rms2[nQuantities];        //!< resolution maps vs (Pt, Eta) (derived)
  Histogram * h_eff2;                     //!< efficiency map vs (Pt, Eta) (derived)

};

//! @brief Derived histograms of the performance analysis (resolutions, efficiencies): DerivedCalculator with the
//! folders ParticlePerformanceHistos, registered as the task class CAP::PerformanceDerivedCalculator.
using PerformanceDerivedCalculator = DerivedCalculator<ParticlePerformanceHistos,1>;

} // namespace CAP

#endif /* CAP__ParticlePerformanceHistos*/
