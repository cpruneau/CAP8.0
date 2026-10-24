/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani, Akash Raj
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau, Akash Raj,  Nov 2024
 *
 * *********************************************************************/
#ifndef CAP__JetCalculator
#define CAP__JetCalculator
#include "Calculator.hpp"
#include "JetFilter.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Task computing the derived jet histograms from previously filled jet histograms, for each jet filter.
//! @warning Work in progress: the Jets library is built only with FastJet, and this header does not compile as is (Calculator.hpp and ManagedList do not exist).
//!
class JetCalculator
:
public Calculator,
public ManagedList<JetFilter>
{
public:

  //! @brief Default constructor.
  JetCalculator();
  //! @brief Copy constructor. @param task object to copy
  JetCalculator(const JetCalculator & task);
  //! @brief Copy assignment. @param task object to copy @return this object
  JetCalculator & operator=(const JetCalculator & task);
  //! @brief Destructor.
  virtual ~JetCalculator() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Execute the task (process the current event).
  virtual void execute();
  //! @brief Create the histograms. @param configuration configuration
  virtual void create(const Configuration & configuration);
  //! @brief Load the histograms from a file. @param inputFile file @param configuration configuration
  virtual void loadFrom(const Folder & inputFile,const Configuration & configuration);
  //! @brief Normalize the histograms.
  virtual void scaleHistograms();

protected:

  //! Jet filters.
  std::vector<JetFilter*> jetFilters;
  //! @brief Number of jet filters. @return number
  unsigned int nJetFilters();

  ClassDef(JetCalculator,0)
};

} // namespace CAP

#endif /* CAP__JetCalculator */
