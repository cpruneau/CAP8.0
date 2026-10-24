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
//! @file Shape.hpp
//! @brief Shape: rectangle, ellipse, triangle, polygon or line (with arrows) drawn over a graph.
#ifndef CAP__Shape
#define CAP__Shape
#include <vector>
#include "Geom2D.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Graph;

  //!
  //! @brief Shape drawn over a graph: rectangle, ellipse, triangle, polygon, or line with no, one or two arrow heads.
  //!
  //! Its points are fractions of the cell of the graph ((0,0) lower left, (1,1) upper right): the shape keeps its place
  //! in the graph when the axes are zoomed, and when the canvas is drawn at another size or saved (SVG, PDF).
  //!  - rectangle, ellipse, triangle: two opposite corners of the box holding the shape (the triangle has its apex in
  //!    the middle of the side of the second corner, its base on the side of the first);
  //!  - polygon: its vertices (closed);
  //!  - line: its ends (the arrow head of a single arrow at the second one).
  //!
  //! The closed shapes are filled with fillAttributes() (color, opacity, solid or hatch pattern, see FillAttributes)
  //! and outlined with lineAttributes() (color, opacity, style, width); a line uses lineAttributes().
  //! @code
  //!   Shape box(Shape::kRectangle);
  //!   box.setPoints({ Point2D(0.2, 0.6), Point2D(0.5, 0.8) });
  //!   box.fillAttributes().set(kYellow, 0.4, FillAttributes::kHatchForward);
  //!   graph.addShape(box);
  //! @endcode
  //!
  class Shape : public Geom2D
  {
  public:

  //! Kinds of shapes
  enum Kind { kRectangle, kEllipse, kTriangle, kPolygon, kLine };
  //! Arrow heads of a line
  enum Arrows { kNoArrow, kSingleArrow, kDoubleArrow };

  //! @brief Shape of a kind, with no points. @param kind kind
  Shape(Kind kind=kRectangle);
  //! @brief Copy constructor. @param src shape copied
  Shape(const Shape & src);
  //! @brief Copy assignment. @param rhs shape copied @return this shape
  Shape & operator=(const Shape & rhs);
  //! @brief Destructor.
  virtual ~Shape() { }

  Kind kind() const { return _kind; }   //!< Kind of the shape
  void setKind(Kind kind) { _kind = kind; }   //!< Set the kind of the shape
  Arrows arrows() const { return _arrows; }   //!< Arrow heads (lines)
  void setArrows(Arrows arrows) { _arrows = arrows; }   //!< Set the arrow heads (lines)
  bool isClosed() const { return _kind != kLine; }   //!< Whether the shape is closed (filled)
  //! @brief Readable name of a kind. @param kind kind @return "rectangle", "ellipse", ...
  static const char * kindName(Kind kind);

  //! @brief Outline of the shape: vertices of the closed shapes (an ellipse as 72 of them), points of a line.
  //! @return points, in fractions of the cell
  std::vector<Point2D> outline() const;
  //! @brief Whether the shape can be drawn: 2 distinct points (3 for a polygon). @return true if so
  bool isValid() const;
  //! @brief Move the shape. @param dx,dy displacement, in fractions of the cell
  void move(float dx, float dy);

  //! @brief Whether a point is on the shape: inside a closed shape (if filled, or near its outline) or near a line.
  //! @param p point, in fractions of the cell @param toleranceX,toleranceY distance counted as near, in fractions of the cell
  //! @return true if so
  bool contains(const Point2D & p, float toleranceX, float toleranceY) const;

  //! @brief Point of the canvas of a point of the cell of a graph. @param graph graph @param p point, in fractions of its
  //! cell @return point of the canvas (normalized coordinates)
  static Point2D toCanvas(const Graph & graph, const Point2D & p);
  //! @brief Point of the cell of a graph of a point of the canvas. @param graph graph @param p point of the canvas
  //! @return point, in fractions of the cell
  static Point2D fromCanvas(const Graph & graph, const Point2D & p);

  //! @brief Draw the shape over a graph (in its cell). @param painter painter @param graph graph
  void draw(Painter & painter, const Graph & graph) const;
  using Drawable2D::draw;

  protected:

  Kind   _kind;   //!< kind of the shape
  Arrows _arrows;   //!< arrow heads of a line

  ClassDef(Shape,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Shape
