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
#ifndef CAP__BaseHistos
#define CAP__BaseHistos
#include "HistogramFolder.hpp"
#include "Particle.hpp"
#include "MathConstants.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Base class of the histogram groups filled by the analysis tasks (single, pair, global, ... histograms).
//!
//! A BaseHistos is a HistogramFolder: the histograms are created by createHistograms(), filled event by event by the
//! owning task, and saved, imported and scaled through the HistogramFolder interface. The derived quantities computed
//! from these histograms at the end of the run are held by a BaseDerivedHistos.
//!
class BaseHistos : public HistogramFolder
{
public:
  //! @brief Default constructor.
  BaseHistos();
  //! @brief Copy constructor. @param group group to copy
  BaseHistos(const BaseHistos & group);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  BaseHistos & operator=(const BaseHistos & rhs);
  //! @brief Destructor.
  virtual ~BaseHistos(){ }
  //! @brief Copy the content of another group (warning only in the base class; derived classes override it).
  //! @param source group to copy
  virtual void cloneB(const BaseHistos & source);
  ClassDef(BaseHistos,0)
};

} // namespace CAP

#endif /* CAP__BaseDerivedHistos  */



