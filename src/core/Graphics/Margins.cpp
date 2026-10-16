#include "Margins.hpp"

ClassImp(CAP::GRAPHICS::Margins);

namespace CAP
{
  namespace GRAPHICS
  {

  float Margins::_defaultLeft   = 0.10;
  float Margins::_defaultBottom = 0.10;
  float Margins::_defaultRight  = 0.05;
  float Margins::_defaultTop    = 0.05;

  Margins::Margins()
  :
  _left(_defaultLeft),
  _bottom(_defaultBottom),
  _right(_defaultRight),
  _top(_defaultTop)
  { }

  Margins::Margins(float left, float bottom, float right, float top)
  :
  _left(left),
  _bottom(bottom),
  _right(right),
  _top(top)
  { }

  Margins::Margins(const Margins & src)
  :
  _left(src._left),
  _bottom(src._bottom),
  _right(src._right),
  _top(src._top)
  { }

  Margins  & Margins::operator=(const Margins & rhs)
  {
  if (this != &rhs)
    {
    _left   = rhs._left;
    _bottom = rhs._bottom;
    _right  = rhs._right;
    _top    = rhs._top;
    }
  return *this;
  }



  } //  namespace GRAPHICS
} // namespace CAP
