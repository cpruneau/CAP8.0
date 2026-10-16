//
// MacOsCanvasDemo.C -- shows a CAP Graphics canvas in a native macOS window.
// Built on macOS with the Graphics library and installed as <CAP>/bin/MacOsCanvasDemo: run that program; the
// window stays open until it is closed. (main() is left out when the file is read by ROOT's interpreter.)
//
#include <cmath>
#include <vector>
#include "Canvas.hpp"
#include "Axis.hpp"
#include "Line.hpp"
#include "Polygon.hpp"
#include "Marker.hpp"
#include "MarkerVector.hpp"
#include "DataPointColumns.hpp"
#include "MacOsCanvas.hpp"

using namespace CAP::GRAPHICS;

void MacOsCanvasDemo()
{
  Canvas canvas("c1", "CAP demo", 800, 600);

  // frame (outline only)
  Polygon frame;
  frame.setFromCenter(false);
  frame.setPosition(0.15, 0.15);
  frame.setSize(0.75, 0.75);
  frame.fillAttributes().setStyle(0);
  canvas.addChild(&frame);

  // axes: positions in canvas units (0..1), ranges in data units
  Axis x(Point2D(0.15,0.15), Point2D(0.90,0.15), 0.0, 10.0);
  x.setTitle("x [GeV]");
  Axis y(Point2D(0.15,0.15), Point2D(0.15,0.90), 0.0, 1.0);
  y.setTitle("y");
  y.setTickSide(-1);           // ticks point into the frame
  canvas.addChild(&x);
  canvas.addChild(&y);

  // data points y = exp(-x/3), red full circles
  std::vector<double> xs, ys;
  for (int i=0; i<=20; i++) { xs.push_back(0.5*i); ys.push_back(std::exp(-0.5*i/3.0)); }
  CAP::MATH::DataPointColumns data(xs, ys);
  MarkerVector markers(&data);
  markers.setRanges(AxisRange(0,10), AxisRange(0,1));
  markers.setFrame(0.15, 0.15, 0.90, 0.90);
  markers.markerAttributes().set(2, 1.0, 20, 1.0);   // color, alpha, style, size
  canvas.addChild(&markers);

  // the same function as a blue dashed curve
  std::vector<Point2D> curve;
  for (int i=0; i<=100; i++) { float xv = 0.1*i; curve.push_back(markers.toCanvas(xv, std::exp(-xv/3.0))); }
  Line line(curve);
  line.lineAttributes().set(4, 1.0, 2, 2.0);          // color, alpha, style (dashed), width
  canvas.addChild(&line);

  // a labelled, semi-transparent triangle and a square marker
  Polygon triangle(3);
  triangle.setPosition(0.75, 0.75);
  triangle.setSize(0.08, 0.10);
  triangle.fillAttributes().set(3, 0.6, 1001);
  triangle.setText("n=3");
  canvas.addChild(&triangle);

  Marker square;
  square.setPosition(0.6, 0.6);
  square.markerAttributes().set(1, 1.0, 21, 2.0);
  canvas.addChild(&square);

  MacOsCanvas window(&canvas);
  window.show();               // returns when the window is closed
}

#ifndef __CLING__
int main() { MacOsCanvasDemo(); return 0; }
#endif
