/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__NuDynDerivedHistos
#define CAP__NuDynDerivedHistos
#include "HistogramFolder.hpp"
#include "NuDynHistos.hpp"
#include "ParticleFilter.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Quantities derived from the multiplicity moments (NuDynHistos) as a function of the event variable:
//! factorial cumulants F2, normalized moments R2 = f2/(f1 f1) and the nu_dyn variable
//! nu_dyn = R2_11 + R2_22 - 2 R2_12.
//!
class NuDynDerivedHistos : public NuDynHistos
{
public:

  //! @brief Default constructor.
  NuDynDerivedHistos();
  //! @brief Copy constructor. @param source group to copy
  NuDynDerivedHistos(const NuDynDerivedHistos & source);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  NuDynDerivedHistos & operator=(const NuDynDerivedHistos & rhs);
  //! @brief Destructor.
  virtual ~NuDynDerivedHistos() {}
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
  //! @brief Compute F2, R2 and nu_dyn from the moments. @param baseHistos moments
  virtual void calculateDerivedHistograms(const NuDynHistos & baseHistos);

protected:

  //! @brief Copy the content of another group. @param group group to copy
  void cloneD(const NuDynDerivedHistos & group);
  //! Factorial cumulant F2 of selection 1 vs event variable.
  Histogram * h_F2_11_evt;
  //! Factorial cumulant F2 of selections 1 and 2 vs event variable.
  Histogram * h_F2_12_evt;
  //! Factorial cumulant F2 of selection 2 vs event variable.
  Histogram * h_F2_22_evt;
  //! R2 of selection 1 vs event variable.
  Histogram * h_R2_11_evt;
  //! R2 of selections 1 and 2 vs event variable.
  Histogram * h_R2_12_evt;
  //! R2 of selection 2 vs event variable.
  Histogram * h_R2_22_evt;
  //! nu_dyn vs event variable.
  Histogram * h_nudyn_evt;

  ClassDef(NuDynDerivedHistos,0)
};

} // namespace CAP

#endif /* CAP__NuDynDerivedHistos  */



