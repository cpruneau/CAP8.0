#ifndef CAP__Geom2D
#define CAP__Geom2D
#include "String.hpp"
#include "Drawable2D.hpp"
#include "FillAttributes.hpp"
#include "LineAttributes.hpp"
#include "TextAttributes.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Geom2D : public Drawable2D
  {
  public:

  Geom2D();
  Geom2D(const Geom2D & src);
  Geom2D  & operator=(const Geom2D & rhs);
  virtual ~Geom2D() { }
  virtual void reset();

  //! Size in normalized canvas units; the shape is placed at position() (its center if fromCenter(), else its lower-left corner).
  void setSize(float width, float height) { _width = width; _height = height; }
  void setWidth(float width)   { _width  = width;  }
  void setHeight(float height) { _height = height; }
  //! Rotation in degrees, counter-clockwise, around the shape's center.
  void setAngle(float angle)   { _angle  = angle;  }
  //! Text drawn at the center of the shape (empty: no text).
  void setText(const String & text) { _text = text; }

  float width()  const { return _width;  }
  float height() const { return _height; }
  float angle()  const { return _angle;  }
  const String & text() const { return _text; }

  void setFillAttributes(const FillAttributes & fillAttributes) { _fillAttributes = fillAttributes; }
  void setLineAttributes(const LineAttributes & lineAttributes) { _lineAttributes = lineAttributes; }
  void setTextAttributes(const TextAttributes & textAttributes) { _textAttributes = textAttributes; }

  FillAttributes & fillAttributes() { return _fillAttributes; }
  LineAttributes & lineAttributes() { return _lineAttributes; }
  TextAttributes & textAttributes() { return _textAttributes; }
  const FillAttributes & fillAttributes() const { return _fillAttributes; }
  const LineAttributes & lineAttributes() const { return _lineAttributes; }
  const TextAttributes & textAttributes() const { return _textAttributes; }


  protected:

  float  _width;
  float  _height;
  float  _angle;
  String _text;

  FillAttributes _fillAttributes;
  LineAttributes _lineAttributes;
  TextAttributes _textAttributes;

  ClassDef(Geom2D,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Geom2D
