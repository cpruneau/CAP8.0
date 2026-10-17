/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Point2D.hpp
//! @brief Point2D: point or vector of the plane.
#ifndef CAP_GRAPHICS_Point2D
#define CAP_GRAPHICS_Point2D

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! @brief Point (or displacement) in two dimensions, used to place and draw graphics objects.
  //!
  //! Coordinates are floats: canvas (normalized) coordinates in [0,1], or device units once mapped by a Painter.
  //! Data to be plotted are kept in the Math data point classes (CAP::MATH::DataPoints and its derived classes);
  //! Point2D only holds positions on the drawing.
  //!
  class Point2D
  {
  public:

  //! @brief Point at the origin.
  Point2D() : _x(0.0f), _y(0.0f) { }
  //! @brief Point (x,y). @param x first coordinate @param y second coordinate
  Point2D(float x, float y) : _x(x), _y(y) { }

  //! @brief First coordinate. @return x
  float x() const { return _x; }
  //! @brief Second coordinate. @return y
  float y() const { return _y; }
  //! @brief Set the first coordinate. @param x value
  void setX(float x) { _x = x; }
  //! @brief Set the second coordinate. @param y value
  void setY(float y) { _y = y; }
  //! @brief Set both coordinates. @param x first coordinate @param y second coordinate
  void set(float x, float y) { _x = x; _y = y; }

  Point2D operator+(const Point2D & p) const { return Point2D(_x+p._x, _y+p._y); }   //!< Sum
  Point2D operator-(const Point2D & p) const { return Point2D(_x-p._x, _y-p._y); }   //!< Difference
  Point2D operator*(float a) const { return Point2D(a*_x, a*_y); }   //!< Product by a number
  Point2D & operator+=(const Point2D & p) { _x += p._x; _y += p._y; return *this; }   //!< Add a point
  Point2D & operator-=(const Point2D & p) { _x -= p._x; _y -= p._y; return *this; }   //!< Subtract a point
  Point2D & operator*=(float a) { _x *= a; _y *= a; return *this; }   //!< Multiply by a number
  bool operator==(const Point2D & p) const { return _x==p._x && _y==p._y; }   //!< Equality of both coordinates
  bool operator!=(const Point2D & p) const { return !(*this==p); }   //!< Whether a coordinate differs

  protected:

  float _x; //!< first coordinate
  float _y; //!< second coordinate
  };

  //! @brief Product of a number and a point. @param a number @param p point @return a*p
  inline Point2D operator*(float a, const Point2D & p) { return p*a; }

  } // namespace GRAPHICS
} // namespace CAP

#endif // CAP_GRAPHICS_Point2D
