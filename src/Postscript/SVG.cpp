// @(#)root/postscript:$Id$
// Author: Olivier Couet

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifdef WIN32
#pragma optimize("",off)
#endif

#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>
#include <fstream>

#include "TROOT.hpp"
#include "TDatime.hpp"
#include "TBase64.hpp"
#include "Color.hpp"
#include "VirtualPad.hpp"
#include "Point2Ds.hpp"
#include "TSVG.hpp"
#include "TStyle.hpp"
#include "TMath.hpp"
#include "TObjString.hpp"
#include "TObjArray.hpp"
#include "snprintf.hpp"

int TSVG::fgLineJoin = 0;
int TSVG::fgLineCap  = 0;

const double kEpsilon = 1e-9;

/** \class TSVG
\ingroup PS

\brief Interface to SVG

[SVG](http://www.w3.org/Graphics/SVG/Overview.hpptm8)
(Scalable Vector Graphics) is a language for describing
two-dimensional graphics in XML. SVG allows high quality vector graphics in
HTML pages.

To print a ROOT canvas "c1" into an SVG file simply do:
~~~ {.cpp}
     c1->print("c1.svg");
~~~
The result is the ASCII file `c1.svg`.

It can be open directly using a web browser or included in a html document
the following way:
~~~ {.cpp}
<embed width="95%" height="500" src="c1.svg">
~~~
It is best viewed with Internet Explorer and you need the
[Adobe SVG Viewer](http://www.adobe.com/svg/viewer/install/main.hpptml)

To zoom using the Adobe SVG Viewer, position the mouse over
the area you want to zoom and click the right button.

To define the zoom area,
use Control+drag to mark the boundaries of the zoom area.

To pan, use Alt+drag.
By clicking with the right mouse button on the SVG graphics you will get
a pop-up menu giving other ways to interact with the image.

SVG files can be used directly in compressed mode to minimize the time
transfer over the network. Compressed SVG files should be created using
`gzip` on a normal ASCII SVG file and should then be renamed
using the file extension `.svgz`.
*/

////////////////////////////////////////////////////////////////////////////////
/// Default SVG constructor

TSVG::TSVG() : VirtualPS()
{
   gVirtualPS   = this;
   setTitle("SVG");
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize the SVG interface
///
///  - fname : SVG file name
///  - wtype : SVG workstation type. Not used in the SVG driver. But as TSVG
///            inherits from VirtualPS it should be kept. Anyway it is not
///            necessary to specify this parameter at creation time because it
///            has a default value (which is ignore in the SVG case).

TSVG::TSVG(const char *fname, int wtype, bool compact) : VirtualPS(fname, wtype)
{
   setTitle("SVG");
   fCompact = compact;
   open(fname, wtype);
}

////////////////////////////////////////////////////////////////////////////////
/// open a SVG file

void TSVG::open(const char *fname, int wtype)
{
   if (fStream) {
      Warning("open", "SVG file already open");
      return;
   }

   fLenBuffer = 0;
   _type      = std::abs(wtype);
   SetLineJoin(gStyle->GetJoinLinePS());
   SetLineCap(gStyle->GetCapLinePS());
   SetLineScale(gStyle->GetLineScalePS());
   gStyle->GetPaperSize(_xPageSize, _yPageSize);
   float xrange, yrange;
   if (gPad) {
      double ww = gPad->GetWw();
      double wh = gPad->GetWh();
      ww *= gPad->GetWNDC();
      wh *= gPad->GetHNDC();
      double ratio = wh/ww;
      xrange = _xPageSize;
      yrange = _xPageSize*ratio;
      if (yrange > _yPageSize) { yrange = _yPageSize; xrange = yrange/ratio;}
      _xPageSize = xrange; _yPageSize = yrange;
   }

   // open OS file
   if (!openStream(fname)) {
      Error("open", "Cannot open file: %s", fname);
      return;
   }

   gVirtualPS = this;

   ClearBuffer();

   fBoundingBox = kFALSE;

   _rangeDefined       = kFALSE;

   // Set a default range
   Range(_xPageSize, _yPageSize);

   newPage();
}

////////////////////////////////////////////////////////////////////////////////
/// Default SVG destructor

TSVG::~TSVG()
{
   close();
}

////////////////////////////////////////////////////////////////////////////////
/// close a SVG file

void TSVG::close(Option_t *)
{
   if (!gVirtualPS || !fStream)
      return;
   if (gPad)
      gPad->Update();
   printStr("</svg>@");

   // close file stream
   closeStream();

   gVirtualPS = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// Activate an already open SVG file

void TSVG::On()
{
   // _type is used to know if the SVG file is open. Unlike TPostScript, TSVG
   // has no "workstation type". In fact there is only one SVG type.

   if (!_type) {
      Error("On", "no SVG file open");
      Off();
      return;
   }
   gVirtualPS = this;
}

////////////////////////////////////////////////////////////////////////////////
/// Deactivate an already open SVG file

void TSVG::Off()
{
   gVirtualPS = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a Box

void TSVG::drawBox(double x1, double y1, double x2, double  y2)
{
   double x[4], y[4];
   double ix1 = XtoSVG(TMath::Min(x1,x2));
   double ix2 = XtoSVG(TMath::Max(x1,x2));
   double iy1 = YtoSVG(TMath::Min(y1,y2));
   double iy2 = YtoSVG(TMath::Max(y1,y2));
   int fillis = fFillStyle/1000;
   int fillsi = fFillStyle%1000;

   if (fillis == 3 || fillis == 2) {
      if (fillsi > 99) {
         x[0] = x1;   y[0] = y1;
         x[1] = x2;   y[1] = y1;
         x[2] = x2;   y[2] = y2;
         x[3] = x1;   y[3] = y2;
         return;
      }
      if (fillsi > 0 && fillsi < 26) {
         x[0] = x1;   y[0] = y1;
         x[1] = x2;   y[1] = y1;
         x[2] = x2;   y[2] = y2;
         x[3] = x1;   y[3] = y2;
         drawPS(-4, &x[0], &y[0]);
      }
      if (fillsi == -3) {
         printStr("@");
         printFast(9,"<rect x=\"");
         writeReal(ix1, kFALSE);
         printFast(5,"\" y=\"");
         writeReal(iy2, kFALSE);
         printFast(9,"\" width=\"");
         writeReal(ix2-ix1, kFALSE);
         printFast(10,"\" height=\"");
         writeReal(iy1-iy2, kFALSE);
         printFast(7,"\" fill=");
         setColorAlpha(5, kTRUE, kFALSE);
         printFast(2,"/>");
      }
   }
   if (fillis == 1) {
      printStr("@");
      printFast(9,"<rect x=\"");
      writeReal(ix1, kFALSE);
      printFast(5,"\" y=\"");
      writeReal(iy2, kFALSE);
      printFast(9,"\" width=\"");
      writeReal(ix2-ix1, kFALSE);
      printFast(10,"\" height=\"");
      writeReal(iy1-iy2, kFALSE);
      printFast(7,"\" fill=");
      setColorAlpha(fFillColor, kTRUE, kFALSE);
      printFast(2,"/>");
   }
   if (fillis == 0) {
      if (fLineWidth<=0) return;
      printStr("@");
      printFast(9,"<rect x=\"");
      writeReal(ix1, kFALSE);
      printFast(5,"\" y=\"");
      writeReal(iy2, kFALSE);
      printFast(9,"\" width=\"");
      writeReal(ix2-ix1, kFALSE);
      printFast(10,"\" height=\"");
      writeReal(iy1-iy2, kFALSE);
      printFast(21,"\" fill=\"none\" stroke=");
      setColorAlpha(fLineColor, kFALSE, kTRUE);
      printFast(2,"/>");
   }
}

////////////////////////////////////////////////////////////////////////////////
/// print a svg path statement for specified points

void TSVG::printPath(bool convert, int n, double *xps, double *yps, bool close_path)
{
   double idx = 0, idy = 0;

   double ixd0 = convert ? XtoSVG(xps[0]) : xps[0];
   double iyd0 = convert ? YtoSVG(yps[0]) : yps[0];
   double firstx = ixd0, firsty = iyd0;

   printFast(1,"M");
   writeReal(ixd0, kFALSE);
   printFast(1,",");
   writeReal(iyd0, kFALSE);

   for (int i = 1; i < n; i++) {
      double ixdi = convert ? XtoSVG(xps[i]) : xps[i];
      double iydi = convert ? YtoSVG(yps[i]) : yps[i];

      double ix   = ixdi - ixd0;
      double iy   = iydi - iyd0;

      if (fCompact && (TMath::Abs(ix) < kEpsilon))
         ix = 0;
      if (fCompact && (TMath::Abs(iy) < kEpsilon))
         iy = 0;

      ixd0 = ixdi;
      iyd0 = iydi;
      if(ix && iy) {
         if(idx) {
            MovePS(idx, 0);
            idx = 0;
         }
         if(idy) {
            MovePS(0, idy);
            idy = 0;
         }
         MovePS(ix, iy);
         continue;
      }
      if (ix) {
         if(idy) {
            MovePS(0, idy);
            idy = 0;
         }
         if(!idx || (ix*idx > 0)) {
            idx += ix;
         } else {
            MovePS(idx, 0);
            idx  = ix;
         }
         continue;
      }
      if(iy) {
         if(idx) {
            MovePS(idx, 0);
            idx = 0;
         }
         if(!idy || (iy*idy > 0)) {
            idy += iy;
         } else {
            MovePS(0, idy);
            idy  = iy;
         }
      }
   }
   if(idx)
      MovePS(idx, 0);
   if(idy)
      MovePS(0, idy);

   if (close_path || ((TMath::Abs(ixd0 - firstx) < kEpsilon) && (TMath::Abs(iyd0 - firsty) < kEpsilon)))
      printFast(1, "z");
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a Frame around a box
///
///  - mode = -1  the box looks as it is behind the screen
///  - mode =  1  the box looks as it is in front of the screen
///  - border is the border size in already pre-computed SVG units dark is the
///    color for the dark part of the frame light is the color for the light
///    part of the frame

void TSVG::drawFrame(double xl, double yl, double xt, double  yt,
                     int mode, int border, int dark, int light)
{
   double xps[7], yps[7];

   //- Draw top&left part of the box
   xps[0] = XtoSVG(xl);          yps[0] = YtoSVG(yl);
   xps[1] = xps[0] + border;     yps[1] = yps[0] - border;
   xps[2] = xps[1];              yps[2] = YtoSVG(yt) + border;
   xps[3] = XtoSVG(xt) - border; yps[3] = yps[2];
   xps[4] = XtoSVG(xt);          yps[4] = YtoSVG(yt);
   xps[5] = xps[0];              yps[5] = yps[4];
   xps[6] = xps[0];              yps[6] = yps[0];

   printStr("@");
   printFast(9,"<path d=\"");
   printPath(kFALSE, 7, xps, yps);
   printFast(7,"\" fill=");
   setColorAlpha(mode == -1 ? dark : light);
   printLineJointAttributes();
   printFast(2,"/>");

   //- Draw bottom&right part of the box
   xps[0] = XtoSVG(xl);          yps[0] = YtoSVG(yl);
   xps[1] = xps[0] + border;     yps[1] = yps[0] - border;
   xps[2] = XtoSVG(xt) - border; yps[2] = yps[1];
   xps[3] = xps[2];              yps[3] = YtoSVG(yt) + border;
   xps[4] = XtoSVG(xt);          yps[4] = YtoSVG(yt);
   xps[5] = xps[4];              yps[5] = yps[0];
   xps[6] = xps[0];              yps[6] = yps[0];

   printStr("@");
   printFast(9,"<path d=\"");
   printPath(kFALSE, 7, xps, yps);
   printFast(7,"\" fill=");
   setColorAlpha(mode == -1 ? light : dark);
   printLineJointAttributes();
   printFast(2,"/>");
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a PolyLine
///
///  Draw a polyline through  the points  xy.
///  - If NN=1 moves only to point x,y.
///  - If NN=0 the x,y are  written  in the SVG        file
///       according to the current transformation.
///  - If NN>0 the line is clipped as a line.
///  - If NN<0 the line is clipped as a fill area.

void TSVG::DrawPolyLine(int, Point2Ds *)
{
   Warning("DrawPolyLine", "not implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a PolyLine in NDC space
///
///  Draw a polyline through  the points  xy.
///  --If NN=1 moves only to point x,y.
///  --If NN=0 the x,y are  written  in the SVG        file
///       according to the current transformation.
///  --If NN>0 the line is clipped as a line.
///  - If NN<0 the line is clipped as a fill area.

void TSVG::DrawPolyLineNDC(int, Point2Ds *)
{
   Warning("DrawPolyLineNDC", "not implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// Implementation of polymarker printing

template<class T>
void TSVG::printPolyMarker(int n, T *xw, T* yw)
{
   fMarkerStyle = TMath::Abs(fMarkerStyle);
   int ms = TAttMarker::GetMarkerStyleBase(fMarkerStyle);

   if (ms == 4)
      ms = 24;
   else if (ms >= 6 && ms <= 8)
      ms = 20;
   else if (ms >= 9 && ms <= 19)
      ms = 1;

   // Define the marker size
   float msize  = fMarkerSize - TMath::Floor(TAttMarker::GetMarkerLineWidth(fMarkerStyle)/2.)/4.;
   if (fMarkerStyle == 1 || (fMarkerStyle >= 9 && fMarkerStyle <= 19)) msize = 0.01;
   if (fMarkerStyle == 6) msize = 0.02;
   if (fMarkerStyle == 7) msize = 0.04;

   const int kBASEMARKER = 8;
   float sbase = msize*kBASEMARKER;
   float s2x = sbase / float(gPad->GetWw() * gPad->GetAbsWNDC());
   msize = this->UtoSVG(s2x) - this->UtoSVG(0);

   double m  = msize;
   double m2 = m/2;
   double m3 = m/3;
   double m6 = m/6;
   double m4 = m/4.;
   double m8 = m/8.;
   double m0 = m/10.;

   // Draw the marker according to the type
   printStr("@");
   if ((ms > 19 && ms < 24) || ms == 29 || ms == 33 || ms == 34 ||
       ms == 39 || ms == 41 || ms == 43 || ms == 45 ||
       ms == 47 || ms == 48 || ms == 49) {
      printStr("<g fill=");
      setColorAlpha(int(fMarkerColor), kTRUE, kFALSE);
      printStr(">");
   } else {
      printStr("<g stroke=");
      setColorAlpha(int(fMarkerColor), kFALSE, kTRUE);
      printStr(" stroke-width=\"");
      writeReal(TMath::Max(1, int(TAttMarker::GetMarkerLineWidth(fMarkerStyle))), kFALSE);
      printStr("\" fill=\"none\"");
      printLineJointAttributes();
      printStr(">");
   }
   for (int i = 0; i < n; i++) {
      double ix = XtoSVG(xw[i]);
      double iy = YtoSVG(yw[i]);
      printStr("@");
      // Dot (.)
      if (ms == 1) {
         printStr("<line x1=\"");
         writeReal(ix-1, kFALSE);
         printStr("\" y1=\"");
         writeReal(iy, kFALSE);
         printStr("\" x2=\"");
         writeReal(ix, kFALSE);
         printStr("\" y2=\"");
         writeReal(iy, kFALSE);
         printStr("\"/>");
      // Plus (+)
      } else if (ms == 2) {
         printStr("<line x1=\"");
         writeReal(ix-m2, kFALSE);
         printStr("\" y1=\"");
         writeReal(iy, kFALSE);
         printStr("\" x2=\"");
         writeReal(ix+m2, kFALSE);
         printStr("\" y2=\"");
         writeReal(iy, kFALSE);
         printStr("\"/>");

         printStr("<line x1=\"");
         writeReal(ix, kFALSE);
         printStr("\" y1=\"");
         writeReal(iy-m2, kFALSE);
         printStr("\" x2=\"");
         writeReal(ix, kFALSE);
         printStr("\" y2=\"");
         writeReal(iy+m2, kFALSE);
         printStr("\"/>");
      // X shape (X)
      } else if (ms == 5) {
         printStr("<line x1=\"");
         writeReal(ix-m2*0.707, kFALSE);
         printStr("\" y1=\"");
         writeReal(iy-m2*0.707, kFALSE);
         printStr("\" x2=\"");
         writeReal(ix+m2*0.707, kFALSE);
         printStr("\" y2=\"");
         writeReal(iy+m2*0.707, kFALSE);
         printStr("\"/>");

         printStr("<line x1=\"");
         writeReal(ix-m2*0.707, kFALSE);
         printStr("\" y1=\"");
         writeReal(iy+m2*0.707, kFALSE);
         printStr("\" x2=\"");
         writeReal(ix+m2*0.707, kFALSE);
         printStr("\" y2=\"");
         writeReal(iy-m2*0.707, kFALSE);
         printStr("\"/>");
      // Asterisk shape (*)
      } else if (ms == 3 || ms == 31) {
         printStr("<line x1=\"");
         writeReal(ix-m2, kFALSE);
         printStr("\" y1=\"");
         writeReal(iy, kFALSE);
         printStr("\" x2=\"");
         writeReal(ix+m2, kFALSE);
         printStr("\" y2=\"");
         writeReal(iy, kFALSE);
         printStr("\"/>");

         printStr("<line x1=\"");
         writeReal(ix, kFALSE);
         printStr("\" y1=\"");
         writeReal(iy-m2, kFALSE);
         printStr("\" x2=\"");
         writeReal(ix, kFALSE);
         printStr("\" y2=\"");
         writeReal(iy+m2, kFALSE);
         printStr("\"/>");

         printStr("<line x1=\"");
         writeReal(ix-m2*0.707, kFALSE);
         printStr("\" y1=\"");
         writeReal(iy-m2*0.707, kFALSE);
         printStr("\" x2=\"");
         writeReal(ix+m2*0.707, kFALSE);
         printStr("\" y2=\"");
         writeReal(iy+m2*0.707, kFALSE);
         printStr("\"/>");

         printStr("<line x1=\"");
         writeReal(ix-m2*0.707, kFALSE);
         printStr("\" y1=\"");
         writeReal(iy+m2*0.707, kFALSE);
         printStr("\" x2=\"");
         writeReal(ix+m2*0.707, kFALSE);
         printStr("\" y2=\"");
         writeReal(iy-m2*0.707, kFALSE);
         printStr("\"/>");
      // Circle
      } else if (ms == 24 || ms == 20) {
         printStr("<circle cx=\"");
         writeReal(ix, kFALSE);
         printStr("\" cy=\"");
         writeReal(iy, kFALSE);
         printStr("\" r=\"");
         if (m2<=0) m2=1;
         writeReal(m2, kFALSE);
         printStr("\"/>");
      // Square
      } else if (ms == 25 || ms == 21) {
         printStr("<rect x=\"");
         writeReal(ix-m2, kFALSE);
         printStr("\" y=\"");
         writeReal(iy-m2, kFALSE);
         printStr("\" width=\"");
         writeReal(m, kFALSE);
         printStr("\" height=\"");
         writeReal(m, kFALSE);
         printStr("\"/>");
      // Down triangle
      } else if (ms == 26 || ms == 22) {
         printStr("<polygon points=\"");
         writeReal(ix); printStr(","); writeReal(iy-m2);
         writeReal(ix+m2); printStr(","); writeReal(iy+m2);
         writeReal(ix-m2); printStr(","); writeReal(iy+m2);
         printStr("\"/>");
      // Up triangle
      } else if (ms == 23 || ms == 32) {
         printStr("<polygon points=\"");
         writeReal(ix-m2); printStr(","); writeReal(iy-m2);
         writeReal(ix+m2); printStr(","); writeReal(iy-m2);
         writeReal(ix); printStr(","); writeReal(iy+m2);
         printStr("\"/>");
      // Diamond
      } else if (ms == 27 || ms == 33) {
         printStr("<polygon points=\"");
         writeReal(ix); printStr(","); writeReal(iy-m2);
         writeReal(ix+m3); printStr(","); writeReal(iy);
         writeReal(ix); printStr(","); writeReal(iy+m2);
         writeReal(ix-m3); printStr(","); writeReal(iy);
         printStr("\"/>");
      // Cross
      } else if (ms == 28 || ms == 34) {
         printStr("<polygon points=\"");
         writeReal(ix-m6); printStr(","); writeReal(iy-m6);
         writeReal(ix-m6); printStr(","); writeReal(iy-m2);
         writeReal(ix+m6); printStr(","); writeReal(iy-m2);
         writeReal(ix+m6); printStr(","); writeReal(iy-m6);
         writeReal(ix+m2); printStr(","); writeReal(iy-m6);
         writeReal(ix+m2); printStr(","); writeReal(iy+m6);
         writeReal(ix+m6); printStr(","); writeReal(iy+m6);
         writeReal(ix+m6); printStr(","); writeReal(iy+m2);
         writeReal(ix-m6); printStr(","); writeReal(iy+m2);
         writeReal(ix-m6); printStr(","); writeReal(iy+m6);
         writeReal(ix-m2); printStr(","); writeReal(iy+m6);
         writeReal(ix-m2); printStr(","); writeReal(iy-m6);
         printStr("\"/>");
      } else if (ms == 29 || ms == 30) {
         printStr("<polygon points=\"");
         writeReal(ix); printStr(","); writeReal(iy+m2);
         writeReal(ix+0.112255*m); printStr(","); writeReal(iy+0.15451*m);
         writeReal(ix+0.47552*m); printStr(","); writeReal(iy+0.15451*m);
         writeReal(ix+0.181635*m); printStr(","); writeReal(iy-0.05902*m);
         writeReal(ix+0.29389*m); printStr(","); writeReal(iy-0.40451*m);
         writeReal(ix); printStr(","); writeReal(iy-0.19098*m);
         writeReal(ix-0.29389*m); printStr(","); writeReal(iy-0.40451*m);
         writeReal(ix-0.181635*m); printStr(","); writeReal(iy-0.05902*m);
         writeReal(ix-0.47552*m); printStr(","); writeReal(iy+0.15451*m);
         writeReal(ix-0.112255*m); printStr(","); writeReal(iy+0.15451*m);
         printStr("\"/>");
      } else if (ms == 35) {
         printStr("<polygon points=\"");
         writeReal(ix-m2); printStr(","); writeReal(iy   );
         writeReal(ix   ); printStr(","); writeReal(iy-m2);
         writeReal(ix+m2); printStr(","); writeReal(iy   );
         writeReal(ix   ); printStr(","); writeReal(iy+m2);
         writeReal(ix-m2); printStr(","); writeReal(iy   );
         writeReal(ix+m2); printStr(","); writeReal(iy   );
         writeReal(ix   ); printStr(","); writeReal(iy+m2);
         writeReal(ix   ); printStr(","); writeReal(iy-m2);
         printStr("\"/>");
      } else if (ms == 36) {
         printStr("<polygon points=\"");
         writeReal(ix-m2); printStr(","); writeReal(iy-m2);
         writeReal(ix+m2); printStr(","); writeReal(iy-m2);
         writeReal(ix+m2); printStr(","); writeReal(iy+m2);
         writeReal(ix-m2); printStr(","); writeReal(iy+m2);
         writeReal(ix-m2); printStr(","); writeReal(iy-m2);
         writeReal(ix+m2); printStr(","); writeReal(iy+m2);
         writeReal(ix-m2); printStr(","); writeReal(iy+m2);
         writeReal(ix+m2); printStr(","); writeReal(iy-m2);
         printStr("\"/>");
      } else if (ms == 37 || ms == 39) {
         printStr("<polygon points=\"");
         writeReal(ix   ); printStr(","); writeReal(iy   );
         writeReal(ix+m4); printStr(","); writeReal(iy+m2);
         writeReal(ix-m4); printStr(","); writeReal(iy+m2);
         writeReal(ix   ); printStr(","); writeReal(iy   );
         writeReal(ix-m4); printStr(","); writeReal(iy-m2);
         writeReal(ix-m2); printStr(","); writeReal(iy);
         writeReal(ix   ); printStr(","); writeReal(iy   );
         writeReal(ix+m2); printStr(","); writeReal(iy   );
         writeReal(ix+m4); printStr(","); writeReal(iy-m2);
         writeReal(ix   ); printStr(","); writeReal(iy   );
         printStr("\"/>");
      } else if (ms == 38) {
         printStr("<polygon points=\"");
         writeReal(ix-m2); printStr(","); writeReal(iy   );
         writeReal(ix-m2); printStr(","); writeReal(iy-m4);
         writeReal(ix-m4); printStr(","); writeReal(iy-m2);
         writeReal(ix+m4); printStr(","); writeReal(iy-m2);
         writeReal(ix+m2); printStr(","); writeReal(iy-m4);
         writeReal(ix+m2); printStr(","); writeReal(iy+m4);
         writeReal(ix+m4); printStr(","); writeReal(iy+m2);
         writeReal(ix-m4); printStr(","); writeReal(iy+m2);
         writeReal(ix-m2); printStr(","); writeReal(iy+m4);
         writeReal(ix-m2); printStr(","); writeReal(iy   );
         writeReal(ix+m2); printStr(","); writeReal(iy   );
         writeReal(ix   ); printStr(","); writeReal(iy   );
         writeReal(ix   ); printStr(","); writeReal(iy-m2);
         writeReal(ix   ); printStr(","); writeReal(iy+m2);
         writeReal(ix   ); printStr(","); writeReal(iy);
         printStr("\"/>");
      } else if (ms == 40 || ms == 41) {
         printStr("<polygon points=\"");
         writeReal(ix   ); printStr(","); writeReal(iy   );
         writeReal(ix+m4); printStr(","); writeReal(iy+m2);
         writeReal(ix+m2); printStr(","); writeReal(iy+m4);
         writeReal(ix   ); printStr(","); writeReal(iy   );
         writeReal(ix+m2); printStr(","); writeReal(iy-m4);
         writeReal(ix+m4); printStr(","); writeReal(iy-m2);
         writeReal(ix   ); printStr(","); writeReal(iy   );
         writeReal(ix-m4); printStr(","); writeReal(iy-m2);
         writeReal(ix-m2); printStr(","); writeReal(iy-m4);
         writeReal(ix   ); printStr(","); writeReal(iy   );
         writeReal(ix-m2); printStr(","); writeReal(iy+m4);
         writeReal(ix-m4); printStr(","); writeReal(iy+m2);
         writeReal(ix   ); printStr(","); writeReal(iy   );
         printStr("\"/>");
      } else if (ms == 42 || ms == 43) {
         printStr("<polygon points=\"");
         writeReal(ix   ); printStr(","); writeReal(iy+m2);
         writeReal(ix-m8); printStr(","); writeReal(iy+m8);
         writeReal(ix-m2); printStr(","); writeReal(iy   );
         writeReal(ix-m8); printStr(","); writeReal(iy-m8);
         writeReal(ix   ); printStr(","); writeReal(iy-m2);
         writeReal(ix+m8); printStr(","); writeReal(iy-m8);
         writeReal(ix+m2); printStr(","); writeReal(iy   );
         writeReal(ix+m8); printStr(","); writeReal(iy+m8);
         writeReal(ix   ); printStr(","); writeReal(iy+m2);
         printStr("\"/>");
      } else if (ms == 44) {
         printStr("<polygon points=\"");
         writeReal(ix   ); printStr(","); writeReal(iy   );
         writeReal(ix+m4); printStr(","); writeReal(iy+m2);
         writeReal(ix-m4); printStr(","); writeReal(iy+m2);
         writeReal(ix+m4); printStr(","); writeReal(iy-m2);
         writeReal(ix-m4); printStr(","); writeReal(iy-m2);
         writeReal(ix   ); printStr(","); writeReal(iy   );
         writeReal(ix+m2); printStr(","); writeReal(iy+m4);
         writeReal(ix+m2); printStr(","); writeReal(iy-m4);
         writeReal(ix-m2); printStr(","); writeReal(iy+m4);
         writeReal(ix-m2); printStr(","); writeReal(iy-m4);
         writeReal(ix   ); printStr(","); writeReal(iy   );
         printStr("\"/>");
      } else if (ms == 45) {
         printStr("<polygon points=\"");
         writeReal(ix+m0); printStr(","); writeReal(iy+m0);
         writeReal(ix+m4); printStr(","); writeReal(iy+m2);
         writeReal(ix-m4); printStr(","); writeReal(iy+m2);
         writeReal(ix-m0); printStr(","); writeReal(iy+m0);
         writeReal(ix-m2); printStr(","); writeReal(iy+m4);
         writeReal(ix-m2); printStr(","); writeReal(iy-m4);
         writeReal(ix-m0); printStr(","); writeReal(iy-m0);
         writeReal(ix-m4); printStr(","); writeReal(iy-m2);
         writeReal(ix+m4); printStr(","); writeReal(iy-m2);
         writeReal(ix+m0); printStr(","); writeReal(iy-m0);
         writeReal(ix+m2); printStr(","); writeReal(iy-m4);
         writeReal(ix+m2); printStr(","); writeReal(iy+m4);
         writeReal(ix+m0); printStr(","); writeReal(iy+m0);
         printStr("\"/>");
      } else if (ms == 46 || ms == 47) {
         printStr("<polygon points=\"");
         writeReal(ix   ); printStr(","); writeReal(iy+m4);
         writeReal(ix-m4); printStr(","); writeReal(iy+m2);
         writeReal(ix-m2); printStr(","); writeReal(iy+m4);
         writeReal(ix-m4); printStr(","); writeReal(iy   );
         writeReal(ix-m2); printStr(","); writeReal(iy-m4);
         writeReal(ix-m4); printStr(","); writeReal(iy-m2);
         writeReal(ix   ); printStr(","); writeReal(iy-m4);
         writeReal(ix+m4); printStr(","); writeReal(iy-m2);
         writeReal(ix+m2); printStr(","); writeReal(iy-m4);
         writeReal(ix+m4); printStr(","); writeReal(iy   );
         writeReal(ix+m2); printStr(","); writeReal(iy+m4);
         writeReal(ix+m4); printStr(","); writeReal(iy+m2);
         writeReal(ix   ); printStr(","); writeReal(iy+m4);
         printStr("\"/>");
      } else if (ms == 48) {
         printStr("<polygon points=\"");
         writeReal(ix   ); printStr(","); writeReal(iy+m4*1.01);
         writeReal(ix-m4); printStr(","); writeReal(iy+m2);
         writeReal(ix-m2); printStr(","); writeReal(iy+m4);
         writeReal(ix-m4); printStr(","); writeReal(iy   );
         writeReal(ix-m2); printStr(","); writeReal(iy-m4);
         writeReal(ix-m4); printStr(","); writeReal(iy-m2);
         writeReal(ix   ); printStr(","); writeReal(iy-m4);
         writeReal(ix+m4); printStr(","); writeReal(iy-m2);
         writeReal(ix+m2); printStr(","); writeReal(iy-m4);
         writeReal(ix+m4); printStr(","); writeReal(iy   );
         writeReal(ix+m2); printStr(","); writeReal(iy+m4);
         writeReal(ix+m4); printStr(","); writeReal(iy+m2);
         writeReal(ix   ); printStr(","); writeReal(iy+m4*0.99);
         writeReal(ix+m4*0.99); printStr(","); writeReal(iy   );
         writeReal(ix   ); printStr(","); writeReal(iy-m4*0.99);
         writeReal(ix-m4*0.99); printStr(","); writeReal(iy   );
         writeReal(ix   ); printStr(","); writeReal(iy+m4*0.99);
         printStr("\"/>");
      } else if (ms == 49) {
         printStr("<polygon points=\"");
         writeReal(ix-m6); printStr(","); writeReal(iy-m6*1.01);
         writeReal(ix-m6); printStr(","); writeReal(iy-m2);
         writeReal(ix+m6); printStr(","); writeReal(iy-m2);
         writeReal(ix+m6); printStr(","); writeReal(iy-m6);
         writeReal(ix+m2); printStr(","); writeReal(iy-m6);
         writeReal(ix+m2); printStr(","); writeReal(iy+m6);
         writeReal(ix+m6); printStr(","); writeReal(iy+m6);
         writeReal(ix+m6); printStr(","); writeReal(iy+m2);
         writeReal(ix-m6); printStr(","); writeReal(iy+m2);
         writeReal(ix-m6); printStr(","); writeReal(iy+m6);
         writeReal(ix-m2); printStr(","); writeReal(iy+m6);
         writeReal(ix-m2); printStr(","); writeReal(iy-m6);
         writeReal(ix-m6); printStr(","); writeReal(iy-m6*0.99);
         writeReal(ix-m6); printStr(","); writeReal(iy+m6);
         writeReal(ix+m6); printStr(","); writeReal(iy+m6);
         writeReal(ix+m6); printStr(","); writeReal(iy-m6);
         printStr("\"/>");
      } else {
         printStr("<line x1=\"");
         writeReal(ix-1, kFALSE);
         printStr("\" y1=\"");
         writeReal(iy, kFALSE);
         printStr("\" x2=\"");
         writeReal(ix, kFALSE);
         printStr("\" y2=\"");
         writeReal(iy, kFALSE);
         printStr("\"/>");
      }
   }
   printStr("@");
   printStr("</g>");
}

////////////////////////////////////////////////////////////////////////////////
/// Paint PolyMarker

void TSVG::drawMarkers(int n, float *xw, float *yw)
{
   printPolyMarker<float>(n, xw, yw);
}


////////////////////////////////////////////////////////////////////////////////
/// Paint PolyMarker

void TSVG::drawMarkers(int n, double *xw, double *yw)
{
   printPolyMarker<double>(n, xw, yw);
}


////////////////////////////////////////////////////////////////////////////////
/// print line style attributes on the end of "path" string

void TSVG::printLineStyleOnEndOfPath()
{
   printFast(21,"\" fill=\"none\" stroke=");
   setColorAlpha(fLineColor, kFALSE, kTRUE);
   if(fLineWidth > 1.) {
      printFast(15," stroke-width=\"");
      writeReal(fLineWidth, kFALSE);
      printFast(1,"\"");
   }
   if (fLineStyle > 1) {
      printFast(19," stroke-dasharray=\"");
      TString st = (TString)gStyle->GetLineStyleString(fLineStyle);
      TObjArray *tokens = st.Tokenize(" ");
      for (int j = 0; j<tokens->GetEntries(); j++) {
         int it;
         sscanf(((TObjString*)tokens->At(j))->GetName(), "%d", &it);
         if (j>0) printFast(1,",");
         writeReal(it/4);
      }
      delete tokens;
      printFast(1,"\"");
   }
}

////////////////////////////////////////////////////////////////////////////////
/// print line join attributes - if present

void TSVG::printLineJointAttributes()
{
   if (fgLineJoin)
      printStr(TString::Format(" stroke-linejoin=\"%s\"", fgLineJoin == 1 ? "round" : "bevel"));
   if (fgLineCap)
      printStr(TString::Format(" stroke-linecap=\"%s\"", fgLineCap == 1 ? "round" : "square"));
}


////////////////////////////////////////////////////////////////////////////////
/// This function defines a path with xw and yw and draw it according the
/// value of nn:
///
///  - If nn>0 a line is drawn.
///  - If nn<0 a closed polygon is drawn.

void TSVG::drawPS(int nn, double *xw, double *yw)
{
   int n, fais = 0, fasi = 0;

   if (nn > 0) {
      if (fLineWidth <= 0)
         return;
      n = nn;
   } else {
      n = -nn;
      fais = fFillStyle/1000;
      fasi = fFillStyle%1000;
      if (fais == 3 || fais == 2) {
         if (fasi > 100 && fasi <125) {
            return;
         }
         if (fasi > 0 && fasi < 26) {
         }
      }
   }

   if(n <= 1) {
      Error("drawPS", "Two points are needed");
      return;
   }

   printStr("@");
   printFast(9,"<path d=\"");

   printPath(kTRUE, n, xw, yw, nn < 0);

   if (nn > 0)
      printLineStyleOnEndOfPath();
   else {
      printFast(7,"\" fill=");
      if (fais == 0) {
         printFast(14,"\"none\" stroke=");
         setColorAlpha(fFillColor, kFALSE, kTRUE);
      } else {
         setColorAlpha(fFillColor);
      }
   }
   printLineJointAttributes();
   printFast(2,"/>");
}

////////////////////////////////////////////////////////////////////////////////
/// This method draw N segments
void TSVG::DrawSegments(int n, double *xw, double *yw)
{
   if (fLineWidth < 0)
      return;

   if(n < 1) {
      Error("DrawSegments", "At least one segment has to be provided");
      return;
   }

   printStr("@");
   printFast(9,"<path d=\"");

   for(int i = 0; i < 2*n; i += 2) {
      double ixd0 = XtoSVG(xw[i]);
      double iyd0 = YtoSVG(yw[i]);
      double ixd1 = XtoSVG(xw[i+1]);
      double iyd1 = YtoSVG(yw[i+1]);

      double dx   = ixd1 - ixd0;
      double dy   = iyd1 - iyd0;

      if (fCompact && (TMath::Abs(dx) < kEpsilon))
         dx = 0;
      if (fCompact && (TMath::Abs(dy) < kEpsilon))
         dy = 0;

      if (dx || dy) {
         printFast(1,"M");
         writeReal(ixd0, kFALSE);
         printFast(1,",");
         writeReal(iyd0, kFALSE);
         MovePS(dx, dy);
      }
   }

   printLineStyleOnEndOfPath();
   printLineJointAttributes();
   printFast(2,"/>");
}


////////////////////////////////////////////////////////////////////////////////
/// Begin the cell Array painting

void TSVG::cellArrayBegin(int width, int height, double x1, double x2, double y1, double y2)
{
   double svgx1 = XtoSVG(x1);
   double svgx2 = XtoSVG(x1 + (x2 - x1) * width);
   double svgy1 = YtoSVG(y1);
   double svgy2 = YtoSVG(y1 - (y2 - y1) * height);

   printStr("@<g transform=\"translate(");
   writeReal(svgx1, kFALSE);
   writeReal(svgy1, kTRUE);
   printStr(") scale(");
   writeReal((svgx2 - svgx1) / width, kFALSE);
   writeReal((svgy2 - svgy1) / height, kTRUE);
   printStr(")\">@");
   printStr(TString::Format("<image width=\"%d\" height=\"%d\" href=\"data:image/png;base64,", width, height));
}

////////////////////////////////////////////////////////////////////////////////
/// Paint the cell Array as single pixel

void TSVG::cellArrayFill(int, int, int)
{
   Warning("cellArrayFill", "not implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// Paint the cell Array as png image
/// Disabled in compact mode to avoid creation of large SVG files

void TSVG::cellArrayPng(char *buffer, int size)
{
   if (!fCompact) {
      TString base64 = TBase64::Encode(reinterpret_cast<char *>(buffer), size);
      printFast(base64.Length(), base64.Data());
   }
}

////////////////////////////////////////////////////////////////////////////////
/// End the cell Array painting

void TSVG::cellArrayEnd()
{
   printStr("\"></image>@");
   printStr("</g>@");
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize the SVG file. The main task of the function is to output the
/// SVG header file which consist in `<title>`, `<desc>` and `<defs>`. The
/// HeaderPS provided by the user program is written in the `<defs>` part.

void TSVG::Initialize()
{
   // Title
   printStr("<title>@");
   printStr(GetName());
   printStr("@");
   printStr("</title>@");

   if (fCompact)
      return;

   // Description
   printStr("<desc>@");
   printFast(22,"Creator: ROOT Version ");
   printStr(gROOT->GetVersion());
   printStr("@");
   printFast(14,"CreationDate: ");
   TDatime t;
   printStr(t.AsString());
   //Check a special header is defined in the current style
   int nh = strlen(gStyle->GetHeaderPS());
   if (nh) {
      printFast(nh,gStyle->GetHeaderPS());
   }
   printStr("</desc>@");

   // Definitions
   printStr("<defs>@");
   printStr("</defs>@");

}

////////////////////////////////////////////////////////////////////////////////
/// write float value into output stream
/// In compact form try to store only first 2-3 significant digits

void TSVG::writeReal(float r, bool space)
{
   if (fCompact) {
      auto a = std::abs(r);
      if (a > 10)
         VirtualPS::writeInteger(std::lround(r), space);
      else if (a < 0.005)
         VirtualPS::writeReal(r, space);
      else {
         char str[15];
         snprintf(str, 15, (a > 1) ? "%3.1f" : "%5.3f", r);
         if(space)
            printFast(1," ");
         printStr(str);
      }
   } else
      VirtualPS::writeReal(r, space);
}

////////////////////////////////////////////////////////////////////////////////
/// Move to a new position (ix, iy). The move is done in relative coordinates
/// which allows to have short numbers which decrease the size of the file.
/// This function use the full power of the SVG's paths by using the
/// horizontal and vertical move whenever it is possible.

void TSVG::MovePS(double ix, double iy)
{
   if (ix != 0 && iy != 0)  {
      printFast(1,"l");
      writeReal(ix);
      printFast(1,",");
      writeReal(iy);
   } else if (ix != 0)  {
      printFast(1,"h");
      writeReal(ix);
   } else if (iy != 0)  {
      printFast(1,"v");
      writeReal(iy);
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Start the SVG page. This function initialize the pad conversion
/// coefficients and output the `<svg>` directive which is close later in the
/// the function close.

void TSVG::newPage()
{
   // Compute pad conversion coefficients
   if (gPad) {
      double ww   = gPad->GetWw();
      double wh   = gPad->GetWh();
      _yPageSize        = _xPageSize*wh/ww;
   } else {
      _yPageSize = 27;
   }

   // <svg> directive. It defines the viewBox.
   if(!fBoundingBox) {
      printStr("@<?xml version=\"1.0\" standalone=\"no\"?>");
      printStr("@<svg width=\"");
      writeReal(CMtoSVG(_xPageSize), kFALSE);
      printStr("\" height=\"");
      _yPageSizeSVG = CMtoSVG(_yPageSize);
      writeReal(_yPageSizeSVG, kFALSE);
      printStr("\" viewBox=\"0 0");
      writeReal(CMtoSVG(_xPageSize));
      writeReal(_yPageSizeSVG);
      printStr("\" xmlns=\"http://www.w3.org/2000/svg\" shape-rendering=\"crispEdges\">");
      printStr("@");
      Initialize();
      fBoundingBox  = kTRUE;
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Set the range for the paper in centimetres

void TSVG::Range(float xsize, float ysize)
{
   _xPageSize = xsize;
   _yPageSize = ysize;
   _rangeDefined = kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Set color index for fill areas

void TSVG::SetFillColor( Color_t cindex )
{
   fFillColor = cindex;
}

////////////////////////////////////////////////////////////////////////////////
/// Set color index for lines

void TSVG::SetLineColor( Color_t cindex )
{
   fLineColor = cindex;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the value of the global parameter TSVG::fgLineJoin.
/// This parameter determines the appearance of joining lines in a SVG
/// output.
/// It takes one argument which may be:
///   - 0 (miter join)
///   - 1 (round join)
///   - 2 (bevel join)
/// The default value is 0 (miter join).
///
/// \image html postscript_1.png
///
/// To change the line join behaviour just do:
/// ~~~ {.cpp}
/// gStyle->SetJoinLinePS(2); // Set the PS line join to bevel.
/// ~~~

void TSVG::SetLineJoin( int linejoin )
{
   fgLineJoin = linejoin;
   if (fgLineJoin<0) fgLineJoin=0;
   if (fgLineJoin>2) fgLineJoin=2;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the value of the global parameter TSVG::fgLineCap.
/// This parameter determines the appearance of line caps in a SVG
/// output.
/// It takes one argument which may be:
///   - 0 (butt caps)
///   - 1 (round caps)
///   - 2 (projecting caps)
/// The default value is 0 (butt caps).
///
/// \image html postscript_2.png
///
/// To change the line cap behaviour just do:
/// ~~~ {.cpp}
/// gStyle->SetCapLinePS(2); // Set the PS line cap to projecting.
/// ~~~

void TSVG::SetLineCap( int linecap )
{
   fgLineCap = linecap;
   if (fgLineCap<0) fgLineCap=0;
   if (fgLineCap>2) fgLineCap=2;
}

////////////////////////////////////////////////////////////////////////////////
/// Change the line style
///
///  - linestyle = 2 dashed
///  - linestyle = 3 dotted
///  - linestyle = 4 dash-dotted
///  - linestyle = else solid (1 in is used most of the time)

void TSVG::SetLineStyle(Style_t linestyle)
{
   fLineStyle = linestyle;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the lines width.

void TSVG::SetLineWidth(Width_t linewidth)
{
   fLineWidth = linewidth;
}

////////////////////////////////////////////////////////////////////////////////
/// Set color index for markers.

void TSVG::SetMarkerColor( Color_t cindex )
{
   fMarkerColor = cindex;
}

////////////////////////////////////////////////////////////////////////////////
/// Set RGBa color with its color index

void TSVG::setColorAlpha(int color, bool fill, bool stroke)
{
   if (color < 0)
      color = 0;
   Color *col = gROOT->GeColor(color);
   if (col) {
      setColor(col->GetRed(), col->GetGreen(), col->GetBlue());
      float a = col->GetAlpha();
      if ((a < 1.) && fill)
         printStr(TString::Format(" fill-opacity=\"%3.2f\"",a));
      if ((a < 1.) && stroke)
         printStr(TString::Format(" stroke-opacity=\"%3.2f\"",a));
   } else {
      setColor(1., 1., 1.);
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Set RGB (without alpha channel) color with its color index

void TSVG::setColor(int color)
{
   if (color < 0) color = 0;
   Color *col = gROOT->GeColor(color);
   if (col) {
      setColor(col->GetRed(), col->GetGreen(), col->GetBlue());
   } else {
      setColor(1., 1., 1.);
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Set color with its R G B components
///
///  - r: % of red in [0,1]
///  --g: % of green in [0,1]
///  - b: % of blue in [0,1]

void TSVG::setColor(float r, float g, float b)
{
   if (r <= 0. && g <= 0. && b <= 0. ) {
      printFast(7,"\"black\"");
   } else if (r >= 1. && g >= 1. && b >= 1. ) {
      printFast(7,"\"white\"");
   } else {
      char str[12];
      snprintf(str,12,"\"#%2.2x%2.2x%2.2x\"",int(255.*r)
                                            ,int(255.*g)
                                            ,int(255.*b));
      printStr(str);
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Set color index for text

void TSVG::SeTexColor( Color_t cindex )
{
   fTexColor = cindex;
}

////////////////////////////////////////////////////////////////////////////////
/// Draw text
///
///  - xx: x position of the text
///  - yy: y position of the text
///  - chars: text to be drawn

void TSVG::text(double xx, double yy, const char *chars)
{
   static const char *fontFamily[] = {
   "Times"    , "Times"    , "Times",
   "Helvetica", "Helvetica", "Helvetica"   , "Helvetica",
   "Courier"  , "Courier"  , "Courier"     , "Courier",
   "Times"    ,"Times"     , "ZapfDingbats", "Times"};

   static const char *fontWeight[] = {
   "normal", "bold", "bold",
   "normal", "normal", "bold"  , "bold",
   "normal", "normal", "bold"  , "bold",
   "normal", "normal", "normal", "normal"};

   static const char *fontStyle[] = {
   "italic", "normal" , "italic",
   "normal", "oblique", "normal", "oblique",
   "normal", "oblique", "normal", "oblique",
   "normal", "normal" , "normal", "italic"};

   double ix    = XtoSVG(xx);
   double iy    = YtoSVG(yy);
   double txalh = fTextAlign/10;
   if (txalh <1) txalh = 1; else if (txalh > 3) txalh = 3;
   double txalv = fTextAlign%10;
   if (txalv <1) txalv = 1; else if (txalv > 3) txalv = 3;

   double     wh = (double)gPad->GetPadWidth();
   double     hh = (double)gPad->GetPadHeight();
   float fontrap = 1.09; //scale down compared to X11
   float ftsize;

   int font  = abs(fTextFont)/10;
   if (font > 15 || font < 1)
      font = 1;
   if (wh < hh) {
      ftsize = fTextSize*_xPageSize*gPad->GetAbsWNDC();
   } else {
      ftsize = fTextSize*_yPageSize*gPad->GetAbsHNDC();
   }
   int ifont = font-1;

   double fontsize = CMtoSVG(ftsize/fontrap);
   if( fontsize <= 0) return;

   if (txalv == 3) iy = iy+fontsize;
   if (txalv == 2) iy = iy+(fontsize/2);

   if (fTextAngle != 0.) {
      printStr("@");
      printFast(21,"<g transform=\"rotate(");
      writeReal(-fTextAngle, kFALSE);
      printFast(1,",");
      writeReal(ix, kFALSE);
      printFast(1,",");
      writeReal(iy, kFALSE);
      printFast(3,")\">");
   }

   printStr("@");
   printFast(30,"<text xml:space=\"preserve\" x=\"");
   writeReal(ix, kFALSE);
   printFast(5,"\" y=\"");
   writeReal(iy, kFALSE);
   printFast(1,"\"");
   if (txalh == 2) {
      printFast(21," text-anchor=\"middle\"");
   } else if (txalh == 3) {
      printFast(18," text-anchor=\"end\"");
   }
   printFast(6," fill=");
   setColorAlpha(int(fTexColor), kTRUE, kFALSE);
   printFast(12," font-size=\"");
   writeReal(fontsize, kFALSE);
   printFast(15,"\" font-family=\"");
   printStr(fontFamily[ifont]);
   if (strcmp(fontWeight[ifont],"normal")) {
      printFast(15,"\" font-weight=\"");
      printStr(fontWeight[ifont]);
   }
   if (strcmp(fontStyle[ifont],"normal")) {
      printFast(14,"\" font-style=\"");
      printStr(fontStyle[ifont]);
   }
   printFast(2,"\">");

   if (font == 12 || font == 15) {
      int ichar = chars[0]+848;
      int ic    = ichar;

      // Math Symbols (cf: http://www.fileformat.info/info/unicode/category/Sm/list.hpptm)
      if (ic == 755) ichar =  8804;
      if (ic == 759) ichar =  9827;
      if (ic == 760) ichar =  9830;
      if (ic == 761) ichar =  9829;
      if (ic == 762) ichar =  9824;
      if (ic == 766) ichar =  8594;
      if (ic == 776) ichar =   247;
      if (ic == 757) ichar =  8734;
      if (ic == 758) ichar =   402;
      if (ic == 771) ichar =  8805;
      if (ic == 774) ichar =  8706;
      if (ic == 775) ichar =  8226;
      if (ic == 779) ichar =  8776;
      if (ic == 805) ichar =  8719;
      if (ic == 821) ichar =  8721;
      if (ic == 834) ichar =  8747;
      if (ic == 769) ichar =   177;
      if (ic == 772) ichar =   215;
      if (ic == 768) ichar =   176;
      if (ic == 791) ichar =  8745;
      if (ic == 793) ichar =  8835; // SUPERSET OF
      if (ic == 794) ichar =  8839; // SUPERSET OF OR EQUAL TO
      if (ic == 795) ichar =  8836; // NOT A SUBSET OF
      if (ic == 796) ichar =  8834;
      if (ic == 893) ichar =  8722;
      if (ic == 803) ichar =   169; // COPYRIGHT SIGN
      if (ic == 819) ichar =   169; // COPYRIGHT SIGN
      if (ic == 804) ichar =  8482;
      if (ic == 770) ichar =    34;
      if (ic == 823) ichar = 10072;
      if (ic == 781) ichar = 10072;
      if (ic == 824) ichar =  9117; // LEFT PARENTHESIS LOWER HOOK
      if (ic == 822) ichar =  9115; // LEFT PARENTHESIS UPPER HOOK
      if (ic == 767) ichar =  8595; // DOWNWARDS ARROW
      if (ic == 763) ichar =  8596; // LEFT RIGHT ARROW
      if (ic == 764) ichar =  8592; // LEFTWARDS ARROW
      if (ic == 788) ichar =  8855; // CIRCLED TIMES
      if (ic == 784) ichar =  8501;
      if (ic == 777) ichar =  8800;
      if (ic == 797) ichar =  8838;
      if (ic == 800) ichar =  8736;
      if (ic == 812) ichar =  8656; // LEFTWARDS DOUBLE ARROW
      if (ic == 817) ichar =    60; // LESS-THAN SIGN
      if (ic == 833) ichar =    62; // GREATER-THAN SIGN
      if (ic == 778) ichar =  8803; // STRICTLY EQUIVALENT TO
      if (ic == 809) ichar =  8743; // LOGICAL AND
      if (ic == 802) ichar =  9415; // CIRCLED LATIN CAPITAL LETTER R
      if (ic == 780) ichar =  8230; // HORIZONTAL ELLIPSIS
      if (ic == 801) ichar =  8711; // NABLA
      if (ic == 783) ichar =  8629; // DOWNWARDS ARROW WITH CORNER LEFTWARDS
      if (ic == 782) ichar =  8213;
      if (ic == 799) ichar =  8713;
      if (ic == 792) ichar =  8746;
      if (ic == 828) ichar =  9127;
      if (ic == 765) ichar =  8593; // UPWARDS ARROW
      if (ic == 789) ichar =  8853; // CIRCLED PLUS
      if (ic == 813) ichar =  8657; // UPWARDS DOUBLE ARROW
      if (ic == 773) ichar =  8733; // PROPORTIONAL TO
      if (ic == 790) ichar =  8709; // EMPTY SET
      if (ic == 810) ichar =  8744;
      if (ic == 756) ichar =  8260;
      if (ic == 807) ichar =  8231;
      if (ic == 808) ichar =  8989; // TOP RIGHT CORNER
      if (ic == 814) ichar =  8658; // RIGHTWARDS DOUBLE ARROW
      if (ic == 806) ichar =  8730; // SQUARE ROOT
      if (ic == 827) ichar =  9123;
      if (ic == 829) ichar =  9128;
      if (ic == 786) ichar =  8476;
      if (ic == 785) ichar =  8465;
      if (ic == 787) ichar =  8472;
      if (ic == 882) ichar =  8704; // FOR ALL
      if (ic == 884) ichar =  8707; // THERE EXISTS
      if (ic == 815) ichar =  8659; // DOWNWARDS DOUBLE ARROW
      if (ic == 811) ichar =  8660; // LEFT RIGHT DOUBLE ARROW
      if (ic == 826) ichar =  124;  // VERTICAL LINE
      if (ic == 818) ichar =  9415; // CIRCLED LATIN CAPITAL LETTER R
      if (ic == 820) ichar =  8482; // TRADEMARK SIGN

      // Greek characters
      if (ic == 918) ichar = 934;
      if (ic == 919) ichar = 915;
      if (ic == 920) ichar = 919;
      if (ic == 923) ichar = 922;
      if (ic == 924) ichar = 923;
      if (ic == 925) ichar = 924;
      if (ic == 926) ichar = 925;
      if (ic == 929) ichar = 920;
      if (ic == 930) ichar = 929;
      if (ic == 936) ichar = 926;
      if (ic == 915) ichar = 935;
      if (ic == 937) ichar = 936;
      if (ic == 935) ichar = 937;
      if (ic == 938) ichar = 918;
      if (ic == 951) ichar = 947;
      if (ic == 798) ichar = 8712; // ELEMENT OF, TeX traditionally assigns \epsilon to GREEK LUNATE EPSILON SYMBOL (ϵ), compared to the curly \varepsilon to the GREEK SMALL LETTER EPSILON (Ɛ)
      if (ic == 970) ichar = 950;
      if (ic == 952) ichar = 951;
      if (ic == 961) ichar = 952;
      if (ic == 955) ichar = 954;
      if (ic == 956) ichar = 955;
      if (ic == 957) ichar = 956;
      if (ic == 958) ichar = 957;
      if (ic == 968) ichar = 958;
      if (ic == 934) ichar = 962;
      if (ic == 962) ichar = 961;
      if (ic == 966) ichar = 969;
      if (ic == 950) ichar = 966;
      if (ic == 947) ichar = 967;
      if (ic == 969) ichar = 968;
      if (ic == 967) ichar = 969;
      if (ic == 954) ichar = 966;
      if (ic == 922) ichar = 952;
      if (ic == 753) ichar = 965;
      printStr(TString::Format("&#%4.4d;",ichar));
   } else {
      int len=strlen(chars);
      for (int i=0; i<len;i++) {
         if (chars[i]!='\n') {
            if (chars[i]=='<') {
               printFast(4,"&lt;");
            } else if (chars[i]=='>') {
               printFast(4,"&gt;");
            } else if (chars[i]=='\305') {
               printFast(7,"&#8491;"); // ANGSTROM SIGN
            } else if (chars[i]=='\345') {
               printFast(6,"&#229;");
            } else if (chars[i]=='&') {
               printFast(5,"&amp;");
            } else {
               printFast(1,&chars[i]);
            }
         }
      }
   }

   printStr("@");
   printFast(7,"</text>");

   if (fTextAngle != 0.) {
      printStr("@");
      printFast(4,"</g>");
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Draw text with URL.
///

void TSVG::textUrl(double x, double y, const char *chars, const char *url)
{
   printStr("@");
   printFast(9,"<a href=\"");
   printStr(url);
   printFast(2,"\">");
   printStr("@");
   text(x, y, chars);
   printStr("@");
   printFast(4,"</a>");
   printStr("@");
}

////////////////////////////////////////////////////////////////////////////////
/// write a string of characters in NDC

void TSVG::textNDC(double u, double v, const char *chars)
{
   double x = gPad->GetX1() + u*(gPad->GetX2() - gPad->GetX1());
   double y = gPad->GetY1() + v*(gPad->GetY2() - gPad->GetY1());
   text(x, y, chars);
}

////////////////////////////////////////////////////////////////////////////////
/// Convert U from NDC coordinate to SVG

double TSVG::UtoSVG(double u)
{
   double cm = _xPageSize*(gPad->GetAbsXlowNDC() + u*gPad->GetAbsWNDC());
   return 0.5 + 72*cm/2.54;
}

////////////////////////////////////////////////////////////////////////////////
/// Convert V from NDC coordinate to SVG

double TSVG::VtoSVG(double v)
{
   double cm = _yPageSize*(gPad->GetAbsYlowNDC() + v*gPad->GetAbsHNDC());
   return 0.5 + 72*cm/2.54;
}

////////////////////////////////////////////////////////////////////////////////
/// Convert X from world coordinate to SVG

double TSVG::XtoSVG(double x)
{
   double u = (x - gPad->GetX1())/(gPad->GetX2() - gPad->GetX1());
   return  UtoSVG(u);
}

////////////////////////////////////////////////////////////////////////////////
/// Convert Y from world coordinate to SVG

double TSVG::YtoSVG(double y)
{
   double v = (y - gPad->GetY1())/(gPad->GetY2() - gPad->GetY1());
   return  _yPageSizeSVG-VtoSVG(v);
}

////////////////////////////////////////////////////////////////////////////////
/// Not needed in SVG case

void TSVG::drawPS(int, float *, float *)
{
   Warning("TSVG::drawPS", "not yet implemented");
}
