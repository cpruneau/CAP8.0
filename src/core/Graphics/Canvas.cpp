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
  _background(whiteBackground())
  {
  setName("Canvas");
  setTitle("Canvas");
  }

  Canvas::Canvas(const String & name, const String & title, int width, int height)
  :
  Drawable2D(),
  _width(width),
  _height(height),
  _background(whiteBackground())
  {
  setName(name);
  setTitle(title);
  }

  Canvas::Canvas(const Canvas & src)
  :
  Drawable2D(src),
  _width(src._width),
  _height(src._height),
  _background(src._background)
  { }

  Canvas  & Canvas::operator=(const Canvas & rhs)
  {
  if (this != &rhs)
    {
    Drawable2D::operator=(rhs);
    _width      = rhs._width;
    _height     = rhs._height;
    _background = rhs._background;
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
  for (auto child : _children) if (child) child->draw(painter);
  }

  } // namespace GRAPHICS
} // namespace CAP
