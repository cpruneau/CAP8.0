/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,  Nov 10, 2026
 * Based on TGlauber.C written originally by B. Alver, M. Baker, 
 * C. Loizides, P. Steinberg, [arXiv:0805.4411](https://arxiv.org/abs/0805.4411) [nucl-ex]
 *
 * *********************************************************************/
#include "GlauberPlotter.hpp"

using CAP::MATH::pi;


namespace CAP
{
  void draw(const GlauberEvent & event)
  {
  int partLineColor = kRed;
  int partLineStyle = 1;
  int partLineWidth = 1;
  int partFillColor = kRed;
  int partFillStyle = 1002;

  int spectLineColor = kBlue;
  int spectLineStyle = 1;
  int spectLineWidth = 1;
  int spectFillColor = kBlue;
  int spectFillStyle = 1001;

  GlauberNucleus & nucleusA = event.nucleusA();
  GlauberNucleus & nucleusB = event.nucleusB();

  double r = 0.5*std::sqrt(xs/pi()/10.);
  TEllipse spectEllipse;
  spectEllipse.SetLineStyle(spectLineStyle);
  spectEllipse.SetLineWidth(spectLineWidth);
  spectEllipse.SetFillStyle(spectFillStyle);
  spectEllipse.setFillColor(spectFillColor);
  spectEllipse.setFillStyle(spectFillStyle);

  TEllipse partEllipse;
  partEllipse.SetLineStyle(partLineStyle);
  partEllipse.SetLineWidth(partLineWidth);
  partEllipse.SetFillStyle(partFillStyle);
  partEllipse.setFillColor(spectFillColor);
  partEllipse.setFillStyle(spectFillStyle);

  for (auto & nucleon : nucleusA.nucleons())
    {
    VectorLorentz & pos = nucleon.position();
    if (nucleon->isSpectator())
      spectEllipse.DrawEllipse(pos.x(),pos.y(),r,r,0,360,0,"");
    else
      partEllipse.DrawEllipse(pos.x(),pos.y(),r,r,0,360,0,"");
    }
  for (auto & nucleon : nucleusB.nucleons())
    {
    VectorLorentz & pos = nucleon.position();
    if (nucleon->isSpectator())
      spectEllipse.DrawEllipse(pos.x(),pos.y(),r,r,0,360,0,"");
    else
      partEllipse.DrawEllipse(pos.x(),pos.y(),r,r,0,360,0,"");
    }
  }




#endif



} // namespace CAP
