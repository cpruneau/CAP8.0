/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file LatexTextures.cpp
//! @brief Implementation of LatexTextures.
#include "LatexTextures.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <GLFW/glfw3.h>
#include "nanosvg.h"
#include "nanosvgrast.h"
#include "LatexRenderer.hpp"

namespace CAP
{
  namespace GUI
  {

  namespace
  {
  const int kKeep    = 300;   // textures kept even if not drawn lately
  const int kRecent  = 2;     // textures drawn in the last frames are never deleted (they may be in a draw list)
  }

  LatexTextures & LatexTextures::instance()
  {
  static LatexTextures textures;
  return textures;
  }

  LatexTextures::LatexTextures()
  :
  _textures(),
  _rasterizer(nullptr)
  { }

  LatexTextures::~LatexTextures()
  {
  if (_rasterizer) nsvgDeleteRasterizer(_rasterizer);
  }

  const LatexTextures::Texture * LatexTextures::texture(const GRAPHICS::LatexImage & image, float pixelSize, int frame)
  {
  if (!image.ok || pixelSize < 1.0f) return nullptr;
  int halfPixels = int(std::lround(2.0f*pixelSize));
  auto key = std::make_pair(&image, halfPixels);
  auto found = _textures.find(key);
  if (found != _textures.end()) { found->second.lastFrame = frame; return &found->second; }

  // the paths in an SVG document whose user units are those of the image, shifted to the top left of the ink box
  float inkHeight = image.height + image.depth;
  char header[256];
  std::snprintf(header, sizeof(header), "<svg xmlns='http://www.w3.org/2000/svg' width='%g' height='%g' viewBox='%g %g %g %g'>",
                image.width, inkHeight, image.x, -image.height, image.width, inkHeight);
  std::string document = std::string(header) + image.svg + "</svg>";
  std::vector<char> buffer(document.begin(), document.end());
  buffer.push_back(0);
  NSVGimage * svg = nsvgParse(buffer.data(), "px", 96.0f);
  if (!svg) return nullptr;

  Texture texture;
  texture.scale  = 0.5f*halfPixels/GRAPHICS::LatexImage::kFontSize;
  texture.width  = int(std::ceil(image.width*texture.scale)) + 2*kPadding;
  texture.height = int(std::ceil(inkHeight*texture.scale)) + 2*kPadding;
  texture.lastFrame = frame;
  if (texture.width > 8192 || texture.height > 8192) { nsvgDelete(svg); return nullptr; }
  std::vector<unsigned char> pixels(std::size_t(texture.width)*texture.height*4, 0);
  if (!_rasterizer) _rasterizer = nsvgCreateRasterizer();
  if (!_rasterizer) { nsvgDelete(svg); return nullptr; }
  nsvgRasterize(_rasterizer, svg, float(kPadding), float(kPadding), texture.scale, pixels.data(), texture.width, texture.height, texture.width*4);
  nsvgDelete(svg);
  for (std::size_t k=0; k<pixels.size(); k+=4) { pixels[k] = 255; pixels[k+1] = 255; pixels[k+2] = 255; }   // white, alpha = coverage

  GLint previous = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous);
  GLuint id = 0;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  glBindTexture(GL_TEXTURE_2D, GLuint(previous));
  if (id == 0) return nullptr;
  texture.id = id;
  return &(_textures[key] = texture);
  }

  void LatexTextures::collect(int frame)
  {
  if (int(_textures.size()) <= kKeep) return;
  std::vector<std::pair<int,std::pair<const GRAPHICS::LatexImage*,int>>> old;   // (last frame, key), not drawn lately
  for (const auto & entry : _textures)
    if (entry.second.lastFrame < frame - kRecent) old.push_back(std::make_pair(entry.second.lastFrame, entry.first));
  std::sort(old.begin(), old.end());
  std::size_t excess = _textures.size() - kKeep;
  for (std::size_t k=0; k<old.size() && k<excess; k++)
    {
    auto found = _textures.find(old[k].second);
    GLuint id = found->second.id;
    glDeleteTextures(1, &id);
    _textures.erase(found);
    }
  }

  void LatexTextures::clear()
  {
  for (const auto & entry : _textures)
    {
    GLuint id = entry.second.id;
    glDeleteTextures(1, &id);
    }
  _textures.clear();
  }

  } // namespace GUI
} // namespace CAP
