#ifndef CAP_Partition
#define CAP_Partition
#include <vector>
#include <map>
#include "Aliases.hpp"
#include "Object.hpp"
#include "MathExceptions.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Regular (fixed bin width) partition of a space of rank 1, 2, 3, ...
  //!
  //! A partition is built by adding dimensions (axes) one at a time with addDimension().
  //! Each axis k is divided into nBins(k) bins of equal width spanning [minimum(k), maximum(k)).
  //! The lower edge of an axis is included in its first bin; the upper edge is excluded.
  //!
  //! Bins are addressed either by per-axis indices (i0, i1, i2, ...) or by a single
  //! global (flat) bin index computed as
  //! @code
  //!   index = i0 + n0*(i1 + n1*(i2 + ...))
  //! @endcode
  //! where nk is the number of bins along axis k. The global index runs from 0 to nBinsTotal()-1.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Partition p;
  //!   p.addDimension(10, 0.0, 1.0, "x");
  //!   p.addDimension(20, -1.0, 1.0, "y");
  //!   int k = p.binIndex(0.25, 0.5);   // global index of the bin containing (0.25,0.5)
  //! @endcode
  //!
  //! Partition is the base class providing the binning of CAP::MATH::Histogram.
  //!
  class Partition
  {
  protected:

  unsigned int _rank;                 //!< Number of dimensions (axes)
  unsigned int _type;                 //!< Partition type (regular, profile, etc.); reserved for future use
  unsigned int _nBinsTotal;           //!< Total number of bins (product of the number of bins of all axes)
  std::vector<unsigned int> _nBins;   //!< Number of bins of each axis
  std::vector<float> _minima;         //!< Lower edge of each axis
  std::vector<float> _maxima;         //!< Upper edge of each axis
  std::vector<float> _widths;         //!< Bin width of each axis
  std::vector<String> _titles;        //!< Title of each axis
  std::vector<std::map<unsigned int,String>> _binLabels; //!< Optional labels of the bins of each axis (bin index -> label)

  public:

  //!
  //! @brief Default constructor: creates an empty partition of rank 0 with no bins.
  //!
  Partition();

  //!
  //! @brief Copy constructor.
  //! @param src partition to copy
  //!
  Partition(const Partition & src);

  //!
  //! @brief Destructor.
  //!
  virtual ~Partition();

  //!
  //! @brief Assignment operator: copies all axes of the given partition.
  //! @param rhs partition to copy
  //! @return reference to this partition
  //!
  Partition & operator=(const Partition & rhs);

  //!
  //! @brief Append all the axes of the given partition to this partition.
  //!
  //! The rank of this partition increases by partition.rank().
  //! @param partition partition whose axes are appended
  //! @throws Exception if partition is this partition
  //!
  void addDimension(const Partition & partition);

  //!
  //! @brief Append a new axis to this partition.
  //! @param nBin number of bins of the new axis (must be at least 1)
  //! @param minimum lower edge of the axis
  //! @param maximum upper edge of the axis (must be greater than minimum)
  //! @param title title of the axis
  //! @throws Exception if nBin<1 or minimum>=maximum
  //!
  void addDimension(unsigned int nBin, float minimum, float maximum, String title);

  //!
  //! @brief Replace this partition by the axes of partition1 followed by the axes of partition2.
  //!
  //! Either argument may be this partition.
  //! @param partition1 partition providing the first axes
  //! @param partition2 partition providing the following axes
  //!
  void createPartition(const Partition & partition1,const Partition & partition2);

  //!
  //! @brief Check whether the given partition has the same binning as this partition.
  //! @param rhs partition to compare with
  //! @param sizeAndLimits if true, the axis limits must also be identical; if false, only the rank and number of bins are compared
  //! @return true if the two partitions are compatible
  //!
  bool compatibleWith(const Partition & rhs, bool sizeAndLimits=1) const;

  //!
  //! @brief Number of dimensions (axes) of this partition: 1 for 1D, 2 for 2D, etc.
  //! @return the rank of this partition
  //!
  unsigned int rank() const
  {
  return _rank;
  }

  //!
  //! @brief Number of bins along the given axis.
  //! @param dimIndex axis index (0 to rank()-1)
  //! @return number of bins of axis dimIndex
  //! @throws IndexOutOfBoundException if dimIndex>=rank()
  //!
  unsigned int nBins(unsigned int dimIndex=0) const;

  //!
  //! @brief Total number of bins, i.e., the product of the number of bins of all axes.
  //! @return total number of bins (0 for an empty partition)
  //!
  unsigned int nBinsTotal() const;

  //!
  //! @brief Global bin index of a bin of a rank 1 partition.
  //! @param index bin index along axis 0
  //! @param isStrict if true, throw when the index is out of range; otherwise return -1
  //! @return global bin index, or -1 if the index is out of range and isStrict is false
  //! @throws Exception if the partition rank is not 1
  //! @throws IndexOutOfBoundException if the index is out of range and isStrict is true
  //!
  int binIndex(unsigned int index, bool isStrict=0) const;

  //!
  //! @brief Global bin index of a bin of a rank 2 partition.
  //! @param index0 bin index along axis 0
  //! @param index1 bin index along axis 1
  //! @param isStrict if true, throw when an index is out of range; otherwise return -1
  //! @return global bin index, or -1 if an index is out of range and isStrict is false
  //! @throws Exception if the partition rank is not 2
  //! @throws IndexOutOfBoundException if an index is out of range and isStrict is true
  //!
  int binIndex(unsigned int index0,unsigned int index1, bool isStrict=0) const;

  //!
  //! @brief Global bin index of a bin of a rank 3 partition.
  //! @param index0 bin index along axis 0
  //! @param index1 bin index along axis 1
  //! @param index2 bin index along axis 2
  //! @param isStrict if true, throw when an index is out of range; otherwise return -1
  //! @return global bin index, or -1 if an index is out of range and isStrict is false
  //! @throws Exception if the partition rank is not 3
  //! @throws IndexOutOfBoundException if an index is out of range and isStrict is true
  //!
  int binIndex(unsigned int index0,unsigned int index1,unsigned int index2, bool isStrict=0) const;

  //!
  //! @brief Global bin index of a bin of a partition of any rank.
  //! @param indices bin indices along each axis; the size must equal rank()
  //! @param isStrict if true, throw when an index is out of range; otherwise return -1
  //! @return global bin index, or -1 if an index is out of range and isStrict is false
  //! @throws Exception if indices.size()!=rank()
  //! @throws IndexOutOfBoundException if an index is out of range and isStrict is true
  //!
  int binIndex(const std::vector<unsigned int> & indices, bool isStrict=0) const;

  //!
  //! @brief Global index of the bin containing coordinate x in a rank 1 partition.
  //! @param x coordinate along axis 0
  //! @return global bin index, or -1 if x is outside [minimum, maximum)
  //! @throws Exception if the partition rank is not 1
  //!
  int binIndex(double x) const;

  //!
  //! @brief Global index of the bin containing point (x0,x1) in a rank 2 partition.
  //! @param x0 coordinate along axis 0
  //! @param x1 coordinate along axis 1
  //! @return global bin index, or -1 if the point is outside the partition range
  //! @throws Exception if the partition rank is not 2
  //!
  int binIndex(double x0, double x1) const;

  //!
  //! @brief Global index of the bin containing point (x0,x1,x2) in a rank 3 partition.
  //! @param x0 coordinate along axis 0
  //! @param x1 coordinate along axis 1
  //! @param x2 coordinate along axis 2
  //! @return global bin index, or -1 if the point is outside the partition range
  //! @throws Exception if the partition rank is not 3
  //!
  int binIndex(double x0, double x1, double x2) const;

  //!
  //! @brief Global index of the bin containing point x in a partition of any rank.
  //! @param x coordinates along each axis; the size must equal rank()
  //! @return global bin index, or -1 if the point is outside the partition range
  //! @throws Exception if x.size()!=rank()
  //!
  int binIndex(const std::vector<double> & x) const;

  //!
  //! @brief Index of the bin containing coordinate x along a single axis.
  //! @param x coordinate along axis dimIndex
  //! @param dimIndex axis index (0 to rank()-1)
  //! @return bin index along axis dimIndex, or -1 if x is outside [minimum(dimIndex), maximum(dimIndex))
  //! @throws IndexOutOfBoundException if dimIndex>=rank()
  //!
  int axisBinIndex(double x, unsigned int dimIndex) const;

  //!
  //! @brief Lower edge of the given axis.
  //! @param dimIndex axis index (0 to rank()-1)
  //! @return lower edge of axis dimIndex
  //! @throws IndexOutOfBoundException if dimIndex>=rank()
  //!
  double minimum(unsigned int dimIndex=0) const;

  //!
  //! @brief Upper edge of the given axis.
  //! @param dimIndex axis index (0 to rank()-1)
  //! @return upper edge of axis dimIndex
  //! @throws IndexOutOfBoundException if dimIndex>=rank()
  //!
  double maximum(unsigned int dimIndex=0) const;

  //!
  //! @brief Width of a bin along the given axis.
  //!
  //! All bins of an axis currently have the same width, so binIndex is not used.
  //! @param binIndex bin index along axis dimIndex
  //! @param dimIndex axis index (0 to rank()-1)
  //! @return bin width along axis dimIndex
  //! @throws IndexOutOfBoundException if dimIndex>=rank()
  //!
  double binWidth(unsigned int binIndex, unsigned int dimIndex=0) const;

  //!
  //! @brief Lower edge of a bin along the given axis.
  //! @param binIndex bin index along axis dimIndex
  //! @param dimIndex axis index (0 to rank()-1)
  //! @return lower edge of the bin
  //! @throws IndexOutOfBoundException if dimIndex>=rank()
  //!
  double binLowEdge(unsigned int binIndex, unsigned int dimIndex=0) const;

  //!
  //! @brief Upper edge of a bin along the given axis.
  //! @param binIndex bin index along axis dimIndex
  //! @param dimIndex axis index (0 to rank()-1)
  //! @return upper edge of the bin
  //! @throws IndexOutOfBoundException if dimIndex>=rank()
  //!
  double binHighEdge(unsigned int binIndex, unsigned int dimIndex=0) const;

  //!
  //! @brief Center of a bin along the given axis.
  //! @param binIndex bin index along axis dimIndex
  //! @param dimIndex axis index (0 to rank()-1)
  //! @return center of the bin
  //! @throws IndexOutOfBoundException if dimIndex>=rank()
  //!
  double binCenter(unsigned int binIndex, unsigned int dimIndex=0) const;

  //!
  //! @brief Create a rank 1 partition made of axis k of this partition.
  //! @param k index of the axis to extract
  //! @return new partition of rank 1
  //! @throws IndexOutOfBoundException if k>=rank()
  //!
  Partition extractPartition(unsigned int k=0)  const;

  //!
  //! @brief Create a rank 2 partition made of axes k0 and k1 of this partition, in that order.
  //! @param k0 index of the axis that becomes axis 0 of the new partition
  //! @param k1 index of the axis that becomes axis 1 of the new partition
  //! @return new partition of rank 2
  //! @throws IndexOutOfBoundException if k0 or k1 >= rank()
  //! @throws Exception if k0==k1
  //!
  Partition extractPartition(unsigned int k0, unsigned int k1) const;

  //!
  //! @brief Create a rank 3 partition made of axes k0, k1 and k2 of this partition, in that order.
  //! @param k0 index of the axis that becomes axis 0 of the new partition
  //! @param k1 index of the axis that becomes axis 1 of the new partition
  //! @param k2 index of the axis that becomes axis 2 of the new partition
  //! @return new partition of rank 3
  //! @throws IndexOutOfBoundException if k0, k1 or k2 >= rank()
  //! @throws Exception if two of the axis indices are equal
  //!
  Partition extractPartition(unsigned int k0, unsigned int k1, unsigned int k2) const;

  //!
  //! @brief Title of the given axis.
  //! @param k axis index (0 to rank()-1)
  //! @return title of axis k
  //! @throws IndexOutOfBoundException if k>=rank()
  //!
  const String & dimensionTitle(unsigned int k=0) const;
  //! @brief Set the title of axis k. @param k axis index @param title title @throws Exception if k>=rank()
  void setDimensionTitle(unsigned int k, const String & title);

  //!
  //! @brief Set the label of a bin of an axis (e.g., a particle name), used when the histogram is drawn.
  //! @param binIndex bin index along axis dimIndex @param label label (an empty label removes it)
  //! @param dimIndex axis index
  //! @throws IndexOutOfBoundException if dimIndex>=rank() or binIndex>=nBins(dimIndex)
  //!
  void setBinLabel(unsigned int binIndex, const String & label, unsigned int dimIndex=0);
  //! @brief Label of a bin of an axis. @param binIndex bin index along axis dimIndex @param dimIndex axis index
  //! @return label, or an empty string if the bin has none
  String binLabel(unsigned int binIndex, unsigned int dimIndex=0) const;
  //! @brief Labels of the bins of an axis. @param dimIndex axis index @return map bin index -> label
  const std::map<unsigned int,String> & binLabels(unsigned int dimIndex=0) const;

  //!
  //! @brief Remove all axes: the partition becomes empty (rank 0, no bins).
  //!
  void clear();

  protected:

  //!
  //! @brief Check that dimIndex is a valid axis index.
  //! @param dimIndex axis index to check
  //! @param source name of the calling function, reported in the exception
  //! @throws IndexOutOfBoundException if dimIndex>=rank()
  //!
  void checkDimIndex(unsigned int dimIndex, const char * source) const;

  ClassDef(Partition,2)

  };

  }  // namespace MATH
}  // namespace CAP

#endif
