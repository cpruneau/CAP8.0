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
#ifndef CAP__BalanceFunctionCalculator
#define CAP__BalanceFunctionCalculator
#include "Profile.hpp"
#include "Calculator.hpp"
#include "ClassDef.hpp"

namespace CAP
{

using namespace std;

//!
//! @brief Task computing charge-independent (CI) and charge-dependent (CD) combinations, balance functions and
//! differences of pair observables from previously filled histograms.
//!
//! @warning Not part of the build: it includes Calculator.hpp, which does not exist, and derives from
//! EventProcessorSingle<ParticleFlowHistos,ParticleFlowDerivedHistos>.
//!
class BalanceFunctionCalculator
:
public EventProcessorSingle<ParticleFlowHistos,ParticleFlowDerivedHistos>
{
public:

  //! @brief Default constructor.
  BalanceFunctionCalculator();
  //! @brief Copy constructor. @param task task to copy
  BalanceFunctionCalculator(const BalanceFunctionCalculator & task);
  //! @brief Copy assignment. @param task task to copy @return this task
  BalanceFunctionCalculator & operator=(const BalanceFunctionCalculator & task);
  //! @brief Destructor.
  virtual ~BalanceFunctionCalculator() {}
  //! @brief Add the default configuration.
  virtual void setDefaultConfiguration();
  //! @brief Compute the combinations for each file to analyze.
  virtual void execute();

  //!
  //!Calculate the "charge independent" combination of the given observable defined as
  //!\verbatim
  //! obs_CI = 0.25*(obs_1_2 + obs_1Bar_2 + obs_1_2Bar + obs_1Bar_2Bar)
  //!\endverbatim
  //!in which 1 and 2 represent the first and second particles respectively, and Bar means anti-particle.
  //!
  //!@param histoBaseName Base name of the histograms (should usually correspond to the pair label
  //!@param eventClassName Name of the event filter used to select the events used to fill the given histograms
  //!@param particleName1 Name of the first particle filter used to select particles used to fill the given histograms
  //!@param particleName2 Name of the second particle filter used to select particles used to fill the given histograms
  //!@param obsName Name of the observable being computed
  //!@param obs_1_2  Pointer to histogram of the observable of interest for particle 1 and 2
  //!@param obs_1Bar_2 Pointer to histogram of the observable of interest for particle 1Bar and 2
  //!@param obs_1_2Bar Pointer to histogram of the observable of interest for particle 1 and 2Bar
  //!@param obs_1Bar_2Bar Pointer to histogram of the observable of interest for particle 1Bar and 2Bar
  //!
  virtual Histogram* calculate_CI(const String & histoBaseName,
                            const String & eventClassName,
                            const String & particleName1,
                            const String & particleName2,
                            const String & obsName,
                            Histogram* obs_1_2, Histogram* obs_1Bar_2,
                            Histogram* obs_1_2Bar,
                            Histogram* obs_1Bar_2Bar,
                            HistogramFolder * HistogramFolder);

  //!
  //!Calculate the "charge dependent" combination of the given observable defined as
  //!\verbatim
  //! obs_CI = 0.5*(obs_1_2 - obs_1Bar_2 + obs_1_2Bar - obs_1Bar_2Bar)
  //!\endverbatim
  //!in which 1 and 2 represent the first and second particles respectively, and Bar means anti-particle.
  //!
  //!@param histoBaseName Base name of the histograms (should usually correspond to the pair label
  //!@param eventClassName Name of the event filter used to select the events used to fill the given histograms
  //!@param particleName1 Name of the first particle filter used to select particles used to fill the given histograms
  //!@param particleName2 Name of the second particle filter used to select particles used to fill the given histograms
  //!@param obsName Name of the observable being computed
  //!@param obs_1_2  Pointer to histogram of the observable of interest for particle 1 and 2
  //!@param obs_1Bar_2 Pointer to histogram of the observable of interest for particle 1Bar and 2
  //!@param obs_1_2Bar Pointer to histogram of the observable of interest for particle 1 and 2Bar
  //!@param obs_1Bar_2Bar Pointer to histogram of the observable of interest for particle 1Bar and 2Bar
  //!
  virtual Histogram* calculate_CD(const String & histoBaseName,
                            const String & eventClassName,
                            const String & particleName1,
                            const String & particleName2,
                            const String & obsName,
                            Histogram* obs_1_2,
                            Histogram* obs_1Bar_2,
                            Histogram* obs_1_2Bar,
                            Histogram* obs_1Bar_2Bar,
                            HistogramFolder * HistogramFolder);

  //!
  //!Calculate the "balance function" combination of the given observable defined as
  //!\verbatim
  //! obs_BalFct = (obs_US - obs_LS )/int(rho1_2)f
  //!\endverbatim
  //!in which US and LS represent unlike-signs (unlike baryon numbers, etc) and like-sign (same baryon number sign)
  //!
  //!@param histoBaseName Base name of the histograms (should usually correspond to the pair label
  //!@param eventClassName Name of the event filter used to select the events used to fill the given histograms
  //!@param particleName1 Name of the first particle filter used to select particles used to fill the given histograms
  //!@param particleName2 Name of the second particle filter used to select particles used to fill the given histograms
  //!@param obsName Name of the observable being computed
  //!@param comboName Name of given to the combo
  //!@param obs_US  Pointer to histogram of the observable of interest for a US pair
  //!@param obs_LS Pointer to histogram of the observable of interest for LS pair
  //!
  virtual Histogram* calculate_BalFct(const String & histoBaseName,
                                const String & eventClassName,
                                const String & particleName1,
                                const String & particleName2,
                                const String & obsName,
                                const String & comboName,
                                Histogram* rho1_2,
                                Histogram* obs_US,
                                Histogram* obs_LS,
                                HistogramFolder * HistogramFolder);

//  virtual Histogram* calculate_BalFct2(const String & histoBaseName,
//                                 const String & eventClassName,
//                                 const String & particleName1,
//                                 const String & particleName2,
//                                 const String & obsName,
//                                 const String & comboName,
//                                 Histogram* rho1_1,
//                                 Histogram* rho1_2,
//                                 Histogram* obs_US,
//                                 Histogram* obs_LS,
//                                 HistogramFolder * HistogramFolder);
//
//  virtual Histogram* calculate_BalFct3(const String & histoBaseName,
//                                 const String & eventClassName,
//                                 const String & particleName1,
//                                 const String & particleName2,
//                                 const String & obsName,
//                                 const String & comboName,
//                                 Histogram* rho1_2,
//                                 Histogram* obs_US,
//                                 Histogram* obs_LS,
//                                 HistogramFolder * HistogramFolder);

  //! @brief Sum of the balance functions obtained from obs_12Bar and obs_1Bar2 (see calculate_BalFct() for the parameters).
  virtual Histogram* calculate_BalFctSum(const String & histoBaseName,
                                    const String & eventClassName,
                                    const String & particleName1,
                                    const String & particleName2,
                                    const String & obsName,
                                    const String & comboName,
                                    Histogram* obs_12Bar,
                                   Histogram* obs_1Bar2,
                                   HistogramFolder * HistogramFolder);

  //!
  //!Calculate the "difference" combination of the given observable defined as
  //!\verbatim
  //! obs_Diff =  (obs_first - obs_second)
  //!\endverbatim
  //!in which obs_first and obs_second represent two distinc pair combinations of the given observable. This function is useful to
  //!compute differences of pair histograms such as obs_1Bar_2 and obs_1_2Bar which is expected to be null in the context
  //!of particle production at very large std::sqrt(s). For instance, n2(proton,kaon-) could be very similar to n2(proton-bar,kaon+).
  //!
  //!@param histoBaseName Base name of the histograms (should usually correspond to the pair label
  //!@param eventClassName Name of the event filter used to select the events used to fill the given histograms
  //!@param particleName1 Name of the first particle filter used to select particles used to fill the given histograms
  //!@param particleName2 Name of the second particle filter used to select particles used to fill the given histograms
  //!@param obsName Name of the observable being computed
  //!@param comboName Name of given to the combo
  //!@param obs_first  Pointer to histogram of the observable of the first pair
  //!@param obs_second Pointer to histogram of the observable of the second pair
  //!
  virtual Histogram* calculate_Diff(const String & histoBaseName,
                              const String & eventClassName,
                              const String & particleName1,
                              const String & particleName2,
                              const String & obsName,
                              const String & comboName,
                              Histogram* obs_first,
                              Histogram* obs_second,
                              HistogramFolder * HistogramFolder);


protected:

  //! Number of (event filter, particle filter) combinations.
  unsigned int nFilterCombos;
  //! Index of the first single-particle group of the current event filter.
  unsigned int baseSingleIndex;
  //! Index of the first pair group of the current event filter.
  unsigned int basePairIndex;

  //!
  //! Array containing the names of the single particle  observables involved in the balance function calculation
  //!
  vector<String> sObsNames;

  //!
  //! Array containing the names of the pair observables involved in the balance function calculation
  //!
  vector<String> pObsNames;
  //! Files to analyze.
  vector<String> allFilesToAnalyze;

  //! String appended to the names of the output histograms.
  String appendedString;
  //! Whether the charge-independent combinations are computed.
  bool calculateCI;
  //! Whether the charge-dependent combinations are computed.
  bool calculateCD;
  //! Whether the balance functions are computed.
  bool calculateBF;
  //! Whether the differences are computed.
  bool calculateDiffs;

  ClassDef(BalanceFunctionCalculator,0)
};

} // namespace CAP

#endif /* CAP__BalanceFunctionCalculator */
