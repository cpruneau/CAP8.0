/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Projection3D.hpp
//! @brief Projection3D: orthographic projection of the box of a 3D view onto the frame of a graph.
#ifndef CAP__Projection3D
#define CAP__Projection3D
#include "Graph.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! @brief Orthographic projection of the box of a 3D view (see Graph::is3D()) onto the frame of the graph.
  //!
  //! A point of the box is given by its fractions (fx, fy, fz) along the x, y and z axes, in [0,1] (see fraction(),
  //! which follows the logarithmic scales). The box, a cube, is turned by azimuth() about the vertical axis, then seen
  //! from elevation() above the x-y plane; it is scaled to fit the frame, with room for the labels. depth() orders
  //! points from the eye (larger: farther), for the painter's algorithm.
  //!
  //! With the default angles (30, 30), the corner (x maximum, y minimum) is in front: the x axis runs from the left
  //! to the front, and the y axis from the front to the right.
  //!
  class Projection3D
  {
  public:

  //! @brief Projection of the box of a graph for a view, on a painter of the given size (device units).
  //! @param graph graph (its frame, view angles) @param view ranges and scales @param width width of the painter
  //! @param height height of the painter
  Projection3D(const Graph & graph, const Graph::View & view, float width, float height);

  //! @brief Fraction of a value along axis k (0 x, 1 y, 2 z) of the view, linear or logarithmic (values <= 0 on a
  //! logarithmic axis give 0). @param k axis @param value value @return fraction (not clamped)
  double fraction(int k, double value) const;
  //! @brief Canvas position of a point of the box. @param fx fraction along x @param fy fraction along y
  //! @param fz fraction along z @return canvas point
  Point2D toCanvas(double fx, double fy, double fz) const;
  //! @brief Canvas position of a point given by its values. @param x x @param y y @param z z @return canvas point
  Point2D atValues(double x, double y, double z) const { return toCanvas(fraction(0, x), fraction(1, y), fraction(2, z)); }
  //! @brief Distance from the eye, along the line of sight (larger: farther). @param fx fraction along x
  //! @param fy fraction along y @param fz fraction along z @return depth
  double depth(double fx, double fy, double fz) const;
  //! @brief A direction of the box in the coordinates of the eye: right, up, and away from the eye.
  //! @param dx along x @param dy along y @param dz along z @param right to the right @param up upward @param away
  //! away from the eye
  void direction(double dx, double dy, double dz, double & right, double & up, double & away) const;

  protected:

  const Graph::View _view;   //!< ranges and scales
  double _cosA;   //!< cosine of the azimuth
  double _sinA;   //!< sine of the azimuth
  double _cosE;   //!< cosine of the elevation
  double _sinE;   //!< sine of the elevation
  double _scale;   //!< device units per unit of the box
  double _centerX;   //!< canvas position of the center of the box
  double _centerY;   //!< canvas position of the center of the box
  float  _width;   //!< width of the painter (device units)
  float  _height;   //!< height of the painter (device units)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Projection3D
