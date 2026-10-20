/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ObjectCreator.cpp
//! @brief Dictionaries of the creators of filters, efficiencies and smearers, and FilterCreator.
#include "ObjectCreator.hpp"

templateClassImp(CAP::ObjectCreator);
ClassImp(CAP::EventFilterCreator);
ClassImp(CAP::ParticleFilterCreator);
ClassImp(CAP::EventEfficiencyCreator);
ClassImp(CAP::ParticleEfficiencyCreator);
ClassImp(CAP::EventSmearerCreator);
ClassImp(CAP::ParticleSmearerCreator);
ClassImp(CAP::FilterCreator);

namespace CAP
{

  FilterCreator::FilterCreator()
  :
  Task()
  {
  appendClassName("FilterCreator");
  setMinimumReportLevel(Object::kInfo);
  setName("FilterCreator");
  setTitle("FilterCreator");
  }

} // namespace CAP
