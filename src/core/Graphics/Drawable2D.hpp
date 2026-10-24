#ifndef CAP__Drawable2D
#define CAP__Drawable2D
#include <vector>
#include "TObject.h"
#include "String.hpp"
#include "Renderer.hpp"
#include "Point2D.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Painter;

  //!
  //! Base class of everything that can be drawn on a Canvas.
  //! Points are in normalized canvas coordinates: (0,0) lower-left, (1,1) upper-right.
  //! A drawable is drawn by its renderer, then its children are drawn in the order they were added.
  //! Children are not owned: the caller keeps them alive while they are attached.
  //!
  class Drawable2D
  {
  public:

  Drawable2D();
  Drawable2D(const Drawable2D & src);
  Drawable2D  & operator=(const Drawable2D & rhs);
  virtual ~Drawable2D() { }
  void addChild(Drawable2D * node);
  void setName(const String & name)   { _name = name; }
  void setLabel(const String & label) { _label = label; }
  void setTitle(const String & title) { _title = title; }
  void setExcluded(bool v)            { _excluded = v; }
  void setFromCenter(bool v)          { _fromCenter = v; }
  void setParent(Drawable2D * parent) { _parent = parent; }
  void setRenderer(const Renderer * renderer)  { _renderer = renderer; }
  void setPosition(const Point2D & position)  { _points.clear(); _points.push_back(position); }
  void setPosition(float x, float y)          { setPosition(Point2D(x,y)); }
  void addPoint(const Point2D & point) { _points.push_back(point); }
  void addPoint(float x, float y)      { _points.push_back(Point2D(x,y)); }
  void addPoints(const std::vector<Point2D> & points);
  void setPoints(const std::vector<Point2D> & points) { _points = points; }

  String & name() { return _name; }
  const String & name() const { return _name; }
  String & label() { return _label; }
  const String & label() const { return _label; }
  String & title() { return _title; }
  const String & title() const { return _title; }
  bool  isExcluded() const { return _excluded; }
  bool  fromCenter() const { return _fromCenter; }
  std::vector<Drawable2D*> & children() { return _children; }
  const std::vector<Drawable2D*> & children() const  { return _children; }
  Drawable2D * parent() {return _parent; }
  const Renderer *  renderer() const { return _renderer; }
  //! First point; (0,0) if the drawable has no points.
  Point2D position() const { return _points.empty() ? Point2D() : _points[0]; }
  std::vector<Point2D> & points()  { return _points; }
  const std::vector<Point2D> & points()  const  { return _points; }

  //! Draw this object with its renderer, then its children. Excluded objects (and their children) are skipped.
  virtual void draw(Painter & painter) const;
  virtual void clear();
  virtual void reset();

  protected:

  String _name;
  String _label;
  String _title;
  bool   _excluded;
  bool   _fromCenter;
  Drawable2D * _parent;
  const Renderer * _renderer;
  std::vector<Point2D> _points;
  std::vector<Drawable2D*> _children;

  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Drawable2D
