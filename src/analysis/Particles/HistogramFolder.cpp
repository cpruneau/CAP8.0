/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "HistogramFolder.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Task.hpp"
#include <cmath>


namespace CAP
{

  namespace
  {
  // Bin values and uncertainties are flagged with this value for empty bins (see Histogram::binValues()).
  double unflag(double v) { return v<-1.0E99 ? 0.0 : v; }
  }

  //////////////////////////////////////////////////////////////////////////
  // Construction
  //////////////////////////////////////////////////////////////////////////

  HistogramFolder::HistogramFolder(const String & name)
  :
  Folder(name),
  _histogramBaseName(),
  _parentTask(nullptr)
  {
  setClassName("HistogramFolder");
  }

  HistogramFolder::HistogramFolder(const HistogramFolder & source)
  :
  Folder(source),
  _histogramBaseName(source._histogramBaseName),
  _parentTask(source._parentTask)
  { }

  HistogramFolder & HistogramFolder::operator=(const HistogramFolder & rhs)
  {
  if (this!=&rhs)
    {
    Folder::operator=(rhs);
    _histogramBaseName = rhs._histogramBaseName;
    _parentTask        = rhs._parentTask;
    }
  return *this;
  }

  //////////////////////////////////////////////////////////////////////////
  // Task interface
  //////////////////////////////////////////////////////////////////////////

  void HistogramFolder::configure(const String & taskName, const String & objectType,
                                 const Configuration & configuration, unsigned int index)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  // The histograms of the analyzers (HISTOGRAM types) have short names (e.g. n1_pt): the folders of the files of the
  // job (task, combination of filters) tell them apart. Corrections (efficiencies, smearers), imported from files of
  // their own, keep the base name of their configuration (BASE_NAME).
  if (!objectType.beginsWith("HISTOGRAM"))
    _histogramBaseName = configuration.valueString(createKey(taskName,objectType,index,"BASE_NAME"));
  }

  void HistogramFolder::create()
  {
  throw Exception("HistogramFolder::create() must be implemented by the derived class",__FUNCTION__);
  }

  void HistogramFolder::loadFrom(const Folder & file)
  {
  throw FileException(file.path(),"HistogramFolder::loadFrom() must be implemented by the derived class",__FUNCTION__);
  }

  void HistogramFolder::saveTo(Folder & file) const
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  for (const Histogram * h : histograms())
    {
    if (file.contains(h->name())) file.remove(h->name());
    if (const Profile * prof = dynamic_cast<const Profile *>(h)) file.add(new Profile(*prof),h->name());
    else                                                        file.add(new Histogram(*h),h->name());
    }
  }

  void HistogramFolder::reset()
  {
  for (Histogram * h : histograms()) h->reset();
  }

  void HistogramFolder::print() const
  {
  Object::print();
  printValue("Histogram base name",_histogramBaseName);
  printValue("Number of histograms",nHistograms());
  for (const Histogram * h : histograms())
    printValue(h->name().data(),String(String("rank ")+String(std::to_string(h->rank()).c_str())+", "
                                       +String(std::to_string(h->nBinsTotal()).c_str())+" bins"));
  }

  const String HistogramFolder::parentName() const
  {
  if (!_parentTask) throw Exception("Group has no parent task",__FUNCTION__);
  return _parentTask->name();
  }

  const String HistogramFolder::parentPathName() const
  {
  if (!_parentTask) throw Exception("Group has no parent task",__FUNCTION__);
  return _parentTask->fullTaskPath();
  }

  //////////////////////////////////////////////////////////////////////////
  // Histograms
  //////////////////////////////////////////////////////////////////////////

  unsigned int HistogramFolder::nHistograms() const
  {
  return histograms().size();
  }

  std::vector<HistogramFolder::Histogram *> HistogramFolder::histograms() const
  {
  std::vector<Histogram *> list;
  for (const String & entry : names())
    if (Histogram * h = get<Histogram>(entry)) list.push_back(h);
  return list;
  }


  HistogramFolder::Histogram * HistogramFolder::append(Histogram * h)
  {
  if (!h) throw Exception("Given histogram pointer is null",__FUNCTION__);
  // Store a profile with its own type, so that copies and files keep it a Profile.
  if (Profile * p = dynamic_cast<Profile *>(h)) return add(p,p->name());
  return add(h,h->name());   // the folder takes ownership; it deletes h if the name is rejected
  }

  //////////////////////////////////////////////////////////////////////////
  // Creating histograms
  //////////////////////////////////////////////////////////////////////////

  HistogramFolder::Histogram * HistogramFolder::createHistogram(const String & name,
                                                              unsigned int n_x, double min_x, double max_x,
                                                              const String & title_x, const String & title_y,
                                                              int storageOption)
  {
  MATH::Partition partition;
  partition.addDimension(n_x,min_x,max_x,title_x);
  Histogram * h = new Histogram(partition);
  h->setPrecision(Histogram::precisionFromStorageOption(storageOption));
  h->setName(name);
  h->setTitle(title_y);
  return append(h);
  }

  HistogramFolder::Histogram * HistogramFolder::createHistogram(const String & name,
                                                              unsigned int n_x, double min_x, double max_x,
                                                              unsigned int n_y, double min_y, double max_y,
                                                              const String & title_x, const String & title_y,
                                                              const String & title_z, int storageOption)
  {
  MATH::Partition partition;
  partition.addDimension(n_x,min_x,max_x,title_x);
  partition.addDimension(n_y,min_y,max_y,title_y);
  Histogram * h = new Histogram(partition);
  h->setPrecision(Histogram::precisionFromStorageOption(storageOption));
  h->setName(name);
  h->setTitle(title_z);
  return append(h);
  }

  HistogramFolder::Histogram * HistogramFolder::createHistogram(const String & name,
                                                              unsigned int n_x, double min_x, double max_x,
                                                              unsigned int n_y, double min_y, double max_y,
                                                              unsigned int n_z, double min_z, double max_z,
                                                              const String & title_x, const String & title_y,
                                                              const String & title_z, const String & title_w,
                                                              int storageOption)
  {
  MATH::Partition partition;
  partition.addDimension(n_x,min_x,max_x,title_x);
  partition.addDimension(n_y,min_y,max_y,title_y);
  partition.addDimension(n_z,min_z,max_z,title_z);
  Histogram * h = new Histogram(partition);
  h->setPrecision(Histogram::precisionFromStorageOption(storageOption));
  h->setName(name);
  h->setTitle(title_w);
  return append(h);
  }

  HistogramFolder::Profile * HistogramFolder::createProfile(const String & name,
                                                          unsigned int n_x, double min_x, double max_x,
                                                          const String & title_x, const String & title_y)
  {
  MATH::Partition partition;
  partition.addDimension(n_x,min_x,max_x,title_x);
  Profile * p = new Profile(partition);
  p->setName(name);
  p->setTitle(title_y);
  append(p);
  return p;
  }

  HistogramFolder::Profile * HistogramFolder::createProfile(const String & name,
                                                          unsigned int n_x, double min_x, double max_x,
                                                          unsigned int n_y, double min_y, double max_y,
                                                          const String & title_x, const String & title_y,
                                                          const String & title_z)
  {
  MATH::Partition partition;
  partition.addDimension(n_x,min_x,max_x,title_x);
  partition.addDimension(n_y,min_y,max_y,title_y);
  Profile * p = new Profile(partition);
  p->setName(name);
  p->setTitle(title_z);
  append(p);
  return p;
  }

  //////////////////////////////////////////////////////////////////////////
  // Files
  //////////////////////////////////////////////////////////////////////////

  HistogramFolder::Histogram * HistogramFolder::importHistogram(const Folder & file, const String & name)
  {
  const Histogram * source = file.get<Histogram>(name);
  if (!source) throw FileException(file.path(),String("No histogram named ")+name+" in the file",__FUNCTION__);
  std::unique_ptr<Histogram> copy(source->clone());
  if (contains(name)) remove(name);
  return append(copy.release());
  }

  HistogramFolder::Profile * HistogramFolder::importProfile(const Folder & file, const String & name)
  {
  const Profile * source = file.get<Profile>(name);
  if (!source) throw FileException(file.path(),String("No profile named ")+name+" in the file",__FUNCTION__);
  std::unique_ptr<Profile> copy(new Profile(*source));
  if (contains(name)) remove(name);
  Profile * p = copy.get();
  append(copy.release());
  return p;
  }

  unsigned int HistogramFolder::importAllHistograms(const Folder & file)
  {
  unsigned int n = 0;
  for (const String & entry : file.names())
    {
    const Histogram * source = file.get<Histogram>(entry);
    if (!source) continue;
    if (contains(entry)) remove(entry);
    Histogram * copy = source->clone();
    copy->setName(entry);
    append(copy);
    n++;
    }
  return n;
  }

  //////////////////////////////////////////////////////////////////////////
  // Operations on all histograms
  //////////////////////////////////////////////////////////////////////////

  void HistogramFolder::scaleHistograms(double a)
  {
  for (Histogram * h : histograms()) h->scale(a);
  }

  void HistogramFolder::scaleAllHistoByBinWidth(double scale)
  {
  for (Histogram * h : histograms())
    {
    double volume = 1.0;
    for (unsigned int k=0; k<h->rank(); k++) volume *= (h->maximum(k)-h->minimum(k))/h->nBins(k);
    h->scale(scale/volume);
    }
  }

  const HistogramFolder::Histogram & HistogramFolder::counterpart(const HistogramFolder & other, const Histogram & h, const char * source)
  {
  const Histogram * o = other.histogram(h.name());
  if (!o) throw Exception(String("Group ")+other.name()+" has no histogram named "+h.name(),source);
  if (!h.compatibleWith(*o)) throw Exception(String("Histograms named ")+h.name()+" have different binnings",source);
  return *o;
  }

  void HistogramFolder::requireSameHistograms(const HistogramFolder & other, const char * source) const
  {
  for (const Histogram * h : histograms()) counterpart(other,*h,source);
  }

  bool HistogramFolder::sameSizeAs(const HistogramFolder & rhs) const
  {
  try { requireSameHistograms(rhs,__FUNCTION__); }
  catch (Exception &) { return false; }
  return true;
  }

  HistogramFolder & HistogramFolder::operator+=(const HistogramFolder & rhs)
  {
  requireSameHistograms(rhs,__FUNCTION__);   // check everything first: this group is unchanged on failure
  for (Histogram * h : histograms()) h->add(counterpart(rhs,*h,__FUNCTION__),1.0);
  return *this;
  }

  HistogramFolder & HistogramFolder::operator-=(const HistogramFolder & rhs)
  {
  requireSameHistograms(rhs,__FUNCTION__);
  for (Histogram * h : histograms()) h->add(counterpart(rhs,*h,__FUNCTION__),-1.0);
  return *this;
  }

  namespace
  {
  void sumGroups(const std::vector<const HistogramFolder *> & groups, const std::vector<double> & coefficients,
                 HistogramFolder & sum, const char * source)
  {
  for (const HistogramFolder * g : groups)
    for (const MATH::Histogram * h : sum.histograms())
      {
      const MATH::Histogram * o = g->histogram(h->name());
      if (!o) throw Exception(String("Group ")+g->name()+" has no histogram named "+h->name(),source);
      if (!h->compatibleWith(*o)) throw Exception(String("Histograms named ")+h->name()+" have different binnings",source);
      }
  for (MATH::Histogram * h : sum.histograms())
    {
    std::vector<const MATH::Histogram *> terms;
    for (const HistogramFolder * g : groups) terms.push_back(g->histogram(h->name()));
    h->setAsSumOf(terms,coefficients);
    }
  }
  }

  void HistogramFolder::sumHistogramGroups(double a1, const HistogramFolder & g1, HistogramFolder & sum)
  {
  sumGroups({&g1},{a1},sum,__FUNCTION__);
  }

  void HistogramFolder::sumHistogramGroups(double a1, const HistogramFolder & g1, double a2, const HistogramFolder & g2,
                                          HistogramFolder & sum)
  {
  sumGroups({&g1,&g2},{a1,a2},sum,__FUNCTION__);
  }

  void HistogramFolder::sumHistogramGroups(double a1, const HistogramFolder & g1, double a2, const HistogramFolder & g2,
                                          double a3, const HistogramFolder & g3, HistogramFolder & sum)
  {
  sumGroups({&g1,&g2,&g3},{a1,a2,a3},sum,__FUNCTION__);
  }

  void HistogramFolder::sumHistogramGroups(double a1, const HistogramFolder & g1, double a2, const HistogramFolder & g2,
                                          double a3, const HistogramFolder & g3, double a4, const HistogramFolder & g4,
                                          HistogramFolder & sum)
  {
  sumGroups({&g1,&g2,&g3,&g4},{a1,a2,a3,a4},sum,__FUNCTION__);
  }

  void HistogramFolder::divideHistogramsLists(double a1, const HistogramFolder & numerator,
                                             double a2, const HistogramFolder & denominator, HistogramFolder & ratio)
  {
  ratio.requireSameHistograms(numerator,__FUNCTION__);
  ratio.requireSameHistograms(denominator,__FUNCTION__);
  for (Histogram * h : ratio.histograms())
    h->setAsRatioOf(*numerator.histogram(h->name()),*denominator.histogram(h->name()),a1,a2);
  }

  void HistogramFolder::squareDifferenceLists(const HistogramFolder & group, double sumWeights, double weight, int n)
  {
  requireSameHistograms(group,__FUNCTION__);
  double sum   = weight + sumWeights;
  if (sum<=0) throw Exception("weight + sumWeights must be positive",__FUNCTION__);
  double rwn   = weight/sum;
  double rw    = sumWeights/sum;
  double sqrtN = std::sqrt(double(std::abs(n)));
  for (Histogram * h0 : histograms())
    {
    const Histogram & h1 = *group.histogram(h0->name());
    std::vector<double> v0 = h0->binValues();
    std::vector<double> e0 = h0->binValueUncertainties();
    std::vector<double> v1 = h1.binValues();
    std::vector<long> entries(h0->binEntries());
    const std::vector<long> & entries1 = h1.binEntries();
    std::vector<double> values(v0.size()), errors(v0.size());
    for (std::size_t k=0; k<v0.size(); k++)
      {
      double vAvg    = unflag(v0[k]);
      double evAvg   = unflag(e0[k]);
      double v       = unflag(v1[k]);
      double dv      = v - vAvg;
      double evSqAvg = rw*(evAvg*evAvg + dv*dv*rwn);
      values[k] = vAvg + rwn*dv;
      errors[k] = n>0 ? std::sqrt(evSqAvg)/sqrtN : std::sqrt(evSqAvg);
      entries[k] += entries1[k];
      if (entries[k]==0 && (values[k]!=0 || errors[k]!=0)) entries[k] = 1;
      }
    h0->setWith(entries,values,errors);
    }
  }

  HistogramFolder operator+(const HistogramFolder & lhs, const HistogramFolder & rhs) { HistogramFolder r(lhs); r += rhs; return r; }
  HistogramFolder operator-(const HistogramFolder & lhs, const HistogramFolder & rhs) { HistogramFolder r(lhs); r -= rhs; return r; }

} // namespace CAP
