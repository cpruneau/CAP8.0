#include "Attributes.hpp"


namespace CAP
{
  namespace GRAPHICS
  {
  Attributes::Attributes()
  :
  _color(1),
  _style(1),
  _alpha(1.0)
  { }

  Attributes::Attributes(int color, float alpha, int style)
  :
  _color(color),
  _style(style),
  _alpha(alpha)
  { }

  Attributes::Attributes(const Attributes & src)
  :
  _color(src._color),
  _style(src._style),
  _alpha(src._alpha)
  { }

  Attributes  & Attributes::operator=(const Attributes & rhs)
  {
  //if (rhs != *this)
  //{
  _color  = rhs._color;
  _style  = rhs._style;
  _alpha  = rhs._alpha;
  return *this;
  }

  void Attributes::reset()
  {
  const Attributes & d = *defaultAttributes();
  _color = d._color;
  _style = d._style;
  _alpha = d._alpha;
  }

  Attributes * Attributes::_defaultAttributes = nullptr;

  Attributes * Attributes::defaultAttributes()
  {
  if (!_defaultAttributes) _defaultAttributes = new Attributes();
  return _defaultAttributes;
  }

  void Attributes::setDefaultAttributes(int color, float alpha, int style)
  {
  if (!_defaultAttributes) _defaultAttributes = new Attributes();
  _defaultAttributes->set(color, alpha, style);
  }

  } // namespace GRAPHICS
} // namespace CAP
