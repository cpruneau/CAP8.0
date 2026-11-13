/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestRootConversion.cpp
//! @brief Tests of the conversions between CAP and ROOT histograms: histograms and profiles in memory (values,
//! uncertainties, entries, labels), and whole files (directories and folders, objects skipped).
//
// Works in a temporary directory. Prints each failed check and exits with the number of failures.
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <unistd.h>
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "CapFile.hpp"
#include "Directory.hpp"
#include "RootConversion.hpp"
#include "RootFileConversion.hpp"

using namespace CAP;

namespace
{
int nChecks = 0, nFailures = 0;

void check(bool ok, const std::string & what, int line)
{
  nChecks++;
  if (ok) return;
  nFailures++;
  std::printf("FAILED (line %d): %s\n", line, what.c_str());
}

bool close(double a, double b) { return std::fabs(a - b) <= 1e-9*(1.0 + std::fabs(a) + std::fabs(b)); }
} // namespace

#define CHECK(condition) check((condition), #condition, __LINE__)

int main()
{
  TH1::AddDirectory(false);

  // ---- histograms: ROOT -> CAP -> ROOT
  {
    TH1D r("r", "a ROOT histogram", 10, 0.0, 5.0);
    r.Sumw2();
    for (int k=0; k<100; k++) r.Fill(0.05*k);
    r.GetXaxis()->SetTitle("x");
    std::unique_ptr<MATH::Histogram> h(fromRootHistogram(r));
    CHECK(h->rank()==1 && h->nBins(0)==10 && h->name()=="r" && h->title()=="a ROOT histogram");
    CHECK(close(h->binValueAt(3), r.GetBinContent(4)) && close(h->binUncertaintyAt(3), r.GetBinError(4)));
    CHECK(h->binEntriesAt(3)==long(r.GetBinContent(4)));          // unweighted counts: entries = content
    std::unique_ptr<TH1> back(toRootHistogram(*h));
    CHECK(back->GetNbinsX()==10 && close(back->GetBinContent(4), r.GetBinContent(4)));
    CHECK(close(back->GetEntries(), 100.0));
    CHECK(std::string(back->GetXaxis()->GetTitle())=="x");

    TH2D w("w", "weighted", 4, 0.0, 4.0, 3, 0.0, 3.0);
    w.Sumw2();
    w.Fill(1.5, 1.5, 0.5); w.Fill(1.5, 1.5, 0.25);
    w.GetYaxis()->SetBinLabel(2, "middle");
    std::unique_ptr<MATH::Histogram> h2(fromRootHistogram(w));
    CHECK(h2->rank()==2 && close(h2->binValueAt(1, 1), 0.75) && h2->binEntriesAt(1, 1)==1);   // weighted: one entry
    CHECK(h2->binLabels(1).size()==1);
  }

  // ---- profiles: same sums, same means and errors
  {
    TProfile r("p", "a profile", 5, 0.0, 5.0);
    for (int k=0; k<50; k++) r.Fill(0.1*k, std::sin(0.3*k), 1.0 + 0.1*(k % 3));
    std::unique_ptr<MATH::Histogram> h(fromRootHistogram(r));
    MATH::Profile * p = dynamic_cast<MATH::Profile*>(h.get());
    CHECK(p != nullptr);
    if (p)
      {
      CHECK(close(p->binValueAt(2), r.GetBinContent(3)));
      CHECK(close(p->binUncertaintyAt(2), r.GetBinError(3)));
      std::unique_ptr<TH1> back(toRootHistogram(*p));
      CHECK(back->InheritsFrom(TProfile::Class()));
      CHECK(close(back->GetBinContent(3), r.GetBinContent(3)) && close(back->GetBinError(3), r.GetBinError(3)));
      }
  }

  // ---- files: directories <-> folders, other objects skipped
  char buffer[] = "/tmp/CapTestRootConversion.XXXXXX";
  Directory top{String(mkdtemp(buffer))};
  String rootPath = top.join("input.root");
  {
    std::unique_ptr<TFile> f(TFile::Open(rootPath, "RECREATE"));
    TH1D a("a", "a", 5, 0.0, 1.0); a.Fill(0.5);
    f->WriteTObject(&a, "a");
    TDirectory * d = f->mkdir("singles");
    TProfile p("p", "p", 3, 0.0, 3.0); p.Fill(1.5, 2.0);
    d->WriteTObject(&p, "p");
    TGraph g(2); g.SetPoint(0, 0, 0); g.SetPoint(1, 1, 1);
    d->WriteTObject(&g, "graph");
    f->Close();
  }
  Folder tree("imported");
  std::vector<String> skipped;
  CHECK(importRootFile(rootPath, tree, &skipped)==2);
  CHECK(tree.get<MATH::Histogram>("a")!=nullptr && tree.get<MATH::Profile>("singles/p")!=nullptr);
  CHECK(skipped.size()==1 && skipped[0].str().find("graph")!=std::string::npos);
  // saved as a CAP file, read back lazily, exported to ROOT
  String capPath = top.join("imported.cap");
  {
    CapFile out(capPath, File::kWriteOnly);
    out = tree;
    out.write();
  }
  String outPath = top.join("output.root");
  {
    CapFile in(capPath, File::kReadOnly);
    in.load(File::Loading::kLazy);
    CHECK(exportRootFile(in, outPath)==2);
  }
  {
    std::unique_ptr<TFile> f(TFile::Open(outPath, "READ"));
    TH1 * a = f ? dynamic_cast<TH1*>(f->Get("a")) : nullptr;
    TProfile * p = f ? dynamic_cast<TProfile*>(f->Get("singles/p")) : nullptr;
    CHECK(a && close(a->GetBinContent(3), 1.0));
    CHECK(p && close(p->GetBinContent(2), 2.0));
  }
  top.remove(true);

  std::printf("TestRootConversion: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
