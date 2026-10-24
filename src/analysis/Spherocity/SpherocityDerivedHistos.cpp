/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "Particle.hpp"
#include "SpherocityDerivedHistos.hpp"
#include "GlobalHistos.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "Task.hpp"
#include "Configuration.hpp"


namespace CAP
{

SpherocityDerivedHistos::SpherocityDerivedHistos()
:
SpherocityHistos()
{
  appendClassName("SpherocityDerivedHistos");
  setName("SpherocityDerivedHistos");
  setTitle("SpherocityDerivedHistos");
}

SpherocityDerivedHistos::SpherocityDerivedHistos(const SpherocityDerivedHistos & group)
:
SpherocityHistos(group)
{
  cloneD(group);
}

SpherocityDerivedHistos & SpherocityDerivedHistos::operator=(const SpherocityDerivedHistos & source)
{
  if (this!=&source)
    {
    SpherocityHistos::operator=(source);
    cloneD(source);
    }
  return *this;
}

void SpherocityDerivedHistos::cloneD(const SpherocityDerivedHistos & source)
{
  cloneB(source);
  // in development
}

void SpherocityDerivedHistos::configure(const String & taskName,
                                              const String & objectType,
                                              const Configuration & configuration,
                                              unsigned int index)
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  SpherocityHistos::configure(taskName,objectType,configuration,index);
}

void SpherocityDerivedHistos::create()
{
  if (reportDebug(__FUNCTION__)) { printCR();}
}

//!
//!Load histograms corresponding to the selected event and particle filters from the given file.
void SpherocityDerivedHistos::loadFrom(const Folder & inputFile)
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  SpherocityHistos::loadFrom(inputFile);
}

void SpherocityDerivedHistos::calculateDerivedHistograms(SpherocityHistos & baseHistos __attribute__((unused)))
{
  // in development
}


} // namespace CAP
