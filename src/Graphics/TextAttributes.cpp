#include "TextAttributes.hpp"

ClassImp(CAP::GRAPHICS::TextAttributes);

namespace CAP
{
  namespace GRAPHICS
  {
  TextAttributes::TextAttributes()
  :
  Attributes(),
  _font(42),
  _size(0.04),
  _width(0.0),
  _inset(0.0),
  _wrap(false),
  _clip(false),
  _justification(kLeft)
  { }

  TextAttributes::TextAttributes(const TextAttributes & src)
  :
  Attributes(src),
  _font(src._font),
  _size(src._size),
  _width(src._width),
  _inset(src._inset),
  _wrap(src._wrap),
  _clip(src._clip),
  _justification(src._justification)
  { }

  TextAttributes  & TextAttributes::operator=(const TextAttributes & rhs)
  {
  if (this != &rhs)
    {
    Attributes::operator=(rhs);
    _font  = rhs._font;
    _size  = rhs._size;
    _width = rhs._width;
    _inset = rhs._inset;
    _wrap  = rhs._wrap;
    _clip  = rhs._clip;
    _justification = rhs._justification;
    }
  return *this;
  }

  void TextAttributes::reset()
  {
  const TextAttributes & d = *defaultTextAttributes();
  _color         = d._color;
  _style         = d._style;
  _alpha         = d._alpha;
  _font          = d._font;
  _size          = d._size;
  _width         = d._width;
  _inset         = d._inset;
  _wrap          = d._wrap;
  _clip          = d._clip;
  _justification = d._justification;
  }

  TextAttributes * TextAttributes::_defaultTextAttributes = nullptr;

  TextAttributes * TextAttributes::defaultTextAttributes()
  {
  if (!_defaultTextAttributes) _defaultTextAttributes = new TextAttributes();
  return _defaultTextAttributes;
  }

  void TextAttributes::setDefaultTextAttributes(const TextAttributes & attributes)
  {
  if (!_defaultTextAttributes) _defaultTextAttributes = new TextAttributes();
  *_defaultTextAttributes = attributes;
  }



  } // namespace GRAPHICS
} // namespace CAP
