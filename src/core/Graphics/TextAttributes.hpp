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

  //! @name Fonts
  //! Fonts are ROOT font codes: 10 x index + precision, the index giving the family and the style
  //! (1-3, 13: Times; 4-7: Helvetica; 8-11: Courier; 12, 15: Symbol). Painters map them to the fonts they have.
  //! @{
  enum FontFamily { kSans, kSerif, kMono, kSymbol };
  //! @brief ROOT font index (1 to 15) of a font code. @param font font code (e.g. 42) @return index (e.g. 4)
  static int  fontIndex(int font) { int index = font >= 10 ? font/10 : font; return (index < 1 || index > 15) ? 4 : index; }
  static FontFamily fontFamily(int font);
  static bool isBold(int font);
  static bool isItalic(int font);
  //! @brief ROOT font code (precision 2) of a family and style. @return code (e.g. 42, 62, 132, 82)
  static int  fontCode(FontFamily family, bool bold, bool italic);
  //! @}

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
