/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Projection3D.cpp
//! @brief Implementation of Projection3D.
#include <algorithm>
#include <cmath>
#include "Projection3D.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  Projection3D::Projection3D(const Graph & graph, const Graph::View & view, float width, float height)
  :
  _view(view),
  _cosA(std::cos(graph.azimuth()*M_PI/180.0)),
  _sinA(std::sin(graph.azimuth()*M_PI/180.0)),
  _cosE(std::cos(graph.elevation()*M_PI/180.0)),
  _sinE(std::sin(graph.elevation()*M_PI/180.0)),
  _scale(1.0),
  _centerX(0.5*(graph.frameXLow() + graph.frameXHigh())),
  _centerY(0.5*(graph.frameYLow() + graph.frameYHigh())),
  _width(width > 0 ? width : 1.0f),
  _height(height > 0 ? height : 1.0f)
  {
  // the projected corners of the box (unit scale), to fit it in the frame with room for the labels
  double xLow = 1E9, xHigh = -1E9, yLow = 1E9, yHigh = -1E9;
  for (int corner=0; corner<8; corner++)
    {
    double right, up, away;
    direction((corner & 1) - 0.5, ((corner >> 1) & 1) - 0.5, ((corner >> 2) & 1) - 0.5, right, up, away);
    xLow = std::min(xLow, right); xHigh = std::max(xHigh, right);
    yLow = std::min(yLow, up);    yHigh = std::max(yHigh, up);
    }
  double frameWidth  = (graph.frameXHigh() - graph.frameXLow())*_width;    // device units
  double frameHeight = (graph.frameYHigh() - graph.frameYLow())*_height;
  _scale = 0.88*std::min(frameWidth/(xHigh - xLow), frameHeight/(yHigh - yLow));
  _centerX -= 0.5*(xLow + xHigh)*_scale/_width;     // the projected box centered in the frame
  _centerY -= 0.5*(yLow + yHigh)*_scale/_height;
  }

  double Projection3D::fraction(int k, double value) const
  {
  double low = _view.minimum(k), high = _view.maximum(k);
  if (_view.log(k) && low > 0)
    return value > 0 ? (std::log10(value) - std::log10(low))/(std::log10(high) - std::log10(low)) : 0.0;
  return high > low ? (value - low)/(high - low) : 0.5;
  }

  void Projection3D::direction(double dx, double dy, double dz, double & right, double & up, double & away) const
  {
  // turned by the azimuth about z, then tilted by the elevation about the horizontal axis of the screen
  double x1 =  dx*_cosA + dy*_sinA;
  double y1 = -dx*_sinA + dy*_cosA;
  right = x1;
  up    = dz*_cosE + y1*_sinE;
  away  = y1*_cosE - dz*_sinE;
  }

  Point2D Projection3D::toCanvas(double fx, double fy, double fz) const
  {
  double right, up, away;
  direction(fx - 0.5, fy - 0.5, fz - 0.5, right, up, away);
  return Point2D(float(_centerX + right*_scale/_width), float(_centerY + up*_scale/_height));
  }

  double Projection3D::depth(double fx, double fy, double fz) const
  {
  double right, up, away;
  direction(fx - 0.5, fy - 0.5, fz - 0.5, right, up, away);
  return away;
  }

  } // namespace GRAPHICS
} // namespace CAP
