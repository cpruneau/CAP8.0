#ifndef CAP__Attributes
#define CAP__Attributes
#include "TObject.h"
#include "ClassDef.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Attributes
  {
  public:

  Attributes();
  Attributes(int color, float alpha=1.0, int style=1);
  Attributes(const Attributes & src);
  Attributes  & operator=(const Attributes & rhs);
  virtual ~Attributes() { }

  virtual void set(int color, float alpha=1.0, int style=1) { _color = color; _alpha = alpha; _style = style; }
  void setColor(int color, float alpha=1.0) { _color = color; _alpha = alpha; }
  void setStyle(int style) { _style = style; }
  void setAlpha(float alpha) { _alpha = alpha; }

  int color() const { return _color; }
  int style() const { return _style; }
  float alpha() const { return _alpha; }

  //! Restore color, alpha and style from the default attributes object.
  virtual void reset();

  static Attributes * defaultAttributes();
  static void setDefaultAttributes(int color, float alpha, int style);

  protected:

  int   _color;
  int   _style; 
  float _alpha; // transparency

  private:

  static Attributes * _defaultAttributes;

  ClassDef(Attributes,0)
  };



  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Attributes
