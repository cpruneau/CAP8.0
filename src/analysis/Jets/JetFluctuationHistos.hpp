/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetFluctuationHistos.hpp
//! @brief JetFluctuationHistos: moments of the multiplicities within jets and in the cones of the underlying event, and
//! the quantities derived from them (variances, R2, nu_dyn, and the same corrected for the underlying event).
#ifndef CAP__JetFluctuationHistos
#define CAP__JetFluctuationHistos
#include <vector>
#include "HistogramFolder.hpp"
#include "Configuration.hpp"
#include "JetUnderlyingEvent.hpp"

namespace CAP
{

//!
//! @brief Fluctuations and correlations of the multiplicities within the jets of one combination of an event filter, a
//! particle filter, a radius and a jet filter (JetFluctuationAnalyzer).
//!
//! Each quantity is a profile in three versions: integrated (one bin, name \<q\>), versus the jet pT (\<q\>_vsPt), and
//! versus the number of constituents N of the jet (\<q\>_vsMult). Base quantities, averages per jet of:
//!  - n, nPos, nNeg, n0: N, N+, N-, N0; q, q2: the net charge Q = N+ - N- and Q^2;
//!  - f2, f2Pos, f2Neg, f20: N(N-1), N+(N+ - 1), N-(N- - 1), N0(N0 - 1); f2PosNeg: N+ N-, the numbers of pairs.
//! Underlying event (prefix ue_): the same for the particles of the cones of the underlying event of the event of the jet
//! (UnderlyingEventCones), averaged over the jets and cones (a jet of an event without a cone does not fill them):
//! ue_n, ue_nPos, ue_nNeg, ue_n0, ue_f2, ue_f2Pos, ue_f2Neg, ue_f20, ue_f2PosNeg. Per event: nCones, the number of cones.
//!
//! Derived (calculateDerived(), histograms with the same three versions):
//!  - var_n, var_nPos, var_nNeg, var_n0 (variance F2 + <N> - <N>^2), var_q (<Q^2> - <Q>^2);
//!  - R2_PP = <N+(N+ - 1)>/<N+>^2 - 1, R2_MM, R2_PM = <N+ N->/(<N+><N->) - 1, nudyn = R2_PP + R2_MM - 2 R2_PM;
//!  - corrected for the underlying event (prefix sub_), the particles of a jet taken as those of the jet itself (s) and
//!    of the underlying event (b), independent, b distributed as in a cone: <s> = <n> - <b>;
//!    F2(s) = F2(n) - 2<s><b> - F2(b); <s+ s-> = <n+ n-> - <s+><b-> - <b+><s-> - <b+ b->: sub_n, sub_nPos, sub_nNeg, sub_n0,
//!    sub_f2, sub_f2Pos, sub_f2Neg, sub_f20, sub_f2PosNeg, sub_var_n, sub_var_nPos, sub_var_nNeg, sub_var_n0,
//!    sub_R2_PP, sub_R2_MM, sub_R2_PM, sub_nudyn.
//! Bins without jets (or, for sub_, without cones) are left empty, as are R2 of a bin without particles of the sign.
//!
//! Normalization: nCones per event (normalize()); the profiles are averages and are not scaled.
//! Binning (keys \<task\>:HISTOGRAM:\<name\>): pt_nbins/min/max, mult_nbins/max, nCones_nbins/max.
//!
class JetFluctuationHistos : public HistogramFolder
{
public:

  //! @brief Base quantities of a jet (or cone): index in the lists of profiles.
  enum Moment { kN, kNPos, kNNeg, kN0, kQ, kQ2, kF2, kF2Pos, kF2Neg, kF20, kF2PosNeg, kMoments };
  //! @brief Versions: integrated, vs jet pT, vs jet multiplicity.
  enum Axis { kIntegrated, kVsPt, kVsMult, kAxes };
  //! @brief Derived quantities.
  enum Derived { kVarN, kVarNPos, kVarNNeg, kVarN0, kVarQ, kR2PP, kR2MM, kR2PM, kNudyn,
                 kSubN, kSubNPos, kSubNNeg, kSubN0, kSubF2, kSubF2Pos, kSubF2Neg, kSubF20, kSubF2PosNeg,
                 kSubVarN, kSubVarNPos, kSubVarNNeg, kSubVarN0, kSubR2PP, kSubR2MM, kSubR2PM, kSubNudyn, kDerived };

  //! @brief Default constructor.
  JetFluctuationHistos();
  //! @brief Copy constructor. @param source folder
  JetFluctuationHistos(const JetFluctuationHistos & source);
  //! @brief Copy assignment. @param rhs folder @return this folder
  JetFluctuationHistos & operator=(const JetFluctuationHistos & rhs);
  //! @brief Destructor.
  virtual ~JetFluctuationHistos() {}
  //! @brief Add the default binning. @param configuration configuration @param taskName task
  static void addDefaultConfiguration(Configuration & configuration, const String & taskName);
  //! @brief Read the binning. @param taskName task @param objectType HISTOGRAM @param configuration configuration
  //! @param index index (not used)
  virtual void configure(const String & taskName, const String & objectType, const Configuration & configuration,
                         unsigned int index);
  //! @brief Create the base histograms.
  virtual void create();
  //! @brief Create the derived histograms, empty.
  void createDerived();
  //! @brief Whether the derived histograms exist. @return true if so
  bool hasDerived() const { return _derived[0]!=nullptr; }
  //! @brief Load the base histograms, and the derived ones if the folder has them. @param inputFile folder
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Fill with a jet and the cones of the underlying event of its event. @param jet jet @param cones multiplicities
  //! of the cones
  void fill(const Jet & jet, const std::vector<Multiplicities> & cones);
  //! @brief Fill the number of cones of an event. @param nCones number
  void fillEvent(unsigned int nCones);
  //! @brief Compute the derived histograms (created if needed).
  void calculateDerived();
  //! @brief Normalize nCones per event; the profiles are not scaled. @param eventFactor factor
  void normalize(double eventFactor) { scaleObject(eventFactor); }
  //! @brief Scale the per-event histogram only. @param factor factor
  virtual void scaleObject(double factor);
  //! @brief Name of a base quantity. @param m quantity @return name
  static const char * momentName(unsigned int m);
  //! @brief Name of a derived quantity. @param d quantity @return name
  static const char * derivedName(unsigned int d);
  //! @brief Name of a histogram: quantity and version. @param name quantity @param a version @return name
  String histogramName(const String & name, unsigned int a) const;
  //! @brief Profile of a base quantity of the jets. @param m quantity @param a version @return profile
  Profile * jetProfile(unsigned int m, unsigned int a) const { return _jet[a*kMoments+m]; }
  //! @brief Profile of a base quantity of the cones. @param m quantity @param a version @return profile
  Profile * coneProfile(unsigned int m, unsigned int a) const { return _ue[a*kMoments+m]; }
  //! @brief Derived histogram. @param d quantity @param a version @return histogram
  Histogram * derived(unsigned int d, unsigned int a) const { return _derived[a*kDerived+d]; }

protected:

  //! @brief Set the pointers to the histograms of another folder. @param source folder
  void cloneB(const JetFluctuationHistos & source);
  //! @brief Whether the cones have a base quantity (not Q and Q^2). @param m quantity @return true if so
  static bool coneMoment(unsigned int m) { return m!=kQ && m!=kQ2; }

  int pt_nbins;      double pt_min, pt_max;   //!< jet pT
  int mult_nbins;    double mult_max;         //!< jet multiplicity
  int nCones_nbins;  double nCones_max;       //!< cones per event

  Histogram * h_nCones;              //!< cones per event
  std::vector<Profile*> _jet;        //!< base quantities of the jets [axis*kMoments+moment]
  std::vector<Profile*> _ue;         //!< base quantities of the cones (null for Q, Q^2)
  std::vector<Histogram*> _derived;  //!< derived quantities [axis*kDerived+derived]
};

} // namespace CAP

#endif /* CAP__JetFluctuationHistos */
