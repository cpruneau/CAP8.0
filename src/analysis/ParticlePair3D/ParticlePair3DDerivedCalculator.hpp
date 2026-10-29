/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#ifndef CAP__ParticlePair3DDerivedCalculator
#define CAP__ParticlePair3DDerivedCalculator
#include "EventProcessorHistos.hpp"
#include "ParticleSingleHistos.hpp"
#include "ParticlePair3DHistos.hpp"

namespace CAP
{

//!
//! @brief Derived histograms of a 3D pair analysis (second phase, no event processing): reads the histograms filled by a
//! ParticlePair3DAnalyzer from the input file of the job (its n*m single folders and n*m*m pair folders), computes the
//! derived histograms of each single folder (e, i) and then of each pair folder (e, i, j) from it and the single folders
//! (e, i) and (e, j) (ParticlePair3DHistos::calculateDerived(): n1n1, C2, R2, associated yields vs Qinv and
//! (q_side, q_out, q_long)), and writes the completed folders to the output file of the job.
//!
//! It uses the same filters and histogram settings as the analyzer, and reads and writes the folders of the analyzer:
//! those of the folder named by the key \<name\>:SOURCE_TASK (the name of the analyzer task).
//!
class ParticlePair3DDerivedCalculator
:
public EventProcessorHistos<ParticlePair3DHistos,2,ParticleSingleHistos>
{
public:

  //! @brief Default constructor.
  ParticlePair3DDerivedCalculator();
  //! @brief Copy constructor. @param source task to copy
  ParticlePair3DDerivedCalculator(const ParticlePair3DDerivedCalculator & source);
  //! @brief Copy assignment. @param rhs task to copy @return this task
  ParticlePair3DDerivedCalculator & operator=(const ParticlePair3DDerivedCalculator & rhs);
  //! @brief Destructor.
  virtual ~ParticlePair3DDerivedCalculator() {}
  //! @brief Add the default configuration: that of EventProcessorHistos, the histogram settings and SOURCE_TASK.
  virtual void setDefaultConfiguration() override;
  //! @brief Configure the task (and read SOURCE_TASK). @param configuration configuration
  virtual void configure(const Configuration & configuration) override;
  //!
  //! @brief Compute the derived histograms of every folder from its base histograms.
  //! @throws Exception if there are fewer folders than combinations of filters
  //!
  virtual void execute() override;
  //! @brief Name of the folder of the job files holding the histograms: that of the analyzer (SOURCE_TASK). @return name
  virtual String histogramTaskName() const override { return _sourceTask; }

protected:

  //! Name of the analyzer task whose histograms are completed.
  String _sourceTask;

};

} // namespace CAP

#endif /* CAP__ParticlePair3DDerivedCalculator */
