#ifndef CAP_GRAPHICS_Point3D
#define CAP_GRAPHICS_Point3D
#include "Point2D.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! @brief Point (or displacement) in three dimensions, used to place and draw objects in 3D views.
  //!
  //! Coordinates are floats. Data to be plotted are kept in the Math data point classes (CAP::MATH::DataPoints and
  //! its derived classes); Point3D only holds positions in the drawing space.
  //!
  class Point3D
  {
  public:

  //! @brief Point at the origin.
  Point3D() : _x(0.0f), _y(0.0f), _z(0.0f) { }
  //! @brief Point (x,y,z). @param x first coordinate @param y second coordinate @param z third coordinate
  Point3D(float x, float y, float z) : _x(x), _y(y), _z(z) { }
  //! @brief Point (p.x(),p.y(),z). @param p point in the (x,y) plane @param z third coordinate
  explicit Point3D(const Point2D & p, float z=0.0f) : _x(p.x()), _y(p.y()), _z(z) { }

  //! @brief First coordinate. @return x
  float x() const { return _x; }
  //! @brief Second coordinate. @return y
  float y() const { return _y; }
  //! @brief Third coordinate. @return z
  float z() const { return _z; }
  //! @brief Set the first coordinate. @param x value
  void setX(float x) { _x = x; }
  //! @brief Set the second coordinate. @param y value
  void setY(float y) { _y = y; }
  //! @brief Set the third coordinate. @param z value
  void setZ(float z) { _z = z; }
  //! @brief Set the three coordinates. @param x first @param y second @param z third
  void set(float x, float y, float z) { _x = x; _y = y; _z = z; }
  //! @brief Projection on the (x,y) plane. @return (x,y)
  Point2D xy() const { return Point2D(_x,_y); }

  Point3D operator+(const Point3D & p) const { return Point3D(_x+p._x, _y+p._y, _z+p._z); }
  Point3D operator-(const Point3D & p) const { return Point3D(_x-p._x, _y-p._y, _z-p._z); }
  Point3D operator*(float a) const { return Point3D(a*_x, a*_y, a*_z); }
  Point3D & operator+=(const Point3D & p) { _x += p._x; _y += p._y; _z += p._z; return *this; }
  Point3D & operator-=(const Point3D & p) { _x -= p._x; _y -= p._y; _z -= p._z; return *this; }
  Point3D & operator*=(float a) { _x *= a; _y *= a; _z *= a; return *this; }
  bool operator==(const Point3D & p) const { return _x==p._x && _y==p._y && _z==p._z; }
  bool operator!=(const Point3D & p) const { return !(*this==p); }

  protected:

  float _x; //!< first coordinate
  float _y; //!< second coordinate
  float _z; //!< third coordinate
  };

  inline Point3D operator*(float a, const Point3D & p) { return p*a; }

  } // namespace GRAPHICS
} // namespace CAP

#endif // CAP_GRAPHICS_Point3D
