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
//! @file ImGuiPainter.cpp
//! @brief Implementation of ImGuiPainter.
#include <cmath>
#include <cfloat>
#include "imgui.h"
#include "ImGuiPainter.hpp"
#include "LatexRenderer.hpp"
#include "LatexTextures.hpp"

namespace CAP
{
  namespace GUI
  {
  using namespace GRAPHICS;

  namespace
  {
  const float kPi = 3.14159265358979323846f;

  ImU32 color(int color, float alpha)
  {
  float r, g, b;
  Painter::rgb(color, r, g, b);
  return ImGui::GetColorU32(ImVec4(r, g, b, alpha));
  }

  //! Dash pattern of a line style, in units of the line width; empty for solid lines.
  std::vector<float> dashPattern(int style)
  {
  switch (style)
    {
    case 2:  return {6.0f, 4.0f};               // dashed
    case 3:  return {1.5f, 3.0f};               // dotted
    case 4:  return {8.0f, 3.0f, 1.5f, 3.0f};   // dash-dot
    default: return {};
    }
  }
  ImFont * gFonts[16] = { nullptr };
  } // namespace

  void ImGuiPainter::setFont(int index, ImFont * font)
  {
  if (index >= 1 && index <= 15) gFonts[index] = font;
  }

  ImFont * ImGuiPainter::fontFor(int font)
  {
  return gFonts[TextAttributes::fontIndex(font)];
  }

  ImFont * ImGuiPainter::font(const TextAttributes & attributes) const
  {
  ImFont * f = fontFor(attributes.font());
  if (!f) f = _font;
  return f ? f : ImGui::GetFont();
  }

  ImGuiPainter::ImGuiPainter(ImDrawList * drawList, ImFont * font, float x, float y, float width, float height)
  :
  Painter(),
  _drawList(drawList),
  _font(font),
  _x(x), _y(y), _width(width), _height(height)
  { }

  void ImGuiPainter::toScreen(const Point2D & p, float & x, float & y) const
  {
  x = _x + p.x()*_width;
  y = _y + (1.0f - p.y())*_height;
  }

  Point2D ImGuiPainter::toCanvas(float x, float y) const
  {
  return Point2D((x - _x)/_width, 1.0f - (y - _y)/_height);
  }

  void ImGuiPainter::drawLine(const std::vector<Point2D> & points, const LineAttributes & line)
  {
  if (points.size() < 2 || line.style() == 0) return;
  ImU32 c = color(line.color(), line.alpha());
  float w = line.width() > 0 ? line.width() : 1.0f;
  std::vector<ImVec2> screen;
  screen.reserve(points.size());
  for (const auto & p : points) { float x, y; toScreen(p, x, y); screen.push_back(ImVec2(x, y)); }
  std::vector<float> pattern = dashPattern(line.style());
  if (pattern.empty())
    {
    _drawList->AddPolyline(screen.data(), int(screen.size()), c, ImDrawFlags_None, w);
    return;
    }
  // dashed: walk along the polyline, alternating drawn and skipped lengths
  size_t k = 0;
  float left = pattern[0]*w;
  bool on = true;
  for (size_t i=1; i<screen.size(); i++)
    {
    ImVec2 a = screen[i-1], b = screen[i];
    float dx = b.x-a.x, dy = b.y-a.y, length = std::sqrt(dx*dx + dy*dy), done = 0.0f;
    while (length > 0 && done < length)
      {
      float step = std::fmin(left, length - done);
      ImVec2 p0(a.x + dx*done/length, a.y + dy*done/length);
      ImVec2 p1(a.x + dx*(done+step)/length, a.y + dy*(done+step)/length);
      if (on) _drawList->AddLine(p0, p1, c, w);
      done += step;
      left -= step;
      if (left <= 0) { k = (k+1) % pattern.size(); left = pattern[k]*w; on = !on; }
      }
    }
  }

  void ImGuiPainter::drawPolygon(const std::vector<Point2D> & points, const FillAttributes & fill, const LineAttributes & line)
  {
  if (points.size() < 3) return;
  std::vector<ImVec2> screen;
  for (const auto & p : points) { float x, y; toScreen(p, x, y); screen.push_back(ImVec2(x, y)); }
  if (fill.style() != 0)
    _drawList->AddConcavePolyFilled(screen.data(), int(screen.size()), color(fill.color(), fill.alpha()));
  if (line.style() != 0)
    {
    std::vector<Point2D> closed = points;
    closed.push_back(points[0]);
    drawLine(closed, line);
    }
  }

  void ImGuiPainter::drawMarker(const Point2D & position, const MarkerAttributes & marker)
  {
  float s = 4.0f*marker.size();   // half size: size 1 is 8 pixels across
  if (s <= 0) return;
  float x, y;
  toScreen(position, x, y);
  ImU32 c = color(marker.color(), marker.alpha());
  ImVec2 p(x, y);
  switch (marker.style())
    {
    case 1:  _drawList->AddCircleFilled(p, 0.75f, c); break;
    case 6:  _drawList->AddCircleFilled(p, 1.0f, c);  break;
    case 7:  _drawList->AddCircleFilled(p, 1.5f, c);  break;
    case 2:  _drawList->AddLine(ImVec2(x-s,y), ImVec2(x+s,y), c); _drawList->AddLine(ImVec2(x,y-s), ImVec2(x,y+s), c); break;
    case 5:  _drawList->AddLine(ImVec2(x-s,y-s), ImVec2(x+s,y+s), c); _drawList->AddLine(ImVec2(x-s,y+s), ImVec2(x+s,y-s), c); break;
    case 3:
      {
      float d = 0.7f*s;
      _drawList->AddLine(ImVec2(x-s,y), ImVec2(x+s,y), c); _drawList->AddLine(ImVec2(x,y-s), ImVec2(x,y+s), c);
      _drawList->AddLine(ImVec2(x-d,y-d), ImVec2(x+d,y+d), c); _drawList->AddLine(ImVec2(x-d,y+d), ImVec2(x+d,y-d), c);
      break;
      }
    case 4: case 24: _drawList->AddCircle(p, s, c, 0, 1.0f); break;
    case 21: _drawList->AddRectFilled(ImVec2(x-s,y-s), ImVec2(x+s,y+s), c); break;
    case 25: _drawList->AddRect(ImVec2(x-s,y-s), ImVec2(x+s,y+s), c); break;
    case 22: _drawList->AddTriangleFilled(ImVec2(x-s,y+s), ImVec2(x+s,y+s), ImVec2(x,y-s), c); break;
    case 26: _drawList->AddTriangle(ImVec2(x-s,y+s), ImVec2(x+s,y+s), ImVec2(x,y-s), c); break;
    case 23: _drawList->AddTriangleFilled(ImVec2(x-s,y-s), ImVec2(x+s,y-s), ImVec2(x,y+s), c); break;
    case 32: _drawList->AddTriangle(ImVec2(x-s,y-s), ImVec2(x+s,y-s), ImVec2(x,y+s), c); break;
    case 33: _drawList->AddQuadFilled(ImVec2(x,y-s), ImVec2(x+s,y), ImVec2(x,y+s), ImVec2(x-s,y), c); break;
    case 27: _drawList->AddQuad(ImVec2(x,y-s), ImVec2(x+s,y), ImVec2(x,y+s), ImVec2(x-s,y), c); break;
    case 29: case 30:
      {
      ImVec2 star[10];
      for (int k=0; k<10; k++)
        {
        float phi = kPi/2 + k*kPi/5, r = (k%2 == 0) ? s : 0.4f*s;
        star[k] = ImVec2(x + r*std::cos(phi), y - r*std::sin(phi));
        }
      if (marker.style() == 29) _drawList->AddConcavePolyFilled(star, 10, c);
      else                      _drawList->AddPolyline(star, 10, c, ImDrawFlags_Closed, 1.0f);
      break;
      }
    default: _drawList->AddCircleFilled(p, s, c); break;   // 8, 20 and anything else
    }
  }

  void ImGuiPainter::drawText(const Point2D & position, const String & text, const TextAttributes & attributes, float angle)
  {
  if (text.isNull()) return;
  float size = attributes.size()*_height;
  if (size < 1.0f) return;
  if (LatexRenderer::kind(text) == LatexRenderer::kLatex && drawLatex(position, text, attributes, angle)) return;
  String shown = LatexRenderer::plainText(text);   // plain text, or LaTeX not available, not ready yet or failed
  ImFont * font = this->font(attributes);
  ImVec2 extent = font->CalcTextSizeA(size, FLT_MAX, 0.0f, shown.data());
  float dx = 0.0f;
  if      (attributes.justification() == TextAttributes::kCenter) dx = -0.5f*extent.x;
  else if (attributes.justification() == TextAttributes::kRight)  dx = -extent.x;
  float dy = -0.5f*extent.y;                                    // vertically centered on position
  float x, y;
  toScreen(position, x, y);
  int first = _drawList->VtxBuffer.Size;
  _drawList->AddText(font, size, ImVec2(x + dx, y + dy), color(attributes.color(), attributes.alpha()), shown.data());
  if (angle != 0.0f)
    {
    // rotate the glyphs just added about the anchor (counter-clockwise on screen)
    float a = -angle*kPi/180.0f, ca = std::cos(a), sa = std::sin(a);
    for (int k=first; k<_drawList->VtxBuffer.Size; k++)
      {
      ImVec2 & v = _drawList->VtxBuffer[k].pos;
      float rx = v.x - x, ry = v.y - y;
      v = ImVec2(x + rx*ca - ry*sa, y + rx*sa + ry*ca);
      }
    }
  }

  bool ImGuiPainter::drawLatex(const Point2D & position, const String & text, const TextAttributes & attributes, float angle)
  {
  const LatexImage * image = LatexRenderer::instance().imageLater(text);
  if (!image || !image->ok) return false;
  float size = attributes.size()*_height;                         // font size (screen units)
  float pixelsPerUnit = ImGui::GetIO().DisplayFramebufferScale.x;  // 2 on a Retina display
  if (pixelsPerUnit <= 0) pixelsPerUnit = 1.0f;
  const LatexTextures::Texture * texture = LatexTextures::instance().texture(*image, size*pixelsPerUnit, ImGui::GetFrameCount());
  if (!texture) return false;
  // the texture covers the ink box and its padding; screen units per texture pixel:
  float unit = (size/LatexImage::kFontSize)/texture->scale;
  float w = texture->width*unit, h = texture->height*unit, pad = LatexTextures::kPadding*unit;
  float ink = w - 2*pad;
  float left = attributes.justification() == TextAttributes::kCenter ? -0.5f*ink
             : attributes.justification() == TextAttributes::kRight  ? -ink : 0.0f;
  left -= pad;
  float top = -0.5f*h;                                            // the ink box centered vertically on position
  float x, y;
  toScreen(position, x, y);
  float a = -angle*kPi/180.0f, ca = std::cos(a), sa = std::sin(a);  // counter-clockwise on screen
  auto corner = [&](float u, float v) { return ImVec2(x + u*ca - v*sa, y + u*sa + v*ca); };
  _drawList->AddImageQuad(ImTextureRef(ImTextureID(texture->id)),
                          corner(left, top), corner(left + w, top), corner(left + w, top + h), corner(left, top + h),
                          ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), color(attributes.color(), attributes.alpha()));
  return true;
  }

  float ImGuiPainter::textWidth(const String & text, const TextAttributes & attributes) const
  {
  if (text.isNull()) return 0.0f;
  float size = attributes.size()*_height;
  if (size <= 0) return 0.0f;
  if (LatexRenderer::kind(text) == LatexRenderer::kLatex)
    {
    const LatexImage * image = LatexRenderer::instance().imageLater(text);
    if (image && image->ok) return image->width*size/LatexImage::kFontSize;
    }
  return font(attributes)->CalcTextSizeA(size, FLT_MAX, 0.0f, LatexRenderer::plainText(text).data()).x;
  }

  } // namespace GUI
} // namespace CAP
