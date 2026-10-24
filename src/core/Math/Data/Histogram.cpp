#include <algorithm>
#include <iostream>
#include <random>
#include "CapFile.hpp"
#include "MathConstants.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include "BinaryIO.hpp"


namespace CAP
{
  namespace MATH
  {

  //////////////////////////////////////////////////////////////////////////
  // Construction / assignment
  //////////////////////////////////////////////////////////////////////////

  Histogram::Histogram()
  :
  DataPoints("H","H","Histogram"),
  Partition(),
  _entries(),
  _avgValues(),
  _avgUncertainties()
  {
  addDimension(1,0.0,1.0,"x");
  resizeBins();
  }

  Histogram::Histogram(const Partition & partition)
  :
  DataPoints("H","H","Histogram"),
  Partition(partition),
  _entries(),
  _avgValues(),
  _avgUncertainties()
  {
  resizeBins();
  }

  Histogram::Histogram(const Histogram & rhs)
  :
  DataPoints(rhs),
  Partition(rhs),
  _entries(rhs._entries),
  _avgValues(rhs._avgValues),
  _avgUncertainties(rhs._avgUncertainties)
  {  }

  Histogram::Histogram(const String & name, const String & title, unsigned int n, double minimum, double maximum)
  :
  DataPoints(name,title,"Histogram"),
  Partition(),
  _entries(),
  _avgValues(),
  _avgUncertainties()
  {
  addDimension(n,minimum,maximum,"x");
  resizeBins();
  }

  Histogram::Histogram(const String & name, const String & title,
                       unsigned int n0, double minimum_0, double maximum_0,
                       unsigned int n1, double minimum_1, double maximum_1)
  :
  DataPoints(name,title,"Histogram"),
  Partition(),
  _entries(),
  _avgValues(),
  _avgUncertainties()
  {
  addDimension(n0,minimum_0,maximum_0,"x0");
  addDimension(n1,minimum_1,maximum_1,"x1");
  resizeBins();
  }

  Histogram::Histogram(const String & name, const String & title,
                       unsigned int n0, double minimum_0, double maximum_0,
                       unsigned int n1, double minimum_1, double maximum_1,
                       unsigned int n2, double minimum_2, double maximum_2)
  :
  DataPoints(name,title,"Histogram"),
  Partition(),
  _entries(),
  _avgValues(),
  _avgUncertainties()
  {
  addDimension(n0,minimum_0,maximum_0,"x0");
  addDimension(n1,minimum_1,maximum_1,"x1");
  addDimension(n2,minimum_2,maximum_2,"x2");
  resizeBins();
  }

  Histogram & Histogram::operator= (const Histogram & rhs)
  {
  if (this!=&rhs)
    {
    DataPoints::operator=(rhs);   // name, title and fits
    Partition::operator=(rhs);
    _entries       = rhs._entries;
    _avgValues        = rhs._avgValues;
    _avgUncertainties = rhs._avgUncertainties;
    }
  return *this;
  }

  //////////////////////////////////////////////////////////////////////////
  // Internal helpers
  //////////////////////////////////////////////////////////////////////////

  Histogram::Precision Histogram::precisionFromStorageOption(int storageOption)
  {
  switch (storageOption)
    {
    case 0:
    case 1: return kFloat;
    case 2: return kDouble;
    }
  throw Exception("Invalid storage option (0 int, 1 float, 2 double)","Histogram::precisionFromStorageOption()");
  }

  void Histogram::resizeBins()
  {
  unsigned int n = nBinsTotal();
  _entries.assign(n,0);
  _avgValues.assign(n,0.0);
  _avgUncertainties.assign(n,0.0);
  }

  void Histogram::setEmpty(unsigned int k)
  {
  _entries[k]       = 0;
  _avgValues[k]        = 0.0;
  _avgUncertainties[k] = 0.0;
  }

  void Histogram::setValue(unsigned int k, long entries, double binValue, double binUncertainty)
  {
  if (entries<=0)
    {
    setEmpty(k);
    return;
    }
  _entries[k]          = entries;
  _avgValues[k]        = binValue/double(entries);
  _avgUncertainties[k] = binUncertainty/double(entries);
  }

  void Histogram::assignContent(const Histogram & h)
  {
  if (&h==this) return;
  Partition::operator=(h);
  _entries       = h._entries;
  _avgValues        = h._avgValues;
  _avgUncertainties = h._avgUncertainties;
  }

  void Histogram::checkCompatible(const Histogram & h, const char * source) const
  {
  if (!compatibleWith(h))
    throw Exception("Operation invalid on Histograms of different dimensions",source);
  }

  void Histogram::checkRank(unsigned int requiredRank, const char * source) const
  {
  if (rank()!=requiredRank)
    throw Exception("Invalid function for histogram of this rank",source);
  }

  unsigned int Histogram::checkedIndex(int index, const char * source) const
  {
  if (index<0)
    throw IndexOutOfBoundException(index,nBinsTotal(),"Coordinate(s) outside histogram range",source);
  return (unsigned int) index;
  }

  void Histogram::resolveRange(unsigned int dimIndex, unsigned int first, unsigned int last,
                               unsigned int & iFirst, unsigned int & iLast, const char * source) const
  {
  unsigned int n = nBins(dimIndex);
  iFirst = first;
  iLast  = (last>=n) ? n-1 : last;
  if (iFirst>iLast) throw InvalidBoundsException(first,last,"first>last or first beyond axis",source);
  }

  void Histogram::resolveFlatRange(unsigned int first, unsigned int last,
                                   unsigned int & iFirst, unsigned int & iLast, const char * source) const
  {
  unsigned int n = nBinsTotal();
  if (n==0) throw Exception("Histogram has no bins",source);
  iFirst = first;
  iLast  = (last>=n) ? n-1 : last;
  if (iFirst>iLast) throw InvalidBoundsException(first,last,"first>last or first beyond range",source);
  }

  void Histogram::accumulate(unsigned int kTarget, const Histogram & h, unsigned int kSource)
  {
  long   n = _entries[kTarget] + h._entries[kSource];
  double t = value(kTarget) + h.value(kSource);
  double e = statErrorSum(valueUncertainty(kTarget),h.valueUncertainty(kSource));
  setValue(kTarget,n,t,e);
  }

  //////////////////////////////////////////////////////////////////////////
  // Comparison
  //////////////////////////////////////////////////////////////////////////

  //
  //Check if the two histograms have bin values equal within 2 combined standard deviations.
  //Histograms with different binning are not equal.
  //
  bool  Histogram::operator== (const Histogram & rhs) const
  {
  if (!compatibleWith(rhs)) return false;
  unsigned int n = _entries.size();
  for (unsigned int k=0; k<n; k++)
    {
    double diff        = value(k) - rhs.value(k);
    double uncertainty = statErrorSum(valueUncertainty(k),rhs.valueUncertainty(k));
    if (uncertainty==0.0)
      {
      if (diff!=0.0) return false;
      }
    else if (std::fabs(diff)>2.0*uncertainty)
      return false;
    }
  return true;
  }

  bool  Histogram::operator!= (const Histogram & rhs) const
  {
  return !operator==(rhs);
  }

  //////////////////////////////////////////////////////////////////////////
  // Arithmetic
  //////////////////////////////////////////////////////////////////////////

  Histogram & Histogram::operator+= (const Histogram & rhs)
  {
  add(rhs,1.0);
  return *this;
  }

  Histogram & Histogram::operator-= (const Histogram & rhs)
  {
  add(rhs,-1.0);
  return *this;
  }

  Histogram & Histogram::operator*= (const Histogram & rhs)
  {
  multiplyBy(rhs,1.0);
  return *this;
  }

  Histogram & Histogram::operator/= (const Histogram & rhs)
  {
  divideBy(rhs,1.0);
  return *this;
  }

  Histogram & Histogram::operator+= (double value)
  {
  add(value);
  return *this;
  }

  Histogram & Histogram::operator-= (double value)
  {
  add(-value);
  return *this;
  }

  Histogram & Histogram::operator*= (double value)
  {
  scale(value);
  return *this;
  }

  Histogram & Histogram::operator/= (double value)
  {
  if (value==0.0) throw Exception("Attempting division by zero",__FUNCTION__);
  scale(1.0/value);
  return *this;
  }

  void Histogram::scale(double coefficient)
  {
  unsigned int n = _entries.size();
  double absCoefficient = std::fabs(coefficient);
  for (unsigned int k=0; k<n; k++)
    {
    _avgValues[k]        *= coefficient;
    _avgUncertainties[k] *= absCoefficient;
    }
  }

  // add a constant to every bin value; empty bins become bins with one entry
  void Histogram::add(double constant)
  {
  unsigned int n = _entries.size();
  for (unsigned int k=0; k<n; k++)
    {
    long entries = _entries[k]>0 ? _entries[k] : 1;
    setValue(k,entries,value(k)+constant,valueUncertainty(k));
    }
  }

  // this = this + coefficient*h
  void Histogram::add(const Histogram & h, double coefficient)
  {
  checkCompatible(h,__FUNCTION__);
  unsigned int n = _entries.size();
  for (unsigned int k=0; k<n; k++)
    {
    long   entries     = _entries[k] + h._entries[k];
    double sum         = value(k) + coefficient*h.value(k);
    double uncertainty = statErrorSum(valueUncertainty(k),coefficient*h.valueUncertainty(k));
    setValue(k,entries,sum,uncertainty);
    }
  }

  // this = this * (coefficient*h)
  void Histogram::multiplyBy(const Histogram & h, double coefficient)
  {
  checkCompatible(h,__FUNCTION__);
  unsigned int n = _entries.size();
  for (unsigned int k=0; k<n; k++)
    {
    if (_entries[k]==0 || h._entries[k]==0) { setEmpty(k); continue; }
    double v1 = value(k);
    double e1 = valueUncertainty(k);
    double v2 = coefficient*h.value(k);
    double e2 = std::fabs(coefficient)*h.valueUncertainty(k);
    setValue(k,1,v1*v2,statErrorProduct(v1,e1,v2,e2));
    }
  }

  // this = this / (coefficient*h); bins where the divisor vanishes are set empty
  void Histogram::divideBy(const Histogram & h, double coefficient)
  {
  checkCompatible(h,__FUNCTION__);
  unsigned int n = _entries.size();
  for (unsigned int k=0; k<n; k++)
    {
    double v2 = coefficient*h.value(k);
    if (_entries[k]==0 || h._entries[k]==0 || v2==0.0) { setEmpty(k); continue; }
    double v1 = value(k);
    double e1 = valueUncertainty(k);
    double e2 = std::fabs(coefficient)*h.valueUncertainty(k);
    setValue(k,1,v1/v2,statErrorRatio(v1,e1,v2,e2));
    }
  }

  // replace every bin value by its inverse; empty or zero bins are set empty
  void  Histogram::inverse()
  {
  unsigned int n = _entries.size();
  for (unsigned int k=0; k<n; k++)
    {
    double v = value(k);
    if (_entries[k]==0 || v==0.0) { setEmpty(k); continue; }
    double inv            = 1.0/v;
    double invUncertainty = valueUncertainty(k)*inv*inv;
    setValue(k,1,inv,invUncertainty);   // entry count is no longer meaningful
    }
  }

  void Histogram::setAsSumOf(const Histogram & h1, const Histogram & h2,
                             double coefficient1, double coefficient2)
  {
  if (!h1.compatibleWith(h2))
    throw Exception("Operation invalid on Histogram of different dimensions: h1 and h2",__FUNCTION__);
  Histogram result(h1);
  result.scale(coefficient1);
  result.add(h2,coefficient2);
  assignContent(result);
  }

  void Histogram::setAsSumOf(const std::vector<const Histogram *> & histograms,
                             const std::vector<double> & coefficients)
  {
  if (histograms.empty()) throw Exception("No histograms given",__FUNCTION__);
  if (histograms.size()!=coefficients.size()) throw Exception("histograms and coefficients sizes differ",__FUNCTION__);
  for (auto h : histograms) if (!h) throw NullPointerException("histograms",__FUNCTION__);
  Histogram result(*histograms[0]);
  result.scale(coefficients[0]);
  for (unsigned int iHisto=1; iHisto<histograms.size(); iHisto++)
    result.add(*histograms[iHisto],coefficients[iHisto]);
  assignContent(result);
  }

  void Histogram::setAsProductOf(const Histogram & h1, const Histogram & h2,
                                 double coefficient1, double coefficient2)
  {
  if (!h1.compatibleWith(h2))
    throw Exception("Operation invalid on Histogram of different dimensions: h1 and h2",__FUNCTION__);
  Histogram result(h1);
  result.scale(coefficient1);
  result.multiplyBy(h2,coefficient2);
  assignContent(result);
  }

  void Histogram::setAsProductOf(const std::vector<const Histogram *> & histograms,
                                 const std::vector<double> & coefficients)
  {
  if (histograms.empty()) throw Exception("No histograms given",__FUNCTION__);
  if (histograms.size()!=coefficients.size()) throw Exception("histograms and coefficients sizes differ",__FUNCTION__);
  for (auto h : histograms) if (!h) throw NullPointerException("histograms",__FUNCTION__);
  Histogram result(*histograms[0]);
  result.scale(coefficients[0]);
  for (unsigned int iHisto=1; iHisto<histograms.size(); iHisto++)
    result.multiplyBy(*histograms[iHisto],coefficients[iHisto]);
  assignContent(result);
  }

  void Histogram::setAsRatioOf(const Histogram & h1, const Histogram & h2,
                               double coefficient1, double coefficient2)
  {
  if (!h1.compatibleWith(h2))
    throw Exception("Operation invalid on Histogram of different dimensions: h1 and h2",__FUNCTION__);
  Histogram result(h1);
  result.scale(coefficient1);
  result.divideBy(h2,coefficient2);
  assignContent(result);
  }

  //////////////////////////////////////////////////////////////////////////
  // Bin setters (value: entries*average; uncertainty: uncertainty of that value)
  //////////////////////////////////////////////////////////////////////////

  // set the entry count, keeping the bin value and its uncertainty
  void Histogram::doSetEntries(unsigned int k, long entries)
  {
  double t = value(k);
  double e = valueUncertainty(k);
  if (entries<=0 && (t!=0.0 || e!=0.0))
    throw Exception("entries must be >0 for a bin with non-zero content",__FUNCTION__);
  setValue(k,entries,t,e);
  }

  // set the bin value, keeping the entry count (1 if the bin was empty)
  void Histogram::doSetValue(unsigned int k, double value)
  {
  long entries = _entries[k]>0 ? _entries[k] : 1;
  setValue(k,entries,value,valueUncertainty(k));
  }

  // set the uncertainty of the bin value, keeping the entry count (1 if the bin was empty)
  void Histogram::doSetUncertainty(unsigned int k, double uncertainty)
  {
  long entries = _entries[k]>0 ? _entries[k] : 1;
  setValue(k,entries,value(k),uncertainty);
  }

  void Histogram::doSetBin(unsigned int k, long entries, double value, double uncertainty)
  {
  if (entries<=0 && (value!=0.0 || uncertainty!=0.0))
    throw Exception("entries must be >0 for a bin with non-zero content",__FUNCTION__);
  setValue(k,entries,value,uncertainty);
  }

  void Histogram::setBinEntriesAt(unsigned int i, long entries)                         { doSetEntries(binIndex(i,true),entries); }
  void Histogram::setBinValueAt(unsigned int i, double value)                           { doSetValue(binIndex(i,true),value); }
  void Histogram::setBinUncertaintyAt(unsigned int i, double uncertainty)               { doSetUncertainty(binIndex(i,true),uncertainty); }
  void Histogram::setBinAt(unsigned int i, long entries, double value, double uncertainty) { doSetBin(binIndex(i,true),entries,value,uncertainty); }

  void Histogram::setBinEntriesAt(unsigned int i0, unsigned int i1, long entries)          { doSetEntries(binIndex(i0,i1,true),entries); }
  void Histogram::setBinValueAt(unsigned int i0, unsigned int i1, double value)            { doSetValue(binIndex(i0,i1,true),value); }
  void Histogram::setBinUncertaintyAt(unsigned int i0, unsigned int i1, double uncertainty){ doSetUncertainty(binIndex(i0,i1,true),uncertainty); }
  void Histogram::setBinAt(unsigned int i0, unsigned int i1, long entries, double value, double uncertainty)
  { doSetBin(binIndex(i0,i1,true),entries,value,uncertainty); }

  void Histogram::setBinEntriesAt(unsigned int i0, unsigned int i1, unsigned int i2, long entries)
  { doSetEntries(binIndex(i0,i1,i2,true),entries); }
  void Histogram::setBinValueAt(unsigned int i0, unsigned int i1, unsigned int i2, double value)
  { doSetValue(binIndex(i0,i1,i2,true),value); }
  void Histogram::setBinUncertaintyAt(unsigned int i0, unsigned int i1, unsigned int i2, double uncertainty)
  { doSetUncertainty(binIndex(i0,i1,i2,true),uncertainty); }
  void Histogram::setBinAt(unsigned int i0, unsigned int i1, unsigned int i2, long entries, double value, double uncertainty)
  { doSetBin(binIndex(i0,i1,i2,true),entries,value,uncertainty); }

  void Histogram::setBinEntriesAt(const std::vector<unsigned int> & indices, long entries)
  { doSetEntries(binIndex(indices,true),entries); }
  void Histogram::setBinValueAt(const std::vector<unsigned int> & indices, double value)
  { doSetValue(binIndex(indices,true),value); }
  void Histogram::setBinUncertaintyAt(const std::vector<unsigned int> & indices, double uncertainty)
  { doSetUncertainty(binIndex(indices,true),uncertainty); }
  void Histogram::setBinAt(const std::vector<unsigned int> & indices, long entries, double value, double uncertainty)
  { doSetBin(binIndex(indices,true),entries,value,uncertainty); }

  void Histogram::setBinEntriesAt(double x, long entries)
  { doSetEntries(checkedIndex(binIndex(x),__FUNCTION__),entries); }
  void Histogram::setBinValueAt(double x, double value)
  { doSetValue(checkedIndex(binIndex(x),__FUNCTION__),value); }
  void Histogram::setBinUncertaintyAt(double x, double uncertainty)
  { doSetUncertainty(checkedIndex(binIndex(x),__FUNCTION__),uncertainty); }
  void Histogram::setBinAt(double x, long entries, double value, double uncertainty)
  { doSetBin(checkedIndex(binIndex(x),__FUNCTION__),entries,value,uncertainty); }

  void Histogram::setBinEntriesAt(double x0, double x1, long entries)
  { doSetEntries(checkedIndex(binIndex(x0,x1),__FUNCTION__),entries); }
  void Histogram::setBinValueAt(double x0, double x1, double value)
  { doSetValue(checkedIndex(binIndex(x0,x1),__FUNCTION__),value); }
  void Histogram::setBinUncertaintyAt(double x0, double x1, double uncertainty)
  { doSetUncertainty(checkedIndex(binIndex(x0,x1),__FUNCTION__),uncertainty); }
  void Histogram::setBinAt(double x0, double x1, long entries, double value, double uncertainty)
  { doSetBin(checkedIndex(binIndex(x0,x1),__FUNCTION__),entries,value,uncertainty); }

  void Histogram::setBinEntriesAt(double x0, double x1, double x2, long entries)
  { doSetEntries(checkedIndex(binIndex(x0,x1,x2),__FUNCTION__),entries); }
  void Histogram::setBinValueAt(double x0, double x1, double x2, double value)
  { doSetValue(checkedIndex(binIndex(x0,x1,x2),__FUNCTION__),value); }
  void Histogram::setBinUncertaintyAt(double x0, double x1, double x2, double uncertainty)
  { doSetUncertainty(checkedIndex(binIndex(x0,x1,x2),__FUNCTION__),uncertainty); }
  void Histogram::setBinAt(double x0, double x1, double x2, long entries, double value, double uncertainty)
  { doSetBin(checkedIndex(binIndex(x0,x1,x2),__FUNCTION__),entries,value,uncertainty); }

  void Histogram::setBinEntriesAt(const std::vector<double> & x, long entries)
  { doSetEntries(checkedIndex(binIndex(x),__FUNCTION__),entries); }
  void Histogram::setBinValueAt(const std::vector<double> & x, double value)
  { doSetValue(checkedIndex(binIndex(x),__FUNCTION__),value); }
  void Histogram::setBinUncertaintyAt(const std::vector<double> & x, double uncertainty)
  { doSetUncertainty(checkedIndex(binIndex(x),__FUNCTION__),uncertainty); }
  void Histogram::setBinAt(const std::vector<double> & x, long entries, double value, double uncertainty)
  { doSetBin(checkedIndex(binIndex(x),__FUNCTION__),entries,value,uncertainty); }

  //////////////////////////////////////////////////////////////////////////
  // setWith: bin values (entries*average) and their uncertainties
  //////////////////////////////////////////////////////////////////////////

  template<typename E, typename V, typename U>
  void Histogram::setWithValues(const std::vector<E> * entries, const std::vector<V> & values,
                                const std::vector<U> * uncertainties, const char * source)
  {
  unsigned int n = nBinsTotal();
  if (values.size()!=n)                          throw Exception("values size does not match number of bins",source);
  if (entries && entries->size()!=n)             throw Exception("entries size does not match number of bins",source);
  if (uncertainties && uncertainties->size()!=n) throw Exception("uncertainties size does not match number of bins",source);
  for (unsigned int k=0; k<n; k++)
    {
    long   e = entries ? long((*entries)[k]) : 1;
    double u = uncertainties ? double((*uncertainties)[k]) : 0.0;
    doSetBin(k,e,double(values[k]),u);
    }
  }

  void Histogram::setWith(const std::vector<float> & values)
  { setWithValues<long,float,float>(nullptr,values,nullptr,__FUNCTION__); }
  void Histogram::setWith(const std::vector<double> & values)
  { setWithValues<long,double,double>(nullptr,values,nullptr,__FUNCTION__); }
  void Histogram::setWith(const std::vector<float> & values, const std::vector<float> & uncertainties)
  { setWithValues<long,float,float>(nullptr,values,&uncertainties,__FUNCTION__); }
  void Histogram::setWith(const std::vector<double> & values, const std::vector<double> & uncertainties)
  { setWithValues<long,double,double>(nullptr,values,&uncertainties,__FUNCTION__); }
  void Histogram::setWith(const std::vector<long> & entries, const std::vector<float> & values)
  { setWithValues<long,float,float>(&entries,values,nullptr,__FUNCTION__); }
  void Histogram::setWith(const std::vector<long> & entries, const std::vector<double> & values)
  { setWithValues<long,double,double>(&entries,values,nullptr,__FUNCTION__); }
  void Histogram::setWith(const std::vector<long> & entries, const std::vector<float> & values, const std::vector<float> & uncertainties)
  { setWithValues<long,float,float>(&entries,values,&uncertainties,__FUNCTION__); }
  void Histogram::setWith(const std::vector<long> & entries, const std::vector<double> & values, const std::vector<double> & uncertainties)
  { setWithValues<long,double,double>(&entries,values,&uncertainties,__FUNCTION__); }

  //////////////////////////////////////////////////////////////////////////
  // Extraction and projection (all ranges inclusive)
  //////////////////////////////////////////////////////////////////////////

  Histogram Histogram::extract1(unsigned int first, unsigned int last) const
  {
  checkRank(1,__FUNCTION__);
  unsigned int f, l;
  resolveRange(0,first,last,f,l,__FUNCTION__);
  Partition partition;
  partition.addDimension(l-f+1, binLowEdge(f,0), binHighEdge(l,0), dimensionTitle(0));
  Histogram extracted(partition);
  extracted.setPrecision(precision());
  extracted.setName(name());
  extracted.setTitle(title());
  unsigned int iTarget=0;
  for (unsigned int iSource=f; iSource<=l; iSource++)
    {
    extracted._entries[iTarget]       = _entries[iSource];
    extracted._avgValues[iTarget]        = _avgValues[iSource];
    extracted._avgUncertainties[iTarget] = _avgUncertainties[iSource];
    iTarget++;
    }
  return extracted;
  }

  Histogram Histogram::extract2(unsigned int first0, unsigned int last0,
                                unsigned int first1, unsigned int last1) const
  {
  checkRank(2,__FUNCTION__);
  unsigned int f0, l0, f1, l1;
  resolveRange(0,first0,last0,f0,l0,__FUNCTION__);
  resolveRange(1,first1,last1,f1,l1,__FUNCTION__);
  Partition partition;
  partition.addDimension(l0-f0+1, binLowEdge(f0,0), binHighEdge(l0,0), dimensionTitle(0));
  partition.addDimension(l1-f1+1, binLowEdge(f1,1), binHighEdge(l1,1), dimensionTitle(1));
  Histogram extracted(partition);
  extracted.setPrecision(precision());
  extracted.setName(name());
  extracted.setTitle(title());
  for (unsigned int i0=f0; i0<=l0; i0++)
    {
    for (unsigned int i1=f1; i1<=l1; i1++)
      {
      unsigned int indexSource = binIndex(i0,i1);
      unsigned int indexTarget = extracted.binIndex(i0-f0,i1-f1);
      extracted._entries[indexTarget]       = _entries[indexSource];
      extracted._avgValues[indexTarget]        = _avgValues[indexSource];
      extracted._avgUncertainties[indexTarget] = _avgUncertainties[indexSource];
      }
    }
  return extracted;
  }

  Histogram Histogram::extract3(unsigned int first0, unsigned int last0,
                                unsigned int first1, unsigned int last1,
                                unsigned int first2, unsigned int last2) const
  {
  checkRank(3,__FUNCTION__);
  unsigned int f0, l0, f1, l1, f2, l2;
  resolveRange(0,first0,last0,f0,l0,__FUNCTION__);
  resolveRange(1,first1,last1,f1,l1,__FUNCTION__);
  resolveRange(2,first2,last2,f2,l2,__FUNCTION__);
  Partition partition;
  partition.addDimension(l0-f0+1, binLowEdge(f0,0), binHighEdge(l0,0), dimensionTitle(0));
  partition.addDimension(l1-f1+1, binLowEdge(f1,1), binHighEdge(l1,1), dimensionTitle(1));
  partition.addDimension(l2-f2+1, binLowEdge(f2,2), binHighEdge(l2,2), dimensionTitle(2));
  Histogram extracted(partition);
  extracted.setPrecision(precision());
  extracted.setName(name());
  extracted.setTitle(title());
  for (unsigned int i0=f0; i0<=l0; i0++)
    {
    for (unsigned int i1=f1; i1<=l1; i1++)
      {
      for (unsigned int i2=f2; i2<=l2; i2++)
        {
        unsigned int indexSource = binIndex(i0,i1,i2);
        unsigned int indexTarget = extracted.binIndex(i0-f0,i1-f1,i2-f2);
        extracted._entries[indexTarget]       = _entries[indexSource];
        extracted._avgValues[indexTarget]        = _avgValues[indexSource];
        extracted._avgUncertainties[indexTarget] = _avgUncertainties[indexSource];
        }
      }
    }
  return extracted;
  }

  Histogram Histogram::project2DOnto1D(unsigned int targetAxisIndex, unsigned int first, unsigned int last) const
  {
  checkRank(2,__FUNCTION__);
  if (targetAxisIndex>1) throw IndexOutOfBoundException(targetAxisIndex,2,"targetAxisIndex out of bound",__FUNCTION__);
  unsigned int other = 1-targetAxisIndex;
  unsigned int f, l;
  resolveRange(other,first,last,f,l,__FUNCTION__);
  Histogram projection(extractPartition(targetAxisIndex));
  projection.setPrecision(precision());
  projection.setName(name());
  projection.setTitle(title());
  unsigned int nTarget = nBins(targetAxisIndex);
  for (unsigned int iT=0; iT<nTarget; iT++)
    {
    for (unsigned int j=f; j<=l; j++)
      {
      unsigned int k = (targetAxisIndex==0) ? binIndex(iT,j) : binIndex(j,iT);
      projection.accumulate(iT,*this,k);
      }
    }
  return projection;
  }

  Histogram Histogram::project3DOnto1D(unsigned int targetAxisIndex,
                                       unsigned int first1, unsigned int last1,
                                       unsigned int first2, unsigned int last2) const
  {
  checkRank(3,__FUNCTION__);
  if (targetAxisIndex>2) throw IndexOutOfBoundException(targetAxisIndex,3,"targetAxisIndex out of bound",__FUNCTION__);
  unsigned int a = (targetAxisIndex==0) ? 1 : 0;
  unsigned int b = (targetAxisIndex==2) ? 1 : 2;
  unsigned int fa, la, fb, lb;
  resolveRange(a,first1,last1,fa,la,__FUNCTION__);
  resolveRange(b,first2,last2,fb,lb,__FUNCTION__);
  Histogram projection(extractPartition(targetAxisIndex));
  projection.setPrecision(precision());
  projection.setName(name());
  projection.setTitle(title());
  unsigned int idx[3];
  unsigned int nTarget = nBins(targetAxisIndex);
  for (unsigned int iT=0; iT<nTarget; iT++)
    {
    idx[targetAxisIndex] = iT;
    for (unsigned int j=fa; j<=la; j++)
      {
      idx[a] = j;
      for (unsigned int m=fb; m<=lb; m++)
        {
        idx[b] = m;
        projection.accumulate(iT,*this,binIndex(idx[0],idx[1],idx[2]));
        }
      }
    }
  return projection;
  }

  Histogram Histogram::project3DOnto2D(unsigned int targetAxisIndex1, unsigned int targetAxisIndex2,
                                       unsigned int first, unsigned int last) const
  {
  checkRank(3,__FUNCTION__);
  if (targetAxisIndex1>2) throw IndexOutOfBoundException(targetAxisIndex1,3,"targetAxisIndex1 out of bound",__FUNCTION__);
  if (targetAxisIndex2>2) throw IndexOutOfBoundException(targetAxisIndex2,3,"targetAxisIndex2 out of bound",__FUNCTION__);
  if (targetAxisIndex1==targetAxisIndex2) throw Exception("Target axes must differ",__FUNCTION__);
  unsigned int other = 3 - targetAxisIndex1 - targetAxisIndex2;
  unsigned int f, l;
  resolveRange(other,first,last,f,l,__FUNCTION__);
  Histogram projection(extractPartition(targetAxisIndex1,targetAxisIndex2));
  projection.setPrecision(precision());
  projection.setName(name());
  projection.setTitle(title());
  unsigned int idx[3];
  unsigned int n1 = nBins(targetAxisIndex1);
  unsigned int n2 = nBins(targetAxisIndex2);
  for (unsigned int i1=0; i1<n1; i1++)
    {
    idx[targetAxisIndex1] = i1;
    for (unsigned int i2=0; i2<n2; i2++)
      {
      idx[targetAxisIndex2] = i2;
      unsigned int kTarget = projection.binIndex(i1,i2);
      for (unsigned int m=f; m<=l; m++)
        {
        idx[other] = m;
        projection.accumulate(kTarget,*this,binIndex(idx[0],idx[1],idx[2]));
        }
      }
    }
  return projection;
  }

  //////////////////////////////////////////////////////////////////////////
  // Sums and integrals (ranges inclusive; last beyond the axis is clamped)
  //////////////////////////////////////////////////////////////////////////

  double Histogram::totalEntries() const
  {
  double sum = 0;
  for (auto entry : _entries) sum += entry;
  return sum;
  }

  double Histogram::totalEntries(unsigned int first, unsigned int last) const
  {
  checkRank(1,__FUNCTION__);
  unsigned int f, l;
  resolveRange(0,first,last,f,l,__FUNCTION__);
  double sum = 0.0;
  for (unsigned int k=f; k<=l; k++) sum += _entries[k];
  return sum;
  }

  double Histogram::totalEntries(unsigned int first0, unsigned int last0,
                                 unsigned int first1, unsigned int last1) const
  {
  checkRank(2,__FUNCTION__);
  unsigned int f0, l0, f1, l1;
  resolveRange(0,first0,last0,f0,l0,__FUNCTION__);
  resolveRange(1,first1,last1,f1,l1,__FUNCTION__);
  double sum = 0.0;
  for (unsigned int k0=f0; k0<=l0; k0++)
    for (unsigned int k1=f1; k1<=l1; k1++)
      sum += _entries[binIndex(k0,k1)];
  return sum;
  }

  double Histogram::totalEntries(unsigned int first0, unsigned int last0,
                                 unsigned int first1, unsigned int last1,
                                 unsigned int first2, unsigned int last2) const
  {
  checkRank(3,__FUNCTION__);
  unsigned int f0, l0, f1, l1, f2, l2;
  resolveRange(0,first0,last0,f0,l0,__FUNCTION__);
  resolveRange(1,first1,last1,f1,l1,__FUNCTION__);
  resolveRange(2,first2,last2,f2,l2,__FUNCTION__);
  double sum = 0.0;
  for (unsigned int k0=f0; k0<=l0; k0++)
    for (unsigned int k1=f1; k1<=l1; k1++)
      for (unsigned int k2=f2; k2<=l2; k2++)
        sum += _entries[binIndex(k0,k1,k2)];
  return sum;
  }

  //
  //Sum of bin values, or integral (sum times bin volume) when binSumOnly is false.
  //
  double Histogram::integral(bool binSumOnly) const
  {
  double sum = 0.0;
  for (unsigned int k=0; k<_entries.size(); k++) sum += value(k);
  if (binSumOnly) return sum;
  double w = 1.0;
  for (unsigned int dim=0; dim<rank(); dim++) w *= binWidth(0,dim); // needs to be updated for variable widths
  return sum*w;
  }

  double Histogram::integral(unsigned int first, unsigned int last, bool binSumOnly) const
  {
  checkRank(1,__FUNCTION__);
  unsigned int f, l;
  resolveRange(0,first,last,f,l,__FUNCTION__);
  double sum = 0.0;
  for (unsigned int k=f; k<=l; k++) sum += value(k);
  if (binSumOnly) return sum;
  return sum*binWidth(0,0); // needs to be updated for variable widths
  }

  double Histogram::integral(unsigned int first0, unsigned int last0,
                             unsigned int first1, unsigned int last1,
                             bool binSumOnly)  const
  {
  checkRank(2,__FUNCTION__);
  unsigned int f0, l0, f1, l1;
  resolveRange(0,first0,last0,f0,l0,__FUNCTION__);
  resolveRange(1,first1,last1,f1,l1,__FUNCTION__);
  double sum = 0.0;
  for (unsigned int k0=f0; k0<=l0; k0++)
    for (unsigned int k1=f1; k1<=l1; k1++)
      sum += value(binIndex(k0,k1));
  if (binSumOnly) return sum;
  return sum*binWidth(0,0)*binWidth(0,1); // needs to be updated for variable widths
  }

  double Histogram::integral(unsigned int first0, unsigned int last0,
                             unsigned int first1, unsigned int last1,
                             unsigned int first2, unsigned int last2,
                             bool binSumOnly)  const
  {
  checkRank(3,__FUNCTION__);
  unsigned int f0, l0, f1, l1, f2, l2;
  resolveRange(0,first0,last0,f0,l0,__FUNCTION__);
  resolveRange(1,first1,last1,f1,l1,__FUNCTION__);
  resolveRange(2,first2,last2,f2,l2,__FUNCTION__);
  double sum = 0.0;
  for (unsigned int k0=f0; k0<=l0; k0++)
    for (unsigned int k1=f1; k1<=l1; k1++)
      for (unsigned int k2=f2; k2<=l2; k2++)
        sum += value(binIndex(k0,k1,k2));
  if (binSumOnly) return sum;
  return sum*binWidth(0,0)*binWidth(0,1)*binWidth(0,2); // needs to be updated for variable widths
  }

  //////////////////////////////////////////////////////////////////////////
  // Extrema (global bin index range)
  //////////////////////////////////////////////////////////////////////////

  double Histogram::binQuantity(unsigned int k, int quantity) const
  {
  switch (quantity)
    {
      case 0:  return double(_entries[k]);
      case 1:  return value(k);
      default: return _avgValues[k];
    }
  }

  unsigned int Histogram::extremumBin(int quantity, bool findMaximum, unsigned int first, unsigned int last) const
  {
  unsigned int f, l;
  resolveFlatRange(first,last,f,l,__FUNCTION__);
  unsigned int best = f;
  double bestValue  = binQuantity(f,quantity);
  for (unsigned int k=f+1; k<=l; k++)
    {
    double v = binQuantity(k,quantity);
    if (findMaximum ? (v>bestValue) : (v<bestValue)) { best = k; bestValue = v; }
    }
  return best;
  }

  void Histogram::maximumEntries(unsigned int & index, long & value, unsigned int first, unsigned int last) const
  { index = extremumBin(0,true,first,last);  value = _entries[index]; }
  void Histogram::minimumEntries(unsigned int & index, long & value, unsigned int first, unsigned int last) const
  { index = extremumBin(0,false,first,last); value = _entries[index]; }
  void Histogram::maximumInRange(unsigned int & index, double & value, unsigned int first, unsigned int last) const
  { index = extremumBin(1,true,first,last);  value = this->value(index); }
  void Histogram::minimumInRange(unsigned int & index, double & value, unsigned int first, unsigned int last) const
  { index = extremumBin(1,false,first,last); value = this->value(index); }
  void Histogram::maximumAvgValue(unsigned int & index, double & value, unsigned int first, unsigned int last) const
  { index = extremumBin(2,true,first,last);  value = _avgValues[index]; }
  void Histogram::minimumAvgValue(unsigned int & index, double & value, unsigned int first, unsigned int last) const
  { index = extremumBin(2,false,first,last); value = _avgValues[index]; }

  double Histogram::maximumEntries(unsigned int first, unsigned int last) const  { return _entries[extremumBin(0,true,first,last)]; }
  double Histogram::minimumEntries(unsigned int first, unsigned int last) const  { return _entries[extremumBin(0,false,first,last)]; }
  double Histogram::maximumInRange(unsigned int first, unsigned int last) const    { return value(extremumBin(1,true,first,last)); }
  double Histogram::minimumInRange(unsigned int first, unsigned int last) const    { return value(extremumBin(1,false,first,last)); }
  double Histogram::maximumAvgValue(unsigned int first, unsigned int last) const { return _avgValues[extremumBin(2,true,first,last)]; }
  double Histogram::minimumAvgValue(unsigned int first, unsigned int last) const { return _avgValues[extremumBin(2,false,first,last)]; }

  unsigned int Histogram::binWithMaxEntries(unsigned int first, unsigned int last) const  { return extremumBin(0,true,first,last); }
  unsigned int Histogram::binWithMinEntries(unsigned int first, unsigned int last) const  { return extremumBin(0,false,first,last); }
  unsigned int Histogram::binWithMaxValue(unsigned int first, unsigned int last) const    { return extremumBin(1,true,first,last); }
  unsigned int Histogram::binWithMinValue(unsigned int first, unsigned int last) const    { return extremumBin(1,false,first,last); }
  unsigned int Histogram::binWithMaxAvgValue(unsigned int first, unsigned int last) const { return extremumBin(2,true,first,last); }
  unsigned int Histogram::binWithMinAvgValue(unsigned int first, unsigned int last) const { return extremumBin(2,false,first,last); }

  //////////////////////////////////////////////////////////////////////////
  // Clear / reset / fill
  //////////////////////////////////////////////////////////////////////////

  //
  //Clear this histogram. Effectively, it becomes undefined (rank 0, no bins).
  //
  void Histogram::clear()
  {
  Partition::clear();
  _entries.clear();
  _avgValues.clear();
  _avgUncertainties.clear();
  }

  //
  //Reset the bin content to zero without changing the rank and dimensions of this histogram.
  //
  void Histogram::reset()
  {
  std::fill(_entries.begin(),_entries.end(),0);
  _avgValues.fill(0.0);
  _avgUncertainties.fill(0.0);
  }

  //
  // Fill: adds one entry of the given weight. The bin value grows by weight and the squared
  // uncertainty of the value grows by weight^2. Out-of-range coordinates are ignored.
  //
  void Histogram::fill(double x, double weight)
  {
  int index = binIndex(x);
  if (index<0) return;
  double e = valueUncertainty(index);
  setValue(index,_entries[index]+1,value(index)+weight,std::sqrt(e*e+weight*weight));
  }

  void Histogram::fill(double x0, double x1, double weight)
  {
  int index = binIndex(x0,x1);
  if (index<0) return;
  double e = valueUncertainty(index);
  setValue(index,_entries[index]+1,value(index)+weight,std::sqrt(e*e+weight*weight));
  }

  void Histogram::fill(double x0, double x1, double x2, double weight)
  {
  int index = binIndex(x0,x1,x2);
  if (index<0) return;
  double e = valueUncertainty(index);
  setValue(index,_entries[index]+1,value(index)+weight,std::sqrt(e*e+weight*weight));
  }

  void Histogram::fill(const std::vector<double> & x, double weight)
  {
  int index = binIndex(x);
  if (index<0) return;
  double e = valueUncertainty(index);
  setValue(index,_entries[index]+1,value(index)+weight,std::sqrt(e*e+weight*weight));
  }

  //////////////////////////////////////////////////////////////////////////
  // Bin getters (strict index checking)
  //////////////////////////////////////////////////////////////////////////

  double Histogram::binEntriesAt(unsigned int i) const                                   { return _entries[binIndex(i,true)]; }
  double Histogram::binEntriesAt(unsigned int i0, unsigned int i1) const                 { return _entries[binIndex(i0,i1,true)]; }
  double Histogram::binEntriesAt(unsigned int i0, unsigned int i1, unsigned int i2) const{ return _entries[binIndex(i0,i1,i2,true)]; }
  double Histogram::binEntriesAt(const std::vector<unsigned int> & indices) const        { return _entries[binIndex(indices,true)]; }

  double Histogram::binValueAt(unsigned int i) const                                     { return value(binIndex(i,true)); }
  double Histogram::binValueAt(unsigned int i0, unsigned int i1) const                   { return value(binIndex(i0,i1,true)); }
  double Histogram::binValueAt(unsigned int i0, unsigned int i1, unsigned int i2) const  { return value(binIndex(i0,i1,i2,true)); }
  double Histogram::binValueAt(const std::vector<unsigned int> & indices) const          { return value(binIndex(indices,true)); }

  double Histogram::binUncertaintyAt(unsigned int i) const                                    { return valueUncertainty(binIndex(i,true)); }
  double Histogram::binUncertaintyAt(unsigned int i0, unsigned int i1) const                  { return valueUncertainty(binIndex(i0,i1,true)); }
  double Histogram::binUncertaintyAt(unsigned int i0, unsigned int i1, unsigned int i2) const { return valueUncertainty(binIndex(i0,i1,i2,true)); }
  double Histogram::binUncertaintyAt(const std::vector<unsigned int> & indices) const         { return valueUncertainty(binIndex(indices,true)); }

  double Histogram::binAvgValueAt(unsigned int i) const                                   { return _avgValues[binIndex(i,true)]; }
  double Histogram::binAvgValueAt(unsigned int i0, unsigned int i1) const                 { return _avgValues[binIndex(i0,i1,true)]; }
  double Histogram::binAvgValueAt(unsigned int i0, unsigned int i1, unsigned int i2) const{ return _avgValues[binIndex(i0,i1,i2,true)]; }
  double Histogram::binAvgValueAt(const std::vector<unsigned int> & indices) const        { return _avgValues[binIndex(indices,true)]; }

  double Histogram::binAvgValueUncertaintyAt(unsigned int i) const                                   { return _avgUncertainties[binIndex(i,true)]; }
  double Histogram::binAvgValueUncertaintyAt(unsigned int i0, unsigned int i1) const                 { return _avgUncertainties[binIndex(i0,i1,true)]; }
  double Histogram::binAvgValueUncertaintyAt(unsigned int i0, unsigned int i1, unsigned int i2) const{ return _avgUncertainties[binIndex(i0,i1,i2,true)]; }
  double Histogram::binAvgValueUncertaintyAt(const std::vector<unsigned int> & indices) const        { return _avgUncertainties[binIndex(indices,true)]; }

  std::vector<double> Histogram::binValues() const
  {
  std::vector<double> result;
  result.reserve(_avgValues.size());
  for (unsigned int k=0; k<_avgValues.size(); k++)
    result.push_back(_entries[k]>0 ? value(k) : -1.0E100); // -1.0E100 indicates no data in this bin
  return result;
  }

  std::vector<double> Histogram::binValueUncertainties() const
  {
  std::vector<double> result;
  result.reserve(_avgUncertainties.size());
  for (unsigned int k=0; k<_avgUncertainties.size(); k++)
    result.push_back(_entries[k]>0 ? valueUncertainty(k) : -1.0E100); // -1.0E100 indicates no data in this bin
  return result;
  }

  //////////////////////////////////////////////////////////////////////////
  // Printing
  //////////////////////////////////////////////////////////////////////////

  unsigned int Histogram::size() const
  {
  return nBinsTotal();
  }

  std::vector<double> Histogram::coordinatesAt(unsigned int index) const
  {
  if (index>=nBinsTotal()) throw IndexOutOfBoundException(index,nBinsTotal(),"Out of bound index",__FUNCTION__);
  // global index = i0 + n0*(i1 + n1*(i2 + ...)): axis 0 varies fastest
  std::vector<double> x(rank());
  unsigned int rest = index;
  for (unsigned int k=0; k<rank(); k++)
    {
    unsigned int n = nBins(k);
    x[k] = binCenter(rest%n,k);
    rest /= n;
    }
  return x;
  }

  std::vector<double> Histogram::coordinateErrorsAt(unsigned int index) const
  {
  if (index>=nBinsTotal()) throw IndexOutOfBoundException(index,nBinsTotal(),"Out of bound index",__FUNCTION__);
  return std::vector<double>(rank(),0.0);
  }

  double Histogram::valueAt(unsigned int index) const
  {
  if (index>=nBinsTotal()) throw IndexOutOfBoundException(index,nBinsTotal(),"Out of bound index",__FUNCTION__);
  return value(index);
  }

  double Histogram::statErrorAt(unsigned int index) const
  {
  if (index>=nBinsTotal()) throw IndexOutOfBoundException(index,nBinsTotal(),"Out of bound index",__FUNCTION__);
  return valueUncertainty(index);
  }

  double Histogram::sysErrorAt(unsigned int index) const
  {
  statErrorAt(index);   // same index check
  return 0.0;           // a histogram has no systematic uncertainty
  }

  DataPoint2D Histogram::point(unsigned int index) const
  {
  if (rank()!=1) throw MathException("Data points are only defined for a histogram of rank 1",__FUNCTION__);
  // the uncertainty of the bin is statistical; a histogram has no systematic uncertainty
  return DataPoint2D(binCenter(index), binValueAt(index), binUncertaintyAt(index), 0.0);
  }

  void Histogram::addPoint(const DataPoint2D & /*dataPoint*/)
  {
  throw MathException("Cannot add data points to a histogram: use fill()",__FUNCTION__);
  }

  void Histogram::print() const
  {
  std::cout << *this;
  }

  std::ostream& operator<<(std::ostream& out, const Histogram & h)
  {
  out << "Histogram Name: "  << h.name()  << std::endl;
  out << "Histogram Title: " << h.title() << std::endl;
  out << "Histogram Rank: "  << h.rank()  << std::endl;
  out << " Bin Index    Entries   Values    dValues    AvgValues   dAvgValue" << std::endl;
  for (unsigned int k=0; k<h._entries.size(); k++)
    {
    out
    <<  k << "   "
    << h._entries[k] << "   "
    << h.value(k) << "  "
    << h.valueUncertainty(k) << "  "
    << h._avgValues[k] << "  "
    << h._avgUncertainties[k] << std::endl;
    }
  return out;
  }

  //////////////////////////////////////////////////////////////////////////
  // Free operators
  //////////////////////////////////////////////////////////////////////////

  Histogram operator+(const Histogram & left, const Histogram & right)
  {
  Histogram result(left);
  result += right;
  return result;
  }

  Histogram operator-(const Histogram & left, const Histogram & right)
  {
  Histogram result(left);
  result -= right;
  return result;
  }

  Histogram operator*(const Histogram & left, const Histogram & right)
  {
  Histogram result(left);
  result *= right;
  return result;
  }

  Histogram operator/(const Histogram & left, const Histogram & right)
  {
  Histogram result(left);
  result /= right;
  return result;
  }

  Histogram operator+(const Histogram & left, double right)
  {
  Histogram result(left);
  result.add(right);
  return result;
  }

  Histogram operator+(double left, const Histogram & right)
  {
  Histogram result(right);
  result.add(left);
  return result;
  }

  Histogram operator-(const Histogram & left, double right)
  {
  Histogram result(left);
  result.add(-right);
  return result;
  }

  Histogram operator-(double left, const Histogram & right)
  {
  Histogram result(right);
  result.scale(-1.0);
  result.add(left);
  return result;
  }

  Histogram operator*(const Histogram & left, double right)
  {
  Histogram result(left);
  result.scale(right);
  return result;
  }

  Histogram operator*(double left, const Histogram & right)
  {
  Histogram result(right);
  result.scale(left);
  return result;
  }

  Histogram operator/(const Histogram & left, double right)
  {
  if (right==0.0) throw Exception("Attempting division by zero",__FUNCTION__);
  Histogram result(left);
  result.scale(1.0/right);
  return result;
  }

  Histogram operator/(double left, const Histogram & right)
  {
  Histogram result(right);
  result.inverse();
  result.scale(left);
  return result;
  }

  Histogram operator+(const Histogram & right)
  {
  return Histogram(right);
  }

  Histogram operator-(const Histogram & right)
  {
  Histogram result(right);
  result.scale(-1.0);
  return result;
  }

  // ---------------------------------------------------------------- binary files

  void Histogram::encodeBinary(BinaryWriter & w) const
  {
  w.writeString(title());
  w.writeVarUInt(_type);
  w.writeVarUInt(_rank);
  for (unsigned int k=0; k<_rank; k++)
    {
    w.writeVarUInt(_nBins[k]);
    w.writeFloat32(_minima[k]);
    w.writeFloat32(_maxima[k]);
    w.writeString(_titles[k]);
    }
  // only bins with non-zero entries, value or uncertainty are written
  uint64_t nNonEmpty = 0;
  for (std::size_t k=0; k<_entries.size(); k++)
    if (_entries[k]!=0 || _avgValues[k]!=0.0 || _avgUncertainties[k]!=0.0) nNonEmpty++;
  bool isDouble = _avgValues.isDouble();
  w.writeUInt8(isDouble ? 1 : 0);   // format version 2: storage precision
  w.writeVarUInt(nNonEmpty);
  std::size_t next = 0;   // first global index after the previous non-empty bin
  for (std::size_t k=0; k<_entries.size(); k++)
    {
    if (_entries[k]==0 && _avgValues[k]==0.0 && _avgUncertainties[k]==0.0) continue;
    w.writeVarUInt(k-next);
    w.writeVarInt(_entries[k]);
    if (isDouble) { w.writeFloat64(_avgValues[k]); w.writeFloat64(_avgUncertainties[k]); }
    else          { w.writeFloat32(float(_avgValues[k])); w.writeFloat32(float(_avgUncertainties[k])); }
    next = k+1;
    }
  // format version 3: bin labels of each axis
  for (unsigned int d=0; d<_rank; d++)
    {
    w.writeVarUInt(_binLabels[d].size());
    for (const auto & label : _binLabels[d]) { w.writeVarUInt(label.first); w.writeString(label.second); }
    }
  _fits.encodeBinary(w);   // format version 4: fits attached
  }

  void Histogram::decodeBinary(BinaryReader & r, const String & histogramName, unsigned int version)
  {
  setName(histogramName);
  setTitle(r.readString());
  Partition::clear();
  unsigned int type = (unsigned int) r.readVarUInt();
  uint64_t rank = r.readVarUInt();
  if (rank > 64) throw Exception("Implausible rank","Histogram::decodeBinary()");
  uint64_t total = (rank==0) ? 0 : 1;
  for (uint64_t k=0; k<rank; k++)
    {
    uint64_t n = r.readVarUInt();
    float minimum = r.readFloat32();
    float maximum = r.readFloat32();
    String axisTitle = r.readString();
    total *= n;
    if (n==0 || total > (uint64_t(1)<<31)) throw Exception("Invalid number of bins","Histogram::decodeBinary()");
    addDimension((unsigned int)n,minimum,maximum,axisTitle);
    }
  _type = type;
  bool isDouble = false;
  if (version>=2)
    {
    uint8_t precisionFlag = r.readUInt8();
    if (precisionFlag>1) throw Exception("Invalid storage precision","Histogram::decodeBinary()");
    isDouble = precisionFlag==1;
    }
  setPrecision(isDouble ? kDouble : kFloat);
  resizeBins();
  uint64_t nNonEmpty = r.readVarUInt();
  if (nNonEmpty > total) throw Exception("More non-empty bins than bins","Histogram::decodeBinary()");
  uint64_t k = 0;
  for (uint64_t i=0; i<nNonEmpty; i++)
    {
    k += r.readVarUInt();
    if (k >= total) throw Exception("Bin index out of range","Histogram::decodeBinary()");
    _entries[k]       = long(r.readVarInt());
    if (isDouble) { _avgValues[k] = r.readFloat64(); _avgUncertainties[k] = r.readFloat64(); }
    else          { _avgValues[k] = r.readFloat32(); _avgUncertainties[k] = r.readFloat32(); }
    k++;
    }
  if (version>=3)
    {
    for (unsigned int d=0; d<_rank; d++)
      {
      uint64_t nLabels = r.readVarUInt();
      if (nLabels > _nBins[d]) throw Exception("More bin labels than bins","Histogram::decodeBinary()");
      for (uint64_t i=0; i<nLabels; i++)
        {
        uint64_t binIndex = r.readVarUInt();
        String label = r.readString();
        if (binIndex >= _nBins[d]) throw Exception("Bin label index out of range","Histogram::decodeBinary()");
        _binLabels[d][(unsigned int)binIndex] = label;
        }
      }
    }
  if (version>=4) _fits.decodeBinary(r);
  }

  void Histogram::writeBinary(std::ostream & out) const                                      { writeBinaryObject(out,*this); }
  bool Histogram::readBinary(std::istream & in)                                              { return readBinaryObject(in,*this); }
  void Histogram::saveBinary(const String & fileName, bool append) const                     { saveBinaryObject(fileName,*this,append); }
  void Histogram::loadBinary(const String & fileName, const String & histogramName)          { loadBinaryObject(fileName,*this,histogramName); }
  void Histogram::saveBinary(const String & fileName, const std::vector<Histogram> & hs)     { saveBinaryObjects(fileName,hs); }
  std::vector<Histogram> Histogram::loadAllBinary(const String & fileName)                   { return loadAllBinaryObjects<Histogram>(fileName); }


  //////////////////////////////////////////////////////////////////////////
  // Histogram helper functions (ported from Helpers/RootHistogramHelpers)
  //////////////////////////////////////////////////////////////////////////

  using std::cout;
  using std::endl;

  namespace
  {
  // ROOT-style access used by the ported helper functions: bins are numbered from 1 along each axis, as in TH1/TH2/
  // TH3; the content of a bin is its value (Histogram::binValueAtGlobal()) and its error the uncertainty of the value.
  // Histograms have no under/overflow bins: reading outside 1..n returns 0 and writing there is ignored, which is what
  // the ROOT code relied on. The number of indices must match the rank (HistogramException otherwise).

  void requireRank(const Histogram * h, unsigned int nIndices, const char * source)
  {
  if (!h) throw NullPointerException(source);
  if (h->rank()!=nIndices)
    throw HistogramException(h->name(),String("Bin accessed with ")+String(std::to_string(nIndices).c_str())
                             +" index(es) on a histogram of rank "+String(std::to_string(h->rank()).c_str()),source);
  }

  bool inside(const Histogram * h, unsigned int axis, int i) { return i>=1 && i<=int(h->nBins(axis)); }

  // Global index of ROOT-style bin numbers, or -1 outside the axes.
  long globalIndex(const Histogram * h, int i)
  { requireRank(h,1,"binContent"); return inside(h,0,i) ? long(i-1) : -1; }
  long globalIndex(const Histogram * h, int i, int j)
  {
  requireRank(h,2,"binContent");
  if (!inside(h,0,i) || !inside(h,1,j)) return -1;
  return long(i-1) + long(h->nBins(0))*long(j-1);
  }
  long globalIndex(const Histogram * h, int i, int j, int l)
  {
  requireRank(h,3,"binContent");
  if (!inside(h,0,i) || !inside(h,1,j) || !inside(h,2,l)) return -1;
  return long(i-1) + long(h->nBins(0))*(long(j-1) + long(h->nBins(1))*long(l-1));
  }

  double binContent(const Histogram * h, int i)               { long k=globalIndex(h,i);     return k<0 ? 0.0 : h->binValueAtGlobal(k); }
  double binContent(const Histogram * h, int i, int j)        { long k=globalIndex(h,i,j);   return k<0 ? 0.0 : h->binValueAtGlobal(k); }
  double binContent(const Histogram * h, int i, int j, int l) { long k=globalIndex(h,i,j,l); return k<0 ? 0.0 : h->binValueAtGlobal(k); }
  double binError(const Histogram * h, int i)                 { long k=globalIndex(h,i);     return k<0 ? 0.0 : h->binUncertaintyAtGlobal(k); }
  double binError(const Histogram * h, int i, int j)          { long k=globalIndex(h,i,j);   return k<0 ? 0.0 : h->binUncertaintyAtGlobal(k); }
  double binError(const Histogram * h, int i, int j, int l)   { long k=globalIndex(h,i,j,l); return k<0 ? 0.0 : h->binUncertaintyAtGlobal(k); }
  void setBinContent(Histogram * h, int i, double v)               { long k=globalIndex(h,i);     if (k>=0) h->setBinValueAtGlobal(k,v); }
  void setBinContent(Histogram * h, int i, int j, double v)        { long k=globalIndex(h,i,j);   if (k>=0) h->setBinValueAtGlobal(k,v); }
  void setBinContent(Histogram * h, int i, int j, int l, double v) { long k=globalIndex(h,i,j,l); if (k>=0) h->setBinValueAtGlobal(k,v); }
  void setBinError(Histogram * h, int i, double e)                 { long k=globalIndex(h,i);     if (k>=0) h->setBinUncertaintyAtGlobal(k,e); }
  void setBinError(Histogram * h, int i, int j, double e)          { long k=globalIndex(h,i,j);   if (k>=0) h->setBinUncertaintyAtGlobal(k,e); }
  void setBinError(Histogram * h, int i, int j, int l, double e)   { long k=globalIndex(h,i,j,l); if (k>=0) h->setBinUncertaintyAtGlobal(k,e); }

  // Number of bins along an axis (1 for an axis the histogram does not have, as in ROOT).
  int nBinsX(const Histogram * h) { if (!h) throw NullPointerException("nBinsX"); return h->nBins(0); }
  int nBinsY(const Histogram * h) { if (!h) throw NullPointerException("nBinsY"); return h->rank()>=2 ? int(h->nBins(1)) : 1; }
  int nBinsZ(const Histogram * h) { if (!h) throw NullPointerException("nBinsZ"); return h->rank()>=3 ? int(h->nBins(2)) : 1; }

  double binCenterX(const Histogram * h, int i)  { return h->binCenter(i-1,0); }
  double binCenterY(const Histogram * h, int i)  { return h->binCenter(i-1,1); }
  double binCenterZ(const Histogram * h, int i)  { return h->binCenter(i-1,2); }
  double binWidthX(const Histogram * h, int i)   { return h->binWidth(i-1,0); }
  double binWidthY(const Histogram * h, int i)   { return h->binWidth(i-1,1); }
  double binWidthZ(const Histogram * h, int i)   { return h->binWidth(i-1,2); }
  double binLowEdgeX(const Histogram * h, int i) { return h->binLowEdge(i-1,0); }

  // ROOT TAxis::FindBin: 0 below the axis, n+1 at or above its upper edge.
  int findBin(const Histogram * h, unsigned int axis, double x)
  {
  if (x<h->minimum(axis)) return 0;
  if (x>=h->maximum(axis)) return int(h->nBins(axis))+1;
  int b = h->axisBinIndex(x,axis);
  return b<0 ? int(h->nBins(axis))+1 : b+1;
  }
  int findBinX(const Histogram * h, double x) { return findBin(h,0,x); }
  int findBinY(const Histogram * h, double x) { return findBin(h,1,x); }

  // Random numbers for the Monte Carlo helpers (replaces ROOT's gRandom; fixed seed, as TRandom3's default).
  std::mt19937_64 & randomEngine()
  {
  static std::mt19937_64 engine(4357);
  return engine;
  }
  double uniform01() { return std::uniform_real_distribution<double>(0.0,1.0)(randomEngine()); }

  // ROOT TH2::GetRandom2: a random (x,y) distributed as the bin contents (uniform within the chosen bin).
  void getRandom2(const Histogram * h, double & x, double & y)
  {
  requireRank(h,2,"getRandom2");
  unsigned int n = h->nBinsTotal();
  std::vector<double> cumulative(n);
  double sum = 0.0;
  for (unsigned int k=0; k<n; k++) { double v = h->binValueAtGlobal(k); if (v>0) sum += v; cumulative[k] = sum; }
  if (sum<=0) throw HistogramException(h->name(),"Cannot draw random values from an empty histogram","getRandom2");
  double r = uniform01()*sum;
  unsigned int k = std::upper_bound(cumulative.begin(),cumulative.end(),r) - cumulative.begin();
  if (k>=n) k = n-1;
  unsigned int ix = k % h->nBins(0), iy = k / h->nBins(0);
  x = h->binLowEdge(ix,0) + uniform01()*h->binWidth(ix,0);
  y = h->binLowEdge(iy,1) + uniform01()*h->binWidth(iy,1);
  }

  // Sum of the contents (and the uncertainty) of the ROOT-style bins first..last of a rank 1 histogram, optionally
  // times the bin widths (ROOT IntegralAndError with option "width"). Bins outside 1..n are skipped.
  double integralAndError(const Histogram * h, int first, int last, double & error, bool width)
  {
  double sum = 0.0, e2 = 0.0;
  for (int i=std::max(first,1); i<=std::min(last,nBinsX(h)); i++)
    {
    double w = width ? binWidthX(h,i) : 1.0;
    sum += binContent(h,i)*w;
    double e = binError(h,i)*w;
    e2  += e*e;
    }
  error = std::sqrt(e2);
  return sum;
  }
  double integralAndError(const Histogram * h, int firstX, int lastX, int firstY, int lastY, double & error, bool width)
  {
  double sum = 0.0, e2 = 0.0;
  for (int i=std::max(firstX,1); i<=std::min(lastX,nBinsX(h)); i++)
    for (int j=std::max(firstY,1); j<=std::min(lastY,nBinsY(h)); j++)
      {
      double w = width ? binWidthX(h,i)*binWidthY(h,j) : 1.0;
      sum += binContent(h,i,j)*w;
      double e = binError(h,i,j)*w;
      e2  += e*e;
      }
  error = std::sqrt(e2);
  return sum;
  }
  double integralAll(const Histogram * h)
  {
  double sum = 0.0;
  for (unsigned int k=0; k<h->nBinsTotal(); k++) sum += h->binValueAtGlobal(k);
  return sum;
  }
  } // anonymous namespace

  //////////////////////////////////////////////////////////////////////////
  // Creation
  //////////////////////////////////////////////////////////////////////////

  bool isValid(const String & name, int n_x, double min_x, double max_x, const String & caller, bool verbose);
  bool isValid(const String & name, int n_x, double min_x, double max_x, int n_y, double min_y, double max_y,
               const String & caller, bool verbose);
  bool isValid(const String & name, int n_x, double min_x, double max_x, int n_y, double min_y, double max_y,
               int n_z, double min_z, double max_z, const String & caller, bool verbose);

  Histogram * createNewHistogram(const String & name,
                                 int n_x, double min_x, double max_x,
                                 const String & title_x, const String & title_y,
                                 int storageOption, bool verbose)
  {
  isValid(name,n_x,min_x,max_x,__FUNCTION__,verbose);
  Histogram::Precision precision = Histogram::precisionFromStorageOption(storageOption);
  Histogram * h = new Histogram(name,name,n_x,min_x,max_x);
  h->setPrecision(precision);
  setTitles(h,title_x,title_y);
  return h;
  }

  Histogram * createNewHistogram(const String & name,
                                 int n_x, double min_x, double max_x,
                                 int n_y, double min_y, double max_y,
                                 const String & title_x, const String & title_y, const String & title_z,
                                 int storageOption, bool verbose)
  {
  isValid(name,n_x,min_x,max_x,n_y,min_y,max_y,__FUNCTION__,verbose);
  Histogram::Precision precision = Histogram::precisionFromStorageOption(storageOption);
  Histogram * h = new Histogram(name,name,n_x,min_x,max_x,n_y,min_y,max_y);
  h->setPrecision(precision);
  setTitles(h,title_x,title_y,title_z);
  return h;
  }

  Histogram * createNewHistogram(const String & name,
                                 int n_x, double min_x, double max_x,
                                 int n_y, double min_y, double max_y,
                                 int n_z, double min_z, double max_z,
                                 const String & title_x, const String & title_y, const String & title_z,
                                 const String & title_w, int storageOption, bool verbose)
  {
  isValid(name,n_x,min_x,max_x,n_y,min_y,max_y,n_z,min_z,max_z,__FUNCTION__,verbose);
  Histogram::Precision precision = Histogram::precisionFromStorageOption(storageOption);
  Histogram * h = new Histogram(name,name,n_x,min_x,max_x,n_y,min_y,max_y,n_z,min_z,max_z);
  h->setPrecision(precision);
  setTitles(h,title_x,title_y,title_z,title_w);
  return h;
  }

  Histogram * createNewProfile(const String & name, int n_x, double min_x, double max_x,
                               const String & title_x, const String & title_y, bool verbose)
  {
  return createNewHistogram(name,n_x,min_x,max_x,title_x,title_y,1,verbose);
  }

  Histogram * createNewProfile(const String & name, int n_x, double min_x, double max_x, int n_y, double min_y, double max_y,
                               const String & title_x, const String & title_y, const String & title_z, bool verbose)
  {
  return createNewHistogram(name,n_x,min_x,max_x,n_y,min_y,max_y,title_x,title_y,title_z,1,verbose);
  }

  void setTitles(Histogram * histogram, const String & title_x, const String & title_y,
                 const String & title_z, const String & title_w)
  {
  if (!histogram) throw NullPointerException(__FUNCTION__);
  // The first rank() titles name the axes; the next one is the title of the value (the histogram title).
  const String * titles[4] = {&title_x,&title_y,&title_z,&title_w};
  unsigned int r = histogram->rank();
  for (unsigned int k=0; k<4; k++)
    {
    if (titles[k]->length()==0) continue;
    if (k<r)       histogram->setDimensionTitle(k,*titles[k]);
    else if (k==r) histogram->setTitle(*titles[k]);
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // Copies, files, checks
  //////////////////////////////////////////////////////////////////////////

  Histogram * clone(const Histogram * h1, const String & histoName)
  {
  if (!h1) throw NullPointerException(__FUNCTION__);
  Histogram * h = h1->clone();
  h->setName(histoName);
  return h;
  }

  Histogram * safeClone(const Histogram * h)
  {
  return h ? h->clone() : nullptr;
  }

  void cloneHistoVector(const std::vector<Histogram*> & source, std::vector<Histogram*> & target)
  {
  for (const Histogram * h : source) target.push_back(safeClone(h));
  }

  Histogram * loadHistogram(const CapFile & inputFile, const String & histoName)
  {
  const Histogram * h = inputFile.get<Histogram>(histoName);
  if (!h) throw FileException(inputFile.path(),String("No histogram named ")+histoName,__FUNCTION__);
  return h->clone();
  }

  int getDimension(const Histogram * h)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  return int(h->rank());
  }

  void printDimensions(const Histogram * h)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  cout << "Histo named:" << h->name();
  const char * axes[3] = {" nx:"," ny:"," nz:"};
  for (unsigned int k=0; k<h->rank() && k<3; k++) cout << axes[k] << h->nBins(k);
  cout << endl;
  }

  void printHistoInfo(const Histogram * h)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  printValue("Name",h->name());
  printValue("Title",h->title());
  printValue("Entries",h->totalEntries());
  printValue("Precision",String(h->precision()==Histogram::kDouble ? "double" : "float"));
  const char * axes[3] = {"X","Y","Z"};
  for (unsigned int k=0; k<h->rank() && k<3; k++)
    {
    printValue((String(axes[k])+" Title").data(),h->dimensionTitle(k));
    printValue((String(axes[k])+" nbins").data(),h->nBins(k));
    printValue((String(axes[k])+" Minimum").data(),h->minimum(k));
    printValue((String(axes[k])+" Maximum").data(),h->maximum(k));
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // Arithmetic
  //////////////////////////////////////////////////////////////////////////

  void sumHistograms(double a1, const Histogram * h1, Histogram * sum)
  {
  if (!sameDimensions(__FUNCTION__,h1,sum)) throw IncompatibleHistogramException(__FUNCTION__);
  sum->add(*h1,a1);
  }

  void sumHistograms(double a1, const Histogram * h1, double a2, const Histogram * h2, Histogram * sum)
  {
  if (!sameDimensions(__FUNCTION__,h1,h2,sum)) throw IncompatibleHistogramException(__FUNCTION__);
  sum->setAsSumOf(*h1,*h2,a1,a2);
  }

  void sumHistograms(double a1, const Histogram * h1, double a2, const Histogram * h2,
                     double a3, const Histogram * h3, Histogram * sum)
  {
  if (!sameDimensions(__FUNCTION__,h1,h2,h3,sum)) throw IncompatibleHistogramException(__FUNCTION__);
  sum->setAsSumOf(std::vector<const Histogram*>{h1,h2,h3},std::vector<double>{a1,a2,a3});
  }

  void sumHistograms(double a1, const Histogram * h1, double a2, const Histogram * h2,
                     double a3, const Histogram * h3, double a4, const Histogram * h4, Histogram * sum)
  {
  if (!sameDimensions(__FUNCTION__,h1,h2,h3,h4,sum)) throw IncompatibleHistogramException(__FUNCTION__);
  sum->setAsSumOf(std::vector<const Histogram*>{h1,h2,h3,h4},std::vector<double>{a1,a2,a3,a4});
  }

  void sumHistograms(const std::vector<double> & a, const std::vector<const Histogram*> & histograms, Histogram * sum)
  {
  if (a.size()!=histograms.size()) throw IncompatibleHistogramException(__FUNCTION__);
  for (unsigned int k=0; k<histograms.size(); k++)
    {
    if (!sameDimensions(__FUNCTION__,histograms[k],sum)) throw IncompatibleHistogramException(__FUNCTION__);
    sum->add(*histograms[k],a[k]);
    }
  }

  void divideHistograms(double a1, const Histogram * h1, double a2, const Histogram * h2, Histogram * ratio)
  {
  if (!sameDimensions(__FUNCTION__,h1,h2,ratio)) throw IncompatibleHistogramException(__FUNCTION__);
  ratio->setAsRatioOf(*h1,*h2,a1,a2);
  }

  void differenceHistograms(const Histogram * h1, const Histogram * h2, Histogram * difference)
  {
  if (!sameDimensions(__FUNCTION__,h1,h2,difference)) throw IncompatibleHistogramException(__FUNCTION__);
  difference->setAsSumOf(*h1,*h2,1.0,-1.0);
  }

  void squareDifferenceHistos(Histogram * h1, const Histogram * h2, double sumWeights, double weight, int n)
  {
  if (!ptrExist(__FUNCTION__,h1,h2)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,h1,h2)) throw IncompatibleHistogramException(__FUNCTION__);
  double sum   = weight + sumWeights;
  double rwn   = weight/sum;
  double rw    = sumWeights/sum;
  double sqrtN = std::sqrt(double(std::abs(n)));
  for (unsigned int k=0; k<h1->nBinsTotal(); k++)
    {
    double vAvg    = h1->binValueAtGlobal(k);
    double evAvg   = h1->binUncertaintyAtGlobal(k);
    double v       = h2->binValueAtGlobal(k);
    double dv      = v - vAvg;
    double evSqAvg = rw*(evAvg*evAvg + dv*dv*rwn);
    vAvg  += rwn*dv;
    evAvg  = n>0 ? std::sqrt(evSqAvg)/sqrtN : std::sqrt(evSqAvg);
    h1->setBinValueAtGlobal(k,vAvg);
    h1->setBinUncertaintyAtGlobal(k,evAvg);
    }
  }

  void differenceHistos(const Histogram * h, const Histogram * hRef, Histogram * hDiff, bool correlatedUncertainties)
  {
  if (!h || !hRef || !hDiff) return;
  if (!sameDimensions(__FUNCTION__,h,hRef,hDiff)) throw IncompatibleHistogramException(__FUNCTION__);
  for (unsigned int k=0; k<h->nBinsTotal(); k++)
    {
    double v     = h->binValueAtGlobal(k);
    double ev    = h->binUncertaintyAtGlobal(k);
    double vRef  = hRef->binValueAtGlobal(k);
    double evRef = hRef->binUncertaintyAtGlobal(k);
    double evDiffSq = correlatedUncertainties ? std::fabs(ev*ev - evRef*evRef) : ev*ev + evRef*evRef;
    hDiff->setBinValueAtGlobal(k,v - vRef);
    hDiff->setBinUncertaintyAtGlobal(k,std::sqrt(evDiffSq));
    }
  }

  void ratioHistos(const Histogram * h, const Histogram * hRef, Histogram * hRatio, bool correlatedUncertainties)
  {
  if (!h || !hRef || !hRatio) return;
  if (!sameDimensions(__FUNCTION__,h,hRef,hRatio)) throw IncompatibleHistogramException(__FUNCTION__);
  for (unsigned int k=0; k<h->nBinsTotal(); k++)
    {
    double v      = h->binValueAtGlobal(k);
    double ev     = h->binUncertaintyAtGlobal(k);
    double rev    = (v!=0) ? ev/v : 0.0;
    double vRef   = hRef->binValueAtGlobal(k);
    double evRef  = hRef->binUncertaintyAtGlobal(k);
    double revRef = (vRef!=0) ? evRef/vRef : 0.0;
    if (vRef!=0)
      {
      double vRatio     = v/vRef;
      double revRatioSq = correlatedUncertainties ? std::fabs(rev*rev - revRef*revRef) : rev*rev + revRef*revRef;
      hRatio->setBinValueAtGlobal(k,vRatio);
      hRatio->setBinUncertaintyAtGlobal(k,std::sqrt(revRatioSq)*vRatio);
      }                                   // vRef==0: the bin of hRatio is left unchanged (as in the ROOT version)
    }
  }

  void setHistogram(Histogram * h, double v, double ev)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  for (unsigned int k=0; k<h->nBinsTotal(); k++)
    {
    h->setBinValueAtGlobal(k,v);
    h->setBinUncertaintyAtGlobal(k,ev);
    }
  }

  void scaleByBinWidth1D(Histogram * h, double scale)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  for (int i=1; i<=nBinsX(h); i++)
    {
    double w = scale/binWidthX(h,i);
    setBinContent(h,i,binContent(h,i)*w);
    setBinError(h,i,binError(h,i)*w);
    }
  }

  void scaleByBinWidth2D(Histogram * h, double scale)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  for (int i=1; i<=nBinsX(h); i++)
    for (int j=1; j<=nBinsY(h); j++)
      {
      double w = scale/(binWidthX(h,i)*binWidthY(h,j));
      setBinContent(h,i,j,binContent(h,i,j)*w);
      setBinError(h,i,j,binError(h,i,j)*w);
      }
  }

  void scaleByBinWidth3D(Histogram * h, double scale)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  for (int i=1; i<=nBinsX(h); i++)
    for (int j=1; j<=nBinsY(h); j++)
      for (int l=1; l<=nBinsZ(h); l++)
        {
        double w = scale/(binWidthX(h,i)*binWidthY(h,j)*binWidthZ(h,l));
        setBinContent(h,i,j,l,binContent(h,i,j,l)*w);
        setBinError(h,i,j,l,binError(h,i,j,l)*w);
        }
  }

  void scaleByBinWidth(Histogram * h, double scale)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  switch (h->rank())
    {
    case 1: scaleByBinWidth1D(h,scale); break;
    case 2: scaleByBinWidth2D(h,scale); break;
    case 3: scaleByBinWidth3D(h,scale); break;
    default: throw HistogramException(h->name(),"scaleByBinWidth() supports ranks 1 to 3",__FUNCTION__);
    }
  }

  double findHistoMinimum(const Histogram * h)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  return h->binValueAtGlobal(h->binWithMinValue());
  }

  double findHistoMaximum(const Histogram * h)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  return h->binValueAtGlobal(h->binWithMaxValue());
  }

  double findHistoMinimum2D(const Histogram * h) { return findHistoMinimum(h); }
  double findHistoMaximum2D(const Histogram * h) { return findHistoMaximum(h); }

  //////////////////////////////////////////////////////////////////////////
  // Integrals and averages
  //////////////////////////////////////////////////////////////////////////

  void calculateIntegral(const Histogram * h, double xMin, double xMax, double & result, double & resultError, int option)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  int xFirst = findBinX(h,xMin);
  int xLast  = findBinX(h,xMax);
  switch (option)
    {
    case 0: result = integralAndError(h,xFirst,xLast,resultError,false); break;
    case 1: result = integralAndError(h,xFirst,xLast,resultError,true);  break;
    }
  }

  void calculateIntegral(const Histogram * h, double xMin, double xMax, double yMin, double yMax,
                         double & result, double & resultError, int option)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  int xFirst = findBinX(h,xMin);
  int xLast  = findBinX(h,xMax);
  int yFirst = findBinY(h,yMin);
  int yLast  = findBinY(h,yMax);
  switch (option)
    {
    case 0: result = integralAndError(h,xFirst,xLast,yFirst,yLast,resultError,false); break;
    case 1: result = integralAndError(h,xFirst,xLast,yFirst,yLast,resultError,true);  break;
    }
  }

  DataPointsVector calculateCumulativeIntegral(const Histogram & h, double xLow, double xHigh, double xStep)
  {
  DataPointsVector points;
  if (!(xStep>0)) throw HistogramException(h.name(),"xStep must be positive",__FUNCTION__);
  int ixLow = findBinX(&h,xLow);
  for (double x=xLow; x<xHigh; x+=xStep)
    {
    double esum = 0.0;
    double sum  = integralAndError(&h,ixLow,findBinX(&h,x),esum,true);
    points.addPoint(DataPoint2D(x,0.001,sum,esum,0.0));
    }
  return points;
  }

  void calculateRmsWidth(const Histogram * h, double xLowEdge, double xHighEdge,
                         double & mean, double & meanError, double & rmsWidth, double & rmsWidthError)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  int n = nBinsX(h);
  int xLowBin  = findBinX(h,xLowEdge);
  int xHighBin = findBinX(h,xHighEdge);
  if (xLowBin<1)  xLowBin  = 1;
  if (xHighBin>n) xHighBin = n;
  double sum = 0.0, xSum = 0.0, x2Sum = 0.0;
  for (int iX=xLowBin; iX<xHighBin; iX++)     // (the ROOT version had a stray "break" here and always failed)
    {
    double x = binCenterX(h,iX);
    double v = binContent(h,iX);
    sum   += v;
    xSum  += x*v;
    x2Sum += x*x*v;
    }
  if (sum<=0.0) throw Exception("null sum","calculateRmsWidth");
  mean          = xSum/sum;
  meanError     = 0;
  rmsWidth      = std::sqrt(x2Sum/sum - mean*mean);
  rmsWidthError = 0.0;
  }

  void partitionHistogram(const std::vector<double> & fractions, std::vector<double> & boundaries,
                          const Histogram * histogram, double minimum, double maximum)
  {
  if (!histogram) throw NullPointerException(__FUNCTION__);
  Histogram normalized(*histogram);
  int first = 1;
  int last  = nBinsX(&normalized);
  if (minimum < maximum)
    {
    first = findBinX(&normalized,minimum);
    last  = findBinX(&normalized,maximum);
    }
  double sum = 0.0;
  for (int k=first; k<=last; k++) sum += binContent(&normalized,k);
  if (sum<=0.0) throw Exception("<E> partitionHistograms(...) sum<=0.0",__FUNCTION__);
  normalized.scale(1.0/sum);
  sum = binContent(&normalized,first);
  double esum2 = binError(&normalized,first)*binError(&normalized,first);
  for (int k=first+1; k<=last; k++)
    {
    double ek = binError(&normalized,k);
    sum   += binContent(&normalized,k);
    esum2 += ek*ek;
    setBinContent(&normalized,k,sum);
    setBinError(&normalized,k,std::sqrt(esum2));
    }
  for (double fraction : fractions)
    for (int k=first; k<=last; k++)
      if (binContent(&normalized,k)>=fraction) { boundaries.push_back(binLowEdgeX(&normalized,k)); break; }
  }

  //////////////////////////////////////////////////////////////////////////
  // Profiles (histograms filled with fill(x,value): the profile value is the bin average)
  //////////////////////////////////////////////////////////////////////////

  void calculateR2VsM(const Histogram * h1, const Histogram * h2, const Histogram * h12,
                      Histogram * r2VsM, Histogram * intR2, bool sameFilter)
  {
  if (!ptrExist(__FUNCTION__,h1,h2,h12,r2VsM,intR2)) throw NullPointerException(__FUNCTION__);
  int n1 = nBinsX(h1);
  int n2 = sameFilter ? n1 : nBinsX(h2);
  if (n1!=n2 || n1!=nBinsX(h12) || n1!=nBinsX(r2VsM) || nBinsX(intR2)!=1) throw IncompatibleHistogramException(__FUNCTION__);
  double nSum = 0, vSum = 0;
  for (int i=0; i<n1; ++i)
    {
    double n    = h1->binEntriesAtGlobal(i);
    double v1   = h1->binAvgValueAtGlobal(i);
    double v2   = sameFilter ? v1 : h2->binAvgValueAtGlobal(i);
    double v12  = h12->binAvgValueAtGlobal(i);
    double ev12 = h12->binAvgValueUncertaintyAtGlobal(i);
    double r = 1, er = 0;
    if (v1>0 && v2>0 && v12>0)
      {
      r  = v12/(v1*v2);
      er = r*ev12/v12; // under estimate errors.
      nSum += n;
      vSum += n*r;
      }
    setBinContent(r2VsM,i+1,r);
    setBinError(r2VsM,i+1,er);
    }
  setBinContent(intR2,1,nSum>0 ? vSum/nSum : 0.0);
  setBinError(intR2,1,0.0);
  }

  void calculateBinCorr(const Histogram * h1, const Histogram * h2, Histogram * intBinCorrVsM1, bool sameFilter)
  {
  if (!ptrExist(__FUNCTION__,h1,h2,intBinCorrVsM1)) throw NullPointerException(__FUNCTION__);
  int n1 = nBinsX(h1);
  int n2 = sameFilter ? n1 : nBinsX(h2);
  if (n1!=n2 || nBinsX(intBinCorrVsM1)!=1) throw IncompatibleHistogramException(__FUNCTION__);
  double nSum = 0, sum1 = 0, sum2 = 0, sum12 = 0;
  for (int i=0; i<n1; ++i)
    {
    double n  = h1->binEntriesAtGlobal(i);
    double v1 = h1->binAvgValueAtGlobal(i);
    double v2 = sameFilter ? v1 : h2->binAvgValueAtGlobal(i);
    if (n>0) { nSum += n; sum1 += n*v1; sum2 += n*v2; sum12 += n*v1*v2; }
    }
  if (nSum>0 && sum1>0 && sum2>0)
    {
    sum1 /= nSum; sum2 /= nSum; sum12 /= nSum;
    setBinContent(intBinCorrVsM1,1,sum12/(sum1*sum2));
    }
  else
    setBinContent(intBinCorrVsM1,1,1.); //no stat - so set correction factor to unity
  setBinError(intBinCorrVsM1,1,0.);
  }

  void calculateInclusiveAvgFromProfiles(const Histogram * h1, const Histogram * h2, Histogram * h3)
  {
  if (!ptrExist(__FUNCTION__,h1,h2,h3)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,h1,h2,h3)) throw IncompatibleHistogramException(__FUNCTION__);
  for (unsigned int k=0; k<h1->nBinsTotal(); k++)
    {
    double v1  = h1->binAvgValueAtGlobal(k), ev1 = h1->binAvgValueUncertaintyAtGlobal(k);
    double v2  = h2->binAvgValueAtGlobal(k), ev2 = h2->binAvgValueUncertaintyAtGlobal(k);
    double v = 0, ev = 0;
    if (v2>0 && v1!=0)
      {
      v = v1/v2;
      double re1 = ev1/v1, re2 = ev2/v2;
      ev = v*std::sqrt(re1*re1+re2*re2);
      }
    h3->setBinValueAtGlobal(k,v);
    h3->setBinUncertaintyAtGlobal(k,std::fabs(ev));
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // Correlation functions and other calculations
  //////////////////////////////////////////////////////////////////////////

  double calculateN1N1N1(const Histogram * h_1, const Histogram * h_2, const Histogram * h_3, Histogram * h_123)
  {
  if (!ptrExist(__FUNCTION__,h_1,h_2,h_3,h_123)) throw NullPointerException(__FUNCTION__);
  int n = nBinsX(h_1);
  bool packed = h_123->rank()==1;   // rank 1: bins (i1,i2,i3) packed as i1*n*n + i2*n + i3; rank 3: one bin per (i1,i2,i3)
  double sum = 0.;
  int i4 = 1;
  for (int i1=1;i1<=n;++i1)
    {
    double v1 = binContent(h_1,i1), ev1 = binError(h_1,i1);
    for (int i2=1;i2<=n;++i2)
      {
      double v2 = binContent(h_2,i2), ev2 = binError(h_2,i2);
      for (int i3=1;i3<=n;++i3)
        {
        double v3 = binContent(h_3,i3), ev3 = binError(h_3,i3);
        double v = v1*v2*v3, ev = 0.;
        if (v>0)
          {
          double r1 = ev1/v1, r2 = ev2/v2, r3 = ev3/v3;
          ev = v*std::sqrt(r1*r1+r2*r2+r3*r3);
          }
        if (packed) { setBinContent(h_123,i4,v); setBinError(h_123,i4,ev); }
        else        { setBinContent(h_123,i1,i2,i3,v); setBinError(h_123,i1,i2,i3,ev); }
        sum += v;
        ++i4;
        }
      }
    }
  //return average across bins
  return sum/n/n/n;
  }

  void calculateBf(const Histogram *n2, const Histogram *n1_1, const Histogram *n1_2, Histogram *bf_12, Histogram *bf_21)
  {
  if (!ptrExist(__FUNCTION__,n2,n1_1,n1_2,bf_12,bf_21)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n1_1,n1_2))  throw IncompatibleHistogramException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n2,bf_12,bf_21))  throw IncompatibleHistogramException(__FUNCTION__);
  int nbinsx = nBinsX(n2);
  int nbinsy = nBinsY(n2);
  bf_12->reset();
  bf_21->reset();
  double n11_inte = 0.0;
  double n11_int  = integralAndError(n1_1,1,nBinsX(n1_1),1,nBinsY(n1_1),n11_inte,false);
  double n11_inter = n11_inte/n11_int;
  double n12_inte = 0.0;
  double n12_int  = integralAndError(n1_2,1,nBinsX(n1_2),1,nBinsY(n1_2),n12_inte,false);
  double n12_inter = n12_inte/n12_int;
  for (int ix = 0; ix < nbinsx; ++ix)
    {
    for (int iy = 0; iy < nbinsy; ++iy)
      {
      double v    = binContent(n2,ix+1,iy+1);
      double ve   = binError(n2,ix+1,iy+1);
      double v12  = v/n12_int;
      double v12e = v12*std::sqrt(ve/v*ve/v+n12_inter*n12_inter);
      double v21  = v/n11_int;
      double v21e = v21*std::sqrt(ve/v*ve/v+n11_inter*n11_inter);
      setBinContent(bf_12,ix+1,iy+1,v12);
      setBinError(bf_12,ix+1,iy+1,v12e);
      setBinContent(bf_21,iy+1,ix+1,v21);
      setBinError(bf_21,iy+1,ix+1,v21e);
      }
    }
  }

  void calculateN1N1H2H2_Q3D_MCY(const Histogram * n1_1, const Histogram * n1_2, Histogram * n1n1_Q3D, double /*a1*/, double /*a2*/)
  {
  if (!ptrExist(__FUNCTION__,n1_1,n1_2,n1n1_Q3D)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n1_1,n1_2))  throw IncompatibleHistogramException(__FUNCTION__);
  double Qlong, Qout, Qside, pt1, phi1, y1, pt2, phi2, y2;
  int nIter = 100000000;
  double scalingFactor = integralAll(n1_1)*integralAll(n1_2)/double(nIter);
  for (int k=0;k<nIter;k++)
    {
    getRandom2(n1_1,y1,pt1);
    getRandom2(n1_2,y2,pt2);
    phi1 = 2.0*pi()*uniform01();
    phi2 = 2.0*pi()*uniform01();
    if (calculateQ3DwPtPhiY(pt1,phi1,y1,pt2,phi2,y2,Qlong,Qout,Qside)==0) n1n1_Q3D->fill(Qlong,Qside,Qout,1.0);
    }
  n1n1_Q3D->scale(scalingFactor);
  }

  void calculateN1N1H2H2_Q3D_MCEta(const Histogram * n1_1, const Histogram * n1_2, Histogram * n1n1_Q3D, double /*a1*/, double /*a2*/)
  {
  if (!ptrExist(__FUNCTION__,n1_1,n1_2,n1n1_Q3D)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n1_1,n1_2))  throw IncompatibleHistogramException(__FUNCTION__);
  double Qlong, Qout, Qside, pt1, phi1, eta1, pt2, phi2, eta2;
  int nIter = 100000000;
  double scalingFactor = integralAll(n1_1)*integralAll(n1_2)/double(nIter);
  for (int k=0;k<nIter;k++)
    {
    getRandom2(n1_1,eta1,pt1);
    getRandom2(n1_2,eta2,pt2);
    phi1 = 2.0*pi()*uniform01();
    phi2 = 2.0*pi()*uniform01();
    if (calculateQ3DwPtPhiEta(pt1,phi1,eta1,pt2,phi2,eta2,Qlong,Qout,Qside)==0) n1n1_Q3D->fill(Qlong,Qside,Qout,1.0);
    }
  n1n1_Q3D->scale(scalingFactor);
  }

  void calculateN1N1H2H2_Q3D(const Histogram * n1_1, const Histogram * n1_2, Histogram * n1n1_Q3D, double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,n1_1,n1_2,n1n1_Q3D)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n1_1,n1_2))  throw IncompatibleHistogramException(__FUNCTION__);
  int nx = nBinsX(n1_1);
  int ny = nBinsY(n1_1);
  double Qlong, Qout, Qside, phi1, phi2;
  for (int i1x=1;i1x<=nx;++i1x)
    for (int i1y=1;i1y<=ny;++i1y)
      for (int i2x=1;i2x<=nx;++i2x)
        for (int i2y=1;i2y<=ny;++i2y)
          {
          double v1 = a1*binContent(n1_1,i1x,i1y);
          double v2 = a2*binContent(n1_2,i2x,i2y);
          if (v1>0 && v2>0)
            {
            double v3   = v1*v2;
            double eta1 = binCenterX(n1_1,i1x), pt1 = binCenterY(n1_1,i1y);   // (the ROOT version read pt from the Z axis)
            double eta2 = binCenterX(n1_2,i2x), pt2 = binCenterY(n1_2,i2y);
            for (int k=0;k<100000;k++)
              {
              phi1 = 2.0*pi()*uniform01();
              phi2 = 2.0*pi()*uniform01();
              if (calculateQ3DwPtPhiEta(pt1,phi1,eta1,pt2,phi2,eta2,Qlong,Qout,Qside)==0) n1n1_Q3D->fill(Qlong,Qside,Qout,v3);
              }
            }
          }
  }

  void calculateN1N1H3H3_Q3D(const Histogram * n1_1, const Histogram * n1_2, Histogram * n1n1_Q3D, double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,n1_1,n1_2,n1n1_Q3D)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n1_1,n1_2))  throw IncompatibleHistogramException(__FUNCTION__);
  int nx = nBinsX(n1_1);
  int ny = nBinsY(n1_1);
  int nz = nBinsZ(n1_1);   // (the ROOT version used the number of x bins)
  double Qlong, Qout, Qside;
  for (int i1x=1;i1x<=nx;++i1x)
    for (int i1y=1;i1y<=ny;++i1y)
      for (int i1z=1;i1z<=nz;++i1z)
        for (int i2x=1;i2x<=nx;++i2x)
          for (int i2y=1;i2y<=ny;++i2y)
            for (int i2z=1;i2z<=nz;++i2z)
              {
              double v1 = a1*binContent(n1_1,i1x,i1y,i1z);
              double v2 = a2*binContent(n1_2,i2x,i2y,i2z);
              if (v1>0 && v2>0)
                {
                double eta1 = binCenterX(n1_1,i1x), phi1 = binCenterY(n1_1,i1y), pt1 = binCenterZ(n1_1,i1z);
                double eta2 = binCenterX(n1_2,i2x), phi2 = binCenterY(n1_2,i2y), pt2 = binCenterZ(n1_2,i2z);
                if (calculateQ3DwPtPhiEta(pt1,phi1,eta1,pt2,phi2,eta2,Qlong,Qout,Qside)==0) n1n1_Q3D->fill(Qlong,Qside,Qout,v1*v2);
                }
              }
  }

  // ported from RootHistogramHelpers.cpp line 19
  bool isValid(const String & name,int n_x, double min_x, double max_x, const String & caller, bool verbose)
  {
  if (verbose) printHistoInfo(caller,name,n_x,min_x,max_x);
  if (n_x<1)
    throw HistogramException(name,"Invalid Histo Specs: n_x<1",caller);
  if (min_x>=max_x)
    throw HistogramException(name,"Invalid Histo Specs: min_x>=max_x",caller);
  return true;
  }

  // ported from RootHistogramHelpers.cpp line 29
  bool isValid(const String & name,
               int n_x, double min_x, double max_x,
               int n_y, double min_y, double max_y,
               const String & caller, bool verbose)
  {
  if (verbose) printHistoInfo(caller,name,n_x,min_x,max_x,n_y,min_y,max_y);
  if (n_x<1)
    throw HistogramException(name,"Invalid Histo Specs: n_x<1",caller);
  if (n_y<1)
    throw HistogramException(name,"Invalid Histo Specs: n_y<1",caller);
  if (min_x>=max_x)
    throw HistogramException(name,"Invalid Histo Specs: min_x>=max_x",caller);
  if (min_y>=max_y)
    throw HistogramException(name,"Invalid Histo Specs: min_y>=max_y",caller);
  return true;
  }

  // ported from RootHistogramHelpers.cpp line 46
  bool isValid(const String & name,
               int n_x, double min_x, double max_x,
               int n_y, double min_y, double max_y,
               int n_z, double min_z, double max_z,
               const String & caller, bool verbose)
  {
  if (verbose) printHistoInfo(caller,name,n_x,min_x,max_x,n_y,min_y,max_y,n_z,min_z,max_z);
  if (n_x<1)
    throw HistogramException(name,"Invalid Histo Specs: n_x<1",caller);
  if (n_y<1)
    throw HistogramException(name,"Invalid Histo Specs: n_y<1",caller);
  if (n_z<1)
    throw HistogramException(name,"Invalid Histo Specs: n_z<1",caller);
  if (min_x>=max_x)
    throw HistogramException(name,"Invalid Histo Specs: min_x>=max_x",caller);
  if (min_y>=max_y)
    throw HistogramException(name,"Invalid Histo Specs: min_y>=max_y",caller);
  if (min_z>=max_z)
    throw HistogramException(name,"Invalid Histo Specs: min_z>=max_z",caller);
  return true;
  }

  // ported from RootHistogramHelpers.cpp line 366
  //! Calculate External Product h_1 x h_2 and store into n1n1_12
  //! Verify if dimensions are compatible, aborts if they are not.
  //! The output is a 1D histogram packed according to the size of h_1 and h_2.
  double calculateN1N1(const Histogram * h_1, const Histogram * h_2, Histogram * h_12, double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,h_1,h_2,h_12)) throw NullPointerException(__FUNCTION__);
  int n1  = nBinsX(h_1);
  int n2  = nBinsX(h_2);
  int n3  = nBinsX(h_12);
  if (n3 != n1*n2 )throw HistogramException("3Histos","Incompatible histo dimensions",__FUNCTION__);
  double v1,ev1,v2,ev2,v,ev, r1,r2;
  double sum = 0.;
  double norm = 0.;
  int i3 = 1;
  for (int i1=1; i1<=n1;++i1)
    {
    v1  = a1*binContent(h_1,i1);
    ev1 = a1*binError(h_1,i1);
    for (int i2=1;i2<=n2;++i2)
      {
      v2  = a2*binContent(h_2,i2);
      ev2 = a2*binError(h_2,i2);
      v = v1*v2;
      if (v>0)
        {
        r1 = ev1/v1; r2 = ev2/v2;
        ev = v*std::sqrt(r1*r1+r2*r2);
        }
      else
        ev = 0.;
      setBinContent(h_12,i3,v);
      setBinError(h_12,i3,ev);
      sum  += v;
      norm += 1.;
      ++i3;
      }
    }
  //return average across bins
  return sum/norm;
  }

  // ported from RootHistogramHelpers.cpp line 407
  //Calculate External Product n1_1 x n1_2 and store into n1n1_12
  double calculateN1N1_H1H1H2(const Histogram * h_1, const Histogram * h_2, Histogram * h_12, double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,h_1,h_2,h_12)) throw NullPointerException(__FUNCTION__);
  int n1   = nBinsX(h_1);
  int n2   = nBinsX(h_2);
  int n3x  = nBinsX(h_12);
  int n3y  = nBinsY(h_12);

  if (n3x!=n1 || n3y!=n2) throw HistogramException("3Histos","Incompatible histo dimensions",__FUNCTION__);
  double v1,ev1,v2,ev2,v,ev, r1,r2;
  double sum  = 0.;
  double norm = 0.;
  for (int i1=1; i1<=n1; ++i1)
    {
    v1  = a1*binContent(h_1,i1);
    ev1 = a1*binError(h_1,i1);
    for (int i2=1; i2<=n2; ++i2)
      {
      v2  = a2*binContent(h_2,i2);
      ev2 = a2*binError(h_2,i2);
      v = v1*v2;
      if (v>0)
        {
        r1 = ev1/v1;
        r2 = ev2/v2;
        ev = v*std::sqrt(r1*r1+r2*r2);
        }
      else
        ev = 0.;
      setBinContent(h_12,i1,i2,v);
      setBinError(h_12,i1,i2,ev);
      sum  += v;
      norm += 1.;
      }
    }
  //return average across bins
  return sum/norm;
  }

  // ported from RootHistogramHelpers.cpp line 447
  //Calculate External Product n1_1 x n1_2 and store into n1n1_12
  double calculateN1N1_H2H2H2(const Histogram *h_1, const Histogram * h_2, Histogram * h_12, double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,h_1,h_2,h_12)) throw NullPointerException(__FUNCTION__);
  int n1x = nBinsX(h_1);
  int n1y = nBinsY(h_1);
  int n2x = nBinsX(h_2);
  int n2y = nBinsY(h_2);
  int n3x = nBinsX(h_12);
  int n3y = nBinsY(h_12);
  if (n3x!=(n1x*n1y) || n3y!=(n2x*n2y) ) throw HistogramException("3Histos","Incompatible histo dimensions",__FUNCTION__);
  double v1,ev1,v2,ev2,v,ev, r1,r2;
  double sum  = 0.;
  double norm = 0.;
  for (int i1x=0;i1x<n1x;++i1x)
    {
    for (int i1y=0;i1y<n1y;++i1y)
      {
      v1  = a1*binContent(h_1,i1x+1,i1y+1);
      ev1 = a1*binError(h_1,i1x+1,i1y+1);
      for (int i2x=0;i2x<n2x;++i2x)
        {
        for (int i2y=0;i2y<n2y;++i2y)
          {
          v2  = a2*binContent(h_2,i2x+1,i2y+1);
          ev2 = a2*binError(h_2,i2x+1,i2y+1);
          v = v1*v2;
          if (v>0)
            {
            r1 = ev1/v1;
            r2 = ev2/v2;
            ev = v*std::sqrt(r1*r1+r2*r2);
            }
          else
            ev = 0.;
          int i3x = i1x*n1y+i1y+1;
          int i3y = i2x*n2y+i2y+1;
          setBinContent(h_12,i3x,i3y,v);
          setBinError(h_12,i3x,i3y,ev);
          sum += v;
          norm = 1.;
          }
        }
      }
    }
  //return average across bins
  return sum/norm;
  }

  // ported from RootHistogramHelpers.cpp line 577
  void calculateN2N1(const Histogram * s2, const Histogram* s1, Histogram * target, int single)
  {
  if (!ptrExist(__FUNCTION__,s2,s1,target)) throw NullPointerException(__FUNCTION__);
  int n = nBinsX(s2);
  double v1,v2,v3,ev1,ev2,ev3,v,ev,r1,r2;
  int dPhi_xy;
  int dPhi_xz;
  int dPhi_yz;
  int dPhi12;
  int dPhi13;
  for (int i_x=0;i_x<n; ++i_x)
    {
    int i_x1 = i_x+1;
    for (int i_y=0;i_y<n; ++i_y)
      {
      int i_y1 = i_y+1;
      dPhi_xy = i_x-i_y; if (dPhi_xy<0) dPhi_xy += n; dPhi_xy+=1;
      for (int iz=0;iz<n; ++iz)
        {
        int iz1 = iz+1;
        dPhi_xz = i_x-iz; if (dPhi_xz<0) dPhi_xz += n; dPhi_xz+=1;
        dPhi_yz = i_y-iz; if (dPhi_yz<0) dPhi_yz += n; dPhi_yz+=1;
        v1   = binContent(s2,i_x1,i_y1);
        ev1  = binError(s2,i_x1,i_y1);
        v2   = binContent(s1,iz1);
        ev2  = binError(s1,iz1);
        switch (single)
          {
            case 3: dPhi12 = dPhi_xy; dPhi13 = dPhi_xz; break;
            case 2: dPhi13 = dPhi_xy; dPhi12 = dPhi_xz; break;
            case 1: dPhi12 = dPhi_xz; dPhi13 = dPhi_yz; break;
            default: return;//error("calculateN2N1()") << "invalid argument"<< endl; return;
          }
        v3   = binContent(target,dPhi12,dPhi13);
        ev3  = binError(target,dPhi12,dPhi13);
        v = v1*v2;
        if (v>0)
          {
          r1 = ev1/v1;
          r2 = ev2/v2;
          ev = v*v*(r1*r1+r2*r2);
          }
        else
          ev = 0.;
        v = v+v3;
        ev = std::sqrt(ev3*ev3+ev);
        setBinContent(target,dPhi12,dPhi13,v);
        setBinError(target,dPhi12,dPhi13,ev);
        }
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 633
  //!
  //! Fold s2 and s1 into target
  //!
  void calculateN2N1x(const Histogram * s2, const Histogram* s1, Histogram * target, int single)
  {
  if (!ptrExist(__FUNCTION__,s2,s1,target)) throw NullPointerException(__FUNCTION__);
  int n = nBinsX(s2);
  double v1,v2,v3,ev1,ev2,ev3,v,ev,r1,r2;
  for (int i_x=0;i_x<n; ++i_x)
    {
    int i_x1 = i_x+1;
    for (int i_y=0;i_y<n; ++i_y)
      {
      int i_y1 = i_y+1;
      for (int iz=0;iz<n; ++iz)
        {
        int iz1 = iz+1;
        v1   = binContent(s2,i_x1,i_y1);
        ev1  = binError(s2,i_x1,i_y1);
        v2   = binContent(s1,iz1);
        ev2  = binError(s1,iz1);

        v = v1*v2;
        if (v>0)
          {
          r1 = ev1/v1;
          r2 = ev2/v2;
          ev = v*v*(r1*r1+r2*r2);
          }
        else
          ev = 0.;

        switch (single)
          {
            case 3:
            v3   = binContent(target,i_x1,i_y1,iz1);
            ev3  = binError(target,i_x1,i_y1,iz1);
            break;
            case 2:
            v3   = binContent(target,i_x1,iz1,i_y1);
            ev3  = binError(target,i_x1,iz1,i_y1);
            break;
            case 1:
            v3   = binContent(target,iz1,i_x1,i_y1);
            ev3  = binError(target,iz1,i_x1,i_y1);
            break;
            default:  throw Exception("Invalid Option",__FUNCTION__);
            return;
          }

        v = v+v3;
        ev = std::sqrt(ev3*ev3+ev);
        switch (single)
          {
            case 3:
            setBinContent(target,i_x1,i_y1,iz1,v);
            setBinError(target,i_x1,i_y1,iz1,ev);
            break;
            case 2:
            setBinContent(target,i_x1,iz1,i_y1,v);
            setBinError(target,i_x1,iz1,i_y1,ev);
            break;
            case 1:
            setBinContent(target,iz1,i_x1,i_y1,v);
            setBinError(target,iz1,i_x1,i_y1,ev);
            break;
            default:
            throw Exception("Invalid argument",__FUNCTION__);
          }
        }
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 754
  //!
  //! Check whether the given histograms have the same dimension and size.
  //!
  bool  sameDimensions(const String & caller, const Histogram* h1, const Histogram* h2)
  {
  if (!ptrExist(caller,h1,h2))  throw NullPointerException(__FUNCTION__);
  int nDim1 =  getDimension(h1);
  int nDim2 =  getDimension(h2);
  if ( nDim1==1  &&  nDim2==1  &&  (nBinsX(h1)==nBinsX(h2))) return true;
  if ( nDim1==2  &&  nDim2==2  &&  (nBinsX(h1)==nBinsX(h2))  &&  (nBinsY(h1)==nBinsY(h2))) return true;
  if ( nDim1==3  &&  nDim2==3  &&  (nBinsX(h1)==nBinsX(h2))  &&  (nBinsY(h1)==nBinsY(h2))  &&  (nBinsZ(h1)==nBinsZ(h2))) return true;
  printDimensions(h1);
  printDimensions(h2);
  return false;
  }

  // ported from RootHistogramHelpers.cpp line 767
  bool  sameDimensions(const String & caller, const Histogram* h1, const Histogram* h2, const Histogram* h3)
  {
  if (!ptrExist(caller,h1,h2)) throw NullPointerException(__FUNCTION__);
  int nDim1 =  getDimension(h1);
  int nDim2 =  getDimension(h2);
  int nDim3 =  getDimension(h3);
  if ( nDim1==1  &&  nDim2==1  &&  nDim3==1
      &&  (nBinsX(h1)==nBinsX(h2))
      &&  (nBinsX(h1)==nBinsX(h3)) ) return true;
  if ( nDim1==2  &&  nDim2==2  &&  nDim3==2
      &&  (nBinsX(h1)==nBinsX(h2))  &&  (nBinsY(h1)==nBinsY(h2))
      &&  (nBinsX(h1)==nBinsX(h3))  &&  (nBinsY(h1)==nBinsY(h3))) return true;
  if ( nDim1==3  &&  nDim2==3  &&  nDim3==3
      &&  (nBinsX(h1)==nBinsX(h2))  &&  (nBinsY(h1)==nBinsY(h2))  &&  (nBinsZ(h1)==nBinsZ(h2))
      &&  (nBinsX(h1)==nBinsX(h3))  &&  (nBinsY(h1)==nBinsY(h3))  &&  (nBinsZ(h1)==nBinsZ(h3))) return true;
  printDimensions(h1);
  printDimensions(h2);
  printDimensions(h3);
  return false;
  }

  // ported from RootHistogramHelpers.cpp line 788
  bool  sameDimensions(const String & caller, const Histogram* h1, const Histogram* h2, const Histogram* h3, const Histogram* h4)
  {
  int nDim1 =  getDimension(h1);
  int nDim2 =  getDimension(h2);
  int nDim3 =  getDimension(h3);
  int nDim4 =  getDimension(h4);
  if ( nDim1==1  &&  nDim2==1  &&  nDim3==1  &&  nDim4==1
      &&  (nBinsX(h1)==nBinsX(h2))
      &&  (nBinsX(h1)==nBinsX(h3))
      &&  (nBinsX(h1)==nBinsX(h4)) ) return true;
  if ( nDim1==2  &&  nDim2==2  &&  nDim3==2  &&  nDim4==2
      &&  (nBinsX(h1)==nBinsX(h2))  &&  (nBinsY(h1)==nBinsY(h2))
      &&  (nBinsX(h1)==nBinsX(h3))  &&  (nBinsY(h1)==nBinsY(h3))
      &&  (nBinsX(h1)==nBinsX(h4))  &&  (nBinsY(h1)==nBinsY(h4)) ) return true;
  if ( nDim1==3  &&  nDim2==3  &&  nDim4==3  &&  nDim4==3
      &&  (nBinsX(h1)==nBinsX(h2))  &&  (nBinsY(h1)==nBinsY(h2))  &&  (nBinsZ(h1)==nBinsZ(h2))
      &&  (nBinsX(h1)==nBinsX(h3))  &&  (nBinsY(h1)==nBinsY(h3))  &&  (nBinsZ(h1)==nBinsZ(h3))
      &&  (nBinsX(h1)==nBinsX(h4))  &&  (nBinsY(h1)==nBinsY(h4))  &&  (nBinsZ(h1)==nBinsZ(h4)) ) return true;
  cout << endl;
  cout << "sameDimensions(...)  : " << caller << endl;
  cout << "Incompatible dimensions or sizes" << endl;
  printDimensions(h1);
  printDimensions(h2);
  printDimensions(h3);
  printDimensions(h4);
  return false;
  }

  // ported from RootHistogramHelpers.cpp line 819
  //!
  //! Check whether the given histograms have the same dimension and size.
  //!
  bool  sameDimensions(const String & caller, const Histogram* h1, const Histogram* h2, const Histogram* h3, const Histogram* h4, const Histogram* h5)
  {
  int nDim1 =  getDimension(h1);
  int nDim2 =  getDimension(h2);
  int nDim3 =  getDimension(h3);
  int nDim4 =  getDimension(h4);
  int nDim5 =  getDimension(h5);
  if ( nDim1==1  &&  nDim2==1  &&  nDim3==1  &&  nDim4==1  &&  nDim5==1
      &&  (nBinsX(h1)==nBinsX(h2))
      &&  (nBinsX(h1)==nBinsX(h3))
      &&  (nBinsX(h1)==nBinsX(h4))
      &&  (nBinsX(h1)==nBinsX(h5)) ) return true;
  if ( nDim1==2  &&  nDim2==2  &&  nDim3==2  &&  nDim4==2  &&  nDim5==2
      &&  (nBinsX(h1)==nBinsX(h2))  &&  (nBinsY(h1)==nBinsY(h2))
      &&  (nBinsX(h1)==nBinsX(h3))  &&  (nBinsY(h1)==nBinsY(h3))
      &&  (nBinsX(h1)==nBinsX(h4))  &&  (nBinsY(h1)==nBinsY(h4))
      &&  (nBinsX(h1)==nBinsX(h5))  &&  (nBinsY(h1)==nBinsY(h5))  ) return true;
  if ( nDim1==3  &&  nDim2==3  &&  nDim4==3  &&  nDim4==3  &&  nDim5==3
      &&  (nBinsX(h1)==nBinsX(h2))  &&  (nBinsY(h1)==nBinsY(h2))  &&  (nBinsZ(h1)==nBinsZ(h2))
      &&  (nBinsX(h1)==nBinsX(h3))  &&  (nBinsY(h1)==nBinsY(h3))  &&  (nBinsZ(h1)==nBinsZ(h3))
      &&  (nBinsX(h1)==nBinsX(h4))  &&  (nBinsY(h1)==nBinsY(h4))  &&  (nBinsZ(h1)==nBinsZ(h4))
      &&  (nBinsX(h1)==nBinsX(h5))  &&  (nBinsY(h1)==nBinsY(h5))  &&  (nBinsZ(h1)==nBinsZ(h5)) ) return true;
  cout << endl;
  cout << "sameDimensions(...)  : " << caller << endl;
  cout << "Incompatible dimensions or sizes" << endl;
  printDimensions(h1);
  printDimensions(h2);
  printDimensions(h3);
  printDimensions(h4);
  printDimensions(h5);
  return false;
  }

  // ported from RootHistogramHelpers.cpp line 856
  //!
  //! Calculate the G2 observable based on the given components
  //!
  void calculateDptDpt(const Histogram * spp, const Histogram * spn, const Histogram * snp, const Histogram * snn,
                       const Histogram * avgp1, const Histogram * avgp2,  Histogram * s2dptdpt,  Histogram * dptdpt,
                       bool ijNormalization, int nEta, int nPhi)
  {
  if (!ptrExist(__FUNCTION__,spp,spn,snp,snn,avgp1,avgp2,s2dptdpt,dptdpt)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,spp,spn,snp,snn))  throw IncompatibleHistogramException(__FUNCTION__);
  int nEtaPhi = nEta*nPhi;
  double sumPt1 = 0;
  double sumPt2 = 0;
  for (int iEta1=0;iEta1<nEta;++iEta1)
    {
    for (int iPhi1=0;iPhi1<nPhi;++iPhi1)
      {
      //k = iEta1*nPhi + iPhi1 + 1;
      sumPt1 += binContent(avgp1,iEta1+1, iPhi1+1);
      sumPt2 += binContent(avgp2,iEta1+1, iPhi1+1);
      }
    }
  double p1 = sumPt1/double(nEtaPhi);
  double p2 = sumPt2/double(nEtaPhi);
  double v1,v2,v3,v4,v5,v6;
  double ev4,ev5,ev6;
  for (int iEta1=0;iEta1<nEta;++iEta1)
    {
    //p1  = ptAvg1[iEta1];
    for (int iPhi1=0;iPhi1<nPhi;++iPhi1)
      {
      int k1 = 1+ iEta1*nPhi + iPhi1;
      //c1 = binContent(avgp1,k1+1)/p1;
      for (int iEta2=0;iEta2<nEta;++iEta2)
        {
        //p2  = ptAvg2[iEta2];
        for (int iPhi2=0;iPhi2<nPhi;++iPhi2)
          {
          int k2 = 1+ iEta2*nPhi + iPhi2;
          v1  = binContent(spp,k1,k2); //ev1 = binError(spp,k1,k2);
          v2  = binContent(spn,k1,k2); //ev2 = binError(spn,k1,k2);
          v3  = binContent(snp,k1,k2); //ev3 = binError(snp,k1,k2);
          v4  = binContent(snn,k1,k2); ev4 = binError(snn,k1,k2);
          //c2 = binContent(avgp2,k2+1)/p2;

          if (v4!=0 ) //  &&  ev4/v4<0.5)
            {
            if (ijNormalization)
              {
              v5  = 2*(v1 - v2*p2 - p1*v3 + p1*p2*v4);
              ev5 = v5*ev4/v4;
              v6  = v5/(2*v4);
              ev6 = v6*ev4/v4;
              }
            else
              {
              v5  = v1 - v2*p2 - p1*v3 + p1*p2*v4;
              ev5 = v5*ev4/v4;
              v6  = v5/v4;
              ev6 = v6*ev4/v4;
              }
            }
          else
            {
            v5 = v6 = ev5 = ev6 = 0;
            }
          // s2dptdpt is not filled (as in the ROOT version, where these calls are commented out)
          //setBinContent(s2dptdpt,k,v5); setBinError(s2dptdpt,k,ev5);
          //setBinContent(dptdpt,k,  v6); setBinError(dptdpt,k,  ev6);
          //setBinContent(s2dptdpt,k1, k2, v5); setBinError(s2dptdpt,k1, k2, ev5);
          setBinContent(dptdpt,  k1, k2, v6); setBinError(dptdpt,  k1, k2, ev6);
          }
        }
      }
    }

  }

  // ported from RootHistogramHelpers.cpp line 934
  //!
  //! Calculate the G2 observable based on the given components
  //!
  void calculateDptDpt(const Histogram * spp, const Histogram * spn, const Histogram * snp, const Histogram * snn,
                       const Histogram * avgp1, const Histogram * avgp2,
                       Histogram * dptdpt,
                       bool ijNormalization, int nBins)
  {
  if (!ptrExist(__FUNCTION__,spp,spn,snp,snn,avgp1,avgp2,dptdpt)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,spp,spn,snp,snn))  throw IncompatibleHistogramException(__FUNCTION__);

  double v1,v2,v3,v4,v5,v6,p1,p2;
  double ev4,ev5,ev6;
  for (int i1=1;i1<=nBins;++i1)
    {
    p1 = binContent(avgp1,i1);
    for (int i2=1;i2<=nBins;++i2)
      {
      p2 = binContent(avgp2,i2);
      v1  = binContent(spp,i1,i2); //ev1 = binError(spp,i1,i2);
      v2  = binContent(spn,i1,i2); //ev2 = binError(spn,i1,i2);
      v3  = binContent(snp,i1,i2); //ev3 = binError(snp,i1,i2);
      v4  = binContent(snn,i1,i2); ev4 = binError(snn,i1,i2);
      if (v4!=0) //  &&  ev4/v4<0.5)
        {
        if (ijNormalization)
          {
          v5  = 2*(v1 - v2*p2 - p1*v3 + p1*p2*v4);
          ev5 = v5*ev4/v4;
          v6  = v5/(2*v4);
          ev6 = v6*ev4/v4;
          }
        else
          {
          v5  = v1 - v2*p2 - p1*v3 + p1*p2*v4;
          ev5 = v5*ev4/v4;
          v6  = v5/v4;
          ev6 = v6*ev4/v4;
          }
        }
      else
        {
        v5 = v6 = ev5 = ev6 = 0;
        }
      setBinContent(dptdpt,i1,i2, v6);
      setBinError(dptdpt,i1,i2, ev6);
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 982
  void calculateSc(const Histogram * spp, const Histogram * n1n1, const Histogram * pt1pt1, Histogram * g2, bool ijNormalization)
  {
  if (!ptrExist(__FUNCTION__,spp,n1n1,pt1pt1,g2)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,spp,n1n1,pt1pt1,g2))  throw IncompatibleHistogramException(__FUNCTION__);
  int n1 = nBinsX(spp);
  double v1,v2,v3,v4;
  double ev1,ev4;
  for (int i1=1;i1<=n1;++i1)
    {
    v1  = binContent(spp,i1);    ev1 = binError(spp,i1);
    v2  = binContent(n1n1,i1);   //ev2 = binError(n1n1,i1);
    v3  = binContent(pt1pt1,i1); //ev3 = binError(pt1pt1,i1);
    if (v2>0)
      {
      if (ijNormalization)
        v4  = 2*v1/v2 - v3;
      else
        v4  = v1/v2 - v3;
      ev4 = v1>0 ? v4*ev1/v1 : 0;
      }
    else
      {
      v4 = ev4 = 0;
      }
    setBinContent(g2,i1,v4); setBinError(g2,i1,ev4);
    }
  }

  // ported from RootHistogramHelpers.cpp line 1010
  void calculateG2_H2H2H2H2(const Histogram * spp, const Histogram * n1n1, const Histogram * pt1pt1, Histogram * g2, bool ijNormalization, double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,spp,n1n1,pt1pt1,g2))  throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,spp,n1n1,pt1pt1,g2))  throw IncompatibleHistogramException(__FUNCTION__);

  int n2x = nBinsX(n1n1);
  int n2y = nBinsY(n1n1);
  double v1,v2,v3,v4;
  double ev1,ev4;
  for (int i1=1;i1<=n2x;++i1)
    {
    for (int i2=1;i2<=n2y;++i2)
      {
      v1  = a1*binContent(spp,i1,i2);       ev1 = a1*binError(spp,i1,i2);
      v2  = a2*binContent(n1n1,i1,i2);      //ev2 = a2*binError(n1n1,i1,i2);
      v3  = binContent(pt1pt1,i1,i2);       //ev3 = binError(pt1pt1,i1,i2);
      if (v2>0)
        {
        if (ijNormalization)
          v4  = 2*v1/v2 - v3;
        else
          v4  = v1/v2 - v3;
        ev4 = v1>0 ? v4*ev1/v1 : 0;
        }
      else
        {
        v4 = ev4 = 0;
        }
      setBinContent(g2,i1,i2,v4); setBinError(g2,i1,i2,ev4);
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 1085
  void calculateG2_H1H2H2H2(const Histogram * spp, const Histogram * n1n1, const Histogram * pt1pt1, Histogram * g2, bool ijNormalization, double a1, double a2)
  {

  if (!ptrExist(__FUNCTION__,spp,n1n1,pt1pt1,g2)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n1n1,pt1pt1,g2)) throw IncompatibleHistogramException(__FUNCTION__);
  int n1  = nBinsX(spp);
  int n2x = nBinsX(n1n1);
  int n2y = nBinsY(n1n1);
  int n3x = nBinsX(pt1pt1);
  // int n3y = nBinsY(pt1pt1);
  int n4x = nBinsX(g2);
  //int n4y = nBinsY(g2);
  if (n1!=n2x*n2y || n2x!=n3x || n2x!=n4x)
    throw IncompatibleHistogramException(__FUNCTION__);
  int index;
  double v1,v2,v3,v4;
  double ev1, ev4;
  index = 1;
  for (int i1=1;i1<=n2x;++i1)
    {
    for (int i2=1;i2<=n2y;++i2)
      {
      v1  = a1*binContent(spp,index);   ev1 = a1*binError(spp,index);
      v2  = a2*binContent(n1n1,i1,i2);  // ev2 = a2*binError(n1n1,i1,i2);
      v3  = binContent(pt1pt1,i1,i2);   // ev3 = binError(pt1pt1,i1,i2);
      if (v2>0)
        {
        if (ijNormalization)
          v4  = 2*v1/v2 - v3;
        else
          v4  = v1/v2 - v3;
        ev4 = v1>0 ? v4*ev1/v1 : 0;
        }
      else
        {
        v4 = ev4 = 0;
        }
      setBinContent(g2,i1,i2,v4); setBinError(g2,i1,i2,ev4);
      index++;
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 1129
  int  calculateQ3DwPtPhiEta(double pt1, double phi1, double eta1,
                             double pt2, double phi2, double eta2,
                             double & Qlong, double & Qout, double & Qside)
  {
  double mPart = 0.13957;
  double mSq   = mPart*mPart;

  double mt1   = std::sqrt(mSq + pt1*pt1);
  double px1   = pt1*std::cos(phi1);
  double py1   = pt1*std::sin(phi1);
  double pz1   = mt1*std::sinh(eta1);
  double e1    = std::sqrt(mSq + pt1*pt1 + pz1*pz1);

  double mt2   = std::sqrt(mSq + pt2*pt2);
  double px2   = pt2*std::cos(phi2);
  double py2   = pt2*std::sin(phi2);
  double pz2   = mt2*std::sinh(eta2); // broken
  double e2    = std::sqrt(mSq + pt2*pt2 + pz2*pz2);

  double pt,s,Mlong,roots;
  double ptot[4],q[4];
  //const int g[4]={1,-1,-1,-1};
  //int alpha;
  s=0.0;
  ptot[0] = e1  + e2;
  ptot[1] = px1 + px2;
  ptot[2] = py1 + py2;
  ptot[3] = pz1 + pz2;
  q[0] = e1  - e2;
  q[1] = px1 - px2;
  q[2] = py1 - py2;
  q[3] = pz1 - pz2;
  s = ptot[0]*ptot[0] - ptot[1]*ptot[1] - ptot[2]*ptot[2] - ptot[3]*ptot[3];
  pt=std::sqrt(ptot[1]*ptot[1]+ptot[2]*ptot[2]);
  Mlong=std::sqrt(s+pt*pt);
  roots=std::sqrt(s);

  //if ((p1+p2).P()>(p1+p2).t()) return 3;

  if (pt>0)
    {
    Qside = (ptot[1]*q[2]-ptot[2]*q[1])/pt;
    Qlong = (ptot[0]*q[3]-ptot[3]*q[0])/Mlong;
    Qout  = (roots/Mlong)*(ptot[1]*q[1]+ptot[2]*q[2])/pt;
    }
  else
    {
    Qlong = q[3];
    Qside = q[2];
    Qout  = q[1];
    }

  return 0;
  }

  // ported from RootHistogramHelpers.cpp line 1184
  int  calculateQ3DwPtPhiY(double pt1, double phi1, double y1,
                           double pt2, double phi2, double y2,
                           double & Qlong, double & Qout, double & Qside)
  {
  double mPart = 0.13957;
  double mSq   = mPart*mPart;

  double mt1   = std::sqrt(mSq + pt1*pt1);
  double px1   = pt1*std::cos(phi1);
  double py1   = pt1*std::sin(phi1);
  double pz1   = mt1*std::sinh(y1);
  double e1    = std::sqrt(mSq + pt1*pt1 + pz1*pz1);

  double mt2   = std::sqrt(mSq + pt2*pt2);
  double px2   = pt2*std::cos(phi2);
  double py2   = pt2*std::sin(phi2);
  double pz2   = mt2*std::sinh(y2);
  double e2    = std::sqrt(mSq + pt2*pt2 + pz2*pz2);

  double pt,s,Mlong,roots;
  double ptot[4],q[4];
  //const int g[4]={1,-1,-1,-1};
  //int alpha;
  s=0.0;
  ptot[0] = e1  + e2;
  ptot[1] = px1 + px2;
  ptot[2] = py1 + py2;
  ptot[3] = pz1 + pz2;
  q[0] = e1  - e2;
  q[1] = px1 - px2;
  q[2] = py1 - py2;
  q[3] = pz1 - pz2;
  s = ptot[0]*ptot[0] - ptot[1]*ptot[1] - ptot[2]*ptot[2] - ptot[3]*ptot[3];
  pt=std::sqrt(ptot[1]*ptot[1]+ptot[2]*ptot[2]);
  Mlong=std::sqrt(s+pt*pt);
  roots=std::sqrt(s);

  //  if ((p1+p2).P()>(p1+p2).t()) return 3;

  if (pt>0)
    {
    Qside = (ptot[1]*q[2]-ptot[2]*q[1])/pt;
    Qlong = (ptot[0]*q[3]-ptot[3]*q[0])/Mlong;
    Qout  = (roots/Mlong)*(ptot[1]*q[1]+ptot[2]*q[2])/pt;
    }
  else
    {
    Qlong = q[3];
    Qside = q[2];
    Qout  = q[1];
    }

  return 0;
  }

  // ported from RootHistogramHelpers.cpp line 1416
  void calculateR2_Q3D(const Histogram * n2_Q3D, const Histogram * n1n1_Q3D, Histogram * R2_Q3D, double a1, double a2)
  {

  if (!ptrExist(__FUNCTION__,n2_Q3D,n1n1_Q3D,R2_Q3D)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n2_Q3D,n1n1_Q3D))  throw IncompatibleHistogramException(__FUNCTION__);
  int nx = nBinsX(n2_Q3D);
  int ny = nBinsY(n2_Q3D);
  int nz = nBinsX(n2_Q3D);
  double v1,ev1,er1;
  double v2,ev2,er2;
  double v3,ev3;
  for (int i1=1;i1<=nx;++i1)
    {
    for (int i2=1;i2<=ny;++i2)
      {
      for (int i3=1;i3<=nz;++i3)
        {
        v1  = a1*binContent(n2_Q3D,i1,i2,i3);   ev1 = a1*binError(n2_Q3D,i1,i2,i3);
        v2  = a2*binContent(n1n1_Q3D,i1,i2,i3); ev2 = a2*binError(n1n1_Q3D,i1,i2,i3);
        if (v1>0  &&  v2>0)
          {
          if (v1)
            er1 = ev1/v1;
          else
            er1 = 0;
          if (v2)
            er2 = ev2/v2;
          else
            er2 = 0;
          if (er1< 0.5  &&  er2<0.5)
            {
            v3  = v1/v2 - 1;
            ev3 = v3*std::sqrt(er1*er1 + er2*er2);
            }
          else
            {
            v3 = 0.0;
            ev3 = 0.0;
            }
          }
        else
          {
          v3 = 0.0;
          ev3 = 0.0;
          }
        setBinContent(R2_Q3D,i1,i2,i3,v3);
        setBinError(R2_Q3D,i1,i2,i3,ev3);
        }
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 1469
  // Return the average bin content of the given 1D histogram
  double avgValue(Histogram * h)
  {
  if (!ptrExist(__FUNCTION__,h)) throw NullPointerException(__FUNCTION__);
  int nx = nBinsX(h);
  if (nx<1) return 0.0;
  double nnx = nx;
  double sum = 0.0;
  for (int k=1; k<=nx; k++)
    {
    sum += binContent(h,k);
    }
  return sum/nnx;
  }

  // ported from RootHistogramHelpers.cpp line 1620
  void findMaximum(Histogram * h, int & xMaxValueBin, double & maxValue, int xFirstBin, int xLastBin)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  int n = nBinsX(h);
  if (xFirstBin<1) xFirstBin = 1;
  if (xLastBin<1 || xLastBin>n)  xLastBin = n;
  for (int i=xFirstBin;i<=xLastBin;++i)
    {
    double v = binContent(h,i);
    if (v>maxValue)
      {
      xMaxValueBin = i;
      maxValue = v;
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 1716
  void findMaximum(Histogram * h, int & xMaxValueBin, int & yMaxValueBin, double & maxValue, int xFirstBin, int xLastBin, int yFirstBin, int yLastBin)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  int nx = nBinsX(h);
  int ny = nBinsY(h);
  if (xFirstBin<1) xFirstBin = 1;
  if (xLastBin<1 || xLastBin>nx)  xLastBin = nx;
  if (yFirstBin<1) yFirstBin = 1;
  if (yLastBin<1 || yLastBin>ny)  yLastBin = ny;
  for (int ix=xFirstBin;ix<=xLastBin;++ix)
    {
    for (int iy=yFirstBin;iy<=yLastBin;++iy)
      {
      double v = binContent(h,ix,iy);
      if (v>maxValue)
        {
        xMaxValueBin = ix;
        yMaxValueBin = iy;
        maxValue = v;
        }
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 1637
  void findMinimum(Histogram * h, int & xMinValueBin, double & minValue, int xFirstBin, int xLastBin)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  int n = nBinsX(h);
  if (xFirstBin<1) xFirstBin = 1;
  if (xLastBin<1 || xLastBin>n)  xLastBin = n;
  for (int i=xFirstBin;i<=xLastBin;++i)
    {
    double v = binContent(h,i);
    if (v<minValue)
      {
      xMinValueBin = i;
      minValue = v;
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 1692
  void findMinimum(Histogram * h, int & xMinValueBin, int & yMinValueBin, double & minValue, int xFirstBin, int xLastBin, int yFirstBin, int yLastBin)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  int nx = nBinsX(h);
  int ny = nBinsY(h);
  if (xFirstBin<1) xFirstBin = 1;
  if (xLastBin<1 || xLastBin>nx)  xLastBin = nx;
  if (yFirstBin<1) yFirstBin = 1;
  if (yLastBin<1 || yLastBin>ny)  yLastBin = ny;
  for (int ix=xFirstBin;ix<=xLastBin;++ix)
    {
    for (int iy=yFirstBin;iy<=yLastBin;++iy)
      {
      double v = binContent(h,ix,iy);
      if (v<minValue)
        {
        xMinValueBin = ix;
        yMinValueBin = iy;
        minValue = v;
        }
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 1654
  void findMinMax (Histogram * h,
                   int & xMinValueBin, double & minValue,
                   int & xMaxValueBin, double & maxValue,
                   int xFirstBin, int xLastBin)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  int n = nBinsX(h);
  if (xFirstBin<1) xFirstBin = 1;
  if (xLastBin<1 || xLastBin>n)  xLastBin = n;
  for (int i=xFirstBin;i<=xLastBin;++i)
    {
    double v = binContent(h,i);
    if (v<minValue)
      {
      xMinValueBin = i;
      minValue = v;
      }
    if (v>maxValue)
      {
      xMaxValueBin = i;
      maxValue = v;
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 1679
  void findMinMax (std::vector<Histogram*> histograms,
                   int & xMinValueBin, double & minValue,
                   int & xMaxValueBin, double & maxValue,
                   int xFirstBin, int xLastBin)
  {
  for (auto & histogram : histograms)
    {
    findMinMax (histogram,xMinValueBin,minValue,xMaxValueBin,maxValue,xFirstBin,xLastBin);
    }
  }

  // ported from RootHistogramHelpers.cpp line 1740
  void findMinMax (Histogram * h,
                   int & xMinValueBin, int & yMinValueBin, double & minValue,
                   int & xMaxValueBin, int & yMaxValueBin, double & maxValue,
                   int xFirstBin, int xLastBin,
                   int yFirstBin, int yLastBin)
  {
  if (!h) throw NullPointerException(__FUNCTION__);
  int nx = nBinsX(h);
  int ny = nBinsY(h);
  if (xFirstBin<1) xFirstBin = 1;
  if (xLastBin<1 || xLastBin>nx)  xLastBin = nx;
  if (yFirstBin<1) yFirstBin = 1;
  if (yLastBin<1 || yLastBin>ny)  yLastBin = ny;
  for (int ix=xFirstBin;ix<=xLastBin;++ix)
    {
    for (int iy=yFirstBin;iy<=yLastBin;++iy)
      {
      double v = binContent(h,ix,iy);
      if (v<minValue)
        {
        xMinValueBin = ix;
        yMinValueBin = iy;
        minValue = v;
        }
      if (v>maxValue)
        {
        xMaxValueBin = ix;
        yMaxValueBin = iy;
        maxValue = v;
        }
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 1774
  void findMinMax (std::vector<Histogram*> histograms,
                   int & xMinValueBin, int & yMinValueBin, double & minValue,
                   int & xMaxValueBin, int & yMaxValueBin, double & maxValue,
                   int xFirstBin, int xLastBin,
                   int yFirstBin, int yLastBin)
  {
  for (auto & histogram : histograms)
    {
    findMinMax (histogram,xMinValueBin,yMinValueBin,minValue,xMaxValueBin,yMaxValueBin,maxValue,xFirstBin,xLastBin,yFirstBin,yLastBin);
    }
  }

  // ported from RootHistogramHelpers.cpp line 1870
  void unpack_vsXY_to_vsXVsY(const Histogram * source, Histogram * target)
  {

  if (!ptrExist(__FUNCTION__,source)) throw NullPointerException(__FUNCTION__);
  // Unpack 1D source histogram into a 2D histogram
  int k = 1;
  double v;
  int n  = nBinsX(source);
  int n_x = nBinsX(target);
  int n_y = nBinsY(target);
  if (n!=n_x*n_y)
    throw IncompatibleHistogramException(__FUNCTION__);

  for (int i_x=1; i_x<=n_x; ++i_x)
    {
    for (int i_y=1; i_y<=n_y; ++i_y)
      {
      v  = binContent(source,k);
      //ev = binError(source,k);
      setBinContent(target,i_x,i_y,v);
      setBinError(target,i_x,i_y,v);
      k++;
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 1896
  void correctMerging(Histogram * h, int nEta, int nPhi, bool reverse)
  {

  if (!ptrExist(__FUNCTION__,h)) throw NullPointerException(__FUNCTION__);
  double v, ev;
  int iEta, iPhi, iDphi, iPhiA, iPhiB;
  int ij1, ij2;
  int nEtaPhi = nEta*nPhi;
  for (iEta=0; iEta<nEta; iEta++)
    {
    for (iPhi=0; iPhi<nPhi; iPhi++)
      {
      for (iDphi=1; iDphi<7; iDphi++)
        {
        iPhiA = iPhi+iDphi;  if (iPhiA>=nPhi) iPhiA -= nPhi; // Dphi12 = iPhi-iPhiA < 0
        iPhiB = iPhi-iDphi;  if (iPhiB< 0) iPhiB += nPhi; // Dphi12 = iPhi-iPhiB > 0
                                                          //Deta==0
        ij1 = nEtaPhi*(iEta*nPhi+iPhi) + iEta*nPhi+iPhiA;
        ij2 = nEtaPhi*(iEta*nPhi+iPhi) + iEta*nPhi+iPhiB;
        if (reverse)
          {
          v  = binContent(h,ij1);
          ev = binError(h,ij1);
          setBinContent(h,ij2,v);
          setBinError(h,ij2,ev);
          }
        else
          {
          v  = binContent(h,ij2);
          ev = binError(h,ij2);
          setBinContent(h,ij1,v);
          setBinError(h,ij1,ev);
          }
        if (iEta>0)   //Deta==1
          {
          ij1 = nEtaPhi*(iEta*nPhi+iPhi) + (iEta-1)*nPhi+iPhiA;
          ij2 = nEtaPhi*(iEta*nPhi+iPhi) + (iEta-1)*nPhi+iPhiB;
          if (reverse)
            {
            v  = binContent(h,ij1);
            ev = binError(h,ij1);
            setBinContent(h,ij2,v);
            setBinError(h,ij2,ev);
            }
          else
            {
            v  = binContent(h,ij2);
            ev = binError(h,ij2);
            setBinContent(h,ij1,v);
            setBinError(h,ij1,ev);
            }

          }
        if (iEta<nEta)   //Deta==1
          {
          ij1 = nEtaPhi*(iEta*nPhi+iPhi) + (iEta+1)*nPhi+iPhiA;
          ij2 = nEtaPhi*(iEta*nPhi+iPhi) + (iEta+1)*nPhi+iPhiB;
          if (reverse)
            {
            v  = binContent(h,ij1);
            ev = binError(h,ij1);
            setBinContent(h,ij2,v);
            setBinError(h,ij2,ev);
            }
          else
            {
            v  = binContent(h,ij2);
            ev = binError(h,ij2);
            setBinContent(h,ij1,v);
            setBinError(h,ij1,ev);
            }

          }

        }
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 1979
  //!
  //! Calculate the 'triangle' normalization
  //! target will be divided by bin widths and scaleFactor
  //!
  void calculateAvgH2H2(const Histogram * source, Histogram * target, double scaleFactor)
  {
  if (!ptrExist(__FUNCTION__,source,target)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,source,target))  throw IncompatibleHistogramException(__FUNCTION__);
  int nDeta = nBinsX(source);
  int nDphi = nBinsY(source);
  std::vector<double> denominator(nDeta,0.0);
  for (int iEta=0;iEta<(nDeta+1)/2; ++iEta)
    {
    denominator[iEta] = iEta+1;
    denominator[nDeta-1-iEta] = iEta+1;
    }
  double v1, ev1;
  for (int iEta=1;iEta<=nDeta; ++iEta)
    {
    for (int iPhi=1;iPhi<=nDphi; ++iPhi)
      {
      v1   = binContent(source,iEta,iPhi);
      ev1  = binError(source,  iEta,iPhi);
      v1   = v1/denominator[iEta-1];
      ev1  = ev1/denominator[iEta-1];
      setBinContent(target,iEta,iPhi,v1);
      setBinError(target,iEta,iPhi,ev1);
      }
    }
  double wx = binWidthX(target,1);
  double wy = binWidthY(target,1);
  double factor = 1.0/(wx*wy*scaleFactor);
  target->scale(factor);
  }

  // ported from RootHistogramHelpers.cpp line 2012
  void calculateAverageVsDeta(const Histogram * obs_12, Histogram * avgObs_12, int n)
  {
  if (!ptrExist(__FUNCTION__,obs_12,avgObs_12)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,obs_12,avgObs_12))  throw IncompatibleHistogramException(__FUNCTION__);
  int n_x = nBinsX(obs_12);
  int n_y = nBinsY(obs_12);
  std::vector<double> denom(n_x,1.0);
  int off = (n_x - (2*n-1))/2;
  for (int i_x=0; i_x<n; ++i_x)
    {
    if (i_x==0)
      {
      denom[off + n] = n;
      }
    else
      {
      denom[off + n + i_x] = n-i_x;
      denom[off + n - i_x] = n-i_x;
      }
    }
  for (int i_x=0; i_x<n; ++i_x)
    {
    for (int i_y=1;i_y<=n_y;++i_y)
      {
      double v  = binContent(obs_12,i_x,i_y);
      double ev = binError(obs_12,i_x,i_y);
      v  = v/denom[i_x];
      ev = ev/denom[i_x];
      setBinContent(avgObs_12,i_x,i_y,v);
      setBinError(avgObs_12,i_x,i_y,ev);
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 2049
  //!
  //! Calculate C2 = N2 - N1*N1
  //!
  void calculateC2_H1H1H1(const Histogram * n2_12,
                          const Histogram * n1n1_12,
                          Histogram * c2_12,
                          double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,n2_12,n1n1_12,c2_12))  throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n2_12,n1n1_12,c2_12))  throw IncompatibleHistogramException(__FUNCTION__);
  int n2_12_n_x = nBinsX(n2_12);
  double v1,ev1,v2,ev2,v,ev;
  for (int i1=1;i1<=n2_12_n_x;++i1)
    {
    v1  = a1*binContent(n2_12,i1);    ev1 = a1*binError(n2_12,i1);
    v2  = a2*binContent(n1n1_12,i1);  ev2 = a2*binError(n1n1_12,i1);
    v  = v1-v2;
    ev = std::sqrt(ev1*ev1+ev2*ev2);
    setBinContent(c2_12,i1,v); setBinError(c2_12,i1,ev);
    }
  }

  // ported from RootHistogramHelpers.cpp line 2071
  //!
  //! Calculate C2 = N2 - N1*N1
  //!
  void calculateC2_H2H2H2(const Histogram * n2_12,
                          const Histogram * n1n1_12,
                          Histogram * c2_12,
                          double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,n2_12,n1n1_12,c2_12))  throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n2_12,n1n1_12,c2_12))  throw IncompatibleHistogramException(__FUNCTION__);
  int n2_12_n_x = nBinsX(n2_12);
  int n2_12_n_y = nBinsY(n2_12);
  double v1,ev1,v2,ev2,v,ev;
  for (int i1=1;i1<=n2_12_n_x;++i1)
    {
    for (int i2=1;i2<=n2_12_n_y;++i2)
      {
      v1  = a1*binContent(n2_12,i1,i2);    ev1 = a1*binError(n2_12,i1,i2);
      v2  = a2*binContent(n1n1_12,i1,i2);  ev2 = a2*binError(n1n1_12,i1,i2);
      v  = v1-v2;
      ev = std::sqrt(ev1*ev1+ev2*ev2);
      setBinContent(c2_12,i1,i2,v); setBinError(c2_12,i1,i2,ev);
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 2097
  //!
  //! Calculate C2 = N2 - N1*N1
  //!
  void calculateC2_H3H3H3(const Histogram * n2_12,
                          const Histogram * n1n1_12,
                          Histogram * c2_12,
                          double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,n2_12,n1n1_12,c2_12))  throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n2_12,n1n1_12,c2_12))  throw IncompatibleHistogramException(__FUNCTION__);
  int n2_12_n_x   = nBinsX(n2_12);
  int n2_12_n_y   = nBinsY(n2_12);
  int n2_12_n_z   = nBinsZ(n2_12);
  double v1,ev1,v2,ev2,v,ev;
  for (int i1=1;i1<=n2_12_n_x;++i1)
    {
    for (int i2=1;i2<=n2_12_n_y;++i2)
      {
      for (int i3=1;i2<=n2_12_n_z;++i2)
        {
        v1  = a1*binContent(n2_12,i1,i2,i3);    ev1 = a1*binError(n2_12,i1,i2,i3);
        v2  = a2*binContent(n1n1_12,i1,i2,i3);  ev2 = a2*binError(n1n1_12,i1,i2,i3);
        v  = v1-v2;
        ev = std::sqrt(ev1*ev1+ev2*ev2);
        setBinContent(c2_12,i1,i2,i3,v); setBinError(c2_12,i1,i2,i3,ev);
        }
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 2127
  //!
  //! Calculate R2 = binCorrection*N2/N1/N1 - 1
  //!
  void calculateR2_H1H1H1(const Histogram * n2_12, const Histogram * n1n1_12, Histogram * r2_12, bool ijNormalization, double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,n2_12,n1n1_12,r2_12))  throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n2_12,n1n1_12,r2_12))  throw IncompatibleHistogramException(__FUNCTION__);
  int n2_12_n_x = nBinsX(n2_12);
  double v1,ev1,v2,ev2,v,ev, re1,re2;


  for (int i1=1;i1<=n2_12_n_x;++i1)
    {
    v1  = a1*binContent(n2_12,i1);    ev1 = a1*binError(n2_12,i1);
    v2  = a2*binContent(n1n1_12,i1);  ev2 = a2*binError(n1n1_12,i1);

    if (v1>0  &&  v2>0  &&  ev1/v1<0.5  &&  ev2/v2<0.5 )
      {
      if (ijNormalization) //account for the fact only half the pairs were counted
        {
        v   = 2*v1/v2;
        }
      else // all pairs counted - no need to multiply by 2
        {
        v   = v1/v2;
        }
      re1 = ev1/v1;
      re2 = ev2/v2;
      ev  = v*std::sqrt(re1*re1+re2*re2);
      v   -= 1.000;
      }
    else
      {
      v = 0.;
      ev = 0.000001;
      }
    setBinContent(r2_12,i1,v); setBinError(r2_12,i1,ev);
    }
  }

  // ported from RootHistogramHelpers.cpp line 2167
  //!
  //! Calculate R2 = binCorrection*N2/N1/N1 - 1
  //!
  void calculateR2_H2H2H2(const Histogram * n2_12, const Histogram * n1n1_12, Histogram * r2_12, bool ijNormalization, double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,n2_12,n1n1_12,r2_12)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n2_12,n1n1_12,r2_12))  throw IncompatibleHistogramException(__FUNCTION__);
  int n2_12_n_x = nBinsX(n2_12);
  int n2_12_n_y = nBinsY(n2_12);
  double v1,ev1,v2,ev2,v,ev, re1,re2;
  for (int i_x=1;i_x<=n2_12_n_x;++i_x)
    {
    for (int i_y=1;i_y<=n2_12_n_y;++i_y)
      {
      v1  = a1*binContent(n2_12,i_x,i_y);    ev1 = a1*binError(n2_12,i_x,i_y);
      v2  = a2*binContent(n1n1_12,i_x,i_y);  ev2 = a2*binError(n1n1_12,i_x,i_y);
      if (v1>0  &&  v2>0) //   &&  ev1/v1<0.5  &&  ev2/v2<0.5)
        {
        if (ijNormalization) //account for the fact only half the pairs were counted
          v   = 2*v1/v2;
        else // all pairs counted - no need to multiply by 2
          v   = v1/v2;
        re1 = ev1/v1;
        re2 = ev2/v2;
        ev  = v*std::sqrt(re1*re1+re2*re2);
        v   -= 1.;
        }
      else
        {
        v = 0.;
        ev = 0;
        }
      setBinContent(r2_12,i_x,i_y,v); setBinError(r2_12,i_x,i_y,ev);
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 2204
  //!
  //! Calculate R2 = binCorrection*N2/N1/N1 - 1
  //!
  void calculateR2_H3H3H3(const Histogram * n2_12, const Histogram * n1n1_12, Histogram * r2_12, bool ijNormalization, double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,n2_12,n1n1_12,r2_12)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n2_12,n1n1_12,r2_12))  throw IncompatibleHistogramException(__FUNCTION__);
  int n2_12_n_x = nBinsX(n2_12);
  int n2_12_n_y = nBinsY(n2_12);
  int n2_12_n_z = nBinsZ(n2_12);
  double v1,ev1,v2,ev2,v,ev, re1,re2;
  for (int i_x=1;i_x<=n2_12_n_x;++i_x)
    {
    for (int i_y=1;i_y<=n2_12_n_y;++i_y)
      {
      for (int i_z=1;i_z<=n2_12_n_z;++i_z)
        {
        v1  = a1*binContent(n2_12,i_x,i_y,i_z);    ev1 = a1*binError(n2_12,i_x,i_y,i_z);
        v2  = a2*binContent(n1n1_12,i_x,i_y,i_z);  ev2 = a2*binError(n1n1_12,i_x,i_y,i_z);
        if (v1>0  &&  v2>0) //   &&  ev1/v1<0.5  &&  ev2/v2<0.5)
          {
          if (ijNormalization) //account for the fact only half the pairs were counted
            v   = 2*v1/v2;
          else // all pairs counted - no need to multiply by 2
            v   = v1/v2;
          re1 = ev1/v1;
          re2 = ev2/v2;
          ev  = v*std::sqrt(re1*re1+re2*re2);
          v   -= 1.;
          }
        else
          {
          v = 0.;
          ev = 0;
          }
        setBinContent(r2_12,i_x,i_y,i_z,v); setBinError(r2_12,i_x,i_y,i_z,ev);
        }
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 2244
  //Calculate R2 = N2/N1/N1 - 1
  void calculateR2_H1H2H2(const Histogram * n2_12, const Histogram * n1n1_12, Histogram * r2_12, bool ijNormalization, double a1, double a2)
  {
  if (!ptrExist(__FUNCTION__,n2_12,n1n1_12,r2_12)) throw NullPointerException(__FUNCTION__);
  int n2_12_n_x    = nBinsX(n2_12);
  int n1n1_12_n_x  = nBinsX(n1n1_12);
  int n1n1_12_n_y  = nBinsY(n1n1_12);
  int r2_12_n_x    = nBinsX(r2_12);
  int r2_12_n_y    = nBinsY(r2_12);
  if (n2_12_n_x!=(n1n1_12_n_x*n1n1_12_n_y) || n1n1_12_n_x!=r2_12_n_x || n1n1_12_n_y!=r2_12_n_y )
    throw IncompatibleHistogramException(__FUNCTION__);
  double v1,ev1,v2,ev2,v,ev, re1,re2;
  int i = 1;
  for (int i_x=1;i_x<=n1n1_12_n_x;++i_x)
    {
    for (int i_y=1;i_y<=n1n1_12_n_y;++i_y)
      {
      v1  = a1*binContent(n2_12,i);        ev1 = a1*binError(n2_12,i);
      v2  = a2*binContent(n1n1_12,i_x,i_y);  ev2 = a2*binError(n1n1_12,i_x,i_y);
      if (v1>0  &&  v2>0  &&  ev1/v1<0.5  &&  ev2/v2<0.5)
        {
        if (ijNormalization) //account for the fact only half the pairs were counted
          v   = 2*v1/v2;
        else // all pairs counted - no need to multiply by 2
          v   = v1/v2;
        re1 = ev1/v1;
        re2 = ev2/v2;
        ev  = v*std::sqrt(re1*re1+re2*re2);
        v   -= 1.;
        }
      else
        {
        v = 0.;
        ev = 0;
        }
      setBinContent(r2_12,i_x,i_y,v); setBinError(r2_12,i_x,i_y,ev);
      i++;
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 2384
  //!
  //! Compute the average density of particles based on the given dN/dx histogram.
  //! The histogram is assumed to be a yield per event per bin vs. eta
  //! The average value is thus divided by the bin width to make it into a density.
  //! This yields a density integrated over 2pi. So to use this number, one must also
  //! divide by 2pi.
  //!
  void calculateAverage(Histogram* h, double & avgDensity, double & eAvgDensity)
  {

  int n = nBinsX(h);
  double v, ev, width, sum, esum, avgValue;
  sum  = 0.0;
  esum = 0.0;
  //  int first = 9;
  //  int last  = 32;
  int first = 1;
  int last  = n;
  int norm  = last - first + 1;
  for (int i=first; i<=last; i++)
    {
    v     = binContent(h,i);
    ev    = binError(h,i);
    width = binWidthX(h,i);
    sum  += v/width;
    esum += ev*ev/width/width;
    }
  avgValue    = sum/norm;
  avgDensity  = avgValue/MATH::twoPi();
  eAvgDensity = std::sqrt(esum)/norm/MATH::twoPi();
  }

  // ported from RootHistogramHelpers.cpp line 2444
  void calculateInclusiveAvg(const Histogram * h1, const Histogram * h2, Histogram * h3)
  {
  if (!ptrExist(__FUNCTION__,h1,h2,h3))  throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,h1,h2,h3))  throw IncompatibleHistogramException(__FUNCTION__);

  int n1 = nBinsX(h1);
  double v1,ev1,v2,ev2,v,ev, re1,re2;
  for (int i1=1;i1<=n1;++i1)
    {
    v1  = binContent(h1,i1);  ev1 = binError(h1,i1);
    v2  = binContent(h2,i1);  ev2 = binError(h2,i1);
    if (v1>0  &&  v2>0)
      {
      v = v1/v2;
      re1 = ev1/v1;
      re2 = ev2/v2;
      ev = v*std::sqrt(re1*re1+re2*re2);
      }
    else
      {
      v = 0.;
      ev = 0;
      }
    setBinContent(h3,i1,v); setBinError(h3,i1,ev);
    }
  }

  // ported from RootHistogramHelpers.cpp line 2473
  void calculateInclusiveAvgH2(const Histogram * h1, const Histogram * h2, Histogram * h3)
  {
  if (!ptrExist(__FUNCTION__,h1,h2,h3))  throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,h1,h2,h3))  throw IncompatibleHistogramException(__FUNCTION__);
  int n1x   = nBinsX(h1);
  int n1y   = nBinsY(h1);
  double v1,ev1,v2,ev2,v,ev, re1,re2;
  for (int i_x=1;i_x<=n1x;++i_x)
    {
    for (int i_y=1;i_y<=n1y;++i_y)
      {
      v1  = binContent(h1,i_x,i_y);  ev1 = binError(h1,i_x,i_y);
      v2  = binContent(h2,i_x,i_y);  ev2 = binError(h2,i_x,i_y);
      if (v1>0  &&  v2>0)
        {
        v = v1/v2;
        re1 = ev1/v1;
        re2 = ev2/v2;
        ev = v*std::sqrt(re1*re1+re2*re2);
        }
      else
        {
        v = 0.;
        ev = 0;
        }
      setBinContent(h3,i_x,i_y,v); setBinError(h3,i_x,i_y,ev);
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 2503
  void calculateInclusiveAvgH1H2(const Histogram * h1, const Histogram * h2, Histogram * h3)
  {
  if (!ptrExist(__FUNCTION__,h1,h2,h3))  throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,h1,h2)) throw IncompatibleHistogramException(__FUNCTION__);
  //Calculate average pt by dividing sPt contained in h1 by n1 contained in h2. Store in h3.
  int n3x  = nBinsX(h3);
  int n3y  = nBinsY(h3);
  double v1,ev1,v2,ev2,v,ev, re1,re2;
  int i = 1;
  for (int i_x=1;i_x<=n3x;++i_x)
    {
    for (int i_y=1;i_y<=n3y;++i_y)
      {
      v1  = binContent(h1,i);  ev1 = binError(h1,i);
      v2  = binContent(h2,i);  ev2 = binError(h2,i);
      if (v1>0  &&  v2>0)
        {
        v = v1/v2;
        re1 = ev1/v1;
        re2 = ev2/v2;
        ev = v*std::sqrt(re1*re1+re2*re2);
        }
      else
        {
        v = 0.;
        ev = 0;
        }
      setBinContent(h3,i_x,i_y,v); setBinError(h3,i_x,i_y,ev);
      i++;
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 2562
  void symmetrize3D(Histogram* h)
  {
  if (!ptrExist(__FUNCTION__,h))  throw NullPointerException(__FUNCTION__);
  int n = nBinsX(h);
  int nc = n/2;
  int nf = nc + 1;
  double v1, v2, v3, v4, v5, v6, v7, v8;
  double ev1, ev2, ev3, ev4, ev5, ev6, ev7, ev8;
  double v, ev;
  for (int i1=1; i1<=nc; i1++)
    {
    for (int i2=1; i2<=nc; i2++)
      {
      for (int i3=1; i3<=nc; i3++)
        {
        v1 = binContent(h,nc+i1,nc+i2,nc+i3);  ev1 = binError(h,nc+i1,nc+i2,nc+i3);
        v2 = binContent(h,nc+i1,nc+i2,nf-i3);  ev2 = binError(h,nc+i1,nc+i2,nf-i3);
        v3 = binContent(h,nc+i1,nf-i2,nc+i3);  ev3 = binError(h,nc+i1,nf-i2,nc+i3);
        v4 = binContent(h,nc+i1,nf-i2,nf-i3);  ev4 = binError(h,nc+i1,nf-i2,nf-i3);
        v5 = binContent(h,nf-i1,nc+i2,nc+i3);  ev5 = binError(h,nf-i1,nc+i2,nc+i3);
        v6 = binContent(h,nf-i1,nc+i2,nf-i3);  ev6 = binError(h,nf-i1,nc+i2,nf-i3);
        v7 = binContent(h,nf-i1,nf-i2,nc+i3);  ev7 = binError(h,nf-i1,nf-i2,nc+i3);
        v8 = binContent(h,nf-i1,nf-i2,nf-i3);  ev8 = binError(h,nf-i1,nf-i2,nf-i3);

        v = (v1+v2+v3+v4+v5+v6+v7+v8)/8.0;
        ev = std::sqrt(ev1*ev1+ev2*ev2+ev3*ev3+ev4*ev4+ev5*ev5+ev6*ev6+ev7*ev7+ev8*ev8)/8.0;

        setBinContent(h,nc+i1,nc+i2,nc+i3,v);  setBinError(h,nc+i1,nc+i2,nc+i3,ev);
        setBinContent(h,nc+i1,nc+i2,nf-i3,v);  setBinError(h,nc+i1,nc+i2,nf-i3,ev);
        setBinContent(h,nc+i1,nf-i2,nc+i3,v);  setBinError(h,nc+i1,nf-i2,nc+i3,ev);
        setBinContent(h,nc+i1,nf-i2,nf-i3,v);  setBinError(h,nc+i1,nf-i2,nf-i3,ev);
        setBinContent(h,nf-i1,nc+i2,nc+i3,v);  setBinError(h,nf-i1,nc+i2,nc+i3,ev);
        setBinContent(h,nf-i1,nc+i2,nf-i3,v);  setBinError(h,nf-i1,nc+i2,nf-i3,ev);
        setBinContent(h,nf-i1,nf-i2,nc+i3,v);  setBinError(h,nf-i1,nf-i2,nc+i3,ev);
        setBinContent(h,nf-i1,nf-i2,nf-i3,v);  setBinError(h,nf-i1,nf-i2,nf-i3,ev);

        }
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 2608
  // h must be writtable otherwise this does not work..
  // x axis is DeltaEta. It must have an odd number of bins
  // y axis is DeltaPhi. It must have an even number of bins
  void symmetrizeDeltaEtaDeltaPhi(Histogram * h, bool ijNormalization)
  {
  if (!ptrExist(__FUNCTION__,h))  throw NullPointerException(__FUNCTION__);
  double v1, v2, v3, v4;
  double ev1, ev2, ev3, ev4;
  double sv, esv;
  int nEta = nBinsX(h); //DeltaEta
  int nPhi = nBinsY(h); //DeltaPhi
  int nEtaHalf = (nEta-1)/2;
  int nPhiHalf = (nPhi-2)/2;
  int iEta, iPhi, iPhi1, iEta1;
  double * v = new double[nEta*nPhi];
  double * ev = new double[nEta*nPhi];
  for (int iPhi=0;iPhi<nPhi;iPhi++)
    {
    for (int iEta=0;iEta<nEta;iEta++)
      {
      iPhi1 = iPhi+1;
      iEta1 = iEta+1;
      v[ iEta+iPhi*nEta]  = binContent(h,iEta1,iPhi1);
      ev[iEta+iPhi*nEta]  = binError(h,  iEta1,iPhi1);
      }
    }
  for (iEta=0;iEta<nEtaHalf;iEta++)
    {
    iEta1 = iEta+1;
    for (iPhi=0; iPhi<nPhiHalf;iPhi++)
      {
      iPhi1 = iPhi+1;
      v1 = v[  nEta-iEta1+(nPhi-iPhi1)*nEta];
      v2 = v[  nEta-iEta1+(     iPhi1)*nEta];
      v3 = v[        iEta+(nPhi-iPhi1)*nEta];
      v4 = v[        iEta+(     iPhi1)*nEta];
      ev1 = ev[nEta-iEta1+(nPhi-iPhi1)*nEta];
      ev2 = ev[nEta-iEta1+(     iPhi1)*nEta];
      ev3 = ev[      iEta+(nPhi-iPhi1)*nEta];
      ev4 = ev[      iEta+(     iPhi1)*nEta];
      if (ijNormalization)
        {
        sv = (v1+v2+v3+v4)/2.;
        esv = std::sqrt(ev1*ev1+ev2*ev2+ev3*ev3+ev4*ev4)/2.;
        }
      else
        {
        sv = (v1+v2+v3+v4)/4.;
        esv = std::sqrt(ev1*ev1+ev2*ev2+ev3*ev3+ev4*ev4)/4.;
        }
      setBinContent(h, nEta-iEta, nPhi-iPhi, sv);
      setBinContent(h, nEta-iEta,   iPhi1+1, sv);
      setBinContent(h,     iEta1, nPhi-iPhi, sv);
      setBinContent(h,     iEta1,   iPhi1+1, sv);
      setBinError(h,   nEta-iEta, nPhi-iPhi, esv);
      setBinError(h,   nEta-iEta,   iPhi1+1, esv);
      setBinError(h,       iEta1, nPhi-iPhi, esv);
      setBinError(h,       iEta1,   iPhi1+1, esv);
      }
    }
  iEta  = nEtaHalf;
  iEta1 = iEta+1;
  for (iPhi=0; iPhi<nPhiHalf;iPhi++) // iEta center bin
    {
    iPhi1 = iPhi+1;
    v3 = v[        iEta+(nPhi-iPhi1)*nEta];
    v4 = v[        iEta+(     iPhi1)*nEta];
    ev3 = ev[      iEta+(nPhi-iPhi1)*nEta];
    ev4 = ev[      iEta+(     iPhi1)*nEta];
    if (ijNormalization)
      {
      sv = (v3+v4);
      esv = std::sqrt(ev3*ev3+ev4*ev4);
      }
    else
      {
      sv = (v3+v4)/2.;
      esv = std::sqrt(ev3*ev3+ev4*ev4)/2.;
      }
    setBinContent(h,     iEta1, nPhi-iPhi, sv);
    setBinContent(h,     iEta1,   iPhi1+1, sv);
    setBinError(h,       iEta1, nPhi-iPhi, esv);
    setBinError(h,       iEta1,   iPhi1+1, esv);
    }
  iPhi = 0;
  iPhi1 = iPhi+1;
  for (iEta=0;iEta<nEtaHalf;iEta++)
    {
    iEta1 = iEta+1;
    v1 = v[  nEta-iEta1];
    v3 = v[        iEta];
    ev1 = ev[nEta-iEta1];
    ev3 = ev[      iEta];
    if (ijNormalization)
      {
      sv = (v1+v3);
      esv = std::sqrt(ev1*ev1+ev3*ev3);
      }
    else
      {
      sv = (v1+v3)/2.;
      esv = std::sqrt(ev1*ev1+ev3*ev3)/2.;
      }
    setBinContent(h, nEta-iEta, 1, sv);
    setBinContent(h,     iEta1, 1, sv);
    setBinError(h,   nEta-iEta, 1, esv);
    setBinError(h,       iEta1, 1, esv);

    iPhi  = nPhi/2;
    iPhi1 = iPhi+1;
    v1 = v[  nEta-iEta1 + iPhi*nEta];
    v3 = v[        iEta + iPhi*nEta];
    ev1 = ev[nEta-iEta1 + iPhi*nEta];
    ev3 = ev[      iEta + iPhi*nEta];
    if (ijNormalization)
      {
      sv = (v1+v3);
      esv = std::sqrt(ev1*ev1+ev3*ev3);
      }
    else
      {
      sv = (v1+v3)/2.;
      esv = std::sqrt(ev1*ev1+ev3*ev3)/2.;
      }
    setBinContent(h, nEta-iEta, iPhi1, sv);
    setBinContent(h,     iEta1, iPhi1, sv);
    setBinError(h,   nEta-iEta, iPhi1, esv);
    setBinError(h,       iEta1, iPhi1, esv);
    }
  delete[] v;
  delete[] ev;   // (the ROOT version did not free ev)
  }

  // ported from RootHistogramHelpers.cpp line 2737
  void symmetrizeXX(Histogram * h, bool ijNormalization)
  {
  if (!ptrExist(__FUNCTION__,h))  throw NullPointerException(__FUNCTION__);
  double v1, v2;
  double ev1, ev2;
  double sv, esv;
  int n_x = nBinsX(h); //DeltaEta
  int n_y = nBinsY(h); //DeltaPhi
  int i_x, i_y, i_x1, i_y1;
  double * v  = new double[n_x*n_y];
  double * ev = new double[n_x*n_y];
  for (i_x=0; i_x<n_x; i_x++)
    {
    i_x1 = i_x+1;
    for (i_y=0; i_y<n_y; i_y++)
      {
      i_y1 = i_y+1;
      v[ i_x+i_y*n_x]  = binContent(h,i_x1,i_y1);
      ev[i_x+i_y*n_x]  = binError(h,  i_x1,i_y1);
      }
    }
  for (i_x=0; i_x<n_x; i_x++)
    {
    i_x1 = i_x+1;
    for (i_y=0; i_y<n_y; i_y++)
      {
      i_y1 = i_y+1;
      v1 = v[  i_x+i_y*n_x ];
      v2 = v[  i_y+i_x*n_x ];
      ev1 = ev[ i_x+i_y*n_x ];
      ev2 = ev[ i_y+i_x*n_x];
      if (ijNormalization)
        {
        sv = (v1+v2);
        esv = std::sqrt(ev1*ev1+ev2*ev2);
        }
      else
        {
        sv = (v1+v2)/2.;
        esv = std::sqrt(ev1*ev1+ev2*ev2)/2.;
        }
      setBinContent(h, i_x1, i_y1, sv);
      setBinContent(h, i_y1, i_x1, sv);
      setBinError(h,   i_x1, i_y1, esv);
      setBinError(h,   i_y1, i_x1, esv);
      }
    }
  delete[] v;
  delete[] ev;   // (the ROOT version did not free ev)
  }

  // ported from RootHistogramHelpers.cpp line 2788
  void reduce_n2xEtaPhi_n2DetaDphi(const Histogram * source, Histogram * target,int nEtaBins,int nPhiBins)
  {
  if (!ptrExist(__FUNCTION__,source,target))  throw NullPointerException(__FUNCTION__);
  double v1,v2,ev1;
  int dPhi,dEta, iPhi,iEta,jPhi,jEta, i, j;
  //int nBins = nEtaBins*nPhiBins;
  int nWrk  = nPhiBins*(2*nEtaBins-1);
  int index;
  double * numerator    = new double[nWrk];
  double * numeratorErr = new double[nWrk];
  double * denominator  = new double[nWrk];
  for (int k=0;k<nWrk;++k)
    {
    numerator[k]    = 0;
    numeratorErr[k] = 0;
    denominator[k]  = 0;
    }

  String name = target->name();

  i=1;
  for (iEta=0;iEta<nEtaBins; ++iEta)
    {
    for (iPhi=0;iPhi<nPhiBins; ++iPhi)
      {
      j=1;
      for (jEta=0;jEta<nEtaBins; ++jEta)
        {
        for (jPhi=0;jPhi<nPhiBins; ++jPhi)
          {
          dPhi = iPhi-jPhi; if (dPhi<0) dPhi += nPhiBins; dPhi+=1;
          dEta = iEta-jEta + nEtaBins;
          v1   = binContent(source,i, j);
          ev1  = binError(source,  i, j);
          index = (dEta-1)*nPhiBins + dPhi-1;
          numerator[index]    += v1;
          numeratorErr[index] += ev1*ev1;
          denominator[index]  += 1.;
          //cout << " " << name << "  iEta:" << iEta << " iPhi:" << iPhi << " jEta:" << jEta << " jPhi:" << jPhi << " v1:" << v1 << " ev1:" << ev1 << endl;
          ++j;
          }
        }
      ++i;
      }
    }
  for (dEta=0;dEta<2*nEtaBins-1;++dEta)
    {
    for (dPhi=0;dPhi<nPhiBins;++dPhi)
      {
      //v1   = binContent(target,dEta+1,dPhi+1);
      //ev1  = binError(target,dEta+1,dPhi+1);
      index = dEta*nPhiBins + dPhi;
      v1    = numerator[index];
      ev1   = numeratorErr[index];
      v2    = denominator[index];
      setBinContent(target,dEta+1,dPhi+1,v1/v2);
      setBinError(target,  dEta+1,dPhi+1,std::sqrt(ev1)/v2);
      }
    }
  delete [] numerator;
  delete [] numeratorErr;
  delete [] denominator;
  }

  // ported from RootHistogramHelpers.cpp line 2858
  //!
  //! Calculate the external product of n_1(eta,phi) by n_2(eta,phi) and project onto n1n1_12(Deta,Dphi)
  //! First create a 4D external product of h_1 and h_2 using a 1D std::vector
  //! than project it onto DeltaEta(x-axis) DeltaPhi (y-axis)
  //! Errors are neglected and must be accounted for using a sub-sample analysis.
  //!
  void reduce_n1EtaPhiN1EtaPhiOntoN1N1DetaDphi(const Histogram * h_1, Histogram * h_2, Histogram * h_12, int nDeta,int nDphi)
  {
  { /* no ops */};
  if (!ptrExist(__FUNCTION__,h_1,h_2,h_12)) throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,h_1,h_2)) throw IncompatibleHistogramException(__FUNCTION__);
  std::vector<double> numerator(nDeta*nDphi,0.0);
  std::vector<double> denominator(nDeta*nDphi,0.0);
  int nEta = nBinsX(h_1);
  int nPhi = nBinsY(h_1);
  nDeta = nBinsX(h_12);
  nDphi = nBinsY(h_12);

  int index;
  int iDeta, iDphi;
  double v1, v2, v, r;
  for (int iPhi1=1; iPhi1<=nPhi; iPhi1++)
    {
    for (int iEta1=1; iEta1<=nEta; iEta1++)
      {
      v1 = binContent(h_1,iEta1,iPhi1);
      for (int iPhi2=1; iPhi2<=nPhi; iPhi2++)
        {
        for (int iEta2=1; iEta2<=nEta; iEta2++)
          {
          v2 = binContent(h_2,iEta2,iPhi2);
          v  = v1 * v2;
          iDphi = iPhi1-iPhi2; if (iDphi<0) iDphi += nDphi; //dPhi+=1;
          iDeta = iEta1-iEta2 + nEta - 1;
          // if (iDphi<0 || )
          index = iDphi*nDeta + iDeta;
          if (index<0)
            {
            throw HistogramException("LOGIC","index<0","reduce_n1EtaPhiN1EtaPhiOntoN1N1DetaDphi()");
            }
          if (index>=nDeta*nDphi)
            {
            throw HistogramException("LOGIC","index>=nDeta*nDphi","reduce_n1EtaPhiN1EtaPhiOntoN1N1DetaDphi()");
            }
          numerator[index]    += v;
          denominator[index]  += 1.0;
          }
        }
      }
    }
  double zero = 0.0;
  for (iDphi=0; iDphi<nDphi; iDphi++)
    {
    for (iDeta=0; iDeta<nDeta; iDeta++)
      {
      index = iDphi*nDeta + iDeta;
      double num = numerator[index];
      double den = denominator[index];
      if (den>0) r = num;
      else  r = zero;
      setBinContent(h_12,iDeta+1,iDphi+1,r);
      setBinError(h_12,iDeta+1,iDphi+1,zero);
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 2919
  void reduce_n2xEtaPhi_n2EtaEta(const Histogram * source, Histogram * target,int nEtaBins,int nPhiBins)
  {
  if (!ptrExist(__FUNCTION__,source,target)) throw NullPointerException(__FUNCTION__);
  double v1,v2,ev1,ev2,v,ev;
  int iPhi,iEta,jPhi,jEta, i, j;
  int nBins = nEtaBins*nPhiBins;
  int nEtaEta = nEtaBins*nEtaBins;
  double * work = new double[nEtaEta];
  for (int k=0;k<nEtaEta;++k) work[k]=0;
  i=0;
  for (iEta=0;iEta<nEtaBins; ++iEta)
    {
    for (iPhi=0;iPhi<nPhiBins; ++iPhi)
      {
      j=0;
      for (jEta=0;jEta<nEtaBins; ++jEta)
        {
        for (jPhi=0;jPhi<nPhiBins; ++jPhi)
          {
          v1   = binContent(source,i*nBins+j+1);
          ev1  = binError(source,i*nBins+j+1);
          if (v1> -0.9999999)
            {
            v2   = binContent(target,iEta+1,jEta+1);
            ev2  = binError(target,iEta+1,jEta+1);
            v = v1+v2;
            ev = std::sqrt(ev1*ev1+ev2*ev2);
            setBinContent(target,iEta+1,jEta+1,v);
            setBinError(target,iEta+1,jEta+1,ev);
            work[ iEta*nEtaBins+jEta] += 1;
            }
          ++j;
          }
        }
      ++i;
      }
    }
  for (iEta=0;iEta<nEtaBins; ++iEta)
    {
    for (jEta=0;jEta<nEtaBins; ++jEta)
      {
      v1   = binContent(target,iEta+1,jEta+1);
      ev1  = binError(target,iEta+1,jEta+1);
      v2   = work[ iEta*nEtaBins+jEta ];
      if (v2<=0)throw HistogramException("LOGIC","v2<=0","reduce_n1EtaPhiN1EtaPhiOntoN1N1DetaDphi() index>nDeta*nDphi");
      setBinContent(target,iEta+1,jEta+1,v1/v2);
      setBinError(target,iEta+1,jEta+1,ev1/v2);
      }
    }
  delete [] work;
  }

  // ported from RootHistogramHelpers.cpp line 2971
  void project_n2XYXY_n2XX(const Histogram * source, Histogram * target,int nXBins,int nYBins)
  {
  if (!ptrExist(__FUNCTION__,source,target)) throw NullPointerException(__FUNCTION__);

  double v1,v2,ev1,ev2,v,ev;

  /* sanity checks */
  if ((nBinsX(source) != nXBins*nYBins)
      or (nBinsY(source) != nXBins*nYBins)
      or (nBinsX(target) != nXBins)
      or (nBinsY(target) != nXBins)) throw IncompatibleHistogramException(__FUNCTION__);
  target->reset();
  int i=0;
  for (int iX=0;iX<nXBins; ++iX)
    {
    for (int iY=0;iY<nYBins; ++iY)
      {
      int j=0;
      for (int jX=0;jX<nXBins; ++jX)
        {
        for (int jY=0;jY<nYBins; ++jY)
          {
          v1   = binContent(source,i+1,j+1);
          ev1  = binError(source,i+1,j+1);
          v2   = binContent(target,iX+1,jX+1);
          ev2  = binError(target,iX+1,jX+1);
          v = v1+v2;
          ev = std::sqrt(ev1*ev1+ev2*ev2);
          setBinContent(target,iX+1,jX+1,v);
          setBinError(target,iX+1,jX+1,ev);
          ++j;
          }
        }
      ++i;
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 3010
  void project_n2XYXY_n2YY(const Histogram * source, Histogram * target,int nXBins,int nYBins)
  {
  if (!ptrExist(__FUNCTION__,source,target)) throw NullPointerException(__FUNCTION__);
  double v1,v2,ev1,ev2,v,ev;

  /* sanity checks */
  if ((nBinsX(source) != nXBins*nYBins) ||
      (nBinsY(source) != nXBins*nYBins) ||
      (nBinsX(target) != nYBins) ||
      (nBinsY(target) != nYBins)) throw IncompatibleHistogramException(__FUNCTION__);
  target->reset();
  int i=0;
  for (int iX=0;iX<nXBins; ++iX)
    {
    for (int iY=0;iY<nYBins; ++iY)
      {
      int j=0;
      for (int jX=0;jX<nXBins; ++jX)
        {
        for (int jY=0;jY<nYBins; ++jY)
          {
          v1   = binContent(source,i+1,j+1);
          ev1  = binError(source,i+1,j+1);
          v2   = binContent(target,iY+1,jY+1);
          ev2  = binError(target,iY+1,jY+1);
          v = v1+v2;
          ev = std::sqrt(ev1*ev1+ev2*ev2);
          setBinContent(target,iY+1,jY+1,v);
          setBinError(target,iY+1,jY+1,ev);
          ++j;
          }
        }
      ++i;
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 3050
  //! Shift the given source to the target vertically by nbins
  void shiftY(const Histogram & source, Histogram & target, int nbins)
  {
  int i_x, i_y;
  int n_x = nBinsX((&source));
  int n_y = nBinsY((&source));

  //shift the 1st area
  for (i_x=1;i_x<=n_x;++i_x)
    {
    for (i_y=1;i_y<=n_y-nbins;++i_y)
      {
      double v  = binContent((&source),i_x,i_y);
      double ev = binError((&source),i_x,i_y);
      setBinContent((&target),i_x,  i_y+nbins, v);
      setBinError((&target),i_x,    i_y+nbins, ev);
      }
    for (i_y=n_y-nbins+1;i_y<=n_y;++i_y)
      {
      double v  = binContent((&source),i_x,i_y);
      double ev = binError((&source),i_x,i_y);
      setBinContent((&target),i_x, i_y-(n_y-nbins),v);
      setBinError((&target),i_x,   i_y-(n_y-nbins),ev);
      }
    }
  }

  // ported from RootHistogramHelpers.cpp line 3106
  void calculateF1(Histogram* h_f1_1, Histogram* h_F1_1)
  {
  if (!ptrExist(__FUNCTION__,h_f1_1, h_F1_1)) throw NullPointerException(__FUNCTION__);
  double f1_1, ef1_1;
  int nBins = nBinsX(h_f1_1);
  for (int iBin = 1; iBin <=nBins; iBin++)
    {
    f1_1  = binContent(h_f1_1,iBin);
    ef1_1 = binError(h_f1_1,iBin);
    setBinContent(h_F1_1,iBin, f1_1);
    setBinError(h_F1_1,iBin,   ef1_1);
    }
  }

  // ported from RootHistogramHelpers.cpp line 3121
  void calculateF2R2(Histogram* h_f1_1, Histogram* h_f1_2, Histogram* h_f2_12, Histogram* h_F2_12, Histogram* h_R2_12)
  {
  if (!ptrExist(__FUNCTION__,h_f1_1, h_f1_2, h_f2_12, h_F2_12, h_R2_12)) throw NullPointerException(__FUNCTION__);
  double f1_1, ef1_1, f1_2, ef1_2;
  double f2_12, ef2_12;
  double F2_12, eF2_12, R2_12, eR2_12;
  int nBins = nBinsX(h_f1_1);
  for (int iBin = 1; iBin <=nBins; iBin++)
    {
    f1_1   = binContent(h_f1_1,iBin);   ef1_1 =   binError(h_f1_1,iBin);
    f1_2   = binContent(h_f1_2,iBin);   ef1_2  =  binError(h_f1_2,iBin);
    f2_12  = binContent(h_f2_12,iBin);  ef2_12 =  binError(h_f2_12,iBin);
    calculateF2R2(f1_1,ef1_1,f1_2,ef1_2,f2_12,ef2_12, F2_12,eF2_12,R2_12,eR2_12);
    setBinContent(h_F2_12,iBin, F2_12);
    setBinError(h_F2_12,iBin,   0.0);
    setBinContent(h_R2_12,iBin, R2_12);
    setBinError(h_R2_12,iBin,   0.0);
    }
  }

  // ported from RootHistogramHelpers.cpp line 3244
  void calculateF2R2(double f1_1,double ef1_1,
                     double f1_2,double ef1_2,
                     double f2_12,double ef2_12,
                     double & F2_12,double & eF2_12,
                     double & R2_12,double & eR2_12)
  {

  double ref1_1, ref1_2, ref2_12;
  if (f1_1<1E-20 || f1_2<1E-20)
    {
    F2_12  = 0.0;
    eF2_12 = 0.0;
    R2_12  = 0.0;
    eR2_12 = 0.0;
    }
  else
    {
    F2_12     = f2_12 - f1_1*f1_2;
    ref1_1    = ef1_1/f1_1;
    ref1_2    = ef1_2/f1_2;
    ref2_12   = ef2_12/f2_12;
    eF2_12    = std::sqrt(f1_1*f1_2*f1_1*f1_2*(ref1_1*ref1_1 + ref1_2*ref1_2) + eF2_12*eF2_12);
    R2_12     = F2_12/(f1_1*f1_2);
    eR2_12    = R2_12*std::sqrt(ref1_1*ref1_1 + ref1_2*ref1_2 + ref2_12*ref2_12);
    }
  }

  // ported from RootHistogramHelpers.cpp line 3141
  void calculateNudyn(Histogram* h_R2_11,
                      Histogram* h_R2_12,
                      Histogram* h_R2_21,
                      Histogram* h_R2_22,
                      Histogram* h_nudyn_12)
  {
  if (!ptrExist(__FUNCTION__,h_R2_11, h_R2_12, h_R2_21, h_R2_22, h_nudyn_12)) throw NullPointerException(__FUNCTION__);
  double R2_11, eR2_11, R2_12, eR2_12, R2_21, eR2_21, R2_22, eR2_22, nudyn, enudyn;
  int nBins = nBinsX(h_R2_11);
  for (int iBin = 1; iBin <=nBins; iBin++)
    {
    R2_11   = binContent(h_R2_11,iBin);  eR2_11  =  binError(h_R2_11,iBin);
    R2_12   = binContent(h_R2_12,iBin);  eR2_12  =  binError(h_R2_12,iBin);
    R2_21   = binContent(h_R2_21,iBin);  eR2_21  =  binError(h_R2_21,iBin);
    R2_22   = binContent(h_R2_22,iBin);  eR2_22  =  binError(h_R2_22,iBin);
    calculateNudyn(R2_11, eR2_11, R2_12, eR2_12, R2_21, eR2_21, R2_22, eR2_22, nudyn, enudyn);
    setBinContent(h_nudyn_12,iBin, nudyn);
    setBinError(h_nudyn_12,iBin,   0.0);
    }
  }

  // ported from RootHistogramHelpers.cpp line 3364
  void calculateNudyn(double r2_11,double er2_11,
                      double r2_12,double er2_12,
                      double r2_21,double er2_21,
                      double r2_22,double er2_22,
                      double & nudyn,double & enudyn)
  {
  nudyn = r2_11 + r2_22 - r2_12 - r2_21;
  enudyn = std::sqrt(er2_11*er2_11 + er2_22*er2_22 + er2_12*er2_12 + er2_21*er2_21);
  }

  // ported from RootHistogramHelpers.cpp line 3163
  void calculateF3R3(Histogram* h_f1_1, Histogram* h_f1_2, Histogram* h_f1_3,
                     Histogram* h_f2_12, Histogram* h_f2_13, Histogram* h_f2_23,
                     Histogram* h_f3_123,
                     Histogram* h_F3_123, Histogram* h_R3_123)
  {

  if (!ptrExist(__FUNCTION__,h_f1_1, h_f1_2, h_f1_3, h_f2_12, h_f2_13, h_f2_23, h_f3_123, h_F3_123, h_R3_123))  throw NullPointerException(__FUNCTION__);

  double f1_1, ef1_1, f1_2, ef1_2, f1_3, ef1_3;
  double f2_12, ef2_12, f2_13, ef2_13, f2_23, ef2_23;
  double f3_123, ef3_123;
  double F3_123, eF3_123, R3_123, eR3_123;
  int nBins = nBinsX(h_f1_1);
  for (int iBin = 1; iBin <=nBins; iBin++)
    {
    f1_1   = binContent(h_f1_1,iBin);    ef1_1 =   binError(h_f1_1,iBin);
    f1_2   = binContent(h_f1_2,iBin);    ef1_2  =  binError(h_f1_2,iBin);
    f1_3   = binContent(h_f1_3,iBin);    ef1_3  =  binError(h_f1_2,iBin);
    f2_12  = binContent(h_f2_12,iBin);   ef2_12 =  binError(h_f2_12,iBin);
    f2_13  = binContent(h_f2_13,iBin);   ef2_13 =  binError(h_f2_13,iBin);
    f2_23  = binContent(h_f2_23,iBin);   ef2_23 =  binError(h_f2_23,iBin);
    f3_123 = binContent(h_f3_123,iBin);  ef3_123 =  binError(h_f3_123,iBin);
    calculateF3R3(f1_1,ef1_1,f1_2,ef1_2,f1_3,ef1_3,
                  f2_12,ef2_12,f2_13,ef2_13,f2_23,ef2_23,
                  f3_123,ef3_123,
                  F3_123,eF3_123,R3_123,eR3_123);
    setBinContent(h_F3_123,iBin, F3_123);
    setBinError(h_F3_123,iBin,   0.0);
    setBinContent(h_R3_123,iBin, R3_123);
    setBinError(h_R3_123,iBin,   0.0);
    }
  }

  // ported from RootHistogramHelpers.cpp line 3272
  void calculateF3R3(double f1_1,double ef1_1,double f1_2,double ef1_2, double f1_3,double ef1_3,
                     double f2_12,double ef2_12,double f2_13,double ef2_13,double f2_23,double ef2_23,
                     double f3_123, double ef3_123,
                     double & F3_123,double & eF3_123, double & R3_123,double & eR3_123)
  {

  double ref1_1, ref1_2, ref1_3, ref2_12, ref2_13, ref2_23, reF3_123;
  if (f1_1<1E-20 || f1_2<1E-20 || f1_3<1E-20)
    {
    F3_123   = 0.0;
    eF3_123  = 0.0;
    R3_123   = 0.0;
    eR3_123  = 0.0;
    }
  else
    {
    F3_123    = f3_123 - f2_12*f1_3 - f2_13*f1_2 - f2_23*f1_1 + 2.0*f1_1*f1_2*f1_3;
    ref1_1    = ef1_1/f1_1;
    ref1_2    = ef1_2/f1_2;
    ref1_3    = ef1_3/f1_3;
    ref2_12   = ef2_12/f2_12;
    ref2_13   = ef2_13/f2_13;
    ref2_23   = ef2_23/f2_23;
    eF3_123   = ef3_123*ef3_123;
    eF3_123   += f2_12*f2_12*f1_3*f1_3*(ref2_12*ref2_12 + ref1_3*ref1_3);
    eF3_123   += f2_13*f2_13*f1_2*f1_2*(ref2_13*ref2_13 + ref1_2*ref1_2);
    eF3_123   += f2_23*f2_23*f1_1*f1_1*(ref2_23*ref2_23 + ref1_1*ref1_1);
    eF3_123   += 4.0*f1_1*f1_1*f1_2*f1_2*f1_3*f1_3*std::sqrt(ref1_1*ref1_1+ref1_2*ref1_2+ref1_3*ref1_3);
    eF3_123   = std::sqrt(eF3_123);
    reF3_123   = eF3_123/F3_123;
    R3_123     = F3_123/(f1_1*f1_2*f1_3);
    eR3_123    = R3_123*reF3_123;
    }
  }

  // ported from RootHistogramHelpers.cpp line 3196
  void calculateF4R4(Histogram* h_f1_1, Histogram* h_f1_2, Histogram* h_f1_3, Histogram* h_f1_4,
                     Histogram* h_f2_12, Histogram* h_f2_13, Histogram* h_f2_14,
                     Histogram* h_f2_23, Histogram* h_f2_24, Histogram* h_f2_34,
                     Histogram* h_f3_123, Histogram* h_f3_124, Histogram* h_f3_134, Histogram* h_f3_234,
                     Histogram* h_f4_1234,
                     Histogram* h_F4_1234, Histogram* h_R4_1234)
  {

  if (!ptrExist(__FUNCTION__,h_f1_1, h_f1_2, h_f1_3, h_f1_4, h_f2_12,  h_f2_13,  h_f2_14)) throw NullPointerException(__FUNCTION__);
  if (!ptrExist(__FUNCTION__,h_f2_23,h_f2_24,h_f2_34,h_f3_123,h_f3_124,h_f3_134,h_f3_234)) throw NullPointerException(__FUNCTION__);
  if (!ptrExist(__FUNCTION__,h_f4_1234,h_F4_1234,h_R4_1234)) throw NullPointerException(__FUNCTION__);
  double f1_1, ef1_1, f1_2, ef1_2, f1_3, ef1_3, f1_4, ef1_4;
  double f2_12, ef2_12, f2_13, ef2_13, f2_14, ef2_14, f2_23, ef2_23, f2_24, ef2_24, f2_34, ef2_34;
  double f3_123, ef3_123, f3_124, ef3_124, f3_134, ef3_134, f3_234, ef3_234;
  double f4_1234, ef4_1234;
  double F4_1234, eF4_1234, R4_1234, eR4_1234;
  int nBins = nBinsX(h_f1_1);
  for (int iBin = 1; iBin <=nBins; iBin++)
    {
    f1_1    = binContent(h_f1_1,iBin);   ef1_1   =  binError(h_f1_1,iBin);
    f1_2    = binContent(h_f1_2,iBin);   ef1_2   =  binError(h_f1_2,iBin);
    f1_3    = binContent(h_f1_3,iBin);   ef1_3   =  binError(h_f1_3,iBin);
    f1_4    = binContent(h_f1_4,iBin);   ef1_4   =  binError(h_f1_4,iBin);
    f2_12   = binContent(h_f2_12,iBin);  ef2_12  =  binError(h_f2_12,iBin);
    f2_13   = binContent(h_f2_13,iBin);  ef2_13  =  binError(h_f2_13,iBin);
    f2_14   = binContent(h_f2_14,iBin);  ef2_14  =  binError(h_f2_14,iBin);
    f2_23   = binContent(h_f2_23,iBin);  ef2_23  =  binError(h_f2_23,iBin);
    f2_24   = binContent(h_f2_24,iBin);  ef2_24  =  binError(h_f2_24,iBin);
    f2_34   = binContent(h_f2_34,iBin);  ef2_34  =  binError(h_f2_34,iBin);
    f3_123  = binContent(h_f3_123,iBin); ef3_123 =  binError(h_f3_123,iBin);
    f3_124  = binContent(h_f3_124,iBin); ef3_124 =  binError(h_f3_124,iBin);
    f3_134  = binContent(h_f3_134,iBin); ef3_134 =  binError(h_f3_134,iBin);
    f3_234  = binContent(h_f3_234,iBin); ef3_234 =  binError(h_f3_234,iBin);
    f4_1234 = binContent(h_f4_1234,iBin);ef4_1234 =  binError(h_f4_1234,iBin);
    calculateF4R4(f1_1,ef1_1,f1_2,ef1_2,f1_3,ef1_3,f1_4,ef1_4,
                  f2_12,ef2_12, f2_13,ef2_13, f2_14,ef2_14,
                  f2_23,ef2_23, f2_24,ef2_24, f2_34,ef2_34,
                  f3_123,ef3_123, f3_124,ef3_124, f3_134,ef3_134, f3_234,ef3_234,
                  f4_1234,ef4_1234,
                  F4_1234,eF4_1234,R4_1234,eR4_1234);
    setBinContent(h_F4_1234,iBin, F4_1234);
    setBinError(h_F4_1234,iBin,   0.0);
    setBinContent(h_R4_1234,iBin, R4_1234);
    setBinError(h_R4_1234,iBin,   0.0);
    }
  }

  // ported from RootHistogramHelpers.cpp line 3307
  void calculateF4R4(double f1_1,double ef1_1,double f1_2,double ef1_2, double f1_3,double ef1_3, double f1_4,double ef1_4,
                     double f2_12,double ef2_12,double f2_13,double ef2_13,double f2_14,double ef2_14,double f2_23,double ef2_23,
                     double f2_24,double ef2_24,double f2_34,double ef2_34,
                     double f3_123, double ef3_123,double f3_124, double ef3_124, double f3_134, double ef3_134,double f3_234, double ef3_234,
                     double f4_1234, double ef4_1234,
                     double & F4_1234,double & eF4_1234, double & R4_1234,double & eR4_1234)
  {
  double ref1_1,  ref1_2,  ref1_3; //,  ref1_4;
  double ref2_12, ref2_13, ref2_14;
  double ref3_123, ref3_124, ref3_134, ref3_234;
  if (f1_1<1E-20 || f1_2<1E-20 || f1_3<1E-20 || f1_4<1E-20)
    {
    F4_1234   = 0.0;
    eF4_1234  = 0.0;
    R4_1234   = 0.0;
    eR4_1234  = 0.0;
    }
  else
    {

    F4_1234   = f4_1234 - f3_123*f1_4 - f3_124*f1_3 - f3_134*f1_2 - f3_234*f1_1
    - f2_12*f2_34 - f2_13*f2_24 - f2_14*f2_23
    + 4.0*(f2_12*f1_3*f1_4 + f2_13*f1_2*f1_4 + f2_14*f1_2*f1_3)
    - 6.0*f1_1*f1_2*f1_3*f1_4;
    ref1_1    = ef1_1/f1_1;
    ref1_2    = ef1_2/f1_2;
    ref1_3    = ef1_3/f1_3;
    //ref1_4    = ef1_4/f1_4;
    ref2_12   = ef2_12/f2_12;
    ref2_13   = ef2_13/f2_13;
    ref2_14   = ef2_14/f2_14;
    //ref2_23   = ef2_23/f2_23;
    //ref2_24   = ef2_24/f2_24;
    //ref2_34   = ef2_34/f2_34;
    ref3_123  = ef3_123/f3_123;
    ref3_124  = ef3_124/f3_124;
    ref3_134  = ef3_134/f3_134;
    ref3_234  = ef3_234/f3_234;
    //ref4_1234 = ef4_1234/f4_1234;

    eF4_1234   = ef4_1234*ef4_1234
    + f3_123*f3_123*f1_4*f1_4*(ref3_123*ref3_123 + ef1_4*ef1_4)
    + f3_124*f3_124*f1_3*f1_3*(ref3_124*ref3_124 + ref1_3*ref1_3)
    + f3_134*f3_134*f1_2*f1_2*(ref3_134*ref3_134 + ref1_2*ref1_2)
    + f3_234*f3_234*f1_1*f1_1*(ref3_234*ref3_234 + ref1_1*ref1_1)
    + 16.0*f2_12*f2_12*f2_34*f2_34*(ref2_12*ref2_12 + ef2_34*ef2_34)
    + 16.0*f2_13*f2_13*f2_24*f2_24*(ref2_13*ref2_13 + ef2_24*ef2_24)
    + 16.0*f2_14*f2_14*f2_23*f2_23*(ref2_14*ref2_14 + ef2_23*ef2_23);
    // neglect singles term for now..
    eF4_1234   = std::sqrt(eF4_1234);
    //reF4_1234  = 0.0; //eF4_1234/F4_1234;
    R4_1234    = F4_1234/(f1_1*f1_2*f1_3*f1_4);
    eR4_1234   = 0.0; //R4_1234*reF4_1234;
    }
  }

  // ported from RootHistogramHelpers.cpp line 4391
  void printHistoInfo(const String & source,
                      const String & histoName,
                      int x_nbins, double x_min, double x_max,
                      int y_nbins, double y_min, double y_max,
                      int z_nbins, double min_z, double max_z)
  {
  printValue("Caller",source);
  printValue("Histogram Name",histoName);
  printValue("x_nbins",x_nbins);
  printValue("x_min",x_min);
  printValue("x_max",x_max);
  if (y_nbins<1) return;
  printValue("y_nbins",y_nbins);
  printValue("y_min",y_min);
  printValue("y_max",y_max);
  if (z_nbins<1) return;
  printValue("z_nbins",z_nbins);
  printValue("min_z",min_z);
  printValue("max_z",max_z);
  }

  
  //////////////////////////////////////////////////////////////////////////
  // Mean and random sampling
  //////////////////////////////////////////////////////////////////////////

  double Histogram::mean(unsigned int dimIndex) const
  {
  checkDimIndex(dimIndex,__FUNCTION__);
  double sum = 0.0, sumX = 0.0;
  std::vector<unsigned int> indices(_rank,0);
  for (unsigned int k=0; k<_nBinsTotal; k++)
    {
    // axis index of global bin k along dimIndex: k = i0 + n0*(i1 + n1*i2 ...)
    unsigned int rest = k;
    for (unsigned int d=0; d<dimIndex; d++) rest /= _nBins[d];
    unsigned int i = rest % _nBins[dimIndex];
    double v = value(k);
    sum  += v;
    sumX += v*binCenter(i,dimIndex);
    }
  return sum==0.0 ? 0.0 : sumX/sum;
  }

  void Histogram::random(double & x) const
  {
  checkRank(1,__FUNCTION__);
  unsigned int n = nBinsTotal();
  std::vector<double> cumulative(n);
  double sum = 0.0;
  for (unsigned int k=0; k<n; k++) { double v = value(k); if (v>0) sum += v; cumulative[k] = sum; }
  if (sum<=0) throw HistogramException(name(),"Cannot draw random values from an empty histogram",__FUNCTION__);
  double r = uniform01()*sum;
  unsigned int k = std::upper_bound(cumulative.begin(),cumulative.end(),r) - cumulative.begin();
  if (k>=n) k = n-1;
  x = binLowEdge(k,0) + uniform01()*binWidth(k,0);
  }

  void Histogram::random(double & x, double & y) const
  {
  getRandom2(this,x,y);
  }

} // namespace MATH
} // namespace CAP
