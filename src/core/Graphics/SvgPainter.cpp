/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SvgPainter.cpp
//! @brief Implementation of SvgPainter.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include "SvgPainter.hpp"
#include "LatexRenderer.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  namespace { const double kPi = 3.14159265358979323846; }

  SvgPainter::SvgPainter(float width, float height)
  :
  Painter(),
  _pageWidth(width), _pageHeight(height),
  _regionX(0), _regionY(0), _regionWidth(width), _regionHeight(height),
  _body()
  {
  _body.setf(std::ios::fixed);
  _body.precision(2);
  }

  void SvgPainter::beginRegion(float x, float y, float width, float height)
  {
  _regionX = x; _regionY = y; _regionWidth = width; _regionHeight = height;
  }

  void SvgPainter::endRegion()
  {
  _regionX = 0; _regionY = 0; _regionWidth = _pageWidth; _regionHeight = _pageHeight;
  }

  String SvgPainter::color(int color)
  {
  float r, g, b;
  Painter::rgb(color, r, g, b);
  char text[16];
  std::snprintf(text, sizeof(text), "#%02x%02x%02x", int(255*r+0.5), int(255*g+0.5), int(255*b+0.5));
  return String(text);
  }

  String SvgPainter::color(const Color & color)
  {
  auto byte = [](float v) { return v <= 0 ? 0 : (v >= 1 ? 255 : int(255*v + 0.5f)); };
  char text[16];
  std::snprintf(text, sizeof(text), "#%02x%02x%02x", byte(color.red), byte(color.green), byte(color.blue));
  return String(text);
  }

  void SvgPainter::fillPolygon(const std::vector<Point2D> & points, const Color & c)
  {
  if (points.size() < 3) return;
  _body << "<polygon points=\"";
  for (const auto & p : points) _body << px(p) << "," << py(p) << " ";
  _body << "\" fill=\"" << color(c).data() << "\"";
  if (c.alpha < 1) _body << " fill-opacity=\"" << c.alpha << "\"";
  _body << "/>\n";
  }

  void SvgPainter::fillRectangle(float x0, float y0, float x1, float y1, const Color & c)
  {
  float left = std::min(px(Point2D(x0, 0)), px(Point2D(x1, 0))), right  = std::max(px(Point2D(x0, 0)), px(Point2D(x1, 0)));
  float top  = std::min(py(Point2D(0, y0)), py(Point2D(0, y1))), bottom = std::max(py(Point2D(0, y0)), py(Point2D(0, y1)));
  _body << "<rect x=\"" << left << "\" y=\"" << top << "\" width=\"" << right - left << "\" height=\"" << bottom - top
        << "\" fill=\"" << color(c).data() << "\"";
  if (c.alpha < 1) _body << " fill-opacity=\"" << c.alpha << "\"";
  _body << " shape-rendering=\"crispEdges\"/>\n";
  }

  String SvgPainter::dashArray(int style, float width)
  {
  float w = width > 0 ? width : 1.0f;
  char text[64];
  switch (style)
    {
    case 2:  std::snprintf(text, sizeof(text), "%.1f,%.1f", 6*w, 4*w); break;                 // dashed
    case 3:  std::snprintf(text, sizeof(text), "%.1f,%.1f", 1.5*w, 3*w); break;               // dotted
    case 4:  std::snprintf(text, sizeof(text), "%.1f,%.1f,%.1f,%.1f", 8*w, 3*w, 1.5*w, 3*w); break; // dash-dot
    default: return String("");
    }
  return String(text);
  }

  String SvgPainter::escape(const String & text)
  {
  String out;
  for (const char * c = text.data(); *c; c++)
    {
    switch (*c)
      {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;";  break;
      case '>': out += "&gt;";  break;
      case '"': out += "&quot;"; break;
      default:  out += *c;
      }
    }
  return out;
  }

  void SvgPainter::drawLine(const std::vector<Point2D> & points, const LineAttributes & line)
  {
  if (points.size() < 2 || line.style() == 0) return;
  _body << "<polyline fill=\"none\" stroke=\"" << color(line.color()).data() << "\" stroke-opacity=\"" << line.alpha()
        << "\" stroke-width=\"" << line.width() << "\" stroke-linejoin=\"round\"";
  String dash = dashArray(line.style(), line.width());
  if (!dash.isNull()) _body << " stroke-dasharray=\"" << dash.data() << "\"";
  _body << " points=\"";
  for (const auto & p : points) _body << px(p) << "," << py(p) << " ";
  _body << "\"/>\n";
  }

  void SvgPainter::drawPolygon(const std::vector<Point2D> & points, const FillAttributes & fill, const LineAttributes & line)
  {
  if (points.size() < 3) return;
  bool doFill = fill.style() != 0, doStroke = line.style() != 0;
  if (!doFill && !doStroke) return;
  _body << "<polygon";
  if (doFill) _body << " fill=\"" << color(fill.color()).data() << "\" fill-opacity=\"" << fill.alpha() << "\"";
  else        _body << " fill=\"none\"";
  if (doStroke)
    {
    _body << " stroke=\"" << color(line.color()).data() << "\" stroke-opacity=\"" << line.alpha()
          << "\" stroke-width=\"" << line.width() << "\"";
    String dash = dashArray(line.style(), line.width());
    if (!dash.isNull()) _body << " stroke-dasharray=\"" << dash.data() << "\"";
    }
  _body << " points=\"";
  for (const auto & p : points) _body << px(p) << "," << py(p) << " ";
  _body << "\"/>\n";
  }

  void SvgPainter::drawMarker(const Point2D & position, const MarkerAttributes & marker)
  {
  float s = 4.0f*marker.size();   // half size, as MacOsCanvas: size 1 is 8 points across
  if (s <= 0) return;
  float x = px(position), y = py(position);
  int style = marker.style();
  bool open = (style == 4 || style == 24 || style == 25 || style == 26 || style == 27 || style == 30 || style == 32);
  String c = color(marker.color());
  char paint[160];
  if (open) std::snprintf(paint, sizeof(paint), "fill=\"none\" stroke=\"%s\" stroke-opacity=\"%.2f\" stroke-width=\"1\"", c.data(), marker.alpha());
  else      std::snprintf(paint, sizeof(paint), "fill=\"%s\" fill-opacity=\"%.2f\" stroke=\"none\"", c.data(), marker.alpha());
  auto polygon = [&](std::initializer_list<std::pair<float,float>> pts)
    {
    _body << "<polygon " << paint << " points=\"";
    for (const auto & p : pts) _body << x + p.first << "," << y - p.second << " ";
    _body << "\"/>\n";
    };
  switch (style)
    {
    case 21: case 25: polygon({{-s,-s},{s,-s},{s,s},{-s,s}}); break;
    case 22: case 26: polygon({{-s,-s},{s,-s},{0,s}}); break;
    case 23: case 32: polygon({{-s,s},{s,s},{0,-s}}); break;
    case 33: case 27: polygon({{0,-s},{s,0},{0,s},{-s,0}}); break;
    case 29: case 30:
      {
      _body << "<polygon " << paint << " points=\"";
      for (int k=0; k<10; k++)
        {
        double phi = kPi/2 + k*kPi/5, r = (k%2 == 0) ? s : 0.4*s;
        _body << x + r*std::cos(phi) << "," << y - r*std::sin(phi) << " ";
        }
      _body << "\"/>\n";
      break;
      }
    case 2: case 3: case 5:
      {
      _body << "<path fill=\"none\" stroke=\"" << c.data() << "\" stroke-width=\"1\" d=\"";
      if (style != 5) _body << "M" << x-s << "," << y << "H" << x+s << "M" << x << "," << y-s << "V" << y+s;
      if (style != 2)
        {
        float d = (style == 3) ? 0.7f*s : s;
        _body << "M" << x-d << "," << y-d << "L" << x+d << "," << y+d << "M" << x-d << "," << y+d << "L" << x+d << "," << y-d;
        }
      _body << "\"/>\n";
      break;
      }
    case 1:  _body << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"0.75\" " << paint << "/>\n"; break;
    case 6:  _body << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"1\" " << paint << "/>\n"; break;
    case 7:  _body << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"1.5\" " << paint << "/>\n"; break;
    default: _body << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"" << s << "\" " << paint << "/>\n"; break;
    }
  }

  void SvgPainter::drawText(const Point2D & position, const String & text, const TextAttributes & attributes, float angle)
  {
  if (text.isNull()) return;
  float size = attributes.size()*_regionHeight;
  if (size <= 0) return;
  float x = px(position), y = py(position);
  const LatexImage * image = latexImage(text);
  if (image && image->ok)
    {
    // the paths typeset by LaTeX, scaled to the size, with the ink box centered vertically on position
    float scale = size/LatexImage::kFontSize, w = image->width*scale;
    float dx = attributes.justification() == TextAttributes::kCenter ? -0.5f*w
             : attributes.justification() == TextAttributes::kRight  ? -w : 0.0f;
    float dy = 0.5f*(image->height - image->depth)*scale;
    _body << "<g fill=\"" << color(attributes.color()).data() << "\" fill-opacity=\"" << attributes.alpha()
          << "\" transform=\"translate(" << x << " " << y << ")";
    if (angle != 0) _body << " rotate(" << -angle << ")";
    _body.precision(5);
    _body << " translate(" << dx << " " << dy << ") scale(" << scale << ") translate(" << -image->x << " 0)\">";
    _body.precision(2);
    _body << image->svg << "</g>\n";
    return;
    }
  const char * anchor = attributes.justification() == TextAttributes::kCenter ? "middle"
                      : attributes.justification() == TextAttributes::kRight  ? "end" : "start";
  const int font = attributes.font();
  const char * family = TextAttributes::fontFamily(font) == TextAttributes::kSerif ? "Times New Roman, Times, serif"
                      : TextAttributes::fontFamily(font) == TextAttributes::kMono  ? "Courier New, Courier, monospace"
                      : TextAttributes::fontFamily(font) == TextAttributes::kSymbol ? "Symbol"
                      : "Helvetica, Arial, sans-serif";
  _body << "<text x=\"" << x << "\" y=\"" << y << "\" font-family=\"" << family << "\" font-size=\"" << size << "\"";
  if (TextAttributes::isBold(font))   _body << " font-weight=\"bold\"";
  if (TextAttributes::isItalic(font)) _body << " font-style=\"italic\"";
  _body << " fill=\"" << color(attributes.color()).data() << "\" fill-opacity=\"" << attributes.alpha()
        << "\" text-anchor=\"" << anchor << "\" dominant-baseline=\"central\"";
  if (angle != 0) _body << " transform=\"rotate(" << -angle << " " << x << " " << y << ")\"";
  _body << ">" << escape(LatexRenderer::plainText(text)).data() << "</text>\n";
  }

  String SvgPainter::document() const
  {
  std::ostringstream out;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"" << _pageWidth << "pt\" height=\"" << _pageHeight
      << "pt\" viewBox=\"0 0 " << _pageWidth << " " << _pageHeight << "\">\n"
      << _body.str()
      << "</svg>\n";
  return String(out.str());
  }

  bool SvgPainter::write(const String & fileName) const
  {
  std::ofstream file(fileName.data());
  if (!file) return false;
  file << document().data();
  return bool(file);
  }

  } // namespace GRAPHICS
} // namespace CAP
