#ifndef CAP__Canvas
#define CAP__Canvas
#include "TObject.h"
#include "Aliases.hpp"
#include "Drawable2D.hpp"
#include "FillAttributes.hpp"
#include "LineAttributes.hpp"
#include "Graph.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Painter;

  //!
  //! Top-level drawing area. Elements (Line, Polygon, Marker, MarkerVector, Axis, ...) are
  //! attached with addChild() and drawn in that order, on top of the background.
  //! Canvas is platform-independent: a Painter backend (e.g. MacOsCanvas) displays it.
  //! width() and height() are the requested size in device units (points/pixels); element
  //! coordinates are normalized, (0,0) lower-left to (1,1) upper-right.
  //!
  class Canvas : public Drawable2D
  {
  public:

  Canvas();
  Canvas(const String & name, const String & title, int width=800, int height=600);
  Canvas(const Canvas & src);
  Canvas  & operator=(const Canvas & rhs);
  virtual ~Canvas() { clearGraphs(); }

  void setSize(int width, int height) { _width = width; _height = height; }
  int  width()  const { return _width;  }
  int  height() const { return _height; }

  void setBackground(const FillAttributes & background) { _background = background; }
  FillAttributes & background() { return _background; }
  const FillAttributes & background() const { return _background; }

  //! @name Grid of graphs
  //! @{
  //! @brief Divide the canvas into nColumns x nRows cells, one Graph per cell (owned by the canvas). Existing graphs
  //! are kept, in order (row by row from the top left), and moved to their new cells; graphs beyond the new count
  //! are deleted. A new canvas has no graphs (only its children are drawn) until divide() is called.
  void divide(int nColumns, int nRows);
  int nColumns() const { return _nColumns; }
  int nRows()    const { return _nRows; }
  int nGraphs()  const { return (int)_graphs.size(); }
  //! @brief Graph k, row by row from the top left. @param k index in [0,nGraphs())
  Graph & graph(int k) { return *_graphs.at(k); }
  const Graph & graph(int k) const { return *_graphs.at(k); }
  //! @brief Graph in column column and row row (row 0 at the top).
  Graph & graph(int column, int row) { return *_graphs.at(row*_nColumns + column); }
  //! @brief Index of the graph whose cell contains the canvas point p, or -1.
  int graphAt(const Point2D & p) const;
  //! @brief Divide the canvas into the most square grid holding n graphs: ceil(sqrt(n)) columns, and as many rows as
  //! needed. @param n number of graphs (at least 1)
  void divideFor(int n);
  //!
  //! @brief Show the items one per graph (as CapGui shows the objects of a folder): the canvas is divided for their
  //! number (divideFor()), all its graphs are cleared, and item k is shown in graph k (Graph::show(), replacing).
  //! @param items items (functions, data points)
  //!
  void showOnePerGraph(const std::vector<GraphItem> & items);
  //! @}

  //! @name SVG files
  //! @{

  //!
  //! @brief Write the canvas to an SVG file, its texts typeset by LaTeX where they ask for it (see LatexRenderer).
  //! @param fileName file @param width width in points (0: width()) @param height height in points (0: height())
  //! @return false if the file cannot be written
  //!
  bool writeSvg(const String & fileName, int width=0, int height=0) const;
  //!
  //! @brief Write several canvases to one SVG page, in a grid of the given number of columns (row by row from the
  //! top left), each width x height points. @param fileName file @param canvases canvases @param columns columns of
  //! the page (at least 1) @param width width of each canvas @param height height of each canvas
  //! @return false if there is no canvas or the file cannot be written
  //!
  static bool writeSvg(const String & fileName, const std::vector<const Canvas*> & canvases, int columns, int width, int height);
  //! @}

  //! Paint the background, then draw the graphs and the other children.
  virtual void draw(Painter & painter) const;
  virtual void reset();

  protected:

  int _width;
  int _height;
  FillAttributes _background;
  int _nColumns;
  int _nRows;
  std::vector<Graph*> _graphs; //! owned; not streamed

  void layout();
  void clearGraphs();

  ClassDef(Canvas,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Canvas
