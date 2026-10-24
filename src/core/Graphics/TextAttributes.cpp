#include "TextAttributes.hpp"


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




  TextAttributes::FontFamily TextAttributes::fontFamily(int font)
  {
  switch (fontIndex(font))
    {
    case 1: case 2: case 3: case 13: return kSerif;
    case 8: case 9: case 10: case 11: return kMono;
    case 12: case 14: case 15: return kSymbol;
    default: return kSans;
    }
  }

  bool TextAttributes::isBold(int font)
  {
  int index = fontIndex(font);
  return index == 2 || index == 3 || index == 6 || index == 7 || index == 10 || index == 11;
  }

  bool TextAttributes::isItalic(int font)
  {
  int index = fontIndex(font);
  return index == 1 || index == 3 || index == 5 || index == 7 || index == 9 || index == 11 || index == 15;
  }

  int TextAttributes::fontCode(FontFamily family, bool bold, bool italic)
  {
  int index;
  switch (family)
    {
    case kSerif:  index = bold ? (italic ? 3 : 2) : (italic ? 1 : 13); break;
    case kMono:   index = 8 + (italic ? 1 : 0) + (bold ? 2 : 0); break;
    case kSymbol: index = italic ? 15 : 12; break;
    default:      index = 4 + (italic ? 1 : 0) + (bold ? 2 : 0); break;
    }
  return 10*index + 2;
  }

  } // namespace GRAPHICS
} // namespace CAP
