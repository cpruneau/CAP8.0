/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ClosureIterator
#define CAP__ClosureIterator
#include <vector>
#include "Task.hpp"

namespace CAP
{

//!
//! @brief Task running closure tests (see ClosureCalculator) on all the generator-level files of a directory: for each
//! CAP file (.cap) of IMPORT:PATH whose name holds the tag "_Gen" once, the detector-level file of the same name with
//! the tag "_Reco" is compared with it, and the result written to the file named with the tag "_Closure" in
//! EXPORT:PATH. A tag is "_Gen", "_Reco" or "_Closure" followed by "_", "." or the end of the name: "pp_Gen.cap" and
//! "pp_Gen_run1.cap" are generator-level files ("pp_Reco.cap", "pp_Reco_run1.cap"), "pp_General_Gen.cap" too (only its
//! last "_Gen" is a tag), but not "pp_General.cap"; a name with the tag twice is left out (ambiguous), as are the
//! names with the tag "_Reco" or "_Closure". Only the directory itself is searched (no sub-directory).
//!
//! A file that cannot be compared (no detector-level file, a folder missing, an output file that exists while
//! EXPORT:FORCE_REWRITE is false, ...) is skipped with a warning, and the others are compared; the run ends with a
//! summary: the files compared and skipped, the histograms compared and, with a statistical test, how many were tested
//! and how many fail at the 5% level. It fails only if no file could be compared.
//!
//! Keys \<name\>:HISTOGRAM:METHOD (0: difference, 1: ratio), TEST (statistical test, see
//! ClosureCalculator::StatisticalTest: 0 none, 1 chi2, 2 Kolmogorov, 3 user defined), IMPORT:PATH (directory of the
//! files), IMPORT:GEN_FOLDER, IMPORT:REC_FOLDER (folders compared in each file, "" for all), EXPORT:PATH,
//! EXPORT:FORCE_REWRITE, and IncludedPattern0..4, ExcludedPattern0..4: the files must contain every included pattern
//! and no excluded pattern (plain, case-sensitive substrings of the file name, extension included; a key whose value
//! contains NONE is unused). Paths as in ClosureCalculator (DEFAULT: $CAP_HISTOS_IMPORT_PATH, $CAP_HISTOS_EXPORT_PATH).
//!
class ClosureIterator
:
public Task
{
public:
  //! Outcome of a run (run()).
  struct Summary
  {
    std::vector<String> compared;   //!< generator-level files compared
    std::vector<String> skipped;    //!< generator-level files skipped, each with the reason ("file: reason")
    long nHistograms = 0;           //!< histograms compared
    long nTested     = 0;           //!< histograms tested (statistical test)
    long nRejected   = 0;           //!< histograms whose closure is rejected at the 5% level
  };

  //! @brief Default constructor.
  ClosureIterator();
  //! @brief Destructor.
  virtual ~ClosureIterator() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Run the closure tests of the selected files (see run()).
  //! @throws Exception if no file is selected or none could be compared; FileException if IMPORT:PATH is not a
  //! directory
  virtual void execute();
  //!
  //! @brief Run the closure tests of the generator-level files of a directory (see the class description): a file
  //! that cannot be compared is skipped with a warning. @param method 0 (difference) or 1 (ratio) @param test
  //! statistical test (ClosureCalculator::StatisticalTest) @param importDirectory directory of the files (resolved)
  //! @param genFolder folder compared in the generator-level files ("": all) @param recoFolder folder compared in the
  //! detector-level files ("": all) @param exportDirectory directory of the closure files (resolved)
  //! @param forceRewrite whether existing closure files are replaced @param included patterns every file must contain
  //! @param excluded patterns no file may contain @return summary
  //! @throws Exception if no file is selected or none could be compared; FileException if importDirectory is not a
  //! directory
  //!
  Summary run(int method, int test, const String & importDirectory, const String & genFolder, const String & recoFolder,
              const String & exportDirectory, bool forceRewrite,
              const std::vector<String> & included=std::vector<String>(),
              const std::vector<String> & excluded=std::vector<String>());
  //!
  //! @brief Positions of a tag in a file name: where it is followed by "_", "." or the end of the name.
  //! @param fileName name @param tag tag (e.g. "_Gen") @return positions
  //!
  static std::vector<std::size_t> tagPositions(const String & fileName, const String & tag);
  //!
  //! @brief Name of the counterpart of a generator-level file: its tag "_Gen" replaced by another tag.
  //! @param genFileName name of the generator-level file @param tag new tag (e.g. "_Reco", "_Closure")
  //! @return name, or "" if the name does not hold the tag "_Gen" exactly once
  //!
  static String counterpart(const String & genFileName, const String & tag);
};

} // namespace CAP

#endif /* CAP__ClosureIterator */
