/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ClosureCalculator
#define CAP__ClosureCalculator
#include "Task.hpp"
#include "Folder.hpp"
#include "StatisticalTests.hpp"
#include <functional>
#include <utility>
#include <vector>

namespace CAP
{

//!
//! @brief Task computing a closure test: the difference (METHOD 0) or ratio (METHOD 1) of the histograms of a detector
//! level (reconstructed) analysis and of the generator level analysis, histogram by histogram.
//!
//! The generator-level histograms are those of the folder IMPORT:GEN_FOLDER of the file IMPORT:GEN_FILE_NAME, the
//! detector-level ones those of the folder IMPORT:REC_FOLDER of IMPORT:REC_FILE_NAME (empty folder: the whole file).
//! Both trees are walked: each histogram (or profile) of the generator-level tree is compared with the one at the same
//! place in the detector-level tree, and the result written at that place in EXPORT:CLOSURE_FILE_NAME. Histograms
//! without counterpart are skipped (listed at the INFO level). Gen and reco can be two files (e.g. *_Gen.cap and
//! *_Reco.cap), or two folders of the output file of one job (e.g. ParticleSingleAnalyzer and
//! ParticleSingleAnalyzerSmeared). Profiles are compared by their averages.
//!
//! A statistical test can compare the two levels, histogram by histogram (StatisticalTest, key TEST), with the tests of
//! MATH (StatisticalTests.hpp):
//!  - 1, chi2: MATH::histogramCompatibilityTest() of the values with their uncertainties, compared as they are
//!    (HistogramComparison::kValues): chi2 = sum (reco_i - gen_i)^2/(sigma_reco,i^2 + sigma_gen,i^2) over the bins with
//!    an uncertainty, bins degrees of freedom. Histograms of any rank;
//!  - 2, Kolmogorov-Smirnov: MATH::kolmogorovSmirnovTest() of the two histograms: the largest distance D between their
//!    cumulative distributions (bin values), the p-value with the numbers of entries of the histograms as sample sizes
//!    (each histogram is scaled to its number of entries first, which leaves D unchanged; the sums of the bin values
//!    are used for histograms without entries). Histograms of rank 1 only; binned data make the test conservative;
//!  - 3, user defined: the function given to setUserTest().
//! The result of the test of a histogram is written next to its closure, as a StringParameter \<name\>_TEST holding
//! TestResult::summary() (or the reason why it could not be made, e.g. a histogram of rank 2 for the Kolmogorov
//! test); calculate() lists them, with the number of histograms whose closure fails at the 5% level.
//!
//! Keys \<name\>:HISTOGRAM:METHOD, HISTOGRAM:TEST, IMPORT:PATH, IMPORT:GEN_FILE_NAME, IMPORT:GEN_FOLDER,
//! IMPORT:REC_FILE_NAME, IMPORT:REC_FOLDER, EXPORT:PATH, EXPORT:CLOSURE_FILE_NAME, EXPORT:FORCE_REWRITE. The paths DEFAULT (or empty) are
//! $CAP_HISTOS_IMPORT_PATH and $CAP_HISTOS_EXPORT_PATH; relative paths are relative to them (see
//! FileSystem::configuredPath()). File names are CAP files (the .cap extension may be omitted).
//!
namespace MATH { class Histogram; }

class ClosureCalculator
:
public Task
{
public:
  //! Statistical test run on the output of a closure test (argument test of calculate() and calculateClosureFor()).
  enum StatisticalTest
  {
    kNoTest         = 0,   //!< no test
    kChi2Test       = 1,   //!< chi2 test
    kKolmogorovTest = 2,   //!< Kolmogorov(-Smirnov) test
    kUserTest       = 3    //!< test defined by the user
  };
  //! Test defined by the user: compares the generator-level and detector-level histograms (same binning).
  using UserTest = std::function<MATH::TestResult(const MATH::Histogram & gen, const MATH::Histogram & reco)>;
  //! Result of the test of one histogram: its path and the result (test empty: not made, the reason in nullHypothesis).
  using TestRecord = std::pair<String,MATH::TestResult>;
  //! @brief Set the test run by kUserTest (shared by all the calculators). @param test test (empty: none)
  static void setUserTest(const UserTest & test);
  //! @brief Test run by kUserTest. @return test (empty if none was set)
  static const UserTest & userTest();
  //! @brief Run a statistical test on two histograms (see the class description).
  //! @param test kChi2Test, kKolmogorovTest or kUserTest @param gen generator-level histogram
  //! @param reco detector-level histogram @return result
  //! @throws MathException if the test cannot be made (no bin compared, rank, ...); Exception for kUserTest without
  //! user test, or an unknown test
  static MATH::TestResult runTest(int test, const MATH::Histogram & gen, const MATH::Histogram & reco);

  //! @brief Default constructor.
  ClosureCalculator();
  //! @brief Destructor.
  virtual ~ClosureCalculator() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Run the closure test configured by the keys of the task.
  //! @throws Exception if a file name is not set (NONE) or the method is unknown; FileException if a file cannot be
  //! opened
  virtual void execute();
  //!
  //! @brief Run a closure test: closure = detector - generator (method 0) or detector / generator (method 1).
  //! @param method 0 (difference) or 1 (ratio)
  //! @param test statistical test run on the output (StatisticalTest: 0 none, 1 chi2, 2 Kolmogorov, 3 user defined)
  //! @param importDirectory directory of the input files (resolved)
  //! @param genFileName generator-level file @param genFolder folder of the generator-level histograms ("": all)
  //! @param recoFileName detector-level file @param recoFolder folder of the detector-level histograms ("": all)
  //! @param exportDirectory directory of the output file (resolved) @param closureFileName output file
  //! @param forceRewrite whether an existing output file is replaced
  //! @throws Exception if the method or the test is unknown; FileException if a file cannot be opened
  //!
  //! @return number of histograms compared
  unsigned int calculate(int method,
                         int test,
                         const String & importDirectory,
                         const String & genFileName,
                         const String & genFolder,
                         const String & recoFileName,
                         const String & recoFolder,
                         const String & exportDirectory,
                         const String & closureFileName,
                         bool forceRewrite);
  //!
  //! @brief Closure of two folder trees (see the class description): out receives, at the place of each histogram of
  //! gen that has a counterpart in reco, reco - gen (method 0) or reco / gen (method 1).
  //! @param gen generator-level folder @param reco detector-level folder @param out result @param method 0 or 1
  //! @param test statistical test run on the output (StatisticalTest; passed down to the sub-folders)
  //! @param skipped paths of the histograms of gen without counterpart (added to) @param path path of gen (for skipped)
  //! @param results results of the tests, by path (added to; null: not kept); also written to out (\<name\>_TEST)
  //! @return number of histograms compared
  //!
  static unsigned int calculateClosureFor(const Folder & gen, const Folder & reco, Folder & out, int method, int test,
                                          std::vector<String> & skipped, const String & path="",
                                          std::vector<TestRecord> * results=nullptr);
  //! @brief Results of the statistical tests of the last calculate() (empty without test). @return results, by path
  const std::vector<TestRecord> & testResults() const { return _testResults; }

protected:
  std::vector<TestRecord> _testResults;   //!< results of the tests of the last calculate()
};

} // namespace CAP

#endif /* CAP__ClosureCalculator */
