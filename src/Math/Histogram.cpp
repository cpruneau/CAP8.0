#include "Histogram.hpp"
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include "BinaryIO.hpp"

ClassImp(CAP::MATH::Histogram);

namespace CAP
{
  namespace MATH
  {

  //////////////////////////////////////////////////////////////////////////
  // Construction / assignment
  //////////////////////////////////////////////////////////////////////////

  Histogram::Histogram()
  :
  Object("H","H","Histogram"),
  Partition(),
  _entries(),
  _values(),
  _uncertainties()
  {
  addDimension(1,0.0,1.0,"x");
  resizeBins();
  }

  Histogram::Histogram(const Partition & partition)
  :
  Object("H","H","Histogram"),
  Partition(partition),
  _entries(),
  _values(),
  _uncertainties()
  {
  resizeBins();
  }

  Histogram::Histogram(const Histogram & rhs)
  :
  Object(rhs),
  Partition(rhs),
  _entries(rhs._entries),
  _values(rhs._values),
  _uncertainties(rhs._uncertainties)
  {  }

  Histogram::Histogram(const String & name, const String & title, unsigned int n, double minimum, double maximum)
  :
  Object(name,title,"Histogram"),
  Partition(),
  _entries(),
  _values(),
  _uncertainties()
  {
  addDimension(n,minimum,maximum,"x");
  resizeBins();
  }

  Histogram::Histogram(const String & name, const String & title,
                       unsigned int n0, double minimum_0, double maximum_0,
                       unsigned int n1, double minimum_1, double maximum_1)
  :
  Object(name,title,"Histogram"),
  Partition(),
  _entries(),
  _values(),
  _uncertainties()
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
  Object(name,title,"Histogram"),
  Partition(),
  _entries(),
  _values(),
  _uncertainties()
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
    Object::operator=(rhs);
    Partition::operator=(rhs);
    _entries       = rhs._entries;
    _values        = rhs._values;
    _uncertainties = rhs._uncertainties;
    }
  return *this;
  }

  //////////////////////////////////////////////////////////////////////////
  // Internal helpers
  //////////////////////////////////////////////////////////////////////////

  void Histogram::resizeBins()
  {
  unsigned int n = nBinsTotal();
  _entries.assign(n,0);
  _values.assign(n,0.0);
  _uncertainties.assign(n,0.0);
  }

  void Histogram::setEmpty(unsigned int k)
  {
  _entries[k]       = 0;
  _values[k]        = 0.0;
  _uncertainties[k] = 0.0;
  }

  void Histogram::setTotal(unsigned int k, long entries, double total, double totalUncertainty)
  {
  if (entries<=0)
    {
    setEmpty(k);
    return;
    }
  _entries[k]       = entries;
  _values[k]        = total/double(entries);
  _uncertainties[k] = totalUncertainty/double(entries);
  }

  void Histogram::assignContent(const Histogram & h)
  {
  if (&h==this) return;
  Partition::operator=(h);
  _entries       = h._entries;
  _values        = h._values;
  _uncertainties = h._uncertainties;
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
  double t = total(kTarget) + h.total(kSource);
  double e = statErrorSum(totalUncertainty(kTarget),h.totalUncertainty(kSource));
  setTotal(kTarget,n,t,e);
  }

  //////////////////////////////////////////////////////////////////////////
  // Comparison
  //////////////////////////////////////////////////////////////////////////

  //
  //Check if the two histograms have bin totals equal within 2 combined standard deviations.
  //Histograms with different binning are not equal.
  //
  bool  Histogram::operator== (const Histogram & rhs) const
  {
  if (!compatibleWith(rhs)) return false;
  unsigned int n = _entries.size();
  for (unsigned int k=0; k<n; k++)
    {
    double diff        = total(k) - rhs.total(k);
    double uncertainty = statErrorSum(totalUncertainty(k),rhs.totalUncertainty(k));
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
    _values[k]        *= coefficient;
    _uncertainties[k] *= absCoefficient;
    }
  }

  // add a constant to every bin total; empty bins become bins with one entry
  void Histogram::add(double value)
  {
  unsigned int n = _entries.size();
  for (unsigned int k=0; k<n; k++)
    {
    long entries = _entries[k]>0 ? _entries[k] : 1;
    setTotal(k,entries,total(k)+value,totalUncertainty(k));
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
    double value       = total(k) + coefficient*h.total(k);
    double uncertainty = statErrorSum(totalUncertainty(k),coefficient*h.totalUncertainty(k));
    setTotal(k,entries,value,uncertainty);
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
    double v1 = total(k);
    double e1 = totalUncertainty(k);
    double v2 = coefficient*h.total(k);
    double e2 = std::fabs(coefficient)*h.totalUncertainty(k);
    setTotal(k,1,v1*v2,statErrorProduct(v1,e1,v2,e2));
    }
  }

  // this = this / (coefficient*h); bins where the divisor vanishes are set empty
  void Histogram::divideBy(const Histogram & h, double coefficient)
  {
  checkCompatible(h,__FUNCTION__);
  unsigned int n = _entries.size();
  for (unsigned int k=0; k<n; k++)
    {
    double v2 = coefficient*h.total(k);
    if (_entries[k]==0 || h._entries[k]==0 || v2==0.0) { setEmpty(k); continue; }
    double v1 = total(k);
    double e1 = totalUncertainty(k);
    double e2 = std::fabs(coefficient)*h.totalUncertainty(k);
    setTotal(k,1,v1/v2,statErrorRatio(v1,e1,v2,e2));
    }
  }

  // replace every bin total by its inverse; empty or zero bins are set empty
  void  Histogram::inverse()
  {
  unsigned int n = _entries.size();
  for (unsigned int k=0; k<n; k++)
    {
    double value = total(k);
    if (_entries[k]==0 || value==0.0) { setEmpty(k); continue; }
    double inv            = 1.0/value;
    double invUncertainty = totalUncertainty(k)*inv*inv;
    setTotal(k,1,inv,invUncertainty);   // entry count is no longer meaningful
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
  // Bin setters (value and uncertainty are bin totals)
  //////////////////////////////////////////////////////////////////////////

  // set the entry count, keeping the bin total and its uncertainty
  void Histogram::doSetEntries(unsigned int k, long entries)
  {
  double t = total(k);
  double e = totalUncertainty(k);
  if (entries<=0 && (t!=0.0 || e!=0.0))
    throw Exception("entries must be >0 for a bin with non-zero content",__FUNCTION__);
  setTotal(k,entries,t,e);
  }

  // set the bin total, keeping the entry count (1 if the bin was empty)
  void Histogram::doSetValue(unsigned int k, double value)
  {
  long entries = _entries[k]>0 ? _entries[k] : 1;
  setTotal(k,entries,value,totalUncertainty(k));
  }

  // set the uncertainty of the bin total, keeping the entry count (1 if the bin was empty)
  void Histogram::doSetUncertainty(unsigned int k, double uncertainty)
  {
  long entries = _entries[k]>0 ? _entries[k] : 1;
  setTotal(k,entries,total(k),uncertainty);
  }

  void Histogram::doSetBin(unsigned int k, long entries, double value, double uncertainty)
  {
  if (entries<=0 && (value!=0.0 || uncertainty!=0.0))
    throw Exception("entries must be >0 for a bin with non-zero content",__FUNCTION__);
  setTotal(k,entries,value,uncertainty);
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
  // setWith: values and uncertainties are bin totals
  //////////////////////////////////////////////////////////////////////////

  template<typename E, typename V, typename U>
  void Histogram::setWithTotals(const std::vector<E> * entries, const std::vector<V> & values,
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
  { setWithTotals<long,float,float>(nullptr,values,nullptr,__FUNCTION__); }
  void Histogram::setWith(const std::vector<double> & values)
  { setWithTotals<long,double,double>(nullptr,values,nullptr,__FUNCTION__); }
  void Histogram::setWith(const std::vector<float> & values, const std::vector<float> & uncertainties)
  { setWithTotals<long,float,float>(nullptr,values,&uncertainties,__FUNCTION__); }
  void Histogram::setWith(const std::vector<double> & values, const std::vector<double> & uncertainties)
  { setWithTotals<long,double,double>(nullptr,values,&uncertainties,__FUNCTION__); }
  void Histogram::setWith(const std::vector<long> & entries, const std::vector<float> & values)
  { setWithTotals<long,float,float>(&entries,values,nullptr,__FUNCTION__); }
  void Histogram::setWith(const std::vector<long> & entries, const std::vector<double> & values)
  { setWithTotals<long,double,double>(&entries,values,nullptr,__FUNCTION__); }
  void Histogram::setWith(const std::vector<long> & entries, const std::vector<float> & values, const std::vector<float> & uncertainties)
  { setWithTotals<long,float,float>(&entries,values,&uncertainties,__FUNCTION__); }
  void Histogram::setWith(const std::vector<long> & entries, const std::vector<double> & values, const std::vector<double> & uncertainties)
  { setWithTotals<long,double,double>(&entries,values,&uncertainties,__FUNCTION__); }

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
  extracted.setName(name());
  extracted.setTitle(title());
  unsigned int iTarget=0;
  for (unsigned int iSource=f; iSource<=l; iSource++)
    {
    extracted._entries[iTarget]       = _entries[iSource];
    extracted._values[iTarget]        = _values[iSource];
    extracted._uncertainties[iTarget] = _uncertainties[iSource];
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
  extracted.setName(name());
  extracted.setTitle(title());
  for (unsigned int i0=f0; i0<=l0; i0++)
    {
    for (unsigned int i1=f1; i1<=l1; i1++)
      {
      unsigned int indexSource = binIndex(i0,i1);
      unsigned int indexTarget = extracted.binIndex(i0-f0,i1-f1);
      extracted._entries[indexTarget]       = _entries[indexSource];
      extracted._values[indexTarget]        = _values[indexSource];
      extracted._uncertainties[indexTarget] = _uncertainties[indexSource];
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
        extracted._values[indexTarget]        = _values[indexSource];
        extracted._uncertainties[indexTarget] = _uncertainties[indexSource];
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
  //Sum of bin totals, or integral (sum times bin volume) when binSumOnly is false.
  //
  double Histogram::integral(bool binSumOnly) const
  {
  double sum = 0.0;
  for (unsigned int k=0; k<_entries.size(); k++) sum += total(k);
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
  for (unsigned int k=f; k<=l; k++) sum += total(k);
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
      sum += total(binIndex(k0,k1));
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
        sum += total(binIndex(k0,k1,k2));
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
      case 1:  return total(k);
      default: return _values[k];
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
  void Histogram::maximumValue(unsigned int & index, double & value, unsigned int first, unsigned int last) const
  { index = extremumBin(1,true,first,last);  value = total(index); }
  void Histogram::minimumValue(unsigned int & index, double & value, unsigned int first, unsigned int last) const
  { index = extremumBin(1,false,first,last); value = total(index); }
  void Histogram::maximumAvgValue(unsigned int & index, double & value, unsigned int first, unsigned int last) const
  { index = extremumBin(2,true,first,last);  value = _values[index]; }
  void Histogram::minimumAvgValue(unsigned int & index, double & value, unsigned int first, unsigned int last) const
  { index = extremumBin(2,false,first,last); value = _values[index]; }

  double Histogram::maximumEntries(unsigned int first, unsigned int last) const  { return _entries[extremumBin(0,true,first,last)]; }
  double Histogram::minimumEntries(unsigned int first, unsigned int last) const  { return _entries[extremumBin(0,false,first,last)]; }
  double Histogram::maximumValue(unsigned int first, unsigned int last) const    { return total(extremumBin(1,true,first,last)); }
  double Histogram::minimumValue(unsigned int first, unsigned int last) const    { return total(extremumBin(1,false,first,last)); }
  double Histogram::maximumAvgValue(unsigned int first, unsigned int last) const { return _values[extremumBin(2,true,first,last)]; }
  double Histogram::minimumAvgValue(unsigned int first, unsigned int last) const { return _values[extremumBin(2,false,first,last)]; }

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
  _values.clear();
  _uncertainties.clear();
  }

  //
  //Reset the bin content to zero without changing the rank and dimensions of this histogram.
  //
  void Histogram::reset()
  {
  std::fill(_entries.begin(),_entries.end(),0);
  std::fill(_values.begin(),_values.end(),0.0f);
  std::fill(_uncertainties.begin(),_uncertainties.end(),0.0f);
  }

  //
  // Fill: adds one entry of the given weight. The bin total grows by weight and the squared
  // uncertainty of the total grows by weight^2. Out-of-range coordinates are ignored.
  //
  void Histogram::fill(double x, double weight)
  {
  int index = binIndex(x);
  if (index<0) return;
  double e = totalUncertainty(index);
  setTotal(index,_entries[index]+1,total(index)+weight,std::sqrt(e*e+weight*weight));
  }

  void Histogram::fill(double x0, double x1, double weight)
  {
  int index = binIndex(x0,x1);
  if (index<0) return;
  double e = totalUncertainty(index);
  setTotal(index,_entries[index]+1,total(index)+weight,std::sqrt(e*e+weight*weight));
  }

  void Histogram::fill(double x0, double x1, double x2, double weight)
  {
  int index = binIndex(x0,x1,x2);
  if (index<0) return;
  double e = totalUncertainty(index);
  setTotal(index,_entries[index]+1,total(index)+weight,std::sqrt(e*e+weight*weight));
  }

  void Histogram::fill(const std::vector<double> & x, double weight)
  {
  int index = binIndex(x);
  if (index<0) return;
  double e = totalUncertainty(index);
  setTotal(index,_entries[index]+1,total(index)+weight,std::sqrt(e*e+weight*weight));
  }

  //////////////////////////////////////////////////////////////////////////
  // Bin getters (strict index checking)
  //////////////////////////////////////////////////////////////////////////

  double Histogram::binEntriesAt(unsigned int i) const                                   { return _entries[binIndex(i,true)]; }
  double Histogram::binEntriesAt(unsigned int i0, unsigned int i1) const                 { return _entries[binIndex(i0,i1,true)]; }
  double Histogram::binEntriesAt(unsigned int i0, unsigned int i1, unsigned int i2) const{ return _entries[binIndex(i0,i1,i2,true)]; }
  double Histogram::binEntriesAt(const std::vector<unsigned int> & indices) const        { return _entries[binIndex(indices,true)]; }

  double Histogram::binValueAt(unsigned int i) const                                     { return total(binIndex(i,true)); }
  double Histogram::binValueAt(unsigned int i0, unsigned int i1) const                   { return total(binIndex(i0,i1,true)); }
  double Histogram::binValueAt(unsigned int i0, unsigned int i1, unsigned int i2) const  { return total(binIndex(i0,i1,i2,true)); }
  double Histogram::binValueAt(const std::vector<unsigned int> & indices) const          { return total(binIndex(indices,true)); }

  double Histogram::binUncertaintyAt(unsigned int i) const                                    { return totalUncertainty(binIndex(i,true)); }
  double Histogram::binUncertaintyAt(unsigned int i0, unsigned int i1) const                  { return totalUncertainty(binIndex(i0,i1,true)); }
  double Histogram::binUncertaintyAt(unsigned int i0, unsigned int i1, unsigned int i2) const { return totalUncertainty(binIndex(i0,i1,i2,true)); }
  double Histogram::binUncertaintyAt(const std::vector<unsigned int> & indices) const         { return totalUncertainty(binIndex(indices,true)); }

  double Histogram::binAvgValueAt(unsigned int i) const                                   { return _values[binIndex(i,true)]; }
  double Histogram::binAvgValueAt(unsigned int i0, unsigned int i1) const                 { return _values[binIndex(i0,i1,true)]; }
  double Histogram::binAvgValueAt(unsigned int i0, unsigned int i1, unsigned int i2) const{ return _values[binIndex(i0,i1,i2,true)]; }
  double Histogram::binAvgValueAt(const std::vector<unsigned int> & indices) const        { return _values[binIndex(indices,true)]; }

  double Histogram::binAvgValueUncertaintyAt(unsigned int i) const                                   { return _uncertainties[binIndex(i,true)]; }
  double Histogram::binAvgValueUncertaintyAt(unsigned int i0, unsigned int i1) const                 { return _uncertainties[binIndex(i0,i1,true)]; }
  double Histogram::binAvgValueUncertaintyAt(unsigned int i0, unsigned int i1, unsigned int i2) const{ return _uncertainties[binIndex(i0,i1,i2,true)]; }
  double Histogram::binAvgValueUncertaintyAt(const std::vector<unsigned int> & indices) const        { return _uncertainties[binIndex(indices,true)]; }

  std::vector<double> Histogram::binValues() const
  {
  std::vector<double> result;
  result.reserve(_values.size());
  for (unsigned int k=0; k<_values.size(); k++)
    result.push_back(_entries[k]>0 ? total(k) : -1.0E100); // -1.0E100 indicates no data in this bin
  return result;
  }

  std::vector<double> Histogram::binValueUncertainties() const
  {
  std::vector<double> result;
  result.reserve(_uncertainties.size());
  for (unsigned int k=0; k<_uncertainties.size(); k++)
    result.push_back(_entries[k]>0 ? totalUncertainty(k) : -1.0E100); // -1.0E100 indicates no data in this bin
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
  return total(index);
  }

  double Histogram::statErrorAt(unsigned int index) const
  {
  if (index>=nBinsTotal()) throw IndexOutOfBoundException(index,nBinsTotal(),"Out of bound index",__FUNCTION__);
  return totalUncertainty(index);
  }

  double Histogram::sysErrorAt(unsigned int index) const
  {
  return statErrorAt(index);
  }

  DataPoint2D Histogram::point(unsigned int index) const
  {
  if (rank()!=1) throw MathException("Data points are only defined for a histogram of rank 1",__FUNCTION__);
  double error = binUncertaintyAt(index);
  // a histogram has no systematic uncertainty: the statistical one is returned in its place
  return DataPoint2D(binCenter(index), binValueAt(index), error, error);
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
    << h.total(k) << "  "
    << h.totalUncertainty(k) << "  "
    << h._values[k] << "  "
    << h._uncertainties[k] << std::endl;
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
    if (_entries[k]!=0 || _values[k]!=0.0f || _uncertainties[k]!=0.0f) nNonEmpty++;
  w.writeVarUInt(nNonEmpty);
  std::size_t next = 0;   // first global index after the previous non-empty bin
  for (std::size_t k=0; k<_entries.size(); k++)
    {
    if (_entries[k]==0 && _values[k]==0.0f && _uncertainties[k]==0.0f) continue;
    w.writeVarUInt(k-next);
    w.writeVarInt(_entries[k]);
    w.writeFloat32(_values[k]);
    w.writeFloat32(_uncertainties[k]);
    next = k+1;
    }
  }

  void Histogram::decodeBinary(BinaryReader & r, const String & histogramName, unsigned int /*version*/)
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
  resizeBins();
  uint64_t nNonEmpty = r.readVarUInt();
  if (nNonEmpty > total) throw Exception("More non-empty bins than bins","Histogram::decodeBinary()");
  uint64_t k = 0;
  for (uint64_t i=0; i<nNonEmpty; i++)
    {
    k += r.readVarUInt();
    if (k >= total) throw Exception("Bin index out of range","Histogram::decodeBinary()");
    _entries[k]       = long(r.readVarInt());
    _values[k]        = r.readFloat32();
    _uncertainties[k] = r.readFloat32();
    k++;
    }
  }

  void Histogram::writeBinary(std::ostream & out) const                                      { writeBinaryObject(out,*this); }
  bool Histogram::readBinary(std::istream & in)                                              { return readBinaryObject(in,*this); }
  void Histogram::saveBinary(const String & fileName, bool append) const                     { saveBinaryObject(fileName,*this,append); }
  void Histogram::loadBinary(const String & fileName, const String & histogramName)          { loadBinaryObject(fileName,*this,histogramName); }
  void Histogram::saveBinary(const String & fileName, const std::vector<Histogram> & hs)     { saveBinaryObjects(fileName,hs); }
  std::vector<Histogram> Histogram::loadAllBinary(const String & fileName)                   { return loadAllBinaryObjects<Histogram>(fileName); }

  } // namespace MATH
} // namespace CAP
