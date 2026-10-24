/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "Palettes.hpp"
#include "ColorTable.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  Palettes & Palettes::session()
  {
  static Palettes palettes;
  return palettes;
  }

  Palettes::Palettes()
  :
  _colors(),
  _markers(),
  _shadings(),
  _currentColors("default"),
  _currentMarkers("default"),
  _currentShading("viridis"),
  _currentShadingName("viridis")
  {
  add(ColorPalette("default", { 1, 2, 4, 8, 6, 9, 28, 46 }));
  add(ColorPalette("fits", { 2, 4, 8, 6, 9, 46, 28, 7 }));
  // M. Okabe and K. Ito: distinct for the common forms of color blindness
  ColorTable & table = ColorTable::instance();
  std::vector<int> okabeIto;
  for (unsigned int rgb : { 0x000000u, 0xE69F00u, 0x56B4E9u, 0x009E73u, 0xF0E442u, 0x0072B2u, 0xD55E00u, 0xCC79A7u })
    okabeIto.push_back(table.index(Color::fromBytes(int(rgb >> 16), int((rgb >> 8) & 255), int(rgb & 255))));
  add(ColorPalette("okabe-ito", okabeIto));
  add(MarkerPalette("default", { 20, 21, 22, 23, 33, 29, 34, 47 }));
  add(MarkerPalette("open", { 24, 25, 26, 32, 27, 30, 28, 46 }));
  for (int t=0; t<Palette::kNTypes; t++)
    add(String(Palette::name(Palette::Type(t))), Palette(Palette::Type(t), 0));
  }

  const ColorPalette * Palettes::colorPalette(const String & name) const
  {
  auto found = _colors.find(name.str());
  return found == _colors.end() ? nullptr : &found->second;
  }

  const MarkerPalette * Palettes::markerPalette(const String & name) const
  {
  auto found = _markers.find(name.str());
  return found == _markers.end() ? nullptr : &found->second;
  }

  const ShadingPalette * Palettes::shadingPalette(const String & name) const
  {
  auto found = _shadings.find(name.str());
  return found == _shadings.end() ? nullptr : &found->second;
  }

  std::vector<String> Palettes::colorPaletteNames() const
  {
  std::vector<String> names;
  for (const auto & e : _colors) names.push_back(String(e.first));
  return names;
  }

  std::vector<String> Palettes::markerPaletteNames() const
  {
  std::vector<String> names;
  for (const auto & e : _markers) names.push_back(String(e.first));
  return names;
  }

  std::vector<String> Palettes::shadingPaletteNames() const
  {
  std::vector<String> names;
  for (const auto & e : _shadings) names.push_back(String(e.first));
  return names;
  }

  bool Palettes::setColorPalette(const String & name)
  {
  if (!colorPalette(name)) return false;
  _currentColors = name.str();
  return true;
  }

  bool Palettes::setMarkerPalette(const String & name)
  {
  if (!markerPalette(name)) return false;
  _currentMarkers = name.str();
  return true;
  }

  bool Palettes::setShadingPalette(const String & name)
  {
  if (!shadingPalette(name)) return false;
  _currentShading = name.str();
  _currentShadingName = name;
  return true;
  }

  const ColorPalette & Palettes::colorPaletteOr(const String & name) const
  {
  const ColorPalette * p = name.isNull() || name.str().empty() ? nullptr : colorPalette(name);
  return p ? *p : colorPalette();
  }

  const MarkerPalette & Palettes::markerPaletteOr(const String & name) const
  {
  const MarkerPalette * p = name.isNull() || name.str().empty() ? nullptr : markerPalette(name);
  return p ? *p : markerPalette();
  }

  } // namespace GRAPHICS
} // namespace CAP
