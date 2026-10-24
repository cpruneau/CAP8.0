/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP_MATH_DataPointColumns
#define CAP_MATH_DataPointColumns
#include <memory>
#include <vector>
#include "Aliases.hpp"
#include "BinaryIO.hpp"
#include "DataPoint.hpp"
#include "DataPoint2D.hpp"
#include "DataPoints.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Container of data points of any rank stored as columns.
  //!
  //! Concrete implementation of DataPoints. Point k is made of element k of each column:
  //!  - xColumn(i)      : independent variable i, for i = 0 to rank()-1
  //!  - xErrorColumn(i) : uncertainty on independent variable i (independent variables are treated as uncorrelated)
  //!  - valueColumn()   : dependent value
  //!  - statColumn()    : statistical uncertainty on the dependent value
  //!  - systColumn()    : systematic uncertainty on the dependent value
  //! All columns always have size() elements. Values are stored as float, like the DataPoint classes.
  //!
  //! The rank is set at construction (1 by default) and can be changed while the container is empty, with
  //! setRank(), setColumns(), or by adding a first point of another rank. Points added later must have that rank.
  //! Uncertainties must be non-negative (MathException otherwise).
  //!
  //! Points are read as copies: point(index) returns a DataPoint2D (rank 1), createPoint(index) a point of
  //! any rank (DataPoint2D, DataPoint3D or DataPointND). DataPoints functions coordinatesAt(), valueAt(), ...,
  //! minimum(), maximum(), minimumValue() and maximumValue() work for any rank.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::DataPointColumns data(2);                                 // rank 2: x and y
  //!   data.setColumns({{1,2,3},{4,5,6}}, {}, {10,20,30}, {1,2,3}, {});     // x and y columns, values, stat errors
  //!   data.addPoint(CAP::MATH::DataPoint3D(4.0, 7.0, 40.0, 4.0));
  //!   const std::vector<float> & y = data.xColumn(1);                      // {4,5,6,7}
  //!   double zMax = data.maximumValue();                                   // 40
  //! @endcode
  //!
  class DataPointColumns : public DataPoints
  {
  protected:

  unsigned int _rank;                          //!< number of independent variables
  std::vector< std::vector<float> > _x;        //!< independent variables: _rank columns
  std::vector< std::vector<float> > _xError;   //!< uncertainties on the independent variables: _rank columns
  std::vector<float> _value;                   //!< dependent values
  std::vector<float> _stat;                    //!< statistical uncertainties on the dependent values
  std::vector<float> _syst;                    //!< systematic uncertainties on the dependent values

  //! @brief Throw if index is not a valid point index. @param index point index @param source caller name
  //! @throws MathException if index>=size()
  void checkIndex(unsigned int index, const char * source) const;
  //! @brief Throw if variable is not a valid independent variable. @param variable variable index @param source caller name
  //! @throws MathException if variable>=rank()
  void checkVariable(unsigned int variable, const char * source) const;
  //! @brief Overwrite point index with the values of dataPoint (same rank, already checked). @param index point index @param dataPoint point
  void storePoint(unsigned int index, const DataPoint & dataPoint);

  public:

  //! @name Construction
  //! @{

  //! @brief Empty container of the given rank. @param rank number of independent variables (>= 1)
  //! @throws MathException if rank==0
  explicit DataPointColumns(unsigned int rank=1);

  //!
  //! @brief Rank 1 columns from two vectors x and y (no uncertainties).
  //! @param x x values @param y y values (same size as x)
  //! @throws MathException if the sizes differ
  //!
  DataPointColumns(const std::vector<double> & x, const std::vector<double> & y);

  //!
  //! @brief Rank 1 columns from vectors of values and uncertainties; uncertainty vectors may be empty (zero uncertainties).
  //! @param x x values @param xError uncertainties on x @param y y values
  //! @param yStatError statistical uncertainties on y @param ySysError systematic uncertainties on y
  //! @throws MathException if the sizes differ or an uncertainty is negative
  //!
  DataPointColumns(const std::vector<double> & x,
                   const std::vector<double> & xError,
                   const std::vector<double> & y,
                   const std::vector<double> & yStatError,
                   const std::vector<double> & ySysError);

  //!
  //! @brief Columns of any rank; see setColumns(). The rank is x.size().
  //! @param x one column per independent variable @param xError one column per independent variable, or empty
  //! @param value dependent values @param stat statistical uncertainties (or empty) @param syst systematic uncertainties (or empty)
  //! @throws MathException if x is empty, the sizes differ, or an uncertainty is negative
  //!
  DataPointColumns(const std::vector< std::vector<double> > & x,
                   const std::vector< std::vector<double> > & xError,
                   const std::vector<double> & value,
                   const std::vector<double> & stat,
                   const std::vector<double> & syst);

  //! @brief Destructor.
  virtual ~DataPointColumns() {}

  //! @}

  //! @name DataPoints interface
  //! @{

  //! @brief Number of points. @return number of points
  unsigned int size() const override { return _value.size(); }
  //! @brief Number of independent variables. @return rank
  unsigned int rank() const override { return _rank; }

  //!
  //! @brief Copy of point index as a DataPoint2D.
  //! @param index point index (0 to size()-1) @return the point
  //! @throws MathException if index>=size() or rank()!=1 (use createPoint())
  //!
  DataPoint2D point(unsigned int index) const override;

  //! @brief Independent variables of a point. @param index point index @return rank() values
  std::vector<double> coordinatesAt(unsigned int index) const override;
  //! @brief Uncertainties on the independent variables of a point. @param index point index @return rank() uncertainties
  std::vector<double> coordinateErrorsAt(unsigned int index) const override;
  //! @brief Dependent value of a point. @param index point index @return value
  double valueAt(unsigned int index) const override;
  //! @brief Statistical uncertainty of a point. @param index point index @return stat
  double statErrorAt(unsigned int index) const override;
  //! @brief Systematic uncertainty of a point. @param index point index @return syst
  double sysErrorAt(unsigned int index) const override;

  //! @brief Append a rank 1 point. @param dataPoint point @throws MathException if the rank is not 1 (and the container not empty)
  void addPoint(const DataPoint2D & dataPoint) override;
  //! @brief Append a rank 1 point given its values and uncertainties.
  //! @param x x value @param xError uncertainty on x @param y y value
  //! @param yStatError statistical uncertainty on y @param ySysError systematic uncertainty on y
  //! @throws MathException if the rank is not 1 (and the container not empty), or an uncertainty is negative
  void addPoint(float x, float xError, float y, float yStatError, float ySysError) override;

  using DataPoints::addPoints; // the DataPoints versions loop over addPoint()

  //! @brief Print the points to std::cout, one per line.
  void print() const override;

  //! @}

  //! @name Points of any rank
  //! @{

  //!
  //! @brief Append a point of any rank.
  //! @param dataPoint point (its values are copied into the columns; a DataPoint2D covariance is not kept)
  //! @throws MathException if the container is not empty and the point has another rank
  //!
  void addPoint(const DataPoint & dataPoint);

  //!
  //! @brief Append a point given its values.
  //! @param x independent variables (rank() values) @param xError their uncertainties (rank() values, or empty)
  //! @param value dependent value @param stat statistical uncertainty @param syst systematic uncertainty
  //! @throws MathException if the sizes do not match the rank or an uncertainty is negative
  //!
  void addPoint(const std::vector<double> & x, const std::vector<double> & xError,
                double value, double stat=0.0, double syst=0.0);

  //!
  //! @brief Copy of point index as a point of the matching class: DataPoint2D (rank 1), DataPoint3D (rank 2) or DataPointND.
  //! @param index point index (0 to size()-1) @return the point (owned by the caller)
  //! @throws MathException if index>=size()
  //!
  std::unique_ptr<DataPoint> createPoint(unsigned int index) const;

  //! @}

  //! @name Columns
  //! @{

  //! @brief Independent variable column. @param variable 0 to rank()-1 @return column @throws MathException if variable>=rank()
  const std::vector<float> & xColumn(unsigned int variable) const;
  //! @brief Uncertainty column of an independent variable. @param variable 0 to rank()-1 @return column @throws MathException if variable>=rank()
  const std::vector<float> & xErrorColumn(unsigned int variable) const;
  //! @brief Dependent value column. @return column
  const std::vector<float> & valueColumn() const { return _value; }
  //! @brief Statistical uncertainty column. @return column
  const std::vector<float> & statColumn() const { return _stat; }
  //! @brief Systematic uncertainty column. @return column
  const std::vector<float> & systColumn() const { return _syst; }

  //!
  //! @brief Replace all columns; the rank becomes x.size().
  //! @param x one column per independent variable (at least one, all of the same size n)
  //! @param xError one column of size n per independent variable, or empty (zero uncertainties)
  //! @param value dependent values (size n)
  //! @param stat statistical uncertainties (size n, or empty) @param syst systematic uncertainties (size n, or empty)
  //! @throws MathException if x is empty, the sizes differ, or an uncertainty is negative (the container is then unchanged)
  //!
  void setColumns(const std::vector< std::vector<double> > & x,
                  const std::vector< std::vector<double> > & xError,
                  const std::vector<double> & value,
                  const std::vector<double> & stat,
                  const std::vector<double> & syst);

  //! @}

  //! @name Setters
  //! @{

  //! @brief Set the rank of an empty container. @param rank number of independent variables (>= 1)
  //! @throws MathException if the container is not empty or rank==0
  void setRank(unsigned int rank);

  //!
  //! @brief Replace point index.
  //! @param index point index (0 to size()-1) @param dataPoint new point (same rank)
  //! @throws MathException if index>=size() or the ranks differ
  //!
  void setPoint(unsigned int index, const DataPoint & dataPoint);

  //! @brief Remove point index. @param index point index (0 to size()-1) @throws MathException if index>=size()
  void removePoint(unsigned int index);
  //! @brief Remove all points (the rank is kept).
  void clear() override;
  //! @brief Reserve memory for n points (performance only). @param n number of points
  void reserve(unsigned int n);

  //!
  //! @brief Sort the points by increasing value of one independent variable (points with equal values keep their order).
  //! @param variable 0 to rank()-1 (0, i.e. x, by default) @throws MathException if variable>=rank()
  //!
  void sortByX(unsigned int variable=0);

  //! @}

  //! @name Arithmetic
  //! Point by point, with DataPoint arithmetic: corresponding points must have compatible independent variables
  //! (see DataPoint::compatibleX()); dependent values are added or subtracted and their uncertainties propagated.
  //! @{
  //! @brief Add rhs point by point. @param rhs data set of the same rank and size @return this data set
  //! @throws MathException if the ranks or sizes differ or points are incompatible (this data set is then unchanged)
  DataPointColumns & operator+=(const DataPointColumns & rhs);
  //! @brief Subtract rhs point by point. @param rhs data set of the same rank and size @return this data set
  //! @throws MathException if the ranks or sizes differ or points are incompatible (this data set is then unchanged)
  DataPointColumns & operator-=(const DataPointColumns & rhs);
  //! @}

  //! @name Binary files (independent of ROOT)
  //! Saved as "CAPK" records in the CAP binary format (see BinaryIO.hpp): rank, size, then each column (float32):
  //! x[0], xError[0], ..., x[rank-1], xError[rank-1], value, stat, syst; then the fits attached (format version 2,
  //! see FitList::encodeBinary()). Restored exactly.
  //! @{

  //! @brief Tag of DataPointColumns records in binary files. @return "CAPK"
  static const char * binaryTag() { return "CAPK"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 2; }
  //! @brief Name stored in binary records. @return ""
  String binaryName() const { return ""; }
  //! @brief Write the binary payload after the name. @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Rebuild this object from a binary payload. @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid (this object is then unchanged)
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write as one binary record to a stream. @param out stream @throws FileException if writing fails
  void writeBinary(std::ostream & out) const;
  //! @brief Read the next DataPointColumns record of a stream. @param in stream @return false at the end of the stream
  bool readBinary(std::istream & in);
  //! @brief Save to a binary file. @param fileName file name @param append add at the end of the file instead of replacing it
  void saveBinary(const String & fileName, bool append=false) const;
  //! @brief Load the object at position index among the DataPointColumns objects of a binary file. @param fileName file name @param index position
  void loadBinary(const String & fileName, unsigned int index=0);
  //! @brief Save several objects to one binary file (replacing it). @param fileName file name @param objects objects to save
  static void saveBinary(const String & fileName, const std::vector<DataPointColumns> & objects);
  //! @brief Load all the DataPointColumns objects of a binary file, in order. @param fileName file name @return objects
  static std::vector<DataPointColumns> loadAllBinary(const String & fileName);

  //! @}

  ClassDefOverride(DataPointColumns,1)
  };

  //! @relates DataPointColumns
  //! @brief Sum, point by point. @param left first data set @param right second data set @return left+right @throws MathException as operator+=
  DataPointColumns operator+(const DataPointColumns & left, const DataPointColumns & right);
  //! @relates DataPointColumns
  //! @brief Difference, point by point. @param left first data set @param right second data set @return left-right @throws MathException as operator-=
  DataPointColumns operator-(const DataPointColumns & left, const DataPointColumns & right);

  } // namespace MATH
} // namespace CAP

#endif
