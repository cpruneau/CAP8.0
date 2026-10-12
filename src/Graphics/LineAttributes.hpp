#ifndef CAP__LineAttributes
#define CAP__LineAttributes
#include "Attributes.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class LineAttributes : public Attributes
  {
  public:

  LineAttributes();
  LineAttributes(const LineAttributes & src);
  LineAttributes  & operator=(const LineAttributes & rhs);
  virtual ~LineAttributes() { }
  virtual void reset();
  using Attributes::set; // keep set(color, alpha, style): it leaves the width unchanged
  void set(int color, float alpha, int style, float width) { _color = color; _alpha = alpha; _style = style; _width = width; }
  void setWidth(float width) { _width = width; }
  float width() const { return _width; }

  static LineAttributes * defaultLineAttributes();
  static void setDefaultLineAttributes(int color, float alpha, int style, float width);

  protected:

  float _width;

  private:

  static LineAttributes * _defaultLineAttributes;


  ClassDef(LineAttributes,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__LineAttributes
