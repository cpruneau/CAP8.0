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
#include <cstdio>
#include <string>
#include <map>
#include <memory>
#include "SubSampleStatCalculator.hpp"
#include "FileSystem.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "StringParameter.hpp"
#include "LongParameter.hpp"
#include "HistogramFolder.hpp"
#include "NameHelpers.hpp"
#include "Exceptions.hpp"
#include "DatumFile.hpp"
#include "ManagedObjects.hpp"
#include "WorkflowHistograms.hpp"
#include "XmlDocument.hpp"

namespace Datum
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
_configuration.addProperty(createKey(taskName,"PTHAT:BINS"),                 0);
_configuration.addProperty(createKey(taskName,"PTHAT:NORMALIZATION"),        String("SIGMA"));
}

std::vector<String> SubSampleStatCalculator::histogramPaths(const Folder & folder)
{
std::vector<String> result;
for (const String & path : folder.paths())
  if (folder.get<Histogram>(path)) result.push_back(path);
return result;
}

void SubSampleStatCalculator::weightedSum(const std::vector<const Folder *> & bins, const std::vector<double> & weights,
                                          double divisor, Folder & result)
{
if (bins.empty() || bins.size() != weights.size()) throw Exception("as many weights as bins are needed", __FUNCTION__);
if (!(divisor > 0.0)) throw Exception("the divisor must be > 0", __FUNCTION__);
result = *bins[0];
for (const String & path : histogramPaths(*bins[0]))
  {
  Histogram & out = *result.get<Histogram>(path);
  const unsigned int n = out.nBinsTotal();
  if (MATH::Profile * profile = dynamic_cast<MATH::Profile*>(&out))
    {
    // an average: its sums of weights and weighted values, each bin weighted by w_k
    std::vector<long> entries(n, 0);
    std::vector<double> sw(n, 0.0), sw2(n, 0.0), swy(n, 0.0), swy2(n, 0.0);
    for (size_t i = 0; i < bins.size(); i++)
      {
      const MATH::Profile * p = dynamic_cast<const MATH::Profile*>(&matching(*bins[i], path, n, __FUNCTION__));
      if (!p) throw Exception(String("profile ") + path + String(" is not a profile in a bin"), __FUNCTION__);
      const double w = weights[i];
      const std::vector<long> e = p->binEntries();
      for (unsigned int k = 0; k < n; k++)
        {
        entries[k] += e[k];
        sw[k]   += w * p->sumWeights(k);
        sw2[k]  += w * w * p->sumWeights2(k);
        swy[k]  += w * p->sumWeightedValues(k);
        swy2[k] += w * p->sumWeightedValues2(k);
        }
      }
    for (unsigned int k = 0; k < n; k++) profile->setSums(k, entries[k], sw[k], sw2[k], swy[k], swy2[k]);
    continue;
    }
  std::vector<double> values(n, 0.0), errors2(n, 0.0);
  std::vector<long> entries(n, 0);
  for (size_t i = 0; i < bins.size(); i++)
    {
    std::vector<double> v, e;
    std::vector<long> ne;
    contents(matching(*bins[i], path, n, __FUNCTION__), v, e, ne);
    const double w = weights[i];
    for (unsigned int k = 0; k < n; k++)
      {
      values[k] += w * v[k];
      errors2[k] += w * w * e[k] * e[k];
      entries[k] += ne[k];
      }
    }
  std::vector<double> errors(n);
  for (unsigned int k = 0; k < n; k++) { values[k] /= divisor; errors[k] = std::sqrt(errors2[k]) / divisor; }
  out.setPrecision(Histogram::kDouble);      // cross sections span many orders of magnitude
  setContents(out, entries, values, errors);
  }
}

bool SubSampleStatCalculator::crossSection(const Folder & folder, double & sigma, double & error, long & nAccepted,
                                           double & ptHatMin, double & ptHatMax)
{
// the parameters are named DATUM_..., or CAP_... in the files written before Datum 1.0
for (const char * names : {"DATUM_", "CAP_"})
  {
  const String key = String(names) + String("SIGMA_GEN");
  for (const String & path : folder.paths())
    {
    if (!path.endsWith(key) || !folder.get<StringParameter>(path)) continue;
    const String prefix = String(path.data(), path.length() - key.length()) + String(names);
    auto number = [&](const char * name, double fallback)
      {
      const StringParameter * p = folder.get<StringParameter>(prefix + String(name));
      if (!p) return fallback;
      try { return std::stod(p->value().str()); } catch (const std::exception &) { return fallback; }
      };
    sigma = number("SIGMA_GEN", -1.0);
    error = number("SIGMA_ERR", 0.0);
    ptHatMin = number("PTHAT_MIN", -1.0);
    ptHatMax = number("PTHAT_MAX", -1.0);
    const LongParameter * n = folder.get<LongParameter>(prefix + String("N_ACCEPTED"));
    nAccepted = n ? n->value() : 0;
    return sigma >= 0.0;
    }
  }
return false;
}

std::vector<SubSampleStatCalculator::SubSample> SubSampleStatCalculator::combinePtHatBins(unsigned int nBins,
                                                                                          const String & normalization)
{
// the directories <sub-sample>_pthat<k>, by sub-sample
std::map<String, std::vector<SubSample>> groups;
for (const SubSample & s : findSubSamples())
  {
  const std::string name = FileSystem::name(s.path).str();
  const size_t at = name.rfind("_pthat");
  if (at == std::string::npos) continue;                     // e.g. the <sub-sample>_CrossSection of an earlier run
  unsigned int k = 0;
  try { k = (unsigned int) std::stoul(name.substr(at + 6)); } catch (const std::exception &) { continue; }
  if (k >= nBins)
    throw Exception(String("pT-hat bin ") + String(name) + String(" beyond PTHAT:BINS"), __FUNCTION__);
  std::vector<SubSample> & group = groups[String(name.substr(0, at))];
  group.resize(nBins);
  group[k] = s;
  }
if (groups.empty()) throw Exception(String("no pT-hat bin (<sub-sample>_pthat<k>) in ") + _importPath, __FUNCTION__);
for (const auto & item : groups)
  for (unsigned int k = 0; k < nBins; k++)
    if (item.second[k].path.isEmpty())
      throw Exception(String("sub-sample ") + item.first + String(": pT-hat bin ") + String(std::to_string(k))
                      + String(" missing"), __FUNCTION__);
// the cross section of each bin, averaged over the sub-samples (weighted by their events)
std::vector<double> sumSigmaN(nBins, 0.0), sumN(nBins, 0.0), sumError2N2(nBins, 0.0), low(nBins, -1.0), high(nBins, -1.0);
std::map<String, std::vector<double>> events;
for (const auto & item : groups)
  {
  std::vector<double> & n = events[item.first];
  n.assign(nBins, 0.0);
  for (unsigned int k = 0; k < nBins; k++)
    {
    const SubSample & s = item.second[k];
    std::unique_ptr<DatumFile> file(DatumFile::openOldDatumFile(s.path, s.file));
    double sigma = -1.0, error = 0.0, ptMin = -1.0, ptMax = -1.0;
    long accepted = 0;
    if (!crossSection(file->folder(), sigma, error, accepted, ptMin, ptMax))
      throw Exception(String("no cross section (DATUM_SIGMA_GEN, PythiaEventGenerator) in ") + s.path, __FUNCTION__);
    if (accepted <= 0) throw Exception(String("no event accepted in ") + s.path, __FUNCTION__);
    n[k] = double(accepted);
    sumSigmaN[k] += sigma * n[k];
    sumError2N2[k] += error * error * n[k] * n[k];
    sumN[k] += n[k];
    low[k] = ptMin;
    high[k] = ptMax;
    }
  }
std::vector<double> sigma(nBins), sigmaError(nBins);
double covered = 0.0;
for (unsigned int k = 0; k < nBins; k++)
  {
  sigma[k] = sumSigmaN[k] / sumN[k];
  sigmaError[k] = std::sqrt(sumError2N2[k]) / sumN[k];
  covered += sigma[k];
  }
const bool perEvent = normalization == "COVERED";
if (!perEvent && normalization != "SIGMA")
  throw Exception(String("unknown PTHAT:NORMALIZATION ") + normalization + String(" (SIGMA or COVERED)"), __FUNCTION__);
if (reportInfo(__FUNCTION__))
  {
  printCR();
  printValue("pT-hat bins", int(nBins));
  for (unsigned int k = 0; k < nBins; k++)
    {
    char text[160];
    std::snprintf(text, sizeof(text), "[%g, %g) GeV/c: sigma = %.6g +- %.2g mb", low[k], high[k], sigma[k], sigmaError[k]);
    printValue((String("pT-hat bin ") + String(std::to_string(k))).data(), String(text));
    }
  printValue("Cross section covered (mb)", covered);
  printValue("Normalization", perEvent ? String("per event of the range covered") : String("cross section (mb)"));
  }
// the sum of the bins of each sub-sample, written to <sub-sample>_CrossSection
std::vector<SubSample> result;
for (const auto & item : groups)
  {
  std::vector<std::unique_ptr<DatumFile>> files;
  std::vector<const Folder *> folders;
  std::vector<double> weights;
  for (unsigned int k = 0; k < nBins; k++)
    {
    files.emplace_back(DatumFile::openOldDatumFile(item.second[k].path, item.second[k].file));
    folders.push_back(&files.back()->folder());
    weights.push_back(sigma[k] / events.at(item.first)[k]);
    }
  Folder sum;
  weightedSum(folders, weights, perEvent ? covered : 1.0, sum);
  // the cross section of the bins replaces that of the run of bin 0 (copied with its folder)
  for (const String & path : sum.paths())
    {
    String leaf = FileSystem::name(path);
    if (leaf.beginsWith("DATUM_")) leaf = leaf.subString(6);
    else if (leaf.beginsWith("CAP_")) leaf = leaf.subString(4);           // written before Datum 1.0
    else continue;
    if (leaf == "SIGMA_GEN" || leaf == "SIGMA_ERR" || leaf == "N_ACCEPTED" || leaf == "PTHAT_MIN" || leaf == "PTHAT_MAX")
      sum.remove(path);
    }
  for (unsigned int k = 0; k < nBins; k++)
    {
    char text[200];
    std::snprintf(text, sizeof(text), "%.17g %.17g %.17g %.17g %.17g", low[k], high[k], sigma[k], sigmaError[k],
                  events.at(item.first)[k]);
    String key = String("DATUM_PTHAT_BIN_") + String(std::to_string(k));
    sum.add(new StringParameter(key, String(text)), key);
    }
  char text[64];
  std::snprintf(text, sizeof(text), "%.17g", covered);
  sum.add(new StringParameter("DATUM_SIGMA_COVERED", String(text)), "DATUM_SIGMA_COVERED");
  sum.add(new StringParameter("DATUM_NORMALIZATION", perEvent ? String("COVERED") : String("SIGMA")), "DATUM_NORMALIZATION");
  SubSample combined;
  combined.path = FileSystem::join(_importPath, item.first + String("_CrossSection"));
  combined.file = item.second[0].file;
  FileSystem::createDirectory(FileSystem::expand(combined.path));
  std::unique_ptr<DatumFile> output(DatumFile::openRecreateDatumFile(combined.path, combined.file));
  output->folder() = sum;
  output->write(true);
  result.push_back(combined);
  }
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
String file = ManagedObjects<HistogramFolder>::datumFileName(_importFile);
std::vector<String> directories = listDirsIn(_importPath, false);
std::sort(directories.begin(), directories.end());
std::vector<SubSample> result;
for (const String & directory : directories)
  {
  if (!FileSystem::matches(directory, pattern)) continue;
  String path = FileSystem::join(_importPath, directory);
  if (!FileSystem::isFile(FileSystem::expand(DatumFile::existingFileName(path, file))))
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
const int nPtHatBins = _configuration.valueInt(createKey(taskName,"PTHAT:BINS"),true,0);
const String normalization = upper(_configuration.valueString(createKey(taskName,"PTHAT:NORMALIZATION"),true,"SIGMA"));
if (nPtHatBins > 0) weighting = "EQUAL";          // the sub-samples: the same events in each pT-hat bin
std::vector<String> stages;
XmlDocument workflow;
if (withStages)
  {
  stages = words(stageList == "NONE" ? String("") : stageList);
  if (stages.empty()) throw Exception("JACKKNIFE:WORKFLOW is given without JACKKNIFE:STAGES", __FUNCTION__);
  workflow = XmlDocument::read(FileSystem::expand(workflowFile));
  }

std::vector<SubSample> samples = nPtHatBins > 0 ? combinePtHatBins((unsigned int) nPtHatBins, normalization)
                                                 : findSubSamples();
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
  std::unique_ptr<DatumFile> file(DatumFile::openOldDatumFile(s.path, s.file));
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
    std::unique_ptr<DatumFile> file(DatumFile::openOldDatumFile(s.path, s.file));
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

String outputName = ManagedObjects<HistogramFolder>::datumFileName(_exportFile);
std::unique_ptr<DatumFile> outputFile(DatumFile::openRecreateDatumFile(_exportPath, outputName));
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
    if (DatumFile::isDatumFileName(stem)) stem = FileSystem::withExtension(stem, "");
    name = stem + String("_biasCorrected");
    }
  std::unique_ptr<DatumFile> biasOutput(DatumFile::openRecreateDatumFile(_exportPath, ManagedObjects<HistogramFolder>::datumFileName(name)));
  biasOutput->folder() = biasCorrected;
  biasOutput->write(true);
  }
}

} // namespace Datum
