#ifndef CAP__TextAttributes
#define CAP__TextAttributes
#include "Attributes.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class TextAttributes : public Attributes
  {
  public:

  enum Justification { kLeft, kCenter, kRight };

  TextAttributes();
  TextAttributes(const TextAttributes & src);
  TextAttributes  & operator=(const TextAttributes & rhs);
  virtual ~TextAttributes() {  }
  virtual void reset();

  static TextAttributes * defaultTextAttributes();
  static void setDefaultTextAttributes(const TextAttributes & attributes);

  void  setFont(int font)     { _font  = font; }
  void  setSize(float size)   { _size  = size; }
  void  setWidth(float width) { _width = width; }
  void  setInset(float inset) { _inset = inset; }
  void  setWrap(bool wrap)    { _wrap  = wrap;  }
  void  setClip(bool clip)    { _clip  = clip;  }
  void  setJustification(Justification justification) { _justification = justification; }

  int   font()  const { return _font; }
  float size()  const { return _size; }
  float width() const { return _width; }
  float inset() const { return _inset; }
  bool  wrap()  const { return _wrap;  }
  bool  clip()  const { return _clip;  }
  Justification justification() const { return _justification; }

  protected:

  int   _font;
  float _size;  //! text height as a fraction of the canvas height
  float _width; //! max width of the text
  float _inset; //! size of left/right margins when used in Geom2D
  bool  _wrap;  //! change line when reaching _width
  bool  _clip;  //! clip when getting out of bounding object
  Justification _justification;

  private:

  static TextAttributes * _defaultTextAttributes;

  ClassDef(TextAttributes,0)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__TextAttributes
