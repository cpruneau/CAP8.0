#ifndef CAP_VectorLorentz
#define CAP_VectorLorentz
#include <algorithm>
#include <limits>
#include <cmath>
#include <vector>
#include <ostream>
#include "Aliases.hpp"
#include "BinaryIO.hpp"
#include "Exceptions.hpp"
#include "MathConstants.hpp"
#include "Vector3D.hpp"

namespace CAP
{
namespace MATH
{


//!
//! Class encapsulates Lorentz vectors
//!
//!  Note 1: Uses the convention that c==1 (speed of light equals unity) so that t,x,y,z  have  units of distance (t is ct) and E, px, py, pz have units of energy (MeV or GeV in most cases)
//!  Note 2: The (+,-,-,-) metric is used throughout.
//!  Note 3: double or e are considered the 0th component as in most modern texts.
//!
//! Storage  for lorentz vectors
//! _x0=t, _x1= x,_x2= y, _x3=z (i.e., cartesian coordinates)


class VectorLorentz
{
protected:
  double _x0;
  double _x1;
  double _x2;
  double _x3;

public:

  VectorLorentz();
  VectorLorentz(const double & x0, const double & x1, const double & x2, const double & x3);
  VectorLorentz(const float values[4]);
  VectorLorentz(const double values[4]);
  VectorLorentz(const std::vector<float> & values);
  VectorLorentz(const std::vector<double> & values);
  VectorLorentz(const VectorLorentz & source);
  virtual ~VectorLorentz() {};
  double operator[] (unsigned int index) const;
  inline double t()  const
  {
  return _x0;
  }

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

  void setTXYZ(const double & x0, const double & x1, const double & x2, const double & x3);
  void setTXYZ(const float values[4]);
  void setTXYZ(const double values[4]);
  void setTXYZ(const std::vector<float> & values);
  void setTXYZ(const std::vector<double> & values);
  void setTXYZ(const VectorLorentz & source);
  void setTPhiThetaR(const double & t, const double & phi, const double & theta, const double & r);
  void setTPhiRhoZ(const double & t, const double & phi, const double & rho, const double & z);
  void setMXYZ(const double & m, const double & x, const double & y, const double & z);
  void setMPhiRhoZ(const double & m, const double & phi, const double & rho, const double & z);
  void setMPhiPtY(const double & m, const double & phi, const double & pt, const double & y);
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

  double  deltaPhi(const VectorLorentz & other) const;

  inline double theta() const
  {
  return std::atan2(std::sqrt(_x1*_x1+_x2*_x2),_x3);
  }

  //! cos(theta); returns 1 for a null 3-vector (theta taken as 0, consistent with theta())
  inline double  cosTheta() const
  {
  double r = std::sqrt(_x1*_x1+_x2*_x2+_x3*_x3);
  return (r>0.0) ? _x3/r : 1.0;
  }

  //! sin(theta); returns 0 for a null 3-vector (theta taken as 0, consistent with theta())
  inline double sinTheta() const
  {
  double ptSq = _x1*_x1+_x2*_x2;
  double r    = std::sqrt(ptSq+_x3*_x3);
  return (r>0.0) ? std::sqrt(ptSq)/r : 0.0;
  }

  inline double modulus3() const
  {
  return std::sqrt(_x1*_x1+_x2*_x2+_x3*_x3);
  }

  inline double  modulus3Square() const
  {
  return _x1*_x1+_x2*_x2+_x3*_x3;
  }

  double modulus() const;

  inline double  modulusSquare() const
  {
  return _x0*_x0 - (_x1*_x1+_x2*_x2+_x3*_x3);
  }

  inline double  perp() const
  {
  return std::sqrt(_x1*_x1+_x2*_x2);
  }

  inline double perpSquare() const
  {
  return _x1*_x1+_x2*_x2;
  }

  Vector3D xyz() const;

  inline double massSquare() const
  {
  return modulusSquare();
  }

  double mass() const;
  double betaX() const;
  double betaY() const;
  double betaZ() const;
  double beta() const;
  double betaSquare() const;
  Vector3D betaVector() const;
  double gammaSquare() const;
  double gamma() const;
  double pseudorapidity() const;
  double transverseMassSq() const;
  double transverseMass() const;
  VectorLorentz & operator= (const VectorLorentz & rhs);
  bool  operator== (const VectorLorentz & rhs) const;
  bool  operator!= (const VectorLorentz & rhs) const;
  VectorLorentz & operator+= (const VectorLorentz & rhs);
  VectorLorentz & operator-= (const VectorLorentz & otherVector);
  VectorLorentz operator- () const;
  inline double  scalarProduct3D(const VectorLorentz & rhs) const
  {
  return _x1*rhs._x1 + _x2*rhs._x2 + _x3*rhs._x3;
  }

  inline double  scalarProduct(const VectorLorentz & rhs) const
  {
  return _x0*rhs._x0 - _x1*rhs._x1 - _x2*rhs._x2 - _x3*rhs._x3;
  }

  double distanceT(const VectorLorentz & other) const;
  double distanceX(const VectorLorentz & other) const;
  double distanceY(const VectorLorentz & other) const;
  double distanceZ(const VectorLorentz & other) const;
  double distance2DSq(const VectorLorentz & other) const;
  double distance2D(const VectorLorentz & other) const;
  double distance3DSq(const VectorLorentz & other) const;
  double distance3D(const VectorLorentz & other) const;
  double distanceSq(const VectorLorentz & other) const;
  double distance(const VectorLorentz & other) const;


  Vector3D unit()  const;
  void rotateX(const double & angle);
  void rotateY(const double & angle);
  void rotateZ(const double & angle);
  void boostX(const double & beta);
  void boostY(const double & beta);
  void boostZ(const double & beta);
  void boost(const double & betaX,const double & betaY,const double & betaZ);
  void boost(const Vector3D & v);
  double rapidity() const;
  double  deltaRapidity(const VectorLorentz & other) const;
  double  deltaPseudoRapidity(const VectorLorentz & other) const;
  virtual void print() const;

  friend VectorLorentz operator+(const VectorLorentz & left, const VectorLorentz & right);
  friend VectorLorentz operator-(const VectorLorentz & left, const VectorLorentz & right);
  friend double operator* (const VectorLorentz & left, const VectorLorentz & right);
  friend VectorLorentz operator*(const VectorLorentz & v, double a);
  friend VectorLorentz operator*(double a, const VectorLorentz & v);
  friend std::ostream& operator<<(std::ostream& out, const VectorLorentz & v);


  //! @name Binary files (independent of ROOT)
  //! Saved as "CAPL" records in the CAP binary format (see BinaryIO.hpp): compact, portable, checksummed, and restored
  //! exactly. Records of other classes in the same file are skipped.
  //! @{

  //! @brief Tag of VectorLorentz records in binary files. @return "CAPL"
  static const char * binaryTag() { return "CAPL"; }
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
  //! @brief Read the next VectorLorentz record of a stream into this object, skipping records of other classes.
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
  static void saveBinary(const String & fileName, const std::vector<VectorLorentz> & objects);
  //! @brief Load all the VectorLorentz objects of a binary file, in order. @param fileName file name @return objects
  //! @throws FileException if the file cannot be read or is corrupted
  static std::vector<VectorLorentz> loadAllBinary(const String & fileName);

  //! @}

  ClassDef(VectorLorentz,1)

};


//!
//! Add the two vectors and return a new vector
//!
VectorLorentz operator+ (const VectorLorentz & left, const VectorLorentz & right);
VectorLorentz operator- (const VectorLorentz & left, const VectorLorentz & right);
double operator* (const VectorLorentz & left, const VectorLorentz & right);
VectorLorentz operator* (const VectorLorentz & v, double a);
VectorLorentz operator* (double a, const VectorLorentz & v);
std::ostream& operator<<(std::ostream& out, const VectorLorentz & v);

} // namespace MATH
} // namespace CAP

#endif
