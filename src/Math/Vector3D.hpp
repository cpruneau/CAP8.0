#ifndef CAP_Vector3D
#define CAP_Vector3D
#include <algorithm>
#include <limits>
#include <cmath>
#include <ostream>
#include <iostream>
#include <vector>
#include "Aliases.hpp"
#include "BinaryIO.hpp"

namespace CAP
{
namespace MATH
{

class Vector3D
{
protected:
  double _x1;
  double _x2;
  double _x3;

public:

  friend class VectorLorentz;
  
  Vector3D();
  Vector3D(const double & x1, const double & x2, const double & x3);
  Vector3D(const float values[3]);
  Vector3D(const double values[3]);
  Vector3D(const std::vector<float> & values);
  Vector3D(const std::vector<double> & values);
  Vector3D(const Vector3D & source);
  virtual ~Vector3D() {};
  //! Component at the given index: 0 for x, 1 for y, 2 for z. Throws MathException for index>2.
  double operator[] (unsigned int index) const;

  inline double x()  const
  {
  return _x1;
  }

  inline double y()  const
  {
  return _x2;
  }

  inline double z()  const
  {
  return _x3;
  }

  void setXYZ(const double & x1, const double & x2, const double & x3);
  void setXYZ(const float values[3]);
  void setXYZ(const double values[3]);
  void setXYZ(const std::vector<float> & values);
  void setXYZ(const std::vector<double> & values);
  void setXYZ(const Vector3D & source);
  void setPhiThetaR(const double & phi, const double & theta, const double & r);
  void setPhiRhoZ(const double & phi, const double & rho, const double & z);
  inline double  phi() const
  {
  return std::atan2(_x2,_x1);
  }

  //! cos(phi); returns 1 for a vector along the z axis (phi taken as 0)
  inline double  cosPhi() const
  {
  double rho = std::sqrt(_x1*_x1+_x2*_x2);
  return (rho>0.0) ? _x1/rho : 1.0;
  }

  //! sin(phi); returns 0 for a vector along the z axis (phi taken as 0)
  inline double  sinPhi() const
  {
  double rho = std::sqrt(_x1*_x1+_x2*_x2);
  return (rho>0.0) ? _x2/rho : 0.0;
  }

  double  deltaPhi(const Vector3D & other) const;

  inline double theta() const
  {
  return std::atan2(std::sqrt(_x1*_x1+_x2*_x2),_x3);
  }

  //! cos(theta); returns 1 for the null vector (theta taken as 0, consistent with theta())
  inline double  cosTheta() const
  {
  double r = std::sqrt(_x1*_x1+_x2*_x2+_x3*_x3);
  return (r>0.0) ? _x3/r : 1.0;
  }

  //! sin(theta); returns 0 for the null vector (theta taken as 0, consistent with theta())
  inline double sinTheta() const
  {
  double ptSq = _x1*_x1+_x2*_x2;
  double r    = std::sqrt(ptSq+_x3*_x3);
  return (r>0.0) ? std::sqrt(ptSq)/r : 0.0;
  }

  inline double modulus() const
  {
  return std::sqrt(_x1*_x1+_x2*_x2+_x3*_x3);
  }

  inline double  modulusSquare() const
  {
  return _x1*_x1+_x2*_x2+_x3*_x3;
  }

  inline double  perp() const
  {
  return std::sqrt(_x1*_x1+_x2*_x2);
  }

  inline double perpSquare() const
  {
  return _x1*_x1+_x2*_x2;
  }

  Vector3D & operator= (const Vector3D & rhs);
  bool  operator== (const Vector3D & rhs) const;
  bool  operator!= (const Vector3D & rhs) const;
  Vector3D & operator+= (const Vector3D & rhs);
  Vector3D & operator-= (const Vector3D & otherVector);
  Vector3D operator- () const;
  inline double  scalarProduct(const Vector3D & rhs) const
  {
  return _x1*rhs._x1 + _x2*rhs._x2 + _x3*rhs._x3;
  }

  double distanceX(const Vector3D & other) const;
  double distanceY(const Vector3D & other) const;
  double distanceZ(const Vector3D & other) const;
  double distanceSq(const Vector3D & other) const;
  double distance(const Vector3D & other) const;

  Vector3D unit() const;
  void rotateX(const double & angle);
  void rotateY(const double & angle);
  void rotateZ(const double & angle);
  virtual void print() const;

  friend Vector3D operator+(const Vector3D & left, const Vector3D & right);
  friend Vector3D operator-(const Vector3D & left, const Vector3D & right);
  friend double operator*(const Vector3D & left, const Vector3D & right);
  friend Vector3D operator* (const Vector3D & v, double a);
  friend Vector3D operator* (double a, const Vector3D & v);
  friend std::ostream& operator<<(std::ostream& out, const Vector3D & v);


  //! @name Binary files (independent of ROOT)
  //! Saved as "CAP3" records in the CAP binary format (see BinaryIO.hpp): compact, portable, checksummed, and restored
  //! exactly. Records of other classes in the same file are skipped.
  //! @{

  //! @brief Tag of Vector3D records in binary files. @return "CAP3"
  static const char * binaryTag() { return "CAP3"; }
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
  //! @brief Read the next Vector3D record of a stream into this object, skipping records of other classes.
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
  static void saveBinary(const String & fileName, const std::vector<Vector3D> & objects);
  //! @brief Load all the Vector3D objects of a binary file, in order. @param fileName file name @return objects
  //! @throws FileException if the file cannot be read or is corrupted
  static std::vector<Vector3D> loadAllBinary(const String & fileName);

  //! @}

  ClassDef(Vector3D,1)

};


//!
//! Add the two vectors and return a new vector
//!
Vector3D operator+(const Vector3D & left, const Vector3D & right);
Vector3D operator-(const Vector3D & left, const Vector3D & right);
double operator*(const Vector3D & left, const Vector3D & right);
Vector3D operator* (const Vector3D & v, double a);
Vector3D operator* (double a, const Vector3D & v);
std::ostream& operator<<(std::ostream& out, const Vector3D & v);

} // namespace MATH
} // namespace CAP

#endif
