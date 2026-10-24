/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP_MATH_Table2D
#define CAP_MATH_Table2D
#include <vector>
#include "Aliases.hpp"
#include "BinaryIO.hpp"
#include "Object.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Table of numbers with nRows() rows and nColumns() columns, read from and written to text files.
  //!
  //! Values are accessed with at(row,column) (checked) or operator()(row,column) (unchecked), by row or by column.
  //! Every row has the same number of columns.
  //!
  //! <b>Grid and interpolation.</b> The table may tabulate a function f(x,y) on a regular grid: with setGrid(xMin,dx,
  //! yMin,dy), row i holds x_i = xMin + i*dx and column j holds y_j = yMin + j*dy, so at(i,j) = f(x_i,y_j).
  //! interpolate(x,y) then returns the bilinear interpolation of f; outside the grid it extrapolates linearly from the
  //! nearest cell (or throws, if extrapolation is not allowed).
  //!
  //! <b>Text files.</b> One row per line, values separated by blanks (or commas or tabs). Blank lines and lines starting
  //! with '#' are skipped. All rows must have the same number of values. The grid is not part of the text format.
  //!
  //! <b>Binary files.</b> "CAPB" records of the CAP binary format (see BinaryIO.hpp): rows, columns, grid and values
  //! (float64), restored exactly. Tables can be stored in a Folder / CapFile.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Table2D t("eos");
  //!   t.readText("eos.dat");              // e.g. 101 rows (temperature) x 51 columns (chemical potential)
  //!   t.setGrid(0.100, 0.002, 0.0, 0.01); // x = T from 0.100 GeV in steps of 0.002; y = mu from 0 in steps of 0.01
  //!   double p = t.interpolate(0.1555, 0.123);
  //! @endcode
  //!
  class Table2D : public Object
  {
  protected:

  unsigned int        _nRows;     //!< number of rows
  unsigned int        _nColumns;  //!< number of columns
  std::vector<double> _values;    //!< values, row by row (index row*_nColumns+column)
  bool   _hasGrid;                //!< whether a grid is set
  double _xMin;                   //!< x of row 0
  double _dx;                     //!< x step between rows
  double _yMin;                   //!< y of column 0
  double _dy;                     //!< y step between columns

  //! @brief Throw unless row and column are valid. @param row row index @param column column index @param source caller name
  void checkIndices(unsigned int row, unsigned int column, const char * source) const;

  public:

  //! @name Construction
  //! @{

  //! @brief Empty table (0 rows, 0 columns). @param name name of the table
  Table2D(const String & name="Table2D");
  //! @brief Table of the given size, every value set to value. @param name name @param nRows rows @param nColumns columns
  //! @param value initial value
  Table2D(const String & name, unsigned int nRows, unsigned int nColumns, double value=0.0);
  //! @brief Table from rows of values. @param name name @param rows rows (all of the same size)
  //! @throws MathException if the rows have different sizes
  Table2D(const String & name, const std::vector< std::vector<double> > & rows);
  //! @brief Destructor.
  virtual ~Table2D() {}

  //! @}

  //! @name Size and values
  //! @{

  //! @brief Number of rows. @return rows
  unsigned int nRows() const { return _nRows; }
  //! @brief Number of columns. @return columns
  unsigned int nColumns() const { return _nColumns; }
  //! @brief Whether the table has no values. @return true if empty
  bool empty() const { return _values.empty(); }
  //! @brief Change the size; values are kept where row and column still exist, new ones set to value.
  //! @param nRows rows @param nColumns columns @param value value of new cells
  void resize(unsigned int nRows, unsigned int nColumns, double value=0.0);
  //! @brief Remove all values (0 rows, 0 columns); the grid is removed too.
  void clear() override;
  //! @brief Set every value to zero (the size and grid are kept).
  void reset() override;

  //! @brief Value at (row,column). @param row row @param column column @return value @throws MathException if out of range
  double at(unsigned int row, unsigned int column) const;
  //! @brief Set the value at (row,column). @param row row @param column column @param value value
  //! @throws MathException if out of range
  void setAt(unsigned int row, unsigned int column, double value);
  //! @brief Value at (row,column), unchecked. @param row row @param column column @return value
  double operator()(unsigned int row, unsigned int column) const { return _values[std::size_t(row)*_nColumns+column]; }
  //! @brief Value at (row,column), unchecked and writable. @param row row @param column column @return reference to the value
  double & operator()(unsigned int row, unsigned int column) { return _values[std::size_t(row)*_nColumns+column]; }

  //! @brief Values of a row. @param row row @return nColumns() values @throws MathException if out of range
  std::vector<double> row(unsigned int row) const;
  //! @brief Values of a column. @param column column @return nRows() values @throws MathException if out of range
  std::vector<double> column(unsigned int column) const;
  //! @brief Replace a row. @param row row @param values nColumns() values @throws MathException if out of range or wrong size
  void setRow(unsigned int row, const std::vector<double> & values);
  //! @brief Replace a column. @param column column @param values nRows() values @throws MathException if out of range or wrong size
  void setColumn(unsigned int column, const std::vector<double> & values);
  //! @brief Append a row (the first row added to an empty table sets the number of columns).
  //! @param values row values @throws MathException if the size differs from nColumns() or is zero
  void addRow(const std::vector<double> & values);
  //! @brief All values, row by row (index row*nColumns()+column). @return values
  const std::vector<double> & values() const { return _values; }
  //! @brief All rows. @return nRows() vectors of nColumns() values
  std::vector< std::vector<double> > rows() const;

  //! @}

  //! @name Grid and interpolation
  //! @{

  //!
  //! @brief Set the grid: row i holds x = xMin + i*dx, column j holds y = yMin + j*dy.
  //! @param xMin x of row 0 @param dx x step (> 0) @param yMin y of column 0 @param dy y step (> 0)
  //! @throws MathException if dx or dy is not positive
  //!
  void setGrid(double xMin, double dx, double yMin, double dy);
  //! @brief Whether a grid is set. @return true if set
  bool hasGrid() const { return _hasGrid; }
  //! @brief x of row 0. @return xMin
  double xMin() const { return _xMin; }
  //! @brief x step. @return dx
  double dx() const { return _dx; }
  //! @brief x of the last row. @return xMin+(nRows()-1)*dx
  double xMax() const { return _xMin + (_nRows>0 ? (_nRows-1)*_dx : 0.0); }
  //! @brief y of column 0. @return yMin
  double yMin() const { return _yMin; }
  //! @brief y step. @return dy
  double dy() const { return _dy; }
  //! @brief y of the last column. @return yMin+(nColumns()-1)*dy
  double yMax() const { return _yMin + (_nColumns>0 ? (_nColumns-1)*_dy : 0.0); }
  //! @brief x of a row. @param row row @return xMin+row*dx
  double xAt(unsigned int row) const { return _xMin + row*_dx; }
  //! @brief y of a column. @param column column @return yMin+column*dy
  double yAt(unsigned int column) const { return _yMin + column*_dy; }

  //!
  //! @brief Bilinear interpolation of the tabulated function at (x,y).
  //! With a single row (column), the function is taken as constant in x (y).
  //! @param x x value @param y y value
  //! @param allowExtrapolation if true, points outside the grid are extrapolated linearly from the nearest cell;
  //! if false, they throw
  //! @return interpolated value
  //! @throws MathException if there is no grid, the table is empty, or (x,y) is outside and extrapolation is not allowed
  //!
  double interpolate(double x, double y, bool allowExtrapolation=true) const;

  //! @}

  //! @name Text files
  //! @{

  //!
  //! @brief Read the table from a text file, replacing the values (the grid is kept).
  //! @param fileName path of the file
  //! @throws FileException if the file cannot be read or holds rows of different sizes or non-numeric values
  //! (the table is then unchanged)
  //!
  void readText(const String & fileName);
  //!
  //! @brief Write the table to a text file (replacing it), one row per line, in scientific notation.
  //! @param fileName path of the file @param width field width @param precision number of digits after the point
  //! @throws FileException if the file cannot be written
  //!
  void writeText(const String & fileName, int width=16, int precision=6) const;

  //! @}

  //! @brief Print the name, size, grid and values.
  void print() const override;

  //! @name Binary files (independent of ROOT)
  //! @{

  //! @brief Tag of Table2D records in binary files. @return "CAPB"
  static const char * binaryTag() { return "CAPB"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 1; }
  //! @brief Name stored in binary records. @return name()
  String binaryName() const { return name(); }
  //! @brief Write the binary payload after the name. @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Rebuild this table from a binary payload. @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid (this table is then unchanged)
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write as one binary record to a stream. @param out stream @throws FileException if writing fails
  void writeBinary(std::ostream & out) const;
  //! @brief Read the next Table2D record of a stream. @param in stream @return false at the end of the stream
  bool readBinary(std::istream & in);
  //! @brief Save to a binary file. @param fileName file name @param append add at the end of the file instead of replacing it
  void saveBinary(const String & fileName, bool append=false) const;
  //! @brief Load the table of the given name (the first one if name is empty) from a binary file. @param fileName file name @param name name
  void loadBinary(const String & fileName, const String & name="");

  //! @}

  ClassDefOverride(Table2D,1)
  };

  } // namespace MATH
} // namespace CAP

#endif
