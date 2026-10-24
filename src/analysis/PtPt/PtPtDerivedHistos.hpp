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
#ifndef CAP__PtPtDerivedHistos
#define CAP__PtPtDerivedHistos
#include "HistogramFolder.hpp"
#include "PtPtHistos.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Quantities derived from the pT moments: factorial cumulants F1 to F4, normalized cumulants R2 to R4, average
//! pT moments q1, q1q1, ... and the correlators of pT deviations dq1dq1, dq1dq1dq1, dq1dq1dq1dq1, vs the event variable.
//!
class PtPtDerivedHistos : public PtPtHistos
{
public:

  //! @brief Default constructor.
  PtPtDerivedHistos();
  //! @brief Copy constructor. @param source group to copy
  PtPtDerivedHistos(const PtPtDerivedHistos & source);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  PtPtDerivedHistos & operator=(const PtPtDerivedHistos & rhs);
  //! @brief Destructor.
  virtual ~PtPtDerivedHistos() {}
  //! @brief Read the event variable name and binning. @param taskName name of the task @param objectType object type
  //! @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);

  //! @brief Create the derived histograms.
  virtual void create();
  //! @brief Load the derived histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Compute the derived quantities from the moments. @param baseHistos moments
  virtual void calculateDerivedHistograms(const PtPtHistos & baseHistos);

protected:
  //! @brief Copy the content of another group. @param group group to copy
  void cloneD(const PtPtDerivedHistos & group);
  Histogram * h_F1_evt;
  Histogram * h_F2_evt;
  Histogram * h_F3_evt;
  Histogram * h_F4_evt;
  Histogram * h_R2_evt;
  Histogram * h_R3_evt;
  Histogram * h_R4_evt;

  Histogram * h_q1_evt;
  Histogram * h_q1q1_evt;
  Histogram * h_q1q1q1_evt;
  Histogram * h_q1q1q1q1_evt;
  Histogram * h_dq1dq1_evt;
  Histogram * h_dq1dq1dq1_evt;
  Histogram * h_dq1dq1dq1dq1_evt;
  
  //! Particle filters.
  vector<ParticleFilter*> particleFilters;
  ClassDef(PtPtDerivedHistos,0)
};

} // namespace CAP

#endif /* CAP__PtPtDerivedHistos  */



