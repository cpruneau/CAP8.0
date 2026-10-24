#ifndef CAP_Vector2D
#define CAP_Vector2D
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
//! @brief Vector in two dimensions with cartesian components (x, y).
//!
//! Companion of Vector3D, with the same interface restricted to the plane.
//! Components can be set from cartesian (setXY) or polar (setPhiR) coordinates.
//! The azimuthal angle phi is measured from the x axis, in (-pi, pi].
//!
//! Example:
//! @code
//!   CAP::MATH::Vector2D v(3.0,4.0);
//!   double r = v.modulus();     // 5
//!   v.rotate(M_PI/2);           // (-4,3)
//! @endcode
//!
class Vector2D
{
protected:
  double _x1;  //!< x component
  double _x2;  //!< y component

public:

  //! @brief Default constructor: null vector (0,0).
  Vector2D();
  //! @brief Construct from cartesian components. @param x1 x component @param x2 y component
  Vector2D(const double & x1, const double & x2);
  //! @brief Construct from an array {x, y}. @param values array of 2 components
  Vector2D(const float values[2]);
  //! @brief Construct from an array {x, y}. @param values array of 2 components
  Vector2D(const double values[2]);
  //! @brief Construct from a vector {x, y, ...}; only the first 2 elements are used.
  //! @param values vector with at least 2 elements
  //! @throws MathException if values has fewer than 2 elements
  Vector2D(const std::vector<float> & values);
  //! @brief Construct from a vector {x, y, ...}; only the first 2 elements are used.
  //! @param values vector with at least 2 elements
  //! @throws MathException if values has fewer than 2 elements
  Vector2D(const std::vector<double> & values);
  //! @brief Copy constructor. @param source vector to copy
  Vector2D(const Vector2D & source);
  //! @brief Destructor.
  virtual ~Vector2D() {}

  //! @brief Component at the given index: 0 for x, 1 for y.
  //! @param index component index (0 or 1) @return component value
  //! @throws MathException if index>1
  double operator[] (unsigned int index) const;

  //! @brief x component. @return x
  inline double x()  const
  {
  return _x1;
  }

  //! @brief y component. @return y
  inline double y()  const
  {
  return _x2;
  }

  //! @brief Set the cartesian components. @param x1 x component @param x2 y component
  void setXY(const double & x1, const double & x2);
  //! @brief Set the components from an array {x, y}. @param values array of 2 components
  void setXY(const float values[2]);
  //! @brief Set the components from an array {x, y}. @param values array of 2 components
  void setXY(const double values[2]);
  //! @brief Set the components from a vector {x, y, ...}.
  //! @param values vector with at least 2 elements @throws MathException if values has fewer than 2 elements
  void setXY(const std::vector<float> & values);
  //! @brief Set the components from a vector {x, y, ...}.
  //! @param values vector with at least 2 elements @throws MathException if values has fewer than 2 elements
  void setXY(const std::vector<double> & values);
  //! @brief Copy the components of another vector. @param source vector to copy
  void setXY(const Vector2D & source);
  //! @brief Set the components from polar coordinates: x = r cos(phi), y = r sin(phi).
  //! @param phi azimuthal angle (radians) @param r modulus
  void setPhiR(const double & phi, const double & r);

  //! @brief Azimuthal angle, atan2(y,x). @return phi in (-pi, pi]; 0 for the null vector
  inline double  phi() const
  {
  return std::atan2(_x2,_x1);
  }

  //! @brief cos(phi); returns 1 for the null vector (phi taken as 0). @return cos(phi)
  inline double  cosPhi() const
  {
  double r = std::sqrt(_x1*_x1+_x2*_x2);
  return (r>0.0) ? _x1/r : 1.0;
  }

  //! @brief sin(phi); returns 0 for the null vector (phi taken as 0). @return sin(phi)
  inline double  sinPhi() const
  {
  double r = std::sqrt(_x1*_x1+_x2*_x2);
  return (r>0.0) ? _x2/r : 0.0;
  }

  //! @brief Azimuthal angle between this and the other vector, phi - other.phi, mapped onto [0, 2pi).
  //! @param other other vector @return angle difference in [0, 2pi)
  double  deltaPhi(const Vector2D & other) const;

  //! @brief length of the vector. @return sqrt(x^2 + y^2)
  inline double modulus() const
  {
  return std::sqrt(_x1*_x1+_x2*_x2);
  }

  //! @brief Squared length of the vector. @return x^2 + y^2
  inline double  modulusSquare() const
  {
  return _x1*_x1+_x2*_x2;
  }

  //! @brief Assignment operator. @param rhs vector to copy @return reference to this vector
  Vector2D & operator= (const Vector2D & rhs);
  //! @brief Exact component-wise equality. @param rhs vector to compare with @return true if all components are equal
  bool  operator== (const Vector2D & rhs) const;
  //! @brief Negation of operator==. @param rhs vector to compare with @return true if a component differs
  bool  operator!= (const Vector2D & rhs) const;
  //! @brief Add a vector to this vector. @param rhs vector to add @return reference to this vector
  Vector2D & operator+= (const Vector2D & rhs);
  //! @brief Subtract a vector from this vector. @param rhs vector to subtract @return reference to this vector
  Vector2D & operator-= (const Vector2D & rhs);
  //! @brief Unary minus. @return (-x, -y)
  Vector2D operator- () const;

  //! @brief Scalar (dot) product. @param rhs other vector @return x*rhs.x + y*rhs.y
  inline double  scalarProduct(const Vector2D & rhs) const
  {
  return _x1*rhs._x1 + _x2*rhs._x2;
  }

  //! @brief z component of the cross product of the two vectors taken in the xy plane.
  //! @param rhs other vector @return x*rhs.y - y*rhs.x (positive if rhs is counter-clockwise from this vector)
  inline double  crossProduct(const Vector2D & rhs) const
  {
  return _x1*rhs._x2 - _x2*rhs._x1;
  }

  //! @brief Difference of the x components. @param other other vector @return x - other.x
  double distanceX(const Vector2D & other) const;
  //! @brief Difference of the y components. @param other other vector @return y - other.y
  double distanceY(const Vector2D & other) const;
  //! @brief Squared distance between the two points. @param other other vector @return squared distance
  double distanceSq(const Vector2D & other) const;
  //! @brief Distance between the two points. @param other other vector @return distance
  double distance(const Vector2D & other) const;

  //! @brief Unit vector in the direction of this vector.
  //! @return vector of length 1 @throws MathException for the null vector
  Vector2D unit() const;
  //! @brief Active rotation of this vector by the given angle (counter-clockwise for angle>0).
  //! @param angle rotation angle (radians)
  void rotate(const double & angle);
  //! @brief Print the vector as (x,y) to std::cout.
  virtual void print() const;

  friend Vector2D operator+(const Vector2D & left, const Vector2D & right);
  friend Vector2D operator-(const Vector2D & left, const Vector2D & right);
  friend double operator*(const Vector2D & left, const Vector2D & right);
  friend Vector2D operator* (const Vector2D & v, double a);
  friend Vector2D operator* (double a, const Vector2D & v);
  friend std::ostream& operator<<(std::ostream& out, const Vector2D & v);


  //! @name Binary files (independent of ROOT)
  //! Saved as "CAP2" records in the CAP binary format (see BinaryIO.hpp): compact, portable, checksummed, and restored
  //! exactly. Records of other classes in the same file are skipped.
  //! @{

  //! @brief Tag of Vector2D records in binary files. @return "CAP2"
  static const char * binaryTag() { return "CAP2"; }
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
  //! @brief Read the next Vector2D record of a stream into this object, skipping records of other classes.
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
  static void saveBinary(const String & fileName, const std::vector<Vector2D> & objects);
  //! @brief Load all the Vector2D objects of a binary file, in order. @param fileName file name @return objects
  //! @throws FileException if the file cannot be read or is corrupted
  static std::vector<Vector2D> loadAllBinary(const String & fileName);

  //! @}

  ClassDef(Vector2D,1)

};

//! @relates Vector2D
//! @brief Sum of two vectors. @param left first vector @param right second vector @return left+right
Vector2D operator+(const Vector2D & left, const Vector2D & right);
//! @relates Vector2D
//! @brief Difference of two vectors. @param left first vector @param right second vector @return left-right
Vector2D operator-(const Vector2D & left, const Vector2D & right);
//! @relates Vector2D
//! @brief Scalar (dot) product of two vectors. @param left first vector @param right second vector @return left.right
double operator*(const Vector2D & left, const Vector2D & right);
//! @relates Vector2D
//! @brief Vector scaled by a number. @param v vector @param a scale factor @return a*v
Vector2D operator* (const Vector2D & v, double a);
//! @relates Vector2D
//! @brief Vector scaled by a number. @param a scale factor @param v vector @return a*v
Vector2D operator* (double a, const Vector2D & v);
//! @relates Vector2D
//! @brief Write the vector as (x,y) to a stream. @param out output stream @param v vector @return the output stream
std::ostream& operator<<(std::ostream& out, const Vector2D & v);

} // namespace MATH
} // namespace CAP

#endif
