/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,  Nov 10, 2026
 * Based on TGlauber.C written originally by B. Alver, M. Baker, 
 * C. Loizides, P. Steinberg, [arXiv:0805.4411](https://arxiv.org/abs/0805.4411) [nucl-ex]
 *
 * *********************************************************************/
//! @file GlauberTasks.cpp
//! @brief Task classes of the Glauber library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "GlauberAnalyzer.hpp"
#include "GlauberGenerator.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(GlauberTasks)
{
  CAP::registerTask<CAP::GlauberAnalyzer>("CAP::GlauberAnalyzer");
  CAP::registerTask<CAP::GlauberGenerator>("CAP::GlauberGenerator");
}
