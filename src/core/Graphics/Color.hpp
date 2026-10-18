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
//! @file Color.hpp
//! @brief Color: a color given by its red, green, blue and alpha components (for the painters).
#ifndef CAP__Color
#define CAP__Color

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! @brief A color given by its components in [0,1]: red, green, blue, and alpha (opacity).
  //!
  //! Most attributes of CAP graphics use ROOT color indices (see Painter::rgb()); a Color is used where any color may
  //! be needed, e.g. for the cells of a color map (see Palette, Painter::fillRectangle()).
  //!
  struct Color
  {
    float red   = 0.0f;   //!< red component, in [0,1]
    float green = 0.0f;   //!< green component, in [0,1]
    float blue  = 0.0f;   //!< blue component, in [0,1]
    float alpha = 1.0f;   //!< opacity, in [0,1] (1: opaque)

    //! @brief Black.
    Color() { }
    //! @brief Color of the given components. @param r red @param g green @param b blue @param a alpha
    Color(float r, float g, float b, float a=1.0f) : red(r), green(g), blue(b), alpha(a) { }
    //! @brief Color of the given components in [0,255]. @param r red @param g green @param b blue @param a alpha in [0,1]
    static Color fromBytes(int r, int g, int b, float a=1.0f) { return Color(r/255.0f, g/255.0f, b/255.0f, a); }
    //! @brief Whether two colors are equal. @param c color @return true if all the components are equal
    bool operator==(const Color & c) const { return red==c.red && green==c.green && blue==c.blue && alpha==c.alpha; }
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Color
