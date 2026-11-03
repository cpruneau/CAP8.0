/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file WorkflowHistograms.hpp
//! @brief WorkflowHistograms: the histograms the stages of a workflow (RunCAP.py) write, made empty without running.
#ifndef CAP__WorkflowHistograms
#define CAP__WorkflowHistograms
#include <memory>
#include <vector>
#include "Folder.hpp"
#include "String.hpp"
#include "XmlDocument.hpp"

namespace CAP
{

//!
//! @brief The histograms a workflow file writes (the XML file of RunCAP.py, a TASK element per stage), known without
//! running it: each stage is made and configured as bin/CAP would, then each of its analysis tasks makes its folders
//! of histograms empty (AnalysisTask::makeEmptyHistograms()): the folders of the combinations of its event and
//! particle filters, with the histograms its settings ask for. Nothing is initialized or executed: no event is
//! generated, no input file is read. On Unix (macOS, Linux), each stage is made in a child process, as RunCAP.py runs
//! each in a job of its own: the stores of filters and histograms of a stage do not meet those of the others, and
//! its messages are not shown (what fails is in its problems).
//! @code
//!   std::vector<WorkflowHistograms::Stage> stages = WorkflowHistograms::make(XmlDocument::read("wf.xml"), error);
//!   for (auto & s : stages) ... s.outputFile ... s.folder->paths() ...
//! @endcode
//!
struct WorkflowHistograms
{
  //! A stage of the workflow and what its job writes
  struct Stage
  {
  String name;         //!< name of the stage (attribute stage of its TASK element, or the name of the task)
  String task;         //!< name of the main task of the stage
  String outputFile;   //!< output file of its job (JOB:OUTPUT_FILE)
  std::shared_ptr<Folder> folder;   //!< the folders of histograms of its job, empty
  String problems;     //!< what could not be made, one per line
  };
  //!
  //! @brief The stages of a workflow and the histograms they write. @param workflow document (root CAP, a TASK element
  //! per stage) @param error set to the reason of a failure (a stage that cannot be made is reported in its problems)
  //! @return stages, in order
  //!
  static std::vector<Stage> make(const XmlDocument & workflow, String & error);
  //!
  //! @brief Run stages of a workflow on histograms given in memory, as bin/CAP runs their jobs (each stage made from
  //! its TASK element, initialized, given the histograms, executed, finalized), the output of a stage being the input
  //! of the next; on Unix, in a child process (the stores of filters and histograms are those of one job, as when
  //! RunCAP.py runs it; its messages are not shown unless verbose). Used by the jackknife (SubSampleStatCalculator)
  //! to derive the observables of each replica. @param workflow document (root CAP, a TASK element per stage)
  //! @param stages names of the stages (attribute stage, or name, of their TASK elements), in order
  //! @param input histograms given to the first stage (as its input file) @param output set to what the last stage
  //! writes (as its output file) @param error set to the reason of a failure @param verbose show the messages of the
  //! stages @return false on failure
  //!
  static bool runStages(const XmlDocument & workflow, const std::vector<String> & stages, const Folder & input,
                        Folder & output, String & error, bool verbose=false);
};

} // namespace CAP

#endif /* CAP__WorkflowHistograms */
