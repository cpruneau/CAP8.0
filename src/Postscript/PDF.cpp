// @(#)root/postscript:$Id: PDF.cxx,v 1.0
// Author: Olivier Couet

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/** \class PDF
\ingroup PS

\brief Interface to PDF.

Like PostScript, PDF is a vector graphics output format allowing a very high
graphics output quality. The functionalities provided by this class are very
similar to those provided by `TPostScript`.

Compare to PostScript output, the PDF files are usually smaller because some
parts of them can be compressed.

PDF also allows to define table of contents. This facility can be used in ROOT.
The following example shows how to proceed:
~~~ {.cpp}
{
   TCanvas* canvas = new TCanvas("canvas");
   TH1F* histo = new TH1F("histo","test 1",10,0.,10.);
   histo->SetFillColor(2);
   histo->Fill(2.);
   histo->Draw();
   canvas->print("plots.pdf(","Title:One bin filled");
   histo->Fill(4.);
   histo->Draw();
   canvas->print("plots.pdf","Title:Two bins filled");
   histo->Fill(6.);
   histo->Draw();
   canvas->print("plots.pdf","Title:Three bins filled");
   histo->Fill(8.);
   histo->Draw();
   canvas->print("plots.pdf","Title:Four bins filled");
   histo->Fill(8.);
   histo->Draw();
   canvas->print("plots.pdf)","Title:The fourth bin content is 2");
}
~~~
Each character string following the keyword "Title:" makes a new entry in
the table of contents.
*/

#ifdef WIN32
#pragma optimize("",off)
#endif

#include <cstdlib>
#include <cstring>
#include <cctype>
#include <fstream>

#include "TROOT.hpp"
#include "TDatime.hpp"
#include "Color.hpp"
#include "VirtualPad.hpp"
#include "Point2Ds.hpp"
#include "PDF.hpp"
#include "TStyle.hpp"
#include "TMath.hpp"
#include "TStorage.hpp"
#include "text.hpp"
#include "zlib.hpp"
#include "TObjString.hpp"
#include "TObjArray.hpp"
#include "snprintf.hpp"

// To scale fonts to the same size as the old TT version
const float kScale = 0.93376068;

// Objects numbers
const int kObjRoot             =  1; // Root object
const int kObjInfo             =  2; // Info object
const int kObjOutlines         =  3; // Outlines object
const int kObjPages            =  4; // Pages object (pages index)
const int kObjPageResources    =  5; // Pages Resources object
const int kObjContents         =  6; // Table of content
const int kObjFont             =  7; // First Font object (14 in total)
const int kObjColorSpace       = 22; // ColorSpace object
const int kObjPatternResourses = 23; // Pattern Resources object
const int kObjPatternList      = 24; // Pattern list object
const int kObjTransList        = 25; // List of transparencies
const int kObjPattern          = 26; // First pattern object (25 in total)
const int kObjFirstPage        = 51; // First page object

// Number of fonts
const int kNumberOfFonts = 15;

int PDF::fgLineJoin = 0;
int PDF::fgLineCap = 0;


//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Default PDF constructor

PDF::PDF()
: VirtualPS()
{
   _currentPage = kObjFirstPage;
   setTitle("PDF");
   gVirtualPS = this;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Initialize the PDF interface
//!
//!  - fname : PDF file name
//!  - wtype : PDF workstation type. Not used in the PDF driver. But as PDF
//!            inherits from VirtualPS it should be kept. Anyway it is not
//!            necessary to specify this parameter at creation time because it
//!            has a default value (which is ignore in the PDF case).

PDF::PDF(const String & fileName, int type)
: VirtualPS(fileName, type)
{
   _currentPage = kObjFirstPage;
   setTitle("PDF");
   open(fileName, type);
}

PDF::~PDF()
{
   close();
}



void PDF::close()
{
   if (!gVirtualPS || !fStream) return;
   writeCompressedBuffer();
   printStr("endstream@");
   int streamLength = fNByte-fStartStream-10;
   endObject();
   newObject(_currentPage + 2);
   writeInteger(streamLength, false);
   printStr("@");
   endObject();
   newObject(_currentPage + 3);
   printStr("<<@");
   if (!strstr(title(),"PDF"))
     {
      printStr("/Title (");
      printStr(title());
      printStr(")@");
     }
   else
     {
      printStr("/Title (Page");
      writeInteger(_pageCount);
      printStr(")@");
     }
   printStr("/Dest [");
   writeInteger(_currentPage);
   printStr(" 0 R /XYZ null null 0]@");
   printStr("/Parent");
   writeInteger(kObjContents);
   printStr(" 0 R");
   printStr("@");
   if (_pageCount > 1)
     {
      printStr("/Prev");
      writeInteger(_pageObjects[_pageCount - 2] + 3);
      printStr(" 0 R");
      printStr("@");
     }
   printStr(">>@");
   endObject();
   writeUrlObjects();
   printStr("@");
   newObject(kObjOutlines);
   printStr("<<@");
   printStr("/Type /Outlines@");
   printStr("/Count");
   writeInteger(_pageCount+1);
   printStr("@");
   printStr("/First");
   writeInteger(kObjContents);
   printStr(" 0 R");
   printStr("@");
   printStr("/Last");
   writeInteger(kObjContents);
   printStr(" 0 R");
   printStr("@");
   printStr(">>@");
   endObject();

   newObject(kObjContents);
   printStr("<<@");
   printStr("/Title (Contents)@");
   printStr("/Dest [");
   writeInteger(kObjFirstPage);
   printStr(" 0 R /XYZ null null 0]@");
   printStr("/Count");
   writeInteger(_pageCount);
   printStr("@");
   printStr("/Parent");
   writeInteger(kObjOutlines);
   printStr(" 0 R");
   printStr("@");
   printStr("/First");
   writeInteger(kObjFirstPage+3);
   printStr(" 0 R");
   printStr("@");
   printStr("/Last");
   writeInteger(_pageObjects[_pageCount - 1] + 3);
   printStr(" 0 R");
   printStr("@");
   printStr(">>@");
   endObject();

   // List of all the pages
   newObject(kObjPages);
   printStr("<<@");
   printStr("/Type /Pages@");
   printStr("/Count");
   writeInteger(_pageCount);
   printStr("@");
   printStr("/Kids [");
   for (std::size_t i = 0; i < _pageObjects.size(); i++) {
      writeInteger(_pageObjects[i]);
      printStr(" 0 R");
   }
   printStr(" ]");
   printStr("@");
   printStr(">>@");
   endObject();

   if (!_pageObjects.empty())
      _pageObjects.clear();
   if (!_hasURLs.empty())
      _hasURLs.clear();
   if (!fRectX1.empty())
      fRectX1.clear();
   if (!fRectY1.empty())
      fRectY1.clear();
   if (!fRectX2.empty())
      fRectX2.clear();
   if (!fRectY2.empty())
      fRectY2.clear();

   // List of transparencies
   newObject(kObjTransList);
   printStr("<<@");
   for (std::size_t i = 0; i < _alphas.size(); i++) {
      printStr(
      TString::Format("/ca%3.2f << /Type /ExtGState /ca %3.2f >> /CA%3.2f << /Type /ExtGState /CA %3.2f >>@",
      _alphas[i],_alphas[i],_alphas[i],_alphas[i]));
   }
   printStr(">>@");
   endObject();
   if (!_alphas.empty())
      _alphas.clear();

   // Cross-Reference Table
   int refInd = fNByte;
   printStr("xref@");
   printStr("0");
   writeInteger(_objectPos.size() + 1);
   printStr("@");
   printStr("0000000000 65535 f @");
   char str[21];
   for (std::size_t i = 0; i < _objectPos.size(); ++i) {
      snprintf(str,21,"%10.10d 00000 n @", _objectPos[i]);
      printStr(str);
   }

   // Trailer
   printStr("trailer@");
   printStr("<<@");
   printStr("/Size");
   writeInteger(_objectPos.size() + 1);
   printStr("@");
   printStr("/Root");
   writeInteger(kObjRoot);
   printStr(" 0 R");
   printStr("@");
   printStr("/Info");
   writeInteger(kObjInfo);
   printStr(" 0 R@");
   printStr(">>@");
   printStr("startxref@");
   writeInteger(refInd, false);
   printStr("@");
   printStr("%%EOF@");
   closeStream();
   gVirtualPS = nullptr;
}

void PDF::drawBox(const Point2D & p1, const Point2D & p2) //double x1, double y1, double x2, double  y2)
{
   static double x[4], y[4];
   double ix1 = XtoPDF(x1);
   double ix2 = XtoPDF(x2);
   double iy1 = YtoPDF(y1);
   double iy2 = YtoPDF(y2);
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
         setColor(5);
         if (_alpha == 1) printFast(15," q 0.4 w [] 0 d");
         writeReal(ix1);
         writeReal(iy1);
         writeReal(ix2 - ix1);
         writeReal(iy2 - iy1);
         if (_alpha == 1) printFast(8," re b* Q");
         else             printFast(6," re f*");
      }
   }
   if (fillis == 1) {
      setColor(fFillColor);
      if (_alpha == 1) printFast(15," q 0.4 w [] 0 d");
      writeReal(ix1);
      writeReal(iy1);
      writeReal(ix2 - ix1);
      writeReal(iy2 - iy1);
      if (_alpha == 1) printFast(8," re b* Q");
      else             printFast(6," re f*");
   }
   if (fillis == 0) {
      if (fLineWidth<=0) return;
      setColor(fLineColor);
      writeReal(ix1);
      writeReal(iy1);
      writeReal(ix2 - ix1);
      writeReal(iy2 - iy1);
      printFast(5," re S");
   }
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Draw a Frame around a box
//!
//!  - mode = -1  box looks as it is behind the screen
//!  - mode =  1  box looks as it is in front of the screen
//!  - border is the border size in already precomputed PDF units
//!  - dark  is the color for the dark part of the frame
//!  - light is the color for the light part of the frame

void PDF::drawFrame(double xl, double yl, double xt, double  yt,
                            int mode, int border, int dark, int light)
{
   static double xps[7], yps[7];
   int i;

   // Draw top&left part of the box
   if (mode == -1) setColor(dark);
   else            setColor(light);
   xps[0] = XtoPDF(xl);          yps[0] = YtoPDF(yl);
   xps[1] = xps[0] + border;     yps[1] = yps[0] + border;
   xps[2] = xps[1];              yps[2] = YtoPDF(yt) - border;
   xps[3] = XtoPDF(xt) - border; yps[3] = yps[2];
   xps[4] = XtoPDF(xt);          yps[4] = YtoPDF(yt);
   xps[5] = xps[0];              yps[5] = yps[4];
   xps[6] = xps[0];              yps[6] = yps[0];

   MoveTo(xps[0], yps[0]);
   for (i=1;i<7;i++) LineTo(xps[i], yps[i]);
   printFast(3," f*");

   // Draw bottom&right part of the box
   if (mode == -1) setColor(light);
   else            setColor(dark);
   xps[0] = XtoPDF(xl);          yps[0] = YtoPDF(yl);
   xps[1] = xps[0] + border;     yps[1] = yps[0] + border;
   xps[2] = XtoPDF(xt) - border; yps[2] = yps[1];
   xps[3] = xps[2];              yps[3] = YtoPDF(yt) - border;
   xps[4] = XtoPDF(xt);          yps[4] = YtoPDF(yt);
   xps[5] = xps[4];              yps[5] = yps[0];
   xps[6] = xps[0];              yps[6] = yps[0];

   MoveTo(xps[0], yps[0]);
   for (i=1;i<7;i++) LineTo(xps[i], yps[i]);
   printFast(3," f*");
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Draw Fill area with hatch styles

void PDF::DrawHatch(float, float, int, float *, float *)
{
   Warning("DrawHatch", "hatch fill style not yet implemented");
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Draw Fill area with hatch styles

void PDF::DrawHatch(float, float, int, double *, double *)
{
   Warning("DrawHatch", "hatch fill style not yet implemented");
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Draw a PolyLine
//!
//!  Draw a polyline through  the points points.
//!
//!  - If NN=1 moves only to point x,y.
//!  - If NN=0 the x,y are  written  in the PDF file
//!       according to the current transformation.
//!  - If NN>0 the line is clipped as a line.
//!  - If NN<0 the line is clipped as a fill area.

void PDF::DrawPolyLine(int nn, const std::vector<Points2D> & points, const LineAttributes & lineAttributes)
{
   int  n;
   writeReal(XtoPDF(xy[0].x()));
   writeReal(YtoPDF(xy[0].y()));
   if (n <= 1) {
      if (n == 0) return;
      printFast(2," m");
      return;
   }

   printFast(2," m");

   for (int i=1;i<n;i++) LineTo(XtoPDF(xy[i].x()), YtoPDF(xy[i].y()));

   if (nn > 0) {
      if (xy[0].x() == xy[n-1].x() && xy[0].y() == xy[n-1].y()) printFast(3," cl");
      printFast(2," S");
   } else {
      printFast(3," f*");
   }

   SetLineStyle(linestylesav);
   SetLineWidth(linewidthsav);
}

//!
//! Draw a PolyLine in NDC space
//!
//!  - If NN=1 moves only to point x,y.
//!  - If NN=0 the x,y are  written in the PDF file
//!       according to the current transformation.
//!  - If NN>0 the line is clipped as a line.
//!  - If NN<0 the line is clipped as a fill area.

void PDF::drawPolyLineNDC(int nn, const std::vector<Points2D> & points, const LineAttributes & lineAttributes)
{
   int  n;
   writeReal(UtoPDF(points[0].x()));
   writeReal(VtoPDF(points[0].y()));
   if (n <= 1) {
      if (n == 0) return;
      printFast(2," m");
      return;
   }
   printFast(2," m");

   for (int i=1;i<n;i++) LineTo(UtoPDF(points[i].x()), VtoPDF(points[i].y()));

   if (nn > 0) {
      if (points[0].x() == points[n-1].x() && points[0].y() == points[n-1].y()) printFast(3," cl");
      printFast(2," S");
   } else {
      printFast(3," f*");
   }
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Draw markers at the n WC points xw, yw

void PDF::drawMarkers(int n, float *xw, float *yw)
{
   fMarkerStyle = TMath::Abs(fMarkerStyle);
   Style_t linestylesav = fLineStyle;
   Width_t linewidthsav = fLineWidth;
   SetLineStyle(1);
   SetLineWidth(TMath::Max(1, int(TAttMarker::GetMarkerLineWidth(fMarkerStyle))));
   setColor(int(fMarkerColor));
   int ms = TAttMarker::GetMarkerStyleBase(fMarkerStyle);

   if (ms == 4)
      ms = 24;
   else if (ms >= 6 && ms <= 8)
      ms = 20;
   else if (ms >= 9 && ms <= 19)
      ms = 1;

   // Define the marker size
   float msize  = fMarkerSize - TMath::Floor(TAttMarker::GetMarkerLineWidth(fMarkerStyle)/2.)/4.*_linearScale;
   if (fMarkerStyle == 1 || (fMarkerStyle >= 9 && fMarkerStyle <= 19)) {
     msize = 1.;
   } else if (fMarkerStyle == 6) {
     msize = 1.;
   } else if (fMarkerStyle == 7) {
     msize = 1.5;
   } else {
      const int kBASEMARKER = 8;
      float sbase = msize*kBASEMARKER;
      float s2x = sbase / float(gPad->GetWw() * gPad->GetAbsWNDC());
      msize = this->UtoPDF(s2x) - this->UtoPDF(0);
   }

   double m  = msize;
   double m2 = m/2;
   double m3 = m/3;
   double m4 = m2*1.333333333333;
   double m6 = m/6;
   double m0 = m/10.;
   double m8 = m/4;
   double m9 = m/8;

   // Draw the marker according to the type
   double ix,iy;
   for (int i=0;i<n;i++) {
      ix = XtoPDF(xw[i]);
      iy = YtoPDF(yw[i]);
      // Dot (.)
      if (ms == 1) {
         MoveTo(ix-1, iy);
         LineTo(ix  , iy);
      // Plus (+)
      } else if (ms == 2) {
         MoveTo(ix-m2, iy);
         LineTo(ix+m2, iy);
         MoveTo(ix   , iy-m2);
         LineTo(ix   , iy+m2);
      // X shape (X)
      } else if (ms == 5) {
         MoveTo(ix-m2*0.707, iy-m2*0.707);
         LineTo(ix+m2*0.707, iy+m2*0.707);
         MoveTo(ix-m2*0.707, iy+m2*0.707);
         LineTo(ix+m2*0.707, iy-m2*0.707);
      // Asterisk shape (*)
      } else if (ms == 3 || ms == 31) {
         MoveTo(ix-m2, iy);
         LineTo(ix+m2, iy);
         MoveTo(ix   , iy-m2);
         LineTo(ix   , iy+m2);
         MoveTo(ix-m2*0.707, iy-m2*0.707);
         LineTo(ix+m2*0.707, iy+m2*0.707);
         MoveTo(ix-m2*0.707, iy+m2*0.707);
         LineTo(ix+m2*0.707, iy-m2*0.707);
      // Circle
      } else if (ms == 24 || ms == 20) {
         MoveTo(ix-m2, iy);
         writeReal(ix-m2); writeReal(iy+m4);
         writeReal(ix+m2); writeReal(iy+m4);
         writeReal(ix+m2); writeReal(iy)   ; printFast(2," c");
         writeReal(ix+m2); writeReal(iy-m4);
         writeReal(ix-m2); writeReal(iy-m4);
         writeReal(ix-m2); writeReal(iy)   ; printFast(4," c h");
      // Square
      } else if (ms == 25 || ms == 21) {
         writeReal(ix-m2); writeReal(iy-m2);
         writeReal(m)    ; writeReal(m)    ; printFast(3," re");
      // Down triangle
      } else if (ms == 23 || ms == 32) {
         MoveTo(ix   , iy-m2);
         LineTo(ix+m2, iy+m2);
         LineTo(ix-m2, iy+m2);
         printFast(2," h");
      // Up triangle
      } else if (ms == 26 || ms == 22) {
         MoveTo(ix-m2, iy-m2);
         LineTo(ix+m2, iy-m2);
         LineTo(ix   , iy+m2);
         printFast(2," h");
      } else if (ms == 27 || ms == 33) {
         MoveTo(ix   , iy-m2);
         LineTo(ix+m3, iy);
         LineTo(ix   , iy+m2);
         LineTo(ix-m3, iy)   ;
         printFast(2," h");
      } else if (ms == 28 || ms == 34) {
         MoveTo(ix-m6, iy-m6);
         LineTo(ix-m6, iy-m2);
         LineTo(ix+m6, iy-m2);
         LineTo(ix+m6, iy-m6);
         LineTo(ix+m2, iy-m6);
         LineTo(ix+m2, iy+m6);
         LineTo(ix+m6, iy+m6);
         LineTo(ix+m6, iy+m2);
         LineTo(ix-m6, iy+m2);
         LineTo(ix-m6, iy+m6);
         LineTo(ix-m2, iy+m6);
         LineTo(ix-m2, iy-m6);
         printFast(2," h");
      } else if (ms == 29 || ms == 30) {
         MoveTo(ix           , iy+m2);
         LineTo(ix+0.112255*m, iy+0.15451*m);
         LineTo(ix+0.47552*m , iy+0.15451*m);
         LineTo(ix+0.181635*m, iy-0.05902*m);
         LineTo(ix+0.29389*m , iy-0.40451*m);
         LineTo(ix           , iy-0.19098*m);
         LineTo(ix-0.29389*m , iy-0.40451*m);
         LineTo(ix-0.181635*m, iy-0.05902*m);
         LineTo(ix-0.47552*m , iy+0.15451*m);
         LineTo(ix-0.112255*m, iy+0.15451*m);
         printFast(2," h");
      } else if (ms == 35 ) {
      // diamond with cross
         MoveTo(ix-m2, iy   );
         LineTo(ix   , iy-m2);
         LineTo(ix+m2, iy   );
         LineTo(ix   , iy+m2);
         LineTo(ix-m2, iy   );
         LineTo(ix+m2, iy   );
         LineTo(ix   , iy+m2);
         LineTo(ix   , iy-m2);
         printFast(2," h");
      } else if (ms == 36 ) {
      // square with diagonal cross
         MoveTo(ix-m2, iy-m2);
         LineTo(ix+m2, iy-m2);
         LineTo(ix+m2, iy+m2);
         LineTo(ix-m2, iy+m2);
         LineTo(ix-m2, iy-m2);
         LineTo(ix+m2, iy+m2);
         LineTo(ix-m2, iy+m2);
         LineTo(ix+m2, iy-m2);
         printFast(2," h");
      } else if (ms == 37 || ms == 39 ) {
      // square with cross
         MoveTo(ix   , iy   );
         LineTo(ix-m8, iy+m2);
         LineTo(ix-m2, iy   );
         LineTo(ix   , iy   );
         LineTo(ix-m8, iy-m2);
         LineTo(ix+m8, iy-m2);
         LineTo(ix   , iy   );
         LineTo(ix+m2, iy   );
         LineTo(ix+m8, iy+m2);
         LineTo(ix   , iy   );
         printFast(2," h");
      } else if (ms == 38 ) {
      // + shaped marker with octagon
         MoveTo(ix-m2, iy   );
         LineTo(ix-m2, iy-m8);
         LineTo(ix-m8, iy-m2);
         LineTo(ix+m8, iy-m2);
         LineTo(ix+m2, iy-m8);
         LineTo(ix+m2, iy+m8);
         LineTo(ix+m8, iy+m2);
         LineTo(ix-m8, iy+m2);
         LineTo(ix-m2, iy+m8);
         LineTo(ix-m2, iy   );
         LineTo(ix+m2, iy   );
         LineTo(ix   , iy   );
         LineTo(ix   , iy-m2);
         LineTo(ix   , iy+m2);
         LineTo(ix   , iy);
         printFast(2," h");
      } else if (ms == 40 || ms == 41 ) {
       // four triangles X
         MoveTo(ix   , iy   );
         LineTo(ix+m8, iy+m2);
         LineTo(ix+m2, iy+m8);
         LineTo(ix   , iy   );
         LineTo(ix+m2, iy-m8);
         LineTo(ix+m8, iy-m2);
         LineTo(ix   , iy   );
         LineTo(ix-m8, iy-m2);
         LineTo(ix-m2, iy-m8);
         LineTo(ix   , iy   );
         LineTo(ix-m2, iy+m8);
         LineTo(ix-m8, iy+m2);
         LineTo(ix   , iy   );
         printFast(2," h");
      } else if (ms == 42 || ms == 43 ) {
      // double diamonds
         MoveTo(ix   , iy+m2);
         LineTo(ix-m9, iy+m9);
         LineTo(ix-m2, iy   );
         LineTo(ix-m9, iy-m9);
         LineTo(ix   , iy-m2);
         LineTo(ix+m9, iy-m9);
         LineTo(ix+m2, iy   );
         LineTo(ix+m9, iy+m9);
         LineTo(ix   , iy+m2);
         printFast(2," h");
      } else if (ms == 44 ) {
      // open four triangles plus
         MoveTo(ix   , iy   );
         LineTo(ix+m8, iy+m2);
         LineTo(ix-m8, iy+m2);
         LineTo(ix+m8, iy-m2);
         LineTo(ix-m8, iy-m2);
         LineTo(ix   , iy   );
         LineTo(ix+m2, iy+m8);
         LineTo(ix+m2, iy-m8);
         LineTo(ix-m2, iy+m8);
         LineTo(ix-m2, iy-m8);
         LineTo(ix   , iy   );
         printFast(2," h");
      } else if (ms == 45 ) {
      // filled four triangles plus
         MoveTo(ix+m0, iy+m0);
         LineTo(ix+m8, iy+m2);
         LineTo(ix-m8, iy+m2);
         LineTo(ix-m0, iy+m0);
         LineTo(ix-m2, iy+m8);
         LineTo(ix-m2, iy-m8);
         LineTo(ix-m0, iy-m0);
         LineTo(ix-m8, iy-m2);
         LineTo(ix+m8, iy-m2);
         LineTo(ix+m0, iy-m0);
         LineTo(ix+m2, iy-m8);
         LineTo(ix+m2, iy+m8);
         LineTo(ix+m0, iy+m0);
         printFast(2," h");
      } else if (ms == 46 || ms == 47 ) {
      // four triangles X
         MoveTo(ix   , iy+m8);
         LineTo(ix-m8, iy+m2);
         LineTo(ix-m2, iy+m8);
         LineTo(ix-m8, iy   );
         LineTo(ix-m2, iy-m8);
         LineTo(ix-m8, iy-m2);
         LineTo(ix   , iy-m8);
         LineTo(ix+m8, iy-m2);
         LineTo(ix+m2, iy-m8);
         LineTo(ix+m8, iy   );
         LineTo(ix+m2, iy+m8);
         LineTo(ix+m8, iy+m2);
         LineTo(ix   , iy+m8);
         printFast(2," h");
      } else if (ms == 48 ) {
      // four filled squares X
         MoveTo(ix   , iy+m8*1.01);
         LineTo(ix-m8, iy+m2);
         LineTo(ix-m2, iy+m8);
         LineTo(ix-m8, iy   );
         LineTo(ix-m2, iy-m8);
         LineTo(ix-m8, iy-m2);
         LineTo(ix   , iy-m8);
         LineTo(ix+m8, iy-m2);
         LineTo(ix+m2, iy-m8);
         LineTo(ix+m8, iy   );
         LineTo(ix+m2, iy+m8);
         LineTo(ix+m8, iy+m2);
         LineTo(ix   , iy+m8*0.99);
         LineTo(ix+m8*0.99, iy   );
         LineTo(ix   , iy-m8*0.99);
         LineTo(ix-m8*0.99, iy   );
         LineTo(ix   , iy+m8*0.99);
         printFast(2," h");
      } else if (ms == 49 ) {
      // four filled squares plus
         MoveTo(ix-m6, iy-m6*1.01);
         LineTo(ix-m6, iy-m2);
         LineTo(ix+m6, iy-m2);
         LineTo(ix+m6, iy-m6);
         LineTo(ix+m2, iy-m6);
         LineTo(ix+m2, iy+m6);
         LineTo(ix+m6, iy+m6);
         LineTo(ix+m6, iy+m2);
         LineTo(ix-m6, iy+m2);
         LineTo(ix-m6, iy+m6);
         LineTo(ix-m2, iy+m6);
         LineTo(ix-m2, iy-m6);
         LineTo(ix-m6, iy-m6*0.99);
         LineTo(ix-m6, iy+m6);
         LineTo(ix+m6, iy+m6);
         LineTo(ix+m6, iy-m6);
         printFast(2," h");
      } else {
         MoveTo(ix-m6, iy-m6);
         LineTo(ix-m6, iy-m2);
      }

      if ((ms > 19 && ms < 24) || ms == 29 || ms == 33 || ms == 34 ||
          ms == 39 || ms == 41 || ms == 43 || ms == 45 ||
          ms == 47 || ms == 48 || ms == 49) {
         printFast(2," f");
      } else {
         printFast(2," S");
      }
   }

   SetLineStyle(linestylesav);
   SetLineWidth(linewidthsav);
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Draw markers at the n WC points xw, yw

void PDF::drawMarkers(int n, double *xw, double *yw)
{
   fMarkerStyle = TMath::Abs(fMarkerStyle);
   Style_t linestylesav = fLineStyle;
   Width_t linewidthsav = fLineWidth;
   SetLineStyle(1);
   SetLineWidth(TMath::Max(1, int(TAttMarker::GetMarkerLineWidth(fMarkerStyle))));
   setColor(int(fMarkerColor));
   int ms = TAttMarker::GetMarkerStyleBase(fMarkerStyle);

   if (ms == 4)
      ms = 24;
   else if (ms >= 6 && ms <= 8)
      ms = 20;
   else if (ms >= 9 && ms <= 19)
      ms = 1;

   // Define the marker size
   float msize  = fMarkerSize - TMath::Floor(TAttMarker::GetMarkerLineWidth(fMarkerStyle)/2.)/4.*_linearScale;
   if (fMarkerStyle == 1 || (fMarkerStyle >= 9 && fMarkerStyle <= 19)) {
     msize = 1.;
   } else if (fMarkerStyle == 6) {
     msize = 1.5;
   } else if (fMarkerStyle == 7) {
     msize = 3.;
   } else {
      const int kBASEMARKER = 8;
      float sbase = msize*kBASEMARKER;
      float s2x = sbase / float(gPad->GetWw() * gPad->GetAbsWNDC());
      msize = this->UtoPDF(s2x) - this->UtoPDF(0);
   }

   double m  = msize;
   double m2 = m/2;
   double m3 = m/3;
   double m4 = m2*1.333333333333;
   double m6 = m/6;
   double m8 = m/4;
   double m9 = m/8;

   // Draw the marker according to the type
   double ix,iy;
   for (int i=0;i<n;i++) {
      ix = XtoPDF(xw[i]);
      iy = YtoPDF(yw[i]);
      // Dot (.)
      if (ms == 1) {
         MoveTo(ix-1, iy);
         LineTo(ix  , iy);
      // Plus (+)
      } else if (ms == 2) {
         MoveTo(ix-m2, iy);
         LineTo(ix+m2, iy);
         MoveTo(ix   , iy-m2);
         LineTo(ix   , iy+m2);
      // X shape (X)
      } else if (ms == 5) {
         MoveTo(ix-m2*0.707, iy-m2*0.707);
         LineTo(ix+m2*0.707, iy+m2*0.707);
         MoveTo(ix-m2*0.707, iy+m2*0.707);
         LineTo(ix+m2*0.707, iy-m2*0.707);
      // Asterisk shape (*)
      } else if (ms == 3 || ms == 31) {
         MoveTo(ix-m2, iy);
         LineTo(ix+m2, iy);
         MoveTo(ix   , iy-m2);
         LineTo(ix   , iy+m2);
         MoveTo(ix-m2*0.707, iy-m2*0.707);
         LineTo(ix+m2*0.707, iy+m2*0.707);
         MoveTo(ix-m2*0.707, iy+m2*0.707);
         LineTo(ix+m2*0.707, iy-m2*0.707);
      // Circle
      } else if (ms == 24 || ms == 20) {
         MoveTo(ix-m2, iy);
         writeReal(ix-m2); writeReal(iy+m4);
         writeReal(ix+m2); writeReal(iy+m4);
         writeReal(ix+m2); writeReal(iy)   ; printFast(2," c");
         writeReal(ix+m2); writeReal(iy-m4);
         writeReal(ix-m2); writeReal(iy-m4);
         writeReal(ix-m2); writeReal(iy)   ; printFast(4," c h");
      // Square
      } else if (ms == 25 || ms == 21) {
         writeReal(ix-m2); writeReal(iy-m2);
         writeReal(m)    ; writeReal(m)    ; printFast(3," re");
      // Down triangle
      } else if (ms == 23 || ms == 32) {
         MoveTo(ix   , iy-m2);
         LineTo(ix+m2, iy+m2);
         LineTo(ix-m2, iy+m2);
         printFast(2," h");
      // Up triangle
      } else if (ms == 26 || ms == 22) {
         MoveTo(ix-m2, iy-m2);
         LineTo(ix+m2, iy-m2);
         LineTo(ix   , iy+m2);
         printFast(2," h");
      } else if (ms == 27 || ms == 33) {
         MoveTo(ix   , iy-m2);
         LineTo(ix+m3, iy);
         LineTo(ix   , iy+m2);
         LineTo(ix-m3, iy)   ;
         printFast(2," h");
      } else if (ms == 28 || ms == 34) {
         MoveTo(ix-m6, iy-m6);
         LineTo(ix-m6, iy-m2);
         LineTo(ix+m6, iy-m2);
         LineTo(ix+m6, iy-m6);
         LineTo(ix+m2, iy-m6);
         LineTo(ix+m2, iy+m6);
         LineTo(ix+m6, iy+m6);
         LineTo(ix+m6, iy+m2);
         LineTo(ix-m6, iy+m2);
         LineTo(ix-m6, iy+m6);
         LineTo(ix-m2, iy+m6);
         LineTo(ix-m2, iy-m6);
         printFast(2," h");
      } else if (ms == 29 || ms == 30) {
         MoveTo(ix           , iy-m2);
         LineTo(ix-0.112255*m, iy-0.15451*m);
         LineTo(ix-0.47552*m , iy-0.15451*m);
         LineTo(ix-0.181635*m, iy+0.05902*m);
         LineTo(ix-0.29389*m , iy+0.40451*m);
         LineTo(ix           , iy+0.19098*m);
         LineTo(ix+0.29389*m , iy+0.40451*m);
         LineTo(ix+0.181635*m, iy+0.05902*m);
         LineTo(ix+0.47552*m , iy-0.15451*m);
         LineTo(ix+0.112255*m, iy-0.15451*m);
         printFast(2," h");
      } else if (ms == 35 ) {
         MoveTo(ix-m2, iy   );
         LineTo(ix   , iy-m2);
         LineTo(ix+m2, iy   );
         LineTo(ix   , iy+m2);
         LineTo(ix-m2, iy   );
         LineTo(ix+m2, iy   );
         LineTo(ix   , iy+m2);
         LineTo(ix   , iy-m2);
         printFast(2," h");
      } else if (ms == 36 ) {
         MoveTo(ix-m2, iy-m2);
         LineTo(ix+m2, iy-m2);
         LineTo(ix+m2, iy+m2);
         LineTo(ix-m2, iy+m2);
         LineTo(ix-m2, iy-m2);
         LineTo(ix+m2, iy+m2);
         LineTo(ix-m2, iy+m2);
         LineTo(ix+m2, iy-m2);
         printFast(2," h");
      } else if (ms == 37 || ms == 39 ) {
         MoveTo(ix   , iy   );
         LineTo(ix-m8, iy+m2);
         LineTo(ix-m2, iy   );
         LineTo(ix   , iy   );
         LineTo(ix-m8, iy-m2);
         LineTo(ix+m8, iy-m2);
         LineTo(ix   , iy   );
         LineTo(ix+m2, iy   );
         LineTo(ix+m8, iy+m2);
         LineTo(ix   , iy   );
         printFast(2," h");
      } else if (ms == 38 ) {
         MoveTo(ix-m2, iy   );
         LineTo(ix-m2, iy-m8);
         LineTo(ix-m8, iy-m2);
         LineTo(ix+m8, iy-m2);
         LineTo(ix+m2, iy-m8);
         LineTo(ix+m2, iy+m8);
         LineTo(ix+m8, iy+m2);
         LineTo(ix-m8, iy+m2);
         LineTo(ix-m2, iy+m8);
         LineTo(ix-m2, iy   );
         LineTo(ix+m2, iy   );
         LineTo(ix   , iy   );
         LineTo(ix   , iy-m2);
         LineTo(ix   , iy+m2);
         LineTo(ix   , iy   );
         printFast(2," h");
      } else if (ms == 40 || ms == 41 ) {
         MoveTo(ix   , iy   );
         LineTo(ix+m8, iy+m2);
         LineTo(ix+m2, iy+m8);
         LineTo(ix   , iy   );
         LineTo(ix+m2, iy-m8);
         LineTo(ix+m8, iy-m2);
         LineTo(ix   , iy   );
         LineTo(ix-m8, iy-m2);
         LineTo(ix-m2, iy-m8);
         LineTo(ix   , iy   );
         LineTo(ix-m2, iy+m8);
         LineTo(ix-m8, iy+m2);
         LineTo(ix   , iy   );
         printFast(2," h");
      } else if (ms == 42 || ms == 43 ) {
         MoveTo(ix   , iy+m2);
         LineTo(ix-m9, iy+m9);
         LineTo(ix-m2, iy   );
         LineTo(ix-m9, iy-m9);
         LineTo(ix   , iy-m2);
         LineTo(ix+m9, iy-m9);
         LineTo(ix+m2, iy   );
         LineTo(ix+m9, iy+m9);
         LineTo(ix   , iy+m2);
         printFast(2," h");
      } else if (ms == 44 ) {
         MoveTo(ix   , iy   );
         LineTo(ix+m8, iy+m2);
         LineTo(ix-m8, iy+m2);
         LineTo(ix+m8, iy-m2);
         LineTo(ix-m8, iy-m2);
         LineTo(ix   , iy   );
         LineTo(ix+m2, iy+m8);
         LineTo(ix+m2, iy-m8);
         LineTo(ix-m2, iy+m8);
         LineTo(ix-m2, iy-m8);
         LineTo(ix   , iy   );
         printFast(2," h");
      } else if (ms == 45 ) {
         MoveTo(ix+m6/2., iy+m6/2.);
         LineTo(ix+m8, iy+m2);
         LineTo(ix-m8, iy+m2);
         LineTo(ix-m6/2., iy+m6/2.);
         LineTo(ix-m2, iy+m8);
         LineTo(ix-m2, iy-m8);
         LineTo(ix-m6/2., iy-m6/2.);
         LineTo(ix-m8, iy-m2);
         LineTo(ix+m8, iy-m2);
         LineTo(ix+m6/2., iy-m6/2.);
         LineTo(ix+m2, iy-m8);
         LineTo(ix+m2, iy+m8);
         LineTo(ix+m6/2., iy+m6/2.);
         printFast(2," h");
      } else if (ms == 46 || ms == 47 ) {
         MoveTo(ix   , iy+m8);
         LineTo(ix-m8, iy+m2);
         LineTo(ix-m2, iy+m8);
         LineTo(ix-m8, iy   );
         LineTo(ix-m2, iy-m8);
         LineTo(ix-m8, iy-m2);
         LineTo(ix   , iy-m8);
         LineTo(ix+m8, iy-m2);
         LineTo(ix+m2, iy-m8);
         LineTo(ix+m8, iy   );
         LineTo(ix+m2, iy+m8);
         LineTo(ix+m8, iy+m2);
         LineTo(ix   , iy+m8);
         printFast(2," h");
      } else if (ms == 48 ) {
         MoveTo(ix   , iy+m8*1.005);
         LineTo(ix-m8, iy+m2);
         LineTo(ix-m2, iy+m8);
         LineTo(ix-m8, iy   );
         LineTo(ix-m2, iy-m8);
         LineTo(ix-m8, iy-m2);
         LineTo(ix   , iy-m8);
         LineTo(ix+m8, iy-m2);
         LineTo(ix+m2, iy-m8);
         LineTo(ix+m8, iy   );
         LineTo(ix+m2, iy+m8);
         LineTo(ix+m8, iy+m2);
         LineTo(ix   , iy+m8*0.995);
         LineTo(ix+m8*0.995, iy   );
         LineTo(ix   , iy-m8*0.995);
         LineTo(ix-m8*0.995, iy   );
         LineTo(ix   , iy+m8*0.995);
         printFast(2," h");
      } else if (ms == 49 ) {
         MoveTo(ix-m6, iy-m6*1.01);
         LineTo(ix-m6, iy-m2);
         LineTo(ix+m6, iy-m2);
         LineTo(ix+m6, iy-m6);
         LineTo(ix+m2, iy-m6);
         LineTo(ix+m2, iy+m6);
         LineTo(ix+m6, iy+m6);
         LineTo(ix+m6, iy+m2);
         LineTo(ix-m6, iy+m2);
         LineTo(ix-m6, iy+m6);
         LineTo(ix-m2, iy+m6);
         LineTo(ix-m2, iy-m6);
         LineTo(ix-m6, iy-m6*0.99);
         LineTo(ix-m6, iy+m6);
         LineTo(ix+m6, iy+m6);
         LineTo(ix+m6, iy-m6);
         MoveTo(ix-m6, iy-m6*1.01);
         printFast(2," h");
      } else {
         MoveTo(ix-1, iy);
         LineTo(ix  , iy);
      }
      if ((ms > 19 && ms < 24) || ms == 29 || ms == 33 || ms == 34 ||
          ms == 39 || ms == 41 || ms == 43 || ms == 45 ||
          ms == 47 || ms == 48 || ms == 49) {
         printFast(2," f");
      } else {
         printFast(2," S");
      }
   }

   SetLineStyle(linestylesav);
   SetLineWidth(linewidthsav);
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Draw a PolyLine
//!
//!  Draw a polyline through the points xw,yw.
//!
//!  - If nn=1 moves only to point xw,yw.
//!  - If nn=0 the XW(1) and YW(1) are  written  in the PDF file
//!            according to the current NT.
//!  - If nn>0 the line is clipped as a line.
//!  - If nn<0 the line is clipped as a fill area.

void PDF::drawPS(int nn, float *xw, float *yw)
{
   static float dyhatch[24] = {.0075,.0075,.0075,.0075,.0075,.0075,.0075,.0075,
                                 .01  ,.01  ,.01  ,.01  ,.01  ,.01  ,.01  ,.01  ,
                                 .015 ,.015 ,.015 ,.015 ,.015 ,.015 ,.015 ,.015};
   static float anglehatch[24] = {180, 90,135, 45,150, 30,120, 60,
                                    180, 90,135, 45,150, 30,120, 60,
                                    180, 90,135, 45,150, 30,120, 60};
   int  n = 0, fais = 0 , fasi = 0;

   Style_t linestylesav = fLineStyle;
   Width_t linewidthsav = fLineWidth;

   if (nn > 0) {
      if (fLineWidth<=0) return;
      n = nn;
      SetLineStyle(fLineStyle);
      SetLineWidth(fLineWidth);
      setColor(int(fLineColor));
   }
   if (nn < 0) {
      n = -nn;
      SetLineStyle(1);
      SetLineWidth(1);
      setColor(int(fFillColor));
      fais = fFillStyle/1000;
      fasi = fFillStyle%1000;
      if (fais == 3 || fais == 2) {
         if (fasi > 100 && fasi <125) {
            DrawHatch(dyhatch[fasi-101],anglehatch[fasi-101], n, xw, yw);
            SetLineStyle(linestylesav);
            SetLineWidth(linewidthsav);
            return;
         }
         if (fasi > 0 && fasi < 26) {
            SetFillPatterns(fasi, int(fFillColor));
         }
      }
   }

   writeReal(XtoPDF(xw[0]));
   writeReal(YtoPDF(yw[0]));
   if (n <= 1) {
      if (n == 0) return;
      printFast(2," m");
      return;
   }

   printFast(2," m");

   for (int i=1;i<n;i++) LineTo(XtoPDF(xw[i]), YtoPDF(yw[i]));

   if (nn > 0) {
      if (xw[0] == xw[n-1] && yw[0] == yw[n-1]) printFast(2," h");
      printFast(2," S");
   } else {
      if (fais == 0) {printFast(2," s"); return;}
      if (fais == 3 || fais == 2) {
         if (fasi > 0 && fasi < 26) {
            printFast(3," f*");
            _red   = -1;
            _green = -1;
            _blue  = -1;
            _alpha = -1.;
         }
         SetLineStyle(linestylesav);
         SetLineWidth(linewidthsav);
         return;
      }
      printFast(3," f*");
   }

   SetLineStyle(linestylesav);
   SetLineWidth(linewidthsav);
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Draw a PolyLine
//!
//! Draw a polyline through  the points xw,yw.
//!
//!  - If nn=1 moves only to point xw,yw.
//!  - If nn=0 the xw(1) and YW(1) are  written  in the PDF file
//!            according to the current NT.
//!  - If nn>0 the line is clipped as a line.
//!  - If nn<0 the line is clipped as a fill area.

void PDF::drawPS(int nn, double *xw, double *yw)
{
   static float dyhatch[24] = {.0075,.0075,.0075,.0075,.0075,.0075,.0075,.0075,
                                 .01  ,.01  ,.01  ,.01  ,.01  ,.01  ,.01  ,.01  ,
                                 .015 ,.015 ,.015 ,.015 ,.015 ,.015 ,.015 ,.015};
   static float anglehatch[24] = {180, 90,135, 45,150, 30,120, 60,
                                    180, 90,135, 45,150, 30,120, 60,
                                    180, 90,135, 45,150, 30,120, 60};
   int  n = 0, fais = 0, fasi = 0;

   Style_t linestylesav = fLineStyle;
   Width_t linewidthsav = fLineWidth;

   if (nn > 0) {
      if (fLineWidth<=0) return;
      n = nn;
      SetLineStyle(fLineStyle);
      SetLineWidth(fLineWidth);
      setColor(int(fLineColor));
   }
   if (nn < 0) {
      n = -nn;
      SetLineStyle(1);
      SetLineWidth(1);
      setColor(int(fFillColor));
      fais = fFillStyle/1000;
      fasi = fFillStyle%1000;
      if (fais == 3 || fais == 2) {
         if (fasi > 100 && fasi <125) {
            DrawHatch(dyhatch[fasi-101],anglehatch[fasi-101], n, xw, yw);
            SetLineStyle(linestylesav);
            SetLineWidth(linewidthsav);
            return;
         }
         if (fasi > 0 && fasi < 26) {
            SetFillPatterns(fasi, int(fFillColor));
         }
      }
   }

   writeReal(XtoPDF(xw[0]));
   writeReal(YtoPDF(yw[0]));
   if (n <= 1) {
      if (n == 0) return;
      printFast(2," m");
      return;
   }

   printFast(2," m");

   for (int i=1;i<n;i++) LineTo(XtoPDF(xw[i]), YtoPDF(yw[i]));

   if (nn > 0) {
      if (xw[0] == xw[n-1] && yw[0] == yw[n-1]) printFast(2," h");
      printFast(2," S");
   } else {
      if (fais == 0) {printFast(2," s"); return;}
      if (fais == 3 || fais == 2) {
         if (fasi > 0 && fasi < 26) {
            printFast(3," f*");
            _red   = -1;
            _green = -1;
            _blue  = -1;
            _alpha = -1.;
         }
         SetLineStyle(linestylesav);
         SetLineWidth(linewidthsav);
         return;
      }
      printFast(3," f*");
   }

   SetLineStyle(linestylesav);
   SetLineWidth(linewidthsav);
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! close the current opened object

void PDF::endObject()
{
   if (!_opened)
      Warning("endObject", "No Object currently opened.");
   _opened = kFALSE;

   printStr("endobj@");
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Font encoding

void PDF::FontEncode()
{
   static const char *sdtfonts[] = {
   "/Times-Italic"         , "/Times-Bold"         , "/Times-BoldItalic",
   "/Helvetica"            , "/Helvetica-Oblique"  , "/Helvetica-Bold"  ,
   "/Helvetica-BoldOblique", "/Courier"            , "/Courier-Oblique" ,
   "/Courier-Bold"         , "/Courier-BoldOblique", "/Symbol"          ,
   "/Times-Roman"          , "/ZapfDingbats"       , "/Symbol"};

   for (int i=0; i<kNumberOfFonts; i++) {
      newObject(kObjFont+i);
      printStr("<<@");
      printStr("/Type /Font@");
      printStr("/Subtype /Type1@");
      printStr("/Name /F");
      writeInteger(i+1,false);
      printStr("@");
      printStr("/BaseFont ");
      printStr(sdtfonts[i]);
      printStr("@");
      if (i!=11 && i!=13 && i!=14) {
         printStr("/Encoding /WinAnsiEncoding");
         printStr("@");
      }
      printStr(">>@");
      endObject();
   }
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Draw a line to a new position

void PDF::LineTo(double x, double y)
{
   writeReal(x);
   writeReal(y);
   printFast(2," l");
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Move to a new position

void PDF::MoveTo(double x, double y)
{
   writeReal(x);
   writeReal(y);
   printFast(2," m");
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Create a new object in the PDF file

void PDF::newObject(int n)
{
   if (_opened)
      Warning("newObject", "An Object is already open.");
   _opened = kTRUE;
   if (n > (int) _objectPos.size())
      _objectPos.resize(n, 0); // filling new elements with 0
   if (n > 0)
      _objectPos[n-1] = fNByte;
   else
      Error("newObject", "Wrong id %d is specified.", n);
   writeInteger(n, false);
   printStr(" 0 obj");
   printStr("@");
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Start a new PDF page.

void PDF::newPage()
{
   if (!_pageNotEmpty) return;

   // Compute pad conversion coefficients
   if (gPad) {
      double ww   = gPad->GetWw();
      double wh   = gPad->GetWh();
      _yPageSize        = _xPageSize*wh/ww;
   } else {
      _yPageSize = 27;
   }

   _pageCount++;
   fA = 1.;
   fB = 0.;
   fC = 0.;
   fD = 1.;
   fE = 0.;
   fF = 0.;

   if (_pageCount>1) {
      // close the currently opened page
      writeCompressedBuffer();
      printStr("endstream@");
      int streamLength = fNByte-fStartStream-10;
      endObject();
      newObject(_currentPage + 2);
      writeInteger(streamLength, false);
      printStr("@");
      endObject();
      newObject(_currentPage + 3);
      printStr("<<@");
      if (!strstr(title(),"PDF")) {
         printStr("/Title (");
         printStr(title());
         printStr(")@");
      } else {
         printStr("/Title (Page");
         writeInteger(_pageCount-1);
         printStr(")@");
      }
      printStr("/Dest [");
      writeInteger(_currentPage);
      printStr(" 0 R /XYZ null null 0]@");
      printStr("/Parent");
      writeInteger(kObjContents);
      printStr(" 0 R");
      printStr("@");
      printStr("/Next");
      writeInteger(_currentPage + 7 + fNbUrl);
      printStr(" 0 R");
      printStr("@");
      if (_pageCount>2) {
         printStr("/Prev");
         writeInteger(_pageObjects[_pageCount - 3] + 3);
         printStr(" 0 R");
         printStr("@");
      }
      printStr(">>@");
      endObject();
      writeUrlObjects();
      _currentPage = _currentPage + fNbUrl + 4; // object number of the next page
      fNbUrl = 1;
   }

   // Start a new page
   printStr("@");
   newObject(_currentPage);
   _pageObjects.push_back(_currentPage);
   printStr("<<@");
   printStr("/Type /Page@");
   printStr("@");
   printStr("/Parent");
   writeInteger(kObjPages);
   printStr(" 0 R");
   printStr("@");

   double xlow=0, ylow=0, xup=1, yup=1;
   if (gPad) {
      xlow = gPad->GetAbsXlowNDC();
      xup  = xlow + gPad->GetAbsWNDC();
      ylow = gPad->GetAbsYlowNDC();
      yup  = ylow + gPad->GetAbsHNDC();
   }

   printStr("/MediaBox [");
   double width, height;
   switch (_pageFormat) {
      case 100 :
         width  = 8.5*2.54;
         height = 11.*2.54;
         break;
      case 200 :
         width  = 8.5*2.54;
         height = 14.*2.54;
         break;
      case 300 :
         width  = 11.*2.54;
         height = 17.*2.54;
         break;
      default  :
         width  = 21.0*TMath::Power(TMath::Sqrt(2.), 4-_pageFormat);
         height = 29.7*TMath::Power(TMath::Sqrt(2.), 4-_pageFormat);
   };
   writeReal(cmToPDF(_xPageSize*xlow));
   writeReal(cmToPDF(_yPageSize*ylow));
   writeReal(cmToPDF(width));
   writeReal(cmToPDF(height));
   printStr("]");
   printStr("@");

   double xmargin = cmToPDF(0.7);
   double ymargin = 0;
   if (_pageOrientation == 1) ymargin = cmToPDF(TMath::Sqrt(2.)*0.7);
   if (_pageOrientation == 2) ymargin = cmToPDF(height)-cmToPDF(0.7);

   printStr("/CropBox [");
   if (_pageOrientation == 1) {
      writeReal(xmargin);
      writeReal(ymargin);
      writeReal(xmargin+cmToPDF(_xPageSize*xup));
      writeReal(ymargin+cmToPDF(_yPageSize*yup));
   }
   if (_pageOrientation == 2) {
      writeReal(xmargin);
      writeReal(cmToPDF(height)-cmToPDF(_xPageSize*xup)-xmargin);
      writeReal(xmargin+cmToPDF(_yPageSize*yup));
      writeReal(cmToPDF(height)-xmargin);
   }
   printStr("]");
   printStr("@");

   if (_pageOrientation == 1) printStr("/Rotate 0@");
   if (_pageOrientation == 2) printStr("/Rotate 90@");

   printStr("/Resources");
   writeInteger(kObjPageResources);
   printStr(" 0 R");
   printStr("@");

   printStr("/Contents");
   writeInteger(_currentPage + 1);
   printStr(" 0 R@");

   printStr("/Annots");
   writeInteger(_currentPage + 4);
   printStr(" 0 R");
   printStr("@");

   printStr(">>@");
   endObject();

   newObject(_currentPage + 1);
   printStr("<<@");
   printStr("/Length");
   writeInteger(_currentPage + 2);
   printStr(" 0 R@");
   printStr("/Filter [/FlateDecode]@");
   printStr(">>@");
   printStr("stream@");
   fStartStream = fNByte;
   _compress = kTRUE;

   // Force the line width definition next time PDF::SetLineWidth will be called.
   fLineWidth = -1;

   // Force the color definition next time PDF::setColor will be called.
   _red   = -1;
   _green = -1;
   _blue  = -1;
   _alpha = -1.;

   if (_pageOrientation == 2) {
      ymargin = cmToPDF(height)-cmToPDF(_xPageSize*xup)-xmargin;
      xmargin = xmargin+cmToPDF(_yPageSize*yup);
   }

   writeCM(1, 0, 0, 1, xmargin, ymargin);
   if (_pageOrientation == 2)
      writeCM(0, 1, -1, 0, 0, 0);
   if (fgLineJoin) {
      writeInteger(fgLineJoin);
      printFast(2," j");
   }
   if (fgLineCap) {
      writeInteger(fgLineCap);
      printFast(2," J");
   }
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Deactivate an already open PDF file

void PDF::Off()
{
   gVirtualPS = nullptr;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Activate an already open PDF file

void PDF::On()
{
   // _type is used to know if the PDF file is open. Unlike TPostScript, PDF
   // has no "workstation type".

   if (!_type) {
      Error("On", "no PDF file open");
      Off();
      return;
   }
   gVirtualPS = this;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! open a PDF file

void PDF::open(const char *fname, int wtype)
{
   if (fStream) {
      Warning("open", "PDF file already open");
      return;
   }

   _bufferSize = 0;
   _red       = -1;
   _green     = -1;
   _blue      = -1;
   _alpha     = -1.;
   _type      = abs(wtype);
   SetLineJoin(gStyle->GetJoinLinePS());
   SetLineCap(gStyle->GetCapLinePS());
   SetLineScale(gStyle->GetLineScalePS()/4.);
   gStyle->GetPaperSize(_xPageSize, _yPageSize);
   float xrange, yrange;
   if (gPad) {
      double ww = gPad->GetWw();
      double wh = gPad->GetWh();
      if (_type == 113) {
         ww *= gPad->GetWNDC();
         wh *= gPad->GetHNDC();
      }
      double ratio = wh/ww;
      xrange = _xPageSize;
      yrange = _xPageSize*ratio;
      if (yrange > _yPageSize) { yrange = _yPageSize; xrange = yrange/ratio;}
      _xPageSize = xrange; _yPageSize = yrange;
   }

   // open OS file
   if (!openStream(fname, kTRUE)) {
      Error("open", "Cannot open file: %s", fname);
      return;
   }

   gVirtualPS = this;

   ClearBuffer();

   // The page orientation is last digit of PDF workstation type
   //  orientation = 1 for portrait
   //  orientation = 2 for landscape
   _pageOrientation = _type%10;
   if (_pageOrientation < 1 || _pageOrientation > 2) {
      Error("open", "Invalid page orientation %d", _pageOrientation);
      return;
   }

   // format = 0-99 is the European page format (A4,A3 ...)
   // format = 100 is the US format  8.5x11.0 inch
   // format = 200 is the US format  8.5x14.0 inch
   // format = 300 is the US format 11.0x17.0 inch
   _pageFormat = _type/1000;
   if (_pageFormat == 0)  _pageFormat = 4;
   if (_pageFormat == 99) _pageFormat = 0;

   _rangeDefined = kFALSE;

   // Set a default range
   Range(_xPageSize, _yPageSize);

   _objectPos.clear();
   _pageCount = 0;
   _hasURL = kFALSE;

   printStr("%PDF-1.4@");
   printStr("%\342\343\317\323");
   printStr("@");

   newObject(kObjRoot);
   printStr("<<@");
   printStr("/Type /Catalog@");
   printStr("/Pages");
   writeInteger(kObjPages);
   printStr(" 0 R@");
   printStr("/Outlines");
   writeInteger(kObjOutlines);
   printStr(" 0 R@");
   printStr("/PageMode /UseOutlines@");
   printStr(">>@");
   endObject();

   newObject(kObjInfo);
   printStr("<<@");
   printStr("/Creator (ROOT Version ");
   printStr(gROOT->GetVersion());
   printStr(")");
   printStr("@");
   printStr("/CreationDate (");
   TDatime t;
   int toff = t.Convert(kFALSE) - t.Convert(kTRUE); // time zone and dst offset
   toff = toff/60;
   char str[24];
   snprintf(str,24,"D:%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%c%2.2d'%2.2d'",
            t.year()  , t.GetMonth(),
            t.GetDay()   , t.GetHour(),
            t.GetMinute(), t.GetSecond(),
            toff < 0 ? '-' : '+',
            // TMath::Abs(toff/60), TMath::Abs(toff%60)); // format-truncation warning
            TMath::Abs(toff/60) & 0x3F, TMath::Abs(toff%60) & 0x3F); // now 2 digits
   printStr(str);
   printStr(")");
   printStr("@");
   printStr("/ModDate (");
   printStr(str);
   printStr(")");
   printStr("@");
   printStr("/Title (");
   if (strlen(GetName())<=80) printStr(GetName());
   printStr(")");
   printStr("@");
   printStr("/Keywords (ROOT)@");
   printStr(">>@");
   endObject();

   newObject(kObjPageResources);
   printStr("<<@");
   printStr("/ProcSet [/PDF /text]@");

   printStr("/Font@");
   printStr("<<@");
   for (int i=0; i<kNumberOfFonts; i++) {
      printStr(" /F");
      writeInteger(i+1,false);
      writeInteger(kObjFont+i);
      printStr(" 0 R");
   }
   printStr("@");
   printStr(">>@");

   printStr("/ExtGState");
   writeInteger(kObjTransList);
   printStr(" 0 R @");
   if (!_alphas.empty()) _alphas.clear();

   printStr("/ColorSpace << /Cs8");
   writeInteger(kObjColorSpace);
   printStr(" 0 R >>");
   printStr("@");
   printStr("/Pattern");
   writeInteger(kObjPatternList);
   printStr(" 0 R");
   printStr("@");
   printStr(">>@");
   endObject();

   FontEncode();
   PatternEncode();

   newPage();
   _pageNotEmpty = kFALSE;
}



void PDF::ensureBufferSize(int requiredSize)
{
   if (requiredSize >= _bufferSize)
     {
      // increase buffer size by integer factor, normally 2
      int mult = (requiredSize + 1) / _bufferSize + 1;
      _buffer = TStorage::ReAllocChar(_buffer, mult*_bufferSize, _bufferSize);
      _bufferSize = mult*_bufferSize;
   }
}

void PDF::printStr(const char *str)
{
   int len = strlen(str);
   if (len == 0) return;
   _pageNotEmpty = kTRUE;
   if (_compress)
     {
      ensureBufferSize(_bufferSize + len);
      strcpy(_buffer + _bufferSize, str);
      _bufferSize += len;
   }
   else
      VirtualPS::printStr(str);
}

void PDF::printFast(int len, const char *str)
{
   _pageNotEmpty = kTRUE;
   if (_compress) {
      ensureBufferSize(_bufferSize + len);
      strcpy(_buffer + _bufferSize, str);
      _bufferSize += len;
   } else {
      VirtualPS::printFast(len, str);
   }
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Set the range for the paper in centimetres

void PDF::Range(float xsize, float ysize)
{
   _xPageSize = xsize;
   _yPageSize = ysize;
   _rangeDefined = kTRUE;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Set the alpha channel value.

void PDF::SetAlpha(float a)
{
   if (a == _alpha) return;
   _alpha = a;
   if (_alpha  <= 0.000001) _alpha  = 0;

   bool known = kFALSE;
   for (int i=0; i<(int)_alphas.size(); i++) {
      if (_alpha == _alphas[i]) {
         known = kTRUE;
         break;
      }
   }
   if (!known) _alphas.push_back(_alpha);
   printStr(TString::Format(" /ca%3.2f gs /CA%3.2f gs",_alpha,_alpha));
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Set color with its color index.

void PDF::setColor(int color)
{
   if (color < 0) color = 0;
   Color *col = gROOT->GeColor(color);

   if (col) {
      setColor(col->GetRed(), col->GetGreen(), col->GetBlue());
      SetAlpha(col->GetAlpha());
   } else {
      setColor(1., 1., 1.);
      SetAlpha(1.);
   }
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Set color with its R G B components:
//!
//!  - r: % of red in [0,1]
//!  - g: % of green in [0,1]
//!  - b: % of blue in [0,1]

void PDF::setColor(float r, float g, float b)
{
   if (r == _red && g == _green && b == _blue) return;

   _red   = r;
   _green = g;
   _blue  = b;
   if (_red   <= 0.000001) _red   = 0;
   if (_green <= 0.000001) _green = 0;
   if (_blue  <= 0.000001) _blue  = 0;

   if (gStyle->psColorMode()) {
      double colCyan, colMagenta, colYellow;
      double colBlack = TMath::Min(TMath::Min(1-_red,1-_green),1-_blue);
      if (colBlack==1) {
         colCyan    = 0;
         colMagenta = 0;
         colYellow  = 0;
      } else {
         colCyan    = (1-_red-colBlack)/(1-colBlack);
         colMagenta = (1-_green-colBlack)/(1-colBlack);
         colYellow  = (1-_blue-colBlack)/(1-colBlack);
      }
      if (colCyan    <= 0.000001) colCyan    = 0;
      if (colMagenta <= 0.000001) colMagenta = 0;
      if (colYellow  <= 0.000001) colYellow  = 0;
      if (colBlack   <= 0.000001) colBlack   = 0;
      writeReal(colCyan);
      writeReal(colMagenta);
      writeReal(colYellow);
      writeReal(colBlack);
      printFast(2," K");
      writeReal(colCyan);
      writeReal(colMagenta);
      writeReal(colYellow);
      writeReal(colBlack);
      printFast(2," k");
   } else {
      writeReal(_red);
      writeReal(_green);
      writeReal(_blue);
      printFast(3," RG");
      writeReal(_red);
      writeReal(_green);
      writeReal(_blue);
      printFast(3," rg");
   }
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Set color index for fill areas

void PDF::SetFillColor( Color_t cindex )
{
   fFillColor = cindex;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Set the fill patterns (1 to 25) for fill areas

void PDF::SetFillPatterns(int ipat, int color)
{
   char cpat[10];
   Color *col = gROOT->GeColor(color);
   if (!col) return;
   printStr(" /Cs8 cs");
   double colRed   = col->GetRed();
   double colGreen = col->GetGreen();
   double colBlue  = col->GetBlue();
   if (gStyle->psColorMode()) {
      double colBlack = TMath::Min(TMath::Min(1-colRed,1-colGreen),1-colBlue);
      if (colBlack==1) {
         writeReal(0);
         writeReal(0);
         writeReal(0);
         writeReal(colBlack);
      } else {
         double colCyan    = (1-colRed-colBlack)/(1-colBlack);
         double colMagenta = (1-colGreen-colBlack)/(1-colBlack);
         double colYellow  = (1-colBlue-colBlack)/(1-colBlack);
         writeReal(colCyan);
         writeReal(colMagenta);
         writeReal(colYellow);
         writeReal(colBlack);
      }
   } else {
      writeReal(colRed);
      writeReal(colGreen);
      writeReal(colBlue);
   }

   if (_pageOrientation == 2) {
      switch (ipat) {
         case 4:  ipat = 5;  break;
         case 5:  ipat = 4;  break;
         case 6:  ipat = 7;  break;
         case 7:  ipat = 6;  break;
         case 17: ipat = 18; break;
         case 18: ipat = 17; break;
         case 20: ipat = 16; break;
         case 16: ipat = 20; break;
         case 21: ipat = 22; break;
         case 22: ipat = 21; break;
      }
   }
   snprintf(cpat,10," /P%2.2d scn", ipat);
   printStr(cpat);
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Set color index for lines

void PDF::SetLineColor( Color_t cindex )
{
   fLineColor = cindex;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Set the value of the global parameter PDF::fgLineJoin.
//! This parameter determines the appearance of joining lines in a PDF
//! output.
//! It takes one argument which may be:
//!   - 0 (miter join)
//!   - 1 (round join)
//!   - 2 (bevel join)
//! The default value is 0 (miter join).
//!
//! \image html postscript_1.png
//!
//! To change the line join behaviour just do:
//! ~~~ {.cpp}
//! gStyle->SetJoinLinePS(2); // Set the PDF line join to bevel.
//! ~~~

void PDF::SetLineJoin( int linejoin )
{
   fgLineJoin = linejoin;
   if (fgLineJoin<0) fgLineJoin=0;
   if (fgLineJoin>2) fgLineJoin=2;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Set the value of the global parameter PDF::fgLineCap.
//! This parameter determines the appearance of line caps in a PDF
//! output.
//! It takes one argument which may be:
//!   - 0 (butt caps)
//!   - 1 (round caps)
//!   - 2 (projecting caps)
//! The default value is 0 (butt caps).
//!
//! \image html postscript_2.png
//!
//! To change the line cap behaviour just do:
//! ~~~ {.cpp}
//! gStyle->SetCapLinePS(2); // Set the PDF line cap to projecting.
//! ~~~

void PDF::SetLineCap( int linecap )
{
   fgLineCap = linecap;
   if (fgLineCap<0) fgLineCap=0;
   if (fgLineCap>2) fgLineCap=2;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Change the line style
//!
//!  - linestyle = 2 dashed
//!  - linestyle = 3 dotted
//!  - linestyle = 4 dash-dotted
//!  - linestyle = else solid (1 in is used most of the time)

void PDF::SetLineStyle(Style_t linestyle)
{
   if ( linestyle == fLineStyle) return;
   fLineStyle = linestyle;
   TString st = (TString)gStyle->GetLineStyleString(linestyle);
   printFast(2," [");
   TObjArray *tokens = st.Tokenize(" ");
   for (int j = 0; j<tokens->GetEntries(); j++) {
      int it;
      sscanf(((TObjString*)tokens->At(j))->GetName(), "%d", &it);
      writeInteger((int)(it/4));
   }
   delete tokens;
   printFast(5,"] 0 d");
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Change the line width

void PDF::SetLineWidth(Width_t linewidth)
{
   if (linewidth == fLineWidth) return;
   fLineWidth = linewidth;
   if (fLineWidth!=0) {
      writeReal(_linearScale*fLineWidth);
      printFast(2," w");
   }
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Set color index for markers.

void PDF::SetMarkerColor( Color_t cindex )
{
   fMarkerColor = cindex;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Set color index for text

void PDF::SeTexColor( Color_t cindex )
{
   fTexColor = cindex;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Draw text
//!
//!  - xx: x position of the text
//!  - yy: y position of the text
//!  - chars: text to be drawn

void PDF::text(double xx, double yy, const char *chars)
{
   if (fTextSize <= 0) return;

   const double kDEGRAD = TMath::Pi()/180.;
   char str[8];
   double x = xx;
   double y = yy;

   // Font and text size
   int font = abs(fTextFont)/10;
   if (font > kNumberOfFonts || font < 1) font = 1;

   double wh = (double)gPad->XtoPixel(gPad->x2());
   double hh = (double)gPad->YtoPixel(gPad->y1());
   float tsize, ftsize;
   if (wh < hh) {
      tsize = fTextSize*wh;
      int sizeTTF = (int)(tsize*kScale+0.5); // TTF size
      ftsize = (sizeTTF*_xPageSize*gPad->GetAbsWNDC())/wh;
   } else {
      tsize = fTextSize*hh;
      int sizeTTF = (int)(tsize*kScale+0.5); // TTF size
      ftsize = (sizeTTF*_yPageSize*gPad->GetAbsHNDC())/hh;
   }
   double fontsize = 72*(ftsize)/2.54;
   if (fontsize <= 0) return;

   // text color
   setColor(int(fTexColor));

   // Clipping
   printStr(" q");
   double x1 = XtoPDF(gPad->x1());
   double x2 = XtoPDF(gPad->x2());
   double y1 = YtoPDF(gPad->y1());
   double y2 = YtoPDF(gPad->y2());
   writeReal(x1);
   writeReal(y1);
   writeReal(x2 - x1);
   writeReal(y2 - y1);
   printStr(" re W n");

   // Start the text
   if (!_compress) printStr("@");

   // text alignment
   float tsizex = gPad->AbsPixeltoX(int(tsize))-gPad->AbsPixeltoX(0);
   float tsizey = gPad->AbsPixeltoY(0)-gPad->AbsPixeltoY(int(tsize));
   int txalh = fTextAlign/10;
   if (txalh < 1) txalh = 1; else if (txalh > 3) txalh = 3;
   int txalv = fTextAlign%10;
   if (txalv < 1) txalv = 1; else if (txalv > 3) txalv = 3;
   if (txalv == 3) {
      y -= 0.8*tsizey*TMath::Cos(kDEGRAD*fTextAngle);
      x += 0.8*tsizex*TMath::Sin(kDEGRAD*fTextAngle);
   } else if (txalv == 2) {
      y -= 0.4*tsizey*TMath::Cos(kDEGRAD*fTextAngle);
      x += 0.4*tsizex*TMath::Sin(kDEGRAD*fTextAngle);
   }

   if (txalh > 1) {
      text t;
      UInt_t w=0, h;
      t.SeTextSize(fTextSize);
      t.SeTextFont(fTextFont);
      t.GeTextExtent(w, h, chars);
      double twx = gPad->AbsPixeltoX(w)-gPad->AbsPixeltoX(0);
      double twy = gPad->AbsPixeltoY(0)-gPad->AbsPixeltoY(w);
      if (txalh == 2) {
         x = x-(twx/2)*TMath::Cos(kDEGRAD*fTextAngle);
         y = y-(twy/2)*TMath::Sin(kDEGRAD*fTextAngle);
      }
      if (txalh == 3) {
         x = x-twx*TMath::Cos(kDEGRAD*fTextAngle);
         y = y-twy*TMath::Sin(kDEGRAD*fTextAngle);
      }
   }

   // text angle
   double a, b, c, d, e, f;
   if (fTextAngle == 0) {
      a = 1;
      b = 0;
      c = 0;
      d = 1;
      e = XtoPDF(x);
      f = YtoPDF(y);
   } else if (fTextAngle == 90) {
      a = 0;
      b = 1;
      c = -1;
      d = 0;
      e = XtoPDF(x);
      f = YtoPDF(y);
   } else if (fTextAngle == 270) {
      a = 0;
      b = -1;
      c = 1;
      d = 0;
      e = XtoPDF(x);
      f = YtoPDF(y);
   } else {
      a = TMath::Cos(kDEGRAD * fTextAngle);
      b = TMath::Sin(kDEGRAD * fTextAngle);
      c = -TMath::Sin(kDEGRAD * fTextAngle);
      d = TMath::Cos(kDEGRAD * fTextAngle);
      e = XtoPDF(x);
      f = YtoPDF(y);
   }
   writeCM(a, b, c, d, e, f, kFALSE);

   // Symbol Italic tan(15) = .26794
   if (font == 15)
      writeCM(1, 0, 0.26794, 1, 0, 0, kFALSE);

   if (_hasURL)
      computeRect(chars, fontsize, a, b, c, d, e, f);

   printStr(" BT");

   snprintf(str,8," /F%d",font);
   printStr(str);
   writeReal(fontsize);
   printStr(" Tf");

   const int len=strlen(chars);

   // Calculate the individual character placements.
   // Otherwise, if a string is printed in one line the kerning is not
   // performed. In order to measure the precise character positions we need to
   // trick FreeType into rendering high-resolution characters otherwise it will
   // stick to the screen pixel grid which is far worse than we can achieve on
   // print.
   const float scale = 16.0;
   // Save current text attributes.
   text saveAtText;
   saveAtText.TAtText::operator=(*this);
   text t;
   t.SeTextSize(fTextSize * scale);
   t.SeTextFont(fTextFont);
   UInt_t wa1=0, wa0=0;
   t.GeTextAdvance(wa0, chars, kFALSE);
   t.GeTextAdvance(wa1, chars);
   t.TAtText::Modify();
   bool kerning;
   if (wa0-wa1 != 0) kerning = kTRUE;
   else              kerning = kFALSE;
   int *charDeltas = nullptr;
   if (kerning) {
        charDeltas = new int[len];
        for (int i = 0;i < len;i++) {
            UInt_t ww=0;
            t.GeTextAdvance(ww, chars + i);
            charDeltas[i] = wa1 - ww;
        }
        for (int i = len - 1;i > 0;i--) {
            charDeltas[i] -= charDeltas[i-1];
        }
        char tmp[2];
        tmp[1] = 0;
        for (int i = 1;i < len;i++) {
            tmp[0] = chars[i-1];
            UInt_t width=0;
            t.GeTextAdvance(width, &tmp[0], kFALSE);
            double wwl = gPad->AbsPixeltoX(width - charDeltas[i]) - gPad->AbsPixeltoX(0);
            wwl -= 0.5*(gPad->AbsPixeltoX(1) - gPad->AbsPixeltoX(0)); // half a pixel ~ rounding error
            charDeltas[i] = (int)((1000.0/float(fontsize))*(XtoPDF(wwl) - XtoPDF(0))/scale);
        }
   }
   // Restore text attributes.
   saveAtText.TAtText::Modify();

   // Output the text. Escape some characters if needed
   if (kerning) printStr(" [");
   else         printStr(" (");

   for (int i=0; i<len;i++) {
      if (chars[i]!='\n') {
         if (kerning) printStr("(");
         if (chars[i]=='(' || chars[i]==')') {
            snprintf(str,8,"\\%c",chars[i]);
         } else {
            snprintf(str,8,"%c",chars[i]);
         }
         printStr(str);
         if (kerning) {
            printStr(") ");
            if (i < len-1) {
               writeInteger(charDeltas[i+1]);
            }
         }
      }
   }

   if (kerning) printStr("] TJ ET Q");
   else         printStr(") Tj ET Q");
   if (!_compress) printStr("@");
   if (kerning) delete [] charDeltas;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! write a string of characters
//!
//! This method writes the string chars into a PDF file
//! at position xx,yy in world coordinates.

void PDF::text(double, double, const wchar_t *)
{
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Draw text with URL. Same as text.
//!

void PDF::textUrl(double x, double y, const char *chars, const char *url)
{
   _hasURL = kTRUE;
   text(x, y, chars);
   fNbUrl++;
   _hasURLs.push_back(url);
   _hasURL = kFALSE;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! write a string of characters in NDC

void PDF::textNDC(double u, double v, const char *chars)
{
   double x = gPad->x1() + u*(gPad->x2() - gPad->x1());
   double y = gPad->y1() + v*(gPad->y2() - gPad->y1());
   text(x, y, chars);
}



double PDF::UtoPDF(double u)
{
   double cm = _xPageSize*(gPad->GetAbsXlowNDC() + u*gPad->GetAbsWNDC());
   return 72*cm/2.54;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Convert V from NDC coordinate to PDF

double PDF::VtoPDF(double v)
{
   double cm = _yPageSize*(gPad->GetAbsYlowNDC() + v*gPad->GetAbsHNDC());
   return 72*cm/2.54;
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Convert X from world coordinate to PDF

double PDF::XtoPDF(double x)
{
   double u = (x - gPad->x1())/(gPad->x2() - gPad->x1());
   return  UtoPDF(u);
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! Convert Y from world coordinate to PDF

double PDF::YtoPDF(double y)
{
   double v = (y - gPad->y1())/(gPad->y2() - gPad->y1());
   return  VtoPDF(v);
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! write the buffer in a compressed way

void PDF::writeCompressedBuffer()
{
   z_stream stream;
   int err;
   char *out = new char[2*_bufferSize];

   stream.next_in   = (Bytef*)_buffer;
   stream.avail_in  = (uInt)_bufferSize;
   stream.next_out  = (Bytef*)out;
   stream.avail_out = (uInt)2*_bufferSize;
   stream.zalloc    = (alloc_func)nullptr;
   stream.zfree     = (free_func)nullptr;
   stream.opaque    = (voidpf)nullptr;

   err = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
   if (err != Z_OK) {
      Error("writeCompressedBuffer", "error in deflateInit (zlib)");
      delete [] out;
      return;
   }

   err = deflate(&stream, Z_FINISH);
   if (err != Z_STREAM_END) {
      deflateEnd(&stream);
      Error("writeCompressedBuffer", "error in deflate (zlib)");
      delete [] out;
      return;
   }

   err = deflateEnd(&stream);
   if (err != Z_OK) {
      Error("writeCompressedBuffer", "error in deflateEnd (zlib)");
   }

   fStream->write(out, stream.total_out);

   fNByte += stream.total_out;
   fStream->write("\n",1); fNByte++;
   _bufferSize = 0;
   delete [] out;
   _compress = kFALSE;
}

//! write a Real number to the file.
//! This method overwrites VirtualPS::writeReal. Some PDF reader like
//! Acrobat do not work when a PDF file contains real numbers with exponent. This
//! method writes the real number "z" using the format "%f" instead of the
//! format "%g" when writing it with "%g" generates a number with exponent.

void PDF::writeReal(float z, bool space)
{
   char str[15];
   if (space) {
      snprintf(str,15," %g", z);
      if (strstr(str,"e") || strstr(str,"E")) snprintf(str,15," %10.8f", z);
   } else {
      snprintf(str,15,"%g", z);
      if (strstr(str,"e") || strstr(str,"E")) snprintf(str,15,"%10.8f", z);
   }
   printStr(str);
}

void PDF::patternEncode()
{
   int patternNb = kObjPattern;
   newObject(kObjColorSpace);
   if (gStyle->psColorMode()) {
      printStr("[/Pattern /DeviceCMYK]@");
   } else {
      printStr("[/Pattern /DeviceRGB]@");
   }
   endObject();
   newObject(kObjPatternResourses);
   printStr("<</ProcSet[/PDF]>>@");
   endObject();

   newObject(kObjPatternList);
   printStr("<<@");
   printStr(" /P01");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P02");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P03");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P04");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P05");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P06");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P07");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P08");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P09");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P10");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P11");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P12");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P13");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P14");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P15");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P16");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P17");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P18");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P19");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P20");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P21");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P22");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P23");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P24");
   writeInteger(patternNb++);
   printStr(" 0 R");
   printStr(" /P25");
   writeInteger(patternNb++);
   printStr(" 0 R@");
   printStr(">>@");
   endObject();

   patternNb = kObjPattern;

   // P01
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[1 0 0 1 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 98 4]/XStep 98/YStep 4/Length 91/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211*\3442T\310T\3402P0P04\200\340\242T\256p\205<\240\220\027P0K\301P\241\034(\254\340\253\020m\250\020k\240\220\302e\244`\242\220\313ei\t\244r\200\272\215A\034\v \225\003\2241\202\310\030\201e\f!2\206@N0W \027@\200\001\0|c\024\357\n", 93);
   fNByte += 93;
   printStr("endstream@");
   endObject();

   // P02
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.75 0 0 0.75 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 96 4]/XStep 96/YStep 4/Length 92/Filter/FlateDecode>>@");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211$\2121\n\2000\024C\367\234\"G\370\277\025\321+\b\016\342\340P\334tP\252\240\213\3277\332!\204\274\227\v\316\2150\032\335J\356\025\023O\241Np\247\363\021f\317\344\214\234\215\v\002+\036h\033U\326/~\243Ve\231PL\370\215\027\343\032#\006\274\002\f\0\242`\025:\n", 94);
   fNByte += 94;
   printStr("endstream@");
   endObject();

   // P03
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.5 0 0 0.5 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 96 16]/XStep 96/YStep 16/Length 93/Filter/FlateDecode>>@");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211$\2121\n\2000\024C\367\234\"G\370\261(\366\n\202\20388\210\233\016J\025t\361\372\376\332!\204\274\227\033\342N\030\215\262\222g\303\304\313Q\347\360\240\370:f\317Y\f\\\214+**\360Dls'\177\306\274\032\257\344\256.\252\376\215\212\221\217\021\003>\001\006\0\317\243\025\254\n", 95);
   fNByte += 95;
   printStr("endstream@");
   endObject();

   // P04
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.06 0 0 0.06 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 63/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211*\3442T\310T\3402P0P04\200\340\242T\256p\205<\240\220\027P0K\301D\241\034(\254\340\253\020\035k\240\220\002V\231\313\005S\233\303\025\314\025\310\005\020`\0\344\270\r\274\n", 65);
   fNByte += 65;
   printStr("endstream@");
   endObject();

   // P05
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.06 0 0 0.06 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 66/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211*\3442T\310T\3402P0P04\200\340\242T\256p\205<\240\220\027P0K\301D\241\034(\254\340\253\020\035k\240\220\302\005Q\223\313\005\"\r\024r\270\202\271\002\271\0\002\f\0\344\320\r\274\n", 68);
   fNByte += 68;
   printStr("endstream@");
   endObject();

   // P06
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.03 0 0 0.03 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 66/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211*\3442T\310T\3402P0P04\200\340\242T\256p\205<\240\220\027P0K\301D\241\034(\254\340\253\020\035k\240\220\302e\nR\232\v\242@js\270\202\271\002\271\0\002\f\0\345X\r\305\n", 68);
   fNByte += 68;
   printStr("endstream@");
   endObject();

   // P07
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.03 0 0 0.03 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 68/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211*\3442T\310T\3402P0P04\200\340\242T\256p\205<\240\220\027P0K\301D\241\034(\254\340\253\020\035k\240\220\002\02465P\310\345\002)\0042r\270\202\271\002\271\0\002\f\0\345=\r\305\n", 70);
   fNByte += 70;
   printStr("endstream@");
   endObject();

   // P08
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.06 0 0 0.06 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 101 101]/XStep 100/YStep 100/Length 139/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211D\217\261\016\3020\fDw\177\305\315L6Q\225|\003\022C\305\300Puk+\201\032$\272\360\373\330\265\323\016\271\330\367\234\344\"x\201\030\214\252\232\030+%\353VZ.jd\367\205\003x\241({]\311\324]\323|\342\006\033J\201:\306\325\230Jg\226J\261\275D\257#\337=\220\260\354k\233\351\211\217Z75\337\020\374\324\306\035\303\310\230\342x=\303\371\275\307o\332s\331\223\224\240G\330\a\365\364\027`\0\nX1}\n",141);
   fNByte += 141;
   printStr("endstream@");
   endObject();

   // P09
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.06 0 0 0.06 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 108/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211*\3442T\310T\3402P0P04\200\340\242T\256p\205<\240\220\027P0K\301D\241\034(\254\340\253\020\035k\240\220\002\02465P\310\005RFFz&\020\002,d\240\220\314en\256g\0065\b,\001b\230\202$\240\232\214@\362\246`\2169H\336\024\2426\231\v&\200,\n\326\030\314\025\310\005\020`\0\f@\036\227\n", 110);
   fNByte += 110;
   printStr("endstream@");
   endObject();

   // P10
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.06 0 0 0.06 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 93/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211*\3442T\310T\3402P0P04\200\340\242T\256p\205<\240\220\027P0K\301D\241\034(\254\340\253\020\035k\240\220\002\02465P\310\345\002)\0042r\200\332\r\241\\C \017dN.\027L\312\0\302\205\2535\205j6\205X\224\303\025\314\025\310\005\020`\0\2127\031\t\n", 95);
   fNByte += 95;
   printStr("endstream@");
   endObject();

   // P11
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.125 0 0 0.125 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 164/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211\\\2171\016\3020\fEw\237\342\037\301ip\223^\001\211\001u`@l0\200(\022,\\\037;v\204\332\241\211\336\373\337V\363\246\204;\210\301H\354\337\347F'\274T\355U>\220\360U\215\003\316\027\306\2655\027=\a\306\223\304I\002m\332\330\356&\030\325\333fZ\275F\337\205\235\265O\270\032\004\331\214\336\305\270\004\227`\357i\256\223\342;]\344\255(!\372\356\205j\030\377K\335\220\344\377\210\274\306\022\330\337T{\214,\212;\301\3508\006\346\206\021O=\216|\212|\246#\375\004\030\0\216FF\207\n", 166);
   fNByte += 166;
   printStr("endstream@");
   endObject();

   // P12
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.125 0 0 0.125 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 226/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211<P;n\3030\f\335y\n\236 \220DK\242\256P\240C\321\241C\221\311\311\220\242\016\220.\275~D\221/\203I\342}\370(?(\363\215)q\342\234\374\373\273\322\027\337'\3646\301\037\316\374?a~\347\357s\342\313\2045\361A9\237\322fc\231\200\236F\263\301\334;\211\017\207\rN\311\252S\\\227{\247\006w\207\244\303\255p+(\205\333\360e/v\356a\315\317\360\272\320b|w\276\203o\340k\b\004\027\v$b\226\235,\242\254t(\024\nu\305Vm\313\021\375\327\272\257\227fuf\226ju\356\222x\030\024\313\261S\215\377\341\274,\203\254\253Z\\\262A\262\205eD\350\210\320\201\225\212\320\036\241\355\025\372JE,\2266\344\366\310U\344\016HFx>\351\203\236\002\f\0d}e\216\n", 228);
   fNByte += 228;
   printStr("endstream@");
   endObject();

   // P13
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.06 0 0 0.06 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 69/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211*\3442T\310T\3402P0P04\200\340\242T\256p\205<\240\220\027P0K\301D\241\034(\254\340\253\020\035k\240\220\002V\231\313\005S\233\303\005\241!\" ~0W \027@\200\001\0\331\227\020\253\n", 71);
   fNByte += 71;
   printStr("endstream@");
   endObject();

   // P14
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.15 0 0 0.15 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 80/YStep 80/Length 114/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\2114\214=\n\2000\f\205\367\234\342\035!-\241\364\f\202\20388\210\233\016J+\350\342\365M\3723\224\327\367}I\036r8A\f\206\343\372\336\203\026\334\212\006\205\027\004\237b\214X7\306\256\33032\331\240~\022y[\315\026\206\222\372\330}\264\036\253\217\335\353\240\030\b%\223\245o=X\227\346\245\355K\341\345@\3613M\364\v0\0\207o\"\261\n", 116);
   fNByte += 116;
   printStr("endstream@");
   endObject();

   // P15
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.102 0 0 0.102 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 60 60]/XStep 60/YStep 60/Length 218/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211<\2211\016\3020\fEw\237\302'@\211c\267w@b@\f\f\210\2510\200(\022,\\\037\347\307\256Z\325\221\375\337\377\225\363\241\312\017\246\302\205'\274\337;\235\371\355\215\275\267\236\\\371\307\265\360\201/\327\3027o\233\361J\262\233\247~\362g\336\211zur!A]{\035}\031S\343\006p\241\226dKI\v\326\202\265\3153\331)X)\335fE\205M\235\373\327\r*\374\026\252\022\216u\223\200\361I\211\177\031\022\001#``\342GI\211\004c\221gi\246\231\247\221\247\231\247\233$XM3\315<\215<\315<K\211e\036#\215a4\366\344\035lm\214Z\314b\211Xj\337K\\\201$\332\325\v\365\2659\204\362\242\274'\v\221\r\321\211\216\364\027`\0\212'_\215\n", 220);
   fNByte += 220;
   printStr("endstream@");
   endObject();

   // P16
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.1 0 0 0.05 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 123/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211*\3442T\310T\3402P0P04\200\340\242T\256p\205<\240\220\027P0K\301D\241\034(\254\340\253\020\035k\240\220\302ej\240\0D\271 \332\314X\317B\301\330\002H\230\233*\030\231\202\310d.CC=#\020\v*\rV\235\214\254\v\210r@\264\261\031P\241\031H5D\253\021H\267\005\3104 \v\344\016\260\002\020\003lB0W \027@\200\001\0hU \305\n", 125);
   fNByte += 125;
   printStr("endstream@");
   endObject();

   // P17
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.06 0 0 0.06 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 66/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211*\3442T\310T\3402P0P04\200\340\242T\256p\205<\240\220\027P0K\301D\241\034(\254\340\253\020md\242\020k\240\220\002V\234\313\005S\236\303\025\314\025\310\005\020`\0\r\351\016B\n", 68);
   fNByte += 68;
   printStr("endstream@");
   endObject();

   // P18
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.06 0 0 0.06 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 69/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211*\3442T\310T\3402P0P04\200\340\242T\256p\205<\240\220\027P0K\301D\241\034(\254\340\253\020md\242\020k\240\220\302\005Q\226\313\005\"\r\024r\270\202\271\002\271\0\002\f\0\016\001\016B\n", 71);
   fNByte += 71;
   printStr("endstream@");
   endObject();

   // P19
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.117 0 0 0.117 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 149/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211L\216;\016\302@\fD{\237bN\020\331+6a\257\200D\201((P\252@\001R\220\240\341\372\370\263\216(\326\266f\336\330\373&\301\003\304`\b\307\373\334\351\202\227J\a\025\237\020|U\306\021\327\231q\243\306\250\214\325\372T\006\336\367\032\262\326\205\3124\264b\243$\"n.\244=\314\250!\2139\033\327\022i=\323\317\2518\332T}\347.\202\346W\373\372j\315\221\344\266\213=\237\241\344\034\361\264!\236w\344\177\271o8\323\211~\002\f\0\366\3026\233\n", 151);
   fNByte += 151;
   printStr("endstream@");
   endObject();

   // P20
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.05 0 0 0.1 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 122/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211<L;\016\2030\f\335}\212w\002\344$M\2323 1 \006\006\304\224vhU\220`\341\372<\aT\311\366\263\336o\023\207\017D\241pz\355\376\226\021+\251\226\344\027\017\034\244\321a\232\025/\211\n\316r\343ORh\262}\317\210\344\032o\310)\302\2233\245\252[m\274\332\313\277!$\332\371\371\210`N\242\267$\217\263\246\252W\257\245\006\351\345\024`\0o\347 \305\n", 124);
   fNByte += 124;
   printStr("endstream@");
   endObject();

   // P21
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.125 0 0 0.125 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 101 101]/XStep 100/YStep 100/Length 117/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211D\2151\n\2000\fE\367\234\342\037!)\224\336Ap\020\a\aq\323A\251\202.^\337$-\025\022^\372\033^n\022\354 \006CX\274\237\215&\\\032u\032\036\020\274\032\243\307\2740V]\027\234\024\242\"\033\2642En\324\312\224bc\262\\\230\377\301\332WM\224\212(U\221\375\265\301\025\016?\350\317P\215\221\033\213o\244\201>\001\006\0\031I'f\n", 119);
   fNByte += 119;
   printStr("endstream@");
   endObject();

   // P22
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.125 0 0 0.125 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 101 101]/XStep 100/YStep 100/Length 118/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211<\215=\n\204P\f\204\373\234b\216\220<\b\357\016\302\026ba!vZ(\273\v\332x}\223\274\237\"|\223a\230\271Hp\200\030\fa\211\273w\232\3617k0\363\204\3401\033\037,+c#\3170~\2244\304\327EV\243r\247\272oOcr\337\323]H\t\226\252\334\252r\255\362\257\213(\t\304\250\326\315T\267\032\275q\242\221^\001\006\0\272\367(&\n", 120);
   fNByte += 120;
   printStr("endstream@");
   endObject();

   // P23
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.06 0 0 0.06 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 169/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211<\220\273\n\0021\020E\373\371\212[[M\326\331\354\344\027\004\v\261\260\020;\025\224D\320\306\337w\036\254p\363\230\223\341$\344M\005\017\020\203Q8\307\347F'\274\f\355\f>Q\3605\214=\316\005\v.\214kt\217\230;)\324\366\245Fa\213e\320v\212r\022X\006\211Fi\3242\250J\224\302\020\367h\212\254I\\\325R\225o\03143\346U\235@a\t[\202Za\tA\202E`\351~O\002\235`\351~S\202\306h.m\253\264)\232K\217t\310\017q\354\a\353\247\364\377C\356\033\372\t0\0\bm:\375\n", 171);
   fNByte += 171;
   printStr("endstream@");
   endObject();

   // P24
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.125 0 0 0.125 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 100 100]/XStep 100/YStep 100/Length 280/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\211DQ9N\004A\f\314\373\025\216\211\326\343v\037_@\"@\004\004\210\f\220@\003\022$|\177\335\345j\220v\345\251\303\343*\215\312\273\024\275\\d\375?\361dM\3162\306\337\214\337Y\336n\240m\217\036\301y\343\\<,i\250\0038F\035)\347l\322\026o\377\023\353|[\254\177\343\005;\315\317ky\224\257\240n\203\374\020\225\337\240\345N\236T\272<_\344\245\304^\3238\030\tc\236E\233xO\034\363\204>\251\317\324\233\023{\352\235\376\336S\357Fl\251\017\372\207\247>xoh&_\366Ud\331\253\314D\023\332\241\211\016\205\246\235\326\236*\275\307\204z8!s\031\335\306\\\306C\306\\\225\376\312\\\225\307\252\246\356\364\273Q\347\271:\371\341l\177\311e\210\3571\211\251#\374\302H\037:\342c\241\323\2617\320 \034\250\0\302\323a{\005%\302a\373(Zx\313\026\213@\215p\324}\026=\274e\217E8s\326}\026M\036\312}\271\n0\0\215\263\207\016\n", 282);
   fNByte += 282;
   printStr("endstream@");
   endObject();

   // P25
   newObject(patternNb++);
   printStr("<</Type/Pattern/Matrix[0.125 0 0 0.125 20 28]/PatternType 1/Resources");
   writeInteger(kObjPatternResourses);
   printStr(" 0 R/PaintType 2/TilingType 1/BBox[0 0 101 101]/XStep 100/YStep 100/Length 54/Filter/FlateDecode>>");
   printStr("@");
   fStream->write("stream",6); fNByte += 6;
   fStream->write("\r\nH\2112T\310T\3402P0P\310\34526P\0\242\034.s\004m\016\242\r\r\f\024@\030\302\002\321iZP\305`M\346\310\212\201R\0\001\006\0\206\322\017\200\n", 56);
   fNByte += 56;
   printStr("endstream@");
   endObject();
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! write and Accumulate (if `acc` is true) the Current Transformation Matrix (CTM)
//!
//! The Current Transformation Matrix (CTM, not CMT) is defined by the six parameters
//! `a` `b` `c` `d` `e` `f` passed to the PDF `cm` operator (see the PDF Reference Guide
//! page 156 [1] for details).
//!
//! To correctly define the \Rect fields of the Annots created for each #url, one must keep
//! track of the current CTM and apply it to the last transformation matrix used for the
//! text (for example rotations).
//!
//! [1] https://opensource.adobe.com/dc-acrobat-sdk-docs/pdfstandards/pdfreference1.4.pdf

void PDF::writeCM(double a, double b, double c, double d, double e, double f, bool acc)
{
   writeReal(a);
   writeReal(b);
   writeReal(c);
   writeReal(d);
   writeReal(e);
   writeReal(f);
   printStr(" cm");

   // accumulate in CTM ---
   if (acc) {
      double na, nb, nc, nd, ne, nf;
      na = fA * a + fC * b;
      nb = fB * a + fD * b;
      nc = fA * c + fC * d;
      nd = fB * c + fD * d;
      ne = fA * e + fC * f + fE;
      nf = fB * e + fD * f + fF;
      fA = na;
      fB = nb;
      fC = nc;
      fD = nd;
      fE = ne;
      fF = nf;
   }
}

//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//!//
//! write the annotation objects containing the URLs

void PDF::writeUrlObjects()
{
   int i;
   newObject(_currentPage + 4);
   printStr("@");
   printStr("[");
   for (i = 0; i < fNbUrl - 1; i++) {
      writeInteger(_currentPage + 5 + i);
      printStr(" 0 R");
   }
   printStr(" ]@");
   endObject();
   for (i = 0; i < fNbUrl - 1; i++) {
      newObject(_currentPage + 5 + i);
      printStr("<<@");
      printStr("/Type /Annot@");
      printStr("/Subtype /Link@");
      printStr("/Rect [");
      writeReal(fRectX1[i], kFALSE);
      writeReal(fRectY1[i]);
      writeReal(fRectX2[i]);
      writeReal(fRectY2[i]);
      printStr("]@");
      printStr("/Border [0 0 0]@");
      printStr("/A << /S /URI /URI (");
      printStr(_hasURLs[i].c_str());
      printStr(") >>@");
      printStr(">>@");
      endObject();
   }
   if (!_hasURLs.empty())
      _hasURLs.clear();
   if (!fRectX1.empty())
      fRectX1.clear();
   if (!fRectY1.empty())
      fRectY1.clear();
   if (!fRectX2.empty())
      fRectX2.clear();
   if (!fRectY2.empty())
      fRectY2.clear();
}

//!
//! Compute the Rect for url
//!
void PDF::computeRect(const char* chars, double fontsize,
                       double a, double b, double c, double d, double e, double f)
{
   double W = 0.52 * fontsize * strlen(chars);
   double ascent = 0.72 * fontsize;
   double descent = 0.22 * fontsize;

   int ax = fTextAlign / 10;
   int ay = fTextAlign % 10;
   double xShift = 0;
   double yShift = 0;
   if (ax == 2)
      xShift = -W / 2.0;
   if (ax == 3)
      xShift = -W;
   if (ay == 2)
      yShift = -(ascent - descent) / 2.0;
   if (ay == 3)
      yShift = -ascent;
   double x1 = xShift;
   double x2 = xShift + W;
   double y1 = -descent + yShift;
   double y2 = ascent + yShift;

   double A, B, C, D, E, F;
   A = fA * a + fC * b;
   B = fB * a + fD * b;
   C = fA * c + fC * d;
   D = fB * c + fD * d;
   E = fA * e + fC * f + fE;
   F = fB * e + fD * f + fF;

   double bx1 = A * x1 + C * y1 + E;
   double by1 = B * x1 + D * y1 + F;
   double bx2 = A * x2 + C * y1 + E;
   double by2 = B * x2 + D * y1 + F;
   double bx3 = A * x2 + C * y2 + E;
   double by3 = B * x2 + D * y2 + F;
   double bx4 = A * x1 + C * y2 + E;
   double by4 = B * x1 + D * y2 + F;

   double xmin = bx1;
   double xmax = bx1;
   double ymin = by1;
   double ymax = by1;

   if (bx2 < xmin)
      xmin = bx2;
   if (bx3 < xmin)
      xmin = bx3;
   if (bx4 < xmin)
      xmin = bx4;
   if (bx2 > xmax)
      xmax = bx2;
   if (bx3 > xmax)
      xmax = bx3;
   if (bx4 > xmax)
      xmax = bx4;
   if (by2 < ymin)
      ymin = by2;
   if (by3 < ymin)
      ymin = by3;
   if (by4 < ymin)
      ymin = by4;
   if (by2 > ymax)
      ymax = by2;
   if (by3 > ymax)
      ymax = by3;
   if (by4 > ymax)
      ymax = by4;

   fRectX1.push_back(xmin);
   fRectY1.push_back(ymin);
   fRectX2.push_back(xmax);
   fRectY2.push_back(ymax);
}
