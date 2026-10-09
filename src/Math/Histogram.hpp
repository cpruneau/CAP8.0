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
#include "BinaryIO.hpp"

namespace CAP
{
  namespace MATH
  {

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
  //! The bin <em>value</em> (total) is n*a = binValueAt() and its uncertainty is n*u = binUncertaintyAt().
  //! Filling with weight w adds w to the bin total and w^2 to the squared uncertainty of the total.
  //! All arithmetic operations are carried out on bin totals, with statistical uncertainties
  //! propagated assuming uncorrelated operands.
  //!
  //! <b>Ranges.</b> Functions taking first/last bin arguments use inclusive ranges [first, last].
  //! A value of last beyond the end of the axis (e.g., Histogram::allBins, the default) is clamped
  //! to the last bin of the axis.
  //!
  //! <b>Products, ratios and inversions.</b> The entry count of such results has no meaning: bins
  //! are stored with one entry (average equals total). Bins where the result is undefined
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
  //! with ChiSquare: the point of global bin k has the bin centers as coordinates, the bin total as value,
  //! the uncertainty of the bin total as statistical uncertainty, and no coordinate uncertainty.
  //! A histogram has no systematic uncertainty: the statistical uncertainty is returned when the systematic
  //! one is requested. For rank 1, point(k) also returns these as a DataPoint2D (x = bin center, y = bin total).
  //!
  class Histogram
  :
  public Object,
  public Partition,
  public DataPoints
  {
  protected:
  std::vector<long>  _entries;        //!< Number of entries of each bin
  std::vector<float> _values;         //!< Average value per entry of each bin
  std::vector<float> _uncertainties;  //!< Uncertainty of the average value of each bin

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
  //! @brief Total value (entries*average) of global bin k.
  //! @param k global bin index (not checked)
  //! @return bin total
  //!
  double total(unsigned int k) const            { return double(_entries[k])*_values[k]; }

  //!
  //! @brief Uncertainty of the total value of global bin k.
  //! @param k global bin index (not checked)
  //! @return uncertainty of the bin total
  //!
  double totalUncertainty(unsigned int k) const { return double(_entries[k])*_uncertainties[k]; }

  //!
  //! @brief Store the content of global bin k from its entry count and totals.
  //!
  //! If entries<=0 the bin is set empty.
  //! @param k global bin index (not checked)
  //! @param entries number of entries
  //! @param total bin total
  //! @param totalUncertainty uncertainty of the bin total
  //!
  void   setTotal(unsigned int k, long entries, double total, double totalUncertainty);

  //!
  //! @brief Set global bin k empty (zero entries, value and uncertainty).
  //! @param k global bin index (not checked)
  //!
  void   setEmpty(unsigned int k);

  //!
  //! @brief Resize the bin arrays to nBinsTotal() and set all bins empty.
  //!
  void   resizeBins();

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
  //! @brief Set the entry count of global bin k, keeping the bin total and its uncertainty.
  //! @param k global bin index (not checked)
  //! @param entries number of entries
  //! @throws Exception if entries<=0 and the bin has non-zero content
  //!
  void   doSetEntries(unsigned int k, long entries);

  //!
  //! @brief Set the total value of global bin k, keeping its entry count (1 if the bin was empty).
  //! @param k global bin index (not checked)
  //! @param value bin total
  //!
  void   doSetValue(unsigned int k, double value);

  //!
  //! @brief Set the uncertainty of the total of global bin k, keeping its entry count (1 if the bin was empty).
  //! @param k global bin index (not checked)
  //! @param uncertainty uncertainty of the bin total
  //!
  void   doSetUncertainty(unsigned int k, double uncertainty);

  //!
  //! @brief Set the full content of global bin k.
  //! @param k global bin index (not checked)
  //! @param entries number of entries
  //! @param value bin total
  //! @param uncertainty uncertainty of the bin total
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
  //! @param quantity 0: entries, 1: total value, 2: average value
  //! @return requested quantity
  //!
  double binQuantity(unsigned int k, int quantity) const;

  //!
  //! @brief Find the global bin with the largest or smallest value of a bin quantity.
  //!
  //! When several bins share the extremum, the first one is returned.
  //! @param quantity 0: entries, 1: total value, 2: average value
  //! @param findMaximum true to find the maximum, false for the minimum
  //! @param first first global bin of the search range
  //! @param last last global bin of the search range (clamped)
  //! @return global index of the extremum bin
  //!
  unsigned int extremumBin(int quantity, bool findMaximum, unsigned int first, unsigned int last) const;

  //!
  //! @brief Add the content of bin kSource of histogram h into bin kTarget of this histogram.
  //!
  //! Entries and totals are summed; uncertainties are added in quadrature.
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
  //! @param values total value of each bin
  //! @param uncertainties uncertainty of each bin total, or nullptr for zero uncertainties
  //! @param source name of the calling function, reported in the exception
  //! @throws Exception if a vector size differs from nBinsTotal()
  //!
  template<typename E, typename V, typename U>
  void   setWithTotals(const std::vector<E> * entries, const std::vector<V> & values,
                       const std::vector<U> * uncertainties, const char * source);

  public:

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
  //! Two histograms are equal if they have compatible binning and all their bin totals agree
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
  //! @brief Add histogram rhs bin by bin: entries and totals are summed, uncertainties added in quadrature.
  //! @param rhs histogram to add (same binning)
  //! @return reference to this histogram
  //! @throws Exception if the binnings differ
  //!
  Histogram & operator+= (const Histogram & rhs);

  //!
  //! @brief Subtract histogram rhs bin by bin: entries are summed, totals subtracted, uncertainties added in quadrature.
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
  //! @brief Add a constant to every bin total (see add(double)).
  //! @param value constant to add
  //! @return reference to this histogram
  //!
  Histogram & operator+= (double value);

  //!
  //! @brief Subtract a constant from every bin total (see add(double)).
  //! @param value constant to subtract
  //! @return reference to this histogram
  //!
  Histogram & operator-= (double value);

  //!
  //! @brief Multiply every bin total and its uncertainty by a constant (see scale()).
  //! @param value multiplicative constant
  //! @return reference to this histogram
  //!
  Histogram & operator*= (double value);

  //!
  //! @brief Divide every bin total and its uncertainty by a constant.
  //! @param value divisor
  //! @return reference to this histogram
  //! @throws Exception if value==0
  //!
  Histogram & operator/= (double value);

  //!
  //! @brief Multiply every bin total by coefficient and its uncertainty by |coefficient|. Entry counts are unchanged.
  //! @param coefficient multiplicative coefficient
  //!
  void scale(double coefficient);

  //!
  //! @brief Add a constant to every bin total. Uncertainties are unchanged; empty bins become bins with one entry.
  //! @param value constant to add
  //!
  void add(double value);

  //!
  //! @brief Add coefficient*h bin by bin: entries summed, totals combined, uncertainties added in quadrature.
  //! @param h histogram to add (same binning)
  //! @param coefficient coefficient multiplying h
  //! @throws Exception if the binnings differ
  //!
  void add(const Histogram & h, double coefficient=1.0);

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
  //! @brief Replace every bin total v by 1/v, with uncertainty e/v^2.
  //!
  //! Result bins hold one entry; empty bins and bins with a zero total are set empty.
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
  void setAsSumOf(const Histogram & h1, const Histogram & h2,
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
  void setAsSumOf(const std::vector<const Histogram *> & histograms,
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
  //! Values and uncertainties are bin totals. Setting the entries keeps the bin total;
  //! setting the value or uncertainty keeps the entry count (one entry if the bin was empty).
  //! Indices are checked strictly: an out of range index throws IndexOutOfBoundException,
  //! and a number of indices different from the rank throws Exception.
  //! Setting entries<=0 for a bin with non-zero content throws Exception.
  //! @{

  //! @brief Set the entry count of a bin of a rank 1 histogram. @param binIndex bin index @param entries number of entries
  void setBinEntriesAt(unsigned int binIndex, long entries);
  //! @brief Set the total value of a bin of a rank 1 histogram. @param binIndex bin index @param value bin total
  void setBinValueAt(unsigned int binIndex, double value);
  //! @brief Set the uncertainty of the total of a bin of a rank 1 histogram. @param binIndex bin index @param uncertainty uncertainty of the bin total
  void setBinUncertaintyAt(unsigned int binIndex, double uncertainty);
  //! @brief Set the full content of a bin of a rank 1 histogram.
  //! @param binIndex bin index @param entries number of entries @param value bin total @param uncertainty uncertainty of the bin total
  void setBinAt(unsigned int binIndex, long entries, double value, double uncertainty);

  //! @brief Set the entry count of a bin of a rank 2 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param entries number of entries
  void setBinEntriesAt(unsigned int binIndex0, unsigned int binIndex1, long entries);
  //! @brief Set the total value of a bin of a rank 2 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param value bin total
  void setBinValueAt(unsigned int binIndex0, unsigned int binIndex1, double value);
  //! @brief Set the uncertainty of the total of a bin of a rank 2 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param uncertainty uncertainty of the bin total
  void setBinUncertaintyAt(unsigned int binIndex0, unsigned int binIndex1, double uncertainty);
  //! @brief Set the full content of a bin of a rank 2 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1
  //! @param entries number of entries @param value bin total @param uncertainty uncertainty of the bin total
  void setBinAt(unsigned int binIndex0, unsigned int binIndex1, long entries, double value, double uncertainty);

  //! @brief Set the entry count of a bin of a rank 3 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param binIndex2 bin index along axis 2 @param entries number of entries
  void setBinEntriesAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2, long entries);
  //! @brief Set the total value of a bin of a rank 3 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param binIndex2 bin index along axis 2 @param value bin total
  void setBinValueAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2, double value);
  //! @brief Set the uncertainty of the total of a bin of a rank 3 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param binIndex2 bin index along axis 2 @param uncertainty uncertainty of the bin total
  void setBinUncertaintyAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2, double uncertainty);
  //! @brief Set the full content of a bin of a rank 3 histogram.
  //! @param binIndex0 bin index along axis 0 @param binIndex1 bin index along axis 1 @param binIndex2 bin index along axis 2
  //! @param entries number of entries @param value bin total @param uncertainty uncertainty of the bin total
  void setBinAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2, long entries, double value, double uncertainty);

  //! @brief Set the entry count of a bin of a histogram of any rank.
  //! @param binIndices bin indices along each axis (size must equal rank()) @param entries number of entries
  void setBinEntriesAt(const std::vector<unsigned int> & binIndices, long entries);
  //! @brief Set the total value of a bin of a histogram of any rank.
  //! @param binIndices bin indices along each axis (size must equal rank()) @param value bin total
  void setBinValueAt(const std::vector<unsigned int> & binIndices, double value);
  //! @brief Set the uncertainty of the total of a bin of a histogram of any rank.
  //! @param binIndices bin indices along each axis (size must equal rank()) @param uncertainty uncertainty of the bin total
  void setBinUncertaintyAt(const std::vector<unsigned int> & binIndices, double uncertainty);
  //! @brief Set the full content of a bin of a histogram of any rank.
  //! @param binIndices bin indices along each axis (size must equal rank())
  //! @param entries number of entries @param value bin total @param uncertainty uncertainty of the bin total
  void setBinAt(const std::vector<unsigned int> & binIndices, long entries, double value, double uncertainty);

  //! @}

  //! @name Bin setters by coordinates
  //! The bin is the one containing the given coordinates. Values and uncertainties are bin totals.
  //! Coordinates outside the histogram range throw IndexOutOfBoundException; a rank mismatch throws Exception.
  //! @{

  //! @brief Set the entry count of the bin containing x (rank 1). @param x coordinate @param entries number of entries
  void setBinEntriesAt(double x, long entries);
  //! @brief Set the total value of the bin containing x (rank 1). @param x coordinate @param value bin total
  void setBinValueAt(double x, double value);
  //! @brief Set the uncertainty of the total of the bin containing x (rank 1). @param x coordinate @param uncertainty uncertainty of the bin total
  void setBinUncertaintyAt(double x, double uncertainty);
  //! @brief Set the full content of the bin containing x (rank 1).
  //! @param x coordinate @param entries number of entries @param value bin total @param uncertainty uncertainty of the bin total
  void setBinAt(double x, long entries, double value, double uncertainty);

  //! @brief Set the entry count of the bin containing (x0,x1) (rank 2).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param entries number of entries
  void setBinEntriesAt(double x0, double x1, long entries);
  //! @brief Set the total value of the bin containing (x0,x1) (rank 2).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param value bin total
  void setBinValueAt(double x0, double x1, double value);
  //! @brief Set the uncertainty of the total of the bin containing (x0,x1) (rank 2).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param uncertainty uncertainty of the bin total
  void setBinUncertaintyAt(double x0, double x1, double uncertainty);
  //! @brief Set the full content of the bin containing (x0,x1) (rank 2).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1
  //! @param entries number of entries @param value bin total @param uncertainty uncertainty of the bin total
  void setBinAt(double x0, double x1, long entries, double value, double uncertainty);

  //! @brief Set the entry count of the bin containing (x0,x1,x2) (rank 3).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param x2 coordinate along axis 2 @param entries number of entries
  void setBinEntriesAt(double x0, double x1, double x2, long entries);
  //! @brief Set the total value of the bin containing (x0,x1,x2) (rank 3).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param x2 coordinate along axis 2 @param value bin total
  void setBinValueAt(double x0, double x1, double x2, double value);
  //! @brief Set the uncertainty of the total of the bin containing (x0,x1,x2) (rank 3).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param x2 coordinate along axis 2 @param uncertainty uncertainty of the bin total
  void setBinUncertaintyAt(double x0, double x1, double x2, double uncertainty);
  //! @brief Set the full content of the bin containing (x0,x1,x2) (rank 3).
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param x2 coordinate along axis 2
  //! @param entries number of entries @param value bin total @param uncertainty uncertainty of the bin total
  void setBinAt(double x0, double x1, double x2, long entries, double value, double uncertainty);

  //! @brief Set the entry count of the bin containing point x (any rank).
  //! @param x coordinates along each axis (size must equal rank()) @param entries number of entries
  void setBinEntriesAt(const std::vector<double> & x, long entries);
  //! @brief Set the total value of the bin containing point x (any rank).
  //! @param x coordinates along each axis (size must equal rank()) @param value bin total
  void setBinValueAt(const std::vector<double> & x, double value);
  //! @brief Set the uncertainty of the total of the bin containing point x (any rank).
  //! @param x coordinates along each axis (size must equal rank()) @param uncertainty uncertainty of the bin total
  void setBinUncertaintyAt(const std::vector<double> & x, double uncertainty);
  //! @brief Set the full content of the bin containing point x (any rank).
  //! @param x coordinates along each axis (size must equal rank())
  //! @param entries number of entries @param value bin total @param uncertainty uncertainty of the bin total
  void setBinAt(const std::vector<double> & x, long entries, double value, double uncertainty);

  //! @}

  //! @name Setting all bins at once
  //! Vectors are indexed by global bin index and must have nBinsTotal() elements (Exception otherwise).
  //! Values and uncertainties are bin totals. When no entries are given, each bin gets one entry;
  //! when no uncertainties are given, they are set to zero.
  //! @{

  //! @brief Set all bin totals. @param values total value of each bin
  void setWith(const std::vector<float> & values);
  //! @brief Set all bin totals. @param values total value of each bin
  void setWith(const std::vector<double> & values);
  //! @brief Set all bin totals and their uncertainties.
  //! @param values total value of each bin @param uncertainties uncertainty of each bin total
  void setWith(const std::vector<float> & values, const std::vector<float> & uncertainties);
  //! @brief Set all bin totals and their uncertainties.
  //! @param values total value of each bin @param uncertainties uncertainty of each bin total
  void setWith(const std::vector<double> & values, const std::vector<double> & uncertainties);
  //! @brief Set all bin entries and totals.
  //! @param entries number of entries of each bin @param values total value of each bin
  void setWith(const std::vector<long> & entries,  const std::vector<float> & values);
  //! @brief Set all bin entries and totals.
  //! @param entries number of entries of each bin @param values total value of each bin
  void setWith(const std::vector<long> & entries,  const std::vector<double> & values);
  //! @brief Set all bin entries, totals and uncertainties.
  //! @param entries number of entries of each bin @param values total value of each bin @param uncertainties uncertainty of each bin total
  void setWith(const std::vector<long> & entries,  const std::vector<float> & values,  const std::vector<float> & uncertainties);
  //! @brief Set all bin entries, totals and uncertainties.
  //! @param entries number of entries of each bin @param values total value of each bin @param uncertainties uncertainty of each bin total
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
  //! Bins [first, last] of the other axis are summed (entries and totals summed, uncertainties in quadrature).
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
  //! With binSumOnly=false, the sum of bin totals is multiplied by the bin width, area or volume.
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

  //! @brief Sum of all bin totals, or integral of the histogram.
  //! @param binSumOnly if true, return the sum of bin totals; if false, multiply it by the bin volume @return sum or integral
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
  //! "Value" refers to the bin total, "AvgValue" to the average value per entry.
  //! @{

  //! @brief Find the bin with the largest number of entries.
  //! @param index [out] global index of that bin @param value [out] its number of entries
  //! @param first first global bin of the search @param last last global bin of the search
  void maximumEntries(unsigned int & index, long & value, unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Find the bin with the smallest number of entries.
  //! @param index [out] global index of that bin @param value [out] its number of entries
  //! @param first first global bin of the search @param last last global bin of the search
  void minimumEntries(unsigned int & index, long & value, unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Find the bin with the largest total value.
  //! @param index [out] global index of that bin @param value [out] its total value
  //! @param first first global bin of the search @param last last global bin of the search
  void maximumValue(unsigned int & index, double & value, unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Find the bin with the smallest total value.
  //! @param index [out] global index of that bin @param value [out] its total value
  //! @param first first global bin of the search @param last last global bin of the search
  void minimumValue(unsigned int & index, double & value, unsigned int first=0, unsigned int last=allBins) const;
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
  //! @brief Largest bin total. @param first first global bin @param last last global bin @return largest bin total
  double maximumValue(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Smallest bin total. @param first first global bin @param last last global bin @return smallest bin total
  double minimumValue(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Largest bin average. @param first first global bin @param last last global bin @return largest bin average
  double maximumAvgValue(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Smallest bin average. @param first first global bin @param last last global bin @return smallest bin average
  double minimumAvgValue(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Bin with the largest entry count. @param first first global bin @param last last global bin @return global bin index
  unsigned int  binWithMaxEntries(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Bin with the smallest entry count. @param first first global bin @param last last global bin @return global bin index
  unsigned int  binWithMinEntries(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Bin with the largest total. @param first first global bin @param last last global bin @return global bin index
  unsigned int  binWithMaxValue(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Bin with the smallest total. @param first first global bin @param last last global bin @return global bin index
  unsigned int  binWithMinValue(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Bin with the largest average. @param first first global bin @param last last global bin @return global bin index
  unsigned int  binWithMaxAvgValue(unsigned int first=0, unsigned int last=allBins) const;
  //! @brief Bin with the smallest average. @param first first global bin @param last last global bin @return global bin index
  unsigned int  binWithMinAvgValue(unsigned int first=0, unsigned int last=allBins) const;

  //! @}

  //! @name Clearing and filling
  //! Each fill adds one entry to the bin containing the given coordinates: the bin total increases
  //! by weight and the squared uncertainty of the total by weight^2. Coordinates outside the
  //! histogram range are ignored. A rank mismatch throws Exception.
  //! @{

  //! @brief Clear this histogram: all axes and bins are removed (rank 0, no bins). Name and title are kept.
  virtual void clear();
  //! @brief Reset all bins to zero (entries, values, uncertainties) without changing the binning.
  virtual void reset();
  //! @brief Fill a rank 1 histogram. @param x0 coordinate @param weight weight of the entry
  void fill(double x0, double weight=1.0);
  //! @brief Fill a rank 2 histogram. @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param weight weight of the entry
  void fill(double x0, double x1, double weight);
  //! @brief Fill a rank 3 histogram.
  //! @param x0 coordinate along axis 0 @param x1 coordinate along axis 1 @param x2 coordinate along axis 2 @param weight weight of the entry
  void fill(double x0, double x1, double x2, double weight);
  //! @brief Fill a histogram of any rank. @param x coordinates along each axis (size must equal rank()) @param weight weight of the entry
  void fill(const std::vector<double> & x, double weight=1.0);

  //! @}

  //! @name Bin getters
  //! Indices are checked strictly: an out of range index throws IndexOutOfBoundException,
  //! and a number of indices different from the rank throws Exception.
  //! "Value" is the bin total (entries*average); "AvgValue" is the average value per entry.
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

  //! @brief Total value of a bin (rank 1). @param binIndex bin index @return bin total
  double binValueAt(unsigned int binIndex) const;
  //! @brief Total value of a bin (rank 2). @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @return bin total
  double binValueAt(unsigned int binIndex0, unsigned int binIndex1) const;
  //! @brief Total value of a bin (rank 3).
  //! @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @param binIndex2 index along axis 2 @return bin total
  double binValueAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2) const;
  //! @brief Total value of a bin (any rank). @param indices bin indices along each axis @return bin total
  double binValueAt(const std::vector<unsigned int> & indices) const;

  //! @brief Uncertainty of the total of a bin (rank 1). @param binIndex bin index @return uncertainty of the bin total
  double binUncertaintyAt(unsigned int binIndex) const;
  //! @brief Uncertainty of the total of a bin (rank 2).
  //! @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @return uncertainty of the bin total
  double binUncertaintyAt(unsigned int binIndex0, unsigned int binIndex1) const;
  //! @brief Uncertainty of the total of a bin (rank 3).
  //! @param binIndex0 index along axis 0 @param binIndex1 index along axis 1 @param binIndex2 index along axis 2 @return uncertainty of the bin total
  double binUncertaintyAt(unsigned int binIndex0, unsigned int binIndex1, unsigned int binIndex2) const;
  //! @brief Uncertainty of the total of a bin (any rank). @param indices bin indices along each axis @return uncertainty of the bin total
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
  //! @brief Read-only access to the average values of all bins. @return vector of bin averages
  const std::vector<float> & binAvgValues() const { return _values; }
  //! @brief Read-only access to the uncertainties of the bin averages. @return vector of uncertainties
  const std::vector<float> & binAvgValueUncertainties() const { return _uncertainties; }
  //! @brief Writable access to the entries of all bins. @return vector of bin entries
  std::vector<long>  & binEntries() { return _entries; }
  //! @brief Writable access to the average values of all bins. @return vector of bin averages
  std::vector<float> & binAvgValues() { return _values; }
  //! @brief Writable access to the uncertainties of the bin averages. @return vector of uncertainties
  std::vector<float> & binAvgValueUncertainties() { return _uncertainties; }
  //! @brief Total value (entries*average) of every bin. @return vector of bin totals; empty bins are flagged with -1.0E100
  std::vector<double> binValues() const;
  //! @brief Uncertainty of the total value of every bin. @return vector of uncertainties; empty bins are flagged with -1.0E100
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
  //! @brief Total of a bin. @param index global bin index @return bin total
  //! @throws IndexOutOfBoundException if index>=size()
  double valueAt(unsigned int index) const override;
  //! @brief Uncertainty of the total of a bin. @param index global bin index @return uncertainty of the bin total
  //! @throws IndexOutOfBoundException if index>=size()
  double statErrorAt(unsigned int index) const override;
  //! @brief Same as statErrorAt(): a histogram has no systematic uncertainty. @param index global bin index @return uncertainty of the bin total
  //! @throws IndexOutOfBoundException if index>=size()
  double sysErrorAt(unsigned int index) const override;

  //!
  //! @brief Data point of a bin: x = bin center, y = bin total, statistical and systematic uncertainty = uncertainty of the bin total, no x uncertainty.
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
  //!    non-empty bin (varint), the entries (zigzag varint), the average value and its uncertainty (float32)
  //! Integers use variable-length encoding (1 byte below 128), multi-byte values are little-endian, and bins with
  //! zero entries, value and uncertainty are not written: a typical filled bin takes about 10 bytes, an empty bin 0.
  //! The content is restored exactly. Corrupted or truncated data is detected (FileException).
  //! Records of other classes in the same file are skipped. See BinaryIO.hpp for the record format and encodings.
  //! @{

  //! @brief Tag of histogram records in binary files. @return "CAPH"
  static const char * binaryTag() { return "CAPH"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 1; }
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
  //! Each bin line lists: global index, entries, total, uncertainty of the total, average, uncertainty of the average.
  //! @param out output stream
  //! @param h histogram to write
  //! @return the output stream
  //!
  friend std::ostream& operator<<(std::ostream& out, const Histogram & h);

  ClassDef(Histogram,1)

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
  //! @brief Add a constant to every bin total. @param left histogram @param right constant @return left+right
  Histogram operator+(const Histogram & left, double right);
  //! @relates Histogram
  //! @brief Add a constant to every bin total. @param left constant @param right histogram @return left+right
  Histogram operator+(double left, const Histogram & right);
  //! @relates Histogram
  //! @brief Subtract a constant from every bin total. @param left histogram @param right constant @return left-right
  Histogram operator-(const Histogram & left, double right);
  //! @relates Histogram
  //! @brief Subtract every bin total from a constant. @param left constant @param right histogram @return left-right
  Histogram operator-(double left, const Histogram & right);
  //! @relates Histogram
  //! @brief Scale every bin total by a constant. @param left histogram @param right constant @return left*right
  Histogram operator*(const Histogram & left, double right);
  //! @relates Histogram
  //! @brief Scale every bin total by a constant. @param left constant @param right histogram @return left*right
  Histogram operator*(double left, const Histogram & right);
  //! @relates Histogram
  //! @brief Divide every bin total by a non-zero constant (Exception if right==0). @param left histogram @param right constant @return left/right
  Histogram operator/(const Histogram & left, double right);
  //! @relates Histogram
  //! @brief Divide a constant by every bin total (see Histogram::inverse). @param left constant @param right histogram @return left/right
  Histogram operator/(double left, const Histogram & right);
  //! @relates Histogram
  //! @brief Unary plus: copy of the histogram. @param right histogram @return copy of right
  Histogram operator+(const Histogram & right);
  //! @relates Histogram
  //! @brief Unary minus: bin totals negated, uncertainties unchanged. @param right histogram @return -right
  Histogram operator-(const Histogram & right);

  } // namespace MATH
} // namespace CAP

#endif
