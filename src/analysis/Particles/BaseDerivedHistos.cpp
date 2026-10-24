/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "BaseDerivedHistos.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Task.hpp"
#include "Configuration.hpp"
#include "NameHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"


namespace CAP
{

BaseDerivedHistos::BaseDerivedHistos()
:
BaseHistos()
{
  appendClassName("BaseDerivedHistos");
  setMinimumReportLevel(Object::kInfo);
  setName("BaseDerivedHistos");
  setTitle("BaseDerivedHistos");
}

BaseDerivedHistos::BaseDerivedHistos(const BaseDerivedHistos & source)
:
BaseHistos(source)
{
  cloneD(source);
}

BaseDerivedHistos & BaseDerivedHistos::operator=(const BaseDerivedHistos & rhs)
{
  if (reportWarning(__FUNCTION__)) { printCR(); }
  if (this!=&rhs)
    {
    BaseHistos::operator=(rhs);
    cloneD(rhs);
    }
  return *this;
}




} // namespace CAP
