/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Graph.hpp
//! @brief Graph: a plot of functions and data points in a cell of a canvas; GraphItem: one object shown.
#ifndef CAP__Graph
#define CAP__Graph
#include <algorithm>
#include <memory>
#include <vector>
#include "Drawable2D.hpp"
#include "Axis.hpp"
#include "Margins.hpp"
#include "FillAttributes.hpp"
#include "LineAttributes.hpp"
#include "MarkerAttributes.hpp"
#include "TextAttributes.hpp"
#include "Rect2D.hpp"
#include "Grid2D.hpp"
#include "Palette.hpp"
#include "Shape.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace MATH
  {
  class Function;
  class DataPoints;
  struct FitRecord;
  }

  namespace GRAPHICS
  {

  //!
  //! @brief One object shown in a Graph: a function or a set of data points, with its drawing style.
  //!
  //! Objects of rank 1 (f(x), points (x,y)) are drawn as curves and markers with error bars. Objects of rank 2
  //! (f(x,y), histograms of rank 2, points (x,y) with a value z) are drawn as color maps on the cells of a Grid2D
  //! (see grid()): the bins of a histogram, the nodes of points on a regular grid, or mapSamples() x mapSamples() cells
  //! where a function is evaluated, over the part of its range that is visible (zooming in shows more detail).
  //! Scattered points of rank 2 form no grid: they are drawn as markers colored by their value. The colors come
  //! from the palette and the z range of the graph (see Graph).
  //!
  //! On a map can also be drawn: contour lines (showContours(): at contourIntervals() equal intervals of the z range,
  //! or at given contourLevels(); filled contours are a palette in as many bands, see Palette), the values written
  //! in the cells (showValues(), where they fit), and the points themselves as colored disks (showMapPoints(), always
  //! for scattered points), optionally sized by their values and outlined.
  //!
  //! The objects are not owned: they must outlive the graphs that show them (in CapApp they are owned by the
  //! application or by the files it opened).
  //!
  class GraphItem
  {
  public:

  //! @brief What the item shows: a function or data points
  enum Kind { kFunction, kData };
  //! How the systematic uncertainties of data points are drawn: boxes around the points, brackets above and below
  //! the points, or a band joining the points.
  enum SysStyle { kSysBoxes, kSysBrackets, kSysBand };

  //! @brief Function f(x) drawn as a curve over [xMinimum, xMaximum]. @param function function of rank 1
  //! @param xMinimum lower end of the curve @param xMaximum upper end @param label legend label (the function's
  //! name if empty)
  GraphItem(const MATH::Function * function, float xMinimum, float xMaximum, const String & label="");
  //! @brief Data points drawn as markers with error bars (rank 1) or as a color map (rank 2). @param data data
  //! points @param label legend label (the name of the data if empty)
  GraphItem(const MATH::DataPoints * data, const String & label="");
  //! @brief Function f(x,y) drawn as a color map over [xMinimum, xMaximum] x [yMinimum, yMaximum].
  //! @param function function of rank 2 @param xMinimum left @param xMaximum right @param yMinimum bottom
  //! @param yMaximum top @param label legend label (the function's name if empty)
  GraphItem(const MATH::Function * function, float xMinimum, float xMaximum, float yMinimum, float yMaximum,
            const String & label="");

  //! @brief Number of variables of the object shown: 1 for f(x) and points (x,y), 2 for f(x,y) and points (x,y,z).
  //! @return rank
  unsigned int rank() const;
  //! @brief Whether the item is drawn as a map (rank 2). @return true if so
  bool isMap() const { return rank() == 2; }
  //! @brief Whether a fit can be made to the item (see MATH::Fitter): data points with at least one point, or a
  //! function, of rank 1 or 2. @return true if so
  bool isFittable() const;
  //!
  //! @brief Default style of the k-th item of a graph (what CapGui gives the objects it shows): line and marker
  //! colors cycling through black, red, blue, green, magenta, dark blue, brown and dark red, and markers through
  //! filled circle, square, triangles, diamond, star, cross and a few others. @param k index of the item in its graph
  //!
  void applyDefaultStyle(int k);

  Kind kind() const { return _kind; }   //!< Function or data points
  const MATH::Function   * function() const { return _function; }   //!< The function shown (kFunction), or null
  const MATH::DataPoints * data()     const { return _data; }   //!< The data points shown (kData), or null
  //! @brief Show other data points (e.g. a new version of the same object); ignored for a function. @param data data
  void setData(const MATH::DataPoints * data) { if (_kind == kData && data) _data = data; }
  //! @brief The object shown (function or data), to compare items. @return pointer
  const void * object() const { return _kind == kFunction ? (const void*)_function : (const void*)_data; }

  const String & label() const { return _label; }   //!< Label of the item in the legend
  void setLabel(const String & label) { _label = label; }   //!< Set the label of the item in the legend

  //! Curve range of a function (ignored for data).
  float xMinimum() const { return _xMinimum; }
  float xMaximum() const { return _xMaximum; }   //!< Upper end of the curve of a function
  void  setXRange(float xMinimum, float xMaximum) { _xMinimum = xMinimum; _xMaximum = xMaximum; }   //!< Set the range of the curve of a function
  //! Range in y of a function of rank 2 (ignored otherwise).
  float yMinimum() const { return _yMinimum; }
  float yMaximum() const { return _yMaximum; }   //!< Upper end in y of a function of rank 2
  void  setYRange(float yMinimum, float yMaximum) { _yMinimum = yMinimum; _yMaximum = yMaximum; }   //!< Set the range in y of a function of rank 2
  //! Number of points at which a function is evaluated to draw its curve.
  int   nSamples() const { return _nSamples; }
  void  setNSamples(int n) { _nSamples = n < 2 ? 2 : n; }   //!< Set the number of points of the curve (at least 2)

  //! @name Maps (rank 2)
  //! @{

  //! @brief Whether the color map is drawn. @return true if so
  bool showColorMap() const { return _showColorMap; }
  //! @brief Draw the color map or not. @param v draw it
  void setShowColorMap(bool v) { _showColorMap = v; }
  //! @brief Number of cells along x and along y where a function of rank 2 is evaluated. @return number
  int  mapSamples() const { return _mapSamples; }
  //! @brief Set the number of cells along each axis where a function of rank 2 is evaluated. @param n number (2 to 1000)
  void setMapSamples(int n) { _mapSamples = n < 2 ? 2 : (n > 1000 ? 1000 : n); }
  //!
  //! @brief The cells and values drawn for an item of rank 2 (see Grid2D). A function is evaluated over the part of
  //! its range inside the given rectangle; the grid is kept until the rectangle, the scales or the number of
  //! samples change. @param xLow left @param xHigh right @param yLow bottom @param yHigh top (visible range)
  //! @param logX logarithmic x axis @param logY logarithmic y axis @param samples cells along each axis of a
  //! function (0: mapSamples())
  //! @return grid, or nullptr for an item of rank 1, scattered points, or a function outside the rectangle
  //!
  std::shared_ptr<const Grid2D> grid(double xLow, double xHigh, double yLow, double yHigh, bool logX, bool logY, int samples=0) const;

  //! @brief Whether contour lines are drawn. @return true if so
  bool showContours() const { return _showContours; }
  //! @brief Draw contour lines or not. @param v draw them
  void setShowContours(bool v) { _showContours = v; }
  //! @brief Number of equal intervals of the z range bounded by the automatic contour levels (n-1 levels).
  //! @return number of intervals
  int  contourIntervals() const { return _contourIntervals; }
  //! @brief Set the number of intervals of the automatic contour levels. @param n number (2 to 100)
  void setContourIntervals(int n) { _contourIntervals = n < 2 ? 2 : (n > 100 ? 100 : n); }
  //! @brief Contour levels given explicitly (empty: automatic, see contourIntervals()). @return levels
  const std::vector<double> & contourLevels() const { return _contourLevels; }
  //! @brief Set the contour levels (empty: automatic). @param levels levels
  void setContourLevels(const std::vector<double> & levels) { _contourLevels = levels; }
  //! @brief Line of the contours. @return line attributes
  LineAttributes & contourLineAttributes() { return _contourLine; }
  const LineAttributes & contourLineAttributes() const { return _contourLine; }   //!< Line of the contours
  //! @brief Whether each contour line takes the color of its level in the palette. @return true if so
  bool contourUsesPalette() const { return _contourUsesPalette; }
  //! @brief Color the contour lines with the palette, or with their line color. @param v use the palette
  void setContourUsesPalette(bool v) { _contourUsesPalette = v; }

  //! @brief Whether the values are written in the cells (where they fit). @return true if so
  bool showValues() const { return _showValues; }
  //! @brief Write the values in the cells or not. @param v write them
  void setShowValues(bool v) { _showValues = v; }
  //! @brief printf format of the values written in the cells. @return format, e.g. "%.3g"
  const String & valueFormat() const { return _valueFormat; }
  //! @brief Set the printf format of the values written in the cells. @param format format with one %g, %f or %e
  void setValueFormat(const String & format) { _valueFormat = format; }
  //! @brief Size of the values written in the cells, in points (0: automatic). @return size
  float valuePointSize() const { return _valuePointSize; }
  //! @brief Set the size of the values written in the cells. @param size size in points (0: automatic)
  void setValuePointSize(float size) { _valuePointSize = size < 0 ? 0 : size; }
  //! @brief ROOT color of the values written in the cells (-1: black or white, whichever stands out on the cell).
  //! @return color
  int  valueColor() const { return _valueColor; }
  //! @brief Set the color of the values written in the cells. @param color ROOT color index (-1: automatic)
  void setValueColor(int color) { _valueColor = color; }

  //! @brief Whether the points (nodes of a grid, bins of a histogram) are also drawn as colored disks; scattered
  //! points are always. @return true if so
  bool showMapPoints() const { return _showMapPoints; }
  //! @brief Draw the points of a map as colored disks or not. @param v draw them
  void setShowMapPoints(bool v) { _showMapPoints = v; }
  //! @brief Whether the area of the disks is proportional to the magnitude of the values. @return true if so
  bool pointSizeByValue() const { return _pointSizeByValue; }
  //! @brief Size the disks by the values or not. @param v size by value
  void setPointSizeByValue(bool v) { _pointSizeByValue = v; }
  //! @brief Whether the disks have a thin dark outline. @return true if so
  bool pointOutline() const { return _pointOutline; }
  //! @brief Outline the disks or not. @param v outline them
  void setPointOutline(bool v) { _pointOutline = v; }
  //! @}

  //! @name 3D views (rank 2)
  //! @{

  //! @brief How a map is drawn in a 3D view: a shaded surface colored by the palette, a mesh (wire frame with the
  //! hidden lines removed), or lego boxes (one per cell, from the bottom of the z range or from zero).
  enum Style3D { kSurface, kMesh, kLego };
  //! @brief How the item is drawn in a 3D view. @return style
  Style3D style3D() const { return _style3D; }
  //! @brief Set how the item is drawn in a 3D view. @param style style
  void setStyle3D(Style3D style) { _style3D = style; }
  //! @brief Whether the lines of the cells are drawn on a surface or on lego boxes. @return true if so
  bool showSurfaceLines() const { return _showSurfaceLines; }
  //! @brief Draw the lines of the cells on a surface or on lego boxes, or not. @param v draw them
  void setShowSurfaceLines(bool v) { _showSurfaceLines = v; }
  //! @brief Number of cells along each axis where a function of rank 2 is evaluated for a 3D view. @return number
  int  surfaceSamples() const { return _surfaceSamples; }
  //! @brief Set the number of cells along each axis of a function in a 3D view. @param n number (4 to 200)
  void setSurfaceSamples(int n) { _surfaceSamples = n < 4 ? 4 : (n > 200 ? 200 : n); }
  //! @}

  LineAttributes   & lineAttributes()   { return _line; }   //!< Line of the curve, or through the data points
  MarkerAttributes & markerAttributes() { return _marker; }   //!< Markers of the data points (their color is that of the error bars)
  const LineAttributes   & lineAttributes()   const { return _line; }   //!< Line of the curve, or through the data points
  const MarkerAttributes & markerAttributes() const { return _marker; }   //!< Markers of the data points

  //! What is drawn: line through the points (functions: the curve), markers, error bars of the statistical
  //! uncertainties (and of the x uncertainties), systematic uncertainties (drawn as set by sysStyle()).
  bool showLine()      const { return _showLine; }
  bool showMarkers()   const { return _showMarkers; }   //!< Whether markers are drawn
  bool showErrors()    const { return _showErrors; }   //!< Whether the statistical (and x) error bars are drawn
  bool showSysErrors() const { return _showSysErrors; }   //!< Whether the systematic uncertainties are drawn
  void setShowLine(bool v)      { _showLine = v; }   //!< Draw the line or not
  void setShowMarkers(bool v)   { _showMarkers = v; }   //!< Draw the markers or not
  void setShowErrors(bool v)    { _showErrors = v; }   //!< Draw the error bars or not
  void setShowSysErrors(bool v) { _showSysErrors = v; }   //!< Draw the systematic uncertainties or not

  //! @name Systematic uncertainties
  //! @{
  SysStyle sysStyle() const { return _sysStyle; }   //!< How the systematic uncertainties are drawn
  void setSysStyle(SysStyle style) { _sysStyle = style; }   //!< Set how the systematic uncertainties are drawn
  //! @brief Fill and outline of the boxes or band. Unless sysUsesMarkerColor() is false, their colors are those of
  //! the markers (the alpha of the fill is kept).
  FillAttributes & sysFillAttributes() { return _sysFill; }
  LineAttributes & sysLineAttributes() { return _sysLine; }   //!< Outline of the boxes or band of the systematic uncertainties
  const FillAttributes & sysFillAttributes() const { return _sysFill; }   //!< Fill of the boxes or band
  const LineAttributes & sysLineAttributes() const { return _sysLine; }   //!< Outline of the boxes or band
  bool sysUsesMarkerColor() const { return _sysUsesMarkerColor; }   //!< Whether the boxes or band take the color of the markers
  void setSysUsesMarkerColor(bool v) { _sysUsesMarkerColor = v; }   //!< Use the color of the markers for the boxes or band, or their own
  //! @brief Half width of the boxes and brackets of points without x uncertainty, as a fraction of the frame width.
  float sysWidth() const { return _sysWidth; }
  void  setSysWidth(float width) { _sysWidth = width < 0 ? 0 : width; }   //!< Set the half width of the boxes (fraction of the frame width, at least 0)
  //! @}

  //! @name Fits
  //!
  //! The fits attached to the object shown (MATH::DataPoints::addFit(), MATH::Function::addFit()) are drawn over the
  //! range of each fit (or over the whole range of the graph, see extendFits()), each with its own line, and listed
  //! in the legend under the item: fits of rank 1 of items of rank 1 as curves; fits of rank 2 of maps as contour
  //! lines (at the contour levels of the map, see Graph::contourLevels()) in a plane view, and as wire meshes in a 3D
  //! view. Fits are indexed as in the object; a fit is drawn only on an item of its rank (see Graph::drawnFit()).
  //! @{

  //! @brief Number of fits attached to the object shown. @return number
  unsigned int nFits() const;
  //! @brief Fit k of the object shown. @param k index @return fit, or nullptr if there is none
  const MATH::FitRecord * fit(unsigned int k) const;
  //! @brief Whether the fits of the item are drawn (see also Graph::showFits()). @return true if so
  bool showFits() const { return _showFits; }
  //! @brief Draw the fits of the item or not. @param v draw them
  void setShowFits(bool v) { _showFits = v; }
  //! @brief Whether fit k is drawn (when the fits are). @param k index @return true if so (the default)
  bool isFitShown(unsigned int k) const { return k >= _fitHidden.size() || !_fitHidden[k]; }
  //! @brief Draw fit k or not. @param k index @param v draw it
  void setFitShown(unsigned int k, bool v);
  //! @brief Line of the curve of fit k: the one set, or defaultFitLine(k). @param k index @return line
  LineAttributes fitLineAttributes(unsigned int k) const;
  //! @brief Set the line of the curve of fit k. @param k index @param line line
  void setFitLineAttributes(unsigned int k, const LineAttributes & line);
  //! @brief Default line of fit k: solid, 2 points wide, in a color of a cycle of 8 (red, blue, green, magenta, ...).
  //! @param k index @return line
  static LineAttributes defaultFitLine(unsigned int k);
  //! @brief Whether the curves of the fits extend over the whole x range of the graph, rather than the range of
  //! each fit. @return true if so
  bool extendFits() const { return _extendFits; }
  //! @brief Draw the curves of the fits over the whole x range of the graph, or over the range of each fit. @param v extend them
  void setExtendFits(bool v) { _extendFits = v; }
  //!
  //! @brief Quality of a fit as a short text: "χ²/ndf = 12.3/10" for a chi square or a binned (Poisson)
  //! likelihood, "-2 ln L = 45.6" for an unbinned likelihood. @param fit fit @return text (UTF-8)
  //!
  static String fitSummary(const MATH::FitRecord & fit);
  //! @}

  protected:

  Kind _kind;   //!< function or data points
  const MATH::Function   * _function;   //!< function shown (not owned)
  const MATH::DataPoints * _data;   //!< data points shown (not owned)
  String _label;   //!< label in the legend
  float  _xMinimum;   //!< lower end of the curve of a function
  float  _xMaximum;   //!< upper end of the curve of a function
  float  _yMinimum;   //!< lower end in y of a function of rank 2
  float  _yMaximum;   //!< upper end in y of a function of rank 2
  int    _nSamples;   //!< number of points of the curve
  int    _mapSamples;   //!< cells along each axis where a function of rank 2 is evaluated
  bool   _showColorMap;   //!< the color map of an item of rank 2 is drawn
  mutable std::shared_ptr<const Grid2D> _grid;   //!< grid of the last drawing (kept while it is valid)
  mutable std::vector<double> _gridKey;   //!< what the grid was made for (object, range, scales, samples)
  bool   _showContours;   //!< contour lines are drawn
  int    _contourIntervals;   //!< intervals of the automatic contour levels
  std::vector<double> _contourLevels;   //!< explicit contour levels (empty: automatic)
  LineAttributes _contourLine;   //!< line of the contours
  bool   _contourUsesPalette;   //!< contour lines colored by the palette
  bool   _showValues;   //!< values written in the cells
  String _valueFormat;   //!< printf format of the values
  float  _valuePointSize;   //!< size of the values in points (0: automatic)
  int    _valueColor;   //!< color of the values (-1: automatic)
  bool   _showMapPoints;   //!< points of a map drawn as colored disks
  bool   _pointSizeByValue;   //!< disk area proportional to the values
  bool   _pointOutline;   //!< disks outlined
  Style3D _style3D;   //!< how the item is drawn in a 3D view
  bool   _showSurfaceLines;   //!< lines of the cells drawn in a 3D view
  int    _surfaceSamples;   //!< cells along each axis of a function in a 3D view
  LineAttributes   _line;   //!< line of the curve or through the points
  MarkerAttributes _marker;   //!< markers of the data points
  bool   _showLine;   //!< the line is drawn
  bool   _showMarkers;   //!< the markers are drawn
  bool   _showErrors;   //!< the error bars are drawn
  bool   _showSysErrors;   //!< the systematic uncertainties are drawn
  SysStyle _sysStyle;   //!< boxes, brackets or band
  FillAttributes _sysFill;   //!< fill of the boxes or band
  LineAttributes _sysLine;   //!< outline of the boxes or band
  bool   _sysUsesMarkerColor;   //!< the boxes or band take the color of the markers
  float  _sysWidth;   //!< half width of the boxes of points without x uncertainty (fraction of the frame width)
  bool   _showFits = true;   //!< the fits of the object are drawn
  bool   _extendFits = false;   //!< the curves of the fits cover the whole x range
  std::vector<bool> _fitHidden;   //!< fits not drawn (by index)
  std::vector<LineAttributes> _fitLines;   //!< lines of the fits (by index)
  std::vector<bool> _fitLineSet;   //!< lines set (the others are defaultFitLine())
  };

  //!
  //! @brief A plot: a frame with an x and a y axis, showing functions and data points (GraphItem).
  //!
  //! A graph occupies a cell of its canvas (setCell(), in normalized canvas coordinates); the frame is the cell
  //! minus the margins (fractions of the cell), which hold the axis labels and the title. Each axis is linear or
  //! logarithmic (setLogX(), setLogY()): unlike ROOT, the scale belongs to the graph, not to the canvas. Ranges are
  //! automatic (covering all the items, rounded to nice values) unless set with setXRange() / setYRange().
  //!
  //! Items are added with add() or replace() (clear the graph, then add). The graph does not own the objects shown.
  //! A graph without items is not drawn at all (no frame, axes or title).
  //! dimension() is k2D, or k3D for a 3D view of the maps: a box with the x, y and z axes, seen from the direction
  //! given by azimuth() and elevation() (orthographic projection), in which each item of rank 2 is drawn as a
  //! surface, a mesh or lego boxes (GraphItem::style3D()); items of rank 1 are not drawn in a 3D view.
  //!
  //! Items of rank 2 (functions f(x,y), histograms of rank 2, points with a value z) are drawn as color maps: the
  //! graph then has a third axis, z, drawn along a color scale on the right of the frame (showColorScale()), whose
  //! range is automatic (the values in view, rounded to nice values, and including zero for lego boxes in a 3D
  //! view; symmetric about zero with setSymmetricZ())
  //! unless set with setZRange(), linear or logarithmic (setLogZ()), and whose colors come from palette(). Items of
  //! rank 1 and 2 can be shown together (e.g. a curve over a map).
  //!
  //! The title, the axis titles and the legend have automatic positions, which can be changed by the program or
  //! with the mouse: setTitleOffset(), Axis::setTitleOffset() (fractions of the cell), setLegendPosition()
  //! (fractions of the frame). Text sizes are automatic (following the size of the cell) unless set in points with
  //! setTitlePointSize(), setLegendPointSize(), Axis::setLabelPointSize() and Axis::setTitlePointSize().
  //!
  //! Each drawing records where the parts of the graph went (titleRect(), legendRect(), Axis::titleRect(),
  //! Axis::bodyRect(), the positions of the data points); pick() uses them to tell what is at a canvas position.
  //!
  class Graph : public Drawable2D
  {
  public:

  //! @brief Plane view (k2D: curves, and maps as color maps), or 3D view of the maps (k3D)
  enum Dimension { k2D, k3D };

  //! @brief Ranges and scales actually used to draw the graph (automatic or user ranges).
  struct View
  {
  float xMinimum;   //!< ranges of the axes
  float xMaximum;   //!< ranges of the axes
  float yMinimum;   //!< ranges of the axes
  float yMaximum;   //!< ranges of the axes
  bool  logX;   //!< logarithmic axes
  bool  logY;   //!< logarithmic axes
  float zMinimum = 0.0f;   //!< range of the values of the maps (when hasZ)
  float zMaximum = 1.0f;   //!< range of the values of the maps (when hasZ)
  bool  logZ = false;      //!< logarithmic z scale
  bool  hasZ = false;      //!< the graph shows maps (items of rank 2)
  //! @brief Lower end of axis k (0 x, 1 y, 2 z). @param k axis @return minimum
  float minimum(int k) const { return k == 0 ? xMinimum : (k == 1 ? yMinimum : zMinimum); }
  //! @brief Upper end of axis k (0 x, 1 y, 2 z). @param k axis @return maximum
  float maximum(int k) const { return k == 0 ? xMaximum : (k == 1 ? yMaximum : zMaximum); }
  //! @brief Whether axis k (0 x, 1 y, 2 z) is logarithmic. @param k axis @return true if so
  bool  log(int k) const { return k == 0 ? logX : (k == 1 ? logY : logZ); }
  };

  //! @brief Parts of a graph, as found by pick() (kResiduals: the panel of the residuals of the fits).
  enum Part { kNothing, kCell, kFrame, kTitle, kLegend, kAxis, kAxisTitle, kDataPoint, kResiduals };
  //! @brief What the panel of the residuals shows: data - fit, or the pulls (data - fit)/(statistical uncertainty).
  enum ResidualMode { kDifference, kPull };

  //! @brief What is at a canvas position: the part, and for an axis its index (0 x, 1 y; 2 z for the 3D graphs),
  //! for a data point the index of the item and of the point.
  struct Pick
  {
  Part part  = kNothing;   //!< part found
  int  axis  = -1;   //!< index of the axis (kAxis, kAxisTitle)
  int  item  = -1;   //!< index of the item (kDataPoint)
  int  index = -1;   //!< index of the data point (kDataPoint)
  };

  //! @brief Position at which a data point was drawn (canvas units).
  struct PointPosition
  {
  int     item;   //!< index of the item
  int     index;   //!< index of the point in its data
  Point2D position;   //!< canvas position where it was drawn
  };

  //! @brief Constructor: an empty graph filling the canvas, with automatic linear ranges.
  Graph();
  //! @brief Copy constructor (the objects shown are not copied, see GraphItem). @param src graph to copy
  Graph(const Graph & src);
  //! @brief Copy assignment. @param rhs graph to copy @return this graph
  Graph & operator=(const Graph & rhs);
  //! @brief Destructor.
  virtual ~Graph() { }

  //! @name Placement
  //! @{

  //! @brief Cell occupied by the graph (normalized canvas coordinates, lower-left and upper-right corners).
  void setCell(float xLow, float yLow, float xHigh, float yHigh);
  float cellXLow()  const { return _cellXLow;  }   //!< Left edge of the cell
  float cellYLow()  const { return _cellYLow;  }   //!< Bottom edge of the cell
  float cellXHigh() const { return _cellXHigh; }   //!< Right edge of the cell
  float cellYHigh() const { return _cellYHigh; }   //!< Top edge of the cell
  //! @brief Margins between the cell and the frame, as fractions of the cell size.
  Margins & margins() { return _margins; }
  const Margins & margins() const { return _margins; }   //!< Margins between the cell and the frame
  void setMargins(const Margins & margins) { _margins = margins; }   //!< Set the margins between the cell and the frame
  //! @brief Frame (plotting area) in normalized canvas coordinates.
  float frameXLow()  const;
  float frameYLow()  const;   //!< Bottom edge of the frame
  float frameXHigh() const;   //!< Right edge of the frame
  float frameYHigh() const;   //!< Top edge of the frame
  //! @brief True if the canvas point p is inside the cell of the graph (e.g. to find the target of a drop).
  bool cellContains(const Point2D & p) const;
  //! @brief True if the canvas point p is inside the frame.
  bool frameContains(const Point2D & p) const;
  //! @}

  //! @name Items
  //! @{

  //! @brief Add an item (an object already shown is not added twice). @return the item in the graph
  GraphItem & add(const GraphItem & item);
  //! @brief Remove all the items, then add item. @return the item in the graph
  GraphItem & replace(const GraphItem & item);
  //! @brief Remove the item showing object. @return true if it was found
  bool remove(const void * object);
  //!
  //! @brief Show an item as CapGui does: added (or replacing the items), with the default style of its place
  //! (GraphItem::applyDefaultStyle()); in a graph that was empty (or when replacing), the titles are taken from the
  //! object (setTitlesFrom()) and the ranges become automatic. An object already shown is not added again.
  //! @param item item @param replace remove the items shown first @return the item in the graph
  //!
  GraphItem & show(const GraphItem & item, bool replace=false);
  //!
  //! @brief Titles from the object of an item: the title of the graph is the title of the object (or its label);
  //! the axis titles are the dimension titles of a histogram (the y title for rank 2 only), empty otherwise; the
  //! z title is cleared. @param item item
  //!
  void setTitlesFrom(const GraphItem & item);
  //! @brief Remove all the items.
  void clearItems() { _items.clear(); }
  std::vector<GraphItem> & items() { return _items; }   //!< Items shown, in drawing order
  const std::vector<GraphItem> & items() const { return _items; }   //!< Items shown, in drawing order
  //! @}

  //! @name Shapes drawn over the graph (rectangles, ellipses, triangles, polygons, lines and arrows; see Shape)
  //! @{
  //! @brief Add a shape, drawn over everything else (the shapes added later over the earlier ones). @param shape shape
  //! @return the shape added
  Shape & addShape(const Shape & shape) { _shapes.push_back(shape); return _shapes.back(); }
  //! @brief Remove all the shapes.
  void clearShapes() { _shapes.clear(); }
  std::vector<Shape> & shapes() { return _shapes; }   //!< Shapes, in drawing order
  const std::vector<Shape> & shapes() const { return _shapes; }   //!< Shapes, in drawing order
  //! @brief Shape under a point of the canvas, the top one. @param p point (normalized canvas coordinates)
  //! @param toleranceX,toleranceY distance counted as near (canvas units) @return index, or -1
  int shapeAt(const Point2D & p, float toleranceX, float toleranceY) const;
  //! @}

  //! @brief Draw the graph (frame, axes, items, legend), then its shapes. @param painter painter
  void draw(Painter & painter) const override;

  //! @name Scales and ranges
  //! @{
  void setLogX(bool v) { _logX = v; }   //!< Logarithmic x axis or not
  void setLogY(bool v) { _logY = v; }   //!< Logarithmic y axis or not
  bool isLogX() const { return _logX; }   //!< Whether the x axis is logarithmic
  bool isLogY() const { return _logY; }   //!< Whether the y axis is logarithmic
  //! @brief Fixed x range (the y range stays automatic unless set too).
  void setXRange(float minimum, float maximum) { _xMinimum = minimum; _xMaximum = maximum; _autoX = false; }
  void setYRange(float minimum, float maximum) { _yMinimum = minimum; _yMaximum = maximum; _autoY = false; }   //!< Fixed y range (the x range stays automatic unless set too)
  //! @brief Back to automatic ranges (x, y and z).
  void setAutoRange() { _autoX = true; _autoY = true; _autoZ = true; }
  void setAutoX() { _autoX = true; }   //!< Back to an automatic x range
  void setAutoY() { _autoY = true; }   //!< Back to an automatic y range
  bool isAutoX() const { return _autoX; }   //!< Whether the x range is automatic
  bool isAutoY() const { return _autoY; }   //!< Whether the y range is automatic
  //! @brief Whether the range of axis k (0 x, 1 y, 2 z) is automatic. @param k axis @return true if so
  bool isAuto(int k) const { return k == 0 ? _autoX : (k == 1 ? _autoY : _autoZ); }
  //! @brief Back to an automatic range for axis k (0 x, 1 y, 2 z). @param k axis
  void setAuto(int k) { if (k == 0) _autoX = true; else if (k == 1) _autoY = true; else _autoZ = true; }
  //! @brief Fixed range of axis k (0 x, 1 y, 2 z). @param k axis @param minimum lower end @param maximum upper end
  void setRange(int k, float minimum, float maximum)
  { if (k == 0) setXRange(minimum, maximum); else if (k == 1) setYRange(minimum, maximum); else setZRange(minimum, maximum); }
  //! @brief Whether axis k (0 x, 1 y, 2 z) is logarithmic. @param k axis @return true if so
  bool isLog(int k) const { return k == 0 ? _logX : (k == 1 ? _logY : _logZ); }
  //! @brief Logarithmic axis k (0 x, 1 y, 2 z) or not. @param k axis @param v logarithmic
  void setLog(int k, bool v) { if (k == 0) _logX = v; else if (k == 1) _logY = v; else _logZ = v; }
  //! @brief Ranges and scales used for drawing: automatic ranges cover all the items (rounded to nice values,
  //! except the x and y ranges of a graph showing only maps, which are those of the maps; on a log axis, the
  //! positive values only). @return view
  View view() const;
  //! @brief Number of major tick intervals that falls on nice values (1, 2, 5 x 10^k) for a linear axis whose
  //! range was rounded by view(). @param minimum lower end @param maximum upper end @return intervals (>= 1)
  static int niceTickCount(double minimum, double maximum);
  //! @brief Canvas position of the data point (x,y) for the given view.
  Point2D toCanvas(const View & view, double x, double y) const;
  //! @brief Data coordinates of a canvas point for the given view (inverse of toCanvas()).
  void fromCanvas(const View & view, const Point2D & p, double & x, double & y) const;
  //!
  //! @brief Zoom on the rectangle between two canvas points (as dragging in CapGui): the x range, the y range, or
  //! both, are set to the data coordinates of the rectangle in the current view. @param a corner @param b opposite
  //! corner @param x set the x range @param y set the y range @return false if the rectangle is empty along an axis set
  //!
  bool zoom(const Point2D & a, const Point2D & b, bool x=true, bool y=true);
  //!
  //! @brief Value of the maps at a point (what CapGui shows under the mouse): of the last map drawn that has a value
  //! there. @param view view @param x abscissa @param y ordinate @param z set to the value @param item set to the
  //! index of the item @return false if no map has a value there
  //!
  bool valueAt(const View & view, double x, double y, double & z, int & item) const;
  //! @}

  //! @name Maps: values (z), palette and color scale
  //! @{

  //! @brief Whether the graph shows items of rank 2 (maps), and so has a z axis. @return true if so
  bool hasMaps() const;
  void setLogZ(bool v) { _logZ = v; }   //!< Logarithmic z scale or not
  bool isLogZ() const { return _logZ; }   //!< Whether the z scale is logarithmic
  //! @brief Fixed z range (values outside it take the color of the nearest end).
  void setZRange(float minimum, float maximum) { _zMinimum = minimum; _zMaximum = maximum; _autoZ = false; }
  void setAutoZ() { _autoZ = true; }   //!< Back to an automatic z range
  bool isAutoZ() const { return _autoZ; }   //!< Whether the z range is automatic
  //! @brief Automatic z range symmetric about zero (e.g. with the diverging palette, for values of both signs).
  void setSymmetricZ(bool v) { _symmetricZ = v; }
  bool isSymmetricZ() const { return _symmetricZ; }   //!< Whether the automatic z range is symmetric about zero
  //! @brief Colors of the values. @return palette
  Palette palette() const { return Palette(Palette::Type(_paletteType), _paletteSteps); }
  //! @brief Set the colors of the values. @param palette palette
  void setPalette(const Palette & palette) { _paletteType = int(palette.type()); _paletteSteps = palette.nSteps(); }
  bool showColorScale() const { return _showColorScale; }   //!< Whether the color scale is shown (when the graph shows maps)
  void setShowColorScale(bool v) { _showColorScale = v; }   //!< Show or hide the color scale
  //! @brief Fraction of the cell width taken on the right of the frame by the color scale and its labels (0 when
  //! it is not shown). @return fraction
  float colorScaleWidth() const;
  //! @brief Color of a value for the given view (palette, z range and scale). @param view view @param z value
  //! @return color (values outside the range take the color of the nearest end)
  Color colorOf(const View & view, double z) const;
  //! @brief Contour levels of an item for the given view: its explicit levels, or the bounds of its
  //! contourIntervals() equal intervals of the z range (in log10 on a logarithmic z scale), ends excluded.
  //! @param view view @param item item @return levels, increasing
  std::vector<double> contourLevels(const View & view, const GraphItem & item) const;
  //! @}

  //! @name Appearance
  //! @{
  Dimension dimension() const { return _dimension; }   //!< 2D or 3D
  void setDimension(Dimension dimension) { _dimension = dimension; }   //!< Set 2D or 3D
  //! @brief Whether the graph is drawn as a 3D view (k3D, with maps to show). @return true if so
  bool is3D() const { return _dimension == k3D && hasMaps(); }
  //! @brief Direction from which a 3D view is seen: angle about the vertical (z) axis, in degrees (0: the x axis
  //! runs to the right). @return azimuth
  float azimuth() const { return _azimuth; }
  //! @brief Height of the eye above the x-y plane of a 3D view, in degrees (90: from above). @return elevation
  float elevation() const { return _elevation; }
  //! @brief Set the direction of a 3D view. @param azimuth degrees (any, taken modulo 360) @param elevation degrees
  //! (clamped to [-90, 90])
  void setViewAngles(float azimuth, float elevation);
  //! @brief Back to the default direction of a 3D view (azimuth 30, elevation 30).
  void resetViewAngles() { setViewAngles(30.0f, 30.0f); }
  //! Axis templates: title, ticks, line and text attributes (ranges and scales are set from the graph).
  Axis & xAxis() { return _xAxis; }
  Axis & yAxis() { return _yAxis; }   //!< Template of the y axis
  const Axis & xAxis() const { return _xAxis; }   //!< Template of the x axis
  const Axis & yAxis() const { return _yAxis; }   //!< Template of the y axis
  Axis & zAxis() { return _zAxis; }   //!< Template of the z axis (the axis of the color scale)
  const Axis & zAxis() const { return _zAxis; }   //!< Template of the z axis
  //! @brief Number of axes: 3 when the graph shows maps (the z axis of the color scale), 2 otherwise. @return number
  int nAxes() const { return hasMaps() ? 3 : 2; }
  Axis & axis(int k) { return k == 0 ? _xAxis : (k == 1 ? _yAxis : _zAxis); }   //!< Axis k: 0 x, 1 y, 2 z
  const Axis & axis(int k) const { return k == 0 ? _xAxis : (k == 1 ? _yAxis : _zAxis); }   //!< Axis k: 0 x, 1 y, 2 z
  FillAttributes & frameFill() { return _frameFill; }   //!< Background of the frame
  LineAttributes & frameLine() { return _frameLine; }   //!< Outline of the frame
  const FillAttributes & frameFill() const { return _frameFill; }   //!< Background of the frame
  const LineAttributes & frameLine() const { return _frameLine; }   //!< Outline of the frame
  TextAttributes & titleAttributes() { return _titleAttributes; }   //!< Color and font of the title
  const TextAttributes & titleAttributes() const { return _titleAttributes; }   //!< Color and font of the title
  bool showLegend() const { return _showLegend; }   //!< Whether the legend is shown
  void setShowLegend(bool v) { _showLegend = v; }   //!< Show or hide the legend
  //! @brief Whether the fits of the items are drawn (each item can also hide its own, GraphItem::showFits()).
  bool showFits() const { return _showFits; }
  void setShowFits(bool v) { _showFits = v; }   //!< Draw the fits of the items or not
  //! @brief Whether the legend gives the quality of each fit (GraphItem::fitSummary()) after its label.
  bool showFitStatistics() const { return _showFitStatistics; }
  void setShowFitStatistics(bool v) { _showFitStatistics = v; }   //!< Give the quality of the fits in the legend or not
  //! @brief Whether the graph draws fits. @param rank only fits of this rank (1 or 2; 0: any) @return true if so
  bool hasFits(unsigned int rank=0) const;
  //!
  //! @brief Fit k of an item, if it is drawn: the fits are shown by the graph and the item, fit k is shown
  //! (GraphItem::isFitShown()), and its rank is that of the item (1 or 2). @param item item @param k index of the fit
  //! @return fit, or nullptr if it is not drawn
  //!
  const MATH::FitRecord * drawnFit(const GraphItem & item, unsigned int k) const;
  //!
  //! @brief Show a map of residuals (MATH::residualMap()) as such: the diverging palette (blue below zero, red above),
  //! continuous, with an automatic z range symmetric about zero, and the z axis titled "pull" or "data - fit".
  //! @param pulls the map holds pulls (otherwise differences)
  //!
  void setResidualMapStyle(bool pulls=true);
  //! @}

  //! @name Positions and sizes of the texts
  //! @{

  //! @brief Displacement of the title from its automatic position (centered above the frame), in fractions of the
  //! cell. @param dx horizontal displacement @param dy vertical displacement
  void  setTitleOffset(float dx, float dy) { _titleOffsetX = dx; _titleOffsetY = dy; }
  float titleOffsetX() const { return _titleOffsetX; }   //!< Horizontal displacement of the title (fraction of the cell)
  float titleOffsetY() const { return _titleOffsetY; }   //!< Vertical displacement of the title (fraction of the cell)
  //! @brief Size of the title in device units (points); 0: automatic.
  void  setTitlePointSize(float size) { _titlePointSize = size < 0 ? 0 : size; }
  float titlePointSize() const { return _titlePointSize; }   //!< Size of the title in points (0: automatic)
  //! @brief Upper left corner of the legend, in fractions of the frame ((0,0) lower left, (1,1) upper right).
  //! The default, automatic, position is the upper right corner of the frame.
  void  setLegendPosition(float x, float y) { _legendX = x; _legendY = y; _autoLegend = false; }
  void  setAutoLegendPosition() { _autoLegend = true; }   //!< Back to the automatic position (upper right corner of the frame)
  bool  isAutoLegendPosition() const { return _autoLegend; }   //!< Whether the legend is at its automatic position
  float legendX() const { return _legendX; }   //!< Left edge of the legend (fraction of the frame)
  float legendY() const { return _legendY; }   //!< Top edge of the legend (fraction of the frame)
  //! @brief Whether a text was moved from its automatic position (title, axis titles, legend). @return true if so
  bool hasMovedTexts() const;
  //! @brief Back to the automatic positions of the title, the axis titles (x, y, z) and the legend.
  void resetTextPositions();
  //! @brief Size of the legend text in device units (points); 0: automatic.
  void  setLegendPointSize(float size) { _legendPointSize = size < 0 ? 0 : size; }
  float legendPointSize() const { return _legendPointSize; }   //!< Size of the legend text in points (0: automatic)
  //! @brief Background and border of the legend (default: none).
  FillAttributes & legendFill() { return _legendFill; }
  LineAttributes & legendLine() { return _legendLine; }   //!< Border of the legend
  const FillAttributes & legendFill() const { return _legendFill; }   //!< Background of the legend
  const LineAttributes & legendLine() const { return _legendLine; }   //!< Border of the legend
  //! @}

  //! @name Layout of the last drawing
  //! @{
  const Rect2D & titleRect()  const { return _titleRect; }    //!< Where the title was drawn (empty if not)
  const Rect2D & legendRect() const { return _legendRect; }   //!< Where the legend was drawn (empty if not)
  const std::vector<PointPosition> & pointPositions() const { return _pointPositions; }   //!< Where the data points were drawn (inside the frame)
  //! Set by the renderer.
  void setDrawnLayout(const Rect2D & title, const Rect2D & legend, const std::vector<PointPosition> & points) const
  { _titleRect = title; _legendRect = legend; _pointPositions = points; }
  //! @brief What is at the canvas position p, according to the last drawing: legend, title, axis titles, data
  //! points (within toleranceX, toleranceY of p, canvas units), axes (line, ticks and labels), frame, cell. @return pick
  Pick pick(const Point2D & p, float toleranceX, float toleranceY) const;
  //! @}

  //! @name Residuals of the fits
  //!
  //! Optionally, a panel below the frame shows the residuals of the fits drawn in the graph (see GraphItem::fit()),
  //! each in the color of its fit: for data points, data - fit (with the statistical error bars) or the pulls
  //! (data - fit)/(statistical uncertainty), at the points inside the range of the fit; for a function, the curve
  //! f(x) - fit(x). The panel shares the x axis of the frame (whose labels and title then go below the panel); its
  //! y range is automatic (symmetric about zero) unless set. It is drawn when showResiduals() is set and the graph
  //! draws fits (hasFits()), in a plane view; the frame then takes the upper part of the area it had.
  //! @{

  //! @brief Whether the panel of the residuals is asked for. @return true if so
  bool showResiduals() const { return _showResiduals; }
  //! @brief Ask for the panel of the residuals, or not. @param v show it
  void setShowResiduals(bool v) { _showResiduals = v; }
  //! @brief Whether the panel of the residuals is drawn (asked for, fits drawn, plane view). @return true if so
  bool hasResidualPanel() const;
  //! @brief What the panel shows. @return data - fit, or pulls
  ResidualMode residualMode() const { return _residualMode; }
  //! @brief Set what the panel shows. @param mode data - fit, or pulls
  void setResidualMode(ResidualMode mode) { _residualMode = mode; }
  //! @brief Height of the panel, as a fraction of the area of the frame and the panel. @return fraction
  float residualFraction() const { return _residualFraction; }
  //! @brief Set the height of the panel. @param fraction fraction of the area of the frame and the panel (0.1 to 0.6)
  void setResidualFraction(float fraction) { _residualFraction = std::min(0.6f, std::max(0.1f, fraction)); }
  //! @brief Bottom of the panel (normalized canvas coordinates). @return bottom
  float residualYLow() const;
  //! @brief Top of the panel (below the frame, by residualGap()). @return top
  float residualYHigh() const;
  //! @brief Space between the panel and the frame (5% of the cell height), which keeps the labels of their y axes
  //! apart. @return space (normalized canvas coordinates)
  float residualGap() const;
  //! @brief Whether a canvas point is inside the panel (when it is drawn). @param p point @return true if so
  bool residualContains(const Point2D & p) const;
  //! @brief Fixed y range of the panel. @param minimum lower end @param maximum upper end
  void setResidualRange(float minimum, float maximum) { _residualMinimum = minimum; _residualMaximum = maximum; _autoResidual = false; }
  //! @brief Back to an automatic y range of the panel.
  void setAutoResidualRange() { _autoResidual = true; }
  //! @brief Whether the y range of the panel is automatic. @return true if so
  bool isAutoResidualRange() const { return _autoResidual; }
  //!
  //! @brief Y range of the panel: the fixed one, or the residuals (with their error bars) in the x range of the view,
  //! rounded to nice values, symmetric about zero. @param view view @param low lower end @param high upper end
  //!
  void residualRange(const View & view, double & low, double & high) const;
  //! @brief Canvas position of a residual. @param view view @param low lower end of the y range of the panel
  //! @param high upper end @param x abscissa @param r residual @return position
  Point2D toResidualCanvas(const View & view, double low, double high, double x, double r) const;
  //! @brief Abscissa and residual of a canvas point of the panel (inverse of toResidualCanvas(), with the range of
  //! residualRange()). @param view view @param p canvas point @param x set to the abscissa @param r set to the residual
  void fromResidualCanvas(const View & view, const Point2D & p, double & x, double & r) const;
  //! @brief Template of the y axis of the panel (title: "data - fit" or "pull" when empty). @return axis
  Axis & residualAxis() { return _residualAxis; }
  const Axis & residualAxis() const { return _residualAxis; }   //!< Template of the y axis of the panel
  //!
  //! @brief Residual of a data point to a fit, as drawn in the panel.
  //! @param item item of data points @param fit fit of the item @param index index of the point @param mode mode
  //! @param x set to the abscissa @param r set to the residual @param error set to its error bar (0 for a pull)
  //! @return false if there is none (point outside the range of the fit, unless GraphItem::extendFits(); no
  //! uncertainty for a pull; value not finite)
  //!
  static bool residualAt(const GraphItem & item, const MATH::FitRecord & fit, unsigned int index, ResidualMode mode,
                         double & x, double & r, double & error);
  //! @}

  //! @brief Back to an empty graph with default settings.
  virtual void reset();

  protected:

  float _cellXLow;   //!< cell of the graph (normalized canvas coordinates)
  float _cellYLow;   //!< cell of the graph (normalized canvas coordinates)
  float _cellXHigh;   //!< cell of the graph (normalized canvas coordinates)
  float _cellYHigh;   //!< cell of the graph (normalized canvas coordinates)
  Margins _margins;   //!< margins between the cell and the frame
  std::vector<GraphItem> _items; //!<! items shown (not streamed: the objects shown are not owned)
  std::vector<Shape> _shapes; //!<! shapes drawn over the graph
  bool  _logX;   //!< logarithmic axes
  bool  _logY;   //!< logarithmic axes
  bool  _autoX;   //!< automatic ranges
  bool  _autoY;   //!< automatic ranges
  float _xMinimum;   //!< fixed ranges (when not automatic)
  float _xMaximum;   //!< fixed ranges (when not automatic)
  float _yMinimum;   //!< fixed ranges (when not automatic)
  float _yMaximum;   //!< fixed ranges (when not automatic)
  Dimension _dimension;   //!< 2D or 3D
  float _azimuth;   //!< direction of a 3D view: angle about z (degrees)
  float _elevation;   //!< direction of a 3D view: height of the eye (degrees)
  Axis _xAxis;   //!< template of the x axis
  Axis _yAxis;   //!< template of the y axis
  Axis _zAxis;   //!< template of the z axis (color scale)
  bool  _logZ;   //!< logarithmic z scale
  bool  _autoZ;   //!< automatic z range
  float _zMinimum;   //!< fixed z range (when not automatic)
  float _zMaximum;   //!< fixed z range (when not automatic)
  bool  _symmetricZ;   //!< automatic z range symmetric about zero
  int   _paletteType;   //!< palette (Palette::Type)
  int   _paletteSteps;   //!< bands of the palette (0: continuous)
  bool  _showColorScale;   //!< the color scale is shown
  FillAttributes _frameFill;   //!< background of the frame
  LineAttributes _frameLine;   //!< outline of the frame
  TextAttributes _titleAttributes;   //!< color and font of the title
  bool _showLegend;   //!< the legend is shown
  bool _showFits = true;   //!< the fits of the items are drawn
  bool _showFitStatistics = true;   //!< the legend gives the quality of the fits
  bool _showResiduals = false;   //!< the panel of the residuals is asked for
  ResidualMode _residualMode = kDifference;   //!< data - fit, or pulls
  float _residualFraction = 0.3f;   //!< height of the panel (fraction of the area of the frame and the panel)
  bool  _autoResidual = true;   //!< automatic y range of the panel
  float _residualMinimum = -1.0f;   //!< fixed y range of the panel
  float _residualMaximum = 1.0f;   //!< fixed y range of the panel
  Axis  _residualAxis;   //!< template of the y axis of the panel
  float _titleOffsetX;   //!< displacement of the title (fractions of the cell)
  float _titleOffsetY;   //!< displacement of the title (fractions of the cell)
  float _titlePointSize;   //!< size of the title in points (0: automatic)
  bool  _autoLegend;   //!< the legend is at its automatic position
  float _legendX;   //!< upper left corner of the legend (fractions of the frame)
  float _legendY;   //!< upper left corner of the legend (fractions of the frame)
  float _legendPointSize;   //!< size of the legend text in points (0: automatic)
  FillAttributes _legendFill;   //!< background of the legend
  LineAttributes _legendLine;   //!< border of the legend
  mutable Rect2D _titleRect;                          //!<! where the title was drawn (not streamed)
  mutable Rect2D _legendRect;                         //!<! where the legend was drawn (not streamed)
  mutable std::vector<PointPosition> _pointPositions; //!<! where the data points were drawn (not streamed)

  ClassDef(Graph,0)
  };

  //! @brief Round [low,high] outward to multiples of a 1, 2, 5 x 10^k step (about 5 intervals); step is set.
  void niceLinear(double & low, double & high, double & step);

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Graph
