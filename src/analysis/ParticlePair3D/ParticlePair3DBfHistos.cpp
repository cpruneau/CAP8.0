/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/

#include "ParticleSingleHistos.hpp"
#include "ParticleSingleDerivedHistos.hpp"
#include "ParticlePairHistos.hpp"
#include "HistogramFolder.hpp"

#include "ParticlePair3DBfHistos.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "TRandom.h"
#include "Task.hpp"
#include "Configuration.hpp"

ClassImp(CAP::ParticlePair3DBfHistos);

namespace CAP
{

ParticlePair3DBfHistos::ParticlePair3DBfHistos()
:
ParticlePair3DDerivedHistos(),
h_B12_Qinv(nullptr),
h_B12_DeltaPs(nullptr),
h_B12_DeltaPo(nullptr),
h_B12_DeltaPl(nullptr),
h_B21_Qinv(nullptr),
h_B21_DeltaPs(nullptr),
h_B21_DeltaPo(nullptr),
h_B21_DeltaPl(nullptr),
h_Bs_Qinv(nullptr),
h_Bs_DeltaPs(nullptr),
h_Bs_DeltaPo(nullptr),
h_Bs_DeltaPl(nullptr),
mass1(0.13957039),
mass2(0.13957039)
{ }

ParticlePair3DBfHistos::ParticlePair3DBfHistos(const ParticlePair3DBfHistos & source)
:
ParticlePair3DDerivedHistos(source),
h_B12_Qinv(nullptr),
h_B12_DeltaPs(nullptr),
h_B12_DeltaPo(nullptr),
h_B12_DeltaPl(nullptr),
h_B21_Qinv(nullptr),
h_B21_DeltaPs(nullptr),
h_B21_DeltaPo(nullptr),
h_B21_DeltaPl(nullptr),
h_Bs_Qinv(nullptr),
h_Bs_DeltaPs(nullptr),
h_Bs_DeltaPo(nullptr),
h_Bs_DeltaPl(nullptr),
mass1(source.mass1),
mass2(source.mass2)
{
  cloneBF(source);
}

ParticlePair3DBfHistos & ParticlePair3DBfHistos::operator=(const ParticlePair3DBfHistos & rhs)
{
  if (this!=&rhs)
    {
    ParticlePair3DDerivedHistos::operator=(rhs);
    cloneBF(rhs);
    mass1 = rhs.mass1;
    mass2 = rhs.mass2;
    }
  return *this;
}

void ParticlePair3DBfHistos::cloneBF(const ParticlePair3DBfHistos & source)
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_B12_Qinv     = histogramLike(source.h_B12_Qinv);
  h_B12_DeltaPs  = histogramLike(source.h_B12_DeltaPs);
  h_B12_DeltaPo  = histogramLike(source.h_B12_DeltaPo);
  h_B12_DeltaPl  = histogramLike(source.h_B12_DeltaPl);
  h_B21_Qinv     = histogramLike(source.h_B21_Qinv);
  h_B21_DeltaPs  = histogramLike(source.h_B21_DeltaPs);
  h_B21_DeltaPo  = histogramLike(source.h_B21_DeltaPo);
  h_B21_DeltaPl  = histogramLike(source.h_B21_DeltaPl);
  h_Bs_Qinv      = histogramLike(source.h_Bs_Qinv);
  h_Bs_DeltaPs   = histogramLike(source.h_Bs_DeltaPs);
  h_Bs_DeltaPo   = histogramLike(source.h_Bs_DeltaPo);
  h_Bs_DeltaPl   = histogramLike(source.h_Bs_DeltaPl);
}

void ParticlePair3DBfHistos::configure(const String & taskName,
                            const String & objectType,
                            const Configuration & configuration,
                            unsigned int index)
{
  ParticlePair3DHistos::configure(taskName,objectType,configuration,index);
}


void ParticlePair3DBfHistos::create()
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_B12_Qinv      = createHistogram(createName(_histogramBaseName,"B12_Qinv"),    Qinv_nbins,   Qinv_min,    Qinv_max,    "Q_{inv}","B_{12}", 2);
  h_B12_DeltaPs   = createHistogram(createName(_histogramBaseName,"B12_DeltaPs"), DeltaPs_nbins,DeltaPs_min, DeltaPs_max, "p_{s}","B_{12}");
  h_B12_DeltaPo   = createHistogram(createName(_histogramBaseName,"B12_DeltaPo"), DeltaPo_nbins,DeltaPo_min, DeltaPo_max, "p_{o}","B_{12}");
  h_B12_DeltaPl   = createHistogram(createName(_histogramBaseName,"B12_DeltaPl"), DeltaPl_nbins,DeltaPl_min, DeltaPl_max, "p_{l}","B_{12}");
  h_B21_Qinv      = createHistogram(createName(_histogramBaseName,"B21_Qinv"),    Qinv_nbins,   Qinv_min,    Qinv_max,    "Q_{inv}","B_{21}", 2);
  h_B21_DeltaPs   = createHistogram(createName(_histogramBaseName,"B21_DeltaPs"), DeltaPs_nbins,DeltaPs_min, DeltaPs_max, "p_{s}","B_{21}");
  h_B21_DeltaPo   = createHistogram(createName(_histogramBaseName,"B21_DeltaPo"), DeltaPo_nbins,DeltaPo_min, DeltaPo_max, "p_{o}","B_{21}");
  h_B21_DeltaPl   = createHistogram(createName(_histogramBaseName,"B21_DeltaPl"), DeltaPl_nbins,DeltaPl_min, DeltaPl_max, "p_{l}","B_{21}");
  h_Bs_Qinv       = createHistogram(createName(_histogramBaseName,"Bs_Qinv"),     Qinv_nbins,   Qinv_min,    Qinv_max,    "Q_{inv}","B_{s}", 2);
  h_Bs_DeltaPs    = createHistogram(createName(_histogramBaseName,"Bs_DeltaPs"),  DeltaPs_nbins,DeltaPs_min, DeltaPs_max, "p_{s}","B_{s}");
  h_Bs_DeltaPo    = createHistogram(createName(_histogramBaseName,"Bs_DeltaPo"),  DeltaPo_nbins,DeltaPo_min, DeltaPo_max, "p_{o}","B_{s}");
  h_Bs_DeltaPl    = createHistogram(createName(_histogramBaseName,"Bs_DeltaPl"),  DeltaPl_nbins,DeltaPl_min, DeltaPl_max, "p_{l}","B_{s1}");
}

void ParticlePair3DBfHistos::loadFrom(const Folder & inputFile)
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_B12_Qinv    = importHistogram(inputFile,createName(_histogramBaseName,"B12_Qinv"));
  h_B12_DeltaPs = importHistogram(inputFile,createName(_histogramBaseName,"B12_DeltaPs"));
  h_B12_DeltaPo = importHistogram(inputFile,createName(_histogramBaseName,"B12_DeltaPo"));
  h_B12_DeltaPl = importHistogram(inputFile,createName(_histogramBaseName,"B12_DeltaPl"));
  h_B21_Qinv    = importHistogram(inputFile,createName(_histogramBaseName,"B21_Qinv"));
  h_B21_DeltaPs = importHistogram(inputFile,createName(_histogramBaseName,"B21_DeltaPs"));
  h_B21_DeltaPo = importHistogram(inputFile,createName(_histogramBaseName,"B21_DeltaPo"));
  h_B21_DeltaPl = importHistogram(inputFile,createName(_histogramBaseName,"B21_DeltaPl"));
  h_Bs_Qinv     = importHistogram(inputFile,createName(_histogramBaseName,"Bs_Qinv"));
  h_Bs_DeltaPs  = importHistogram(inputFile,createName(_histogramBaseName,"Bs_DeltaPs"));
  h_Bs_DeltaPo  = importHistogram(inputFile,createName(_histogramBaseName,"Bs_DeltaPo"));
  h_Bs_DeltaPl  = importHistogram(inputFile,createName(_histogramBaseName,"Bs_DeltaPl"));
}

//!
//! HistogramFolder from ParticlePair3DBfHistos must be normalized "per event" before calling this function
//!
//!  calculateBfHistograms(pair12, pair12B,pair1B2,pair1B2B);  // ++, +-, -+, --
//!
void ParticlePair3DBfHistos::calculateBfHistograms(ParticlePair3DDerivedHistos & pair12B,
                                                   ParticlePair3DDerivedHistos & pair1B2B,
                                                   ParticlePair3DDerivedHistos & pair1B2,
                                                   ParticlePair3DDerivedHistos & pair12)
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  // (-+) - (++)
  h_B12_Qinv   ->setAsSumOf(*pair1B2.h_a12_Qinv,*pair12.h_a12_Qinv,1.0,-1.0);
  h_B12_DeltaPs->setAsSumOf(*pair1B2.h_a12_DeltaPs,*pair12.h_a12_DeltaPs,1.0,-1.0);
  h_B12_DeltaPo->setAsSumOf(*pair1B2.h_a12_DeltaPo,*pair12.h_a12_DeltaPo,1.0,-1.0);
  h_B12_DeltaPl->setAsSumOf(*pair1B2.h_a12_DeltaPl,*pair12.h_a12_DeltaPl,1.0,-1.0);

  // (+-) - (--)
  h_B21_Qinv   ->setAsSumOf(*pair12B.h_a21_Qinv,*pair1B2B.h_a21_Qinv,1.0,-1.0);
  h_B21_DeltaPs->setAsSumOf(*pair12B.h_a21_DeltaPs,*pair1B2B.h_a21_DeltaPs,1.0,-1.0);
  h_B21_DeltaPo->setAsSumOf(*pair12B.h_a21_DeltaPo,*pair1B2B.h_a21_DeltaPo,1.0,-1.0);
  h_B21_DeltaPl->setAsSumOf(*pair12B.h_a21_DeltaPl,*pair1B2B.h_a21_DeltaPl,1.0,-1.0);

  // 0.5( (-+) - (++) ) + 0.5 (  (+-) - (--) )
  h_Bs_Qinv    ->setAsSumOf(*h_B12_Qinv,*h_B21_Qinv,0.5,0.5);
  h_Bs_DeltaPs ->setAsSumOf(*h_B12_DeltaPs,*h_B12_DeltaPs,0.5,0.5);
  h_Bs_DeltaPo ->setAsSumOf(*h_B12_DeltaPo,*h_B12_DeltaPo,0.5,0.5);
  h_Bs_DeltaPl ->setAsSumOf(*h_B12_DeltaPl,*h_B12_DeltaPl,0.5,0.5);
}


//! set the masses to be used in the calculation of q_inv
//! The default value set at construction are the pion+ mass.
//!
void ParticlePair3DBfHistos::setMasses(double m1, double m2)
{
  mass1 = m1;
  mass2 = m2;
}


} //  namespace CAP
