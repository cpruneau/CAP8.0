/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file MathReflection.cpp
//! @brief Classes of the Math library described for the run-time reflection (see Reflection.hpp): histograms and
//! random numbers. Registered when the library is loaded.
#include "Reflection.hpp"
#include "Histogram.hpp"
#include "MersenneTwister.hpp"

CAP_REFLECTION(Math)
{
  using namespace CAP;
  using namespace CAP::MATH;
  using namespace CAP::REFLECTION;

  ClassBuilder<Partition>("CAP::MATH::Partition")
    .property("rank", &Partition::rank)
    .property("nBinsTotal", &Partition::nBinsTotal)
    .method("nBins", &Partition::nBins)
    .method("minimum", &Partition::minimum)
    .method("maximum", &Partition::maximum);

  ClassBuilder<DataPoints>("CAP::MATH::DataPoints")
    .base<Object>()
    .property("size", &DataPoints::size);

  ClassBuilder<Histogram>("CAP::MATH::Histogram")
    .base<DataPoints>()
    .base<Partition>()
    .constructor<const String &, const String &, unsigned int, double, double>()
    .constructor<const String &, const String &, unsigned int, double, double, unsigned int, double, double>()
    .constructor<const String &, const String &, unsigned int, double, double, unsigned int, double, double,
                 unsigned int, double, double>()
    .constructor<const Histogram &>()
    .function("fill", +[](Histogram & h, double x) { h.fill(x, 1.0); })
    .method("fill", static_cast<void (Histogram::*)(double, double)>(&Histogram::fill))
    .method("fill", static_cast<void (Histogram::*)(double, double, double)>(&Histogram::fill))
    .method("fill", static_cast<void (Histogram::*)(double, double, double, double)>(&Histogram::fill))
    .method("binValueAt", static_cast<double (Histogram::*)(unsigned int) const>(&Histogram::binValueAt))
    .method("binValueAt", static_cast<double (Histogram::*)(unsigned int, unsigned int) const>(&Histogram::binValueAt))
    .method("binUncertaintyAt", static_cast<double (Histogram::*)(unsigned int) const>(&Histogram::binUncertaintyAt))
    .method("binEntriesAt", static_cast<double (Histogram::*)(unsigned int) const>(&Histogram::binEntriesAt))
    .method("mean", &Histogram::mean)
    .function("integral", +[](const Histogram & h) { return h.integral(); })
    .method("scale", &Histogram::scale)
    .method("print", &Histogram::print);

  ClassBuilder<RandomGenerator>("CAP::MATH::RandomGenerator")
    .method("random", &RandomGenerator::random)
    .method("setSeed", &RandomGenerator::setSeed)
    .method("uniform", &RandomGenerator::uniform)
    .method("uniformInteger", &RandomGenerator::uniformInteger)
    .method("gauss", &RandomGenerator::gauss)
    .function("exp", +[](RandomGenerator & g, double mean) { return g.exp(mean); })
    .method("exp", &RandomGenerator::exp)
    .method("poisson", &RandomGenerator::poisson)
    .method("binomial", &RandomGenerator::binomial)
    .method("gamma", &RandomGenerator::gamma)
    .method("beta", &RandomGenerator::beta)
    .method("chiSquare", &RandomGenerator::chiSquare)
    .method("polynomial", &RandomGenerator::polynomial);

  ClassBuilder<MersenneTwister>("CAP::MATH::MersenneTwister")
    .base<RandomGenerator>()
    .constructor<>()
    .constructor<std::uint64_t>()
    .method("next32", &MersenneTwister::next32)
    .property("seed", &MersenneTwister::seed);
}
