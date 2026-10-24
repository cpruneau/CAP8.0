/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ColorTable.hpp
//! @brief ColorTable: the colors of CAP by number (ROOT's numbering), independent of ROOT.
#ifndef CAP__ColorTable
#define CAP__ColorTable
#include <map>
#include "Color.hpp"
#include "String.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! @brief The colors of CAP by number, with ROOT's numbering and values: attributes, configurations and the GUI store
  //! color numbers, and those written for ROOT keep their meaning.
  //!
  //! The table starts with ROOT's 446 default colors below 1000 (0-9 the basic colors, 920-923 grays, the families
  //! kRed = 632, kPink = 900, kMagenta = 616, kViolet = 880, kBlue = 600, kAzure = 860, kCyan = 432, kTeal = 840,
  //! kGreen = 416, kSpring = 820, kYellow = 400, kOrange = 800 with offsets -10 to +4, ...). Other colors get numbers from
  //! 1000 up, the first time they are asked for (see index()).
  //! @code
  //!   ColorTable & colors = ColorTable::instance();
  //!   Color c = colors.color(634);                         // kRed+2
  //!   int k = colors.index(Color(0.2f, 0.4f, 0.6f));        // a number for any color
  //!   colors.set(2000, Color(1.0f, 0.5f, 0.0f), "my orange");
  //! @endcode
  //!
  class ColorTable
  {
  public:
  //! @brief The table of the session. @return table
  static ColorTable & instance();

  //! @brief Whether a number is a color of the table. @param index number @return true if so
  bool has(int index) const { return _colors.count(index) > 0; }
  //! @brief Color of a number (opaque). @param index number @return color, black if the number is not a color
  Color color(int index) const;
  //! @brief Components of a color. @param index number @param red,green,blue components in [0,1] (black if unknown)
  void rgb(int index, float & red, float & green, float & blue) const;
  //! @brief Number of a color: the first of the table with the same components (to 1/512), or a new number from 1000
  //! up. The opacity is not part of a color number. @param color color @return number
  int index(const Color & color);
  //! @brief Define (or redefine) a color. @param index number @param color color @param name name (e.g. "my orange")
  void set(int index, const Color & color, const String & name="");
  //! @brief Name of a color: ROOT's name of the default colors ("kRed+2", "black"...), the name given to set(), or
  //! its components as "#rrggbb". @param index number @return name
  String name(int index) const;
  //! @brief Number of colors in the table. @return number
  int size() const { return int(_colors.size()); }

  private:
  //! A color and its name
  struct Entry { Color color; String name; };
  ColorTable();
  std::map<int, Entry> _colors;   //!< colors by number
  int _next;   //!< next number given to a new color
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__ColorTable
