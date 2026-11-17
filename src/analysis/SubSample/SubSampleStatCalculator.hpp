/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef DATUM__SubSampleStatCalculator
#define DATUM__SubSampleStatCalculator
#include <vector>
#include "Task.hpp"

using namespace std;

namespace Datum
{
class Folder;
namespace MATH { class Histogram; }

//!
//! @brief Task combining the histograms of sub-samples (independent jobs of the same analysis): their average, with
//! the uncertainty given by the spread of the sub-samples, or a jackknife estimate of the uncertainty.
//!
//! Each sub-directory of \<name\>:HISTOGRAM:IMPORT:PATH whose name matches \<name\>:SUBSAMPLE:PATTERN (default *)
//! and that holds the file \<name\>:HISTOGRAM:IMPORT:FILE_NAME is a sub-sample. The files may hold the histograms of
//! any stage: those filled by the analyzers, the derived histograms, the balance functions (any tree of folders; the
//! histograms are matched by their paths). The histograms of the files are per event (the analyzers scale them by the
//! number of events), so that the sub-samples are combined by averages, weighted by \<name\>:WEIGHTS: EVENTS, the
//! number of executions of the task of each file (its accountant Executed, if the files have one; equal weights
//! otherwise, with a warning), or EQUAL.
//!
//! With N sub-samples of values x_i and weights w_i (W their sum), bin by bin, the mode \<name\>:MODE is
//! - SUBSAMPLE: the weighted average m = sum w_i x_i / W, with the uncertainty of the mean
//!   sigma^2 = N/(N-1) sum w_i^2 (x_i - m)^2 / W^2 (for equal weights, the variance of the sub-samples divided by N);
//! - JACKKNIFE: the delete-one jackknife. The replica i is the sample without sub-sample i. Without stages, each
//!   replica is the weighted average of the other sub-samples, theta_i = (W m - w_i x_i)/(W - w_i), and the value is
//!   theta = m. With stages (\<name\>:JACKKNIFE:WORKFLOW, a workflow file of RunDatum.py, and \<name\>:JACKKNIFE:STAGES,
//!   the names of stages of it, e.g. "DerivedHistos BalanceFct"), the stages are run on the whole sample (the average
//!   of the sub-samples) and on each replica, as bin/Datum would run them on files: theta is what they make of the whole
//!   sample and theta_i what they make of replica i, so that nonlinear observables (correlation functions, balance
//!   functions, ratios, cumulants) are computed from (nearly) all the statistics. In both cases the uncertainty is
//!   sigma^2 = (N-1)/N sum (theta_i - mean)^2, with mean the average of the theta_i, and the bias-corrected values
//!   N theta - (N-1) mean are written apart, to \<name\>:JACKKNIFE:BIAS_FILE_NAME (default: the output file name with
//!   _biasCorrected), with the same structure.
//!
//! Production in pT-hat bins (\<name\>:PTHAT:BINS = K > 0): each sub-sample is K jobs, in the directories
//! \<sub-sample\>_pthat\<k\> (k = 00 ... K-1), run with PythiaEventGenerator PTHAT_MIN and PTHAT_MAX set to the edges of
//! bin k and the raw histograms kept (JOB:NORMALIZE false); each file holds the cross section sigma_k,s of its run and
//! its number of events N_k,s (DATUM_SIGMA_GEN, DATUM_N_ACCEPTED). First, the K bins of each sub-sample s are added with
//! the weights sigma_k/N_k,s, sigma_k the cross section of bin k averaged over the sub-samples (weighted by N_k,s):
//! H_s = sum_k sigma_k/N_k,s H_k,s, the differential cross section in mb, whatever the event filter or the jets of the
//! folder (raw counts are additive); the profiles are averages weighted by sigma_k/N_k,s. With
//! \<name\>:PTHAT:NORMALIZATION COVERED (default SIGMA), H_s is divided by the cross section of the range covered,
//! sum_k sigma_k (per event of that range; for later stages computing ratios, correlation functions, ...). H_s is
//! written to the directory \<sub-sample\>_CrossSection (with the cross sections of the bins: DATUM_PTHAT_BIN_\<k\>,
//! DATUM_SIGMA_COVERED), and these are the sub-samples combined as above (equal weights).
//!
//! The result (the histograms of the first sub-sample, or of the stages, with the values and uncertainties so computed;
//! the numbers of entries summed in SUBSAMPLE mode) and the summed accountants are written to
//! \<name\>:HISTOGRAM:EXPORT:PATH/EXPORT:FILE_NAME.
//!
class SubSampleStatCalculator
:
public Task
{
public:

  //! A sub-sample: its file and weight
  struct SubSample
  {
  String path;          //!< directory
  String file;          //!< file name in it
  double weight = 1.0;  //!< weight
  };

  //! @brief Default constructor.
  SubSampleStatCalculator();
  //! @brief Copy constructor. @param src task to copy
  SubSampleStatCalculator(const SubSampleStatCalculator & src);
  //! @brief Copy assignment. @param rhs task to copy @return this task
  SubSampleStatCalculator & operator=(const SubSampleStatCalculator & rhs);
  //! @brief Destructor.
  virtual ~SubSampleStatCalculator() {}
  //! @brief Add the default configuration (see the class description).
  virtual void setDefaultConfiguration();
  //! @brief Combine the sub-samples (see the class description). @throws Exception for fewer than 2 sub-samples
  virtual void execute();

  //! @name The statistics (for programs and tests)
  //! @{
  //!
  //! @brief Average of folders of histograms, weighted, with the uncertainty of the mean (SUBSAMPLE mode).
  //! @param samples folders (histograms matched by path; the first gives the structure) @param weights weights
  //! @param result set to the average (a copy of the first folder with the values, uncertainties and summed entries)
  //!
  static void average(const std::vector<const Folder *> & samples, const std::vector<double> & weights, Folder & result);
  //!
  //! @brief Jackknife of results: the value theta, its jackknife uncertainty and the bias-corrected values, from the
  //! value of the whole sample and the values of the N replicas. @param whole theta (histograms by path)
  //! @param replicas theta_i @param result set to theta with the jackknife uncertainties
  //! @param biasCorrected set to N theta - (N-1) mean, with the same uncertainties
  //!
  static void jackknife(const Folder & whole, const std::vector<const Folder *> & replicas, Folder & result,
                        Folder & biasCorrected);
  //!
  //! @brief The replica without sub-sample i: the weighted average of the others, (W m - w_i x_i)/(W - w_i).
  //! @param mean weighted average of all the sub-samples (W the sum of the weights) @param sample sub-sample i
  //! @param weight w_i @param sumWeights W @param replica set to the replica (a copy of mean with the values)
  //!
  static void leaveOneOut(const Folder & mean, const Folder & sample, double weight, double sumWeights, Folder & replica);
  //! @brief Paths of the histograms of a folder tree. @param folder folder @return paths
  static std::vector<String> histogramPaths(const Folder & folder);
  //!
  //! @brief Weighted sum of folders of raw histograms (pT-hat bins): the histograms sum_k w_k H_k / divisor
  //! (uncertainties added in quadrature, entries summed), the profiles averages weighted by w_k.
  //! @param bins folders (histograms matched by path; the first gives the structure) @param weights w_k
  //! @param divisor divisor of the histograms (not of the profiles) @param result set to the sum
  //!
  static void weightedSum(const std::vector<const Folder *> & bins, const std::vector<double> & weights, double divisor,
                          Folder & result);
  //!
  //! @brief The cross section of the run saved in a folder tree by PythiaEventGenerator (DATUM_SIGMA_GEN, DATUM_SIGMA_ERR,
  //! DATUM_N_ACCEPTED, DATUM_PTHAT_MIN, DATUM_PTHAT_MAX). @param folder folder @param sigma sigma (mb) @param error error
  //! @param nAccepted events @param ptHatMin lower edge @param ptHatMax upper edge @return false if not found
  //!
  static bool crossSection(const Folder & folder, double & sigma, double & error, long & nAccepted, double & ptHatMin,
                           double & ptHatMax);
  //! @}

protected:

  //! @brief The sub-samples of the import path (see the class description). @return sub-samples
  std::vector<SubSample> findSubSamples();
  //! @brief Add the pT-hat bins of each sub-sample (see the class description). @param nBins K @param normalization
  //! SIGMA or COVERED @return the sub-samples so made @throws Exception if a bin or a cross section is missing
  std::vector<SubSample> combinePtHatBins(unsigned int nBins, const String & normalization);

  Accountant _totalTaskExecuted;    //!< Sum of the numbers of executed tasks of the sub-samples.
  Accountant _eventAccepted;        //!< Numbers of accepted events of the current sub-sample.
  Accountant _totalEventAccepted;   //!< Sum of the numbers of accepted events of the sub-samples.
  String _importPath;   //!< Directory holding one subdirectory per sub-sample.
  String _importFile;   //!< Name of the file of each sub-sample.
  String _exportPath;   //!< Output directory.
  String _exportFile;   //!< Output file name.
};

} // namespace Datum

#endif /* DATUM__SubSampleStatCalculator */
