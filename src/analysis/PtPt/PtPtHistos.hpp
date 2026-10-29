/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__PtPtHistos
#define CAP__PtPtHistos
#include "HistogramFolder.hpp"
#include "ParticleFilter.hpp"
#include "Configuration.hpp"

namespace CAP
{


//!
//! @brief Event-wise moments for transverse momentum correlations, as profiles vs an event variable: factorial moments
//! f1 to f4 of the multiplicity and products of the pT sums Q_n = sum of pT^n (Q1, Q1Q1, ..., Q2Q1, Q3Q1, Q4).
//! @warning fill() does nothing yet.
//!
class PtPtHistos : public HistogramFolder
{
public:

  //! @brief Default constructor.
  PtPtHistos();
  //! @brief Copy constructor. @param source group to copy
  PtPtHistos(const PtPtHistos & source);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  PtPtHistos & operator=(const PtPtHistos & rhs);
  //! @brief Destructor.
  virtual ~PtPtHistos(){ }
  //!
  //! @brief Add the default histogram settings (keys \<taskName\>:HISTOGRAM:evtName, evt_nbins, evt_min, evt_max) to a configuration: those of
  //! every task working on these histograms (PtPtAnalyzer, PtPtDerivedCalculator). @param configuration configuration
  //! @param taskName name of the task
  //!
  static void addDefaultConfiguration(Configuration & configuration, const String & taskName);
  //! @brief Read the event variable name and binning. @param taskName name of the task @param objectType object type
  //! @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Create the derived histograms, empty; see calculateDerived().
  virtual void createDerived();
  //! @brief Whether the derived histograms exist (created or loaded). @return true if they do
  bool hasDerived() const { return h_F1_evt!=nullptr; }
  //! @brief Load the derived histograms from a folder of a file. @param inputFile folder
  virtual void loadDerivedFrom(const Folder & inputFile);
  //! @brief Compute the derived histograms from the base histograms (createDerived() first).
  //! @throws Exception if the derived histograms were not created
  virtual void calculateDerived();
  //! @brief Fill the moments of an event (does nothing yet). @param mult event variable @param moments pT moments
  virtual void fill(double mult,vector<double> & moments);

protected:
  //! @brief Copy the content of another group. @param group group to copy
  void cloneB(const PtPtHistos & group);

  //! Distribution of the event variable.
  Histogram * h_evt;
  //! Factorial moments f1 to f4 of the multiplicity vs event variable.
  Profile* h_f1_evt;
  Profile* h_f2_evt;
  Profile* h_f3_evt;
  Profile* h_f4_evt;
  //! Averages of products of pT sums Q_n = sum pT^n vs event variable (Q1, Q1Q1, Q1Q1Q1, ..., Q4).
  Profile* h_Q1_evt;
  Profile* h_Q1Q1_evt;
  Profile* h_Q1Q1Q1_evt;
  Profile* h_Q1Q1Q1Q1_evt;
  Profile* h_Q2_evt;
  Profile* h_Q2Q1_evt;
  Profile* h_Q2Q1Q1_evt;
  Profile* h_Q2Q2_evt;
  Profile* h_Q3_evt;
  Profile* h_Q3Q1_evt;
  Profile* h_Q4_evt;


  //! Name of the event variable.
  String evtName;
  int    evt_nbins;  ///< number of bins of the event variable axis
  double evt_min;  ///< lower edge of the event variable axis
  double evt_max;  ///< upper edge of the event variable axis



  // Derived histograms (createDerived(), calculateDerived(); computed by PtPtDerivedCalculator)
  Histogram * h_F1_evt;
  Histogram * h_F2_evt;
  Histogram * h_F3_evt;
  Histogram * h_F4_evt;
  Histogram * h_R2_evt;
  Histogram * h_R3_evt;
  Histogram * h_R4_evt;
  Histogram * h_q1_evt;
  Histogram * h_q1q1_evt;
  Histogram * h_q1q1q1_evt;
  Histogram * h_q1q1q1q1_evt;
  Histogram * h_dq1dq1_evt;
  Histogram * h_dq1dq1dq1_evt;
  Histogram * h_dq1dq1dq1dq1_evt;

};

} // namespace CAP

#endif /* CAP__PtPtHistos  */



