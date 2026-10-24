#ifndef CAP_VectorND
#define CAP_VectorND
#include <cmath>
#include <ostream>
#include <iostream>
#include <vector>
#include "String.hpp"
#include "BinaryIO.hpp"
#include "ClassDef.hpp"

namespace CAP
{
namespace MATH
{

//!
//! @brief Vector with an arbitrary number of dimensions n and a Euclidean scalar product.
//!
//! Companion of Vector2D and Vector3D for any dimension. The components are stored in
//! double precision and accessed with 0-based indices: v[0], v[1], ..., v[n-1].
//! Indexing is 0-based, as in Vector2D, Vector3D and VectorLorentz.
//!
//! Operations between two vectors (sum, difference, scalar product, distance) require
//! both vectors to have the same dimension and throw MathException otherwise.
//!
//! Example:
//! @code
//!   std::vector<float> data = {1.0f, 2.0f, 2.0f, 4.0f};
//!   CAP::MATH::VectorND u(data);          // 4 dimensions
//!   CAP::MATH::VectorND v(4, 1.0);        // (1,1,1,1)
//!   double s = u*v;                       // scalar product: 9
//!   double r = u.modulus();               // 5
//! @endcode
//!
class VectorND
{
protected:
  std::vector<double> _x;  //!< Components x[0] ... x[n-1]

  //!
  //! @brief Throw if the other vector does not have the same dimension as this vector.
  //! @param other vector to check
  //! @param source name of the calling function, reported in the exception
  //! @throws MathException if the dimensions differ
  //!
  void checkDimension(const VectorND & other, const char * source) const;

public:

  //! @brief Default constructor: vector of dimension 0.
  VectorND();

  //!
  //! @brief Construct a vector of n dimensions with all components equal to value.
  //! @param n number of dimensions
  //! @param value value of every component
  //!
  explicit VectorND(unsigned int n, double value=0.0);

  //!
  //! @brief Construct from an array of n components.
  //! @param values array of at least n components
  //! @param n number of dimensions
  //!
  VectorND(const float values[], unsigned int n);

  //!
  //! @brief Construct from an array of n components.
  //! @param values array of at least n components
  //! @param n number of dimensions
  //!
  VectorND(const double values[], unsigned int n);

  //!
  //! @brief Construct from a std::vector<float>; the dimension is values.size().
  //! @param values components of the vector
  //!
  VectorND(const std::vector<float> & values);

  //!
  //! @brief Construct from a std::vector<double>; the dimension is values.size().
  //! @param values components of the vector
  //!
  VectorND(const std::vector<double> & values);

  //! @brief Copy constructor. @param source vector to copy
  VectorND(const VectorND & source);

  //! @brief Destructor.
  virtual ~VectorND() {}

  //! @brief Number of dimensions (components) of this vector. @return dimension
  inline unsigned int dimension() const
  {
  return _x.size();
  }

  //!
  //! @brief Read a component (0-based index).
  //! @param index component index, 0 to dimension()-1
  //! @return component value
  //! @throws MathException if index>=dimension()
  //!
  double operator[] (unsigned int index) const;

  //!
  //! @brief Access a component for reading or writing (0-based index).
  //! @param index component index, 0 to dimension()-1
  //! @return reference to the component
  //! @throws MathException if index>=dimension()
  //!
  double & operator[] (unsigned int index);

  //! @brief Read-only access to all the components. @return vector of components
  inline const std::vector<double> & values() const
  {
  return _x;
  }

  //!
  //! @brief Set one component (0-based index).
  //! @param index component index, 0 to dimension()-1
  //! @param value new value
  //! @throws MathException if index>=dimension()
  //!
  void setValue(unsigned int index, double value);

  //! @brief Replace all components; the dimension becomes values.size(). @param values new components
  void setValues(const std::vector<float> & values);
  //! @brief Replace all components; the dimension becomes values.size(). @param values new components
  void setValues(const std::vector<double> & values);
  //! @brief Replace all components by n values from an array. @param values array of at least n components @param n number of dimensions
  void setValues(const float values[], unsigned int n);
  //! @brief Replace all components by n values from an array. @param values array of at least n components @param n number of dimensions
  void setValues(const double values[], unsigned int n);
  //! @brief Copy the components (and dimension) of another vector. @param source vector to copy
  void setValues(const VectorND & source);

  //! @brief Set every component to the same value, keeping the dimension. @param value new value of all components
  void fill(double value);

  //!
  //! @brief Change the number of dimensions; existing components are kept, new ones are set to value.
  //! @param n new number of dimensions
  //! @param value value of added components
  //!
  void resize(unsigned int n, double value=0.0);

  //! @brief length (Euclidean norm) of the vector. @return sqrt(sum of x[i]^2)
  double modulus() const;

  //! @brief Squared length of the vector. @return sum of x[i]^2
  double modulusSquare() const;

  //!
  //! @brief Scalar (dot) product of this vector with rhs.
  //! @param rhs other vector (same dimension)
  //! @return sum of x[i]*rhs.x[i]
  //! @throws MathException if the dimensions differ
  //!
  double scalarProduct(const VectorND & rhs) const;

  //! @brief Assignment operator: copies the components and the dimension. @param rhs vector to copy @return reference to this vector
  VectorND & operator= (const VectorND & rhs);
  //! @brief Exact equality: same dimension and identical components. @param rhs vector to compare with @return true if equal
  bool  operator== (const VectorND & rhs) const;
  //! @brief Negation of operator==. @param rhs vector to compare with @return true if different
  bool  operator!= (const VectorND & rhs) const;
  //! @brief Add a vector of the same dimension. @param rhs vector to add @return reference to this vector @throws MathException if the dimensions differ
  VectorND & operator+= (const VectorND & rhs);
  //! @brief Subtract a vector of the same dimension. @param rhs vector to subtract @return reference to this vector @throws MathException if the dimensions differ
  VectorND & operator-= (const VectorND & rhs);
  //! @brief Multiply every component by a number. @param a scale factor @return reference to this vector
  VectorND & operator*= (double a);
  //! @brief Divide every component by a number. @param a divisor @return reference to this vector @throws MathException if a==0
  VectorND & operator/= (double a);
  //! @brief Unary minus. @return vector with all components negated
  VectorND operator- () const;

  //!
  //! @brief Squared Euclidean distance between the two points.
  //! @param other other vector (same dimension)
  //! @return squared distance
  //! @throws MathException if the dimensions differ
  //!
  double distanceSq(const VectorND & other) const;

  //!
  //! @brief Euclidean distance between the two points.
  //! @param other other vector (same dimension)
  //! @return distance
  //! @throws MathException if the dimensions differ
  //!
  double distance(const VectorND & other) const;

  //! @brief Unit vector in the direction of this vector. @return vector of length 1 @throws MathException for a null vector
  VectorND unit() const;

  //! @brief Print the vector as (x0,x1,...) to std::cout.
  virtual void print() const;

  friend VectorND operator+(const VectorND & left, const VectorND & right);
  friend VectorND operator-(const VectorND & left, const VectorND & right);
  friend double operator*(const VectorND & left, const VectorND & right);
  friend VectorND operator* (const VectorND & v, double a);
  friend VectorND operator* (double a, const VectorND & v);
  friend std::ostream& operator<<(std::ostream& out, const VectorND & v);


  //! @name Binary files (independent of ROOT)
  //! Saved as "CAPV" records in the CAP binary format (see BinaryIO.hpp): compact, portable, checksummed, and restored
  //! exactly. Records of other classes in the same file are skipped.
  //! @{

  //! @brief Tag of VectorND records in binary files. @return "CAPV"
  static const char * binaryTag() { return "CAPV"; }
  //! @brief Format version written (and highest version read). @return version
  static unsigned int binaryVersion() { return 1; }
  //! @brief Name stored in binary records. @return ""
  String binaryName() const { return ""; }
  //! @brief Write the binary payload after the name (used by writeBinary()). @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //! @brief Rebuild this object from a binary payload (used by readBinary()). @param r reader @param name stored name @param version format version
  //! @throws Exception if the data is invalid
  void decodeBinary(BinaryReader & r, const String & name, unsigned int version);
  //! @brief Write this vector as one binary record to a stream (open in binary mode). @param out stream @throws FileException if writing fails
  void writeBinary(std::ostream & out) const;
  //! @brief Read the next VectorND record of a stream into this object, skipping records of other classes.
  //! @param in stream @return false at the end of the stream (unchanged) @throws FileException if the data is invalid (unchanged)
  bool readBinary(std::istream & in);
  //! @brief Save this vector to a binary file. @param fileName file name @param append add at the end of the file instead of replacing it
  //! @throws FileException if the file cannot be written
  void saveBinary(const String & fileName, bool append=false) const;
  //! @brief Load from a binary file the vector at position index among the objects of this class (0 = the first one).
  //! @param fileName file name @param index position
  //! @throws FileException if the file cannot be read, is corrupted, or holds fewer objects (this one is then unchanged)
  void loadBinary(const String & fileName, unsigned int index=0);
  //! @brief Save several objects to one binary file (replacing it). @param fileName file name @param objects objects to save, in order
  //! @throws FileException if the file cannot be written
  static void saveBinary(const String & fileName, const std::vector<VectorND> & objects);
  //! @brief Load all the VectorND objects of a binary file, in order. @param fileName file name @return objects
  //! @throws FileException if the file cannot be read or is corrupted
  static std::vector<VectorND> loadAllBinary(const String & fileName);

  //! @}

  ClassDef(VectorND,1)

};

//! @relates VectorND
//! @brief Sum of two vectors of the same dimension. @param left first vector @param right second vector @return left+right
//! @throws MathException if the dimensions differ
VectorND operator+(const VectorND & left, const VectorND & right);
//! @relates VectorND
//! @brief Difference of two vectors of the same dimension. @param left first vector @param right second vector @return left-right
//! @throws MathException if the dimensions differ
VectorND operator-(const VectorND & left, const VectorND & right);
//! @relates VectorND
//! @brief Scalar (dot) product of two vectors of the same dimension. @param left first vector @param right second vector @return left.right
//! @throws MathException if the dimensions differ
double operator*(const VectorND & left, const VectorND & right);
//! @relates VectorND
//! @brief Vector scaled by a number. @param v vector @param a scale factor @return a*v
VectorND operator* (const VectorND & v, double a);
//! @relates VectorND
//! @brief Vector scaled by a number. @param a scale factor @param v vector @return a*v
VectorND operator* (double a, const VectorND & v);
//! @relates VectorND
//! @brief Write the vector as (x0,x1,...) to a stream. @param out output stream @param v vector @return the output stream
std::ostream& operator<<(std::ostream& out, const VectorND & v);

} // namespace MATH
} // namespace CAP

#endif
