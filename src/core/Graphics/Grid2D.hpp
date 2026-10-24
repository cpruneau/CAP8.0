/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Grid2D.hpp
//! @brief Grid2D: values on the cells of a rectangular grid, for the color maps of rank-2 functions and data.
#ifndef CAP__Grid2D
#define CAP__Grid2D
#include <vector>

namespace CAP
{
  namespace MATH { class Function; class DataPoints; class Histogram; }

  namespace GRAPHICS
  {

  //!
  //! @brief Values on the cells of a rectangular grid: what a color map draws.
  //!
  //! Cell (i,j) spans [xEdge(i), xEdge(i+1)] x [yEdge(j), yEdge(j+1)]; the edges need not be equally spaced. A cell
  //! without a value (e.g. an empty bin of a histogram, a point where a function cannot be evaluated) holds NaN and is
  //! not drawn.
  //!
  //! Grids are made from the objects that graphs show:
  //!  - fromHistogram(): the bins of a rank-2 histogram (empty bins, with no content and no uncertainty, have no
  //!    value);
  //!  - fromFunction(): a rank-2 function evaluated at the centers of nx x ny cells covering a range (equally spaced,
  //!    or equally spaced in log10 along a logarithmic axis);
  //!  - fromPoints(): rank-2 data points whose coordinates form a regular grid (every combination of the distinct x
  //!    and y values once): the edges are halfway between the nodes. Scattered points form no grid.
  //!
  //! contour() traces the lines where the values cross a level (marching squares on the centers of the cells).
  //!
  class Grid2D
  {
  public:

  //! @brief A point of a contour line (data coordinates).
  struct Point
  {
    double x;   //!< x
    double y;   //!< y
  };

  //! @brief Empty grid (no cells).
  Grid2D() { }
  //! @brief Grid of the given edges, without values. @param xEdges edges along x (increasing) @param yEdges edges
  //! along y (increasing)
  Grid2D(const std::vector<double> & xEdges, const std::vector<double> & yEdges);

  //! @brief Number of cells along x. @return number
  int nX() const { return _xEdges.size() > 1 ? int(_xEdges.size()) - 1 : 0; }
  //! @brief Number of cells along y. @return number
  int nY() const { return _yEdges.size() > 1 ? int(_yEdges.size()) - 1 : 0; }
  //! @brief Whether the grid has no cells. @return true if so
  bool isEmpty() const { return nX() == 0 || nY() == 0; }
  //! @brief Edges along x. @return nX()+1 edges
  const std::vector<double> & xEdges() const { return _xEdges; }
  //! @brief Edges along y. @return nY()+1 edges
  const std::vector<double> & yEdges() const { return _yEdges; }
  //! @brief Value of cell (i,j). @param i index along x @param j index along y @return value, or NaN if none
  double value(int i, int j) const { return _values[std::size_t(i) + std::size_t(nX())*j]; }
  //! @brief Set the value of cell (i,j). @param i index along x @param j index along y @param value value (NaN: none)
  void setValue(int i, int j, double value) { _values[std::size_t(i) + std::size_t(nX())*j] = value; }
  //!
  //! @brief Range of the values of the cells that overlap a rectangle.
  //! @param xLow left @param xHigh right @param yLow bottom @param yHigh top of the rectangle
  //! @param positiveOnly only the values > 0 (for a logarithmic scale) @param low lowest value @param high highest
  //! value @return false if no cell has a value
  //!
  bool range(double xLow, double xHigh, double yLow, double yHigh, bool positiveOnly, double & low, double & high) const;
  //! @brief The cell holding (x,y). @param x x @param y y @param i index along x @param j index along y
  //! @return false if (x,y) is outside the grid
  bool cell(double x, double y, int & i, int & j) const;

  //!
  //! @brief Lines where the values cross a level: marching squares on the centers of the cells, the crossings
  //! interpolated linearly between neighboring centers (in log10 of the values if logValues); squares with a corner
  //! without value are skipped, and ambiguous squares (saddles) are resolved by the mean of their corners.
  //! @param level level @param logValues interpolate log10 of the values (values <= 0 have none)
  //! @return polylines; a closed line ends with its first point
  //!
  std::vector<std::vector<Point>> contour(double level, bool logValues=false) const;

  //! @brief The bins of a rank-2 histogram. @param histogram histogram @return grid (empty if the rank is not 2)
  static Grid2D fromHistogram(const MATH::Histogram & histogram);
  //!
  //! @brief A rank-2 function evaluated at the centers of the cells of a grid.
  //! @param function function @param xLow left @param xHigh right @param nX number of cells along x @param yLow bottom
  //! @param yHigh top @param nY number of cells along y @param logX cells equally spaced in log10(x) (if xLow > 0)
  //! @param logY cells equally spaced in log10(y) (if yLow > 0) @return grid (empty if the rank is not 2 or a range
  //! is empty)
  //!
  static Grid2D fromFunction(const MATH::Function & function, double xLow, double xHigh, int nX,
                             double yLow, double yHigh, int nY, bool logX=false, bool logY=false);
  //! @brief Rank-2 points on a regular grid. @param data data points @param grid set to the grid
  //! @return false if the points do not form a regular grid (or are not of rank 2)
  static bool fromPoints(const MATH::DataPoints & data, Grid2D & grid);
  //! @brief Grid of rank-2 data: fromHistogram() for a histogram, otherwise fromPoints(). @param data data
  //! @param grid set to the grid @return false if the data form no grid
  static bool fromData(const MATH::DataPoints & data, Grid2D & grid);

  protected:

  std::vector<double> _xEdges;   //!< edges along x
  std::vector<double> _yEdges;   //!< edges along y
  std::vector<double> _values;   //!< values of the cells (NaN: none), x varying fastest
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Grid2D
