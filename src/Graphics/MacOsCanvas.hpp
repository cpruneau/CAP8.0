#ifndef CAP__MacOsCanvas
#define CAP__MacOsCanvas
#include "Painter.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Canvas;

  //!
  //! macOS backend: shows a Canvas in a native window (AppKit), drawing with CoreGraphics
  //! and CoreText. Available on macOS only (MacOsCanvas.mm).
  //!
  //! Usage:
  //!   Canvas canvas("c1", "My plot", 800, 600);
  //!   canvas.addChild(&xAxis); ...
  //!   MacOsCanvas window(&canvas);
  //!   window.show();   // blocks until the window is closed (close button, Cmd-W or Cmd-Q)
  //!
  //! The window opens at the canvas' width() x height() (in points) and can be resized;
  //! the canvas is redrawn to fill it. Neither the canvas nor its elements are owned.
  //!
  class MacOsCanvas : public Painter
  {
  public:

  MacOsCanvas(Canvas * canvas=nullptr);
  virtual ~MacOsCanvas() { }

  void setCanvas(Canvas * canvas) { _canvas = canvas; }
  Canvas * canvas() { return _canvas; }

  //! Open a window showing the canvas and run the macOS event loop until the window is closed.
  void show();

  // Painter interface
  virtual float width()  const { return _width;  }
  virtual float height() const { return _height; }
  virtual void drawLine(const std::vector<Point2D> & points, const LineAttributes & line);
  virtual void drawPolygon(const std::vector<Point2D> & points, const FillAttributes & fill, const LineAttributes & line);
  virtual void drawMarker(const Point2D & position, const MarkerAttributes & marker);
  virtual void drawText(const Point2D & position, const String & text, const TextAttributes & attributes, float angle=0.0);
  virtual float textWidth(const String & text, const TextAttributes & attributes) const;

  //! Internal: called by the window's view to repaint. cgContext is a CGContextRef;
  //! width and height are the view size in points.
  void paint(void * cgContext, float width, float height);

  private:

  MacOsCanvas(const MacOsCanvas &) = delete;
  MacOsCanvas & operator=(const MacOsCanvas &) = delete;

  Canvas * _canvas;
  void *   _context;  // CGContextRef while painting, nullptr otherwise
  float    _width;
  float    _height;
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__MacOsCanvas
