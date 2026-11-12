/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "FileListing.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include "SubSampleStatCalculator.hpp"
#include "FileSystem.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "HistogramFolder.hpp"
#include "NameHelpers.hpp"
#include "Exceptions.hpp"
#include "CapFile.hpp"
#include "ManagedObjects.hpp"
#include "WorkflowHistograms.hpp"
#include "XmlDocument.hpp"

namespace CAP
{
using MATH::Histogram;

namespace
{
//! Values of an empty bin are flagged by Histogram::binValues(): they count as 0 here (a histogram per event).
inline double unflag(double v) { return v < -1.0E99 ? 0.0 : v; }

//! Values, uncertainties and entries of a histogram, empty bins as 0.
void contents(const Histogram & h, std::vector<double> & values, std::vector<double> & errors, std::vector<long> & entries)
{
  values = h.binValues();
  errors = h.binValueUncertainties();
  entries = h.binEntries();
  for (auto & v : values) v = unflag(v);
  for (auto & e : errors) e = unflag(e);
}

//! Set a histogram; a bin with content but no entry is given one (Histogram requires entries for content).
void setContents(Histogram & h, std::vector<long> entries, const std::vector<double> & values,
                 const std::vector<double> & errors)
{
  for (size_t k = 0; k < entries.size(); k++)
    {
    if (entries[k] < 0) entries[k] = 0;
    if (entries[k] == 0 && (values[k] != 0.0 || errors[k] != 0.0)) entries[k] = 1;
    }
  h.setWith(entries, values, errors);
}

//! The histogram at path of a sample, with the binning of the reference. @throws Exception if missing or different
const Histogram & matching(const Folder & sample, const String & path, unsigned int nBins, const char * caller)
{
  const Histogram * h = sample.get<Histogram>(path);
  if (!h) throw Exception(String("histogram ") + path + String(" missing in a sample"), caller);
  if (h->nBinsTotal() != nBins) throw Exception(String("histogram ") + path + String(" has a different binning in a sample"), caller);
  return *h;
}

//!
//! Weighted average of samples with the uncertainty of the mean, accumulated one sample at a time (one pass, the
//! sums shifted by the first sample's values, for precision).
//!
class MeanAccumulator
{
public:
  void add(const Folder & sample, double weight)
  {
    if (!(weight > 0.0)) throw Exception("sub-sample weight must be > 0", "SubSampleStatCalculator");
    if (_n == 0)
      {
      _reference = sample;
      for (const String & path : SubSampleStatCalculator::histogramPaths(sample))
        {
        Sums & s = _sums[path];
        std::vector<double> e;
        contents(*sample.get<Histogram>(path), s.shift, e, s.entries);
        size_t n = s.shift.size();
        s.swd.assign(n, 0.0); s.sw2d.assign(n, 0.0); s.sw2dd.assign(n, 0.0);
        }
      }
    else
      {
      for (auto & item : _sums)
        {
        Sums & s = item.second;
        std::vector<double> v, e;
        std::vector<long> entries;
        contents(matching(sample, item.first, s.shift.size(), "SubSampleStatCalculator"), v, e, entries);
        for (size_t k = 0; k < v.size(); k++)
          {
          double d = v[k] - s.shift[k];
          s.swd[k]   += weight * d;
          s.sw2d[k]  += weight * weight * d;
          s.sw2dd[k] += weight * weight * d * d;
          s.entries[k] += entries[k];
          }
        }
      }
    _sw += weight;
    _sw2 += weight * weight;
    _n++;
  }
  //! The average; with the uncertainty of the mean (N/(N-1) sum w^2 (x-m)^2 / W^2) if errors, 0 otherwise.
  void result(Folder & out, bool errors = true) const
  {
    if (_n == 0) throw Exception("no sample", "SubSampleStatCalculator");
    out = _reference;
    for (const auto & item : _sums)
      {
      const Sums & s = item.second;
      size_t n = s.shift.size();
      std::vector<double> m(n), se(n, 0.0);
      for (size_t k = 0; k < n; k++)
        {
        double delta = s.swd[k] / _sw;
        m[k] = s.shift[k] + delta;
        if (errors && _n > 1)
          {
          double q = s.sw2dd[k] - 2.0 * delta * s.sw2d[k] + delta * delta * _sw2;
          if (q < 0.0) q = 0.0;
          se[k] = std::sqrt(double(_n) / double(_n - 1) * q) / _sw;
          }
        }
      setContents(*out.get<Histogram>(item.first), s.entries, m, se);
      }
  }
  //! The weighted average of the histogram at path, in double precision.
  std::vector<double> mean(const String & path) const
  {
    const Sums & s = _sums.at(path);
    std::vector<double> m(s.shift.size());
    for (size_t k = 0; k < m.size(); k++) m[k] = s.shift[k] + s.swd[k] / _sw;
    return m;
  }
  std::vector<String> paths() const
  {
    std::vector<String> result;
    for (const auto & item : _sums) result.push_back(item.first);
    return result;
  }
  double sumWeights() const { return _sw; }
  unsigned int size() const { return _n; }
private:
  struct Sums { std::vector<double> shift, swd, sw2d, sw2dd; std::vector<long> entries; };
  Folder _reference;
  std::map<String, Sums> _sums;
  double _sw = 0.0, _sw2 = 0.0;
  unsigned int _n = 0;
};

//!
//! Jackknife of results: the value theta of the whole sample and the sums of e_i = theta_i - theta over the replicas.
//!
class JackknifeAccumulator
{
public:
  explicit JackknifeAccumulator(const Folder & whole) : _whole(whole)
  {
    for (const String & path : SubSampleStatCalculator::histogramPaths(whole))
      {
      Sums & s = _sums[path];
      std::vector<double> e;
      contents(*whole.get<Histogram>(path), s.theta, e, s.entries);
      s.se.assign(s.theta.size(), 0.0);
      s.se2.assign(s.theta.size(), 0.0);
      }
  }
  void add(const Folder & replica)
  {
    for (auto & item : _sums)
      {
      Sums & s = item.second;
      std::vector<double> v, e;
      std::vector<long> entries;
      contents(matching(replica, item.first, s.theta.size(), "SubSampleStatCalculator"), v, e, entries);
      for (size_t k = 0; k < v.size(); k++)
        {
        double d = v[k] - s.theta[k];
        s.se[k] += d;
        s.se2[k] += d * d;
        }
      }
    _n++;
  }
  //! Set theta of the histogram at path in double precision (instead of the values stored in whole).
  void setTheta(const String & path, const std::vector<double> & theta) { _sums.at(path).theta = theta; }
  //! Add theta_i - theta of the histogram at path (call countReplica() once per replica).
  void addDifferences(const String & path, const std::vector<double> & d)
  {
    Sums & s = _sums.at(path);
    for (size_t k = 0; k < d.size(); k++) { s.se[k] += d[k]; s.se2[k] += d[k] * d[k]; }
  }
  void countReplica() { _n++; }
  //! theta with sigma^2 = (N-1)/N sum (theta_i - mean)^2, and N theta - (N-1) mean with the same uncertainties.
  void result(Folder & out, Folder & biasCorrected) const
  {
    if (_n < 2) throw Exception("the jackknife needs at least 2 replicas", "SubSampleStatCalculator");
    out = _whole;
    biasCorrected = _whole;
    double n = double(_n);
    for (const auto & item : _sums)
      {
      const Sums & s = item.second;
      size_t nb = s.theta.size();
      std::vector<double> sigma(nb), corrected(nb);
      for (size_t k = 0; k < nb; k++)
        {
        double q = s.se2[k] - s.se[k] * s.se[k] / n;
        if (q < 0.0) q = 0.0;
        sigma[k] = std::sqrt((n - 1.0) / n * q);
        corrected[k] = s.theta[k] - (n - 1.0) * s.se[k] / n;
        }
      setContents(*out.get<Histogram>(item.first), s.entries, s.theta, sigma);
      setContents(*biasCorrected.get<Histogram>(item.first), s.entries, corrected, sigma);
      }
  }
private:
  struct Sums { std::vector<double> theta, se, se2; std::vector<long> entries; };
  Folder _whole;
  std::map<String, Sums> _sums;
  unsigned int _n = 0;
};

//! Words of a list separated by spaces or commas.
std::vector<String> words(const String & text)
{
  std::vector<String> result;
  std::string word;
  for (char c : text.str())
    {
    if (c == ' ' || c == ',' || c == '\t' || c == '\n')
      { if (!word.empty()) result.push_back(String(word)); word.clear(); }
    else word += c;
    }
  if (!word.empty()) result.push_back(String(word));
  return result;
}

String upper(String s) { s.toUpper(); return s; }
} // namespace

SubSampleStatCalculator::SubSampleStatCalculator()
:
Task(),
_totalTaskExecuted(),
_eventAccepted(),
_totalEventAccepted(),
_importPath(),
_importFile(),
_exportPath(),
_exportFile()
{
appendClassName("SubSampleStatCalculator");
setMinimumReportLevel(Object::kInfo);
setName("SubSampleStatCalculator");
}

SubSampleStatCalculator::SubSampleStatCalculator(const SubSampleStatCalculator & src)
:
Task(src),
_totalTaskExecuted(src._totalTaskExecuted),
_eventAccepted(src._eventAccepted),
_totalEventAccepted(src._totalEventAccepted),
_importPath(src._importPath),
_importFile(src._importFile),
_exportPath(src._exportPath),
_exportFile(src._exportFile)
{ }

SubSampleStatCalculator & SubSampleStatCalculator::operator=(const SubSampleStatCalculator & rhs)
{
if (this!=&rhs)
  {
  Task::operator=(rhs);
  _totalTaskExecuted  = rhs._totalTaskExecuted;
  _eventAccepted      = rhs._eventAccepted;
  _totalEventAccepted = rhs._totalEventAccepted;
  _importPath = rhs._importPath;
  _importFile = rhs._importFile;
  _exportPath = rhs._exportPath;
  _exportFile = rhs._exportFile;
  }
return *this;
}

void SubSampleStatCalculator::setDefaultConfiguration()
{
Task::setDefaultConfiguration();
const String & taskName = name();
String objectType = "HISTOGRAM";
_configuration.addProperty(createKey(taskName,objectType,"IMPORT:PATH"),      String("NONE"));
_configuration.addProperty(createKey(taskName,objectType,"IMPORT:FILE_NAME"), String("NONE"));
_configuration.addProperty(createKey(taskName,objectType,"EXPORT:PATH"),      String("NONE"));
_configuration.addProperty(createKey(taskName,objectType,"EXPORT:FILE_NAME"), String("NONE"));
_configuration.addProperty(createKey(taskName,"MODE"),                       String("SUBSAMPLE"));
_configuration.addProperty(createKey(taskName,"SUBSAMPLE:PATTERN"),          String("*"));
_configuration.addProperty(createKey(taskName,"WEIGHTS"),                    String("EVENTS"));
_configuration.addProperty(createKey(taskName,"JACKKNIFE:WORKFLOW"),         String("NONE"));
_configuration.addProperty(createKey(taskName,"JACKKNIFE:STAGES"),           String("NONE"));
_configuration.addProperty(createKey(taskName,"JACKKNIFE:BIAS_FILE_NAME"),   String("NONE"));
}

std::vector<String> SubSampleStatCalculator::histogramPaths(const Folder & folder)
{
std::vector<String> result;
for (const String & path : folder.paths())
  if (folder.get<Histogram>(path)) result.push_back(path);
return result;
}

void SubSampleStatCalculator::average(const std::vector<const Folder *> & samples, const std::vector<double> & weights,
                                      Folder & result)
{
if (samples.size() != weights.size()) throw Exception("as many weights as samples are needed", __FUNCTION__);
MeanAccumulator mean;
for (size_t i = 0; i < samples.size(); i++) mean.add(*samples[i], weights[i]);
mean.result(result);
}

void SubSampleStatCalculator::jackknife(const Folder & whole, const std::vector<const Folder *> & replicas,
                                        Folder & result, Folder & biasCorrected)
{
JackknifeAccumulator jack(whole);
for (const Folder * replica : replicas) jack.add(*replica);
jack.result(result, biasCorrected);
}

void SubSampleStatCalculator::leaveOneOut(const Folder & mean, const Folder & sample, double weight, double sumWeights,
                                          Folder & replica)
{
double rest = sumWeights - weight;
if (!(rest > 0.0)) throw Exception("the other sub-samples have no weight", __FUNCTION__);
replica = mean;
double scale = std::sqrt(sumWeights / rest);
for (const String & path : histogramPaths(mean))
  {
  std::vector<double> m, em, x, ex;
  std::vector<long> nm, nx;
  contents(*mean.get<Histogram>(path), m, em, nm);
  contents(matching(sample, path, m.size(), __FUNCTION__), x, ex, nx);
  for (size_t k = 0; k < m.size(); k++)
    {
    m[k] = (sumWeights * m[k] - weight * x[k]) / rest;
    em[k] *= scale;   // the uncertainty of an average of fewer samples (not used by the jackknife itself)
    nm[k] -= nx[k];
    }
  Histogram & r = *replica.get<Histogram>(path);
  r.setPrecision(Histogram::kDouble);   // the differences between replicas are small: no rounding to float
  setContents(r, nm, m, em);
  }
}

std::vector<SubSampleStatCalculator::SubSample> SubSampleStatCalculator::findSubSamples()
{
const String & taskName = name();
String pattern = _configuration.valueString(createKey(taskName,"SUBSAMPLE:PATTERN"));
if (pattern.isNull() || pattern == "NONE") pattern = "*";
String file = ManagedObjects<HistogramFolder>::capFileName(_importFile);
std::vector<String> directories = listDirsIn(_importPath, false);
std::sort(directories.begin(), directories.end());
std::vector<SubSample> result;
for (const String & directory : directories)
  {
  if (!FileSystem::matches(directory, pattern)) continue;
  String path = FileSystem::join(_importPath, directory);
  if (!FileSystem::isFile(FileSystem::expand(FileSystem::join(path, file, ".cap"))))
    {
    if (reportWarning(__FUNCTION__))
      std::cout << "No file " << file << " in " << path << ": directory skipped" << std::endl;
    continue;
    }
  SubSample s;
  s.path = path;
  s.file = file;
  result.push_back(s);
  }
return result;
}

void SubSampleStatCalculator::execute()
{
const String & taskName = name();
String objectType = "HISTOGRAM";
_importPath  = _configuration.valueString(createKey(taskName,objectType,"IMPORT:PATH"));
_importFile  = _configuration.valueString(createKey(taskName,objectType,"IMPORT:FILE_NAME"));
_exportPath  = _configuration.valueString(createKey(taskName,objectType,"EXPORT:PATH"));
_exportFile  = _configuration.valueString(createKey(taskName,objectType,"EXPORT:FILE_NAME"));
String mode     = upper(_configuration.valueString(createKey(taskName,"MODE")));
String weighting = upper(_configuration.valueString(createKey(taskName,"WEIGHTS")));
String workflowFile = _configuration.valueString(createKey(taskName,"JACKKNIFE:WORKFLOW"));
String stageList    = _configuration.valueString(createKey(taskName,"JACKKNIFE:STAGES"));
String biasFile     = _configuration.valueString(createKey(taskName,"JACKKNIFE:BIAS_FILE_NAME"));
if (mode != "SUBSAMPLE" && mode != "JACKKNIFE")
  throw Exception(String("unknown MODE ") + mode + String(" (SUBSAMPLE or JACKKNIFE)"), __FUNCTION__);
if (weighting != "EVENTS" && weighting != "EQUAL")
  throw Exception(String("unknown WEIGHTS ") + weighting + String(" (EVENTS or EQUAL)"), __FUNCTION__);
bool withStages = mode == "JACKKNIFE" && !workflowFile.isNull() && workflowFile != "NONE";
std::vector<String> stages;
XmlDocument workflow;
if (withStages)
  {
  stages = words(stageList == "NONE" ? String("") : stageList);
  if (stages.empty()) throw Exception("JACKKNIFE:WORKFLOW is given without JACKKNIFE:STAGES", __FUNCTION__);
  workflow = XmlDocument::read(FileSystem::expand(workflowFile));
  }

std::vector<SubSample> samples = findSubSamples();
if (samples.size() < 2)
  throw Exception(String("fewer than 2 sub-samples in ") + _importPath, __FUNCTION__);
if (reportInfo(__FUNCTION__))
  {
  printCR();
  printValue("Mode", mode);
  printValue("Sub-samples", int(samples.size()));
  if (withStages) printValue("Stages", stageList);
  }

// Pass 1: weights, accountants and the weighted average.
_totalTaskExecuted.initialize("Executed",0);
_totalEventAccepted.clear();
bool haveAccountants = true;
MeanAccumulator mean;
for (size_t i = 0; i < samples.size(); i++)
  {
  SubSample & s = samples[i];
  std::unique_ptr<CapFile> file(CapFile::openOldCapFile(s.path, s.file));
  Accountant executed("Executed",0);
  try
    {
    file->importAccountant(executed);
    if (_totalTaskExecuted.size() == 0) _totalTaskExecuted = executed;
    else _totalTaskExecuted.increment(executed);
    _eventAccepted.initialize("EventAccepted",0);
    try
      {
      file->importAccountant(_eventAccepted);
      if (_totalEventAccepted.size() == 0) _totalEventAccepted = _eventAccepted;
      else _totalEventAccepted.increment(_eventAccepted);
      }
    catch (const std::exception &) { }
    }
  catch (const std::exception &)
    {
    if (i > 0 && haveAccountants)
      throw Exception(String("no accountant Executed in ") + s.path + String(" (the other files have one)"), __FUNCTION__);
    haveAccountants = false;
    }
  if (weighting == "EVENTS" && haveAccountants)
    {
    s.weight = double(executed.valueAt(0));
    if (!(s.weight > 0.0)) throw Exception(String("no executed task in ") + s.path, __FUNCTION__);
    }
  else s.weight = 1.0;
  if (i == 0 && weighting == "EVENTS" && !haveAccountants && reportWarning(__FUNCTION__))
    std::cout << "The sub-sample files have no accountant Executed: equal weights" << std::endl;
  mean.add(file->folder(), s.weight);
  }

Folder result, biasCorrected;
if (mode == "SUBSAMPLE")
  {
  mean.result(result);
  }
else
  {
  // Pass 2: the replicas without each sub-sample, and their results.
  Folder average;
  mean.result(average);
  double sumWeights = mean.sumWeights();
  Folder whole;
  String error;
  if (withStages)
    {
    if (!WorkflowHistograms::runStages(workflow, stages, average, whole, error))
      throw Exception(String("stages on the whole sample: ") + error, __FUNCTION__);
    }
  else
    {
    whole = average;
    }
  JackknifeAccumulator jack(whole);
  std::map<String, std::vector<double>> means;
  if (!withStages)
    for (const String & path : mean.paths())
      {
      means[path] = mean.mean(path);
      jack.setTheta(path, means[path]);
      }
  for (size_t i = 0; i < samples.size(); i++)
    {
    const SubSample & s = samples[i];
    std::unique_ptr<CapFile> file(CapFile::openOldCapFile(s.path, s.file));
    if (withStages)
      {
      Folder replica, derived;
      leaveOneOut(average, file->folder(), s.weight, sumWeights, replica);
      if (!WorkflowHistograms::runStages(workflow, stages, replica, derived, error))
        throw Exception(String("stages on the replica without ") + s.path + String(": ") + error, __FUNCTION__);
      jack.add(derived);
      }
    else
      {
      // theta_i - theta = w_i (m - x_i)/(W - w_i), in double precision
      double rest = sumWeights - s.weight;
      if (!(rest > 0.0)) throw Exception("the other sub-samples have no weight", __FUNCTION__);
      for (const auto & item : means)
        {
        std::vector<double> x, e;
        std::vector<long> entries;
        contents(matching(file->folder(), item.first, item.second.size(), __FUNCTION__), x, e, entries);
        for (size_t k = 0; k < x.size(); k++) x[k] = s.weight * (item.second[k] - x[k]) / rest;
        jack.addDifferences(item.first, x);
        }
      jack.countReplica();
      }
    if (reportInfo(__FUNCTION__)) std::cout << "Replica " << i+1 << "/" << samples.size() << " done" << std::endl;
    }
  jack.result(result, biasCorrected);
  }

String outputName = ManagedObjects<HistogramFolder>::capFileName(_exportFile);
std::unique_ptr<CapFile> outputFile(CapFile::openRecreateCapFile(_exportPath, outputName));
outputFile->folder() = result;
if (haveAccountants)
  {
  outputFile->exportAccountant(_totalTaskExecuted);
  if (_totalEventAccepted.size() > 0) outputFile->exportAccountant(_totalEventAccepted);
  }
outputFile->write(true);
if (mode == "JACKKNIFE")
  {
  String name = biasFile;
  if (name.isNull() || name == "NONE")
    {
    String stem = outputName;
    if (stem.endsWith(".cap")) stem = String(stem.data(), stem.length() - 4);
    name = stem + String("_biasCorrected");
    }
  std::unique_ptr<CapFile> biasOutput(CapFile::openRecreateCapFile(_exportPath, ManagedObjects<HistogramFolder>::capFileName(name)));
  biasOutput->folder() = biasCorrected;
  biasOutput->write(true);
  }
}

} // namespace CAP
