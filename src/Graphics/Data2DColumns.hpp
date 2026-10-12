#ifndef CAP__Data2DColumns
#define CAP__Data2DColumns
#include "Data2D.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Data2DColumns : public Data2D
  {
  public:

  Data2DColumns();
  Data2DColumns(std::vector<float> & x,
                std::vector<float> & y);
  Data2DColumns(std::vector<float> & x,
                std::vector<float> & y,
                std::vector<float> & yStatError);
  Data2DColumns(std::vector<float> & x,
                std::vector<float> & y,
                std::vector<float> & yStatError,
                std::vector<float> & ySystError);
  Data2DColumns(std::vector<float> & x,
                std::vector<float> & xError,
                std::vector<float> & y,
                std::vector<float> & yStatError,
                std::vector<float> & ySystError);

  Data2DColumns(const Data2DColumns & src);
  Data2DColumns  & operator=(const Data2DColumns & rhs);
  virtual ~Data2DColumns() { }

  unsigned int size() const { return _x.size(); }
  DataPoint2D point(unsigned int index) const
    { return DataPoint2D(_x[index], _xError[index], _y[index], _yStatError[index], _ySysError[index]); }

  float xMinimum() const;
  float xMaximum() const;
  float xAtYMaximum() const;
  float xAtYMinimum() const;

  float yMinimum() const;
  float yMaximum() const;
  float yAtXMaximum() const;
  float yAtXMinimum() const;

  //! @brief Replace all the columns with the values of the given points (covariances are not stored by columns). @param points points
  void setColumns(const std::vector<DataPoint2D> & points);

  void addPoint(float x, float xError, float y, float yStatError, float ySysError);
  void addPoint(const DataPoint2D & dataPoint);
  using Data2D::addPoints; // the Data2D versions loop over addPoint()


  //! @name Binary files (independent of ROOT)
  //! Saved as "CAPC" records in the CAP binary format (see BinaryIO.hpp): the Data2D description (type, names,
  //! titles, labels) and the points; compact, portable, checksummed, and restored exactly. Records of other classes
  //! in the same file are skipped.
  //! @{

  //! @brief Tag of Data2DColumns records in binary files. @return "CAPC"
  static const char * binaryTag() { return "CAPC"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 1; }
  //! @brief Name stored in binary records. @return name()
  String binaryName() const { return name(); }
  //! @brief Write the binary payload after the name (used by writeBinary()). @param w writer
  void encodeBinary(CAP::BinaryWriter & w) const;
  //! @brief Rebuild this object from a binary payload (used by readBinary()). @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid
  void decodeBinary(CAP::BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write this set of columns as one binary record to a stream (open in binary mode). @param out stream @throws FileException if writing fails
  void writeBinary(std::ostream & out) const;
  //! @brief Read the next Data2DColumns record of a stream into this object, skipping records of other classes.
  //! @param in stream @return false at the end of the stream (unchanged) @throws FileException if the data is invalid (unchanged)
  bool readBinary(std::istream & in);
  //! @brief Save this set of columns to a binary file. @param fileName file name @param append add at the end of the file instead of replacing it
  //! @throws FileException if the file cannot be written
  void saveBinary(const String & fileName, bool append=false) const;
  //! @brief Load from a binary file the first set of columns, or the first one with the given name.
  //! @param fileName file name @param name name to look for (empty: the first one)
  //! @throws FileException if the file cannot be read, is corrupted, or holds no such object (this one is then unchanged)
  void loadBinary(const String & fileName, const String & name="");
  //! @brief Save several objects to one binary file (replacing it). @param fileName file name @param objects objects to save, in order
  //! @throws FileException if the file cannot be written
  static void saveBinary(const String & fileName, const std::vector<Data2DColumns> & objects);
  //! @brief Load all the Data2DColumns objects of a binary file, in order. @param fileName file name @return objects
  //! @throws FileException if the file cannot be read or is corrupted
  static std::vector<Data2DColumns> loadAllBinary(const String & fileName);

  //! @}

  protected:

  std::vector<float> _x;
  std::vector<float> _xError;
  std::vector<float> _y;
  std::vector<float> _yStatError;
  std::vector<float> _ySysError;

  ClassDef(Data2DColumns,0)
  };

  //! @relates Data2DColumns
  //! @brief Sum, point by point (DataPoint2D arithmetic: compatible x values required); the description (names, titles,
  //! labels) is that of left. @param left first @param right second @return left+right
  //! @throws MathException if the numbers of points differ or x values are incompatible
  Data2DColumns operator+(const Data2DColumns & left, const Data2DColumns & right);
  //! @relates Data2DColumns
  //! @brief Difference, point by point; see operator+. @param left first @param right second @return left-right
  Data2DColumns operator-(const Data2DColumns & left, const Data2DColumns & right);

  } //  namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Data2DColumns
