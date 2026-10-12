#ifndef CAP__Canvas
#define CAP__Canvas
#include "TObject.h"
#include "Aliases.hpp"
#include "Drawable2D.hpp"
#include "FillAttributes.hpp"
#include "LineAttributes.hpp"

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
  virtual ~Canvas() { }

  void setSize(int width, int height) { _width = width; _height = height; }
  int  width()  const { return _width;  }
  int  height() const { return _height; }

  void setBackground(const FillAttributes & background) { _background = background; }
  FillAttributes & background() { return _background; }
  const FillAttributes & background() const { return _background; }

  //! Paint the background, then draw all children.
  virtual void draw(Painter & painter) const;
  virtual void reset();

  protected:

  int _width;
  int _height;
  FillAttributes _background;

  ClassDef(Canvas,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Canvas
