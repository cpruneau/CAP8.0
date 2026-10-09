#ifndef CAP_MATH_DataPointsVector
#define CAP_MATH_DataPointsVector
#include <vector>
#include "Aliases.hpp"
#include "DataPoint2D.hpp"
#include "DataPoints.hpp"
#include "BinaryIO.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Container of two-dimensional data points stored as a std::vector<DataPoint2D>.
  //!
  //! Concrete implementation of DataPoints. Each point keeps all its information,
  //! including the x-y covariance set from an error matrix (see DataPoint2D).
  //! Points are indexed from 0 to size()-1; index-checked access throws MathException.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::DataPointsVector data(std::vector<double>{1,2,3}, std::vector<double>{4,6,5});
  //!   data.addPoint(CAP::MATH::DataPoint2D(4.0f, 7.0f));
  //!   data[0].setErrors(0.1, 0.2, 0.5);        // modify a stored point in place
  //!   std::vector<double> y = data.yValues();  // {4,6,5,7}
  //! @endcode
  //!
  class DataPointsVector : public DataPoints
  {
  protected:

  std::vector<DataPoint2D> _dataPoints;  //!< Stored points

  //!
  //! @brief Throw if index is not a valid point index.
  //! @param index point index @param source name of the calling function, reported in the exception
  //! @throws MathException if index>=size()
  //!
  void checkIndex(unsigned int index, const char * source) const;

  public:

  //! @name Construction and assignment
  //! @{

  //! @brief Default constructor: empty container.
  DataPointsVector();

  //! @brief Construct from a vector of points (copied). @param dataPoints points to store
  DataPointsVector(const std::vector<DataPoint2D> & dataPoints);

  //!
  //! @brief Construct from two vectors x and y (no uncertainties).
  //! @param x x values @param y y values (same size as x)
  //! @throws MathException if x and y have different sizes
  //!
  DataPointsVector(const std::vector<double> & x, const std::vector<double> & y);

  //!
  //! @brief Construct from vectors of values and uncertainties; uncertainty vectors may be empty (zero uncertainties).
  //! @param x x values @param xError uncertainties on x @param y y values
  //! @param yStatError statistical uncertainties on y @param ySysError systematic uncertainties on y
  //! @throws MathException if the vector sizes differ
  //!
  DataPointsVector(const std::vector<double> & x,
                     const std::vector<double> & xError,
                     const std::vector<double> & y,
                     const std::vector<double> & yStatError,
                     const std::vector<double> & ySysError);

  //! @brief Copy constructor. @param source container to copy
  DataPointsVector(const DataPointsVector & source);

  //! @brief Destructor.
  virtual ~DataPointsVector() {}

  //! @brief Assignment operator. @param rhs container to copy @return reference to this container
  DataPointsVector & operator=(const DataPointsVector & rhs);

  //! @}

  //! @name DataPoints interface
  //! @{

  //! @brief Number of stored points. @return number of points
  unsigned int size() const override { return _dataPoints.size(); }

  //!
  //! @brief Copy of point number index.
  //! @param index point index (0 to size()-1) @return the point
  //! @throws MathException if index>=size()
  //!
  DataPoint2D point(unsigned int index) const override;

  //! @brief Append one point. @param dataPoint point to append
  void addPoint(const DataPoint2D & dataPoint) override;

  //! @brief Append one point given its values and uncertainties.
  //! @param x x value @param xError uncertainty on x @param y y value
  //! @param yStatError statistical uncertainty on y @param ySysError systematic uncertainty on y
  void addPoint(float x, float xError, float y, float yStatError, float ySysError) override;

  //! @brief Append all the given points. @param dataPoints points to append
  void addPoints(const std::vector<DataPoint2D> & dataPoints) override;

  using DataPoints::addPoints; // keep the x/y vector versions visible

  //! @}

  //! @name Getters
  //! @{

  //! @brief Read-only access to the stored points. @return vector of points
  const std::vector<DataPoint2D> & dataPoints() const { return _dataPoints; }

  //! @brief Writable access to the stored points. @return vector of points
  std::vector<DataPoint2D> & dataPoints() { return _dataPoints; }

  //!
  //! @brief Read-only access to point number index (no copy).
  //! @param index point index (0 to size()-1) @return reference to the point
  //! @throws MathException if index>=size()
  //!
  const DataPoint2D & operator[](unsigned int index) const;

  //!
  //! @brief Writable access to point number index.
  //! @param index point index (0 to size()-1) @return reference to the point
  //! @throws MathException if index>=size()
  //!
  DataPoint2D & operator[](unsigned int index);

  //! @brief x values of all points. @return vector of x values
  std::vector<double> xValues() const;
  //! @brief Uncertainties on x of all points. @return vector of x uncertainties
  std::vector<double> xErrors() const;
  //! @brief y values of all points. @return vector of y values
  std::vector<double> yValues() const;
  //! @brief Statistical uncertainties on y of all points. @return vector of statistical y uncertainties
  std::vector<double> yStatErrors() const;
  //! @brief Systematic uncertainties on y of all points. @return vector of systematic y uncertainties
  std::vector<double> ySysErrors() const;
  //! @brief Total uncertainties on y (statistical and systematic in quadrature) of all points. @return vector of total y uncertainties
  std::vector<double> yErrors() const;

  //! @}

  //! @name Setters
  //! @{

  //!
  //! @brief Replace point number index.
  //! @param index point index (0 to size()-1) @param dataPoint new point
  //! @throws MathException if index>=size()
  //!
  void setPoint(unsigned int index, const DataPoint2D & dataPoint);

  //! @brief Replace all stored points. @param dataPoints new points
  void setPoints(const std::vector<DataPoint2D> & dataPoints);

  //!
  //! @brief Replace all stored points by points built from two vectors x and y.
  //! @param x x values @param y y values (same size as x)
  //! @throws MathException if x and y have different sizes (the container is then left unchanged)
  //!
  void setPoints(const std::vector<double> & x, const std::vector<double> & y);

  //!
  //! @brief Replace all stored points by points built from vectors of values and uncertainties.
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
  //! @brief Remove point number index.
  //! @param index point index (0 to size()-1)
  //! @throws MathException if index>=size()
  //!
  void removePoint(unsigned int index);

  //! @brief Remove all points.
  void clear() { _dataPoints.clear(); }

  //! @brief Reserve memory for n points (performance only; size() is unchanged). @param n number of points
  void reserve(unsigned int n) { _dataPoints.reserve(n); }

  //! @brief Sort the points by increasing x (points with equal x keep their order).
  void sortByX();

  //! @}


  //! @name Arithmetic
  //! Point by point, with DataPoint2D arithmetic: the x values of corresponding points must be compatible
  //! (see DataPoint2D::compatibleX()), y values are added or subtracted and their uncertainties propagated.
  //! @{
  //! @brief Add rhs point by point. @param rhs data set with the same number of points @return this data set
  //! @throws MathException if the numbers of points differ or x values are incompatible (this data set is then unchanged)
  DataPointsVector & operator+=(const DataPointsVector & rhs);
  //! @brief Subtract rhs point by point. @param rhs data set with the same number of points @return this data set
  //! @throws MathException if the numbers of points differ or x values are incompatible (this data set is then unchanged)
  DataPointsVector & operator-=(const DataPointsVector & rhs);
  //! @}

  //! @name Binary files (independent of ROOT)
  //! Saved as "CAPD" records in the CAP binary format (see BinaryIO.hpp): compact, portable, checksummed, and restored
  //! exactly. Records of other classes in the same file are skipped.
  //! @{

  //! @brief Tag of DataPointsVector records in binary files. @return "CAPD"
  static const char * binaryTag() { return "CAPD"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 1; }
  //! @brief Name stored in binary records. @return ""
  String binaryName() const { return ""; }
  //! @brief Write the binary payload after the name (used by writeBinary()). @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Rebuild this object from a binary payload (used by readBinary()). @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid
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

  ClassDef(DataPointsVector,1)
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
