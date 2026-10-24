#include "Drawable2D.hpp"


namespace CAP
{
  namespace GRAPHICS
  {

  Drawable2D::Drawable2D()
  :
  _name(""),
  _label(""),
  _title(""),
  _excluded(false),
  _fromCenter(true),
  _parent(nullptr),
  _renderer(nullptr),
  _points(),
  _children()
  {
  _points.push_back(Point2D());
  }

  Drawable2D::Drawable2D(const Drawable2D & src)
  :
  _name(src._name),
  _label(src._label),
  _title(src._title),
  _excluded(src._excluded),
  _fromCenter(src._fromCenter),
  _parent(src._parent),
  _renderer(src._renderer),
  _points(src._points),
  _children(src._children)
  { }

  Drawable2D  & Drawable2D::operator=(const Drawable2D & rhs)
  {
  if (this != &rhs)
    {
    _name       = rhs._name;
    _label      = rhs._label;
    _title      = rhs._title;
    _excluded   = rhs._excluded;
    _fromCenter = rhs._fromCenter;
    _parent     = rhs._parent;
    _renderer   = rhs._renderer;
    _points     = rhs._points;
    _children   = rhs._children;
    }
  return *this;
  }

  //!
  //! Remove everything: name, label, title, points, renderer and children (children are detached, not deleted).
  //!
  void Drawable2D::clear()
  {
  _name      = "";
  _label     = "";
  _title     = "";
  _excluded  = false;
  _fromCenter = true;
  _parent    = nullptr;
  _renderer  = nullptr;
  _points.clear();
  for (auto child : _children) child->setParent(nullptr);
  _children.clear();
  }

  //!
  //! Restore the default flags of this object and its children; points and children are kept.
  //!
  void Drawable2D::reset()
  {
  _excluded   = false;
  _fromCenter = true;
  for (auto child : _children) child->reset();
  }

  void Drawable2D::addChild(Drawable2D * node)
  {
  if (!node) return;
  node->setParent(this);
  _children.push_back(node);
  }

  void Drawable2D::addPoints(const std::vector<Point2D> & points)
  {
  for (const auto & point : points) _points.push_back(point);
  }

  void Drawable2D::draw(Painter & painter) const
  {
  if (_excluded) return;
  if (_renderer) _renderer->draw(*this, painter);
  for (auto child : _children) if (child) child->draw(painter);
  }

  } // namespace GRAPHICS
} // namespace CAP
