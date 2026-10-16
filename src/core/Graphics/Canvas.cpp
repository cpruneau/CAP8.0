#include "Canvas.hpp"
#include "Painter.hpp"

ClassImp(CAP::GRAPHICS::Canvas);

namespace CAP
{
  namespace GRAPHICS
  {

  namespace
  {
  FillAttributes whiteBackground()
  {
  FillAttributes white;
  white.set(0, 1.0, 1001); // ROOT kWhite, opaque, solid
  return white;
  }
  }

  Canvas::Canvas()
  :
  Drawable2D(),
  _width(800),
  _height(600),
  _background(whiteBackground()),
  _nColumns(0),
  _nRows(0),
  _graphs()
  {
  setName("Canvas");
  setTitle("Canvas");
  }

  Canvas::Canvas(const String & name, const String & title, int width, int height)
  :
  Drawable2D(),
  _width(width),
  _height(height),
  _background(whiteBackground()),
  _nColumns(0),
  _nRows(0),
  _graphs()
  {
  setName(name);
  setTitle(title);
  }

  Canvas::Canvas(const Canvas & src)
  :
  Drawable2D(src),
  _width(src._width),
  _height(src._height),
  _background(src._background),
  _nColumns(src._nColumns),
  _nRows(src._nRows),
  _graphs()
  {
  for (auto g : src._graphs) _graphs.push_back(new Graph(*g));
  }

  Canvas  & Canvas::operator=(const Canvas & rhs)
  {
  if (this != &rhs)
    {
    Drawable2D::operator=(rhs);
    _width      = rhs._width;
    _height     = rhs._height;
    _background = rhs._background;
    clearGraphs();
    _nColumns = rhs._nColumns;
    _nRows    = rhs._nRows;
    for (auto g : rhs._graphs) _graphs.push_back(new Graph(*g));
    }
  return *this;
  }

  void Canvas::reset()
  {
  Drawable2D::reset();
  _background = whiteBackground();
  }

  void Canvas::draw(Painter & painter) const
  {
  if (_excluded) return;
  std::vector<Point2D> corners{Point2D(0,0), Point2D(1,0), Point2D(1,1), Point2D(0,1)};
  LineAttributes noLine;
  noLine.setStyle(0);
  painter.drawPolygon(corners, _background, noLine);
  for (auto graph : _graphs) if (graph) graph->draw(painter);
  for (auto child : _children) if (child) child->draw(painter);
  }

  void Canvas::clearGraphs()
  {
  for (auto g : _graphs) delete g;
  _graphs.clear();
  }

  void Canvas::divide(int nColumns, int nRows)
  {
  if (nColumns < 1) nColumns = 1;
  if (nRows    < 1) nRows    = 1;
  size_t n = size_t(nColumns)*size_t(nRows);
  while (_graphs.size() > n) { delete _graphs.back(); _graphs.pop_back(); }
  while (_graphs.size() < n) _graphs.push_back(new Graph());
  _nColumns = nColumns;
  _nRows    = nRows;
  layout();
  }

  void Canvas::layout()
  {
  float w = 1.0f/_nColumns;
  float h = 1.0f/_nRows;
  for (int row=0; row<_nRows; row++)
    for (int column=0; column<_nColumns; column++)
      {
      float yHigh = 1.0f - row*h;   // row 0 at the top
      _graphs[row*_nColumns + column]->setCell(column*w, yHigh - h, (column+1)*w, yHigh);
      }
  }

  int Canvas::graphAt(const Point2D & p) const
  {
  for (size_t k=0; k<_graphs.size(); k++) if (_graphs[k]->cellContains(p)) return int(k);
  return -1;
  }

  } // namespace GRAPHICS
} // namespace CAP
