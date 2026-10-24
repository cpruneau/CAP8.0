/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
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
