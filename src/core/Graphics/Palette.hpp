/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Palette.hpp
//! @brief Palette: colors of the values of a color map, from the lowest to the highest.
#ifndef CAP__Palette
#define CAP__Palette
#include <vector>
#include "Aliases.hpp"
#include "Color.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! @brief Colors of the values of a color map: color(t) for t in [0,1], from the lowest value (0) to the highest (1).
  //!
  //! The palettes are defined by colors at equally spaced values of t, linearly interpolated:
  //!  - kViridis: dark blue, green, yellow (matplotlib's default): perceptually uniform, readable in grays and by
  //!    readers with color blindness. The default.
  //!  - kBird: ROOT's default palette, blue, green, yellow.
  //!  - kGrayscale: white (low) to black (high), for printing.
  //!  - kDiverging: blue, white (middle), red; for values of both signs, e.g. correlation functions, with a z range
  //!    centered on zero (see Graph::setSymmetricZ()).
  //!
  //! With nSteps() > 1 the colors are those of nSteps bands of equal width (as filled contours); with 0, they vary
  //! continuously.
  //!
  class Palette
  {
  public:

  //! @brief Built-in palettes.
  enum Type { kViridis, kBird, kGrayscale, kDiverging, kNTypes };

  //! @brief Palette. @param type palette @param nSteps number of bands (0: continuous)
  Palette(Type type=kViridis, int nSteps=0);

  //! @brief The palette. @return type
  Type type() const { return _type; }
  //! @brief Number of bands. @return number of bands (0: continuous)
  int  nSteps() const { return _nSteps; }
  //! @brief Color of a value. @param t position of the value in its range, clamped to [0,1] @return color
  Color color(double t) const;

  //! @brief Name of a palette ("viridis", "bird", "grayscale", "diverging"). @param type palette @return name
  static const char * name(Type type);
  //! @brief Palette of a name (see name()). @param name name @return palette (kViridis if the name is unknown)
  static Type type(const String & name);

  protected:

  Type _type;     //!< palette
  int  _nSteps;   //!< number of bands (0: continuous)
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Palette
