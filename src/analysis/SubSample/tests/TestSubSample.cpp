/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestSubSample.cpp
//! @brief Tests of SubSampleStatCalculator: the weighted average of sub-samples with the uncertainty of the mean, the
//! jackknife of the results (option B) and the jackknife through stages of a workflow (option A, with a test stage
//! squaring a histogram), against formulas computed independently; and the task itself on files (directory pattern,
//! directories without the file, weights from the accountants, the output and bias-corrected files).
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <random>
#include <string>
#include <unistd.h>
#include <vector>
#include "CapFile.hpp"
#include "Exceptions.hpp"
#include "FileSystem.hpp"
#include "Folder.hpp"
#include "Histogram.hpp"
#include "NameHelpers.hpp"
#include "SubSampleStatCalculator.hpp"
#include "TaskRegistration.hpp"
#include "WorkflowHistograms.hpp"
#include "XmlDocument.hpp"

using namespace CAP;
using MATH::Histogram;

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
#define CHECK(x) check((x), #x, __LINE__)

bool close(double a, double b, double tolerance = 2e-6)
{
  return std::fabs(a - b) <= tolerance * (1.0 + std::fabs(a) + std::fabs(b));
}

const unsigned int nBins = 4;

//! A stage of a test workflow: squares the histogram Ana/pairs/h of its input into Square/h2.
class TestSquare : public Task
{
public:
  TestSquare() { appendClassName("TestSquare"); setName("TestSquare"); }
  void loadHistograms(const Folder & top) override
  {
    const Histogram * h = top.get<Histogram>("Ana/pairs/h");
    if (!h) throw Exception("no Ana/pairs/h", "TestSquare");
    _h.reset(new Histogram(*h));
  }
  void execute() override
  {
    std::vector<double> v = _h->binValues(), e = _h->binValueUncertainties();
    for (unsigned int k = 0; k < v.size(); k++) { e[k] = 2.0 * std::fabs(v[k]) * e[k]; v[k] = v[k] * v[k]; }
    _h->setWith(_h->binEntries(), v, e);
  }
  void saveHistograms(Folder & top) override
  {
    Histogram * out = new Histogram(*_h);
    out->setName("h2");
    top.addFolder("Square")->add(out, "h2");
  }
private:
  std::unique_ptr<Histogram> _h;
};

//! A sub-sample: two histograms in nested folders.
Folder makeSample(std::mt19937 & random, std::vector<double> & h, std::vector<double> & s)
{
  std::normal_distribution<double> gauss(0.0, 1.0);
  Folder top;
  Folder * pairs = top.addFolder("Ana")->addFolder("pairs");
  Histogram * hh = pairs->add(new Histogram("h", "h", nBins, 0.0, 1.0), "h");
  Histogram * hs = top.folder("Ana")->add(new Histogram("s", "s", nBins, 0.0, 1.0), "s");
  h.resize(nBins); s.resize(nBins);
  std::vector<long> entries(nBins, 10);
  std::vector<double> errors(nBins, 0.1);
  for (unsigned int k = 0; k < nBins; k++) { h[k] = 2.0 + 0.3 * gauss(random); s[k] = -1.0 + 0.5 * gauss(random); }
  hh->setWith(entries, h, errors);
  hs->setWith(entries, s, errors);
  return top;
}

std::vector<double> valuesOf(const Folder & f, const char * path)
{
  const Histogram * h = f.get<Histogram>(path);
  return h ? h->binValues() : std::vector<double>();
}
std::vector<double> errorsOf(const Folder & f, const char * path)
{
  const Histogram * h = f.get<Histogram>(path);
  return h ? h->binValueUncertainties() : std::vector<double>();
}

//! The task with its configuration made settable.
class TestCalculator : public SubSampleStatCalculator
{
public:
  TestCalculator() { setDefaultConfiguration(); }
  void set(const char * key, const String & value) { _configuration.addProperty(createKey(name(), key), value); }
};
} // namespace

int main()
{
  registerTask<TestSquare>("TestSquare");
  std::mt19937 random(12345);
  const unsigned int n = 5;
  std::vector<Folder> samples;
  std::vector<std::vector<double>> h(n), s(n);
  std::vector<double> w = {100, 250, 180, 90, 300};
  for (unsigned int i = 0; i < n; i++) samples.push_back(makeSample(random, h[i], s[i]));
  std::vector<const Folder *> pointers;
  for (auto & f : samples) pointers.push_back(&f);
  double W = 0.0;
  for (double x : w) W += x;

  // --- SUBSAMPLE: weighted average and uncertainty of the mean
  Folder mean;
  SubSampleStatCalculator::average(pointers, w, mean);
  CHECK(SubSampleStatCalculator::histogramPaths(mean).size() == 2);
  std::vector<double> m(nBins);
  {
  std::vector<double> mv = valuesOf(mean, "Ana/pairs/h"), me = errorsOf(mean, "Ana/pairs/h");
  CHECK(mv.size() == nBins);
  bool ok = true;
  for (unsigned int k = 0; k < nBins && mv.size() == nBins; k++)
    {
    double sx = 0.0;
    for (unsigned int i = 0; i < n; i++) sx += w[i] * h[i][k];
    m[k] = sx / W;
    double q = 0.0;
    for (unsigned int i = 0; i < n; i++) q += w[i] * w[i] * (h[i][k] - m[k]) * (h[i][k] - m[k]);
    double se = std::sqrt(double(n) / (n - 1) * q) / W;
    ok = ok && close(mv[k], m[k]) && close(me[k], se);
    }
  CHECK(ok);
  CHECK(mean.get<Histogram>("Ana/pairs/h")->binEntries()[0] == long(10 * n));
  // equal weights: the standard deviation of the sub-samples divided by sqrt(N)
  Folder equal;
  SubSampleStatCalculator::average(pointers, std::vector<double>(n, 1.0), equal);
  std::vector<double> ev = valuesOf(equal, "Ana/s"), ee = errorsOf(equal, "Ana/s");
  ok = true;
  for (unsigned int k = 0; k < nBins; k++)
    {
    double a = 0.0, q = 0.0;
    for (unsigned int i = 0; i < n; i++) a += s[i][k] / n;
    for (unsigned int i = 0; i < n; i++) q += (s[i][k] - a) * (s[i][k] - a);
    ok = ok && close(ev[k], a) && close(ee[k], std::sqrt(q / (n - 1) / n));
    }
  CHECK(ok);
  }

  // --- JACKKNIFE B: replicas leave one sub-sample out of the average
  std::vector<Folder> replicas(n);
  std::vector<const Folder *> replicaPointers;
  for (unsigned int i = 0; i < n; i++)
    {
    SubSampleStatCalculator::leaveOneOut(mean, samples[i], w[i], W, replicas[i]);
    replicaPointers.push_back(&replicas[i]);
    }
  {
  bool ok = true;
  for (unsigned int i = 0; i < n; i++)
    {
    std::vector<double> r = valuesOf(replicas[i], "Ana/pairs/h");
    for (unsigned int k = 0; k < nBins; k++)
      {
      double sx = 0.0;
      for (unsigned int j = 0; j < n; j++) if (j != i) sx += w[j] * h[j][k];
      ok = ok && close(r[k], sx / (W - w[i]));
      }
    }
  CHECK(ok);
  }
  Folder jackB, biasB;
  SubSampleStatCalculator::jackknife(mean, replicaPointers, jackB, biasB);
  {
  std::vector<double> jv = valuesOf(jackB, "Ana/pairs/h"), je = errorsOf(jackB, "Ana/pairs/h");
  std::vector<double> bv = valuesOf(biasB, "Ana/pairs/h");
  bool ok = true;
  for (unsigned int k = 0; k < nBins; k++)
    {
    double tbar = 0.0;
    std::vector<double> t(n);
    for (unsigned int i = 0; i < n; i++)
      {
      double sx = 0.0;
      for (unsigned int j = 0; j < n; j++) if (j != i) sx += w[j] * h[j][k];
      t[i] = sx / (W - w[i]);
      tbar += t[i] / n;
      }
    double q = 0.0;
    for (unsigned int i = 0; i < n; i++) q += (t[i] - tbar) * (t[i] - tbar);
    ok = ok && close(jv[k], m[k]) && close(je[k], std::sqrt((n - 1.0) / n * q))
            && close(bv[k], n * m[k] - (n - 1.0) * tbar);
    }
  CHECK(ok);
  }
  // equal weights, a linear observable: the jackknife equals the uncertainty of the mean and has no bias
  {
  std::vector<double> ones(n, 1.0);
  Folder equal;
  SubSampleStatCalculator::average(pointers, ones, equal);
  std::vector<Folder> r(n);
  std::vector<const Folder *> rp;
  for (unsigned int i = 0; i < n; i++) { SubSampleStatCalculator::leaveOneOut(equal, samples[i], 1.0, n, r[i]); rp.push_back(&r[i]); }
  Folder j, b;
  SubSampleStatCalculator::jackknife(equal, rp, j, b);
  std::vector<double> ee = errorsOf(equal, "Ana/s"), je = errorsOf(j, "Ana/s");
  std::vector<double> ev = valuesOf(equal, "Ana/s"), bv = valuesOf(b, "Ana/s");
  bool ok = true;
  for (unsigned int k = 0; k < nBins; k++) ok = ok && close(ee[k], je[k]) && close(ev[k], bv[k]);
  CHECK(ok);
  }

  // --- JACKKNIFE A: stages (square) on the whole sample and on each replica
  XmlDocument workflow = XmlDocument::parse(
    "<CAP><TASK name=\"Square\" class=\"TestSquare\" stage=\"Square\"/></CAP>");
  Folder whole;
  String error;
  bool ran = WorkflowHistograms::runStages(workflow, {String("Square")}, mean, whole, error);
  CHECK(ran);
  if (!ran) std::printf("  runStages: %s\n", error.str().c_str());
  std::vector<double> squared = valuesOf(whole, "Square/h2");
  CHECK(squared.size() == nBins && close(squared[0], m[0] * m[0]));
  std::vector<Folder> derived(n);
  std::vector<const Folder *> derivedPointers;
  for (unsigned int i = 0; i < n; i++)
    {
    CHECK(WorkflowHistograms::runStages(workflow, {String("Square")}, replicas[i], derived[i], error));
    derivedPointers.push_back(&derived[i]);
    }
  Folder jackA, biasA;
  SubSampleStatCalculator::jackknife(whole, derivedPointers, jackA, biasA);
  {
  std::vector<double> jv = valuesOf(jackA, "Square/h2"), je = errorsOf(jackA, "Square/h2");
  std::vector<double> bv = valuesOf(biasA, "Square/h2");
  bool ok = jv.size() == nBins;
  for (unsigned int k = 0; k < nBins && ok; k++)
    {
    std::vector<double> t(n);
    double tbar = 0.0;
    for (unsigned int i = 0; i < n; i++)
      {
      double sx = 0.0;
      for (unsigned int j = 0; j < n; j++) if (j != i) sx += w[j] * h[j][k];
      t[i] = (sx / (W - w[i])) * (sx / (W - w[i]));
      tbar += t[i] / n;
      }
    double q = 0.0;
    for (unsigned int i = 0; i < n; i++) q += (t[i] - tbar) * (t[i] - tbar);
    double theta = m[k] * m[k];
    ok = ok && close(jv[k], theta) && close(je[k], std::sqrt((n - 1.0) / n * q))
            && close(bv[k], n * theta - (n - 1.0) * tbar);
    }
  CHECK(ok);
  }
  // an unknown stage is reported
  {
  Folder out;
  String problem;
  CHECK(!WorkflowHistograms::runStages(workflow, {String("Nope")}, mean, out, problem));
  CHECK(!problem.isNull() && problem.str().find("Nope") != std::string::npos);
  }

  // --- The task on files
  char directory[] = "/tmp/cap_subsample_XXXXXX";
  if (!::mkdtemp(directory)) { std::printf("FAILED: mkdtemp\n"); return 1; }
  String base(directory);
  String input = FileSystem::join(base, "jobs");
  for (unsigned int i = 0; i < n; i++)
    {
    String path = FileSystem::join(input, String("job_") + String(std::to_string(i)));
    FileSystem::createDirectory(path);
    CapFile file{FileSystem::join(path, "Histos.cap")};
    file.folder() = samples[i];
    Accountant executed("Executed", 1);
    executed.setValueAt(0, long(w[i]));
    file.exportAccountant(executed);
    file.write();
    }
  FileSystem::createDirectory(FileSystem::join(input, "job_9"));     // no file: skipped
  FileSystem::createDirectory(FileSystem::join(input, "other"));     // not matching the pattern
  {
  CapFile file{FileSystem::join(FileSystem::join(input, "other"), "Histos.cap")};
  file.folder() = samples[0];
  file.write();
  }
  auto run = [&](const char * mode, const char * workflowFile, const char * stages, const char * output)
  {
    TestCalculator task;
    task.set("HISTOGRAM:IMPORT:PATH", input);
    task.set("HISTOGRAM:IMPORT:FILE_NAME", "Histos");
    task.set("HISTOGRAM:EXPORT:PATH", base);
    task.set("HISTOGRAM:EXPORT:FILE_NAME", output);
    task.set("MODE", mode);
    task.set("SUBSAMPLE:PATTERN", "job_*");
    task.set("JACKKNIFE:WORKFLOW", workflowFile);
    task.set("JACKKNIFE:STAGES", stages);
    task.configure(task.configuration());
    task.setMinimumReportLevel(Object::kError);
    task.execute();
  };
  auto load = [&](const char * name) -> Folder
  {
    CapFile file{FileSystem::join(base, name)};
    file.load();
    return file.folder();
  };
  try
    {
    run("SUBSAMPLE", "NONE", "NONE", "Average");
    Folder out = load("Average.cap");
    CHECK(out.contains("Accountant_Executed_0") || out.contains("Accountant_Executed_N"));
    std::vector<double> a = valuesOf(out, "Ana/pairs/h"), ae = errorsOf(out, "Ana/pairs/h");
    std::vector<double> b = valuesOf(mean, "Ana/pairs/h"), be = errorsOf(mean, "Ana/pairs/h");
    bool ok = a.size() == nBins;
    for (unsigned int k = 0; k < nBins && ok; k++) ok = close(a[k], b[k]) && close(ae[k], be[k]);
    CHECK(ok);
    CapFile file{FileSystem::join(base, "Average.cap")};
    file.load();
    Accountant total("Executed", 0);
    file.importAccountant(total);
    CHECK(total.size() == 1 && total.valueAt(0) == long(W));

    run("jackknife", "NONE", "NONE", "JackB");
    Folder jb = load("JackB.cap"), bb = load("JackB_biasCorrected.cap");
    a = errorsOf(jb, "Ana/s"); b = errorsOf(jackB, "Ana/s");
    std::vector<double> c = valuesOf(bb, "Ana/s"), d = valuesOf(biasB, "Ana/s");
    ok = a.size() == nBins && c.size() == nBins;
    for (unsigned int k = 0; k < nBins && ok; k++) ok = close(a[k], b[k]) && close(c[k], d[k]);
    CHECK(ok);

    String workflowFile = FileSystem::join(base, "workflow.xml");
    { std::ofstream text(workflowFile.str()); text << "<CAP><TASK name=\"Square\" class=\"TestSquare\" stage=\"Square\"/></CAP>\n"; }
    run("JACKKNIFE", workflowFile.str().c_str(), "Square", "JackA");
    Folder ja = load("JackA.cap"), ba = load("JackA_biasCorrected.cap");
    a = errorsOf(ja, "Square/h2"); b = errorsOf(jackA, "Square/h2");
    c = valuesOf(ba, "Square/h2"); d = valuesOf(biasA, "Square/h2");
    ok = a.size() == nBins && c.size() == nBins;
    for (unsigned int k = 0; k < nBins && ok; k++) ok = close(a[k], b[k]) && close(c[k], d[k]);
    CHECK(ok);
    }
  catch (const std::exception & e)
    {
    nFailures++;
    std::printf("FAILED: exception %s\n", e.what());
    }
  // errors: an unknown mode, fewer than 2 sub-samples
  {
  bool thrown = false;
  try { run("BOOTSTRAP", "NONE", "NONE", "X"); } catch (const std::exception &) { thrown = true; }
  CHECK(thrown);
  thrown = false;
  try
    {
    TestCalculator task;
    task.set("HISTOGRAM:IMPORT:PATH", input);
    task.set("HISTOGRAM:IMPORT:FILE_NAME", "Histos");
    task.set("HISTOGRAM:EXPORT:PATH", base);
    task.set("HISTOGRAM:EXPORT:FILE_NAME", "X");
    task.set("SUBSAMPLE:PATTERN", "job_0");
    task.configure(task.configuration());
    task.setMinimumReportLevel(Object::kError);
    task.execute();
    }
  catch (const std::exception &) { thrown = true; }
  CHECK(thrown);
  }
  FileSystem::removeTree(base);

  std::printf("TestSubSample: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures == 0 ? 0 : 1;
}
