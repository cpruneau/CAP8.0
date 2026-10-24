#ifndef CAP_MATH_Matrix
#define CAP_MATH_Matrix
#include <vector>
#include <ostream>
#include "String.hpp"
#include "BinaryIO.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Lightweight n by m matrix of doubles.
  //!
  //! Elements are addressed by (row, column), both starting at 0, and stored row by row.
  //! Addition and subtraction require matrices of the same dimensions; the product A*B requires
  //! A to be n x m and B to be m x p, and gives an n x p matrix. Dimension mismatches throw MathException.
  //!
  //! Example:
  //! @code
  //!   using namespace CAP::MATH;
  //!   Matrix a(2, 2, {1.0, 2.0,
  //!                   3.0, 4.0});
  //!   Matrix b(2, 3, 1.0);           // 2 x 3, all elements 1
  //!   Matrix c = a*b;                // 2 x 3
  //!   double d = a.determinant();    // -2
  //!   a(0,1) = 5.0;                  // or a.setElement(0,1,5.0)
  //! @endcode
  //!
  class Matrix
  {
  public:

  //! @name Construction and assignment
  //! @{

  //! @brief Default constructor: empty (0 x 0) matrix.
  Matrix();
  //!
  //! @brief Matrix of nRows x nColumns elements, all set to value.
  //! @param nRows number of rows @param nColumns number of columns @param value value of all elements
  //!
  Matrix(unsigned int nRows, unsigned int nColumns, double value=0.0);
  //!
  //! @brief Matrix of nRows x nColumns elements with the given values, listed row by row.
  //! @param nRows number of rows @param nColumns number of columns @param values nRows*nColumns values, row by row
  //! @throws MathException if values.size() != nRows*nColumns
  //!
  Matrix(unsigned int nRows, unsigned int nColumns, const std::vector<double> & values);
  //! @brief Copy constructor. @param source matrix to copy
  Matrix(const Matrix & source);
  //! @brief Assignment operator. @param rhs matrix to copy @return reference to this matrix
  Matrix & operator=(const Matrix & rhs);
  //! @brief Destructor.
  virtual ~Matrix() {}

  //! @}

  //! @name Dimensions
  //! @{

  //! @brief Number of rows. @return number of rows
  unsigned int nRows() const { return _nRows; }
  //! @brief Number of columns. @return number of columns
  unsigned int nColumns() const { return _nColumns; }
  //! @brief Whether the matrix is square. @return true if nRows() == nColumns()
  bool isSquare() const { return _nRows==_nColumns; }
  //! @brief Whether both matrices have the same dimensions. @param other matrix to compare to @return true if same dimensions
  bool sameDimensions(const Matrix & other) const { return _nRows==other._nRows && _nColumns==other._nColumns; }

  //! @}

  //! @name Elements
  //! All throw IndexOutOfBoundException if row >= nRows() or column >= nColumns().
  //! @{

  //! @brief Element (row, column). @param row row index @param column column index @return element value
  double element(unsigned int row, unsigned int column) const;
  //! @brief Set element (row, column). @param row row index @param column column index @param value new value
  void setElement(unsigned int row, unsigned int column, double value);
  //! @brief Writable access to element (row, column). @param row row index @param column column index @return reference to the element
  double & operator()(unsigned int row, unsigned int column);
  //! @brief Element (row, column). @param row row index @param column column index @return element value
  double operator()(unsigned int row, unsigned int column) const;
  //! @brief All elements, row by row. @return values
  const std::vector<double> & values() const { return _values; }

  //! @}

  //! @name Arithmetic
  //! @{

  //! @brief Sum of this matrix and rhs. @param rhs matrix to add @return sum
  //! @throws MathException if the dimensions differ
  Matrix add(const Matrix & rhs) const;
  //! @brief Difference of this matrix and rhs. @param rhs matrix to subtract @return difference
  //! @throws MathException if the dimensions differ
  Matrix subtract(const Matrix & rhs) const;
  //! @brief Matrix product: this (n x m) times rhs (m x p), giving an n x p matrix. @param rhs right-hand matrix @return product
  //! @throws MathException if nColumns() != rhs.nRows()
  Matrix multiply(const Matrix & rhs) const;

  //! @brief Same as add(). @param rhs matrix to add @return sum
  Matrix operator+(const Matrix & rhs) const { return add(rhs); }
  //! @brief Same as subtract(). @param rhs matrix to subtract @return difference
  Matrix operator-(const Matrix & rhs) const { return subtract(rhs); }
  //! @brief Same as multiply(). @param rhs right-hand matrix @return product
  Matrix operator*(const Matrix & rhs) const { return multiply(rhs); }

  //!
  //! @brief Determinant, computed by LU decomposition with partial pivoting (order n^3 operations).
  //! @return determinant (1 for the empty matrix)
  //! @throws MathException if the matrix is not square
  //!
  double determinant() const;

  //! @brief Transpose. @return matrix with rows and columns exchanged
  Matrix transpose() const;
  //!
  //! @brief Inverse of a square matrix, by Gauss-Jordan elimination with partial pivoting.
  //! @return inverse
  //! @throws MathException if the matrix is not square, or singular (a pivot vanishes, relative to the size of the
  //! elements)
  //!
  Matrix inverse() const;
  //!
  //! @brief Solution x of A x = b, A being this square matrix (Gaussian elimination with partial pivoting).
  //! @param b right-hand side (nRows() values)
  //! @return x
  //! @throws MathException if the matrix is not square, the sizes differ, or the matrix is singular
  //!
  std::vector<double> solve(const std::vector<double> & b) const;
  //! @brief Identity matrix. @param n size @return n x n identity
  static Matrix identity(unsigned int n);

  //! @}


  //! @name Binary files (independent of ROOT)
  //! Saved as "CAPM" records in the CAP binary format (see BinaryIO.hpp): compact, portable, checksummed, and restored
  //! exactly. Records of other classes in the same file are skipped.
  //! @{

  //! @brief Tag of Matrix records in binary files. @return "CAPM"
  static const char * binaryTag() { return "CAPM"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 1; }
  //! @brief Name stored in binary records. @return ""
  String binaryName() const { return ""; }
  //! @brief Write the binary payload after the name (used by writeBinary()). @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Rebuild this object from a binary payload (used by readBinary()). @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write this matrix as one binary record to a stream (open in binary mode). @param out stream @throws FileException if writing fails
  void writeBinary(std::ostream & out) const;
  //! @brief Read the next Matrix record of a stream into this object, skipping records of other classes.
  //! @param in stream @return false at the end of the stream (unchanged) @throws FileException if the data is invalid (unchanged)
  bool readBinary(std::istream & in);
  //! @brief Save this matrix to a binary file. @param fileName file name @param append add at the end of the file instead of replacing it
  //! @throws FileException if the file cannot be written
  void saveBinary(const String & fileName, bool append=false) const;
  //! @brief Load from a binary file the matrix at position index among the objects of this class (0 = the first one).
  //! @param fileName file name @param index position
  //! @throws FileException if the file cannot be read, is corrupted, or holds fewer objects (this one is then unchanged)
  void loadBinary(const String & fileName, unsigned int index=0);
  //! @brief Save several objects to one binary file (replacing it). @param fileName file name @param objects objects to save, in order
  //! @throws FileException if the file cannot be written
  static void saveBinary(const String & fileName, const std::vector<Matrix> & objects);
  //! @brief Load all the Matrix objects of a binary file, in order. @param fileName file name @return objects
  //! @throws FileException if the file cannot be read or is corrupted
  static std::vector<Matrix> loadAllBinary(const String & fileName);

  //! @}

  //! @brief Write the matrix to a stream, one row per line. @param out output stream @param m matrix @return the output stream
  friend std::ostream & operator<<(std::ostream & out, const Matrix & m);

  protected:

  //! @brief Throw IndexOutOfBoundException if (row, column) is outside the matrix. @param source name of the calling function
  void checkIndices(unsigned int row, unsigned int column, const char * source) const;
  //! @brief Throw MathException if rhs does not have the same dimensions. @param source name of the calling function
  void checkSameDimensions(const Matrix & rhs, const char * source) const;

  unsigned int _nRows;          //!< number of rows
  unsigned int _nColumns;       //!< number of columns
  std::vector<double> _values;  //!< elements, row by row: element (i,j) is _values[i*_nColumns + j]

  };

  //! @relates Matrix
  //! @brief Write the matrix to a stream, one row per line. @param out output stream @param m matrix @return the output stream
  std::ostream & operator<<(std::ostream & out, const Matrix & m);

  } // namespace MATH
} // namespace CAP

#endif
