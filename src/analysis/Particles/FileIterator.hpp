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
#ifndef CAP__FileIterator
#define CAP__FileIterator
#include "EventProcessor.hpp"

namespace CAP
{

  //!
    //! @brief Task repeating the same subtasks on several input/output directories (e.g., to post-process the
    //! histograms of many runs).
    //!
    //! For each directory pair, execute() sets \<name\>:HistogramsImportPath and \<name\>:HistogramsExportPath in the
    //! configuration of each subtask and runs it. The directories are built from the environment variables
    //! CAP_HISTOS_IMPORT_PATH and CAP_HISTOS_EXPORT_PATH according to \<name\>:nLevels (0: these directories only).
    //! @note nLevels=1 (one level of subdirectories) is not implemented: execute() then throws.
  //!
  class FileIterator
  :
  public EventProcessor
  {
  public:

  //! @brief Default constructor.
  FileIterator();
  //! @brief Copy constructor. @param task task to copy
  FileIterator(const FileIterator & task);
  //! @brief Copy assignment. @param task task to copy @return copy
  FileIterator operator=(const FileIterator & task);
  //! @brief Destructor.
  virtual ~FileIterator() {}
  //! @brief Add the default configuration: \<name\>:nLevels = 0.
  virtual void setDefaultConfiguration();
  //! @brief Configure the task (reads \<name\>:nLevels). @param configuration configuration
  virtual void configure(const Configuration & configuration);
  //! @brief Run the subtasks on each directory (see the class description).
  //! @throws Exception if no directory is selected
  virtual void execute();

  protected:

//  bool isGrid;
//  int nBunches;
//  int nSubBunches;
//  String bunchLabel;
//  String subBunchLabel;
  //! Number of directory levels to iterate over (0: the import/export directories themselves).
  int _nLevels;
  ClassDef(FileIterator,0)
  };

} // namespace CAP

#endif /* CAP__FileIterator */
