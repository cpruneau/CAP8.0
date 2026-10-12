#ifndef CAP__Data2DPoints
#define CAP__Data2DPoints
#include "Aliases.hpp"
#include "Data2D.hpp"
#include "DataPointsVector.hpp"   // CAP::MATH::DataPointsVector (Math)

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! @brief Graphics data container that stores its points as DataPoint2D objects.
  //!
  //! Adds the Graphics attributes of Data2D (names, titles, labels) to a
  //! CAP::MATH::DataPointsVector, which holds the points. All point storage and
  //! access is delegated to that object (composition rather than a second base class,
  //! which would duplicate the CAP::MATH::DataPoints base inherited through Data2D).
  //!
  //! For an empty container the extrema return the values used historically by Graphics
  //! (1.0E30 or -1.0E30) instead of throwing.
  //!
  class Data2DPoints : public Data2D
  {
  public:

  //! @brief Default constructor: empty container of type kRow.
  Data2DPoints();
  //! @brief Copy constructor. @param src container to copy
  Data2DPoints(const Data2DPoints & src);
  //! @brief Assignment operator. @param rhs container to copy @return reference to this container
  Data2DPoints  & operator=(const Data2DPoints & rhs);
  //! @brief Destructor.
  virtual ~Data2DPoints() { }

  //! @brief Append one point given its values and uncertainties.
  //! @param x x value @param xError uncertainty on x @param y y value
  //! @param yStatError statistical uncertainty on y @param ySysError systematic uncertainty on y
  void addPoint(float x, float xError, float y, float yStatError, float ySysError) override;
  //! @brief Append one point. @param dataPoint point to append
  void addPoint(const DataPoint2D & dataPoint) override;
  //! @brief Append all the given points. @param dataPoints points to append
  void addPoints(const std::vector<DataPoint2D> & dataPoints) override;
  using Data2D::addPoints; // keep the float-vector and double-vector versions visible

  //! @brief Number of points. @return number of points
  unsigned int size() const override { return _points.size(); }
  //! @brief Copy of point number index. @param index point index (0 to size()-1) @return the point
  //! @throws MathException if index>=size()
  DataPoint2D point(unsigned int index) const override { return _points.point(index); }

  //! @brief Smallest x value (1.0E30 if empty). @return minimum of x
  float xMinimum()    const override;
  //! @brief Largest x value (-1.0E30 if empty). @return maximum of x
  float xMaximum()    const override;
  //! @brief x value of the point with the largest y (1.0E30 if empty). @return x at maximum y
  float xAtYMaximum() const override;
  //! @brief x value of the point with the smallest y (1.0E30 if empty). @return x at minimum y
  float xAtYMinimum() const override;
  //! @brief Smallest y value (1.0E30 if empty). @return minimum of y
  float yMinimum()    const override;
  //! @brief Largest y value (-1.0E30 if empty). @return maximum of y
  float yMaximum()    const override;
  //! @brief y value of the point with the largest x (1.0E30 if empty). @return y at maximum x
  float yAtXMaximum() const override;
  //! @brief y value of the point with the smallest x (1.0E30 if empty). @return y at minimum x
  float yAtXMinimum() const override;

  //! @brief Copies of the stored points. To modify a point in place use points().pointAs<DataPoint2D>(index).
  //! @return vector of points
  std::vector<DataPoint2D> dataPoints() const;

  //! @brief Writable access to the underlying Math container (setters, sorting, column getters, ...). @return the point container
  CAP::MATH::DataPointsVector & points() { return _points; }
  //! @brief Read-only access to the underlying Math container. @return the point container
  const CAP::MATH::DataPointsVector & points() const { return _points; }


  //! @name Binary files (independent of ROOT)
  //! Saved as "CAPG" records in the CAP binary format (see BinaryIO.hpp): the Data2D description (type, names,
  //! titles, labels) and the points; compact, portable, checksummed, and restored exactly. Records of other classes
  //! in the same file are skipped.
  //! @{

  //! @brief Tag of Data2DPoints records in binary files. @return "CAPG"
  static const char * binaryTag() { return "CAPG"; }
  //! @brief Format version written (and highest version read); it is also the version of the embedded
  //! DataPointsVector payload (2 since DataPointsVector holds DataPoint objects). @return version
  static unsigned int binaryVersion() { return 2; }
  //! @brief Name stored in binary records. @return name()
  String binaryName() const { return name(); }
  //! @brief Write the binary payload after the name (used by writeBinary()). @param w writer
  void encodeBinary(CAP::BinaryWriter & w) const;
  //! @brief Rebuild this object from a binary payload (used by readBinary()). @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid
  void decodeBinary(CAP::BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write this set of points as one binary record to a stream (open in binary mode). @param out stream @throws FileException if writing fails
  void writeBinary(std::ostream & out) const;
  //! @brief Read the next Data2DPoints record of a stream into this object, skipping records of other classes.
  //! @param in stream @return false at the end of the stream (unchanged) @throws FileException if the data is invalid (unchanged)
  bool readBinary(std::istream & in);
  //! @brief Save this set of points to a binary file. @param fileName file name @param append add at the end of the file instead of replacing it
  //! @throws FileException if the file cannot be written
  void saveBinary(const String & fileName, bool append=false) const;
  //! @brief Load from a binary file the first set of points, or the first one with the given name.
  //! @param fileName file name @param name name to look for (empty: the first one)
  //! @throws FileException if the file cannot be read, is corrupted, or holds no such object (this one is then unchanged)
  void loadBinary(const String & fileName, const String & name="");
  //! @brief Save several objects to one binary file (replacing it). @param fileName file name @param objects objects to save, in order
  //! @throws FileException if the file cannot be written
  static void saveBinary(const String & fileName, const std::vector<Data2DPoints> & objects);
  //! @brief Load all the Data2DPoints objects of a binary file, in order. @param fileName file name @return objects
  //! @throws FileException if the file cannot be read or is corrupted
  static std::vector<Data2DPoints> loadAllBinary(const String & fileName);

  //! @}

  protected:

  CAP::MATH::DataPointsVector _points;  //!< Storage of the points

  ClassDef(Data2DPoints,0)
  };

  //! @relates Data2DPoints
  //! @brief Sum, point by point (DataPoint2D arithmetic: compatible x values required); the description (names, titles,
  //! labels) is that of left. @param left first @param right second @return left+right
  //! @throws MathException if the numbers of points differ or x values are incompatible
  Data2DPoints operator+(const Data2DPoints & left, const Data2DPoints & right);
  //! @relates Data2DPoints
  //! @brief Difference, point by point; see operator+. @param left first @param right second @return left-right
  Data2DPoints operator-(const Data2DPoints & left, const Data2DPoints & right);

  } //  namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Data2DPoints
