/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticlePerformanceHistos
#define CAP__ParticlePerformanceHistos
#include "HistogramFolder.hpp"
#include "Particle.hpp"
#include "Configuration.hpp"

namespace CAP
{


//!
//! @brief Older copy of ParticlePerformanceHistos (same class name).
//! @note Not part of the build (not listed in Performance/CMakeLists.txt).
//!
class ParticlePerformanceHistos : public HistogramFolder
{
public:
  
  //! @brief Default constructor.
  ParticlePerformanceHistos();
  //! @brief Destructor.
  virtual ~ParticlePerformanceHistos() {}
  //! @brief Configure the object from keys \<taskName\>:\<objectType\>:... (binning, options).
  //! @param taskName name of the task @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  virtual void fill(Particle & particle, double weight);
  
protected:

  Histogram * h_n1_dpt;
  Histogram * h_n1_deta;
  Histogram * h_n1_dphi;
  Histogram * h_n1_dy;

  Profile * p_n1_dptVsPt;
  Profile * p_n1_dptVsEta;
  Profile * p_n1_detaVsPt;
  Profile * p_n1_detaVsEta;
  Profile * p_n1_dyVsPt;
  Profile * p_n1_dyVsEta;
  Profile * p_n1_dphiVsPt;
  Profile * p_n1_dphiVsEta;


};

} // namespace CAP

#endif /* CAP__ParticlePerformanceHistos  */



