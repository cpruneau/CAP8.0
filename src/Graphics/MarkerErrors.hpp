#ifndef CAP__MarkerErrors
#define CAP__MarkerErrors
#include "Drawable2D.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Marker : public Drawable2D
  {
  public:

  Marker();
  Marker(const Marker & src);
  Marker  & operator=(const Marker & rhs);
  virtual ~Marker() { }
  virtual void set(int color, float alpha=1.0, int style=1, float size=1.0) { _color = color; _alpha = alpha; _style = style; _width = width; }
  void setWidth(float width) { _width = width; }
  float width() const { return _width; }


  virtual void draw();

  protected:

  ClassDef(Marker,0)
  };

  class MarkerVector : public Drawable2D
  {
  public:

  MarkerVector();
  MarkerVector(const MarkerVector & src);
  MarkerVector  & operator=(const MarkerVector & rhs);
  virtual ~MarkerVector() { }
  void addPoint(float x, float y) { _points.push_back(Point2D(x,y)); }
  void addPoint(const Point2D & point) { _points.push_back(point); }
  void addPoints(const std::vector<float> & x, const std::vector<float> & x);
  void addPoints(const std::vector<double> & x, const std::vector<double> & x);
  void addPoints(const std::vector<Point2D> & points) { _points = points; }
  std::vector<Point2D> & points() { return _points; }
  const std::vector<Point2D> & points() const { return _points; }
  virtual void draw();

  protected:

  std::vector<Point2D> _points;

  ClassDef(MarkerVector,0)
  };

  } // namespace GRAPHICS
}   // namespace CAP

#endif // !CAP__MarkerErrors
