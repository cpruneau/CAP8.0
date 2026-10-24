/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticlePerformanceDerivedHistos
#define CAP__ParticlePerformanceDerivedHistos
#include "ParticlePerformanceHistos.hpp"
#include "ClassDef.hpp"

namespace CAP
{
//!
//! @brief Quantities derived from the performance histograms (resolutions and biases).
//!
class ParticlePerformanceDerivedHistos : public ParticlePerformanceHistos
{
public:
  
  //! @brief Default constructor.
  //! @brief Default constructor.
  ParticlePerformanceDerivedHistos();
  //! @brief Destructor.
  virtual ~ParticlePerformanceDerivedHistos() {}
  //! @brief Configure the object from keys \<taskName\>:\<objectType\>:... (binning, options).
  //! @param taskName name of the task @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String& taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Compute the derived histograms. @param base performance histograms
  virtual void calculateDerivedHistograms(const ParticlePerformanceHistos & base);

  ClassDef(ParticlePerformanceDerivedHistos,0)
};

} // namespace CAP

#endif /* CAP__ParticlePerformanceDerivedHistos  */



