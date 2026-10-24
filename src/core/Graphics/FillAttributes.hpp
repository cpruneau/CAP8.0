#ifndef CAP__FillAttributes
#define CAP__FillAttributes
#include "Attributes.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! @brief Fill of a closed shape: color, opacity and style (ROOT-like codes).
  //!
  //! Styles: 0 hollow, 1001 (or any value that is not a pattern) solid, and the hatch patterns 3004 (////), 3005 (\\\\),
  //! 3006 (vertical lines), 3007 (horizontal lines), 3013 (both diagonals) and 3010 (grid). The patterns are drawn by
  //! Painter::drawShape() with lines of the fill color, so that every painter (screen, SVG, PDF) shows them.
  //!
  class FillAttributes : public Attributes
  {
  public:

  //! Fill styles
  enum Style { kHollow = 0, kSolid = 1001, kHatchForward = 3004, kHatchBackward = 3005, kHatchVertical = 3006,
               kHatchHorizontal = 3007, kGrid = 3010, kCrossHatch = 3013 };

  //! @brief Whether a style is a hatch pattern. @param style style @return true for the patterns listed above
  static bool isPattern(int style)
  { return style == kHatchForward || style == kHatchBackward || style == kHatchVertical || style == kHatchHorizontal
           || style == kGrid || style == kCrossHatch; }

  FillAttributes();
  FillAttributes(const FillAttributes & src);
  FillAttributes  & operator=(const FillAttributes & rhs);
  virtual ~FillAttributes() { }
  virtual void reset();

  static FillAttributes * defaultFillAttributes();
  static void setDefaultFillAttributes(int color, float alpha, int style);

  protected:

  private:
  static FillAttributes * _defaultFillAttributes;


  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__FillAttributes
