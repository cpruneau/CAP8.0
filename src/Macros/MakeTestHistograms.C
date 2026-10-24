/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file MakeTestHistograms.C
//! @brief ROOT macro creating test histograms in a HistogramFolder and saving them in a CAP file.
//!
//! Create test histograms in a HistogramFolder, fill them, and save the folder in a CAP file, to try the graphical
//! interface (bin/CapGui) on known contents.
//!
//! The histograms are created in step 1 and filled in step 2: replace the example of step 2 by your own filling code
//! (it uses TRandom3 random numbers). Step 3 saves the folder in the file.
//!
//! Usage (after sourcing SetupCAP.sh):
//!   root -l -b -q 'MakeTestHistograms.C("myTest.cap")'           // 100000 entries per histogram
//!   root -l -b -q 'MakeTestHistograms.C("myTest", 1000000)'      // ".cap" is added if missing
//!   CapGui myTest.cap                                             // look at it
//!
#include <cmath>
#include <iostream>
#include "TRandom3.h"
#include "TString.h"

//! @brief Create, fill and save the test histograms. @param fileName CAP file (".cap" is added if missing)
//! @param nEntries entries per histogram @return 0 if the file was written
int MakeTestHistograms(const char * fileName="TestHistograms.cap", long nEntries=100000)
{
  using namespace CAP;
  using MATH::Histogram;
  using MATH::Profile;

  HistogramFolder histograms("TestHistograms");   // saved at the top of the file
  HistogramFolder pairs("pairs");                 // saved in the sub-folder "pairs" of the file

  // ---------------------------------------------------------------- 1. create the histograms
  // createHistogram(name, nBins, min, max, x title, value title) for 1D; (name, nx, xmin, xmax, ny, ymin, ymax, x title,
  // y title, value title) for 2D; createProfile(name, nBins, min, max, x title, mean value title) for profiles.
  Histogram * gauss   = histograms.createHistogram("gauss",   100, -5.0,  5.0, "x", "counts");
  Histogram * narrow  = histograms.createHistogram("narrow",  100, -5.0,  5.0, "x", "counts");
  Histogram * expo    = histograms.createHistogram("expo",    100,  0.0, 10.0, "t", "N(t)");
  Histogram * pt      = histograms.createHistogram("pt",      100,  0.0,  5.0, "p_{T} (GeV/c)", "dN/dp_{T}");
  Histogram * flat    = histograms.createHistogram("flat",     50,  0.0,  1.0, "u", "counts");
  Histogram * xy      = histograms.createHistogram("xy",       50, -3.0,  3.0, 50, -3.0, 3.0, "x", "y", "counts");
  Profile   * meanY   = histograms.createProfile(  "meanY_vs_x", 50, -3.0, 3.0, "x", "<y>");
  Histogram * dphi    = pairs.createHistogram(     "dphi",     72, -M_PI/2, 3*M_PI/2, "#Delta#phi", "pairs");

  // ---------------------------------------------------------------- 2. fill them (replace by your own code)
  // fill(x) or fill(x, weight) for 1D, fill(x, y, weight) for 2D, fill(x, value) for profiles.
  TRandom3 random(124325);
  for (long k=0; k<nEntries; k++)
    {
    double x = random.Gaus(0.0, 1.0);
    double y = 0.5*x + random.Gaus(0.0, 0.8);
    gauss->fill(x);
    narrow->fill(random.Gaus(1.0, 0.3));
    expo->fill(random.Exp(1.5));
    pt->fill(random.Gaus(0.0, 0.7)*random.Gaus(0.0, 0.7) + random.Exp(0.5));
    flat->fill(random.Uniform());
    xy->fill(x, y, 1.0);
    meanY->fill(x, y);
    dphi->fill(random.Uniform() < 0.3 ? random.Gaus(0.0, 0.4) : random.Uniform(-M_PI/2, 3*M_PI/2));
    }

  // ---------------------------------------------------------------- 3. save the folders in the CAP file
  try
    {
    String path = CapFile::makeFileName("", fileName);    // ".cap" appended if missing
    CapFile file(path, File::kWriteOnly);
    file.setFolderName(histograms.name());
    histograms.saveTo(file);                               // copies of the histograms, at the top of the file
    Folder * subFolder = file.addFolder(pairs.name());
    for (const Histogram * h : pairs.histograms()) subFolder->add(new Histogram(*h), h->name());
    file.write();
    std::cout << "Saved " << histograms.nHistograms() + pairs.nHistograms() << " histograms in " << path.data()
              << " (" << file.fileSize() << " bytes)" << std::endl;
    }
  catch (const std::exception & e)
    {
    std::cout << "<ERROR> " << e.what() << std::endl;
    return 1;
    }
  return 0;
}
