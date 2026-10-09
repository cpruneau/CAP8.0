#include <cstdlib>
#include <cstring>
#include <cctype>
#include <fstream>
#include "Color.hpp"
#include "VirtualPad.hpp"
#include "Points.hpp"
#include "TeXDump.hpp"


/**
 \class TeXDump
\ingroup PS

\brief Interface to TeX.

This class allow to generate <b>PGF/TikZ</b> vector graphics output
which can be included in TeX and LaTeX documents.

PGF is a TeX macro package for generating graphics. It is platform
and format-independent and works together with the most important TeX
backend drivers, including pdftex and dvips. It comes with a
user-friendly syntax layer called TikZ.

To generate a such file it is enough to do:
~~~ {.cpp}
   gStyle->SetPaperSize(10.,10.);
   hpx->Draw();
   gPad->print("hpx.tex");
~~~

Then, the generated file (`hpx.tex`) can be included in a
LaTeX document (`simple.tex`) in the following way:
~~~ {.cpp}
\documentclass{article}
\usepackage{tikz}
\usetikzlibrary{patterns}
\usetikzlibrary{plotmarks}
\title{A simple LaTeX example}
\date{July 2013}
\begin{document}
\maketitle
The following image as been generated using the TTeXDump class:
\par
\input{hpx.tex}
\end{document}
~~~

Note the three directives needed at the top of the LaTeX file:
~~~ {.cpp}
\usepackage{tikz}
\usetikzlibrary{patterns}
\usetikzlibrary{plotmarks}
~~~

Then including the picture in the document is done with the
`\input` directive.

 The command `pdflatex simple.tex` will generate the
corresponding pdf file `simple.pdf`.
*/

////////////////////////////////////////////////////////////////////////////////
/// Default TeX constructor

TTeXDump::TTeXDump() : VirtualPS()
{
   gVirtualPS    = this;
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize the TeX interface
///
///  --fname : TeX file name
///  - wtype : TeX workstation type. Not used in the TeX driver. But as TTeXDump
///            inherits from VirtualPS it should be kept. Anyway it is not
///            necessary to specify this parameter at creation time because it
///            has a default value (which is ignore in the TeX case).

TTeXDump::TTeXDump(const char *fname, int wtype) : VirtualPS(fname, wtype)
{
   gVirtualPS    = this;

   open(fname, wtype);
}

////////////////////////////////////////////////////////////////////////////////
/// open a TeX file

void TTeXDump::open(const char *fname, int wtype)
{
   if (fStream) {
      Warning("open", "TeX file already open");
      return;
   }

   SetLineScale(gStyle->GetLineScalePS());
   fLenBuffer = 0;
   _type      = abs(wtype);

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
      Error("open", "Cannot open file:%s", fname);
      return;
   }

   gVirtualPS = this;

   ClearBuffer();

   fBoundingBox = kFALSE;
   _rangeDefined       = kFALSE;
   fStandalone  = kFALSE;

   // Set a default range
   Range(_xPageSize, _yPageSize);

   if (strstr(title(),"Standalone"))
      fStandalone = kTRUE;
   if (fStandalone) {
      printStr("\\documentclass{standalone}@");
      printStr("\\usepackage{tikz}@");
      printStr("\\usetikzlibrary{patterns,plotmarks}@");
      printStr("\\begin{document}@");
   } else {
      printStr("%\\documentclass{standalone}@");
      printStr("%\\usepackage{tikz}@");
      printStr("%\\usetikzlibrary{patterns,plotmarks}@");
      printStr("%\\begin{document}@");
   }

   newPage();
}

////////////////////////////////////////////////////////////////////////////////
/// Default TeX destructor

TTeXDump::~TTeXDump()
{
   close();
}

////////////////////////////////////////////////////////////////////////////////
/// close a TeX file

void TTeXDump::close(Option_t *)
{
   if (!gVirtualPS || !fStream)
      return;
   if (gPad)
      gPad->Update();
   printStr("@");
   printStr("\\end{tikzpicture}@");
   if (fStandalone) {
      printStr("\\end{document}@");
   } else {
      printStr("%\\end{document}@");
   }

   // close file stream
   closeStream();

   gVirtualPS = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// Activate an already open TeX file

void TTeXDump::On()
{
   // _type is used to know if the TeX file is open. Unlike TPostScript, TTeXDump
   // has no "workstation type". In fact there is only one TeX type.

   if (!_type) {
      Error("On", "no TeX file open");
      Off();
      return;
   }
   gVirtualPS = this;
}

////////////////////////////////////////////////////////////////////////////////
/// Deactivate an already open TeX file

void TTeXDump::Off()
{
   gVirtualPS = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a Box

void TTeXDump::drawBox(double x1, double y1, double x2, double y2)
{
   float x1c = XtoTeX(x1);
   float y1c = YtoTeX(y1);
   float x2c = XtoTeX(x2);
   float y2c = YtoTeX(y2);

   int fillis = fFillStyle/1000;
   int fillsi = fFillStyle%1000;

   if (fillis==1) {
      setColor(fFillColor);
      printStr("@");
      if (fCurrentAlpha != 1.) {
         printStr("\\fill [c");
         printStr(", fill opacity=");
         writeReal(fCurrentAlpha, kFALSE);
      }
      else {
         printStr("\\draw [color=c, fill=c");
      }
      printStr("] (");
      writeReal(x1c, kFALSE);
      printFast(1,",");
      writeReal(y1c, kFALSE);
      printStr(") rectangle (");
      writeReal(x2c, kFALSE);
      printFast(1,",");
      writeReal(y2c, kFALSE);
      printStr(");");
   }
   if (fillis>1 && fillis<4) {
      setColor(fFillColor);
      printStr("@");
      printStr("\\draw [pattern=");
      switch (fillsi) {
      case 1 :
         printStr("crosshatch dots");
         break;
      case 2 :
      case 3 :
         printStr("dots");
         break;
      case 4 :
         printStr("north east lines");
         break;
      case 5 :
         printStr("north west lines");
         break;
      case 6 :
         printStr("vertical lines");
         break;
      case 7 :
         printStr("horizontal lines");
         break;
      case 10 :
          printStr("bricks");
         break;
      case 13 :
         printStr("crosshatch");
         break;
      }
      printStr(", draw=none, pattern color=c");
      if (fCurrentAlpha != 1.) {
         printStr(", fill opacity=");
         writeReal(fCurrentAlpha, kFALSE);
      }
      printStr("] (");
      writeReal(x1c, kFALSE);
      printFast(1,",");
      writeReal(y1c, kFALSE);
      printStr(") rectangle (");
      writeReal(x2c, kFALSE);
      printFast(1,",");
      writeReal(y2c, kFALSE);
      printStr(");");
   }
   if (fillis == 0) {
      if (fLineWidth<=0) return;
      setColor(fLineColor);
      printStr("@");
      printStr("\\draw [c");
      printStr(",line width=");
      writeReal(0.3*_linearScale*fLineWidth, kFALSE);
      if (fCurrentAlpha != 1.) {
         printStr(", opacity=");
         writeReal(fCurrentAlpha, kFALSE);
      }
      printStr("] (");
      writeReal(x1c, kFALSE);
      printFast(1,",");
      writeReal(y1c, kFALSE);
      printStr(") -- (");
      writeReal(x1c, kFALSE);
      printFast(1,",");
      writeReal(y2c, kFALSE);
      printStr(") -- (");
      writeReal(x2c, kFALSE);
      printFast(1,",");
      writeReal(y2c, kFALSE);
      printStr(") -- (");
      writeReal(x2c, kFALSE);
      printFast(1,",");
      writeReal(y1c, kFALSE);
      printStr(") -- (");
      writeReal(x1c, kFALSE);
      printFast(1,",");
      writeReal(y1c, kFALSE);
      printStr(");");
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a Frame around a box
///
/// mode = -1  the box looks as it is behind the screen
/// mode =  1  the box looks as it is in front of the screen
/// border is the border size in already pre-computed TeX units dark is the
/// color for the dark part of the frame light is the color for the light
/// part of the frame

void TTeXDump::drawFrame(double, double, double, double,
                         int, int, int, int)
{
   Warning("drawFrame", "not yet implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a PolyLine
///
///  Draw a polyline through  the points  xy.
///  - If NN=1 moves only to point x,y.
///  - If NN=0 the x,y are  written in the TeX file
///       according to the current transformation.
///  - If NN>0 the line is clipped as a line.
///  - If NN<0 the line is clipped as a fill area.

void TTeXDump::DrawPolyLine(int, Point2Ds *)
{
   Warning("DrawPolyLine", "not yet implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a PolyLine in NDC space
///
///  Draw a polyline through  the points  xy.
///  - If NN=1 moves only to point x,y.
///  - If NN=0 the x,y are  written in the TeX file
///       according to the current transformation.
///  - If NN>0 the line is clipped as a line.
///  - If NN<0 the line is clipped as a fill area.

void TTeXDump::DrawPolyLineNDC(int, Point2Ds *)
{
   Warning("DrawPolyLineNDC", "not yet implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// Paint PolyMarker

void TTeXDump::drawMarkers(int, float *, float *)
{
   Warning("drawMarkers", "not yet implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// Paint PolyMarker

void TTeXDump::drawMarkers(int n, double *xw, double *yw)
{
   float x, y;

   setColor(fMarkerColor);

   printStr("@");
   printStr("\\foreach \\P in {");

   x = XtoTeX(xw[0]);
   y = YtoTeX(yw[0]);

   printStr("(");
   writeReal(x, kFALSE);
   printFast(1,",");
   writeReal(y, kFALSE);
   printStr(")");

   for (int i=1;i<n;i++) {
      x = XtoTeX(xw[i]);
      y = YtoTeX(yw[i]);
      printFast(3,", (");
      writeReal(x, kFALSE);
      printFast(1,",");
      writeReal(y, kFALSE);
      printFast(1,")");
   }

   printStr("}{\\draw[mark options={color=c,fill=c");

   if (fCurrentAlpha != 1.) {
      printStr(",opacity=");
      writeReal(fCurrentAlpha, kFALSE);
   }

   if (TAttMarker::GetMarkerStyleBase(fMarkerStyle) == 23 || TAttMarker::GetMarkerStyleBase(fMarkerStyle) == 32) printStr(",rotate=180");

   printStr(TString::Format("},mark size=%fpt", 8./3.33*(fMarkerSize - TMath::Floor(TAttMarker::GetMarkerLineWidth(fMarkerStyle)/2.)/4.)));
   printStr(TString::Format(", line width=%fpt", 4./3.33*TMath::Floor(TAttMarker::GetMarkerLineWidth(fMarkerStyle)/2.)));
   printStr(", mark=");
   switch (TAttMarker::GetMarkerStyleBase(fMarkerStyle)) {
   case 1 :
      printStr("*");
      printStr(",mark size=1pt");
      break;
   case 2 :
      printStr("+");
      break;
   case 3 :
      printStr("asterisk");
      break;
   case 4 :
      printStr("o");
      break;
   case 5 :
      printStr("x");
      break;
   case 20 :
      printStr("*");
      break;
   case 21 :
      printStr("square*");
      break;
   case 22 :
      printStr("triangle*");
      break;
   case 23 :
      printStr("triangle*");
      break;
   case 24 :
      printStr("o");
      break;
   case 25 :
      printStr("square");
      break;
   case 26 :
      printStr("triangle");
      break;
   case 27 :
      printStr("diamond");
      break;
   case 28 :
      printStr("cross");
      break;
   case 29 :
      printStr("newstar*");
      break;
   case 30 :
      printStr("newstar");
      break;
   case 31 :
      printStr("10-pointed star");
      break;
   case 32 :
      printStr("triangle");
      break;
   case 33 :
      printStr("diamond*");
      break;
   case 34 :
      printStr("cross*");
      break;
   }
   printStr("] plot coordinates {\\P};}");
}

////////////////////////////////////////////////////////////////////////////////
/// This function defines a path with xw and yw and draw it according the
/// value of nn:
///
///  - If nn>0 a line is drawn.
///  - If nn<0 a closed polygon is drawn.

void TTeXDump::drawPS(int nn, double *xw, double *yw)
{
   int  n = TMath::Abs(nn);
   float x, y;

   if( n <= 1) {
      Error("drawPS", "Two points are needed");
      return;
   }

   x = XtoTeX(xw[0]);
   y = YtoTeX(yw[0]);

   int fillis = fFillStyle/1000;
   int fillsi = fFillStyle%1000;

   if (nn>0) {
      if (fLineWidth<=0) return;
      setColor(fLineColor);
      printStr("@");
      printStr("\\draw [c");
      TString spec = gStyle->GetLineStyleString(fLineStyle);
      TString tikzSpec;
      TString stripped = TString{spec.Strip(TString::kBoth)};
      if (stripped.Length()) {
         tikzSpec.Append(",dash pattern=");
         Ssiz_t i{0}, j{0};
         bool on{true}, iterate{true};

         while (iterate){
            j = stripped.Index(" ", 1, i, TString::kExact);
            if (j == kNPOS){
               iterate = false;
               j = stripped.Length();
            }

            if (on) {
               tikzSpec.Append("on ");
               on = false;
            } else {
               tikzSpec.Append("off ");
               on = true;
            }
            int num = TString{stripped(i, j - i)}.Atoi();
            float pt = 0.2*num;
            tikzSpec.Append(TString::Format("%.2fpt ", pt));
            i = j + 1;
         }
         printStr(tikzSpec.Data());
      }
      printStr(",line width=");
      writeReal(0.3*_linearScale*fLineWidth, kFALSE);
      if (fCurrentAlpha != 1.) {
         printStr(",opacity=");
         writeReal(fCurrentAlpha, kFALSE);
      }
   } else {
      setColor(fFillColor);
      if (fillis==1) {
         printStr("@");
         printStr("\\draw [c, fill=c");
      } else if (fillis==0) {
         printStr("@");
         printStr("\\draw [c");
      } else {
         printStr("\\draw [pattern=");
         switch (fillsi) {
         case 1 :
            printStr("crosshatch dots");
            break;
         case 2 :
         case 3 :
            printStr("dots");
            break;
         case 4 :
            printStr("north east lines");
            break;
         case 5 :
            printStr("north west lines");
            break;
         case 6 :
            printStr("vertical lines");
            break;
         case 7 :
            printStr("horizontal lines");
            break;
         case 10 :
             printStr("bricks");
            break;
         case 13 :
            printStr("crosshatch");
            break;
         }
         printStr(", draw=none, pattern color=c");
      }
      if (fCurrentAlpha != 1.) {
         printStr(", fill opacity=");
         writeReal(fCurrentAlpha, kFALSE);
      }
   }
   printStr("] (");
   writeReal(x, kFALSE);
   printFast(1,",");
   writeReal(y, kFALSE);
   printStr(") -- ");

   for (int i=1;i<n;i++) {
      x = XtoTeX(xw[i]);
      y = YtoTeX(yw[i]);
      printFast(1,"(");
      writeReal(x, kFALSE);
      printFast(1,",");
      writeReal(y, kFALSE);
      printFast(1,")");
      if (i<n-1) printStr(" -- ");
      else printStr(";@");
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Start the TeX page. This function starts the tikzpicture environment

void TTeXDump::newPage()
{
   // Compute pad conversion coefficients
   if (gPad) {
      double ww   = gPad->GetWw();
      double wh   = gPad->GetWh();
      _yPageSize        = _xPageSize*wh/ww;
   } else {
      _yPageSize = 27;
   }

   if(!fBoundingBox) {
      printStr("\\begin{tikzpicture}@");
      printStr("\\def\\CheckTikzLibraryLoaded#1{ \\ifcsname tikz@library@#1@loaded\\endcsname \\else \\PackageWarning{tikz}{usetikzlibrary{#1} is missing in the preamble.} \\fi }@");
      printStr("\\CheckTikzLibraryLoaded{patterns}@");
      printStr("\\CheckTikzLibraryLoaded{plotmarks}@");
      DefineMarkers();
      fBoundingBox = kTRUE;
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Set the range for the paper in centimetres

void TTeXDump::Range(float xsize, float ysize)
{
   _xPageSize = xsize;
   _yPageSize = ysize;

   _rangeDefined = kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Set color index for fill areas

void TTeXDump::SetFillColor( Color_t cindex )
{
   fFillColor = cindex;
}

////////////////////////////////////////////////////////////////////////////////
/// Set color index for lines

void TTeXDump::SetLineColor( Color_t cindex )
{
   fLineColor = cindex;
}

////////////////////////////////////////////////////////////////////////////////
/// Change the line style
///
///  - linestyle = 2 dashed
///  - linestyle = 3 dotted
///  - linestyle = 4 dash-dotted
///  - linestyle = else solid (1 in is used most of the time)

void TTeXDump::SetLineStyle(Style_t linestyle)
{
   fLineStyle = linestyle;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the lines width.

void TTeXDump::SetLineWidth(Width_t linewidth)
{
   fLineWidth = linewidth;
}

////////////////////////////////////////////////////////////////////////////////
/// Set size for markers.

void TTeXDump::SetMarkerSize( Size_t msize)
{
   fMarkerSize = msize;
}

////////////////////////////////////////////////////////////////////////////////
/// Set color index for markers.

void TTeXDump::SetMarkerColor( Color_t cindex)
{
   fMarkerColor = cindex;
}

////////////////////////////////////////////////////////////////////////////////
/// Set color with its color index

void TTeXDump::setColor(int color)
{
   if (color < 0) color = 0;
   Color *col = gROOT->GeColor(color);

   if (col) {
      setColor(col->GetRed(), col->GetGreen(), col->GetBlue());
      fCurrentAlpha = col->GetAlpha();
   } else {
      setColor(1., 1., 1.);
      fCurrentAlpha = 1.;
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Set color with its R G B components
///
///  - r: % of red in [0,1]
///  - g: % of green in [0,1]
///  - b: % of blue in [0,1]

void TTeXDump::setColor(float r, float g, float b)
{
   if (fCurrentRed == r && fCurrentGreen == g && fCurrentBlue == b) return;

   fCurrentRed   = r;
   fCurrentGreen = g;
   fCurrentBlue  = b;
   printStr("@");
   printStr("\\definecolor{c}{rgb}{");
   writeReal(r, kFALSE);
   printFast(1,",");
   writeReal(g, kFALSE);
   printFast(1,",");
   writeReal(b, kFALSE);
   printFast(2,"};");
}

////////////////////////////////////////////////////////////////////////////////
/// Set color index for text

void TTeXDump::SetTexColor( Color_t cindex )
{
   fTexColor = cindex;
}

////////////////////////////////////////////////////////////////////////////////
/// Draw text
///
///  - xx: x position of the text
///  - yy: y position of the text
///  - chars: text to be drawn

void TTeXDump::text(double x, double y, const char *chars)
{
   double wh = (double)gPad->GetPadWidth();
   double hh = (double)gPad->GetPadHeight();
   float tsize, ftsize;
   if (wh < hh) {
      tsize = fTextSize*wh;
      int sizeTTF = (int)(tsize+0.5);
      ftsize = (sizeTTF*_xPageSize*gPad->GetAbsWNDC())/wh;
   } else {
      tsize = fTextSize*hh;
      int sizeTTF = (int)(tsize+0.5);
      ftsize = (sizeTTF*_yPageSize*gPad->GetAbsHNDC())/hh;
   }
   ftsize *= 2.22097;
   if (ftsize <= 0) return;

   TString t(chars);
   if (t.Index("\\")>=0 || t.Index("^{")>=0 || t.Index("_{")>=0) {
      t.Prepend("$");
      t.Append("$");
   } else {
      t.ReplaceAll("<","$<$");
      t.ReplaceAll(">","$>$");
      t.ReplaceAll("_","\\_");
   }
   t.ReplaceAll("&","\\&");
   t.ReplaceAll("#","\\#");
   t.ReplaceAll("%","\\%");

   int txalh = fTextAlign/10;
   if (txalh <1) txalh = 1; else if (txalh > 3) txalh = 3;
   int txalv = fTextAlign%10;
   if (txalv <1) txalv = 1; else if (txalv > 3) txalv = 3;
   setColor(fTexColor);
   printStr("@");
   printStr("\\draw");
   if (txalh!=2 || txalv!=2) {
      printStr(" [anchor=");
      if (txalv==1) printStr("base");
      if (txalv==3) printStr("north");
      if (txalh==1) printStr(" west");
      if (txalh==3) printStr(" east");
      printFast(1,"]");
   }
   printFast(2," (");
   writeReal(XtoTeX(x), kFALSE);
   printFast(1,",");
   writeReal(YtoTeX(y), kFALSE);
   printStr(") node[scale=");
   writeReal(ftsize, kFALSE);
   printStr(", color=c");
   if (fCurrentAlpha != 1.) {
      printStr(",opacity=");
      writeReal(fCurrentAlpha, kFALSE);
   }
   printStr(", rotate=");
   writeReal(fTextAngle, kFALSE);
   printFast(2,"]{");
   printStr(t.Data());
   printFast(2,"};");
}

////////////////////////////////////////////////////////////////////////////////
/// Draw text with URL. Same as text.
///

void TTeXDump::textUrl(double x, double y, const char *chars, const char *)
{
   text(x, y, chars);
}

////////////////////////////////////////////////////////////////////////////////
/// write a string of characters in NDC

void TTeXDump::textNDC(double u, double v, const char *chars)
{
   double x = gPad->GetX1() + u*(gPad->GetX2() - gPad->GetX1());
   double y = gPad->GetY1() + v*(gPad->GetY2() - gPad->GetY1());
   text(x, y, chars);
}

////////////////////////////////////////////////////////////////////////////////
/// Convert U from NDC coordinate to TeX

float TTeXDump::UtoTeX(double u)
{
   double cm = _xPageSize*(gPad->GetAbsXlowNDC() + u*gPad->GetAbsWNDC());
   return cm;
}

////////////////////////////////////////////////////////////////////////////////
/// Convert V from NDC coordinate to TeX

float TTeXDump::VtoTeX(double v)
{
   double cm = _yPageSize*(gPad->GetAbsYlowNDC() + v*gPad->GetAbsHNDC());
   return cm;
}

////////////////////////////////////////////////////////////////////////////////
/// Convert X from world coordinate to TeX

float TTeXDump::XtoTeX(double x)
{
   double u = (x - gPad->GetX1())/(gPad->GetX2() - gPad->GetX1());
   return  UtoTeX(u);
}

////////////////////////////////////////////////////////////////////////////////
/// Convert Y from world coordinate to TeX

float TTeXDump::YtoTeX(double y)
{
   double v = (y - gPad->GetY1())/(gPad->GetY2() - gPad->GetY1());
   return  VtoTeX(v);
}

////////////////////////////////////////////////////////////////////////////////
/// Begin the cell Array painting

void TTeXDump::cellArrayBegin(int, int, double, double, double,
                          double)
{
   Warning("cellArrayBegin", "not yet implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// Paint the cell Array

void TTeXDump::cellArrayFill(int, int, int)
{
   Warning("cellArrayFill", "not yet implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// End the cell Array painting

void TTeXDump::cellArrayEnd()
{
   Warning("cellArrayEnd", "not yet implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// Not needed in TeX case

void TTeXDump::drawPS(int, float *, float *)
{
   Warning("drawPS", "not yet implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// add additional pgfplotmarks

void TTeXDump::DefineMarkers()
{
  // open cross
  printStr("\\pgfdeclareplotmark{cross} {@");
  printStr("\\pgfpathmoveto{\\pgfpoint{-0.3\\pgfplotmarksize}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{+0.3\\pgfplotmarksize}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{+0.3\\pgfplotmarksize}{0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{+1\\pgfplotmarksize}{0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{+1\\pgfplotmarksize}{-0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{+0.3\\pgfplotmarksize}{-0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{+0.3\\pgfplotmarksize}{-1.\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{-0.3\\pgfplotmarksize}{-1.\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{-0.3\\pgfplotmarksize}{-0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{-1.\\pgfplotmarksize}{-0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{-1.\\pgfplotmarksize}{0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{-0.3\\pgfplotmarksize}{0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathclose@");
  printStr("\\pgfusepathqstroke@");
  printStr("}@");

  // filled cross
  printStr("\\pgfdeclareplotmark{cross*} {@");
  printStr("\\pgfpathmoveto{\\pgfpoint{-0.3\\pgfplotmarksize}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{+0.3\\pgfplotmarksize}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{+0.3\\pgfplotmarksize}{0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{+1\\pgfplotmarksize}{0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{+1\\pgfplotmarksize}{-0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{+0.3\\pgfplotmarksize}{-0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{+0.3\\pgfplotmarksize}{-1.\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{-0.3\\pgfplotmarksize}{-1.\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{-0.3\\pgfplotmarksize}{-0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{-1.\\pgfplotmarksize}{-0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{-1.\\pgfplotmarksize}{0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfpoint{-0.3\\pgfplotmarksize}{0.3\\pgfplotmarksize}}@");
  printStr("\\pgfpathclose@");
  printStr("\\pgfusepathqfillstroke@");
  printStr("}@");

  // open star
  printStr("\\pgfdeclareplotmark{newstar} {@");
  printStr("\\pgfpathmoveto{\\pgfqpoint{0pt}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{44}{0.5\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{18}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{-20}{0.5\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{-54}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{-90}{0.5\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{234}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{198}{0.5\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{162}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{134}{0.5\\pgfplotmarksize}}@");
  printStr("\\pgfpathclose@");
  printStr("\\pgfusepathqstroke@");
  printStr("}@");

  // filled star
  printStr("\\pgfdeclareplotmark{newstar*} {@");
  printStr("\\pgfpathmoveto{\\pgfqpoint{0pt}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{44}{0.5\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{18}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{-20}{0.5\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{-54}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{-90}{0.5\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{234}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{198}{0.5\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{162}{\\pgfplotmarksize}}@");
  printStr("\\pgfpathlineto{\\pgfqpointpolar{134}{0.5\\pgfplotmarksize}}@");
  printStr("\\pgfpathclose@");
  printStr("\\pgfusepathqfillstroke@");
  printStr("}@");
}
