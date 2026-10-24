#include "FillAttributes.hpp"


namespace CAP
{
  namespace GRAPHICS
  {

  
  FillAttributes::FillAttributes()
  :
  Attributes()
  { }

  FillAttributes::FillAttributes(const FillAttributes & src)
  :
  Attributes(src)
  { }

  FillAttributes  & FillAttributes::operator=(const FillAttributes & rhs)
  {
  Attributes::operator=(rhs);
  return *this;
  }


  void FillAttributes::reset()
  {
  const FillAttributes & d = *defaultFillAttributes();
  _color = d._color;
  _style = d._style;
  _alpha = d._alpha;
  }

  FillAttributes * FillAttributes::_defaultFillAttributes = nullptr;

  FillAttributes * FillAttributes::defaultFillAttributes()
  {
  if (!_defaultFillAttributes) _defaultFillAttributes = new FillAttributes();
  return _defaultFillAttributes;
  }

  void FillAttributes::setDefaultFillAttributes(int color, float alpha, int style)
  {
  if (!_defaultFillAttributes) _defaultFillAttributes = new FillAttributes();
  _defaultFillAttributes->set(color, alpha, style);
  }

  } // namespace GRAPHICS
} // namespace CAP
