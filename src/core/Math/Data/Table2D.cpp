/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "Table2D.hpp"
#include "MathExceptions.hpp"
#include "TxtFile.hpp"
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>

ClassImp(CAP::MATH::Table2D);

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  // Sanity limit on the number of values when decoding binary data.
  constexpr uint64_t kMaximumDecodedValues = 1ULL<<34;

  // Parse the numbers of a line (separated by blanks, tabs or commas). Returns false if a token is not a number.
  bool parseLine(const std::string & line, std::vector<double> & values, std::string & badToken)
  {
  values.clear();
  std::string text(line);
  for (char & c : text) if (c==',' || c=='\t' || c=='\r') c = ' ';
  std::istringstream in(text);
  std::string token;
  while (in >> token)
    {
    char * end = nullptr;
    errno = 0;
    double v = std::strtod(token.c_str(),&end);
    if (end==token.c_str() || *end!='\0' || errno==ERANGE) { badToken = token; return false; }
    values.push_back(v);
    }
  return true;
  }
  }

  //////////////////////////////////////////////////////////////////////////
  // Construction
  //////////////////////////////////////////////////////////////////////////

  Table2D::Table2D(const String & name)
  :
  Object(name,name,"Table2D"),
  _nRows(0), _nColumns(0), _values(),
  _hasGrid(false), _xMin(0.0), _dx(1.0), _yMin(0.0), _dy(1.0)
  { }

  Table2D::Table2D(const String & name, unsigned int nRows, unsigned int nColumns, double value)
  :
  Table2D(name)
  {
  resize(nRows,nColumns,value);
  }

  Table2D::Table2D(const String & name, const std::vector< std::vector<double> > & rows)
  :
  Table2D(name)
  {
  for (const auto & r : rows) addRow(r);
  }

  //////////////////////////////////////////////////////////////////////////
  // Size and values
  //////////////////////////////////////////////////////////////////////////

  void Table2D::checkIndices(unsigned int row, unsigned int column, const char * source) const
  {
  if (row>=_nRows)       throw MathException("Row index out of range (row>=nRows())",source);
  if (column>=_nColumns) throw MathException("Column index out of range (column>=nColumns())",source);
  }

  void Table2D::resize(unsigned int nRows, unsigned int nColumns, double value)
  {
  if (nRows==0 || nColumns==0) { _nRows = 0; _nColumns = 0; _values.clear(); return; }
  std::vector<double> values(std::size_t(nRows)*nColumns,value);
  for (unsigned int i=0; i<std::min(nRows,_nRows); i++)
    for (unsigned int j=0; j<std::min(nColumns,_nColumns); j++)
      values[std::size_t(i)*nColumns+j] = _values[std::size_t(i)*_nColumns+j];
  _values.swap(values);
  _nRows    = nRows;
  _nColumns = nColumns;
  }

  void Table2D::clear()
  {
  _nRows = 0; _nColumns = 0; _values.clear();
  _hasGrid = false; _xMin = 0.0; _dx = 1.0; _yMin = 0.0; _dy = 1.0;
  }

  void Table2D::reset()
  {
  for (double & v : _values) v = 0.0;
  }

  double Table2D::at(unsigned int row, unsigned int column) const
  {
  checkIndices(row,column,__FUNCTION__);
  return _values[std::size_t(row)*_nColumns+column];
  }

  void Table2D::setAt(unsigned int row, unsigned int column, double value)
  {
  checkIndices(row,column,__FUNCTION__);
  _values[std::size_t(row)*_nColumns+column] = value;
  }

  std::vector<double> Table2D::row(unsigned int row) const
  {
  if (row>=_nRows) throw MathException("Row index out of range (row>=nRows())",__FUNCTION__);
  auto first = _values.begin() + std::size_t(row)*_nColumns;
  return std::vector<double>(first,first+_nColumns);
  }

  std::vector<double> Table2D::column(unsigned int column) const
  {
  if (column>=_nColumns) throw MathException("Column index out of range (column>=nColumns())",__FUNCTION__);
  std::vector<double> c(_nRows);
  for (unsigned int i=0; i<_nRows; i++) c[i] = _values[std::size_t(i)*_nColumns+column];
  return c;
  }

  void Table2D::setRow(unsigned int row, const std::vector<double> & values)
  {
  if (row>=_nRows) throw MathException("Row index out of range (row>=nRows())",__FUNCTION__);
  if (values.size()!=_nColumns) throw MathException("Row size differs from nColumns()",__FUNCTION__);
  std::copy(values.begin(),values.end(),_values.begin()+std::size_t(row)*_nColumns);
  }

  void Table2D::setColumn(unsigned int column, const std::vector<double> & values)
  {
  if (column>=_nColumns) throw MathException("Column index out of range (column>=nColumns())",__FUNCTION__);
  if (values.size()!=_nRows) throw MathException("Column size differs from nRows()",__FUNCTION__);
  for (unsigned int i=0; i<_nRows; i++) _values[std::size_t(i)*_nColumns+column] = values[i];
  }

  void Table2D::addRow(const std::vector<double> & values)
  {
  if (values.empty()) throw MathException("Cannot add an empty row",__FUNCTION__);
  if (_nRows>0 && values.size()!=_nColumns)
    throw MathException("Row size differs from nColumns()",__FUNCTION__);
  _values.insert(_values.end(),values.begin(),values.end());
  _nColumns = values.size();
  _nRows++;
  }

  std::vector< std::vector<double> > Table2D::rows() const
  {
  std::vector< std::vector<double> > r;
  r.reserve(_nRows);
  for (unsigned int i=0; i<_nRows; i++) r.push_back(row(i));
  return r;
  }

  //////////////////////////////////////////////////////////////////////////
  // Grid and interpolation
  //////////////////////////////////////////////////////////////////////////

  void Table2D::setGrid(double xMin, double dx, double yMin, double dy)
  {
  if (!(dx>0)) throw MathException("dx must be positive",__FUNCTION__);
  if (!(dy>0)) throw MathException("dy must be positive",__FUNCTION__);
  _hasGrid = true;
  _xMin = xMin; _dx = dx;
  _yMin = yMin; _dy = dy;
  }

  double Table2D::interpolate(double x, double y, bool allowExtrapolation) const
  {
  if (!_hasGrid) throw MathException("No grid set: call setGrid() first",__FUNCTION__);
  if (empty())   throw MathException("Table is empty",__FUNCTION__);
  // Tolerance so that points on the last grid line are inside.
  double tx = 1.0E-9*_dx, ty = 1.0E-9*_dy;
  if (!allowExtrapolation && (x<_xMin-tx || x>xMax()+tx || y<_yMin-ty || y>yMax()+ty))
    throw MathException("Point outside the grid and extrapolation not allowed",__FUNCTION__);
  // Cell (i,j) with corners (i,j),(i+1,j),(i,j+1),(i+1,j+1), clamped to the table: outside, the nearest cell is
  // extended linearly. A single row (column) is taken as constant along x (y).
  double u = 0.0, v = 0.0;
  unsigned int i = 0, j = 0;
  if (_nRows>1)
    {
    double fx = (x-_xMin)/_dx;
    long   ix = long(std::floor(fx));
    if (ix<0) ix = 0;
    if (ix>long(_nRows)-2) ix = long(_nRows)-2;
    i = ix;
    u = fx - ix;
    }
  if (_nColumns>1)
    {
    double fy = (y-_yMin)/_dy;
    long   iy = long(std::floor(fy));
    if (iy<0) iy = 0;
    if (iy>long(_nColumns)-2) iy = long(_nColumns)-2;
    j = iy;
    v = fy - iy;
    }
  unsigned int i1 = _nRows>1    ? i+1 : i;
  unsigned int j1 = _nColumns>1 ? j+1 : j;
  double f00 = (*this)(i,j),  f10 = (*this)(i1,j);
  double f01 = (*this)(i,j1), f11 = (*this)(i1,j1);
  return f00*(1-u)*(1-v) + f10*u*(1-v) + f01*(1-u)*v + f11*u*v;
  }

  //////////////////////////////////////////////////////////////////////////
  // Text files
  //////////////////////////////////////////////////////////////////////////

  void Table2D::readText(const String & fileName)
  {
  TxtFile file(fileName,File::kReadOnly);
  file.read();
  Table2D table(name());
  std::vector<double> values;
  std::string badToken;
  for (unsigned int k=0; k<file.nLines(); k++)
    {
    std::string line(file.line(k).data());
    std::size_t first = line.find_first_not_of(" \t\r");
    if (first==std::string::npos || line[first]=='#') continue;
    if (!parseLine(line,values,badToken))
      throw FileException(fileName,String("Line ")+String(std::to_string(k+1).c_str())+": not a number: "
                          +String(badToken.c_str()),"Table2D::readText()");
    if (values.empty()) continue;
    if (table._nRows>0 && values.size()!=table._nColumns)
      throw FileException(fileName,String("Line ")+String(std::to_string(k+1).c_str())+" has "
                          +String(std::to_string(values.size()).c_str())+" values instead of "
                          +String(std::to_string(table._nColumns).c_str()),"Table2D::readText()");
    table.addRow(values);
    }
  _nRows    = table._nRows;
  _nColumns = table._nColumns;
  _values.swap(table._values);
  }

  void Table2D::writeText(const String & fileName, int width, int precision) const
  {
  TxtFile file(fileName,File::kWriteOnly);
  for (unsigned int i=0; i<_nRows; i++)
    {
    std::ostringstream line;
    line << std::scientific << std::setprecision(precision);
    for (unsigned int j=0; j<_nColumns; j++)
      {
      if (j>0) line << "  ";
      line << std::setw(width) << (*this)(i,j);
      }
    file.addLine(String(line.str().c_str()));
    }
  file.write();
  }

  void Table2D::print() const
  {
  Object::print();
  std::cout << "Rows: " << _nRows << "  Columns: " << _nColumns;
  if (_hasGrid) std::cout << "  x: " << _xMin << " + i*" << _dx << "  y: " << _yMin << " + j*" << _dy;
  std::cout << std::endl;
  for (unsigned int i=0; i<_nRows; i++)
    {
    for (unsigned int j=0; j<_nColumns; j++) std::cout << std::setw(14) << (*this)(i,j);
    std::cout << std::endl;
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // Binary I/O
  //////////////////////////////////////////////////////////////////////////

  void Table2D::encodeBinary(BinaryWriter & w) const
  {
  w.writeVarUInt(_nRows);
  w.writeVarUInt(_nColumns);
  w.writeUInt8(_hasGrid ? 1 : 0);
  if (_hasGrid)
    {
    w.writeFloat64(_xMin); w.writeFloat64(_dx);
    w.writeFloat64(_yMin); w.writeFloat64(_dy);
    }
  for (double v : _values) w.writeFloat64(v);
  }

  void Table2D::decodeBinary(BinaryReader & r, const String & name, unsigned int /*version*/)
  {
  uint64_t nRows    = r.readVarUInt();
  uint64_t nColumns = r.readVarUInt();
  if ((nRows==0) != (nColumns==0)) throw Exception("Invalid table size","Table2D::decodeBinary()");
  if (nColumns>0 && nRows>kMaximumDecodedValues/nColumns) throw Exception("Invalid table size","Table2D::decodeBinary()");
  uint8_t grid = r.readUInt8();
  if (grid>1) throw Exception("Invalid grid flag","Table2D::decodeBinary()");
  double xMin=0.0, dx=1.0, yMin=0.0, dy=1.0;
  if (grid)
    {
    xMin = r.readFloat64(); dx = r.readFloat64();
    yMin = r.readFloat64(); dy = r.readFloat64();
    if (!(dx>0) || !(dy>0)) throw Exception("Invalid grid steps","Table2D::decodeBinary()");
    }
  uint64_t n = nRows*nColumns;
  if (8*n > r.remaining()) throw Exception("More values than data","Table2D::decodeBinary()");
  std::vector<double> values(n);
  for (auto & v : values) v = r.readFloat64();
  setName(name);
  setTitle(name);
  _nRows = nRows; _nColumns = nColumns; _values.swap(values);
  _hasGrid = grid; _xMin = xMin; _dx = dx; _yMin = yMin; _dy = dy;
  }

  void Table2D::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool Table2D::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void Table2D::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void Table2D::loadBinary(const String & fileName, const String & name)  { loadBinaryObject(fileName,*this,name); }

  } // namespace MATH
} // namespace CAP
