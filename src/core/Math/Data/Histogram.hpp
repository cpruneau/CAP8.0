#ifndef CAP_Histogram
#define CAP_Histogram
#include <vector>
#include <cmath>
#include <ostream>
#include <istream>
#include <limits>
#include "Object.hpp"
#include "Partition.hpp"
#include "DataPoint2D.hpp"
#include "DataPoints.hpp"
#include "DataPointsVector.hpp"
#include "BinaryIO.hpp"

namespace CAP
{
  class CapFile;

  namespace MATH
  {

  //!
  //! @brief Bin values of a Histogram stored in single (float) or double precision, read and written as double.
  //!
  //! Only one of the two vectors is used, according to the precision. Elements are accessed with operator[]; the
  //! writable form returns a small proxy, so that v[k] = x, v[k] *= c and v[i] = w[j] work for either precision.
  //!
  class BinValues
  {
  public:

  //! @brief Proxy for one writable element.
  class Element
  {
  public:
  Element(BinValues & owner, std::size_t index) : _owner(owner), _index(index) {}
  operator double() const { return _owner.get(_index); }
  Element & operator=(double v)              { _owner.set(_index,v); return *this; }
  Element & operator=(const Element & other) { _owner.set(_index,double(other)); return *this; }
  Element & operator+=(double v) { _owner.set(_index,_owner.get(_index)+v); return *this; }
  Element & operator-=(double v) { _owner.set(_index,_owner.get(_index)-v); return *this; }
  Element & operator*=(double v) { _owner.set(_index,_owner.get(_index)*v); return *this; }
  Element & operator/=(double v) { _owner.set(_index,_owner.get(_index)/v); return *this; }
  private:
  BinValues & _owner;
  std::size_t _index;
  };

  //! @brief Constructor. @param doublePrecision true to store doubles, false (default) to store floats
  BinValues(bool doublePrecision=false) : _double(doublePrecision), _floats(), _doubles() {}

  //! @brief Whether values are stored as double. @return true for double precision
  bool isDouble() const { return _double; }
  //! @brief Change the precision, converting the stored values. @param doublePrecision true for double precision
  void setDouble(bool doublePrecision)
  {
  if (doublePrecision==_double) return;
  if (doublePrecision) { _doubles.assign(_floats.begin(),_floats.end()); std::vector<float>().swap(_floats); }
  else                 { _floats.assign(_doubles.begin(),_doubles.end()); std::vector<double>().swap(_doubles); }
  _double = doublePrecision;
  }
  //! @brief Number of values. @return size
  std::size_t size() const { return _double ? _doubles.size() : _floats.size(); }
  //! @brief Value k. @param k index @return value
  double get(std::size_t k) const { return _double ? _doubles[k] : double(_floats[k]); }
  //! @brief Set value k. @param k index @param v value (rounded to float in single precision)
  void set(std::size_t k, double v) { if (_double) _doubles[k] = v; else _floats[k] = float(v); }
  //! @brief Value k. @param k index @return value
  double operator[](std::size_t k) const { return get(k); }
  //! @brief Writable value k. @param k index @return proxy
  Element operator[](std::size_t k) { return Element(*this,k); }
  //! @brief Set n values to v. @param n number of values @param v value
  void assign(std::size_t n, double v) { if (_double) _doubles.assign(n,v); else _floats.assign(n,float(v)); }
  //! @brief Set every value to v (size unchanged). @param v value
  void fill(double v) { assign(size(),v); }
  //! @brief Remove all values.
  void clear() { _floats.clear(); _doubles.clear(); }
  //! @brief Copy of the values as double. @return values
  std::vector<double> toVector() const
  {
  return _double ? _doubles : std::vector<double>(_floats.begin(),_floats.end());
  }

  private:
  bool                _double;   //!< precision of the stored values
  std::vector<float>  _floats;   //!< values in single precision (used when !_double)
  std::vector<double> _doubles;  //!< values in double precision (used when _double)

  };

  //!
  //! @brief Histogram of rank 1, 2, 3, ... with regular (fixed width) binning.
  //!
  //! The binning is provided by the base class Partition; bins are addressed either by
  //! per-axis indices, by coordinates, or by a global (flat) bin index (see Partition).
  //!
  //! <b>Bin content.</b> Each bin k stores three numbers:
  //!  - the number of entries, n = binEntriesAt(), i.e., the number of fills;
  //!  - the average value per entry, a = binAvgValueAt();
  //!  - the uncertainty of that average, u = binAvgValueUncertaintyAt().
  //!
  //! The bin <em>value</em> is n*a = binValueAt() and its uncertainty is n*u = binUncertaintyAt().
  //! Filling with weight w adds w to the bin value and w^2 to the squared uncertainty of the value.
  //! All arithmetic operations are carried out on bin values, with statistical uncertainties
  //! propagated assuming uncorrelated operands.
  //!
  //! <b>Ranges.</b> Functions taking first/last bin arguments use inclusive ranges [first, last].
  //! A value of last beyond the end of the axis (e.g., Histogram::allBins, the default) is clamped
  //! to the last bin of the axis.
  //!
  //! <b>Products, ratios and inversions.</b> The entry count of such results has no meaning: bins
  //! are stored with one entry (average equals value). Bins where the result is undefined
  //! (division by zero, empty operand) are set empty (entries, value and uncertainty all zero).
  //!
  //! Example:
  //! @code
  //!   using namespace CAP::MATH;
  //!   Histogram h("pt","Transverse momentum",100,0.0,10.0);
  //!   h.fill(2.5);          // weight 1
  //!   h.fill(3.1,0.8);      // weight 0.8
  //!   Histogram r = h/h;    // bin-by-bin ratio with error propagation
  //!   double sum  = h.integral();
  //! @endcode
  //!
  //! <b>Data points.</b> A histogram is also a set of data points (DataPoints), one per bin, e.g. for use
  //! with ChiSquare: the point of global bin k has the bin centers as coordinates, the bin value as value,
  //! the uncertainty of the bin value as statistical uncertainty, and no coordinate uncertainty.
  //! A histogram has no systematic uncertainty: it is 0. For rank 1, point(k) also returns these as a DataPoint2D
  //! (x = bin center, y = bin value).
  //!
  class Histogram
  :
  public DataPoints,
  public Partition
  {
  protected:
  std::vector<long>  _entries;        //!< Number of entries of each bin
  BinValues _avgValues;               //!< Average value per entry of each bin (float or double, see precision())
  BinValues _avgUncertainties;        //!< Uncertainty of the average value of each bin (same precision as _avgValues)

  //!
  //! @brief Uncertainty of a sum or difference of two uncorrelated quantities.
  //! @param e1 uncertainty of the first quantity
  //! @param e2 uncertainty of the second quantity
  //! @return sqrt(e1^2 + e2^2)
  //!
  static double statErrorSum(double e1, double e2)
  {
  return std::sqrt(e1*e1+e2*e2);
  }

  //!
  //! @brief Uncertainty of the product v1*v2 of two uncorrelated quantities.
  //!
  //! Computed without relative errors to avoid divisions: sqrt((v1*e2)^2 + (v2*e1)^2).
  //! @param v1 first quantity
  //! @param e1 uncertainty of v1
  //! @param v2 second quantity
  //! @param e2 uncertainty of v2
  //! @return uncertainty of v1*v2
  //!
  static double statErrorProduct(double v1, double e1, double v2, double e2)
  {
  double ep1 = v1*e2;
  double ep2 = v2*e1;
  return std::sqrt(ep1*ep1+ep2*ep2);
  }

  //!
  //! @brief Uncertainty of the ratio v1/v2 of two uncorrelated quantities.
  //!
  //! Computed as sqrt(e1^2 + (e2*v1/v2)^2)/|v2|. The caller must ensure v2!=0.
  //! @param v1 numerator
  //! @param e1 uncertainty of v1
  //! @param v2 denominator (non-zero)
  //! @param e2 uncertainty of v2
  //! @return uncertainty of v1/v2
  //!
  static double statErrorRatio(double v1, double e1, double v2, double e2)
  {
  double er2 = e2*v1/v2;
  return std::sqrt(e1*e1+er2*er2)/std::fabs(v2);
  }

  //!
  //! @brief Value (entries*average) of global bin k.
  //! @param k global bin index (not checked)
  //! @return bin value
  //!
  double value(unsigned int k) const            { return double(_entries[k])*_avgValues[k]; }

  //!
  //! @brief Uncertainty of the value of global bin k.
  //! @param k global bin index (not checked)
  //! @return uncertainty of the bin value
  //!
  double valueUncertainty(unsigned int k) const { return double(_entries[k])*_avgUncertainties[k]; }

  //!
  //! @brief Store the content of global bin k from its entry count and values.
  //!
  //! If entries<=0 the bin is set empty.
  //! @param k global bin index (not checked)
  //! @param entries number of entries
  //! @param binValue bin value (entries*average)
  //! @param binUncertainty uncertainty of the bin value
  //!
  void   setValue(unsigned int k, long entries, double binValue, double binUncertainty);

  //!
  //! @brief Set global bin k empty (zero entries, value and uncertainty).
  //! @param k global bin index (not checked)
  //!
  void   setEmpty(unsigned int k);

  //!
  //! @brief Resize the bin arrays to nBinsTotal() and set all bins empty.
  //!
  void   resizeBins();
  //! @brief Throw unless k is a valid global bin index. @param k global bin index @param source caller name
  void   checkGlobal(unsigned int k, const char * source) const
  { if (k>=_entries.size()) throw IndexOutOfBoundException(int(k),int(_entries.size()),"Global bin index out of range",source); }

  //!
  //! @brief Throw if histogram h does not have the same binning as this histogram.
  //! @param h histogram to check
  //! @param source name of the calling function, reported in the exception
  //! @throws Exception if the binnings differ
  //!
  void   checkCompatible(const Histogram & h, const char * source) const;

  //!
  //! @brief Throw if this histogram is not of the required rank.
  //! @param requiredRank required rank
  //! @param source name of the calling function, reported in the exception
  //! @throws Exception if rank()!=requiredRank
  //!
  void   checkRank(unsigned int requiredRank, const char * source) const;

  //!
  //! @brief Convert a user bin range along an axis into a valid inclusive range.
  //!
  //! last is clamped to the last bin of the axis.
  //! @param dimIndex axis index
  //! @param first first bin requested
  //! @param last last bin requested
  //! @param iFirst [out] first bin of the valid range
  //! @param iLast [out] last bin of the valid range
  //! @param source name of the calling function, reported in the exception
  //! @throws InvalidBoundsException if first>last after clamping
  //!
  void   resolveRange(unsigned int dimIndex, unsigned int first, unsigned int last,
                      unsigned int & iFirst, unsigned int & iLast, const char * source) const;

  //!
  //! @brief Convert a user global bin range into a valid inclusive range.
  //!
  //! last is clamped to nBinsTotal()-1.
  //! @param first first global bin requested
  //! @param last last global bin requested
  //! @param iFirst [out] first bin of the valid range
  //! @param iLast [out] last bin of the valid range
  //! @param source name of the calling function, reported in the exception
  //! @throws Exception if the histogram has no bins
  //! @throws InvalidBoundsException if first>last after clamping
  //!
  void   resolveFlatRange(unsigned int first, unsigned int last,
                          unsigned int & iFirst, unsigned int & iLast, const char * source) const;

  //!
  //! @brief Convert a bin index returned by a coordinate lookup into a checked index.
  //! @param index bin index (-1 denotes a coordinate outside the histogram range)
  //! @param source name of the calling function, reported in the exception
  //! @return index as unsigned int
  //! @throws IndexOutOfBoundException if index<0
  //!
  unsigned int checkedIndex(int index, const char * source) const;

  //!
  //! @brief Set the entry count of global bin k, keeping the bin value and its uncertainty.
  //! @param k global bin index (not checked)
  //! @param entries number of entries
  //! @throws Exception if entries<=0 and the bin has non-zero content
  //!
  void   doSetEntries(unsigned int k, long entries);

  //!
  //! @brief Set the value of global bin k, keeping its entry count (1 if the bin was empty).
  //! @param k global bin index (not checked)
  //! @param value bin value
  //!
  void   doSetValue(unsigned int k, double value);

  //!
  //! @brief Set the uncertainty of the value of global bin k, keeping its entry count (1 if the bin was empty).
  //! @param k global bin index (not checked)
  //! @param uncertainty uncertainty of the bin value
  //!
  void   doSetUncertainty(unsigned int k, double uncertainty);

  //!
  //! @brief Set the full content of global bin k.
  //! @param k global bin index (not checked)
  //! @param entries number of entries
  //! @param value bin value
  //! @param uncertainty uncertainty of the bin value
  //! @throws Exception if entries<=0 while value or uncertainty is non-zero
  //!
  void   doSetBin(unsigned int k, long entries, double value, double uncertainty);

  //!
  //! @brief Copy the binning and bin content of h into this histogram, keeping the name and title of this histogram.
  //! @param h source histogram
  //!
  void   assignContent(const Histogram & h);

  //!
  //! @brief Return a bin quantity used by the extremum searches.
  //! @param k global bin index (not checked)
  //! @param quantity 0: entries, 1: value, 2: average value
  //! @return requested quantity
  //!
  double binQuantity(unsigned int k, int quantity) const;

  //!
  //! @brief Find the global bin with the largest or smallest value of a bin quantity.
  //!
  //! When several bins share the extremum, the first one is returned.
  //! @param quantity 0: entries, 1: value, 2: average value
  //! @param findMaximum true to find the maximum, false for the minimum
  //! @param first first global bin of the search range
  //! @param last last global bin of the search range (clamped)
  //! @return global index of the extremum bin
  //!
  unsigned int extremumBin(int quantity, bool findMaximum, unsigned int first, unsigned int last) const;

  //!
  //! @brief Add the content of bin kSource of histogram h into bin kTarget of this histogram.
  //!
  //! Entries and values are summed; uncertainties are added in quadrature.
  //! @param kTarget global bin index in this histogram (not checked)
  //! @param h source histogram
  //! @param kSource global bin index in h (not checked)
  //!
  void   accumulate(unsigned int kTarget, const Histogram & h, unsigned int kSource);

  //!
  //! @brief Common implementation of the setWith() functions.
  //! @tparam E type of the entries
  //! @tparam V type of the values
  //! @tparam U type of the uncertainties
  //! @param entries entries of each bin, or nullptr to use one entry per bin
  //! @param values value of each bin
  //! @param uncertainties uncertainty of each bin value, or nullptr for zero uncertainties
  //! @param source name of the calling function, reported in the exception
  //! @throws Exception if a vector size differs from nBinsTotal()
  //!
  template<typename E, typename V, typename U>
  void   setWithValues(const std::vector<E> * entries, const std::vector<V> & values,
                       const std::vector<U> * uncertainties, const char * source);

  public:

  //! @brief minimum(k) and maximum(k) are the edges of axis k (Partition), not the extrema of the bin coordinates
  //! (DataPoints::minimum()/maximum()); the names would otherwise be ambiguous.
  using Partition::minimum;
  using Partition::maximum;

  //! @name Storage precision
  //! Bin averages and their uncertainties are stored in single (kFloat, default) or double (kDouble) precision;
  //! entry counts are always integers. The precision is copied with the histogram, kept by operations on it, and
  //! saved in binary files. ROOT storage options map as 0 (int) and 1 (float) to kFloat, 2 (double) to kDouble.
  //! @{
  //! @brief Storage precision of the bin values.
  enum Precision { kFloat=1, kDouble=2 };
  //! @brief Storage precision. @return kFloat or kDouble
  Precision precision() const { return _avgValues.isDouble() ? kDouble : kFloat; }
  //! @brief Set the storage precision, converting the stored values. @param precision kFloat or kDouble
  void setPrecision(Precision precision) { _avgValues.setDouble(precision==kDouble); _avgUncertainties.setDouble(precision==kDouble); }
  //! @name Access by global bin index
  //! The global index k runs from 0 to nBinsTotal()-1 over all bins, axis 0 fastest: k = i0 + n0*(i1 + n1*i2).
  //! Values are bin values (entries*average), with their uncertainties. Setting the value or uncertainty of an empty bin
  //! gives it one entry. These functions throw IndexOutOfBoundException if k>=nBinsTotal().
  //! @{
  //! @brief Number of entries of bin k. @param k global bin index @return entries
  long   binEntriesAtGlobal(unsigned int k) const     { checkGlobal(k,__FUNCTION__); return _entries[k]; }
  //! @brief Value of bin k. @param k global bin index @return entries*average
  double binValueAtGlobal(unsigned int k) const       { checkGlobal(k,__FUNCTION__); return value(k); }
  //! @brief Uncertainty of the value of bin k. @param k global bin index @return uncertainty
  double binUncertaintyAtGlobal(unsigned int k) const { checkGlobal(k,__FUNCTION__); return valueUncertainty(k); }
  //! @brief Average value per entry of bin k (the profile value). @param k global bin index @return average
  double binAvgValueAtGlobal(unsigned int k) const    { checkGlobal(k,__FUNCTION__); return _avgValues[k]; }
  //! @brief Uncertainty of the average value of bin k. @param k global bin index @return uncertainty
  double binAvgValueUncertaintyAtGlobal(unsigned int k) const { checkGlobal(k,__FUNCTION__); return _avgUncertainties[k]; }
  //! @brief Set the value of bin k (the uncertainty is kept). @param k global bin index @param value
  void setBinValueAtGlobal(unsigned int k, double value)             { checkGlobal(k,__FUNCTION__); doSetValue(k,value); }
  //! @brief Set the uncertainty of the value of bin k. @param k global bin index @param uncertainty uncertainty
  void setBinUncertaintyAtGlobal(unsigned int k, double uncertainty) { checkGlobal(k,__FUNCTION__); doSetUncertainty(k,uncertainty); }
  //! @brief Set the number of entries of bin k (value and uncertainty kept). @param k global bin index @param entries entries
  void setBinEntriesAtGlobal(unsigned int k, long entries)           { checkGlobal(k,__FUNCTION__); doSetEntries(k,entries); }
  //! @brief Set bin k. @param k global bin index @param entries entries @param value @param uncertainty uncertainty of the value
  void setBinAtGlobal(unsigned int k, long entries, double value, double uncertainty) { checkGlobal(k,__FUNCTION__); doSetBin(k,entries,value,uncertainty); }
  //! @brief Set the average value (profile value) and its uncertainty of bin k, keeping its entries (1 if empty).
  //! @param k global bin index @param average average value @param uncertainty uncertainty of the average
  void setBinAvgValueAtGlobal(unsigned int k, double average, double uncertainty)
  {
  checkGlobal(k,__FUNCTION__);
  long n = _entries[k]>0 ? _entries[k] : 1;
  setValue(k,n,average*n,uncertainty*n);
  }
  //! @}

  //! @brief Precision corresponding to a ROOT-style storage option (0 int, 1 float, 2 double).
  //! @param storageOption 0, 1 or 2 @return kFloat for 0 and 1, kDouble for 2 @throws Exception for other values
  static Precision precisionFromStorageOption(int storageOption);
  //! @}

  //!
  //! @brief Value of a "last" bin argument meaning "up to the last bin of the axis".
  //!
  static constexpr unsigned int allBins = std::numeric_limits<unsigned int>::max();

  //! @name Construction and assignment
  //! @{

  //!
  //! @brief Default constructor: rank 1 histogram named "H" with a single bin spanning [0,1).
  //!
  Histogram();

  //!
  //! @brief Create an empty histogram (named "H") with the binning of the given partition.
  //! @param partition binning of the histogram
  //!
  Histogram(const Partition & partition);

  //!
  //! @brief Create an empty rank 1 histogram.
  //! @param name name of the histogram
  //! @param title title of the histogram
  //! @param n number of bins
  //! @param minimum lower edge of the axis
  //! @param maximum upper edge of the axis
  //! @throws Exception if n<1 or minimum>=maximum
  //!
  Histogram(const String & name, const String & title,
            unsigned int n, double minimum, double maximum);

  //!
  //! @brief Create an empty rank 2 histogram.
  //! @param name name of the histogram
  //! @param title title of the histogram
  //! @param n0 number of bins along axis 0
  //! @param minimum_0 lower edge of axis 0
  //! @param maximum_0 upper edge of axis 0
  //! @param n1 number of bins along axis 1
  //! @param minimum_1 lower edge of axis 1
  //! @param maximum_1 upper edge of axis 1
  //! @throws Exception if an axis has no bins or an empty range
  //!
  Histogram(const String & name, const String & title,
            unsigned int n0, double minimum_0, double maximum_0,
            unsigned int n1, double minimum_1, double maximum_1);

  //!
  //! @brief Create an empty rank 3 histogram.
  //! @param name name of the histogram
  //! @param title title of the histogram
  //! @param n0 number of bins along axis 0
  //! @param minimum_0 lower edge of axis 0
  //! @param maximum_0 upper edge of axis 0
  //! @param n1 number of bins along axis 1
  //! @param minimum_1 lower edge of axis 1
  //! @param maximum_1 upper edge of axis 1
  //! @param n2 number of bins along axis 2
  //! @param minimum_2 lower edge of axis 2
  //! @param maximum_2 upper edge of axis 2
  //! @throws Exception if an axis has no bins or an empty range
  //!
  Histogram(const String & name, const String & title,
            unsigned int n0, double minimum_0, double maximum_0,
            unsigned int n1, double minimum_1, double maximum_1,
            unsigned int n2, double minimum_2, double maximum_2);

  //!
  //! @brief Copy constructor: copies name, title, binning and bin content.
  //! @param source histogram to copy
  //!
  Histogram(const Histogram & source);

  //!
  //! @brief Destructor.
  //!
  virtual ~Histogram() {}

  //!
  //! @brief Polymorphic copy (a Profile is copied as a Profile).
  //! @return new copy, owned by the caller
  //!
  virtual Histogram * clone() const { return new Histogram(*this); }

  //!
  //! @brief Mean of the coordinate along an axis, weighted by the bin values (as ROOT's TH1::GetMean()).
  //! @param dimIndex axis index @return mean, or 0 if the sum of the bin values is 0
  //!
  double mean(unsigned int dimIndex=0) const;

  //!
  //! @brief Random coordinate distributed as the bin values of a rank 1 histogram (uniform within the chosen bin),
  //! as ROOT's TH1::GetRandom(). Negative values are ignored.
  //! @param x coordinate drawn @throws HistogramException if the histogram is empty
  //!
  void random(double & x) const;
  //! @brief Random coordinate of a rank 1 histogram (see random(double&)). @return coordinate
  double random() const { double x; random(x); return x; }

  //!
  //! @brief Random point distributed as the bin values of a rank 2 histogram, as ROOT's TH2::GetRandom2().
  //! @param x coordinate along axis 0 @param y coordinate along axis 1 @throws HistogramException if the histogram is empty
  //!
  void random(double & x, double & y) const;

  //!
  //! @brief Assignment operator: copies name, title, binning and bin content.
  //! @param rhs histogram to copy
  //! @return reference to this histogram
  //!
  Histogram & operator= (const Histogram & rhs);

  //! @}

  //!
  //! @brief Check whether histogram source has the same binning as this histogram.
  //! @param source histogram to compare with
  //! @param sizeAndLimits if true, the axis limits must also be identical
  //! @return true if the binnings are compatible
  //!
  bool compatibleWith(const Histogram & source, bool sizeAndLimits=1) const
  {
  return Partition::compatibleWith(source,sizeAndLimits);
  }

  //! @name Comparison
  //! @{

  //!
  //! @brief Statistical equality test.
  //!
  //! Two histograms are equal if they have compatible binning and all their bin values agree
  //! within two combined standard deviations (exact equality is required for bins with zero uncertainty).
  //! @param rhs histogram to compare with
  //! @return true if the histograms are statistically equal
  //!
  bool  operator== (const Histogram & rhs) const;

  //!
  //! @brief Negation of operator==.
  //! @param rhs histogram to compare with
  //! @return true if the histograms are not statistically equal
  //!
  bool  operator!= (const Histogram & rhs) const;

  //! @}

  //! @name Arithmetic
  //! @{

  //!
  //! @brief Add histogram rhs bin by bin: entries and values are summed, uncertainties added in quadrature.
  //! @param rhs histogram to add (same binning)
  //! @return reference to this histogram
  //! @throws Exception if the binnings differ
  //!
  Histogram & operator+= (const Histogram & rhs);

  //!
  //! @brief Subtract histogram rhs bin by bin: entries are summed, values subtracted, uncertainties added in quadrature.
  //! @param rhs histogram to subtract (same binning)
  //! @return reference to this histogram
  //! @throws Exception if the binnings differ
  //!
  Histogram & operator-= (const Histogram & rhs);

  //!
  //! @brief Multiply by histogram rhs bin by bin (see multiplyBy()).
  //! @param rhs histogram to multiply by (same binning)
  //! @return reference to this histogram
  //! @throws Exception if the binnings differ
  //!
  Histogram & operator*= (const Histogram & rhs);

  //!
  //! @brief Divide by histogram rhs bin by bin (see divideBy()).
  //! @param rhs histogram to divide by (same binning)
  //! @return reference to this histogram
  //! @throws Exception if the binnings differ
  //!
  Histogram & operator/= (const Histogram & rhs);

  //!
  //! @brief Add a constant to every bin value (see add(double)).
  //! @param value constant to add
  //! @return reference to this histogram
  //!
  Histogram & operator+= (double value);

  //!
  //! @brief Subtract a constant from every bin value (see add(double)).
  //! @param value constant to subtract
  //! @return reference to this histogram
  //!
  Histogram & operator-= (double value);

  //!
  //! @brief Multiply every bin value and its uncertainty by a constant (see scale()).
  //! @param value multiplicative constant
  //! @return reference to this histogram
  //!
  Histogram & operator*= (double value);

  //!
  //! @brief Divide every bin value and its uncertainty by a constant.
  //! @param value divisor
  //! @return reference to this histogram
  //! @throws Exception if value==0
  //!
  Histogram & operator/= (double value);

  //!
  //! @brief Multiply every bin value by coefficient and its uncertainty by |coefficient|. Entry counts are unchanged.
  //! @param coefficient multiplicative coefficient
  //!
  virtual void scale(double coefficient);

  //!
  //! @brief Add a constant to every bin value. Uncertainties are unchanged; empty bins become bins with one entry.
  //! @param value constant to add
  //!
  void add(double value);

  //!
  //! @brief Add coefficient*h bin by bin: entries summed, values combined, uncertainties added in quadrature.
  //! @param h histogram to add (same binning)
  //! @param coefficient coefficient multiplying h
  //! @throws Exception if the binnings differ
  //!
  virtual void add(const Histogram & h, double coefficient=1.0);

  //!
  //! @brief Multiply by coefficient*rhs bin by bin.
  //!
  //! Result bins hold one entry; bins where either operand is empty are set empty.
  //! @param rhs histogram to multiply by (same binning)
  //! @param coefficient coefficient multiplying rhs
  //! @throws Exception if the binnings differ
  //!
  void multiplyBy(const Histogram & rhs, double coefficient=1.0);

  //!
  //! @brief Divide by coefficient*rhs bin by bin.
  //!
  //! Result bins hold one entry; bins where either operand is empty or the divisor vanishes are set empty.
  //! @param rhs histogram to divide by (same binning)
  //! @param coefficient coefficient multiplying rhs
  //! @throws Exception if the binnings differ
  //!
  void divideBy(const Histogram & rhs, double coefficient=1.0);

  //!
  //! @brief Replace every bin value v by 1/v, with uncertainty e/v^2.
  //!
  //! Result bins hold one entry; empty bins and bins with a zero value are set empty.
  //!
  void inverse();

  //!
  //! @brief Set this histogram to coefficient1*h1 + coefficient2*h2.
  //!
  //! This histogram takes the binning of h1; its name and title are kept. h1 or h2 may be this histogram.
  //! @param h1 first histogram
  //! @param h2 second histogram (same binning as h1)
  //! @param coefficient1 coefficient multiplying h1
  //! @param coefficient2 coefficient multiplying h2
  //! @throws Exception if h1 and h2 have different binnings
  //!
  virtual void setAsSumOf(const Histogram & h1, const Histogram & h2,
                  double coefficient1=1.0, double coefficient2=1.0);

  //!
  //! @brief Set this histogram to the weighted sum of histograms[i]*coefficients[i].
  //!
  //! This histogram takes the binning of histograms[0]; its name and title are kept.
  //! @param histograms histograms to sum (same binning, non-null)
  //! @param coefficients coefficient multiplying each histogram
  //! @throws Exception if the list is empty, the sizes differ or the binnings differ
  //! @throws NullPointerException if a histogram pointer is null
  //!
  virtual void setAsSumOf(const std::vector<const Histogram *> & histograms,
                  const std::vector<double> & coefficients);

  //!
  //! @brief Set this histogram to (coefficient1*h1)*(coefficient2*h2), bin by bin.
  //!
  //! This histogram takes the binning of h1; its name and title are kept.
  //! @param h1 first histogram
  //! @param h2 second histogram (same binning as h1)
  //! @param coefficient1 coefficient multiplying h1
  //! @param coefficient2 coefficient multiplying h2
  //! @throws Exception if h1 and h2 have different binnings
  //!
  void setAsProductOf(const Histogram & h1, const Histogram & h2,
                      double coefficient1=1.0, double coefficient2=1.0);

  //!
  //! @brief Set this histogram to the product of histograms[i]*coefficients[i], bin by bin.
  //!
  //! This histogram takes the binning of histograms[0]; its name and title are kept.
  //! @param histograms histograms to multiply (same binning, non-null)
  //! @param coefficients coefficient multiplying each histogram
  //! @throws Exception if the list is empty, the sizes differ or the binnings differ
  //! @throws NullPointerException if a histogram pointer is null
  //!
  void setAsProductOf(const std::vector<const Histogram *> & histograms,
                      const std::vector<double> & coefficients);

  //!
  //! @brief Set this histogram to (coefficient1*h1)/(coefficient2*h2), bin by bin.
  //!
  //! This histogram takes the binning of h1; its name and title are kept.
  //! @param h1 numerator histogram
  //! @param h2 denominator histogram (same binning as h1)
  //! @param coefficient1 coefficient multiplying h1
  //! @param coefficient2 coefficient multiplying h2
  //! @throws Exception if h1 and h2 have different binnings
  //!
  void setAsRatioOf(const Histogram & h1, const Histogram & h2,
                    double coefficient1=1.0, double coefficient2=1.0);

  //! @}

  //! @name Bin setters by bin indices
  //! Values are bin values (entries*average), with their uncertainties. Setting the entries keeps the bin value;
  //! setting the value or uncertainty keeps the entry count (one entry if the bin was empty).
  //! Indices are checked strictly: an out of range index throws IndexOutOfBoundException,
  //! and a number of indices different from the rank throws Exception.
  //! Setting entries<=0 for a bin with non-zero content throws Exception.
  //! @{

  //! @brief Set the entry count of a bin of a rank 1 histogram. @param binIndex bin index @param entries number of entries
  void setBinEntriesAt(unsigned int binIndex, long entries);
  //! @brief Set the value of a bin of a rank 1 histogram. @param binIndex bin index @param value bin value
  void setBinValueAt(unsigned int binIndex, double value);
  //! @brief Set the uncertainty of the value of a bin of a rank 1 histogram. @param binIndex bin index @param uncertainty uncertainty of the bin value
  void setBinUncertaintyAt(unsigned int binIndex, double uncertainty);
  //! @brief Set the full content of a bin of a rank 1 histogram.
  //! @param binIndex bin index @param entries number of entries @param value bin value @param uncertainty uncertainty of the bin value
  void setBinAt(unsigned int binIndex, long entries, double value, double uncertainty);

  //! @brief Set the entry count of a bin of a rank 2 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param entries number of entries
  void setBinEntriesAt(unsigned int binIndex0, unsigned int binIndex1, long entries);
  //! @brief Set the value of a bin of a rank 2 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param value bin value
  void setBinValueAt(unsigned int binIndex0, unsigned int binIndex1, double value);
  //! @brief Set the uncertainty of the value of a bin of a rank 2 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param uncertainty uncertainty of the bin value
  void setBinUncertaintyAt(unsigned int binIndex0, unsigned int binIndex1, double uncertainty);
  //! @brief Set the full content of a bin of a rank 2 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1
  //! @param entries number of entries @param value bin value @param uncertainty uncertainty of the bin value
  void setBinAt(unsigned int binIndex0, unsigned int binIndex1, long entries, double value, double uncertainty);

  //! @brief Set the entry count of a bin of a rank 3 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param binIndex2 bin index along axis 2 @param entries number of entries
  void setBinEntriesAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2, long entries);
  //! @brief Set the value of a bin of a rank 3 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param binIndex2 bin index along axis 2 @param value bin value
  void setBinValueAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2, double value);
  //! @brief Set the uncertainty of the value of a bin of a rank 3 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param binIndex2 bin index along axis 2 @param uncertainty uncertainty of the bin value
  void setBinUncertaintyAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2, double uncertainty);
  //! @brief Set the full content of a bin of a rank 3 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param binIndex2 bin index along axis 2
  //! @param entries number of entries @param value bin value @param uncertainty uncertainty of the bin value
  void setBinAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2, long entries, double value, double uncertainty);

  //! @brief Set the entry count of a bin of a histogram of any rank.
  //! @param binIndices bin indices along each axis (size must equal rank()) @param entries number of entries
  void setBinEntriesAt(const std::vector<unsigned int> & binIndices, long entries);
  //! @brief Set the value of a bin of a histogram of any rank.
  //! @param binIndices bin indices along each axis (size must equal rank()) @param value bin value
  void setBinValueAt(const std::vector<unsigned int> & binIndices, double value);
  //! @brief Set the uncertainty of the value of a bin of a histogram of any rank.
  //! @param binIndices bin indices along each axis (size must equal rank()) @param uncertainty uncertainty of the bin value
  void setBinUncertaintyAt(const std::vector<unsigned int> & binIndices, double uncertainty);
  //! @brief Set the full content of a bin of a histogram of any rank.
  //! @param binIndices bin indices along each axis (size must equal rank())
  //! @param entries number of entries @param value bin value @param uncertainty uncertainty of the bin value
  void setBinAt(const std::vector<unsigned int> & binIndices, long entries, double value, double uncertainty);

  //! @}

  //! @name Bin setters by coordinates
  //! The bin is the one containing the given coordinates. Values are bin values (entries*average), with their uncertainties.
  //! Coordinates outside the histogram range throw IndexOutOfBoundException; a rank mismatch throws Exception.
  //! @{

  //! @brief Set the entry count of the bin containing x (rank 1). @param x coordinate @param entries number of entries
  void setBinEntriesAt(double x, long entries);
  //! @brief Set the value of the bin containing x (rank 1). @param x coordinate @param value bin value
  void setBinValueAt(double x, double value);
  //! @brief Set the uncertainty of the value of the bin containing x (rank 1). @param x coordinate @param uncertainty uncertainty of the bin value
  void setBinUncertaintyAt(double x, double uncertainty);
  //! @brief Set the full content of the bin containing x (rank 1).
  //! @param x coordinate @param entries number of entries @param value bin value @param uncertainty uncertainty of the bin value
  void setBinAt(double x, long entries, double value, double uncertainty);

  //! @brief Set the entry count of the bin containing (x0,x1) (rank 2).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param entries number of entries
  void setBinEntriesAt(double x0, double x1, long entries);
  //! @brief Set the value of the bin containing (x0,x1) (rank 2).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param value bin value
  void setBinValueAt(double x0, double x1, double value);
  //! @brief Set the uncertainty of the value of the bin containing (x0,x1) (rank 2).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param uncertainty uncertainty of the bin value
  void setBinUncertaintyAt(double x0, double x1, double uncertainty);
  //! @brief Set the full content of the bin containing (x0,x1) (rank 2).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1
  //! @param entries number of entries @param value bin value @param uncertainty uncertainty of the bin value
  void setBinAt(double x0, double x1, long entries, double value, double uncertainty);

  //! @brief Set the entry count of the bin containing (x0,x1,x2) (rank 3).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param x2 coordinate along axis 2 @param entries number of entries
  void setBinEntriesAt(double x0, double x1, double x2, long entries);
  //! @brief Set the value of the bin containing (x0,x1,x2) (rank 3).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param x2 coordinate along axis 2 @param value bin value
  void setBinValueAt(double x0, double x1, double x2, double value);
  //! @brief Set the uncertainty of the value of the bin containing (x0,x1,x2) (rank 3).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param x2 coordinate along axis 2 @param uncertainty uncertainty of the bin value
  void setBinUncertaintyAt(double x0, double x1, double x2, double uncertainty);
  //! @brief Set the full content of the bin containing (x0,x1,x2) (rank 3).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param x2 coordinate along axis 2
  //! @param entries number of entries @param value bin value @param uncertainty uncertainty of the bin value
  void setBinAt(double x0, double x1, double x2, long entries, double value, double uncertainty);

  //! @brief Set the entry count of the bin containing point x (any rank).
  //! @param x coordinates along each axis (size must equal rank()) @param entries number of entries
  void setBinEntriesAt(const std::vector<double> & x, long entries);
  //! @brief Set the value of the bin containing point x (any rank).
  //! @param x coordinates along each axis (size must equal rank()) @param value bin value
  void setBinValueAt(const std::vector<double> & x, double value);
  //! @brief Set the uncertainty of the value of the bin containing point x (any rank).
  //! @param x coordinates along each axis (size must equal rank()) @param uncertainty uncertainty of the bin value
  void setBinUncertaintyAt(const std::vector<double> & x, double uncertainty);
  //! @brief Set the full content of the bin containing point x (any rank).
  //! @param x coordinates along each axis (size must equal rank())
  //! @param entries number of entries @param value bin value @param uncertainty uncertainty of the bin value
  void setBinAt(const std::vector<double> & x, long entries, double value, double uncertainty);

  //! @}

  //! @name Setting all bins at once
  //! Vectors are indexed by global bin index and must have nBinsTotal() elements (Exception otherwise).
  //! Values are bin values (entries*average), with their uncertainties. When no entries are given, each bin gets one entry;
  //! when no uncertainties are given, they are set to zero.
  //! @{

  //! @brief Set all bin values. @param values value of each bin
  void setWith(const std::vector<float> & values);
  //! @brief Set all bin values. @param values value of each bin
  void setWith(const std::vector<double> & values);
  //! @brief Set all bin values and their uncertainties.
  //! @param values value of each bin @param uncertainties uncertainty of each bin value
  void setWith(const std::vector<float> & values, const std::vector<float> & uncertainties);
  //! @brief Set all bin values and their uncertainties.
  //! @param values value of each bin @param uncertainties uncertainty of each bin value
  void setWith(const std::vector<double> & values, const std::vector<double> & uncertainties);
  //! @brief Set all bin entries and values.
  //! @param entries number of entries of each bin @param values value of each bin
  void setWith(const std::vector<long> & entries,  const std::vector<float> & values);
  //! @brief Set all bin entries and values.
  //! @param entries number of entries of each bin @param values value of each bin
  void setWith(const std::vector<long> & entries,  const std::vector<double> & values);
  //! @brief Set all bin entries, values and uncertainties.
  //! @param entries number of entries of each bin @param values value of each bin @param uncertainties uncertainty of each bin value
  void setWith(const std::vector<long> & entries,  const std::vector<float> & values,  const std::vector<float> & uncertainties);
  //! @brief Set all bin entries, values and uncertainties.
  //! @param entries number of entries of each bin @param values value of each bin @param uncertainties uncertainty of each bin value
  void setWith(const std::vector<long> & entries,  const std::vector<double> & values, const std::vector<double> & uncertainties);

  //! @}

  //! @name Extraction and projection
  //! Bin ranges are inclusive; last is clamped to the end of the axis.
  //! The returned histogram has the name and title of this histogram.
  //! @{

  //!
  //! @brief Extract bins [first, last] of a rank 1 histogram into a new histogram.
  //! @param first first bin to extract
  //! @param last last bin to extract
  //! @return new rank 1 histogram covering the extracted bins
  //! @throws Exception if the rank is not 1
  //! @throws InvalidBoundsException if first>last
  //!
  Histogram extract1(unsigned int first, unsigned int last) const;

  //!
  //! @brief Extract the bins [first0,last0] x [first1,last1] of a rank 2 histogram into a new histogram.
  //! @param first0 first bin along axis 0
  //! @param last0 last bin along axis 0
  //! @param first1 first bin along axis 1
  //! @param last1 last bin along axis 1
  //! @return new rank 2 histogram covering the extracted bins
  //! @throws Exception if the rank is not 2
  //! @throws InvalidBoundsException if a range is invalid
  //!
  Histogram extract2(unsigned int first0, unsigned int last0,
                     unsigned int first1, unsigned int last1) const;

  //!
  //! @brief Extract the bins [first0,last0] x [first1,last1] x [first2,last2] of a rank 3 histogram into a new histogram.
  //! @param first0 first bin along axis 0
  //! @param last0 last bin along axis 0
  //! @param first1 first bin along axis 1
  //! @param last1 last bin along axis 1
  //! @param first2 first bin along axis 2
  //! @param last2 last bin along axis 2
  //! @return new rank 3 histogram covering the extracted bins
  //! @throws Exception if the rank is not 3
  //! @throws InvalidBoundsException if a range is invalid
  //!
  Histogram extract3(unsigned int first0, unsigned int last0,
                     unsigned int first1, unsigned int last1,
                     unsigned int first2, unsigned int last2) const;

  //!
  //! @brief Project a rank 2 histogram onto one of its axes.
  //!
  //! Bins [first, last] of the other axis are summed (entries and values summed, uncertainties in quadrature).
  //! @param targetAxisIndex axis kept in the projection (0 or 1)
  //! @param first first bin of the summed axis
  //! @param last last bin of the summed axis
  //! @return rank 1 histogram with the binning of axis targetAxisIndex
  //! @throws Exception if the rank is not 2
  //! @throws IndexOutOfBoundException if targetAxisIndex>1
  //! @throws InvalidBoundsException if the range is invalid
  //!
  Histogram project2DOnto1D(unsigned int targetAxisIndex, unsigned int first=0, unsigned int last=allBins) const;

  //!
  //! @brief Project a rank 3 histogram onto one of its axes.
  //!
  //! The two other axes, taken in increasing axis order, are summed over [first1, last1] and [first2, last2].
  //! @param targetAxisIndex axis kept in the projection (0, 1 or 2)
  //! @param first1 first bin of the first summed axis
  //! @param last1 last bin of the first summed axis
  //! @param first2 first bin of the second summed axis
  //! @param last2 last bin of the second summed axis
  //! @return rank 1 histogram with the binning of axis targetAxisIndex
  //! @throws Exception if the rank is not 3
  //! @throws IndexOutOfBoundException if targetAxisIndex>2
  //! @throws InvalidBoundsException if a range is invalid
  //!
  Histogram project3DOnto1D(unsigned int targetAxisIndex,
                            unsigned int first1=0, unsigned int last1=allBins,
                            unsigned int first2=0, unsigned int last2=allBins) const;

  //!
  //! @brief Project a rank 3 histogram onto two of its axes.
  //!
  //! The remaining axis is summed over bins [first, last].
  //! @param targetAxisIndex1 axis that becomes axis 0 of the projection
  //! @param targetAxisIndex2 axis that becomes axis 1 of the projection
  //! @param first first bin of the summed axis
  //! @param last last bin of the summed axis
  //! @return rank 2 histogram
  //! @throws Exception if the rank is not 3 or the target axes are equal
  //! @throws IndexOutOfBoundException if a target axis index is greater than 2
  //! @throws InvalidBoundsException if the range is invalid
  //!
  Histogram project3DOnto2D(unsigned int targetAxisIndex1, unsigned int targetAxisIndex2,
                            unsigned int first=0, unsigned int last=allBins) const;

  //! @}

  //! @name Sums and integrals
  //! Bin ranges are inclusive; last is clamped to the end of the axis.
  //! Range functions throw Exception on a rank mismatch and InvalidBoundsException on an invalid range.
  //! With binSumOnly=false, the sum of bin values is multiplied by the bin width, area or volume.
  //! @{

  //! @brief Total number of entries of all bins. @return sum of the bin entries
  double totalEntries() const;
  //! @brief Number of entries in bins [first, last] of a rank 1 histogram.
  //! @param first first bin @param last last bin @return sum of the bin entries
  double totalEntries(unsigned int first, unsigned int last) const;
  //! @brief Number of entries in bins [first0,last0] x [first1,last1] of a rank 2 histogram.
  //! @param first0 first bin along axis 0 @param last0 last bin along axis 0
  //! @param first1 first bin along axis 1 @param last1 last bin along axis 1 @return sum of the bin entries
  double totalEntries(unsigned int first0, unsigned int last0,
                      unsigned int first1, unsigned int last1) const;
  //! @brief Number of entries in bins [first0,last0] x [first1,last1] x [first2,last2] of a rank 3 histogram.
  //! @param first0 first bin along axis 0 @param last0 last bin along axis 0
  //! @param first1 first bin along axis 1 @param last1 last bin along axis 1
  //! @param first2 first bin along axis 2 @param last2 last bin along axis 2 @return sum of the bin entries
  double totalEntries(unsigned int first0, unsigned int last0,
                      unsigned int first1, unsigned int last1,
                      unsigned int first2, unsigned int last2) const;

  //! @brief Sum of all bin values, or integral of the histogram.
  //! @param binSumOnly if true, return the sum of bin values; if false, multiply it by the bin volume @return sum or integral
  double integral(bool binSumOnly=1) const;
  //! @brief Sum or integral of bins [first, last] of a rank 1 histogram.
  //! @param first first bin @param last last bin @param binSumOnly if false, multiply by the bin width @return sum or integral
  double integral(unsigned int first, unsigned int last,
                  bool binSumOnly=1)  const;
  //! @brief Sum or integral of bins [first0,last0] x [first1,last1] of a rank 2 histogram.
  //! @param first0 first bin along axis 0 @param last0 last bin along axis 0
  //! @param first1 first bin along axis 1 @param last1 last bin along axis 1
  //! @param binSumOnly if false, multiply by the bin area @return sum or integral
  double integral(unsigned int first0, unsigned int last0,
                  unsigned int first1, unsigned int last1,
                  bool binSumOnly=1) const;
  //! @brief Sum or integral of bins [first0,last0] x [first1,last1] x [first2,last2] of a rank 3 histogram.
  //! @param first0 first bin along axis 0 @param last0 last bin along axis 0
  //! @param first1 first bin along axis 1 @param last1 last bin along axis 1
  //! @param first2 first bin along axis 2 @param last2 last bin along axis 2
  //! @param binSumOnly if false, multiply by the bin volume @return sum or integral
  double integral(unsigned int first0, unsigned int last0,
                  unsigned int first1, unsigned int last1,
                  unsigned int first2, unsigned int last2,
                  bool binSumOnly=1) const;

  //! @}

  //! @name Extrema
  //! Searches run over the global (flat) bin index range [first, last]; last is clamped to nBinsTotal()-1.
  //! When several bins share the extremum, the first one is returned.
  //! All functions throw Exception if the histogram has no bins and InvalidBoundsException if first>last.
  //! "Value" refers to the bin value, "AvgValue" to the average value per entry.
  //! @{

  //! @brief Find the bin with the largest number of entries.
  //! @param index [out] global index of that bin @param value [out] its number of entries
  //! @param first first global bin of the search @param last last global bin of the search
  void maximumEntries(unsigned int & index, long & value, unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Find the bin with the smallest number of entries.
  //! @param index [out] global index of that bin @param value [out] its number of entries
  //! @param first first global bin of the search @param last last global bin of the search
  void minimumEntries(unsigned int & index, long & value, unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Find the bin with the largest value.
  //! @param index [out] global index of that bin @param value [out] its value
  //! @param first first global bin of the search @param last last global bin of the search
  void maximumInRange(unsigned int & index, double & value, unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Find the bin with the smallest value.
  //! @param index [out] global index of that bin @param value [out] its value
  //! @param first first global bin of the search @param last last global bin of the search
  void minimumInRange(unsigned int & index, double & value, unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Find the bin with the largest average value.
  //! @param index [out] global index of that bin @param value [out] its average value
  //! @param first first global bin of the search @param last last global bin of the search
  void maximumAvgValue(unsigned int & index, double & value, unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Find the bin with the smallest average value.
  //! @param index [out] global index of that bin @param value [out] its average value
  //! @param first first global bin of the search @param last last global bin of the search
  void minimumAvgValue(unsigned int & index, double & value, unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Largest bin entry count. @param first first global bin @param last last global bin @return largest entry count
  double maximumEntries(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Smallest bin entry count. @param first first global bin @param last last global bin @return smallest entry count
  double minimumEntries(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Largest bin value. @param first first global bin @param last last global bin @return largest bin value
  double maximumInRange(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Smallest bin value. @param first first global bin @param last last global bin @return smallest bin value
  double minimumInRange(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Largest bin average. @param first first global bin @param last last global bin @return largest bin average
  double maximumAvgValue(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Smallest bin average. @param first first global bin @param last last global bin @return smallest bin average
  double minimumAvgValue(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Bin with the largest entry count. @param first first global bin @param last last global bin @return global bin index
  unsigned int  binWithMaxEntries(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Bin with the smallest entry count. @param first first global bin @param last last global bin @return global bin index
  unsigned int  binWithMinEntries(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Bin with the largest value. @param first first global bin @param last last global bin @return global bin index
  unsigned int  binWithMaxValue(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Bin with the smallest value. @param first first global bin @param last last global bin @return global bin index
  unsigned int  binWithMinValue(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Bin with the largest average. @param first first global bin @param last last global bin @return global bin index
  unsigned int  binWithMaxAvgValue(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Bin with the smallest average. @param first first global bin @param last last global bin @return global bin index
  unsigned int  binWithMinAvgValue(unsigned int first=0, unsigned int last=allBins) const;

  //! @}

  //! @name Clearing and filling
  //! Each fill adds one entry to the bin containing the given coordinates: the bin value increases
  //! by weight and the squared uncertainty of the value by weight^2. Coordinates outside the
  //! histogram range are ignored. A rank mismatch throws Exception.
  //! @{

  //! @brief Clear this histogram: all axes and bins are removed (rank 0, no bins). Name and title are kept.
  virtual void clear() override;
  //! @brief Reset all bins to zero (entries, values, uncertainties) without changing the binning.
  virtual void reset() override;
  //! @brief Fill a rank 1 histogram. @param x0 coordinate @param weight weight of the entry
  void fill(double x0, double weight=1.0);
  //! @brief Fill a rank 2 histogram. @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param weight weight of the entry
  void fill(double x0, double x1, double weight);
  //! @brief Fill a rank 3 histogram.
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param x2 coordinate along axis 2 @param weight weight of the entry
  void fill(double x0, double x1, double x2, double weight);
  //! @brief Fill a histogram of any rank. @param x coordinates along each axis (size must equal rank()) @param weight weight of the entry
  void fill(const std::vector<double> & x, double weight=1.0);
  //!
  //! @brief Add one entry of the given weight to a bin given by its global index (faster than fill() when the bin is
  //! already known, e.g. in pair loops).
  //! @param k global bin index @param weight weight of the entry @throws IndexOutOfBoundException if k is out of range
  //!
  void fillBin(unsigned int k, double weight=1.0)
  {
  checkGlobal(k,__FUNCTION__);
  double e = valueUncertainty(k);
  setValue(k,_entries[k]+1,value(k)+weight,std::sqrt(e*e+weight*weight));
  }
  //!
  //! @brief Add one entry of the given weight to a bin of a rank 2 histogram given by its bin indices (from 0).
  //! @param i0 bin index along axis 0 @param i1 bin index along axis 1 @param weight weight of the entry
  //! @throws IndexOutOfBoundException if an index is out of range
  //!
  void fillBin(unsigned int i0, unsigned int i1, double weight) { fillBin(binIndex(i0,i1,true),weight); }

  //! @}

  //! @name Bin getters
  //! Indices are checked strictly: an out of range index throws IndexOutOfBoundException,
  //! and a number of indices different from the rank throws Exception.
  //! "Value" is the bin value (entries*average); "AvgValue" is the average value per entry.
  //! @{

  //! @brief Entries of a bin (rank 1). @param binIndex bin index @return number of entries
  double binEntriesAt(unsigned int binIndex) const;
  //! @brief Entries of a bin (rank 2). @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @return number of entries
  double binEntriesAt(unsigned int binIndex0, unsigned int binIndex1) const;
  //! @brief Entries of a bin (rank 3).
  //! @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @param binIndex2 index along axis 2 @return number of entries
  double binEntriesAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2) const;
  //! @brief Entries of a bin (any rank). @param indices bin indices along each axis @return number of entries
  double binEntriesAt(const std::vector<unsigned int> & indices) const;

  //! @brief Value of a bin (rank 1). @param binIndex bin index @return bin value
  double binValueAt(unsigned int binIndex) const;
  //! @brief Value of a bin (rank 2). @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @return bin value
  double binValueAt(unsigned int binIndex0, unsigned int binIndex1) const;
  //! @brief Value of a bin (rank 3).
  //! @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @param binIndex2 index along axis 2 @return bin value
  double binValueAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2) const;
  //! @brief Value of a bin (any rank). @param indices bin indices along each axis @return bin value
  double binValueAt(const std::vector<unsigned int> & indices) const;

  //! @brief Uncertainty of the value of a bin (rank 1). @param binIndex bin index @return uncertainty of the bin value
  double binUncertaintyAt(unsigned int binIndex) const;
  //! @brief Uncertainty of the value of a bin (rank 2).
  //! @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @return uncertainty of the bin value
  double binUncertaintyAt(unsigned int binIndex0, unsigned int binIndex1) const;
  //! @brief Uncertainty of the value of a bin (rank 3).
  //! @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @param binIndex2 index along axis 2 @return uncertainty of the bin value
  double binUncertaintyAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2) const;
  //! @brief Uncertainty of the value of a bin (any rank). @param indices bin indices along each axis @return uncertainty of the bin value
  double binUncertaintyAt(const std::vector<unsigned int> & indices) const;

  //! @brief Average value of a bin (rank 1). @param binIndex bin index @return bin average
  double binAvgValueAt(unsigned int binIndex) const;
  //! @brief Average value of a bin (rank 2). @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @return bin average
  double binAvgValueAt(unsigned int binIndex0, unsigned int binIndex1) const;
  //! @brief Average value of a bin (rank 3).
  //! @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @param binIndex2 index along axis 2 @return bin average
  double binAvgValueAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2) const;
  //! @brief Average value of a bin (any rank). @param indices bin indices along each axis @return bin average
  double binAvgValueAt(const std::vector<unsigned int> & indices) const;

  //! @brief Uncertainty of the average value of a bin (rank 1). @param binIndex bin index @return uncertainty of the bin average
  double binAvgValueUncertaintyAt(unsigned int binIndex) const;
  //! @brief Uncertainty of the average value of a bin (rank 2).
  //! @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @return uncertainty of the bin average
  double binAvgValueUncertaintyAt(unsigned int binIndex0, unsigned int binIndex1) const;
  //! @brief Uncertainty of the average value of a bin (rank 3).
  //! @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @param binIndex2 index along axis 2 @return uncertainty of the bin average
  double binAvgValueUncertaintyAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2) const;
  //! @brief Uncertainty of the average value of a bin (any rank). @param indices bin indices along each axis @return uncertainty of the bin average
  double binAvgValueUncertaintyAt(const std::vector<unsigned int> & indices) const;

  //! @}

  //! @name Direct access to the bin arrays
  //! Arrays are indexed by global bin index.
  //! @{

  //! @brief Read-only access to the entries of all bins. @return vector of bin entries
  const std::vector<long>  & binEntries() const { return _entries; }
  //! @brief Read-only access to the average values of all bins. @return bin averages (float or double storage)
  const BinValues & binAvgValues() const { return _avgValues; }
  //! @brief Read-only access to the uncertainties of the bin averages. @return uncertainties (float or double storage)
  const BinValues & binAvgValueUncertainties() const { return _avgUncertainties; }
  //! @brief Writable access to the entries of all bins. @return vector of bin entries
  std::vector<long>  & binEntries() { return _entries; }
  //! @brief Writable access to the average values of all bins (their number must not change). @return bin averages
  BinValues & binAvgValues() { return _avgValues; }
  //! @brief Writable access to the uncertainties of the bin averages (their number must not change). @return uncertainties
  BinValues & binAvgValueUncertainties() { return _avgUncertainties; }
  //! @brief Value (entries*average) of every bin. @return vector of bin values; empty bins are flagged with -1.0E100
  std::vector<double> binValues() const;
  //! @brief Uncertainty of the value of every bin. @return vector of uncertainties; empty bins are flagged with -1.0E100
  std::vector<double> binValueUncertainties() const;

  //! @}

  //! @name Data points (DataPoints interface)
  //! Points are indexed by the global bin index (see Partition).
  //! @{

  //!
  //! @brief Number of data points, i.e., total number of bins (all axes).
  //! @return number of bins
  //!
  unsigned int size() const override;

  //!
  //! @brief Rank of the histogram (number of axes, see Partition::rank()); also the rank of its data points.
  //! @return rank
  //!
  unsigned int rank() const override { return Partition::rank(); }

  //! @brief Centers of a bin along each axis. @param index global bin index @return rank() bin centers
  //! @throws IndexOutOfBoundException if index>=size()
  std::vector<double> coordinatesAt(unsigned int index) const override;
  //! @brief Uncertainties of the coordinates of a bin: zero. @param index global bin index @return rank() zeros
  //! @throws IndexOutOfBoundException if index>=size()
  std::vector<double> coordinateErrorsAt(unsigned int index) const override;
  //! @brief Value of a bin. @param index global bin index @return bin value
  //! @throws IndexOutOfBoundException if index>=size()
  double valueAt(unsigned int index) const override;
  //! @brief Uncertainty of the value of a bin. @param index global bin index @return uncertainty of the bin value
  //! @throws IndexOutOfBoundException if index>=size()
  double statErrorAt(unsigned int index) const override;
  //! @brief Systematic uncertainty of a bin: 0, a histogram has none. @param index global bin index @return 0
  //! @throws IndexOutOfBoundException if index>=size()
  double sysErrorAt(unsigned int index) const override;

  //!
  //! @brief Data point of a bin: x = bin center, y = bin value, statistical uncertainty = uncertainty of the bin value,
  //! no systematic uncertainty, no x uncertainty.
  //! @param index bin index (0 to size()-1)
  //! @return data point of the bin
  //! @throws MathException if the rank is not 1; IndexOutOfBoundException if index>=size()
  //!
  DataPoint2D point(unsigned int index) const override;

  //!
  //! @brief Not supported: a histogram is filled with fill(), not with data points.
  //! @throws MathException always
  //!
  void addPoint(const DataPoint2D & dataPoint) override;
  using DataPoints::addPoint;

  //! @}

  //! @name Binary files (independent of ROOT)
  //!
  //! Compact, portable binary format. A file holds one or more histogram records, one after the other:
  //!  - "CAPH" (4 bytes), format version (1 byte), payload length (varint), payload, CRC-32 of the payload (4 bytes)
  //!  - payload: name, title, partition type, rank, then for each axis: number of bins, minimum and maximum (float32),
  //!    title; then the number of non-empty bins and, for each, the gap in global bin index since the previous
  //!    non-empty bin (varint), the entries (zigzag varint), the average value and its uncertainty (float32); then
  //!    the bin labels of each axis and the fits attached (format version 4, see FitList::encodeBinary())
  //! Integers use variable-length encoding (1 byte below 128), multi-byte values are little-endian, and bins with
  //! zero entries, value and uncertainty are not written: a typical filled bin takes about 10 bytes, an empty bin 0.
  //! The content is restored exactly. Corrupted or truncated data is detected (FileException).
  //! Records of other classes in the same file are skipped. See BinaryIO.hpp for the record format and encodings.
  //! @{

  //! @brief Tag of histogram records in binary files. @return "CAPH"
  static const char * binaryTag() { return "CAPH"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 4; }
  //! @brief Name stored in binary records. @return name()
  String binaryName() const { return name(); }
  //! @brief Write the binary payload after the name (used by writeBinary()). @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Rebuild this histogram from a binary payload (used by readBinary()). @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);

  //!
  //! @brief Write this histogram as one binary record to a stream (open it in binary mode).
  //! @param out output stream
  //! @throws FileException if writing fails
  //!
  void writeBinary(std::ostream & out) const;
  //!
  //! @brief Read the next binary record from a stream into this histogram (replacing its content).
  //! @param in input stream (binary mode)
  //! @return false if the stream is at its end (this histogram is then unchanged), true if a histogram was read
  //! @throws FileException if the data is corrupted, truncated or of an unsupported format version (unchanged then too)
  //!
  bool readBinary(std::istream & in);
  //!
  //! @brief Save this histogram to a binary file.
  //! @param fileName file name @param append if true, add the histogram at the end of the file instead of replacing it
  //! @throws FileException if the file cannot be written
  //!
  void saveBinary(const String & fileName, bool append=false) const;
  //!
  //! @brief Load a histogram from a binary file: the first one, or the first one with the given name.
  //! @param fileName file name @param name name of the histogram to load (empty: the first histogram of the file)
  //! @throws FileException if the file cannot be read, is corrupted, or holds no histogram with that name
  //!
  void loadBinary(const String & fileName, const String & name="");
  //!
  //! @brief Save several histograms to one binary file (replacing it).
  //! @param fileName file name @param histograms histograms to save, in order
  //! @throws FileException if the file cannot be written
  //!
  static void saveBinary(const String & fileName, const std::vector<Histogram> & histograms);
  //!
  //! @brief Load all the histograms of a binary file, in order.
  //! @param fileName file name @return histograms
  //! @throws FileException if the file cannot be read or is corrupted
  //!
  static std::vector<Histogram> loadAllBinary(const String & fileName);

  //! @}

  //!
  //! @brief Print the name, title, rank and the content of every bin to std::cout.
  //!
  virtual void print() const override;

  //!
  //! @brief Write the name, title, rank and the content of every bin to a stream.
  //!
  //! Each bin line lists: global index, entries, value, uncertainty of the value, average, uncertainty of the average.
  //! @param out output stream
  //! @param h histogram to write
  //! @return the output stream
  //!
  friend std::ostream& operator<<(std::ostream& out, const Histogram & h);


  };

  //! @relates Histogram
  //! @brief Bin-by-bin sum of two histograms with the same binning (Exception otherwise).
  //! @param left first operand @param right second operand @return left+right
  Histogram operator+(const Histogram & left, const Histogram & right);
  //! @relates Histogram
  //! @brief Bin-by-bin difference of two histograms with the same binning (Exception otherwise).
  //! @param left first operand @param right second operand @return left-right
  Histogram operator-(const Histogram & left, const Histogram & right);
  //! @relates Histogram
  //! @brief Bin-by-bin product of two histograms with the same binning (see Histogram::multiplyBy).
  //! @param left first operand @param right second operand @return left*right
  Histogram operator*(const Histogram & left, const Histogram & right);
  //! @relates Histogram
  //! @brief Bin-by-bin ratio of two histograms with the same binning (see Histogram::divideBy).
  //! @param left numerator @param right denominator @return left/right
  Histogram operator/(const Histogram & left, const Histogram & right);
  //! @relates Histogram
  //! @brief Add a constant to every bin value. @param left histogram @param right constant @return left+right
  Histogram operator+(const Histogram & left, double right);
  //! @relates Histogram
  //! @brief Add a constant to every bin value. @param left constant @param right histogram @return left+right
  Histogram operator+(double left, const Histogram & right);
  //! @relates Histogram
  //! @brief Subtract a constant from every bin value. @param left histogram @param right constant @return left-right
  Histogram operator-(const Histogram & left, double right);
  //! @relates Histogram
  //! @brief Subtract every bin value from a constant. @param left constant @param right histogram @return left-right
  Histogram operator-(double left, const Histogram & right);
  //! @relates Histogram
  //! @brief Scale every bin value by a constant. @param left histogram @param right constant @return left*right
  Histogram operator*(const Histogram & left, double right);
  //! @relates Histogram
  //! @brief Scale every bin value by a constant. @param left constant @param right histogram @return left*right
  Histogram operator*(double left, const Histogram & right);
  //! @relates Histogram
  //! @brief Divide every bin value by a non-zero constant (Exception if right==0). @param left histogram @param right constant @return left/right
  Histogram operator/(const Histogram & left, double right);
  //! @relates Histogram
  //! @brief Divide a constant by every bin value (see Histogram::inverse). @param left constant @param right histogram @return left/right
  Histogram operator/(double left, const Histogram & right);
  //! @relates Histogram
  //! @brief Unary plus: copy of the histogram. @param right histogram @return copy of right
  Histogram operator+(const Histogram & right);
  //! @relates Histogram
  //! @brief Unary minus: bin values negated, uncertainties unchanged. @param right histogram @return -right
  Histogram operator-(const Histogram & right);


  //////////////////////////////////////////////////////////////////////////
  // Histogram helper functions
  //
  // Ported from Helpers/RootHistogramHelpers, which keeps the versions for ROOT TH1/TH2/TH3/TProfile (used by
  // RootHistogramGroup). TH1, TH2, TH3 and TProfile arguments are Histogram objects of rank 1, 2 or 3. As in the ROOT
  // versions, bins are numbered from 1 along each axis wherever bin numbers appear in arguments or results; the content
  // of a bin is its value and its error the uncertainty of that value. Histograms have no under/overflow bins.
  //////////////////////////////////////////////////////////////////////////

  //! @name Checks
  //! Null pointers throw NullPointerException; dimension checks compare ranks and numbers of bins.
  //! @{
  template <class... H> bool ptrExist(const String & caller, const H*... histograms) { for (bool ok : {histograms!=nullptr...}) if (!ok) throw NullPointerException(caller); return true; }
  int getDimension(const Histogram * h);
  void printDimensions(const Histogram * h);
  void printHistoInfo(const Histogram * h);
  bool isValid(const String & name, int n_x, double min_x, double max_x, const String & caller, bool verbose=false);
  bool isValid(const String & name, int n_x, double min_x, double max_x, int n_y, double min_y, double max_y, const String & caller, bool verbose=false);
  bool isValid(const String & name, int n_x, double min_x, double max_x, int n_y, double min_y, double max_y, int n_z, double min_z, double max_z, const String & caller, bool verbose=false);
  bool sameDimensions(const String & caller, const Histogram* h1, const Histogram* h2);
  bool sameDimensions(const String & caller, const Histogram* h1, const Histogram* h2, const Histogram* h3);
  bool sameDimensions(const String & caller, const Histogram* h1, const Histogram* h2, const Histogram* h3, const Histogram* h4);
  bool sameDimensions(const String & caller, const Histogram* h1, const Histogram* h2, const Histogram* h3, const Histogram* h4, const Histogram* h5);
  void printHistoInfo(const String & source, const String & histoName, int x_nbins=-1, double x_min=-1, double x_max=-1, int y_nbins=-1, double y_min=-1, double y_max=-1, int z_nbins=-1, double z_min=-1, double z_max=-1);
  //! @}

  //! @name Creation, titles, copies and files
  //! storageOption: 0 (int) and 1 (float) give single precision, 2 (double) double precision (see Histogram::Precision).
  //! Variable-width binning is not available (Histogram axes have bins of equal width). Profiles are histograms filled
  //! with fill(x,value). setTitles() names the first rank() axes and uses the next title as the title of the value.
  //! The caller owns the histograms returned by createNew*, clone, safeClone and loadHistogram.
  //! @{
  Histogram * createNewHistogram(const String & name, int n_x, double min_x, double max_x, const String & title_x="x", const String & title_y="N", int storageOption=1, bool verbose=false);
  Histogram * createNewHistogram(const String & name, int n_x, double min_x, double max_x, int n_y, double min_y, double max_y, const String & title_x="x", const String & title_y="y", const String & title_z="N", int storageOption=1, bool verbose=false);
  Histogram * createNewHistogram(const String & name, int n_x, double min_x, double max_x, int n_y, double min_y, double max_y, int n_z, double min_z, double max_z, const String & title_x="x", const String & title_y="y", const String & title_z="z", const String & title_w="N", int storageOption=1, bool verbose=false);
  Histogram * createNewProfile(const String & name, int n_x, double min_x, double max_x, const String & title_x, const String & title_y, bool verbose=false);
  Histogram * createNewProfile(const String & name, int n_x, double min_x, double max_x, int n_y, double min_y, double max_y, const String & title_x, const String & title_y, const String & title_z, bool verbose=false);
  void setTitles(Histogram * histogram, const String & title_x, const String & title_y, const String & title_z="", const String & title_w="");
  Histogram * clone(const Histogram * h1, const String & histoName);
  Histogram * safeClone(const Histogram * h);
  void cloneHistoVector(const std::vector<Histogram*> & source, std::vector<Histogram*> & target);
  Histogram * loadHistogram(const CapFile & inputFile, const String & histoName);
  //! @}

  //! @name Arithmetic
  //! Operations act bin by bin on bin values and their uncertainties.
  //! @{
  void sumHistograms(double a1, const Histogram * h1, Histogram * sum);
  void sumHistograms(double a1, const Histogram * h1, double a2, const Histogram * h2, Histogram * sum);
  void sumHistograms(double a1, const Histogram * h1, double a2, const Histogram * h2, double a3, const Histogram * h3, Histogram * sum);
  void sumHistograms(double a1, const Histogram * h1, double a2, const Histogram * h2, double a3, const Histogram * h3, double a4, const Histogram * h4, Histogram * sum);
  void sumHistograms(const std::vector<double> & a, const std::vector<const Histogram*> & histograms, Histogram * sum);
  void divideHistograms(double a1, const Histogram * h1, double a2, const Histogram * h2, Histogram * ratio);
  void differenceHistograms(const Histogram * h1, const Histogram * h2, Histogram * difference);
  void squareDifferenceHistos(Histogram * hAvg, const Histogram * h, double sumWeights, double weight, int n);
  void differenceHistos(const Histogram * h, const Histogram * hRef, Histogram * hDiff, bool correlatedUncertainties);
  void ratioHistos(const Histogram * h, const Histogram * hRef, Histogram * hRatio, bool correlatedUncertainties);
  void setHistogram(Histogram * h, double v, double ev);
  //! @}

  //! @name Minima, maxima and bin widths
  //! findMinimum/findMaximum/findMinMax report ROOT-style bin numbers (from 1). They only update the bin and value
  //! arguments when they find a smaller (larger) value, so that they can accumulate over several histograms: initialize
  //! minValue to a large and maxValue to a small number before the first call.
  //! @{
  double findHistoMinimum(const Histogram * h);
  double findHistoMaximum(const Histogram * h);
  double findHistoMinimum2D(const Histogram * h);
  double findHistoMaximum2D(const Histogram * h);
  void scaleByBinWidth1D(Histogram * h, double scale);
  void scaleByBinWidth2D(Histogram * h, double scale);
  void scaleByBinWidth3D(Histogram * h, double scale);
  void scaleByBinWidth(Histogram * h, double scale);
  double avgValue(Histogram * h);
  void findMinimum(Histogram * h, int & xMinValueBin, double & minValue, int xFirstBin=1, int xLastBin=-1);
  void findMaximum(Histogram * h, int & xMaxValueBin, double & maxValue, int xFirstBin=1, int xLastBin=-1);
  void findMinMax(Histogram * h, int & xMinValueBin, double & minValue, int & xMaxValueBin, double & maxValue, int xFirstBin=1, int xLastBin=-1);
  void findMinMax(std::vector<Histogram*> histograms, int & xMinValueBin, double & minValue, int & xMaxValueBin, double & maxValue, int xFirstBin=1, int xLastBin=-1);
  void findMinimum(Histogram * h, int & xMinValueBin, int & yMinValueBin, double & minValue, int xFirstBin=1, int xLastBin=-1, int yFirstBin=1, int yLastBin=-1);
  void findMaximum(Histogram * h, int & xMaxValueBin, int & yMaxValueBin, double & maxValue, int xFirstBin=1, int xLastBin=-1, int yFirstBin=1, int yLastBin=-1);
  void findMinMax(Histogram * h, int & xMinValueBin, int & yMinValueBin, double & minValue, int & xMaxValueBin, int & yMaxValueBin, double & maxValue, int xFirstBin=1, int xLastBin=-1, int yFirstBin=1, int yLastBin=-1);
  void findMinMax(std::vector<Histogram*> histograms, int & xMinValueBin, int & yMinValueBin, double & minValue, int & xMaxValueBin, int & yMaxValueBin, double & maxValue, int xFirstBin=1, int xLastBin=-1, int yFirstBin=1, int yLastBin=-1);
  //! @}

  //! @name Integrals, averages and partitions
  //! Integration ranges are given in axis coordinates. calculateCumulativeIntegral returns its points as a DataPointsVector
  //! (the ROOT version returned a TGraph). partitionHistogram appends the boundaries to its boundaries argument.
  //! @{
  void calculateIntegral(const Histogram * h, double xMin, double xMax, double & result, double & resultError, int option);
  void calculateIntegral(const Histogram * h, double xMin, double xMax, double yMin, double yMax, double & result, double & resultError, int option);
  DataPointsVector calculateCumulativeIntegral(const Histogram & h, double xLow, double xHigh, double xStep);
  void calculateRmsWidth(const Histogram * h, double xLowEdge, double xHighEdge, double & mean, double & meanError, double & rmsWidth, double & rmsWidthError);
  void partitionHistogram(const std::vector<double> & fractions, std::vector<double> & boundaries, const Histogram * histogram, double minimum=+1.0, double maximum=-1.0);
  void calculateAverage(Histogram* h, double & avgDensity, double & eAvgDensity);
  //! @}

  //! @name Profiles
  //! Profile arguments are histograms filled with fill(x,value): their bin averages are the profile values.
  //! @{
  void calculateR2VsM(const Histogram * h1, const Histogram * h2, const Histogram * h12, Histogram * r2VsM, Histogram * intR2, bool sameFilter);
  void calculateBinCorr(const Histogram * h1, const Histogram * h2, Histogram * intBinCorrVsM1, bool sameFilter);
  void calculateInclusiveAvgFromProfiles(const Histogram * h1, const Histogram * h2, Histogram * h3);
  //! @}

  //! @name Correlation functions and derived quantities
  //! Monte Carlo helpers use a fixed-seed std::mt19937_64 generator (instead of ROOT's gRandom).
  //! @{
  double calculateN1N1N1(const Histogram * h_1, const Histogram * h_2, const Histogram * h_3, Histogram * h_123);
  void calculateBf(const Histogram *n2, const Histogram *n1_1, const Histogram *n1_2, Histogram *bf_12, Histogram *bf_21);
  void calculateN1N1H2H2_Q3D_MCY(const Histogram * n1_1, const Histogram * n1_2, Histogram * n1n1_Q3D, double a1, double a2);
  void calculateN1N1H2H2_Q3D_MCEta(const Histogram * n1_1, const Histogram * n1_2, Histogram * n1n1_Q3D, double a1, double a2);
  void calculateN1N1H2H2_Q3D(const Histogram * n1_1, const Histogram * n1_2, Histogram * n1n1_Q3D, double a1, double a2);
  void calculateN1N1H3H3_Q3D(const Histogram * n1_1, const Histogram * n1_2, Histogram * n1n1_Q3D, double a1, double a2);
  double calculateN1N1(const Histogram * h_1, const Histogram * h_2, Histogram * h_12, double a1, double a2);
  double calculateN1N1_H1H1H2(const Histogram * h_1, const Histogram * h_2, Histogram * h_12, double a1, double a2);
  double calculateN1N1_H2H2H2(const Histogram *h_1, const Histogram * h_2, Histogram * h_12, double a1, double a2);
  void calculateN2N1(const Histogram * s2, const Histogram* s1, Histogram * target, int single);
  void calculateN2N1x(const Histogram * s2, const Histogram* s1, Histogram * target, int single);
  void calculateDptDpt(const Histogram * spp, const Histogram * spn, const Histogram * snp, const Histogram * snn, const Histogram * avgp1, const Histogram * avgp2, Histogram * s2dptdpt, Histogram * dptdpt, bool ijNormalization, int nEta, int nPhi);
  void calculateDptDpt(const Histogram * spp, const Histogram * spn, const Histogram * snp, const Histogram * snn, const Histogram * avgp1, const Histogram * avgp2, Histogram * dptdpt, bool ijNormalization, int nBins);
  void calculateSc(const Histogram * spp, const Histogram * n1n1, const Histogram * pt1pt1, Histogram * g2, bool ijNormalization);
  void calculateG2_H1H2H2H2(const Histogram * spp, const Histogram * n1n1, const Histogram * pt1pt1, Histogram * g2, bool ijNormalization, double a1, double a2);
  void calculateG2_H2H2H2H2(const Histogram * spp, const Histogram * n1n1, const Histogram * pt1pt1, Histogram * g2, bool ijNormalization, double a1, double a2);
  int calculateQ3DwPtPhiEta(double pt1, double phi1, double eta1, double pt2, double phi2, double eta2, double & Qlong, double & Qout, double & Qside);
  int calculateQ3DwPtPhiY(double pt1, double phi1, double y1, double pt2, double phi2, double y2, double & Qlong, double & Qout, double & Qside);
  void calculateR2_Q3D(const Histogram * n2_Q3D, const Histogram * n1n1_Q3D, Histogram * R2_Q3D, double a1, double a2);
  void unpack_vsXY_to_vsXVsY(const Histogram * source, Histogram * target);
  void correctMerging(Histogram * h, int nEta, int nPhi, bool reverse);
  void calculateC2_H1H1H1(const Histogram * n2_12, const Histogram * n1n1_12, Histogram * c2_12, double a1=1.0, double a2=1.0);
  void calculateC2_H2H2H2(const Histogram * n2_12, const Histogram * n1n1_12, Histogram * c2_12, double a1=1.0, double a2=1.0);
  void calculateC2_H3H3H3(const Histogram * n2_12, const Histogram * n1n1_12, Histogram * c2_12, double a1=1.0, double a2=1.0);
  void calculateR2_H1H1H1(const Histogram * n2_12, const Histogram * n1n1_12, Histogram * r2_12, bool ijNormalization=0, double a1=1.0, double a2=1.0);
  void calculateR2_H2H2H2(const Histogram * n2_12, const Histogram * n1n1_12, Histogram * r2_12, bool ijNormalization=0, double a1=1.0, double a2=1.0);
  void calculateR2_H3H3H3(const Histogram * n2_12, const Histogram * n1n1_12, Histogram * r2_12, bool ijNormalization=0, double a1=1.0, double a2=1.0);
  void calculateR2_H1H2H2(const Histogram * n2_12, const Histogram * n1n1_12, Histogram * r2_12, bool ijNormalization=0, double a1=1.0, double a2=1.0);
  void calculateAverageVsDeta(const Histogram * obs_12, Histogram * avgObs_12, int n);
  void calculateInclusiveAvg(const Histogram * h1, const Histogram * h2, Histogram * h3);
  void calculateInclusiveAvgH2(const Histogram * h1, const Histogram * h2, Histogram * h3);
  void calculateInclusiveAvgH1H2(const Histogram * h1, const Histogram * h2, Histogram * h3);
  void symmetrize3D(Histogram* h);
  void symmetrizeDeltaEtaDeltaPhi(Histogram * h, bool ijNormalization);
  void symmetrizeXX(Histogram * h, bool ijNormalization);
  void reduce_n2xEtaPhi_n2DetaDphi(const Histogram * source, Histogram * target,int nEtaBins,int nPhiBins);
  void reduce_n2xEtaPhi_n2EtaEta(const Histogram * source, Histogram * target,int nEtaBins,int nPhiBins);
  void reduce_n1EtaPhiN1EtaPhiOntoN1N1DetaDphi(const Histogram * h_1, Histogram * h_2, Histogram * h_12,int nDeta,int nDphi);
  void calculateAvgH2H2(const Histogram * h_1, Histogram * h_2, double scaleFactor);
  void project_n2XYXY_n2XX(const Histogram * source, Histogram * target,int nXBins,int nYBins);
  void project_n2XYXY_n2YY(const Histogram * source, Histogram * target,int nXBins,int nYBins);
  void shiftY(const Histogram & source, Histogram & target, int nbins);
  void calculateF1(Histogram* h_f1_1, Histogram* h_F1_1);
  void calculateF2R2(Histogram* h_f1_1, Histogram* h_f1_2, Histogram* h_f2_12, Histogram* h_F2_12, Histogram* h_R2_12);
  void calculateNudyn(Histogram* h_R2_11, Histogram* h_R2_12, Histogram* h_R2_21, Histogram* h_R2_22, Histogram* h_nudyn_12);
  void calculateF3R3(Histogram* h_f1_1, Histogram* h_f1_2, Histogram* h_f1_3, Histogram* h_f2_12, Histogram* h_f2_13, Histogram* h_f2_23, Histogram* h_f3_123, Histogram* h_F3_123, Histogram* h_R2_123);
  void calculateF4R4(Histogram* h_f1_1, Histogram* h_f1_2, Histogram* h_f1_3, Histogram* h_f1_4, Histogram* h_f2_12, Histogram* h_f2_13, Histogram* h_f2_14, Histogram* h_f2_23, Histogram* h_f2_24, Histogram* h_f2_34, Histogram* h_f3_123, Histogram* h_f3_124, Histogram* h_f3_134, Histogram* h_f3_234, Histogram* h_f4_1234, Histogram* h_F4_1234, Histogram* h_R4_1234);
  void calculateF2R2(double f1_1,double ef1_1,double f1_2,double ef1_2, double f2_12,double ef2_12,double & F2_12,double & eF2_12, double & R2_12,double & eR2_12);
  void calculateF3R3(double f1_1,double ef1_1,double f1_2,double ef1_2, double f1_3,double ef1_3, double f2_12,double ef2_12,double f2_13,double ef2_13,double f2_23,double ef2_23, double f3_123, double ef3_123, double & F3_123,double & eF3_123, double & R3_123,double & eR3_123);
  void calculateF4R4(double f1_1,double ef1_1,double f1_2,double ef1_2, double f1_3,double ef1_3, double f1_4,double ef1_4, double f2_12,double ef2_12,double f2_13,double ef2_13,double f2_14,double ef2_14,double f2_23,double ef2_23, double f2_24,double ef2_24,double f2_34,double ef2_34, double f3_123, double ef3_123,double f3_124, double ef3_124, double f3_134, double ef3_134,double f3_234, double ef3_234, double f4_1234, double ef4_1234, double & F4_1234,double & eF4_1234, double & R4_1234,double & eR4_1234);
  void calculateNudyn(double r2_11,double er2_11, double r2_12,double er2_12, double r2_21,double er2_21, double r2_22,double er2_22, double & nudyn,double & enudyn);
  //! @}

  } // namespace MATH
} // namespace CAP

#endif
