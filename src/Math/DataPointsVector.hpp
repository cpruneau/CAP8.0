#ifndef CAP_MATH_DataPointsVector
#define CAP_MATH_DataPointsVector
#include <type_traits>
#include <vector>
#include "Aliases.hpp"
#include "DataPoint.hpp"
#include "DataPoint2D.hpp"
#include "DataPoint3D.hpp"
#include "DataPointND.hpp"
#include "DataPoints.hpp"
#include "BinaryIO.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Container of data points of any rank, stored polymorphically as DataPoint objects.
  //!
  //! Concrete implementation of DataPoints. The container owns its points: they are copied in (with
  //! DataPoint::clone()) when added, copied deeply when the container is copied, and deleted with it.
  //! Each point keeps all its information (e.g., the x-y covariance of a DataPoint2D).
  //!
  //! All points of a container have the same rank, set by the first point added (rank() is 1 while the
  //! container is empty). Adding or setting a point of another rank throws MathException.
  //! Points are indexed from 0 to size()-1; index-checked access throws MathException.
  //!
  //! Access to the points:
  //!  - operator[](index) returns the point as a DataPoint (any rank);
  //!  - pointAs<P>(index) returns it as its concrete class P (e.g., DataPoint2D), MathException if it is not a P;
  //!  - point(index), from the DataPoints interface, returns a copy as a DataPoint2D (rank 1 only);
  //!  - coordinatesAt(), coordinateErrorsAt(), valueAt(), statErrorAt(), sysErrorAt() work for any rank.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::DataPointsVector data(std::vector<double>{1,2,3}, std::vector<double>{4,6,5});
  //!   data.addPoint(CAP::MATH::DataPoint2D(4.0f, 7.0f));
  //!   data.pointAs<CAP::MATH::DataPoint2D>(0).setErrors(0.1, 0.2, 0.5);   // modify a stored point in place
  //!   std::vector<double> y = data.yValues();                              // {4,6,5,7}
  //!
  //!   CAP::MATH::DataPointsVector surface;                                 // rank 2 points
  //!   surface.addPoint(CAP::MATH::DataPoint3D(1.0, 2.0, 5.0, 0.4));
  //! @endcode
  //!
  class DataPointsVector : public DataPoints
  {
  protected:

  std::vector<DataPoint*> _dataPoints;  //!< Stored points (owned)

  //!
  //! @brief Throw if index is not a valid point index.
  //! @param index point index @param source name of the calling function, reported in the exception
  //! @throws MathException if index>=size()
  //!
  void checkIndex(unsigned int index, const char * source) const;

  //!
  //! @brief Throw if a point of the given rank cannot be added to this container.
  //! @param pointRank rank of the point @param source name of the calling function
  //! @throws MathException if the container is not empty and holds points of another rank
  //!
  void checkRank(unsigned int pointRank, const char * source) const;

  //! @brief Delete all points and empty the container.
  void deletePoints();

  public:

  //! @name Construction and assignment
  //! @{

  //! @brief Default constructor: empty container.
  DataPointsVector();

  //! @brief Construct from a vector of rank 1 points (copied). @param dataPoints points to store
  DataPointsVector(const std::vector<DataPoint2D> & dataPoints);

  //!
  //! @brief Construct from a vector of points of any concrete class derived from DataPoint (copied).
  //! @param dataPoints points to store (all of the same rank)
  //! @throws MathException if the points have different ranks
  //!
  template <class P, typename std::enable_if<std::is_base_of<DataPoint,P>::value && !std::is_same<P,DataPoint2D>::value,int>::type = 0>
  explicit DataPointsVector(const std::vector<P> & dataPoints)
  :
  DataPoints(),
  _dataPoints()
  {
  try { addPoints(dataPoints); }
  catch (...) { deletePoints(); throw; }
  }

  //!
  //! @brief Construct rank 1 points from two vectors x and y (no uncertainties).
  //! @param x x values @param y y values (same size as x)
  //! @throws MathException if x and y have different sizes
  //!
  DataPointsVector(const std::vector<double> & x, const std::vector<double> & y);

  //!
  //! @brief Construct rank 1 points from vectors of values and uncertainties; uncertainty vectors may be empty (zero uncertainties).
  //! @param x x values @param xError uncertainties on x @param y y values
  //! @param yStatError statistical uncertainties on y @param ySysError systematic uncertainties on y
  //! @throws MathException if the vector sizes differ
  //!
  DataPointsVector(const std::vector<double> & x,
                   const std::vector<double> & xError,
                   const std::vector<double> & y,
                   const std::vector<double> & yStatError,
                   const std::vector<double> & ySysError);

  //! @brief Copy constructor (deep copy). @param source container to copy
  DataPointsVector(const DataPointsVector & source);

  //! @brief Move constructor. @param source container whose points are taken over (left empty)
  DataPointsVector(DataPointsVector && source) noexcept;

  //! @brief Destructor: deletes the points.
  virtual ~DataPointsVector();

  //! @brief Assignment operator (deep copy). @param rhs container to copy @return reference to this container
  DataPointsVector & operator=(const DataPointsVector & rhs);

  //! @brief Move assignment. @param rhs container whose points are taken over (left empty) @return reference to this container
  DataPointsVector & operator=(DataPointsVector && rhs) noexcept;

  //! @}

  //! @name DataPoints interface
  //! @{

  //! @brief Number of stored points. @return number of points
  unsigned int size() const override { return _dataPoints.size(); }

  //! @brief Rank of the stored points. @return rank of the points (1 if the container is empty)
  unsigned int rank() const override;

  //!
  //! @brief Copy of point number index as a DataPoint2D.
  //! @param index point index (0 to size()-1) @return the point
  //! @throws MathException if index>=size() or the point is not a DataPoint2D (rank!=1; use operator[] or pointAs())
  //!
  DataPoint2D point(unsigned int index) const override;

  //! @brief Coordinates (independent variables) of a point. @param index point index @return rank() values
  std::vector<double> coordinatesAt(unsigned int index) const override;
  //! @brief Uncertainties of the coordinates of a point. @param index point index @return rank() uncertainties
  std::vector<double> coordinateErrorsAt(unsigned int index) const override;
  //! @brief Dependent value of a point. @param index point index @return value
  double valueAt(unsigned int index) const override;
  //! @brief Statistical uncertainty of the dependent value of a point. @param index point index @return stat
  double statErrorAt(unsigned int index) const override;
  //! @brief Systematic uncertainty of the dependent value of a point. @param index point index @return syst
  double sysErrorAt(unsigned int index) const override;

  //! @brief Append a copy of a rank 1 point. @param dataPoint point to append @throws MathException if the container rank is not 1
  void addPoint(const DataPoint2D & dataPoint) override;

  //! @brief Append a rank 1 point given its values and uncertainties.
  //! @param x x value @param xError uncertainty on x @param y y value
  //! @param yStatError statistical uncertainty on y @param ySysError systematic uncertainty on y
  //! @throws MathException if the container rank is not 1
  void addPoint(float x, float xError, float y, float yStatError, float ySysError) override;

  //! @brief Append copies of rank 1 points. @param dataPoints points to append @throws MathException if the container rank is not 1
  void addPoints(const std::vector<DataPoint2D> & dataPoints) override;

  using DataPoints::addPoints; // keep the x/y vector versions visible

  //! @brief Print the points to std::cout, one per line.
  void print() const override;

  //! @}

  //! @name Points of any rank
  //! @{

  //!
  //! @brief Append a copy of a point of any rank.
  //! @param dataPoint point to append (copied with clone())
  //! @throws MathException if the container is not empty and holds points of another rank
  //!
  void addPoint(const DataPoint & dataPoint);

  //!
  //! @brief Append copies of points of any concrete class derived from DataPoint.
  //! @param dataPoints points to append
  //! @throws MathException if a point has another rank than the container (the points before it are then added)
  //!
  template <class P, typename std::enable_if<std::is_base_of<DataPoint,P>::value && !std::is_same<P,DataPoint2D>::value,int>::type = 0>
  void addPoints(const std::vector<P> & dataPoints)
  {
  for (const auto & p : dataPoints) addPoint(static_cast<const DataPoint &>(p));
  }

  //! @brief Read-only access to the stored points (owned by this container). @return vector of pointers to the points
  const std::vector<DataPoint*> & dataPoints() const { return _dataPoints; }

  //!
  //! @brief Read-only access to point number index (no copy).
  //! @param index point index (0 to size()-1) @return reference to the point
  //! @throws MathException if index>=size()
  //!
  const DataPoint & operator[](unsigned int index) const;

  //!
  //! @brief Writable access to point number index. Its rank must not be changed.
  //! @param index point index (0 to size()-1) @return reference to the point
  //! @throws MathException if index>=size()
  //!
  DataPoint & operator[](unsigned int index);

  //!
  //! @brief Read-only access to point number index as its concrete class.
  //! @tparam P concrete class (DataPoint2D, DataPoint3D, DataPointND, ...)
  //! @param index point index (0 to size()-1) @return reference to the point
  //! @throws MathException if index>=size() or the point is not a P
  //!
  template <class P>
  const P & pointAs(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  const P * p = dynamic_cast<const P*>(_dataPoints[index]);
  if (!p) throw MathException("Point is not of the requested class",__FUNCTION__);
  return *p;
  }

  //!
  //! @brief Writable access to point number index as its concrete class.
  //! @tparam P concrete class (DataPoint2D, DataPoint3D, DataPointND, ...)
  //! @param index point index (0 to size()-1) @return reference to the point
  //! @throws MathException if index>=size() or the point is not a P
  //!
  template <class P>
  P & pointAs(unsigned int index)
  {
  return const_cast<P &>(static_cast<const DataPointsVector &>(*this).pointAs<P>(index));
  }

  //! @}

  //! @name Getters
  //! @{

  //! @brief Independent variable number variable of all points (x for rank 1). @param variable 0 to rank()-1 @return vector of values
  std::vector<double> xValues(unsigned int variable=0) const;
  //! @brief Uncertainties on independent variable number variable of all points. @param variable 0 to rank()-1 @return vector of uncertainties
  std::vector<double> xErrors(unsigned int variable=0) const;
  //! @brief Dependent values of all points (y for rank 1). @return vector of values
  std::vector<double> yValues() const;
  //! @brief Statistical uncertainties on the dependent values of all points. @return vector of statistical uncertainties
  std::vector<double> yStatErrors() const;
  //! @brief Systematic uncertainties on the dependent values of all points. @return vector of systematic uncertainties
  std::vector<double> ySysErrors() const;
  //! @brief Total uncertainties (statistical and systematic in quadrature) on the dependent values of all points. @return vector of total uncertainties
  std::vector<double> yErrors() const;

  //! @}

  //! @name Setters
  //! @{

  //!
  //! @brief Replace point number index by a copy of dataPoint.
  //! @param index point index (0 to size()-1) @param dataPoint new point
  //! @throws MathException if index>=size(), or dataPoint has another rank and the container holds other points
  //!
  void setPoint(unsigned int index, const DataPoint & dataPoint);

  //! @brief Replace all stored points by copies of rank 1 points. @param dataPoints new points
  void setPoints(const std::vector<DataPoint2D> & dataPoints);

  //!
  //! @brief Replace all stored points by rank 1 points built from two vectors x and y.
  //! @param x x values @param y y values (same size as x)
  //! @throws MathException if x and y have different sizes (the container is then left unchanged)
  //!
  void setPoints(const std::vector<double> & x, const std::vector<double> & y);

  //!
  //! @brief Replace all stored points by rank 1 points built from vectors of values and uncertainties.
  //! @param x x values @param xError uncertainties on x (or empty) @param y y values
  //! @param yStatError statistical uncertainties on y (or empty) @param ySysError systematic uncertainties on y (or empty)
  //! @throws MathException if the vector sizes differ (the container is then left unchanged)
  //!
  void setPoints(const std::vector<double> & x,
                 const std::vector<double> & xError,
                 const std::vector<double> & y,
                 const std::vector<double> & yStatError,
                 const std::vector<double> & ySysError);

  //!
  //! @brief Remove (and delete) point number index.
  //! @param index point index (0 to size()-1)
  //! @throws MathException if index>=size()
  //!
  void removePoint(unsigned int index);

  //! @brief Remove (and delete) all points.
  void clear() override { deletePoints(); }

  //! @brief Reserve memory for n points (performance only; size() is unchanged). @param n number of points
  void reserve(unsigned int n) { _dataPoints.reserve(n); }

  //!
  //! @brief Sort the points by increasing value of one independent variable (points with equal values keep their order).
  //! @param variable independent variable used, 0 to rank()-1 (0, i.e. x, by default)
  //!
  void sortByX(unsigned int variable=0);

  //! @}


  //! @name Arithmetic
  //! Point by point, with DataPoint arithmetic: corresponding points must have the same rank and compatible
  //! independent variables (see DataPoint::compatibleX()); dependent values are added or subtracted and their
  //! uncertainties propagated.
  //! @{
  //! @brief Add rhs point by point. @param rhs data set with the same number of points @return this data set
  //! @throws MathException if the numbers of points differ or points are incompatible (this data set is then unchanged)
  DataPointsVector & operator+=(const DataPointsVector & rhs);
  //! @brief Subtract rhs point by point. @param rhs data set with the same number of points @return this data set
  //! @throws MathException if the numbers of points differ or points are incompatible (this data set is then unchanged)
  DataPointsVector & operator-=(const DataPointsVector & rhs);
  //! @}

  //! @name Binary files (independent of ROOT)
  //! Saved as "CAPD" records in the CAP binary format (see BinaryIO.hpp): compact, portable, checksummed, and restored
  //! exactly. Records of other classes in the same file are skipped.
  //! Format version 2: number of points, rank (if any point), then each point in the binary encoding of its class
  //! (DataPoint2D for rank 1, DataPoint3D for rank 2, DataPointND for rank 3 and more). Version 1 (rank 1 only) is still read.
  //! @{

  //! @brief Tag of DataPointsVector records in binary files. @return "CAPD"
  static const char * binaryTag() { return "CAPD"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 2; }
  //! @brief Name stored in binary records. @return ""
  String binaryName() const { return ""; }
  //! @brief Write the binary payload after the name (used by writeBinary()). @param w writer
  //! @throws Exception if a point is not a DataPoint2D, DataPoint3D or DataPointND
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Rebuild this object from a binary payload (used by readBinary()). @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid (this object is then unchanged)
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write this set of data points as one binary record to a stream (open in binary mode). @param out stream @throws FileException if writing fails
  void writeBinary(std::ostream & out) const;
  //! @brief Read the next DataPointsVector record of a stream into this object, skipping records of other classes.
  //! @param in stream @return false at the end of the stream (unchanged) @throws FileException if the data is invalid (unchanged)
  bool readBinary(std::istream & in);
  //! @brief Save this set of data points to a binary file. @param fileName file name @param append add at the end of the file instead of replacing it
  //! @throws FileException if the file cannot be written
  void saveBinary(const String & fileName, bool append=false) const;
  //! @brief Load from a binary file the set of data points at position index among the objects of this class (0 = the first one).
  //! @param fileName file name @param index position
  //! @throws FileException if the file cannot be read, is corrupted, or holds fewer objects (this one is then unchanged)
  void loadBinary(const String & fileName, unsigned int index=0);
  //! @brief Save several objects to one binary file (replacing it). @param fileName file name @param objects objects to save, in order
  //! @throws FileException if the file cannot be written
  static void saveBinary(const String & fileName, const std::vector<DataPointsVector> & objects);
  //! @brief Load all the DataPointsVector objects of a binary file, in order. @param fileName file name @return objects
  //! @throws FileException if the file cannot be read or is corrupted
  static std::vector<DataPointsVector> loadAllBinary(const String & fileName);

  //! @}

  ClassDefOverride(DataPointsVector,2)
  };

  //! @relates DataPointsVector
  //! @brief Sum, point by point. @param left first data set @param right second data set @return left+right @throws MathException as operator+=
  DataPointsVector operator+(const DataPointsVector & left, const DataPointsVector & right);
  //! @relates DataPointsVector
  //! @brief Difference, point by point. @param left first data set @param right second data set @return left-right @throws MathException as operator-=
  DataPointsVector operator-(const DataPointsVector & left, const DataPointsVector & right);

  } // namespace MATH
} // namespace CAP

#endif
