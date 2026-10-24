#include <algorithm>
#include <cmath>
#include "Painter.hpp"
#include "LatexRenderer.hpp"
#include "ColorTable.hpp"

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

  void Painter::fillPolygon(const std::vector<Point2D> & points, const Color & color)
  {
  FillAttributes fill;
  fill.set(ColorTable::instance().index(color), color.alpha, 1001);
  LineAttributes noLine;
  noLine.setStyle(0);
  drawPolygon(points, fill, noLine);
  }

  std::vector<Point2D> Painter::hatchSegments(const std::vector<Point2D> & points, int style, float spacing) const
  {
  std::vector<Point2D> segments;
  if (points.size() < 3 || spacing <= 0 || width() <= 0 || height() <= 0) return segments;
  std::vector<float> angles;   // directions of the lines, degrees
  switch (style)
    {
    case FillAttributes::kHatchForward:    angles = { 45 }; break;
    case FillAttributes::kHatchBackward:   angles = { -45 }; break;
    case FillAttributes::kHatchVertical:   angles = { 90 }; break;
    case FillAttributes::kHatchHorizontal: angles = { 0 }; break;
    case FillAttributes::kCrossHatch:      angles = { 45, -45 }; break;
    case FillAttributes::kGrid:            angles = { 0, 90 }; break;
    default: return segments;
    }
  const float W = width(), H = height();
  for (float angle : angles)
    {
    // device coordinates rotated so that the lines are horizontal: v = const
    float c = std::cos(angle*float(M_PI)/180.0f), s = std::sin(angle*float(M_PI)/180.0f);
    std::vector<float> u(points.size()), v(points.size());
    float vMin = 1e30f, vMax = -1e30f;
    for (size_t k=0; k<points.size(); k++)
      {
      float x = points[k].x()*W, y = points[k].y()*H;
      u[k] =  c*x + s*y;
      v[k] = -s*x + c*y;
      vMin = std::min(vMin, v[k]); vMax = std::max(vMax, v[k]);
      }
    // lines on a grid fixed on the canvas (adjacent shapes match)
    for (float line = std::ceil(vMin/spacing)*spacing; line <= vMax; line += spacing)
      {
      std::vector<float> crossings;
      for (size_t k=0; k<points.size(); k++)
        {
        size_t j = (k + 1) % points.size();
        float v0 = v[k], v1 = v[j];
        if ((v0 <= line && v1 > line) || (v1 <= line && v0 > line))
          crossings.push_back(u[k] + (line - v0)*(u[j] - u[k])/(v1 - v0));
        }
      std::sort(crossings.begin(), crossings.end());
      for (size_t k=0; k+1<crossings.size(); k+=2)
        for (float uu : { crossings[k], crossings[k+1] })
          segments.push_back(Point2D((c*uu - s*line)/W, (s*uu + c*line)/H));
      }
    }
  return segments;
  }

  void Painter::drawShape(const std::vector<Point2D> & points, const FillAttributes & fill, const LineAttributes & line)
  {
  if (!FillAttributes::isPattern(fill.style())) { drawPolygon(points, fill, line); return; }
  LineAttributes hatch;
  hatch.set(fill.color(), fill.alpha(), 1, 1.0f);
  std::vector<Point2D> segments = hatchSegments(points, fill.style());
  for (size_t k=0; k+1<segments.size(); k+=2) drawLine({ segments[k], segments[k+1] }, hatch);
  FillAttributes hollow(fill);
  hollow.setStyle(FillAttributes::kHollow);
  drawPolygon(points, hollow, line);
  }

  void Painter::fillRectangle(float x0, float y0, float x1, float y1, const Color & color)
  {
  fillPolygon({ Point2D(x0, y0), Point2D(x1, y0), Point2D(x1, y1), Point2D(x0, y1) }, color);
  }

  const LatexImage * Painter::latexImage(const String & text)
  {
  return LatexRenderer::instance().image(text);
  }

  void Painter::rgb(int color, float & red, float & green, float & blue)
  {
  ColorTable::instance().rgb(color, red, green, blue);
  }

  } // namespace GRAPHICS
} // namespace CAP
