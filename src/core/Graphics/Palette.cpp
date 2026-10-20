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
//! @file Palette.cpp
//! @brief Implementation of Palette.
#include <cmath>
#include "Palette.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  namespace
  {
  struct Stop { float red, green, blue; };   // components in [0,1]

  //! Colors at t = 0, 1/(n-1), ..., 1 of each palette.
  const std::vector<Stop> & stops(Palette::Type type)
  {
  static const std::vector<Stop> viridis = {        // matplotlib's viridis
    {68/255.f, 1/255.f, 84/255.f},   {71/255.f, 45/255.f, 123/255.f},  {59/255.f, 82/255.f, 139/255.f},
    {44/255.f, 114/255.f, 142/255.f}, {33/255.f, 145/255.f, 140/255.f}, {40/255.f, 174/255.f, 128/255.f},
    {94/255.f, 201/255.f, 98/255.f},  {173/255.f, 220/255.f, 48/255.f}, {253/255.f, 231/255.f, 37/255.f} };
  static const std::vector<Stop> bird = {           // ROOT's kBird
    {0.2082f, 0.1664f, 0.5293f}, {0.0592f, 0.3599f, 0.8684f}, {0.0780f, 0.5041f, 0.8385f},
    {0.0232f, 0.6419f, 0.7914f}, {0.1802f, 0.7178f, 0.6425f}, {0.5301f, 0.7492f, 0.4662f},
    {0.8186f, 0.7328f, 0.3499f}, {0.9956f, 0.7862f, 0.1968f}, {0.9764f, 0.9832f, 0.0539f} };
  static const std::vector<Stop> grayscale = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} };
  static const std::vector<Stop> diverging = {      // ColorBrewer RdBu, blue for the low values
    {33/255.f, 102/255.f, 172/255.f}, {103/255.f, 169/255.f, 207/255.f}, {247/255.f, 247/255.f, 247/255.f},
    {239/255.f, 138/255.f, 98/255.f}, {178/255.f, 24/255.f, 43/255.f} };
  switch (type)
    {
    case Palette::kBird:      return bird;
    case Palette::kGrayscale: return grayscale;
    case Palette::kDiverging: return diverging;
    default:                  return viridis;
    }
  }
  } // namespace

  Palette::Palette(Type type, int nSteps)
  :
  _type(type >= 0 && type < kNTypes ? type : kViridis),
  _nSteps(nSteps > 1 ? nSteps : 0)
  { }

  Color Palette::color(double t) const
  {
  if (!(t >= 0)) t = 0;           // also NaN
  if (t > 1) t = 1;
  if (_nSteps > 1)                // the color of the middle of the band holding t
    {
    int band = int(t*_nSteps);
    if (band >= _nSteps) band = _nSteps - 1;
    t = (band + 0.5)/_nSteps;
    }
  const std::vector<Stop> & s = stops(_type);
  double position = t*(s.size() - 1);
  std::size_t k = std::size_t(position);
  if (k >= s.size() - 1) return Color(s.back().red, s.back().green, s.back().blue);
  float f = float(position - k);
  return Color(s[k].red   + f*(s[k+1].red   - s[k].red),
               s[k].green + f*(s[k+1].green - s[k].green),
               s[k].blue  + f*(s[k+1].blue  - s[k].blue));
  }

  const char * Palette::name(Type type)
  {
  switch (type)
    {
    case kBird:      return "bird";
    case kGrayscale: return "grayscale";
    case kDiverging: return "diverging";
    default:         return "viridis";
    }
  }

  Palette::Type Palette::type(const String & name)
  {
  for (int k=0; k<kNTypes; k++) if (name.equalTo(Palette::name(Type(k)), true)) return Type(k);
  return kViridis;
  }

  } // namespace GRAPHICS
} // namespace CAP
