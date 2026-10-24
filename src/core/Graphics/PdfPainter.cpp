/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PdfPainter.cpp
//! @brief Implementation of PdfPainter.
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include "PdfPainter.hpp"
#include "LatexRenderer.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  namespace
  {
  const double kPi = 3.14159265358979323846;
  const double kKappa = 0.5522847498;   // control points of a quarter circle drawn as a cubic Bezier curve

  //! The 14 standard fonts used (index of PdfPainter::fontIndex()).
  const char * const kFontNames[13] = { "Helvetica", "Helvetica-Bold", "Helvetica-Oblique", "Helvetica-BoldOblique",
                                        "Times-Roman", "Times-Bold", "Times-Italic", "Times-BoldItalic",
                                        "Courier", "Courier-Bold", "Courier-Oblique", "Courier-BoldOblique", "Symbol" };

  //! Widths (1/1000 of the size) of the characters 32-126 of the standard fonts (their font metrics).
  const short kHelvetica[95] = {
    278,278,355,556,556,889,667,191,333,333,389,584,278,333,278,278,556,556,556,556,556,556,556,556,556,556,278,278,
    584,584,584,556,1015,667,667,722,722,667,611,778,722,278,500,667,556,833,722,778,667,778,722,667,611,722,667,944,
    667,667,611,278,278,278,469,556,333,556,556,500,556,556,278,556,556,222,222,500,222,833,556,556,556,556,333,500,
    278,556,500,722,500,500,500,334,260,334,584 };
  const short kHelveticaBold[95] = {
    278,333,474,556,556,889,722,238,333,333,389,584,278,333,278,278,556,556,556,556,556,556,556,556,556,556,333,333,
    584,584,584,611,975,722,722,722,722,667,611,778,722,278,556,722,611,833,722,778,667,778,722,667,611,722,667,944,
    667,667,611,333,278,333,584,556,333,556,611,556,611,556,333,611,611,278,278,556,278,889,611,611,611,611,389,556,
    333,611,556,778,556,556,500,389,280,389,584 };
  const short kTimes[95] = {
    250,333,408,500,500,833,778,180,333,333,500,564,250,333,250,278,500,500,500,500,500,500,500,500,500,500,278,278,
    564,564,564,444,921,722,667,667,722,611,556,722,722,333,389,722,611,889,722,722,556,722,667,556,611,722,722,944,
    722,722,611,333,278,333,469,500,333,444,500,444,500,444,333,500,500,278,278,500,278,778,500,500,500,500,333,389,
    278,500,500,722,500,500,444,480,200,480,541 };

  //! Number in a PDF stream (fixed, short).
  std::string num(double v)
  {
  char text[32];
  if (std::fabs(v) < 5e-4) v = 0;
  std::snprintf(text, sizeof(text), "%.3f", v);
  // strip the trailing zeros
  std::string s(text);
  if (s.find('.') != std::string::npos)
    {
    while (!s.empty() && s.back() == '0') s.pop_back();
    if (!s.empty() && s.back() == '.') s.pop_back();
    }
  return s.empty() || s == "-0" ? "0" : s;
  }

  //! Value of an attribute of an SVG element ("" if absent).
  std::string attribute(const std::string & element, const char * name)
  {
  std::string key = std::string(" ") + name + "=";
  size_t at = element.find(key);
  if (at == std::string::npos) return "";
  at += key.size();
  if (at >= element.size()) return "";
  char quote = element[at];
  if (quote != '\'' && quote != '"') return "";
  size_t end = element.find(quote, at + 1);
  return end == std::string::npos ? "" : element.substr(at + 1, end - at - 1);
  }

  //! Numbers and commands of SVG path data.
  struct PathReader
  {
  const std::string & d;
  size_t at = 0;
  explicit PathReader(const std::string & data) : d(data) { }
  void skip() { while (at < d.size() && (std::isspace((unsigned char) d[at]) || d[at] == ',')) at++; }
  bool command(char & c)
    {
    skip();
    if (at < d.size() && std::isalpha((unsigned char) d[at]) && d[at] != 'e' && d[at] != 'E') { c = d[at++]; return true; }
    return false;
    }
  bool number(double & v)
    {
    skip();
    if (at >= d.size()) return false;
    const char * start = d.c_str() + at;
    char * end = nullptr;
    v = std::strtod(start, &end);
    if (end == start) return false;
    at += size_t(end - start);
    return true;
    }
  bool more() { skip(); return at < d.size() && !std::isalpha((unsigned char) d[at]); }
  };
  } // namespace

  PdfPainter::PdfPainter(float width, float height)
  :
  Painter(),
  _pageWidth(width), _pageHeight(height),
  _regionX(0), _regionY(0), _regionWidth(width), _regionHeight(height),
  _content(),
  _pages(),
  _alphas()
  {
  for (bool & used : _fontsUsed) used = false;
  _content << "1 j\n";                                  // round joins
  }

  void PdfPainter::newPage(float width, float height)
  {
  _pages.push_back(Page{ _content.str(), _pageWidth, _pageHeight });
  _content.str("");
  _content.clear();
  _content << "1 j\n";
  _pageWidth = width; _pageHeight = height;
  endRegion();
  }

  void PdfPainter::beginRegion(float x, float y, float width, float height)
  {
  _regionX = x; _regionY = y; _regionWidth = width; _regionHeight = height;
  }

  void PdfPainter::endRegion()
  {
  _regionX = 0; _regionY = 0; _regionWidth = _pageWidth; _regionHeight = _pageHeight;
  }

  void PdfPainter::setColor(float r, float g, float b, float alpha, bool stroke)
  {
  auto clamp = [](float v) { return v < 0 ? 0.0f : (v > 1 ? 1.0f : v); };
  _content << num(clamp(r)) << " " << num(clamp(g)) << " " << num(clamp(b)) << (stroke ? " RG\n" : " rg\n");
  int perMille = int(1000*clamp(alpha) + 0.5f);
  // one graphics state per opacity (fill and stroke together: the operators drawing next use one of them)
  auto it = _alphas.find(perMille);
  if (it == _alphas.end()) it = _alphas.emplace(perMille, "GS" + std::to_string(_alphas.size())).first;
  _content << "/" << it->second << " gs\n";
  }

  void PdfPainter::setColor(int color, float alpha, bool stroke)
  {
  float r, g, b;
  Painter::rgb(color, r, g, b);
  setColor(r, g, b, alpha, stroke);
  }

  void PdfPainter::setLine(int style, float width)
  {
  float w = width > 0 ? width : 1.0f;
  _content << num(w) << " w ";
  switch (style)
    {
    case 2:  _content << "[" << num(6*w) << " " << num(4*w) << "] 0 d\n"; break;                         // dashed
    case 3:  _content << "[" << num(1.5*w) << " " << num(3*w) << "] 0 d\n"; break;                       // dotted
    case 4:  _content << "[" << num(8*w) << " " << num(3*w) << " " << num(1.5*w) << " " << num(3*w) << "] 0 d\n"; break;
    default: _content << "[] 0 d\n";
    }
  }

  void PdfPainter::path(const std::vector<Point2D> & points, bool close)
  {
  for (size_t k=0; k<points.size(); k++)
    _content << num(px(points[k])) << " " << num(flip(py(points[k]))) << (k == 0 ? " m\n" : " l\n");
  if (close) _content << "h\n";
  }

  void PdfPainter::drawLine(const std::vector<Point2D> & points, const LineAttributes & line)
  {
  if (points.size() < 2 || line.style() == 0) return;
  setColor(line.color(), line.alpha(), true);
  setLine(line.style(), line.width());
  path(points, false);
  _content << "S\n";
  }

  void PdfPainter::drawPolygon(const std::vector<Point2D> & points, const FillAttributes & fill, const LineAttributes & line)
  {
  if (points.size() < 3) return;
  bool doFill = fill.style() != 0, doStroke = line.style() != 0;
  if (doFill)
    {
    setColor(fill.color(), fill.alpha(), false);
    path(points, true);
    _content << "f\n";
    }
  if (doStroke)
    {
    setColor(line.color(), line.alpha(), true);
    setLine(line.style(), line.width());
    path(points, true);
    _content << "S\n";
    }
  }

  void PdfPainter::fillPolygon(const std::vector<Point2D> & points, const Color & c)
  {
  if (points.size() < 3) return;
  setColor(c.red, c.green, c.blue, c.alpha, false);
  path(points, true);
  _content << "f\n";
  }

  void PdfPainter::fillRectangle(float x0, float y0, float x1, float y1, const Color & c)
  {
  float left = std::min(px(Point2D(x0, 0)), px(Point2D(x1, 0))), right  = std::max(px(Point2D(x0, 0)), px(Point2D(x1, 0)));
  float top  = std::min(py(Point2D(0, y0)), py(Point2D(0, y1))), bottom = std::max(py(Point2D(0, y0)), py(Point2D(0, y1)));
  const float overlap = 0.5f;                          // readers smooth the edges: adjacent cells overlap a little
  setColor(c.red, c.green, c.blue, c.alpha, false);
  _content << num(left - overlap) << " " << num(flip(bottom + overlap)) << " " << num(right - left + 2*overlap) << " "
           << num(bottom - top + 2*overlap) << " re f\n";
  }

  void PdfPainter::drawMarker(const Point2D & position, const MarkerAttributes & marker)
  {
  float s = 4.0f*marker.size();   // half size, as SvgPainter: size 1 is 8 points across
  if (s <= 0) return;
  float x = px(position), y = flip(py(position));        // PDF coordinates (y up)
  int style = marker.style();
  bool open = (style == 4 || style == 24 || style == 25 || style == 26 || style == 27 || style == 30 || style == 32);
  if (open) { setColor(marker.color(), marker.alpha(), true); _content << "1 w [] 0 d\n"; }
  else      setColor(marker.color(), marker.alpha(), false);
  const char * paint = open ? "s\n" : "f\n";           // s: close and stroke
  auto polygon = [&](std::initializer_list<std::pair<float,float>> pts)
    {
    bool first = true;
    for (const auto & p : pts) { _content << num(x + p.first) << " " << num(y + p.second) << (first ? " m\n" : " l\n"); first = false; }
    _content << paint;
    };
  auto circle = [&](float r)
    {
    double k = kKappa*r;
    _content << num(x + r) << " " << num(y) << " m\n"
             << num(x + r) << " " << num(y + k) << " " << num(x + k) << " " << num(y + r) << " " << num(x) << " " << num(y + r) << " c\n"
             << num(x - k) << " " << num(y + r) << " " << num(x - r) << " " << num(y + k) << " " << num(x - r) << " " << num(y) << " c\n"
             << num(x - r) << " " << num(y - k) << " " << num(x - k) << " " << num(y - r) << " " << num(x) << " " << num(y - r) << " c\n"
             << num(x + k) << " " << num(y - r) << " " << num(x + r) << " " << num(y - k) << " " << num(x + r) << " " << num(y) << " c\n"
             << paint;
    };
  switch (style)
    {
    case 21: case 25: polygon({{-s,-s},{s,-s},{s,s},{-s,s}}); break;
    case 22: case 26: polygon({{-s,-s},{s,-s},{0,s}}); break;
    case 23: case 32: polygon({{-s,s},{s,s},{0,-s}}); break;
    case 33: case 27: polygon({{0,-s},{s,0},{0,s},{-s,0}}); break;
    case 29: case 30:
      {
      for (int k=0; k<10; k++)
        {
        double phi = kPi/2 + k*kPi/5, r = (k%2 == 0) ? s : 0.4*s;
        _content << num(x + r*std::cos(phi)) << " " << num(y + r*std::sin(phi)) << (k == 0 ? " m\n" : " l\n");
        }
      _content << paint;
      break;
      }
    case 2: case 3: case 5:
      {
      setColor(marker.color(), marker.alpha(), true);
      _content << "1 w [] 0 d\n";
      if (style != 5)
        _content << num(x - s) << " " << num(y) << " m " << num(x + s) << " " << num(y) << " l "
                 << num(x) << " " << num(y - s) << " m " << num(x) << " " << num(y + s) << " l\n";
      if (style != 2)
        {
        float d = (style == 3) ? 0.7f*s : s;
        _content << num(x - d) << " " << num(y - d) << " m " << num(x + d) << " " << num(y + d) << " l "
                 << num(x - d) << " " << num(y + d) << " m " << num(x + d) << " " << num(y - d) << " l\n";
        }
      _content << "S\n";
      break;
      }
    case 1:  circle(0.75f); break;
    case 6:  circle(1.0f); break;
    case 7:  circle(1.5f); break;
    default: circle(s); break;
    }
  }

  void PdfPainter::svgPaths(const std::string & svg, double a, double b, double c, double d, double e, double f)
  {
  // page coordinates (y down) of an image point, then PDF coordinates (y up)
  auto out = [&](double x, double y, const char * op)
    {
    double X = a*x + c*y + e, Y = b*x + d*y + f;
    _content << num(X) << " " << num(flip(float(Y))) << op;
    };
  size_t at = 0;
  bool drawn = false;
  while ((at = svg.find('<', at)) != std::string::npos)
    {
    size_t end = svg.find('>', at);
    if (end == std::string::npos) break;
    std::string element = svg.substr(at, end - at + 1);
    at = end + 1;
    if (element.compare(0, 5, "<rect") == 0)
      {
      double x = std::atof(attribute(element, "x").c_str()), y = std::atof(attribute(element, "y").c_str());
      double w = std::atof(attribute(element, "width").c_str()), h = std::atof(attribute(element, "height").c_str());
      out(x, y, " m "); out(x + w, y, " l "); out(x + w, y + h, " l "); out(x, y + h, " l h\n");
      drawn = true;
      continue;
      }
    if (element.compare(0, 5, "<path") != 0) continue;
    std::string data = attribute(element, "d");
    PathReader reader(data);
    char command = 0;
    double cx = 0, cy = 0, sx = 0, sy = 0, lastX = 0, lastY = 0;   // current point, start of the subpath, last control
    char previous = 0;
    while (true)
      {
      char next;
      if (reader.command(next)) command = next;
      else if (!reader.more()) break;
      else if (command == 0) break;                    // numbers without a command: invalid data
      bool relative = std::islower((unsigned char) command);
      char upper = char(std::toupper((unsigned char) command));
      double ox = relative ? cx : 0, oy = relative ? cy : 0;
      double v[6];
      auto read = [&](int n) { for (int i=0; i<n; i++) if (!reader.number(v[i])) return false; return true; };
      if (upper == 'Z')
        {
        _content << "h\n";
        cx = sx; cy = sy;
        previous = 'Z';
        command = 0;                                   // numbers right after Z: invalid data (stop)
        continue;
        }
      if (upper == 'M')
        {
        if (!read(2)) break;
        cx = ox + v[0]; cy = oy + v[1]; sx = cx; sy = cy;
        out(cx, cy, " m\n");
        command = relative ? 'l' : 'L';                // following pairs are lines
        }
      else if (upper == 'L') { if (!read(2)) break; cx = ox + v[0]; cy = oy + v[1]; out(cx, cy, " l\n"); }
      else if (upper == 'H') { if (!read(1)) break; cx = ox + v[0]; out(cx, cy, " l\n"); }
      else if (upper == 'V') { if (!read(1)) break; cy = (relative ? cy : 0) + v[0]; out(cx, cy, " l\n"); }
      else if (upper == 'C' || upper == 'S')
        {
        double x1, y1;
        if (upper == 'C') { if (!read(6)) break; x1 = ox + v[0]; y1 = oy + v[1]; v[0] = v[2]; v[1] = v[3]; v[2] = v[4]; v[3] = v[5]; }
        else
          {
          if (!read(4)) break;
          bool reflect = (previous == 'C' || previous == 'S');
          x1 = reflect ? 2*cx - lastX : cx; y1 = reflect ? 2*cy - lastY : cy;
          }
        double x2 = ox + v[0], y2 = oy + v[1], x = ox + v[2], y = oy + v[3];
        out(x1, y1, " "); out(x2, y2, " "); out(x, y, " c\n");
        lastX = x2; lastY = y2; cx = x; cy = y;
        }
      else if (upper == 'Q' || upper == 'T')
        {
        double qx, qy;
        if (upper == 'Q') { if (!read(4)) break; qx = ox + v[0]; qy = oy + v[1]; v[0] = v[2]; v[1] = v[3]; }
        else
          {
          if (!read(2)) break;
          bool reflect = (previous == 'Q' || previous == 'T');
          qx = reflect ? 2*cx - lastX : cx; qy = reflect ? 2*cy - lastY : cy;
          }
        double x = ox + v[0], y = oy + v[1];
        // the quadratic curve as a cubic one
        out(cx + 2.0/3*(qx - cx), cy + 2.0/3*(qy - cy), " ");
        out(x + 2.0/3*(qx - x), y + 2.0/3*(qy - y), " ");
        out(x, y, " c\n");
        lastX = qx; lastY = qy; cx = x; cy = y;
        }
      else if (upper == 'A')
        {
        double y0;                                     // arcs (not written by dvisvgm): a line to the end point
        if (!read(6) || !reader.number(y0)) break;
        double x = ox + v[5], y = oy + y0;
        cx = x; cy = y;
        out(cx, cy, " l\n");
        }
      else break;
      previous = upper;
      drawn = true;
      }
    }
  if (drawn) _content << "f\n";
  }

  void PdfPainter::drawText(const Point2D & position, const String & text, const TextAttributes & attributes, float angle)
  {
  if (text.isNull()) return;
  float size = attributes.size()*_regionHeight;
  if (size <= 0) return;
  float x = px(position), y = py(position);           // page coordinates (y down)
  double radians = angle*kPi/180.0, cosine = std::cos(radians), sine = std::sin(radians);
  const LatexImage * image = latexImage(text);
  if (image && image->ok)
    {
    // as SvgPainter: translate(x y) rotate(-angle) translate(dx dy) scale(scale) translate(-image.x 0)
    double scale = size/LatexImage::kFontSize, w = image->width*scale;
    double dx = attributes.justification() == TextAttributes::kCenter ? -0.5*w
              : attributes.justification() == TextAttributes::kRight  ? -w : 0.0;
    double dy = 0.5*(image->height - image->depth)*scale;
    // rotate(-angle) in page coordinates (y down): (u,v) -> (u cos + v sin, -u sin + v cos)
    double a = scale*cosine, b = -scale*sine, c = scale*sine, d = scale*cosine;
    double tx = dx - scale*image->x, ty = dy;
    double e = x + tx*cosine + ty*sine, f = y - tx*sine + ty*cosine;
    setColor(attributes.color(), attributes.alpha(), false);
    svgPaths(image->svg, a, b, c, d, e, f);
    return;
    }
  int font = fontIndex(attributes.font());
  _fontsUsed[font] = true;
  std::string bytes = winAnsi(LatexRenderer::plainText(text));
  float width = standardTextWidth(bytes, font, size);
  float dx = attributes.justification() == TextAttributes::kCenter ? -0.5f*width
           : attributes.justification() == TextAttributes::kRight  ? -width : 0.0f;
  float dy = -0.35f*size;                              // baseline below the center (cap height ~ 0.7 size)
  // text matrix: rotation by angle (counter-clockwise) around the anchor, in PDF coordinates (y up)
  double X = x, Y = flip(y);
  double ox = X + dx*cosine - dy*sine, oy = Y + dx*sine + dy*cosine;
  setColor(attributes.color(), attributes.alpha(), false);
  _content << "BT /F" << font << " " << num(size) << " Tf " << num(cosine) << " " << num(sine) << " " << num(-sine) << " "
           << num(cosine) << " " << num(ox) << " " << num(oy) << " Tm (";
  for (char ch : bytes)
    {
    if (ch == '(' || ch == ')' || ch == '\\') _content << '\\';
    _content << ch;
    }
  _content << ") Tj ET\n";
  }

  int PdfPainter::fontIndex(int font)
  {
  int family = TextAttributes::fontFamily(font);
  if (family == TextAttributes::kSymbol) return 12;
  int base = family == TextAttributes::kSerif ? 4 : (family == TextAttributes::kMono ? 8 : 0);
  return base + (TextAttributes::isBold(font) ? 1 : 0) + (TextAttributes::isItalic(font) ? 2 : 0);
  }

  float PdfPainter::standardTextWidth(const std::string & text, int font, float size)
  {
  double total = 0;
  for (unsigned char ch : text)
    {
    int w;
    if (font >= 8 && font <= 11) w = 600;                                       // Courier: fixed width
    else if (ch < 32 || ch > 126) w = (font >= 4 && font <= 7) ? 500 : 556;
    else if (font >= 4 && font <= 7) w = kTimes[ch - 32];
    else if (font == 1 || font == 3) w = kHelveticaBold[ch - 32];
    else w = kHelvetica[ch - 32];
    total += w;
    }
  return float(total*size/1000.0);
  }

  std::string PdfPainter::winAnsi(const String & text)
  {
  std::string out;
  const unsigned char * s = reinterpret_cast<const unsigned char *>(text.data());
  while (*s)
    {
    unsigned int code;
    int extra;
    if (*s < 0x80)      { code = *s; extra = 0; }
    else if (*s < 0xE0) { code = *s & 0x1F; extra = 1; }
    else if (*s < 0xF0) { code = *s & 0x0F; extra = 2; }
    else                { code = *s & 0x07; extra = 3; }
    s++;
    for (int k=0; k<extra && (*s & 0xC0) == 0x80; k++, s++) code = (code << 6) | (*s & 0x3F);
    if (code < 0x80 || (code >= 0xA0 && code <= 0xFF)) out += char(code);
    else out += '?';
    }
  return out;
  }

  std::string PdfPainter::document() const
  {
  std::vector<Page> pages = _pages;
  pages.push_back(Page{ _content.str(), _pageWidth, _pageHeight });
  int n = int(pages.size());
  std::vector<std::string> objects;                    // object k+1
  // 1 catalog, 2 pages, then a page and its content for each page (3+2k, 4+2k), then the fonts and graphics states
  std::ostringstream fonts, states, kids;
  int next = 3 + 2*n;
  std::vector<std::string> extra;
  for (int k=0; k<13; k++)
    {
    if (!_fontsUsed[k]) continue;
    fonts << "/F" << k << " " << next++ << " 0 R ";
    extra.push_back(std::string("<< /Type /Font /Subtype /Type1 /BaseFont /") + kFontNames[k]
                    + (k == 12 ? "" : " /Encoding /WinAnsiEncoding") + " >>");
    }
  for (const auto & alpha : _alphas)
    {
    states << "/" << alpha.second << " " << next++ << " 0 R ";
    extra.push_back("<< /Type /ExtGState /ca " + num(alpha.first/1000.0) + " /CA " + num(alpha.first/1000.0) + " >>");
    }
  for (int k=0; k<n; k++) kids << (k ? " " : "") << 3 + 2*k << " 0 R";
  objects.push_back("<< /Type /Catalog /Pages 2 0 R >>");
  objects.push_back("<< /Type /Pages /Kids [" + kids.str() + "] /Count " + std::to_string(n) + " >>");
  for (int k=0; k<n; k++)
    {
    const Page & page = pages[size_t(k)];
    objects.push_back("<< /Type /Page /Parent 2 0 R /MediaBox [0 0 " + num(page.width) + " " + num(page.height)
                      + "] /Contents " + std::to_string(4 + 2*k) + " 0 R /Resources << /Font << " + fonts.str()
                      + ">> /ExtGState << " + states.str() + ">> >> >>");
    objects.push_back("<< /Length " + std::to_string(page.content.size()) + " >>\nstream\n" + page.content + "endstream");
    }
  for (const auto & e : extra) objects.push_back(e);

  std::string out = "%PDF-1.4\n%\xE2\xE3\xCF\xD3\n";   // binary marker: the file holds 8-bit text
  std::vector<size_t> offsets;
  for (size_t k=0; k<objects.size(); k++)
    {
    offsets.push_back(out.size());
    out += std::to_string(k + 1) + " 0 obj\n" + objects[k] + "\nendobj\n";
    }
  size_t xref = out.size();
  out += "xref\n0 " + std::to_string(objects.size() + 1) + "\n0000000000 65535 f \n";
  for (size_t offset : offsets)
    {
    char line[24];
    std::snprintf(line, sizeof(line), "%010zu 00000 n \n", offset);
    out += line;
    }
  out += "trailer\n<< /Size " + std::to_string(objects.size() + 1) + " /Root 1 0 R >>\nstartxref\n"
         + std::to_string(xref) + "\n%%EOF\n";
  return out;
  }

  bool PdfPainter::write(const String & fileName) const
  {
  std::ofstream file(fileName.data(), std::ios::binary);
  if (!file) return false;
  std::string bytes = document();
  file.write(bytes.data(), std::streamsize(bytes.size()));
  return bool(file);
  }

  } // namespace GRAPHICS
} // namespace CAP
