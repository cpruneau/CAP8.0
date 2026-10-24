/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__GlobalDerivedHistos
#define CAP__GlobalDerivedHistos
#include "HistogramFolder.hpp"
#include "ParticleFilter.hpp"
#include "GlobalHistos.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Quantities derived from the global histograms: inclusive average pT vs multiplicity (average pT sum divided
//! by the multiplicity).
//!
class GlobalDerivedHistos : public GlobalHistos
{
public://

  //! @brief Default constructor.
  GlobalDerivedHistos();
  //! @brief Destructor.
  virtual ~GlobalDerivedHistos() {} 

  //! @brief Read the binning. @param taskName name of the task @param objectType object type
  //! @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the derived histograms.
  virtual void create();
  //! @brief Load the derived histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Compute the derived histograms. @param baseHistos global histograms
  virtual void calculateDerivedHistograms(const GlobalHistos & baseHistos);

protected:
  
  //! @brief Copy the content of another group. @param group group to copy
  void cloneD(const GlobalDerivedHistos & group);

  Histogram* h_ptAvgIncVsN; //! Inclusive average pT vs multiplicity computed from ratio of ptSum profile average by average number of particles at given mul

  ClassDef(GlobalDerivedHistos,0)

};

}

#endif /* CAP__GlobalDerivedHistos  */



