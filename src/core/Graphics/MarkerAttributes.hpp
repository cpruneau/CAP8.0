#ifndef CAP__MarkerAttributes
#define CAP__MarkerAttributes
#include "Attributes.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class MarkerAttributes : public Attributes
  {
  public:

  MarkerAttributes();
  MarkerAttributes(const MarkerAttributes & src);
  MarkerAttributes  & operator=(const MarkerAttributes & rhs);
  virtual ~MarkerAttributes() {  }
  virtual void reset();
  using Attributes::set; // keep set(color, alpha, style): it leaves the size unchanged
  void set(int color, float alpha, int style, float size) { _color = color; _alpha = alpha; _style = style; _size = size; }
  void setSize(float size) { _size = size; }
  float size() const { return _size; }

  static MarkerAttributes * defaultMarkerAttributes();
  static void setDefaultMarkerAttributes(int color, float alpha, int style, float size);

  protected:

  float _size;

  private:

  static MarkerAttributes * _defaultMarkerAttributes;

  ClassDef(MarkerAttributes,0)
  };

  }  // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__MarkerAttributes
