#include "Matrix.hpp"
#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include "MathExceptions.hpp"


namespace CAP
{
  namespace MATH
  {

  Matrix::Matrix()
  :
  _nRows(0),
  _nColumns(0),
  _values()
  {}

  Matrix::Matrix(unsigned int nRows, unsigned int nColumns, double value)
  :
  _nRows(nRows),
  _nColumns(nColumns),
  _values(std::size_t(nRows)*nColumns,value)
  {}

  Matrix::Matrix(unsigned int nRows, unsigned int nColumns, const std::vector<double> & values)
  :
  _nRows(nRows),
  _nColumns(nColumns),
  _values(values)
  {
  if (values.size()!=std::size_t(nRows)*nColumns)
    throw MathException("Number of values differs from nRows*nColumns",__FUNCTION__);
  }

  Matrix::Matrix(const Matrix & source)
  :
  _nRows(source._nRows),
  _nColumns(source._nColumns),
  _values(source._values)
  {}

  Matrix & Matrix::operator=(const Matrix & rhs)
  {
  if (this!=&rhs)
    {
    _nRows    = rhs._nRows;
    _nColumns = rhs._nColumns;
    _values   = rhs._values;
    }
  return *this;
  }

  void Matrix::checkIndices(unsigned int row, unsigned int column, const char * source) const
  {
  if (row>=_nRows)       throw IndexOutOfBoundException(row,_nRows,"Row index out of range",source);
  if (column>=_nColumns) throw IndexOutOfBoundException(column,_nColumns,"Column index out of range",source);
  }

  void Matrix::checkSameDimensions(const Matrix & rhs, const char * source) const
  {
  if (!sameDimensions(rhs))
    {
    String message = "Matrices have different dimensions: ";
    message += std::to_string(_nRows).c_str();     message += "x";
    message += std::to_string(_nColumns).c_str();  message += " and ";
    message += std::to_string(rhs._nRows).c_str(); message += "x";
    message += std::to_string(rhs._nColumns).c_str();
    throw MathException(message,source);
    }
  }

  double Matrix::element(unsigned int row, unsigned int column) const
  {
  checkIndices(row,column,__FUNCTION__);
  return _values[row*_nColumns+column];
  }

  void Matrix::setElement(unsigned int row, unsigned int column, double value)
  {
  checkIndices(row,column,__FUNCTION__);
  _values[row*_nColumns+column] = value;
  }

  double & Matrix::operator()(unsigned int row, unsigned int column)
  {
  checkIndices(row,column,__FUNCTION__);
  return _values[row*_nColumns+column];
  }

  double Matrix::operator()(unsigned int row, unsigned int column) const
  {
  checkIndices(row,column,__FUNCTION__);
  return _values[row*_nColumns+column];
  }

  Matrix Matrix::add(const Matrix & rhs) const
  {
  checkSameDimensions(rhs,__FUNCTION__);
  Matrix result(*this);
  for (std::size_t k=0; k<_values.size(); k++) result._values[k] += rhs._values[k];
  return result;
  }

  Matrix Matrix::subtract(const Matrix & rhs) const
  {
  checkSameDimensions(rhs,__FUNCTION__);
  Matrix result(*this);
  for (std::size_t k=0; k<_values.size(); k++) result._values[k] -= rhs._values[k];
  return result;
  }

  Matrix Matrix::multiply(const Matrix & rhs) const
  {
  if (_nColumns!=rhs._nRows)
    {
    String message = "Cannot multiply a ";
    message += std::to_string(_nRows).c_str();     message += "x";
    message += std::to_string(_nColumns).c_str();  message += " matrix by a ";
    message += std::to_string(rhs._nRows).c_str(); message += "x";
    message += std::to_string(rhs._nColumns).c_str();
    message += " matrix";
    throw MathException(message,__FUNCTION__);
    }
  Matrix result(_nRows,rhs._nColumns,0.0);
  // i-k-j loop order: rows of rhs and result are read and written contiguously
  for (unsigned int i=0; i<_nRows; i++)
    for (unsigned int k=0; k<_nColumns; k++)
      {
      double a = _values[i*_nColumns+k];
      if (a==0.0) continue;
      const double * b = &rhs._values[k*rhs._nColumns];
      double * c = &result._values[i*rhs._nColumns];
      for (unsigned int j=0; j<rhs._nColumns; j++) c[j] += a*b[j];
      }
  return result;
  }

  Matrix Matrix::transpose() const
  {
  Matrix t(_nColumns, _nRows);
  for (unsigned int r=0; r<_nRows; r++)
    for (unsigned int c=0; c<_nColumns; c++) t._values[c*_nRows + r] = _values[r*_nColumns + c];
  return t;
  }

  Matrix Matrix::identity(unsigned int n)
  {
  Matrix m(n, n, 0.0);
  for (unsigned int k=0; k<n; k++) m._values[k*n + k] = 1.0;
  return m;
  }

  namespace
  {
  //! Gauss-Jordan elimination of a (n x n) with partial pivoting, applied to b (n x m) as well: a becomes the
  //! identity and b the solution. Throws if a pivot vanishes relative to the largest element.
  void eliminate(std::vector<double> & a, std::vector<double> & b, unsigned int n, unsigned int m, const char * source)
  {
  double scale = 0;
  for (double v : a) scale = std::max(scale, std::fabs(v));
  if (!(scale > 0)) throw MathException("Singular matrix (all its elements are zero)", source);
  for (unsigned int col=0; col<n; col++)
    {
    unsigned int pivot = col;
    for (unsigned int r=col+1; r<n; r++) if (std::fabs(a[r*n + col]) > std::fabs(a[pivot*n + col])) pivot = r;
    if (!(std::fabs(a[pivot*n + col]) > 1E-14*scale)) throw MathException("Singular matrix", source);
    if (pivot != col)
      {
      for (unsigned int c=0; c<n; c++) std::swap(a[pivot*n + c], a[col*n + c]);
      for (unsigned int c=0; c<m; c++) std::swap(b[pivot*m + c], b[col*m + c]);
      }
    double p = a[col*n + col];
    for (unsigned int c=0; c<n; c++) a[col*n + c] /= p;
    for (unsigned int c=0; c<m; c++) b[col*m + c] /= p;
    for (unsigned int r=0; r<n; r++)
      {
      if (r == col) continue;
      double f = a[r*n + col];
      if (f == 0) continue;
      for (unsigned int c=0; c<n; c++) a[r*n + c] -= f*a[col*n + c];
      for (unsigned int c=0; c<m; c++) b[r*m + c] -= f*b[col*m + c];
      }
    }
  }
  } // namespace

  Matrix Matrix::inverse() const
  {
  if (!isSquare()) throw MathException("Only a square matrix has an inverse", __FUNCTION__);
  std::vector<double> a(_values);
  Matrix result = identity(_nRows);
  eliminate(a, result._values, _nRows, _nRows, __FUNCTION__);
  return result;
  }

  std::vector<double> Matrix::solve(const std::vector<double> & b) const
  {
  if (!isSquare()) throw MathException("The matrix of a linear system must be square", __FUNCTION__);
  if (b.size() != _nRows) throw MathException("The right-hand side has the wrong size", __FUNCTION__);
  std::vector<double> a(_values), x(b);
  eliminate(a, x, _nRows, 1, __FUNCTION__);
  return x;
  }

  double Matrix::determinant() const
  {
  if (!isSquare()) throw MathException("Determinant requires a square matrix",__FUNCTION__);
  const unsigned int n = _nRows;
  std::vector<double> a(_values);   // LU decomposition in place on a copy
  double det = 1.0;
  for (unsigned int col=0; col<n; col++)
    {
    // partial pivoting: largest element of the column, at or below the diagonal
    unsigned int pivot = col;
    double largest = std::fabs(a[col*n+col]);
    for (unsigned int row=col+1; row<n; row++)
      {
      double v = std::fabs(a[row*n+col]);
      if (v>largest) { largest = v; pivot = row; }
      }
    if (largest==0.0) return 0.0;   // singular matrix
    if (pivot!=col)
      {
      for (unsigned int j=0; j<n; j++) std::swap(a[col*n+j],a[pivot*n+j]);
      det = -det;
      }
    double diagonal = a[col*n+col];
    det *= diagonal;
    for (unsigned int row=col+1; row<n; row++)
      {
      double factor = a[row*n+col]/diagonal;
      if (factor==0.0) continue;
      for (unsigned int j=col+1; j<n; j++) a[row*n+j] -= factor*a[col*n+j];
      }
    }
  return det;
  }

  std::ostream & operator<<(std::ostream & out, const Matrix & m)
  {
  for (unsigned int i=0; i<m._nRows; i++)
    {
    for (unsigned int j=0; j<m._nColumns; j++)
      {
      if (j>0) out << " ";
      out << m._values[i*m._nColumns+j];
      }
    out << std::endl;
    }
  return out;
  }
  void Matrix::encodeBinary(BinaryWriter & w) const
  {
  w.writeVarUInt(_nRows);
  w.writeVarUInt(_nColumns);
  writeDoubles(w,_values);
  }

  void Matrix::decodeBinary(BinaryReader & r, const String & /*name*/, unsigned int /*version*/)
  {
  uint64_t nRows = r.readVarUInt();
  uint64_t nColumns = r.readVarUInt();
  std::vector<double> values = readDoubles(r);
  if (nRows > 0xFFFFFFFFull || nColumns > 0xFFFFFFFFull || values.size()!=nRows*nColumns)
    throw Exception("Matrix dimensions do not match the number of elements","Matrix::decodeBinary()");
  _nRows    = (unsigned int) nRows;
  _nColumns = (unsigned int) nColumns;
  _values   = values;
  }

  void Matrix::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool Matrix::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void Matrix::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void Matrix::loadBinary(const String & fileName, unsigned int index)    { loadBinaryObjectAt(fileName,*this,index); }
  void Matrix::saveBinary(const String & fileName, const std::vector<Matrix> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<Matrix> Matrix::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<Matrix>(fileName); }


  } // namespace MATH
} // namespace CAP
