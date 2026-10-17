#include "Painter.hpp"
#include "LatexRenderer.hpp"
#include "TROOT.h"
#include "TColor.h"

namespace CAP
{
  namespace GRAPHICS
  {

  float Painter::textWidth(const String & text, const TextAttributes & attributes) const
  {
  const LatexImage * image = latexImage(text);
  if (image && image->ok) return image->width*attributes.size()*height()/LatexImage::kFontSize;
  return plainTextWidth(LatexRenderer::plainText(text), attributes);   // without LaTeX: drawn without the $
  }

  float Painter::plainTextWidth(const String & text, const TextAttributes & attributes) const
  {
  int n = 0;
  for (const char * c = text.data(); *c; c++) if ((*c & 0xC0) != 0x80) n++; // count UTF-8 characters
  return 0.55*attributes.size()*height()*n;
  }

  const LatexImage * Painter::latexImage(const String & text)
  {
  return LatexRenderer::instance().image(text);
  }

  void Painter::rgb(int color, float & red, float & green, float & blue)
  {
  if (gROOT)
    {
    TColor * c = gROOT->GetColor(color);
    if (c)
      {
      red   = c->GetRed();
      green = c->GetGreen();
      blue  = c->GetBlue();
      return;
      }
    }
  // ROOT's basic colors 0-9 (kWhite, kBlack, kRed, kGreen, kBlue, kYellow, kMagenta, kCyan, ...)
  static const float table[10][3] =
    {
    {1.00f, 1.00f, 1.00f}, {0.00f, 0.00f, 0.00f}, {1.00f, 0.00f, 0.00f}, {0.00f, 1.00f, 0.00f},
    {0.00f, 0.00f, 1.00f}, {1.00f, 1.00f, 0.00f}, {1.00f, 0.00f, 1.00f}, {0.00f, 1.00f, 1.00f},
    {0.35f, 0.83f, 0.33f}, {0.35f, 0.33f, 0.85f}
    };
  int k = (color >= 0 && color < 10) ? color : 1; // unknown colors fall back to black
  red   = table[k][0];
  green = table[k][1];
  blue  = table[k][2];
  }

  } // namespace GRAPHICS
} // namespace CAP
