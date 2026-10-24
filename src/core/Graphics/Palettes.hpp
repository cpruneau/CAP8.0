/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Palettes.hpp
//! @brief Color, marker and shading palettes, and the library of the palettes of a session.
#ifndef CAP__Palettes
#define CAP__Palettes
#include <map>
#include <vector>
#include "Palette.hpp"
#include "String.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //! @brief Shading palette: the continuous colormap of maps, contours and 3D views (see Palette).
  using ShadingPalette = Palette;

  //!
  //! @brief Colors given in turn to the items of a graph (and to the fits): color k is the color of item k, the list
  //! starting again at its end.
  //!
  class ColorPalette
  {
  public:
  //! @brief Palette of a name and colors. @param name name @param colors color numbers (see ColorTable)
  ColorPalette(const String & name="", const std::vector<int> & colors=std::vector<int>()) : _name(name), _colors(colors) { }
  const String & name() const { return _name; }   //!< Name of the palette
  const std::vector<int> & colors() const { return _colors; }   //!< Colors, in order
  int size() const { return int(_colors.size()); }   //!< Number of colors
  //! @brief Color of item k (cycling through the list). @param k index @return color number (black if empty)
  int color(int k) const { return _colors.empty() ? 1 : _colors[size_t((k < 0 ? 0 : k) % int(_colors.size()))]; }
  private:
  String _name;   //!< name
  std::vector<int> _colors;   //!< color numbers
  };

  //!
  //! @brief Marker styles given in turn to the items of a graph: marker k is the marker of item k, the list starting
  //! again at its end.
  //!
  class MarkerPalette
  {
  public:
  //! @brief Palette of a name and marker styles. @param name name @param markers ROOT marker styles
  MarkerPalette(const String & name="", const std::vector<int> & markers=std::vector<int>()) : _name(name), _markers(markers) { }
  const String & name() const { return _name; }   //!< Name of the palette
  const std::vector<int> & markers() const { return _markers; }   //!< Marker styles, in order
  int size() const { return int(_markers.size()); }   //!< Number of markers
  //! @brief Marker of item k (cycling through the list). @param k index @return style (20, a filled circle, if empty)
  int marker(int k) const { return _markers.empty() ? 20 : _markers[size_t((k < 0 ? 0 : k) % int(_markers.size()))]; }
  private:
  String _name;   //!< name
  std::vector<int> _markers;   //!< marker styles
  };

  //!
  //! @brief The palettes of a session: named color, marker and shading palettes (presets, and those added), and the
  //! current one of each kind, used by the graphs that do not choose their own.
  //!
  //! Presets: colors "default" (black, red, blue, green, ...), "okabe-ito" (safe for color-blind readers), "fits"
  //! (the curves of the fits); markers "default" (filled), "open"; shading "viridis", "bird", "grayscale",
  //! "diverging". The current palette of each kind can be changed at any time; graphs follow it unless they name
  //! their own (Graph::setColorPaletteName(), ...).
  //! @code
  //!   Palettes & palettes = Palettes::session();
  //!   palettes.add(ColorPalette("mine", { 2, 4, 418, 616 }));
  //!   palettes.setColorPalette("okabe-ito");                  // for all the graphs that follow the session
  //!   graph.setColorPaletteName("mine");                       // this graph only
  //! @endcode
  //!
  class Palettes
  {
  public:
  //! @brief The palettes of the session. @return library
  static Palettes & session();
  //! @brief Library with the presets. Usually the one of the session is used (see session()).
  Palettes();

  //! @name Adding and finding palettes (a palette of the same name is replaced)
  //! @{
  void add(const ColorPalette & palette) { _colors[palette.name().str()] = palette; }   //!< Add a color palette
  void add(const MarkerPalette & palette) { _markers[palette.name().str()] = palette; }   //!< Add a marker palette
  //! @brief Add a shading palette. @param name name @param palette palette
  void add(const String & name, const ShadingPalette & palette) { _shadings[name.str()] = palette; }
  //! @brief Color palette of a name. @param name name @return palette, or null
  const ColorPalette * colorPalette(const String & name) const;
  //! @brief Marker palette of a name. @param name name @return palette, or null
  const MarkerPalette * markerPalette(const String & name) const;
  //! @brief Shading palette of a name. @param name name @return palette, or null
  const ShadingPalette * shadingPalette(const String & name) const;
  std::vector<String> colorPaletteNames() const;   //!< Names of the color palettes
  std::vector<String> markerPaletteNames() const;   //!< Names of the marker palettes
  std::vector<String> shadingPaletteNames() const;   //!< Names of the shading palettes
  //! @}

  //! @name The current palettes
  //! @{
  //! @brief Make a color palette current. @param name name @return false (nothing changed) if there is none of that name
  bool setColorPalette(const String & name);
  //! @brief Make a marker palette current. @param name name @return false (nothing changed) if there is none of that name
  bool setMarkerPalette(const String & name);
  //! @brief Make a shading palette current. @param name name @return false (nothing changed) if there is none of that name
  bool setShadingPalette(const String & name);
  const ColorPalette & colorPalette() const { return _colors.at(_currentColors); }   //!< Current color palette
  const MarkerPalette & markerPalette() const { return _markers.at(_currentMarkers); }   //!< Current marker palette
  const ShadingPalette & shadingPalette() const { return _shadings.at(_currentShading); }   //!< Current shading palette
  const String & shadingPaletteName() const { return _currentShadingName; }   //!< Name of the current shading palette
  //! @brief Color palette of a name, or the current one if the name is empty or unknown. @param name name @return palette
  const ColorPalette & colorPaletteOr(const String & name) const;
  //! @brief Marker palette of a name, or the current one if the name is empty or unknown. @param name name @return palette
  const MarkerPalette & markerPaletteOr(const String & name) const;
  //! @}

  private:
  std::map<std::string, ColorPalette> _colors;   //!< color palettes by name
  std::map<std::string, MarkerPalette> _markers;   //!< marker palettes by name
  std::map<std::string, ShadingPalette> _shadings;   //!< shading palettes by name
  std::string _currentColors;   //!< name of the current color palette
  std::string _currentMarkers;   //!< name of the current marker palette
  std::string _currentShading;   //!< name of the current shading palette
  String _currentShadingName;   //!< name of the current shading palette
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Palettes
