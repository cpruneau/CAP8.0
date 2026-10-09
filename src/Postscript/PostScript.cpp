// @(#)root/postscript:$Id$
// Author: Rene Brun, Olivier Couet, Pierre Juillot, Oleksandr Grebenyuk, Yue Shi Lai

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/** \class TPostScript
\ingroup PS

\brief Interface to PostScript.

To generate a Postscript (or encapsulated ps) file corresponding to
a single image in a canvas, you can:

  - Select the <B>print PostScript</B> item in the canvas <B>File</B> menu.
    By default, a Postscript file with the name of the canvas.ps is generated.
  - Click in the canvas area, near the edges, with the right mouse button
    and select the <B>print</B> item. You can select the name of the Postscript
    file. If the file name is xxx.ps, you will generate a Postscript file named
    xxx.ps. If the file name is xxx.eps, you generate an encapsulated Postscript
    file instead.
  - In your program (or macro), you can type:
~~~ {.cpp}
    c1->print("xxx.ps");
~~~
or:
~~~ {.cpp}
    c1->print("xxx.eps");
~~~
    This will generate a file corresponding to the picture in the canvas
    pointed by `c1`.
~~~ {.cpp}
    pad1->print("xxx.ps");
~~~
    prints only the picture in the pad pointed by `pad1`.

The size of the Postscript picture, by default, is computed to keep the aspect
ratio of the picture on the screen, where the size along x is always 20cm. You
can set the size of the PostScript picture before generating the picture
with a command such as:

~~~ {.cpp}
    TPostScript myps("myfile.ps",111)
    myps.Range(xsize,ysize);
    object->Draw();
    myps.close();
~~~
You can set the default paper size with:
~~~ {.cpp}
    gStyle->SetPaperSize(xsize,ysize);
~~~
You can resume writing again in this file with `myps.open();`.
Note that you may have several Postscript files opened simultaneously.

 ## Output type

The output type allows to define how the PostScript output will looks like.
It allows to define the page format (A4, Legal etc..), the orientation
(Portrait, Landscape) and the number of images (zones) per page.
The output type has the following form:

~~~ {.cpp}
    [Format][Nx][Ny][Type]
~~~

Where:

  - Format : Is an integer between 0 and 99 defining the page format:
~~~ {.cpp}
    Format = 3 the paper is in the standard A3 format.
    Format = n (1<n<98) is an An format.
    Format = 4 and Format=0 are the same and define an A4 page.
    The A0 format is selected by Format=99.
    The US format Letter is selected by Format = 100.
    The US format Legal is selected by Format = 200.
    The US format Ledger is selected by Format = 300.
~~~
  - Nx, Ny : Specify respectively the number of zones on the x and y axis.
    Nx and Ny are integers between 1 and 9.
  - Type : Can be equal to:
    -  1 : Portrait mode with a small margin at the bottom of the page.
    -  2 : Landscape mode with a small margin at the bottom of the page.
    -  4 : Portrait mode with a large margin at the bottom of the page.
    -  5 : Landscape mode with a large margin at the bottom of the page.
           The large margin is useful for some PostScript printers (very often
           for the colour printers) as they need more space to grip the paper
           for mechanical reasons. Note that some PostScript colour printers
           can also use the so called special A4 format permitting the full
           usage of the A4 area; in this case larger margins are not necessary
           and Type=1 or 2 can be used.
    -  3 : Encapsulated PostScript. This Type permits the generation of files
           which can be included in other documents, for example in LaTeX files.

## Making several pictures in the same Postscript file: case 1

The following macro is an example illustrating how to open a Postscript
file and draw several pictures. The generation of a new Postscript page
is automatic when `TCanvas::Clear` is called by `object->Draw()`.

~~~ {.cpp}
     {
        TFile f("hsimple.root");
        TCanvas c1("c1","canvas",800,600);

        // select postscript output type
        // type = 111   portrait  ps
        // type = 112   landscape ps
        // type = 113   eps
        int type = 111;

        // create a postscript file and set the paper size
        TPostScript ps("test.ps",type);
        ps.Range(16,24);  //set x,y of printed page

        // draw 3 histograms from file hsimple.root on separate pages
        hpx->Draw();
        c1.Update();      //force drawing in a macro
        hprof->Draw();
        c1.Update();
        hpx->Draw("lego1");
        c1.Update();
        ps.close();
     }
~~~

## Making several pictures in the same Postscript file: case 2

This example shows 2 pages. The canvas is divided.
`TPostScript::newPage` must be called before starting a new
picture.`object->Draw` does not clear the canvas in this case
because we clear only the pads and not the main canvas.
Note that `c1->Update` must be called at the end of the first picture.

~~~ {.cpp}
     {
        TFile *f1 = new TFile("hsimple.root");
        TCanvas *c1 = new TCanvas("c1");
        TPostScript *ps = new TPostScript("file.ps",112);
        c1->Divide(2,1);
        // picture 1
        ps->newPage();
        c1->cd(1);
        hpx->Draw();
        c1->cd(2);
        hprof->Draw();
        c1->Update();

        // picture 2
        ps->newPage();
        c1->cd(1);
        hpxpy->Draw();
        c1->cd(2);
        ntuple->Draw("px");
        c1->Update();
        ps->close();

        // invoke Postscript viewer
        gSystem->Exec("gs file.ps");
     }
~~~

## Making several pictures in the same Postscript file: case 3
This is the recommended way. If the Postscript file name finishes with
"(", the file remains opened (it is not closed). If the Postscript file name
finishes with ")" and the file has been opened with "(", the file is closed.

Example:
~~~ {.cpp}
     {
        TCanvas c1("c1");
        h1.Draw();
        c1.print("c1.ps(");  // write canvas and keep the ps file open
        h2.Draw();
        c1.print("c1.ps");   // canvas is added to "c1.ps"
        h3.Draw();
        c1.print("c1.ps)");  // canvas is added to "c1.ps" and ps file is closed
     }
~~~
The `TCanvas::print("file.ps(")` mechanism is very useful, but it can
be a little inconvenient to have the action of opening/closing a file being
atomic with printing a page. Particularly if pages are being generated in some
loop one needs to detect the special cases of first and last page and then
munge the argument to print() accordingly.
The "[" and "]" can be used instead of "(" and ")" as shown below.

Example:
~~~ {.cpp}
     c1.print("file.ps[");        // No actual print, just open file.ps

     for (int i=0; i<10; ++i) {
        // fill canvas for context i
        // ...

        c1.print("file.ps");      // Actually print canvas to the file
     }

     c1.print("file.ps]");        // No actual print, just close the file
~~~

 ## Color Model

TPostScript support two color model RGB and CMYK. CMY and CMYK models are
subtractive color models unlike RGB which is an additive. They are mainly
used for printing purposes. CMY means Cyan Magenta Yellow to convert RGB
to CMY it is enough to do: C=1-R, M=1-G and Y=1-B. CMYK has one more
component K (black). The conversion from RGB to CMYK is:

~~~ {.cpp}
     double Black   = TMath::Min(TMath::Min(1-Red,1-Green),1-Blue);
     double Cyan    = (1-Red-Black)/(1-Black);
     double Magenta = (1-Green-Black)/(1-Black);
     double Yellow  = (1-Blue-Black)/(1-Black);
~~~
CMYK add the black component which allows to have a better quality for black
printing. PostScript support the CMYK model.

To change the color model use `gStyle->setColorModelPS(c)`.

  - c = 0 means TPostScript will use RGB color model (default)
  - c = 1 means TPostScript will use CMYK color model
*/

#ifdef WIN32
#pragma optimize("",off)
#endif

#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cwchar>
#include <fstream>

#include "strlcpy.hpp"
#include "snprintf.hpp"
#include "Byteswap.hpp"
#include "TROOT.hpp"
#include "TDatime.hpp"
#include "Color.hpp"
#include "VirtualPad.hpp"
#include "Point2Ds.hpp"
#include "TPostScript.hpp"
#include "TStyle.hpp"
#include "TMath.hpp"
#include "text.hpp"
#include "TSystem.hpp"
#include "TEnv.hpp"

#include "../../../graf2d/mathtext/inc/fontembed.hpp"

// to scale fonts to the same size as the old TT version
const float kScale = 0.93376068;

// Array defining if a font must be embedded or not.
int TPostScript::fgLineJoin = 0;
int TPostScript::fgLineCap  = 0;


////////////////////////////////////////////////////////////////////////////////
/// Default PostScript constructor

TPostScript::TPostScript() : VirtualPS()
{
   for (int i = 0; i < 32; i++)
      fPatterns[i] = 0;
   for (int i = 0; i < 29; i++)
      fMustEmbed[i] = kFALSE;
   setTitle("PS");

   gVirtualPS       = this;
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize the PostScript interface
///
///  - fname : PostScript file name
///  - wtype : PostScript workstation type
///
///
///  The possible workstation types are:
///    - 111 ps  Portrait
///    - 112 ps  Landscape
///    - 113 eps

TPostScript::TPostScript(const char *fname, int wtype) : VirtualPS(fname, wtype)
{
   for (int i = 0; i < 32; i++)
      fPatterns[i] = 0;
   for (int i = 0; i < 29; i++)
      fMustEmbed[i] = kFALSE;

   setTitle("PS");
   open(fname, wtype);
}

////////////////////////////////////////////////////////////////////////////////
/// open a PostScript file

void TPostScript::open(const char *fname, int wtype)
{
   if (fStream) {
      Warning("open", "postscript file already open");
      return;
   }

   fMarkerSizeCur = 0;
   _red           = -1;
   _green         = -1;
   _blue          = -1;
   fLenBuffer     = 0;
   fClip          = 0;
   _type          = std::abs(wtype);
   fClear         = kTRUE;
   fZone          = kFALSE;
   fSave          = 0;
   fFontEmbed     = kFALSE;
   SetLineJoin(gStyle->GetJoinLinePS());
   SetLineCap(gStyle->GetCapLinePS());
   SetLineScale(gStyle->GetLineScalePS());
   gStyle->GetPaperSize(_xPageSize, _yPageSize);
   fMode          = _type%10;
   float xrange, yrange;
   if (gPad) {
      double ww = gPad->GetWw();
      double wh = gPad->GetWh();
      if (_type == 113) {
         ww *= gPad->GetWNDC();
         wh *= gPad->GetHNDC();
      }
      double ratio = wh/ww;
      if (_type == 112) {
         xrange = _yPageSize;
         yrange = xrange*ratio;
         if (yrange > _xPageSize) { yrange = _xPageSize; xrange = yrange/ratio;}
      } else {
         xrange = _xPageSize;
         yrange = _xPageSize*ratio;
         if (yrange > _yPageSize) { yrange = _yPageSize; xrange = yrange/ratio;}
      }
      _xPageSize = xrange; _yPageSize = yrange;
   }

   // open OS file
   fFileName = fname;
   if (!openStream(fFileName.Data()) || gSystem->AccessPathName(fFileName.Data(), kWritePermission)) {
      Error("open", "Cannot open file: %s", fFileName.Data());
      return;
   }
   gVirtualPS = this;

   ClearBuffer();

   if( _type == 113) {
      fBoundingBox = kFALSE;
      printStr("%!PS-Adobe-2.0 EPSF-2.0@");
   } else {
      fBoundingBox = kTRUE;
      printStr("%!PS-Adobe-2.0@");
      Initialize();
   }

   fClipStatus = kFALSE;
   _rangeDefined      = kFALSE;

   // Set a default range
   Range(_xPageSize, _yPageSize);

   fPrinted    = kFALSE;
   if (_type == 113)
      newPage();
}

////////////////////////////////////////////////////////////////////////////////
/// Default PostScript destructor

TPostScript::~TPostScript()
{
   close();
}

////////////////////////////////////////////////////////////////////////////////
/// close a PostScript file

void TPostScript::close(Option_t *)
{
   if (!gVirtualPS ||!fStream)
      return;
   if (gPad)
      gPad->Update();
   if(fMode != 3) {
      SaveRestore(-1);
      if( fPrinted ) { printStr("showpage@"); SaveRestore(-1);}
      printStr("@");
      printStr("%%Trailer@");
      printStr("%%Pages: ");
      writeInteger(fNpages);
      printStr("@");
      while (fSave > 0) { SaveRestore(-1); }
   } else {
      printStr("@");
      while (fSave > 0) { SaveRestore(-1); }
      printStr("showpage@");
      printStr("end@");
   }
   printStr("@");
   printStr("%%EOF@");

   // Embed the fonts previously used by TMathText
   if (!fFontEmbed) {
      // close the file fFileName
      if (fStream) {
         printStr("@");
         closeStream();
      }

      // Rename the file fFileName
      TString tmpname = TString::Format("%s_tmp_%d",fFileName.Data(),gSystem->GetPid());
      if (gSystem->Rename(fFileName.Data(), tmpname.Data())) {
         Error("close", "Cannot open temporary file: %s", tmpname.Data());
         return;
      }

      if (!openStream(fFileName.Data()) || gSystem->AccessPathName(fFileName.Data(), kWritePermission)) {
         Error("close", "Cannot open file: %s", fFileName.Data());
         return;
      }

      // Embed the fonts at the right place
      FILE *sg = fopen(tmpname.Data(),"r");
      if (!sg) {
         Error("close", "Cannot open file: %s", tmpname.Data());
         return;
      }
      char line[255];
      while (fgets(line, 255, sg)) {
         if (strstr(line,"EndComments")) printStr("%%DocumentNeededResources: ProcSet (FontSetInit)@");
         fStream->write(line,strlen(line));
         if (!fFontEmbed && strstr(line,"m5")) {
            FontEmbed();
            printStr("@");
         }
      }
      fclose(sg);
      if (gSystem->Unlink(tmpname.Data())) return;
   }

   fFontEmbed = kFALSE;

   // close file stream

   closeStream();

   gVirtualPS = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// Activate an already open PostScript file

void TPostScript::On()
{
   if (!_type) {
      Error("On", "no postscript file open");
      Off();
      return;
   }
   gVirtualPS = this;
}

////////////////////////////////////////////////////////////////////////////////
/// Deactivate an already open PostScript file

void TPostScript::Off()
{
   gVirtualPS = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a cell Array
///
/// Drawing a PostScript cell Array is in fact done thanks to three
/// procedures: cellArrayBegin, cellArrayFill, and cellArrayEnd.
///
///  - cellArrayBegin: Initiate the cell Array by writing the necessary
///                    PostScript procedures and the initial values of the
///                    required parameters. The input parameters are:
///                    - W: number of boxes along the width.
///                    - H: number of boxes along the height
///                    - x1,x2,y1,y2: First box coordinates.
///  - cellArrayFill:  Is called for each box of the cell Array. The first
///                    box is the top left one and the last box is the
///                    bottom right one. The input parameters are the Red,
///                    Green, and Blue components of the box colour. These
///                    Levels are between 0 and 255.
///  - cellArrayEnd:   Finishes the cell Array.
///
/// PostScript cannot handle arrays larger than 65535. So the cell Array
/// is drawn in several pieces.

void TPostScript::cellArrayBegin(int W, int /*H*/, double x1, double x2,
                                 double y1, double y2)
{
   int ix1 = XtoPS(x1);
   int iy1 = YtoPS(y1);

   float wt = (288/2.54)*gPad->GetAbsWNDC()*
   _xPageSize*((x2 - x1)/(gPad->GetX2()-gPad->GetX1()));
   float ht = (288/2.54)*gPad->GetAbsHNDC()*
   _yPageSize*((y2 - y1)/(gPad->GetY2()-gPad->GetY1()));

   fLastCellRed     = 300;
   fLastCellGreen   = 300;
   fLastCellBlue    = 300;
   fNBSameColorCell = 0;

   fNbinCT = 0;
   fNbCellW = W;
   fNbCellLine = 0;
   fMaxLines = 40000/(3*fNbCellW);

   // Define some parameters
   printStr("@/WT"); writeReal(wt)          ; printStr(" def"); // cells width
   printStr(" /HT"); writeReal(ht)          ; printStr(" def"); // cells height
   printStr(" /XS"); writeInteger(ix1)      ; printStr(" def"); // X start
   printStr(" /YY"); writeInteger(iy1)      ; printStr(" def"); // Y start
   printStr(" /NX"); writeInteger(W)        ; printStr(" def"); // Number of columns
   printStr(" /NY"); writeInteger(fMaxLines); printStr(" def"); // Number of lines

   // This PS procedure draws one cell.
   printStr(" /DrawCell ");
   printStr(   "{WT HT XX YY bf");
   printStr(   " /NBBD NBBD 1 add def");
   printStr(   " NBBD NBB eq {exit} if");
   printStr(   " /XX WT XX add def");
   printStr(   " IX NX eq ");
   printStr(      "{/YY YY HT sub def");
   printStr(      " /XX XS def");
   printStr(      " /IX 0 def} if");
   printStr(   " /IX IX 1 add def} def");

   // This PS procedure draws fMaxLines line. It takes care of duplicated
   // colors. Values "n" greater than 300 mean than the previous color
   // should be duplicated n-300 times.
   printStr(" /DrawCT ");
   printStr(   "{/NBB NX NY mul def");
   printStr(   " /XX XS def");
   printStr(   " /IX 1 def");
   printStr(   " /NBBD 0 def");
   printStr(   " /RC 0 def /GC 1 def /BC 2 def");
   printStr(   " 1 1 NBB ");
   printStr(      "{/NB CT RC get def");
   printStr(      " NB 301 ge ");
   printStr(         "{/NBL NB 300 sub def");
   printStr(         " 1 1 NBL ");
   printStr(            "{DrawCell}");
   printStr(         " for");
   printStr(         " /RC RC 1 add def");
   printStr(         " /GC RC 1 add def");
   printStr(         " /BC RC 2 add def}");
   printStr(         "{CT RC get 255 div CT GC get 255 div CT BC get 255 div setrgbcolor");
   printStr(         " DrawCell");
   printStr(         " /RC RC 3 add def");
   printStr(         " /GC GC 3 add def");
   printStr(         " /BC BC 3 add def} ifelse NBBD NBB eq {exit} if} for");
   printStr(         " /YY YY HT sub def clear} def");

   printStr(" /CT [");
}

////////////////////////////////////////////////////////////////////////////////
/// Paint the cell Array

void TPostScript::cellArrayFill(int r, int g, int b)
{
   if (fLastCellRed == r && fLastCellGreen == g && fLastCellBlue == b) {
      fNBSameColorCell++;
   } else {
      if (fNBSameColorCell != 0 ) {
         writeInteger(fNBSameColorCell+300);
         fNBSameColorCell = 0;
      }
      writeInteger(r);
      writeInteger(g);
      writeInteger(b);
      fLastCellRed = r;
      fLastCellGreen = g;
      fLastCellBlue = b;
   }

   fNbinCT++;
   if (fNbinCT == fNbCellW) {
      fNbCellLine++;
      fNbinCT = 0;
   }

   if (fNbCellLine == fMaxLines) {
      if (fNBSameColorCell != 0) writeInteger(fNBSameColorCell+300);
      printStr("] def DrawCT /CT [");
      fNbCellLine = 0;
      fLastCellRed = 300;
      fLastCellGreen = 300;
      fLastCellBlue = 300;
      fNBSameColorCell = 0;
      fNbinCT = 0;
   }
}

////////////////////////////////////////////////////////////////////////////////
/// End the cell Array painting

void TPostScript::cellArrayEnd()
{
   if (fNBSameColorCell != 0 ) writeInteger(fNBSameColorCell+300);
   printStr("] def /NY");
   writeInteger(fNbCellLine);
   printStr(" def DrawCT ");
}

////////////////////////////////////////////////////////////////////////////////
/// Define the markers

void TPostScript::DefineMarkers()
{
   printStr("/mp {newpath /y exch def /x exch def} def@");
   printStr("/side {[w .77 mul w .23 mul] .385 w mul sd w 0 l currenPoint2D t -144 r} def@");
   printStr("/mr {mp x y w2 0 360 arc} def /m24 {mr s} def /m20 {mr f} def@");
   printStr("/mb {mp x y w2 add m w2 neg 0 d 0 w neg d w 0 d 0 w d cl} def@");
   printStr("/mt {mp x y w2 add m w2 neg w neg d w 0 d cl} def@");
   printStr("/w4 {w 4 div} def@");
   printStr("/w6 {w 6 div} def@");
   printStr("/w8 {w 8 div} def@");
   printStr("/m21 {mb f} def /m25 {mb s} def /m22 {mt f} def /m26{mt s} def@");
   printStr("/m23 {mp x y w2 sub m w2 w d w neg 0 d cl f} def@");
   printStr("/m27 {mp x y w2 add m w3 neg w2 neg d w3 w2 neg d w3 w2 d cl s} def@");
   printStr("/m28 {mp x w2 sub y w2 sub w3 add m w3 0 d ");
   printStr(" 0 w3 neg d w3 0 d 0 w3 d w3 0 d ");
   printStr(" 0 w3 d w3 neg 0 d 0 w3 d w3 neg 0 d");
   printStr(" 0 w3 neg d w3 neg 0 d cl s } def@");
   printStr("/m29 {mp gsave x w2 sub y w2 add w3 sub m currenPoint2D t");
   printStr(" 4 {side} repeat cl fill gr} def@");
   printStr("/m30 {mp gsave x w2 sub y w2 add w3 sub m currenPoint2D t");
   printStr(" 4 {side} repeat cl s gr} def@");
   printStr("/m31 {mp x y w2 sub m 0 w d x w2 sub y m w 0 d");
   printStr(" x w2 .707 mul sub y w2 .707 mul add m w 1.44 div w 1.44 div neg d x w2 .707 mul sub y w2 .707 mul");
   printStr(" sub m w 1.44 div w 1.44 div d s} def@");
   printStr("/m32 {mp x y w2 sub m w2 w d w neg 0 d cl s} def@");
   printStr("/m33 {mp x y w2 add m w3 neg w2 neg d w3 w2 neg d w3 w2 d cl f} def@");
   printStr("/m34 {mp x w2 sub y w2 sub w3 add m w3 0 d ");
   printStr(" 0 w3 neg d w3 0 d 0 w3 d w3 0 d ");
   printStr(" 0 w3 d w3 neg 0 d 0 w3 d w3 neg 0 d");
   printStr(" 0 w3 neg d w3 neg 0 d cl f } def@");
   printStr("/m35 {mp x y w2 add m w2 neg w2 neg d w2 w2 neg d w2 w2 d w2 neg w2 d");
   printStr(" x y w2 sub m 0 w d x w2 sub y m w 0 d s} def@");
   printStr("/m36 {mb x w2 sub y w2 add m w w neg d x w2 sub y w2 sub m w w d s} def@");
   printStr("/m37 {mp x y m w4 neg w2 d w4 neg w2 neg d w2 0 d ");
   printStr(" w4 neg w2 neg d w2 0 d w4 neg w2 d w2 0 d w4 neg w2 d w4 neg w2 neg d cl s} def@");
   printStr("/m38 {mp x w4 sub y w2 add m w4 neg w4 neg d 0 w2 neg d w4 w4 neg d");
   printStr(" w2 0 d w4 w4 d 0 w2 d w4 neg w4 d w2 neg 0 d");
   printStr(" x y w2 sub m 0 w d x w2 sub y m w 0 d cl s} def@");
   printStr("/m39 {mp x y m w4 neg w2 d w4 neg w2 neg d w2 0 d ");
   printStr(" w4 neg w2 neg d w2 0 d w4 neg w2 d w2 0 d w4 neg w2 d w4 neg w2 neg d cl f} def@");
   printStr("/m40 {mp x y m w4 w2 d w4 w4 neg d w2 neg w4 neg d w2 w4 neg d w4 neg w4 neg d");
   printStr(" w4 neg w2 d w4 neg w2 neg d w4 neg w4 d w2 w4 d w2 neg w4 d w4 w4 d w4 w2 neg d cl s} def@");
   printStr("/m41 {mp x y m w4 w2 d w4 w4 neg d w2 neg w4 neg d w2 w4 neg d w4 neg w4 neg d");
   printStr(" w4 neg w2 d w4 neg w2 neg d w4 neg w4 d w2 w4 d w2 neg w4 d w4 w4 d w4 w2 neg d cl f} def@");
   printStr("/m42 {mp x y w2 add m w8 neg w2 -3 4 div mul d w2 -3 4 div mul w8 neg d");
   printStr(" w2 3 4 div mul w8 neg d w8 w2 -3 4 div mul d");
   printStr(" w8 w2 3 4 div mul d w2 3 4 div mul w8 d");
   printStr(" w2 -3 4 div mul w8 d w8 neg w2 3 4 div mul d cl s} def@");
   printStr("/m43 {mp x y w2 add m w8 neg w2 -3 4 div mul d w2 -3 4 div mul w8 neg d");
   printStr(" w2 3 4 div mul w8 neg d w8 w2 -3 4 div mul d");
   printStr(" w8 w2 3 4 div mul d w2 3 4 div mul w8 d");
   printStr(" w2 -3 4 div mul w8 d w8 neg w2 3 4 div mul d cl f} def@");
   printStr("/m44 {mp x y m w6 neg w2 d w2 2 3 div mul 0 d w6 neg w2 neg d");
   printStr(" w2 w6 d 0 w2 -2 3 div mul d w2 neg w6 d");
   printStr(" w6 w2 neg d w2 -2 3 div mul 0 d w6 w2 d");
   printStr(" w2 neg w6 neg d 0 w2 2 3 div mul d w2 w6 neg d cl s} def@");
   printStr("/m45 {mp x y m w6 neg w2 d w2 2 3 div mul 0 d w6 neg w2 neg d");
   printStr(" w2 w6 d 0 w2 -2 3 div mul d w2 neg w6 d");
   printStr(" w6 w2 neg d w2 -2 3 div mul 0 d w6 w2 d");
   printStr(" w2 neg w6 neg d 0 w2 2 3 div mul d w2 w6 neg d cl f} def@");
   printStr("/m46 {mp x y w4 add m w4 neg w4 d w4 neg w4 neg d ");
   printStr(" w4 w4 neg d w4 neg w4 neg d w4 w4 neg d w4 w4 d");
   printStr(" w4 w4 neg d w4 w4 d w4 neg w4 d w4 w4 d w4 neg w4 d w4 neg w4 neg d cl s} def@");
   printStr("/m47 {mp x y w4 add m w4 neg w4 d w4 neg w4 neg d");
   printStr(" w4 w4 neg d w4 neg w4 neg d  w4 w4 neg d w4 w4 d");
   printStr(" w4 w4 neg d w4 w4 d w4 neg w4 d w4 w4 d w4 neg w4 d w4 neg w4 neg d cl f} def@");
   printStr("/m48 {mp x y w4 add m w4 neg w4 d w4 neg w4 neg d w4 w4 neg d ");
   printStr(" w4 neg w4 neg d w4 w4 neg d w4 w4 d w4 w4 neg d w4 w4 d");
   printStr(" w4 neg w4 d w4 w4 d w4 neg w4 d w4 neg w4 neg d ");
   printStr(" w4 w4 neg d w4 neg w4 neg d w4 neg w4 d w4 w4 d cl f} def@");
   printStr("/m49 {mp x w2 sub w3 add y w2 sub w3 add m ");
   printStr(" 0 w3 neg d w3 0 d 0 w3 d w3 0 d 0 w3 d w3 neg 0 d 0 w3 d w3 neg 0 d");
   printStr(" 0 w3 neg d w3 neg 0 d 0 w3 neg d w3 0 d 0 w3 d w3 0 d 0 w3 neg d w3 neg 0 d cl f } def@");
   printStr("/m2 {mp x y w2 sub m 0 w d x w2 sub y m w 0 d s} def@");
   printStr("/m5 {mp x w2 .707 mul sub y w2 .707 mul sub m w 1.44 div w 1.44 div d x w2 .707 mul sub y w2 .707 mul add m w 1.44 div w 1.44 div neg d s} def@");
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a Box

void TPostScript::drawBox(double x1, double y1, double x2, double  y2)
{
   double x[4], y[4];
   int ix1 = XtoPS(x1);
   int ix2 = XtoPS(x2);
   int iy1 = YtoPS(y1);
   int iy2 = YtoPS(y2);
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
         writeInteger(ix2 - ix1);
         writeInteger(iy2 - iy1);
         writeInteger(ix1);
         writeInteger(iy1);
         printFast(3," bf");
      }
   }
   if (fillis == 1) {
      setColor(fFillColor);
      writeInteger(ix2 - ix1);
      writeInteger(iy2 - iy1);
      writeInteger(ix1);
      writeInteger(iy1);
      printFast(3," bf");
   }
   if (fillis == 0) {
      if (fLineWidth<=0) return;
      setColor(fLineColor);
      writeInteger(ix2 - ix1);
      writeInteger(iy2 - iy1);
      writeInteger(ix1);
      writeInteger(iy1);
      printFast(3," bl");
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a Frame around a box
///
///  - mode = -1  box looks as it is behind the screen
///  - mode =  1  box looks as it is in front of the screen
///  - border is the border size in already precomputed PostScript units
///  - dark  is the color for the dark part of the frame
///  - light is the color for the light part of the frame

void TPostScript::drawFrame(double xl, double yl, double xt, double  yt,
                            int mode, int border, int dark, int light)
{
   int xps[7], yps[7];
   int i, ixd0, iyd0, idx, idy, ixdi, iydi, ix, iy;

   // Draw top&left part of the box
   if (mode == -1) setColor(dark);
   else            setColor(light);
   int bordPS = 4*border;
   xps[0] = XtoPS(xl);          yps[0] = YtoPS(yl);
   xps[1] = xps[0] + bordPS;    yps[1] = yps[0] + bordPS;
   xps[2] = xps[1];             yps[2] = YtoPS(yt) - bordPS;
   xps[3] = XtoPS(xt) - bordPS; yps[3] = yps[2];
   xps[4] = XtoPS(xt);          yps[4] = YtoPS(yt);
   xps[5] = xps[0];             yps[5] = yps[4];
   xps[6] = xps[0];             yps[6] = yps[0];

   ixd0 = xps[0];
   iyd0 = yps[0];
   writeInteger(ixd0);
   writeInteger(iyd0);

   printFast(2," m");
   idx = 0;
   idy = 0;
   for (i=1;i<7;i++) {
      ixdi = xps[i];
      iydi = yps[i];
      ix   = ixdi - ixd0;
      iy   = iydi - iyd0;
      ixd0 = ixdi;
      iyd0 = iydi;
      if( ix && iy) {
         if( idx ) { MovePS(idx,0); idx = 0; }
         if( idy ) { MovePS(0,idy); idy = 0; }
         MovePS(ix,iy);
         continue;
      }
      if ( ix ) {
         if( idy )  { MovePS(0,idy); idy = 0; }
         if( !idx ) { idx = ix; continue;}
         if( ix*idx > 0 )       idx += ix;
         else { MovePS(idx,0);  idx  = ix; }
         continue;
      }
      if( iy ) {
         if( idx ) { MovePS(idx,0); idx = 0; }
         if( !idy) { idy = iy; continue;}
         if( iy*idy > 0 )         idy += iy;
         else { MovePS(0,idy);    idy  = iy; }
      }
   }
   if( idx ) MovePS(idx,0);
   if( idy ) MovePS(0,idy);
   printFast(2," f");

   // Draw bottom&right part of the box
   if (mode == -1) setColor(light);
   else            setColor(dark);
   xps[0] = XtoPS(xl);          yps[0] = YtoPS(yl);
   xps[1] = xps[0] + bordPS;    yps[1] = yps[0] + bordPS;
   xps[2] = XtoPS(xt) - bordPS; yps[2] = yps[1];
   xps[3] = xps[2];             yps[3] = YtoPS(yt) - bordPS;
   xps[4] = XtoPS(xt);          yps[4] = YtoPS(yt);
   xps[5] = xps[4];             yps[5] = yps[0];
   xps[6] = xps[0];             yps[6] = yps[0];

   ixd0 = xps[0];
   iyd0 = yps[0];
   writeInteger(ixd0);
   writeInteger(iyd0);

   printFast(2," m");
   idx = 0;
   idy = 0;
   for (i=1;i<7;i++) {
      ixdi = xps[i];
      iydi = yps[i];
      ix   = ixdi - ixd0;
      iy   = iydi - iyd0;
      ixd0 = ixdi;
      iyd0 = iydi;
      if( ix && iy) {
         if( idx ) { MovePS(idx,0); idx = 0; }
         if( idy ) { MovePS(0,idy); idy = 0; }
         MovePS(ix,iy);
         continue;
      }
      if ( ix ) {
         if( idy )  { MovePS(0,idy); idy = 0; }
         if( !idx ) { idx = ix; continue;}
         if( ix*idx > 0 )       idx += ix;
         else { MovePS(idx,0);  idx  = ix; }
         continue;
      }
      if( iy ) {
         if( idx ) { MovePS(idx,0); idx = 0; }
         if( !idy) { idy = iy; continue;}
         if( iy*idy > 0 )         idy += iy;
         else { MovePS(0,idy);    idy  = iy; }
      }
   }
   if( idx ) MovePS(idx,0);
   if( idy ) MovePS(0,idy);
   printFast(2," f");
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a PolyLine
///
///  Draw a polyline through  the points xy.
///  - If nn=1 moves only to point x,y.
///  - If nn=0 the x,y are  written  in the PostScript file
///            according to the current transformation.
///  - If nn>0 the line is clipped as a line.
///  - If nn<0 the line is clipped as a fill area.

void TPostScript::DrawPolyLine(int nn, Point2Ds *xy)
{
   int  i, n, ixd0, iyd0, idx, idy, ixdi, iydi, ix, iy;
   Style_t linestylesav = fLineStyle;
   Width_t linewidthsav = fLineWidth;
   if (nn > 0) {
      if (fLineWidth<=0) return;
      n = nn;
      SetStyle(fLineStyle);
      SetWidth(fLineWidth);
      setColor(int(fLineColor));
   } else {
      n = -nn;
      SetStyle(1);
      SetWidth(1);
      setColor(int(fLineColor));
   }

   ixd0 = XtoPS(xy[0].GetX());
   iyd0 = YtoPS(xy[0].GetY());
   writeInteger(ixd0);
   writeInteger(iyd0);
   if( n <= 1) {
      if( n == 0) goto END;
      printFast(2," m");
      goto END;
   }

   printFast(2," m");
   idx = 0;
   idy = 0;
   for (i=1;i<n;i++) {
      ixdi = XtoPS(xy[i].GetX());
      iydi = YtoPS(xy[i].GetY());
      ix   = ixdi - ixd0;
      iy   = iydi - iyd0;
      ixd0 = ixdi;
      iyd0 = iydi;
      if( ix && iy) {
         if( idx ) { MovePS(idx,0); idx = 0; }
         if( idy ) { MovePS(0,idy); idy = 0; }
         MovePS(ix,iy);
         continue;
      }
      if ( ix ) {
         if( idy )  { MovePS(0,idy); idy = 0; }
         if( !idx ) { idx = ix; continue;}
         if( ix*idx > 0 )       idx += ix;
         else { MovePS(idx,0);  idx  = ix; }
         continue;
      }
      if( iy ) {
         if( idx ) { MovePS(idx,0); idx = 0; }
         if( !idy) { idy = iy; continue;}
         if( iy*idy > 0 )         idy += iy;
         else { MovePS(0,idy);    idy  = iy; }
      }
   }
   if( idx ) MovePS(idx,0);
   if( idy ) MovePS(0,idy);

   if (nn > 0 ) {
      if (xy[0].GetX() == xy[n-1].GetX() && xy[0].GetY() == xy[n-1].GetY()) printFast(3," cl");
      printFast(2," s");
   } else {
      printFast(2," f");
   }
END:
   if (nn < 0) {
      SetStyle(linestylesav);
      SetWidth(linewidthsav);
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a PolyLine in NDC space
///
///  Draw a polyline through the points xy.
///  - If nn=1 moves only to point x,y.
///  - If nn=0 the x,y are  written  in the PostScript file
///            according to the current transformation.
///  - If nn>0 the line is clipped as a line.
///  - If nn<0 the line is clipped as a fill area.

void TPostScript::DrawPolyLineNDC(int nn, Point2Ds *xy)
{
   int  i, n, ixd0, iyd0, idx, idy, ixdi, iydi, ix, iy;
   Style_t linestylesav = fLineStyle;
   Width_t linewidthsav = fLineWidth;
   if (nn > 0) {
      if (fLineWidth<=0) return;
      n = nn;
      SetStyle(fLineStyle);
      SetWidth(fLineWidth);
      setColor(int(fLineColor));
   } else {
      n = -nn;
      SetStyle(1);
      SetWidth(1);
      setColor(int(fLineColor));
   }

   ixd0 = UtoPS(xy[0].GetX());
   iyd0 = VtoPS(xy[0].GetY());
   writeInteger(ixd0);
   writeInteger(iyd0);
   if( n <= 1) {
      if( n == 0) goto END;
      printFast(2," m");
      goto END;
   }

   printFast(2," m");
   idx = 0;
   idy = 0;
   for (i=1;i<n;i++) {
      ixdi = UtoPS(xy[i].GetX());
      iydi = VtoPS(xy[i].GetY());
      ix   = ixdi - ixd0;
      iy   = iydi - iyd0;
      ixd0 = ixdi;
      iyd0 = iydi;
      if( ix && iy) {
         if( idx ) { MovePS(idx,0); idx = 0; }
         if( idy ) { MovePS(0,idy); idy = 0; }
         MovePS(ix,iy);
         continue;
      }
      if ( ix ) {
         if( idy )  { MovePS(0,idy); idy = 0; }
         if( !idx ) { idx = ix; continue;}
         if( ix*idx > 0 )       idx += ix;
         else { MovePS(idx,0);  idx  = ix; }
         continue;
      }
      if( iy ) {
         if( idx ) { MovePS(idx,0); idx = 0; }
         if( !idy) { idy = iy; continue;}
         if( iy*idy > 0 )         idy += iy;
         else { MovePS(0,idy);    idy  = iy; }
      }
   }
   if( idx ) MovePS(idx,0);
   if( idy ) MovePS(0,idy);

   if (nn > 0 ) {
      if (xy[0].GetX() == xy[n-1].GetX() && xy[0].GetY() == xy[n-1].GetY()) printFast(3," cl");
      printFast(2," s");
   } else {
      printFast(2," f");
   }
END:
   if (nn < 0) {
      SetStyle(linestylesav);
      SetWidth(linewidthsav);
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Draw markers at the n WC points x, y

void TPostScript::drawMarkers(int n, float *x, float *y)
{
   int i, np, markerstyle;
   float markersize;
   char chtemp[10];

   if (!fMarkerSize) return;
   fMarkerStyle = TMath::Abs(fMarkerStyle);
   Style_t linestylesav = fLineStyle;
   Width_t linewidthsav = fLineWidth;
   SetStyle(1);
   SetWidth(TMath::Max(1, int(TAttMarker::GetMarkerLineWidth(fMarkerStyle))));
   setColor(int(fMarkerColor));
   markerstyle = TAttMarker::GetMarkerStyleBase(fMarkerStyle);
   if (markerstyle <= 0) strlcpy(chtemp, " m20",10);
   if (markerstyle == 1) strlcpy(chtemp, " m20",10);
   if (markerstyle == 2) strlcpy(chtemp, " m2",10);
   if (markerstyle == 3) strlcpy(chtemp, " m31",10);
   if (markerstyle == 4) strlcpy(chtemp, " m24",10);
   if (markerstyle == 5) strlcpy(chtemp, " m5",10);
   if (markerstyle >= 6 && markerstyle <= 19) strlcpy(chtemp, " m20",10);
   if (markerstyle >= 20 && markerstyle <= 49 ) snprintf(chtemp,10," m%d", markerstyle);
   if (markerstyle >= 50) strlcpy(chtemp, " m20",10);

   // Set the PostScript marker size
   if (markerstyle == 1 || (markerstyle >= 9 && markerstyle <= 19)) {
      markersize = 2.;
   } else if (markerstyle == 6) {
      markersize = 4.;
   } else if (markerstyle == 7) {
      markersize = 8.;
   } else {
      float symbolsize  = fMarkerSize - TMath::Floor(TAttMarker::GetMarkerLineWidth(fMarkerStyle)/2.)/4.*_linearScale/4.;
      const int kBASEMARKER = 8;
      float sbase = symbolsize*kBASEMARKER;
      float s2x = sbase / float(gPad->GetWw() * gPad->GetAbsWNDC());
      markersize = this->UtoPS(s2x) - this->UtoPS(0);
   }

   if (fMarkerSizeCur != markersize) {
      fMarkerSizeCur = markersize;
      printFast(3," /w");
      writeInteger(int(markersize+0.5));
      printFast(40," def /w2 {w 2 div} def /w3 {w 3 div} def");
   }

   writeInteger(XtoPS(x[0]));
   writeInteger(YtoPS(y[0]));
   if (n == 1) {
      printStr(chtemp);
      SetStyle(linestylesav);
      SetWidth(linewidthsav);
      return;
   }
   np = 1;
   for (i=1;i<n;i++) {
      writeInteger(XtoPS(x[i]));
      writeInteger(YtoPS(y[i]));
      np++;
      if (np == 100 || i == n-1) {
         writeInteger(np);
         printFast(2," {");
         printStr(chtemp);
         printFast(3,"} R");
         np = 0;
      }
   }
   SetStyle(linestylesav);
   SetWidth(linewidthsav);
}

////////////////////////////////////////////////////////////////////////////////
/// Draw markers at the n WC points x, y

void TPostScript::drawMarkers(int n, double *x, double *y)
{
   int i, np, markerstyle;
   float markersize;
   char chtemp[10];

   if (!fMarkerSize) return;
   fMarkerStyle = TMath::Abs(fMarkerStyle);
   Style_t linestylesav = fLineStyle;
   Width_t linewidthsav = fLineWidth;
   SetStyle(1);
   SetWidth(TMath::Max(1, int(TAttMarker::GetMarkerLineWidth(fMarkerStyle))));
   setColor(int(fMarkerColor));
   markerstyle = TAttMarker::GetMarkerStyleBase(fMarkerStyle);
   if (markerstyle <= 0) strlcpy(chtemp, " m20",10);
   if (markerstyle == 1) strlcpy(chtemp, " m20",10);
   if (markerstyle == 2) strlcpy(chtemp, " m2",10);
   if (markerstyle == 3) strlcpy(chtemp, " m31",10);
   if (markerstyle == 4) strlcpy(chtemp, " m24",10);
   if (markerstyle == 5) strlcpy(chtemp, " m5",10);
   if (markerstyle >= 6 && markerstyle <= 19) strlcpy(chtemp, " m20",10);
   if (markerstyle >= 20 && markerstyle <= 49 ) snprintf(chtemp,10," m%d", markerstyle);
   if (markerstyle >= 50) strlcpy(chtemp, " m20",10);

   // Set the PostScript marker size
   if (markerstyle == 1 || (markerstyle >= 9 && markerstyle <= 19)) {
      markersize = 2.;
   } else if (markerstyle == 6) {
      markersize = 4.;
   } else if (markerstyle == 7) {
      markersize = 8.;
   } else {
      float symbolsize  = fMarkerSize - TMath::Floor(TAttMarker::GetMarkerLineWidth(fMarkerStyle)/2.)/4.*_linearScale/4.;
      const int kBASEMARKER = 8;
      float sbase = symbolsize*kBASEMARKER;
      float s2x = sbase / float(gPad->GetWw() * gPad->GetAbsWNDC());
      markersize = this->UtoPS(s2x) - this->UtoPS(0);
   }

   if (fMarkerSizeCur != markersize) {
      fMarkerSizeCur = markersize;
      printFast(3," /w");
      writeInteger(int(markersize+0.5));
      printFast(40," def /w2 {w 2 div} def /w3 {w 3 div} def");
   }

   writeInteger(XtoPS(x[0]));
   writeInteger(YtoPS(y[0]));
   if (n == 1) {
      printStr(chtemp);
      SetStyle(linestylesav);
      SetWidth(linewidthsav);
      return;
   }
   np = 1;
   for (i=1;i<n;i++) {
      writeInteger(XtoPS(x[i]));
      writeInteger(YtoPS(y[i]));
      np++;
      if (np == 100 || i == n-1) {
         writeInteger(np);
         printFast(2," {");
         printStr(chtemp);
         printFast(3,"} R");
         np = 0;
      }
   }
   SetStyle(linestylesav);
   SetWidth(linewidthsav);
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a PolyLine
///
///  Draw a polyline through the points xw,yw.
///  - If nn=1 moves only to point xw,yw.
///  - If nn=0 the XW(1) and YW(1) are  written  in the PostScript file
///            according to the current NT.
///  - If nn>0 the line is clipped as a line.
///  - If nn<0 the line is clipped as a fill area.

void TPostScript::drawPS(int nn, float *xw, float *yw)
{
   static float dyhatch[24] = {.0075,.0075,.0075,.0075,.0075,.0075,.0075,.0075,
      .01  ,.01  ,.01  ,.01  ,.01  ,.01  ,.01  ,.01  ,
      .015 ,.015 ,.015 ,.015 ,.015 ,.015 ,.015 ,.015};
   static float anglehatch[24] = {180, 90,135, 45,150, 30,120, 60,
      180, 90,135, 45,150, 30,120, 60,
      180, 90,135, 45,150, 30,120, 60};
   int  i, n, ixd0, iyd0, idx, idy, ixdi, iydi, ix, iy, fais, fasi;
   fais = fasi = n = 0;
   int jxd0 = XtoPS(xw[0]);
   int jyd0 = YtoPS(yw[0]);
   Style_t linestylesav = fLineStyle;
   Width_t linewidthsav = fLineWidth;

   if (nn > 0) {
      if (fLineWidth<=0) return;
      n = nn;
      SetStyle(fLineStyle);
      SetWidth(fLineWidth);
      setColor(int(fLineColor));
   }
   if (nn < 0) {
      n = -nn;
      SetStyle(1);
      SetWidth(1);
      setColor(int(fFillColor));
      fais = fFillStyle/1000;
      fasi = fFillStyle%1000;
      if (fais == 3 || fais == 2) {
         if (fasi > 100 && fasi <125) {
            DrawHatch(dyhatch[fasi-101],anglehatch[fasi-101], n, xw, yw);
            goto END;
         }
         if (fasi > 0 && fasi < 26) {
            SetFillPatterns(fasi, int(fFillColor));
         }
      }
   }

   ixd0 = jxd0;
   iyd0 = jyd0;
   writeInteger(ixd0);
   writeInteger(iyd0);
   if( n <= 1) {
      if( n == 0) goto END;
      printFast(2," m");
      goto END;
   }

   printFast(2," m");
   idx = idy = 0;
   for (i=1;i<n;i++) {
      ixdi = XtoPS(xw[i]);
      iydi = YtoPS(yw[i]);
      ix   = ixdi - ixd0;
      iy   = iydi - iyd0;
      ixd0 = ixdi;
      iyd0 = iydi;
      if( ix && iy) {
         if( idx ) { MovePS(idx,0); idx = 0; }
         if( idy ) { MovePS(0,idy); idy = 0; }
         MovePS(ix,iy);
      } else if ( ix ) {
         if( idy )  { MovePS(0,idy); idy = 0;}
         if( !idx ) { idx = ix;}
         else if( TMath::Sign(ix,idx) == ix )       idx += ix;
         else { MovePS(idx,0);  idx  = ix;}
      } else if( iy ) {
         if( idx ) { MovePS(idx,0); idx = 0;}
         if( !idy) { idy = iy;}
         else if( TMath::Sign(iy,idy) == iy)         idy += iy;
         else { MovePS(0,idy);    idy  = iy;}
      }
   }
   if (idx) MovePS(idx,0);
   if (idy) MovePS(0,idy);

   if (nn > 0 ) {
      if (xw[0] == xw[n-1] && yw[0] == yw[n-1]) printFast(3," cl");
      printFast(2," s");
   } else {
      if (fais == 0) {printFast(5," cl s"); goto END;}
      if (fais == 3 || fais == 2) {
         if (fasi > 0 && fasi < 26) {
            printFast(3," FA");
            _red   = -1;
            _green = -1;
            _blue  = -1;
         }
         goto END;
      }
      printFast(2," f");
   }
END:
   if (nn < 0) {
      SetStyle(linestylesav);
      SetWidth(linewidthsav);
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Draw a PolyLine
///
/// Draw a polyline through  the points xw,yw.
///  - If nn=1 moves only to point xw,yw.
///  - If nn=0 the xw(1) and YW(1) are  written  in the PostScript file
///    ---     according to the current NT.
///  - If nn>0 the line is clipped as a line.
///  - If nn<0 the line is clipped as a fill area.

void TPostScript::drawPS(int nn, double *xw, double *yw)
{
   static float dyhatch[24] = {.0075,.0075,.0075,.0075,.0075,.0075,.0075,.0075,
      .01  ,.01  ,.01  ,.01  ,.01  ,.01  ,.01  ,.01  ,
      .015 ,.015 ,.015 ,.015 ,.015 ,.015 ,.015 ,.015};
   static float anglehatch[24] = {180, 90,135, 45,150, 30,120, 60,
      180, 90,135, 45,150, 30,120, 60,
      180, 90,135, 45,150, 30,120, 60};
   int  i, n, ixd0, iyd0, idx, idy, ixdi, iydi, ix, iy, fais, fasi;
   fais = fasi = n = 0;
   int jxd0 = XtoPS(xw[0]);
   int jyd0 = YtoPS(yw[0]);
   Style_t linestylesav = fLineStyle;
   Width_t linewidthsav = fLineWidth;

   if (nn > 0) {
      if (fLineWidth<=0) return;
      n = nn;
      SetStyle(fLineStyle);
      SetWidth(fLineWidth);
      setColor(int(fLineColor));
   }
   if (nn < 0) {
      n = -nn;
      SetStyle(1);
      SetWidth(1);
      setColor(int(fFillColor));
      fais = fFillStyle/1000;
      fasi = fFillStyle%1000;
      if (fais == 3 || fais == 2) {
         if (fasi > 100 && fasi <125) {
            DrawHatch(dyhatch[fasi-101],anglehatch[fasi-101], n, xw, yw);
            goto END;
         }
         if (fasi > 0 && fasi < 26) {
            SetFillPatterns(fasi, int(fFillColor));
         }
      }
   }

   ixd0 = jxd0;
   iyd0 = jyd0;
   writeInteger(ixd0);
   writeInteger(iyd0);
   if( n <= 1) {
      if( n == 0) goto END;
      printFast(2," m");
      goto END;
   }

   printFast(2," m");
   idx = idy = 0;
   for (i=1;i<n;i++) {
      ixdi = XtoPS(xw[i]);
      iydi = YtoPS(yw[i]);
      ix   = ixdi - ixd0;
      iy   = iydi - iyd0;
      ixd0 = ixdi;
      iyd0 = iydi;
      if( ix && iy) {
         if( idx ) { MovePS(idx,0); idx = 0; }
         if( idy ) { MovePS(0,idy); idy = 0; }
         MovePS(ix,iy);
      } else if ( ix ) {
         if( idy )  { MovePS(0,idy); idy = 0;}
         if( !idx ) { idx = ix;}
         else if( TMath::Sign(ix,idx) == ix )       idx += ix;
         else { MovePS(idx,0);  idx  = ix;}
      } else if( iy ) {
         if( idx ) { MovePS(idx,0); idx = 0;}
         if( !idy) { idy = iy;}
         else if( TMath::Sign(iy,idy) == iy)         idy += iy;
         else { MovePS(0,idy);    idy  = iy;}
      }
   }
   if (idx) MovePS(idx,0);
   if (idy) MovePS(0,idy);

   if (nn > 0 ) {
      if (xw[0] == xw[n-1] && yw[0] == yw[n-1]) printFast(3," cl");
      printFast(2," s");
   } else {
      if (fais == 0) {printFast(5," cl s"); goto END;}
      if (fais == 3 || fais == 2) {
         if (fasi > 0 && fasi < 26) {
            printFast(3," FA");
            _red   = -1;
            _green = -1;
            _blue  = -1;
         }
         goto END;
      }
      printFast(2," f");
   }
END:
   if (nn < 0) {
      SetStyle(linestylesav);
      SetWidth(linewidthsav);
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Draw Fill area with hatch styles

void TPostScript::DrawHatch(float, float, int, float *, float *)
{
   Warning("DrawHatch", "hatch fill style not yet implemented");
}

////////////////////////////////////////////////////////////////////////////////
/// Draw Fill area with hatch styles

void TPostScript::DrawHatch(float, float, int, double *, double *)
{
   Warning("DrawHatch", "hatch fill style not yet implemented");
}

////////////////////////////////////////////////////////////////////////////////

bool TPostScript::FontEmbedType1(const char *filename)
{
   std::ifstream font_file(filename, std::ios::binary);

   // We cannot read directly using iostream iterators due to
   // signedness
   font_file.seekg(0, std::ios::end);

   const size_t font_file_length = font_file.tellg();

   font_file.seekg(0, std::ios::beg);

   std::vector<unsigned char> font_data(font_file_length, '\0');

   font_file.read(reinterpret_cast<char *>(&font_data[0]),
                  font_file_length);

   std::string font_name;
   std::string postscript_string =
   mathtext::font_embed_postscript_t::font_embed_type_1(
                                                        font_name, font_data);

   if (!postscript_string.empty()) {
      printRaw(postscript_string.size(), postscript_string.data());
      printStr("@");

      return true;
   }

   return false;
}

////////////////////////////////////////////////////////////////////////////////

bool TPostScript::FontEmbedType2(const char *filename)
{
   std::ifstream font_file(filename, std::ios::binary);

   // We cannot read directly using iostream iterators due to
   // signedness
   font_file.seekg(0, std::ios::end);

   const size_t font_file_length = font_file.tellg();

   font_file.seekg(0, std::ios::beg);

   std::vector<unsigned char> font_data(font_file_length, '\0');

   font_file.read(reinterpret_cast<char *>(&font_data[0]), font_file_length);

   std::string font_name;
   std::string postscript_string =
   mathtext::font_embed_postscript_t::font_embed_type_2(font_name, font_data);

   if (!postscript_string.empty()) {
      printRaw(postscript_string.size(), postscript_string.data());
      printStr("@");

      return true;
   }

   return false;
}

////////////////////////////////////////////////////////////////////////////////

bool TPostScript::FontEmbedType42(const char *filename)
{
   std::ifstream font_file(filename, std::ios::binary);

    // We cannot read directly using iostream iterators due to signedness

   font_file.seekg(0, std::ios::end);

   const size_t font_file_length = font_file.tellg();

   font_file.seekg(0, std::ios::beg);

   std::vector<unsigned char> font_data(font_file_length, '\0');

   font_file.read(reinterpret_cast<char *>(&font_data[0]), font_file_length);

   std::string font_name;
   std::string postscript_string =
   mathtext::font_embed_postscript_t::font_embed_type_42(font_name, font_data);

   if (!postscript_string.empty()) {
      printRaw(postscript_string.size(), postscript_string.data());
      printStr("@");

      return true;
   }
   fprintf(stderr, "%s:%d:\n", __FILE__, __LINE__);

   return false;
}

////////////////////////////////////////////////////////////////////////////////
/// Embed font in PS file.

void TPostScript::FontEmbed()
{
   static const char *fonttable[32][2] = {
      { "Root.TTFont.0", "FreeSansBold.otf" },
      { "Root.TTFont.1", "FreeSerifItalic.otf" },
      { "Root.TTFont.2", "FreeSerifBold.otf" },
      { "Root.TTFont.3", "FreeSerifBoldItalic.otf" },
      { "Root.TTFont.4", "FreeSans.otf" },
      { "Root.TTFont.5", "FreeSansOblique.otf" },
      { "Root.TTFont.6", "FreeSansBold.otf" },
      { "Root.TTFont.7", "FreeSansBoldOblique.otf" },
      { "Root.TTFont.8", "FreeMono.otf" },
      { "Root.TTFont.9", "FreeMonoOblique.otf" },
      { "Root.TTFont.10", "FreeMonoBold.otf" },
      { "Root.TTFont.11", "FreeMonoBoldOblique.otf" },
      { "Root.TTFont.12", "symbol.ttf" },
      { "Root.TTFont.13", "FreeSerif.otf" },
      { "Root.TTFont.14", "wingding.ttf" },
      { "Root.TTFont.15", "symbol.ttf" },
      { "Root.TTFont.STIXGen", "STIXGeneral.otf" },
      { "Root.TTFont.STIXGenIt", "STIXGeneralItalic.otf" },
      { "Root.TTFont.STIXGenBd", "STIXGeneralBol.otf" },
      { "Root.TTFont.STIXGenBdIt", "STIXGeneralBolIta.otf" },
      { "Root.TTFont.STIXSiz1Sym", "STIXSiz1Sym.otf" },
      { "Root.TTFont.STIXSiz1SymBd", "STIXSiz1SymBol.otf" },
      { "Root.TTFont.STIXSiz2Sym", "STIXSiz2Sym.otf" },
      { "Root.TTFont.STIXSiz2SymBd", "STIXSiz2SymBol.otf" },
      { "Root.TTFont.STIXSiz3Sym", "STIXSiz3Sym.otf" },
      { "Root.TTFont.STIXSiz3SymBd", "STIXSiz3SymBol.otf" },
      { "Root.TTFont.STIXSiz4Sym", "STIXSiz4Sym.otf" },
      { "Root.TTFont.STIXSiz4SymBd", "STIXSiz4SymBol.otf" },
      { "Root.TTFont.STIXSiz5Sym", "STIXSiz5Sym.otf" },
      { "Root.TTFont.ME", "DroidSansFallback.ttf" },
      { "Root.TTFont.CJKMing", "DroidSansFallback.ttf" },
      { "Root.TTFont.CJKCothic", "DroidSansFallback.ttf" }
   };

   printStr("%%IncludeResource: ProcSet (FontSetInit)@");

   // try to load font (font must be in Root.TTFontPath resource)
   const char *ttpath = gEnv->GetValue("Root.TTFontPath",
                                       TROOT::GetTTFFontDir());

   for (int fontid = 1; fontid < 30; fontid++) {
      if (fontid != 15 && fMustEmbed[fontid-1]) {
         TString filename = gEnv->GetValue(fonttable[fontid][0], fonttable[fontid][1]);
         const char *ttfont = gSystem->FindFile(ttpath, filename, kReadPermission);
         if (!ttfont) {
            Error("FontEmbed",
                  "font %d (filename '%s') not found in path",
                  fontid, filename.Data());
         } else if (FontEmbedType2(ttfont)) {
            // nothing
         } else if(FontEmbedType1(ttfont)) {
            // nothing
         } else if(FontEmbedType42(ttfont)) {
            // nothing
         } else {
            Error("FontEmbed",
                  "failed to embed font %d (filename '%s')",
                  fontid, filename.Data());
         }
      }
   }
   printStr("%%IncludeResource: font Times-Roman@");
   printStr("%%IncludeResource: font Times-Italic@");
   printStr("%%IncludeResource: font Times-Bold@");
   printStr("%%IncludeResource: font Times-BoldItalic@");
   printStr("%%IncludeResource: font Helvetica@");
   printStr("%%IncludeResource: font Helvetica-Oblique@");
   printStr("%%IncludeResource: font Helvetica-Bold@");
   printStr("%%IncludeResource: font Helvetica-BoldOblique@");
   printStr("%%IncludeResource: font Courier@");
   printStr("%%IncludeResource: font Courier-Oblique@");
   printStr("%%IncludeResource: font Courier-Bold@");
   printStr("%%IncludeResource: font Courier-BoldOblique@");
   printStr("%%IncludeResource: font Symbol@");
   printStr("%%IncludeResource: font ZapfDingbats@");

   fFontEmbed = kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Font Re-encoding

void TPostScript::FontEncode()
{
   printStr("/reEncode ");
   printStr("{exch findfont");
   printStr(" dup length dict begin");
   printStr(" {1 index /FID eq ");
   printStr(" {pop pop}");
   printStr(" {def} ifelse");
   printStr(" } forall");
   printStr(" /Encoding exch def");
   printStr(" currentdict end");
   printStr(" dup /FontName get exch");
   printStr(" definefont pop");
   printStr(" } def");
   printStr(" [/Times-Bold /Times-Italic /Times-BoldItalic /Helvetica");
   printStr(" /Helvetica-Oblique /Helvetica-Bold /Helvetica-BoldOblique");
   printStr(" /Courier /Courier-Oblique /Courier-Bold /Courier-BoldOblique");
   printStr(" /Times-Roman /AvantGarde-Book /AvantGarde-BookOblique");
   printStr(" /AvantGarde-Demi /AvantGarde-DemiOblique /Bookman-Demi");
   printStr(" /Bookman-DemiItalic /Bookman-Light /Bookman-LightItalic");
   printStr(" /Helvetica-Narrow /Helvetica-Narrow-Bold /Helvetica-Narrow-BoldOblique");
   printStr(" /Helvetica-Narrow-Oblique /NewCenturySchlbk-Roman /NewCenturySchlbk-Bold");
   printStr(" /NewCenturySchlbk-BoldItalic /NewCenturySchlbk-Italic");
   printStr(" /Palatino-Bold /Palatino-BoldItalic /Palatino-Italic /Palatino-Roman");
   printStr(" ] {ISOLatin1Encoding reEncode } forall");
}

////////////////////////////////////////////////////////////////////////////////
/// PostScript Initialisation
///
/// This method initialize the following PostScript procedures:
///
/// | Macro Name | Input parameters |            Explanation            |
/// |------------|------------------|-----------------------------------|
/// |     l      | x y              | Draw a line to the x y position   |
/// |     m      | x y              | Move to the position x y          |
/// |     box    | dx dy x y        | Define a box                      |
/// |     bl     | dx dy x y        | Draw a line box                   |
/// |     bf     | dx dy x y        | Draw a filled box                 |
/// |     t      | x y              | Translate                         |
/// |     r      | angle            | Rotate                            |
/// |     rl     | i j              | Roll the stack                    |
/// |     d      | x y              | Draw a relative line to x y       |
/// |     X      | x                | Draw a relative line to x (y=0)   |
/// |     Y      | y                | Draw a relative line to y (x=0)   |
/// |     rm     | x y              | Move relatively to x y            |
/// |     gr     |                  | Restore the graphic context       |
/// |     lw     | lwidth           | Set line width to lwidth          |
/// |     sd     | [] 0             | Set dash line define by []        |
/// |     s      |                  | Stroke mode                       |
/// |     c      | r g b            | Set rgb color to r g b            |
/// |     cl     |                  | close path                        |
/// |     f      |                  | Fill the last describe path       |
/// |     mXX    | x y              | Draw the marker type XX at (x,y)  |
/// |     Zone   | ix iy            | Define the current zone           |
/// |     black  |                  | The color is black                |
/// |     C      | dx dy x y        | Clipping on                       |
/// |     NC     |                  | Clipping off                      |
/// |     R      |                  | repeat                            |
/// |     ita    |                  | Used to make the symbols italic   |
/// |     K      |                  | kshow                             |

void TPostScript::Initialize()
{
   double rpxmin, rpymin, width, heigth;
   rpxmin = rpymin = width = heigth = 0;
   int format;
   fNpages=1;
   for (int i=0;i<32;i++) fPatterns[i]=0;

   // Mode is last digit of PostScript Workstation type
   //    mode=1,2 for portrait/landscape black and white
   //    mode=3 for Encapsulated PostScript File
   //    mode=4 for portrait colour
   //    mode=5 for lanscape colour
   int atype = abs(_type);
   fMode       = atype%10;
   if( fMode <= 0 || fMode > 5) {
      Error("Initialize", "invalid file type %d", fMode);
      return;
   }

   // fNXzone (fNYzone) is the total number of windows in x (y)
   fNXzone = (atype%1000)/100;
   fNYzone = (atype%100)/10;
   if( fNXzone <= 0 ) fNXzone = 1;
   if( fNYzone <= 0 ) fNYzone = 1;
   fIXzone = 1;
   fIYzone = 1;

   // format = 0-99 is the European page format (A4,A3 ...)
   // format = 100 is the US format  8.5x11.0 inch
   // format = 200 is the US format  8.5x14.0 inch
   // format = 300 is the US format 11.0x17.0 inch
   format = atype/1000;
   if( format == 0 )  format = 4;
   if( format == 99 ) format = 0;

   printStr("%%Title: ");
   const char *pstitle = gStyle->titlePS();
   if (gPad && !pstitle[0]) pstitle = gPad->GetMother()->title();
   if (strlen(GetName())<=80) printStr(GetName());
   if(!pstitle[0] && fMode != 3) {;
      printFast(2," (");
      if ( format <= 99 ) {;
         printFast(2," A");
         writeInteger(format);
         printFast(1,")");
      }
      else {
         if ( format == 100 ) printFast(8," Letter)");
         if ( format == 200 ) printFast(7," Legal)");
         if ( format == 300 ) printFast(8," Ledger)");
      }
      printStr("@");
      printStr("%%Pages: (atend)@");
   }
   else {
      if (!strchr(pstitle,'\n')) {
         printFast(2,": ");
         printStr(pstitle);
      }
      printStr("@");
   }

   printFast(24,"%%Creator: ROOT Version ");
   printStr(gROOT->GetVersion());
   printStr("@");
   printFast(16,"%%CreationDate: ");
   TDatime t;
   printStr(t.AsString());
   printStr("@");

   if ( fMode == 1 || fMode == 4) printStr("%%Orientation: Portrait@");
   if ( fMode == 2 || fMode == 5) printStr("%%Orientation: Landscape@");

   printStr("%%EndComments@");
   printStr("%%BeginProlog@");

   if( fMode == 3)printStr("80 dict begin@");

   // Initialisation of PostScript procedures
   printStr("/s {stroke} def /l {lineto} def /m {moveto} def /t {translate} def@");
   printStr("/r {rotate} def /rl {roll}  def /R {repeat} def@");
   printStr("/d {rlineto} def /rm {rmoveto} def /gr {grestore} def /f {eofill} def@");
   if (gStyle->psColorMode()) {
      printStr("/c {setcmykcolor} def /black {0 0 0 1 setcmykcolor} def /sd {setdash} def@");
   } else {
      printStr("/c {setrgbcolor} def /black {0 setgray} def /sd {setdash} def@");
   }
   printStr("/cl {closepath} def /sf {scalefont setfont} def /lw {setlinewidth} def@");
   printStr("/box {m dup 0 exch d exch 0 d 0 exch neg d cl} def@");
   printStr("/NC{systemdict begin initclip end}def/C{NC box clip newpath}def@");
   printStr("/bl {box s} def /bf {gsave box gsave f grestore 1 lw [] 0 sd s grestore} def /Y { 0 exch d} def /X { 0 d} def @");
   printStr("/K {{pop pop 0 moveto} exch kshow} bind def@");
   printStr("/ita {/ang 15 def gsave [1 0 ang dup sin exch cos div 1 0 0] concat} def @");

   DefineMarkers();

   FontEncode();

   // mode=1 for portrait black/white
   if (fMode == 1)  {
      rpxmin = 0.7;
      rpymin = TMath::Sqrt(2.)*rpxmin;
      switch (format) {
         case 100 :
            width  = (8.5*2.54)-2.*rpxmin;
            heigth = (11.*2.54)-2.*rpymin;
            break;
         case 200 :
            width  = (8.5*2.54)-2.*rpxmin;
            heigth = (14.*2.54)-2.*rpymin;
            break;
         case 300 :
            width  = (11.*2.54)-2.*rpxmin;
            heigth = (17.*2.54)-2.*rpymin;
            break;
         default  :
            width  = 21.0-2.*rpxmin;
            heigth = 29.7-2.*rpymin;
      };
   }

   // mode=2 for landscape black/white
   if (fMode == 2)  {
      rpymin = 0.7;
      rpxmin = TMath::Sqrt(2.)*rpymin;
      switch (format) {
         case 100 :
            width  = (11.*2.54)-2.*rpxmin;
            heigth = (8.5*2.54)-2.*rpymin;
            break;
         case 200 :
            width  = (14.*2.54)-2.*rpxmin;
            heigth = (8.5*2.54)-2.*rpymin;
            break;
         case 300 :
            width  = (17.*2.54)-2.*rpxmin;
            heigth = (11.*2.54)-2.*rpymin;
            break;
         default  :
            width  = 29.7-2.*rpxmin;
            heigth = 21-2.*rpymin;
      };
   }

   // mode=3 encapsulated PostScript
   if (fMode == 3)  {
      width   = 20;
      heigth  = 20;
      format  = 4;
      fNXzone = 1;
      fNYzone = 1;
   }

   // mode=4 for portrait colour
   if (fMode == 4)  {
      rpxmin = 0.7;
      rpymin = 3.4;
      switch (format) {
         case 100 :
            width  = (8.5*2.54)-2.*rpxmin;
            heigth = (11.*2.54)-2.*rpymin;
            break;
         case 200 :
            width  = (8.5*2.54)-2.*rpxmin;
            heigth = (14.*2.54)-2.*rpymin;
            break;
         case 300 :
            width  = (11.*2.54)-2.*rpxmin;
            heigth = (17.*2.54)-2.*rpymin;
            break;
         default  :
            width  = (21.0-2*rpxmin);
            heigth = (29.7-2.*rpymin);
      };
   }

   // mode=5 for lanscape colour
   if (fMode == 5)  {
      rpxmin = 3.4;
      rpymin = 0.7;
      switch (format) {
         case 100 :
            width  = (11.*2.54)-2.*rpxmin;
            heigth = (8.5*2.54)-2.*rpymin;
            break;
         case 200 :
            width  = (14.*2.54)-2.*rpxmin;
            heigth = (8.5*2.54)-2.*rpymin;
            break;
         case 300 :
            width  = (17.*2.54)-2.*rpxmin;
            heigth = (11.*2.54)-2.*rpymin;
            break;
         default  :
            width  = (29.7-2*rpxmin);
            heigth = (21-2.*rpymin);
      };
   }

   double value = 0;
   if       (format <  100) value = 21*TMath::Power(TMath::Sqrt(2.), 4-format);
   else  if (format == 100) value = 8.5*2.54;
   else  if (format == 200) value = 8.5*2.54;
   else  if (format == 300) value = 11.*2.54;
   if       (format >= 100) format = 4;

   // Compute size (in points) of the window for each picture = f(fNXzone,fNYzone)
   double sizex = width/double(fNXzone)*TMath::Power(TMath::Sqrt(2.), 4-format);
   double sizey = heigth/double(fNYzone)*TMath::Power(TMath::Sqrt(2.), 4-format);
   int npx      = 4*CMtoPS(sizex);
   int npy      = 4*CMtoPS(sizey);
   if (sizex > sizey) fMaxsize = CMtoPS(sizex);
   else               fMaxsize = CMtoPS(sizey);

   // Procedure Zone
   if (fMode != 3)  {
      printFast(33,"/Zone {/iy exch def /ix exch def ");
      printFast(10," ix 1 sub ");
      writeInteger(npx);
      printFast(5," mul ");
      writeReal(float(fNYzone));
      printFast(8," iy sub ");
      writeInteger(npy);
      printStr(" mul t} def@");
   } else {
      printStr("@");
   }

   printStr("%%EndProlog@");
   printStr("%%BeginSetup@");
   printStr("%%EndSetup@");
   printFast(8,"newpath ");
   SaveRestore(1);
   if (fMode == 1 || fMode == 4)  {
      writeInteger(CMtoPS(rpxmin));
      writeInteger(CMtoPS(rpymin));
      printFast(2," t");
   }
   if (fMode == 2 || fMode == 5)  {
      printFast(7," 90 r 0");
      writeInteger(CMtoPS(-value));
      printFast(3," t ");
      writeInteger(CMtoPS(rpxmin));
      writeInteger(CMtoPS(rpymin));
      printFast(2," t");
   }

   printFast(15," .25 .25 scale ");
   if (fMode != 3) {
      SaveRestore(1);
      printStr("@");
      printStr("%%Page: 1 1@");
      SaveRestore(1);
   }

   //Check is user has defined a special header in the current style
   int nh = strlen(gStyle->GetHeaderPS());
   if (nh) {
      printFast(nh,gStyle->GetHeaderPS());
      if (fMode != 3) SaveRestore(1);
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Move to a new position

void TPostScript::MovePS(int ix, int iy)
{
   if (ix != 0 && iy != 0)  {
      writeInteger(ix);
      writeInteger(iy);
      printFast(2," d");
   } else if (ix != 0)  {
      writeInteger(ix);
      printFast(2," X");
   } else if (iy != 0)  {
      writeInteger(iy);
      printFast(2," Y");
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Move to a new PostScript page

void TPostScript::newPage()
{
   //   Compute pad conversion coefficients
   if (gPad) {
      //     if (!gPad->GetPadPaint()) gPad->Update();
      double ww   = gPad->GetWw();
      double wh   = gPad->GetWh();
      _yPageSize       = _xPageSize*wh/ww;
   } else _yPageSize = 27;

   if(_type  == 113 && !fBoundingBox) {
      bool psave = fPrinted;
      printStr("@%%BoundingBox: ");
      double xlow=0, ylow=0, xup=1, yup=1;
      if (gPad) {
         xlow = gPad->GetAbsXlowNDC();
         xup  = xlow + gPad->GetAbsWNDC();
         ylow = gPad->GetAbsYlowNDC();
         yup  = ylow + gPad->GetAbsHNDC();
      }
      writeInteger(CMtoPS(_xPageSize*xlow));
      writeInteger(CMtoPS(_yPageSize*ylow));
      writeInteger(CMtoPS(_xPageSize*xup));
      writeInteger(CMtoPS(_yPageSize*yup));
      printStr("@");
      Initialize();
      fBoundingBox  = kTRUE;
      fPrinted      = psave;
   }
   if (fPrinted) {
      if (fSave) SaveRestore(-1);
      fClear    = kTRUE;
      fPrinted  = kFALSE;
   }
   Zone();
}

////////////////////////////////////////////////////////////////////////////////
/// Set the range for the paper in centimeters

void TPostScript::Range(float xsize, float ysize)
{
   float xps=0, yps=0, xncm=0, yncm=0, dxwn=0, dywn=0, xwkwn=0, ywkwn=0, xymax=0;

   _xPageSize = xsize;
   _yPageSize = ysize;
   if( _type != 113) { xps = _xPageSize;  yps = _yPageSize; }
   else {              xps = xsize;   yps = ysize; }

   if( xsize <= xps && ysize < yps) {
      if ( xps > yps ) xymax = xps;
      else             xymax = yps;
      xncm  = xsize/xymax;
      yncm  = ysize/xymax;
      dxwn  = ((xps/xymax)-xncm)/2;
      dywn  = ((yps/xymax)-yncm)/2;
   } else {
      if (xps/yps < 1) xwkwn = xps/yps;
      else             xwkwn = 1;
      if (yps/xps < 1) ywkwn = yps/xps;
      else             ywkwn = 1;

      if (xsize < ysize)  {
         xncm = ywkwn*xsize/ysize;
         yncm = ywkwn;
         dxwn = (xwkwn-xncm)/2;
         dywn = 0;
         if( dxwn < 0) {
            xncm = xwkwn;
            dxwn = 0;
            yncm = xwkwn*ysize/xsize;
            dywn = (ywkwn-yncm)/2;
         }
      } else {
         xncm = xwkwn;
         yncm = xwkwn*ysize/xsize;
         dxwn = 0;
         dywn = (ywkwn-yncm)/2;
         if( dywn < 0) {
            yncm = ywkwn;
            dywn = 0;
            xncm = ywkwn*xsize/ysize;
            dxwn = (xwkwn-xncm)/2;
         }
      }
   }
   fXVP1  = dxwn;
   fXVP2  = xncm+dxwn;
   fYVP1  = dywn;
   fYVP2  = yncm+dywn;
   _rangeDefined = kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Compute number of gsaves for restore
/// This allows to write the correct number of grestore at the
/// end of the PS file.

void TPostScript::SaveRestore(int flag)
{
   if (flag == 1) { printFast(7," gsave ");  fSave++; }
   else           { printFast(4," gr ");     fSave--; }
}

////////////////////////////////////////////////////////////////////////////////
/// Set color index for fill areas

void TPostScript::SetFillColor( Color_t cindex )
{
   fFillColor = cindex;
   if (gStyle->GetFillColor() <= 0) cindex = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Patterns definition
///
/// Define the pattern ipat in the current PS file. ipat can vary from
/// 1 to 25. Together with the pattern, the color (color) in which the
/// pattern has to be drawn is also required. A pattern is defined in the
/// current PS file only the first time it is used. Some level 2
/// Postscript functions are used, so on level 1 printers, patterns will
/// not work. This is not a big problem because patterns are
/// defined only if they are used, so if they are not used a PS level 1
/// file will not be polluted by level 2 features, and in any case the old
/// patterns used a lot of memory which made them almost unusable on old
/// level 1 printers. Finally we should say that level 1 devices are
/// becoming very rare. The official PostScript is now level 3 !

void TPostScript::SetFillPatterns(int ipat, int color)
{
   char cdef[28];
   char cpat[5];
   snprintf(cpat,5," P%2.2d", ipat);

   // fPatterns is used as an array of chars. If fPatterns[ipat] != 0 the
   // pattern number ipat as already be defined is this file and it
   // is not necessary to redefine it. fPatterns is set to zero in Initialize.
   // The pattern number 26 allows to know if the macro "cs" has already
   // been defined in the current file (see label 200).
   if (fPatterns[ipat] == 0) {

      // Define the Patterns. Line width must be 1
      // Setting fLineWidth to -1 will force the line width definition next time
      // TPostScript::SetLineWidth will be called.
      fLineWidth = -1;
      printFast(5," 1 lw");
      printStr(" << /PatternType 1 /PaintType 2 /TilingType 1");
      switch (ipat) {
         case 1 :
            printStr(" /BBox [ 0 0 98 4 ]");
            printStr(" /XStep 98 /YStep 4");
            printStr(" /PaintProc { begin gsave");
            printStr(" [1] 0 sd 2 4 m 99 4 l s 1 3 m 98 3 l s");
            printStr(" 2 2 m 99 2 l s 1 1 m 98 1 l s");
            printStr(" gr end } >> [ 4.0 0 0 4.0 0 0 ]");
            break;
         case 2 :
            printStr(" /BBox [ 0 0 96 4 ]");
            printStr(" /XStep 96 /YStep 4");
            printStr(" /PaintProc { begin gsave");
            printStr(" [1 3] 0 sd 2 4 m 98 4 l s 0 3 m 96 3 l s");
            printStr(" 2 2 m 98 2 l s 0 1 m 96 1 l s");
            printStr(" gr end } >> [ 3.0 0 0 3.0 0 0 ]");
            break;
         case 3 :
            printStr(" /BBox [ 0 0 96 16 ]");
            printStr(" /XStep 96 /YStep 16");
            printStr(" /PaintProc { begin gsave");
            printStr(" [1 3] 0 sd 2 13 m 98 13 l s 0 9 m 96 9 l s");
            printStr(" 2 5 m 98 5 l s 0 1 m 96 1 l s");
            printStr(" gr end } >> [ 2.0 0 0 2.0 0 0 ]");
            break;
         case 4 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 0 0 m 100 100 l s");
            printStr(" gr end } >> [ 0.24 0 0 0.24 0 0 ]");
            break;
         case 5 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 0 100 m 100 0 l s");
            printStr(" gr end } >> [ 0.24 0 0 0.24 0 0 ]");
            break;
         case 6 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 50 0 m 50 100 l s");
            printStr(" gr end } >> [ 0.12 0 0 0.12 0 0 ]");
            break;
         case 7 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 0 50 m 100 50 l s");
            printStr(" gr end } >> [ 0.12 0 0 0.12 0 0 ]");
            break;
         case 8 :
            printStr(" /BBox [ 0 0 101 101 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 0 0 m 0 30 l 30 0 l f 0 70 m 0 100 l 30 100 l f");
            printStr(" 70 100 m 100 100 l 100 70 l f 70 0 m 100 0 l");
            printStr(" 100 30 l f 50 20 m 20 50 l 50 80 l 80 50 l f");
            printStr(" 50 80 m 30 100 l s 20 50 m 0 30 l s 50 20 m");
            printStr(" 70 0 l s 80 50 m 100 70 l s");
            printStr(" gr end } >> [ 0.24 0 0 0.24 0 0 ]");
            break;
         case 9 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 0 50 m 50 50 50 180 360 arc");
            printStr(" 0 50 m 0 100 50 270 360 arc");
            printStr(" 50 100 m 100 100 50 180 270 arc s");
            printStr(" gr end } >> [ 0.24 0 0 0.24 0 0 ]");
            break;
         case 10 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 0 50 m 100 50 l 1 1 m 100 1 l");
            printStr(" 0 0 m 0 50 l 100 0 m 100 50 l");
            printStr(" 50 50 m 50 100 l s");
            printStr(" gr end } >> [ 0.24 0 0 0.24 0 0 ]");
            break;
         case 11 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 0 0 m 0 20 l 50 0 m 50 20 l");
            printStr(" 100 0 m 100 20 l 0 80 m 0 100 l");
            printStr(" 50 80 m 50 100 l 100 80 m 100 100 l");
            printStr(" 25 30 m 25 70 l 75 30 m 75 70 l");
            printStr(" 0 100 m 20 85 l 50 100 m 30 85 l");
            printStr(" 50 100 m 70 85 l 100 100 m 80 85 l");
            printStr(" 0 0 m 20 15 l 50 0 m 30 15 l");
            printStr(" 50 0 m 70 15 l 100 0 m 80 15 l");
            printStr(" 5 35 m 45 65 l 5 65 m 45 35 l");
            printStr(" 55 35 m 95 65 l 55 65 m 95 35 l s");
            printStr(" gr end } >> [ 0.5 0 0 0.5 0 0 ]");
            break;
         case 12 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 0 80 m 0 100 20 270 360 arc");
            printStr(" 30 100 m 50 100 20 180 360 arc");
            printStr(" 80 100 m 100 100 20 180 270 arc");
            printStr(" 20 0 m 0 0 20 0 90 arc");
            printStr(" 70 0 m 50 0 20 0 180 arc");
            printStr(" 100 20 m 100 0 20 90 180 arc");
            printStr(" 45 50 m 25 50 20 0 360 arc");
            printStr(" 95 50 m 75 50 20 0 360 arc s");
            printStr(" gr end } >> [ 0.5 0 0 0.5 0 0 ]");
            break;
         case 13 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 0 0 m 100 100 l 0 100 m 100 0 l s");
            printStr(" gr end } >> [ 0.24 0 0 0.24 0 0 ]");
            break;
         case 14 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 80 /YStep 80");
            printStr(" /PaintProc { begin gsave");
            printStr(" 0 20 m 100 20 l 20 0 m 20 100 l");
            printStr(" 0 80 m 100 80 l 80 0 m 80 100 l");
            printStr(" 20 40 m 60 40 l 60 20 m 60 60 l");
            printStr(" 40 40 m 40 80 l 40 60 m 80 60 l s");
            printStr(" gr end } >> [ 0.60 0 0 0.60 0 0 ]");
            break;
         case 15 :
            printStr(" /BBox [ 0 0 60 60 ]");
            printStr(" /XStep 60 /YStep 60");
            printStr(" /PaintProc { begin gsave");
            printStr(" 0 55 m 0 60 5 270 360 arc");
            printStr(" 25 60 m 30 60 5 180 360 arc");
            printStr(" 55 60 m 60 60 5 180 270 arc");
            printStr(" 20 30 m 15 30 5 0 360 arc");
            printStr(" 50 30 m 45 30 5 0 360");
            printStr(" arc 5 0 m 0 0 5 0 90 arc");
            printStr(" 35 0 m 30 0 5 0 180 arc");
            printStr(" 60 5 m 60 0 5 90 180 arc s");
            printStr(" gr end } >> [ 0.41 0 0 0.41 0 0 ]");
            break;
         case 16 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 50 50 m 25 50 25 0 180 arc s");
            printStr(" 50 50 m 75 50 25 180 360 arc s");
            printStr(" gr end } >> [ 0.4 0 0 0.2 0 0 ]");
            break;
         case 17 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" [24] 0 setdash 0 0 m 100 100 l s");
            printStr(" gr end } >> [ 0.24 0 0 0.24 0 0 ]");
            break;
         case 18 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" [24] 0 setdash 0 100 m 100 0 l s");
            printStr(" gr end } >> [ 0.24 0 0 0.24 0 0 ]");
            break;
         case 19 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 90 50 m 50 50 40 0 360 arc");
            printStr(" 0 50 m 0 100 50 270 360 arc");
            printStr(" 50 0 m 0 0 50 0 90 arc");
            printStr(" 100 50 m 100 0 50 90 180 arc");
            printStr(" 50 100 m 100 100 50 180 270 arc s");
            printStr(" gr end } >> [ 0.47 0 0 0.47 0 0 ]");
            break;
         case 20 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 50 50 m 50 75 25 270 450 arc s");
            printStr(" 50 50 m 50 25 25 90  270 arc s");
            printStr(" gr end } >> [ 0.2 0 0 0.4 0 0 ]");
            break;
         case 21 :
            printStr(" /BBox [ 0 0 101 101 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 1 1 m 25 1 l 25 25 l 50 25 l 50 50 l");
            printStr(" 75 50 l 75 75 l 100 75 l 100 100 l");
            printStr(" 50 1 m 75 1 l 75 25 l 100 25 l 100 50 l");
            printStr(" 0 50 m 25 50 l 25 75 l 50 75 l 50 100 l s");
            printStr(" gr end } >> [ 0.5 0 0 0.5 0 0 ]");
            break;
         case 22 :
            printStr(" /BBox [ 0 0 101 101 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 1 100 m 25 100 l 25 75 l 50 75 l 50 50 l");
            printStr(" 75 50 l 75 25 l 100 25 l 100 1 l");
            printStr(" 50 100 m 75 100 l 75 75 l 100 75 l 100 50 l");
            printStr(" 0 50 m 25 50 l 25 25 l 50 25 l 50 1 l s");
            printStr(" gr end } >> [ 0.5 0 0 0.5 0 0 ]");
            break;
         case 23 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" [1 7] 0 sd 0 8 50 { dup dup m 2 mul 0 l s } for");
            printStr(" 0 8 50 { dup dup 2 mul 100 m 50 add exch 50");
            printStr(" add l s } for 100 0 m 100 100 l 50 50 l f");
            printStr(" gr end } >> [ 0.24 0 0 0.24 0 0 ]");
            break;
         case 24 :
            printStr(" /BBox [ 0 0 100 100 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 100 100 m 100 36 l 88 36 l 88 88 l f");
            printStr(" 100 0 m 100 12 l 56 12 l 50 0 l f");
            printStr(" 0 0 m 48 0 l 48 48 l 50 48 l 56 60 l");
            printStr(" 36 60 l 36 12 l 0 12 l f [1 7] 0 sd");
            printStr(" 61 8 87 { dup dup dup 12 exch m 88 exch l s");
            printStr(" 16 exch 4 sub m 88 exch 4 sub l s } for");
            printStr(" 13 8 35 { dup dup dup 0 exch m 36 exch l s");
            printStr(" 4 exch 4 sub m 36 exch 4 sub l s } for");
            printStr(" 37 8 59 { dup dup dup 12 exch m 36 exch l s");
            printStr(" 16 exch 4 sub m 36 exch 4 sub l s } for");
            printStr(" 13 8 60 { dup dup dup 56 exch m 100 exch l s");
            printStr(" 60 exch 4 sub m 100 exch 4 sub l s } for");
            printStr(" gr end } >> [ 0.5 0 0 0.5 0 0 ]");
            break;
         case 25 :
            printStr(" /BBox [ 0 0 101 101 ]");
            printStr(" /XStep 100 /YStep 100");
            printStr(" /PaintProc { begin gsave");
            printStr(" 0 0 m 30 30 l 70 30 l 70 70 l 100 100 l 100 0 l");
            printStr(" f 30 30 m 30 70 l 70 70 l f");
            printStr(" gr end } >> [ 0.5 0 0 0.5 0 0 ]");
      };
      snprintf(cdef,28," makepattern /%s exch def",&cpat[1]);
      printStr(cdef);
      fPatterns[ipat] = 1;
   }

   // Define the macro cs and FA if they are not yet defined.
   if (fPatterns[26] == 0) {
      if (gStyle->psColorMode()) {
         printStr(" /cs {[/Pattern /DeviceCMYK] setColorspace} def");
         printStr(" /FA {f [/DeviceCMYK] setColorspace} def");
      } else {
         printStr(" /cs {[/Pattern /DeviceRGB] setColorspace} def");
         printStr(" /FA {f [/DeviceRGB] setColorspace} def");
      }
      fPatterns[26] = 1;
   }

   // Activate the pattern.
   printFast(3," cs");
   Color *col = gROOT->GeColor(color);
   if (col) {
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
   }
   printFast(4,cpat);
   printFast(9," setColor");
}

////////////////////////////////////////////////////////////////////////////////
/// Set color index for lines

void TPostScript::SetLineColor( Color_t cindex )
{
   fLineColor = cindex;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the value of the global parameter TPostScript::fgLineJoin.
/// This parameter determines the appearance of joining lines in a PostScript
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

void TPostScript::SetLineJoin( int linejoin )
{
   fgLineJoin = linejoin;
   if (fgLineJoin<0) fgLineJoin=0;
   if (fgLineJoin>2) fgLineJoin=2;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the value of the global parameter TPostScript::fgLineCap.
/// This parameter determines the appearance of line caps in a PostScript
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

void TPostScript::SetLineCap( int linecap )
{
   fgLineCap = linecap;
   if (fgLineCap<0) fgLineCap=0;
   if (fgLineCap>2) fgLineCap=2;
}

////////////////////////////////////////////////////////////////////////////////
/// Change the line style
///
///  - linestyle = 2 dashed
///  - linestyle = 3  dotted
///  - linestyle = 4  dash-dotted
///  - linestyle = else = solid
///
/// See TStyle::SetLineStyleString for style definition

void TPostScript::SetLineStyle(Style_t linestyle)
{
   if ( linestyle == fLineStyle) return;
   fLineStyle = linestyle;
}

////////////////////////////////////////////////////////////////////////////////
/// Change the line style in the output file

void TPostScript::SetStyle(Style_t linestyle)
{
   if (linestyle == fStyle) return;

   fStyle = linestyle;

   const char *st = gStyle->GetLineStyleString(fStyle);
   printFast(1,"[");
   int nch = strlen(st);
   printFast(nch,st);
   printFast(6,"] 0 sd");
}

////////////////////////////////////////////////////////////////////////////////
/// Change the line width

void TPostScript::SetLineWidth(Width_t linewidth)
{
   if (linewidth == fLineWidth) return;
   fLineWidth = linewidth;
}

////////////////////////////////////////////////////////////////////////////////
/// Change the line width in the output file

void  TPostScript::SetWidth(Width_t linewidth)
{
   if (linewidth == fWidth) return;

   fWidth = linewidth;

   if (fWidth!=0) {
      writeInteger(int(_linearScale*fWidth));
      printFast(3," lw");
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Set color index for markers

void TPostScript::SetMarkerColor( Color_t cindex )
{
   fMarkerColor = cindex;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the current color.

void TPostScript::setColor(int color)
{
   if (color < 0) color = 0;
   Color *col = gROOT->GeColor(color);
   if (col)
      setColor(col->GetRed(), col->GetGreen(), col->GetBlue());
   else
      setColor(1., 1., 1.);
}

////////////////////////////////////////////////////////////////////////////////
/// Set directly current color (don't go via Color).

void TPostScript::setColor(float r, float g, float b)
{
   if (r == _red && g == _green && b == _blue) return;

   _red   = r;
   _green = g;
   _blue  = b;

   if (_red <= 0 && _green <= 0 && _blue <= 0 ) {
      printFast(6," black");
   } else {
      if (gStyle->psColorMode()) {
         double colBlack   = TMath::Min(TMath::Min(1-_red,1-_green),1-_blue);
         double colCyan    = (1-_red-colBlack)/(1-colBlack);
         double colMagenta = (1-_green-colBlack)/(1-colBlack);
         double colYellow  = (1-_blue-colBlack)/(1-colBlack);
         writeReal(colCyan);
         writeReal(colMagenta);
         writeReal(colYellow);
         writeReal(colBlack);
      } else {
         writeReal(_red);
         writeReal(_green);
         writeReal(_blue);
      }
      printFast(2," c");
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Set color index for text

void TPostScript::SeTexColor( Color_t cindex )
{
   fTexColor = cindex;
}

////////////////////////////////////////////////////////////////////////////////
/// write a string of characters
///
/// This method writes the string chars into a PostScript file
/// at position xx,yy in world coordinates.

void TPostScript::text(double xx, double yy, const char *chars)
{
   static const char *psfont[31][2] = {
      { "Root.PSFont.1",             "/Times-Italic" },
      { "Root.PSFont.2",             "/Times-Bold" },
      { "Root.PSFont.3",             "/Times-BoldItalic" },
      { "Root.PSFont.4",             "/Helvetica" },
      { "Root.PSFont.5",             "/Helvetica-Oblique" },
      { "Root.PSFont.6",             "/Helvetica-Bold" },
      { "Root.PSFont.7",             "/Helvetica-BoldOblique" },
      { "Root.PSFont.8",             "/Courier" },
      { "Root.PSFont.9",             "/Courier-Oblique" },
      { "Root.PSFont.10",            "/Courier-Bold" },
      { "Root.PSFont.11",            "/Courier-BoldOblique" },
      { "Root.PSFont.12",            "/Symbol" },
      { "Root.PSFont.13",            "/Times-Roman" },
      { "Root.PSFont.14",            "/ZapfDingbats" },
      { "Root.PSFont.15",            "/Symbol" },
      { "Root.PSFont.STIXGen",       "/STIXGeneral" },
      { "Root.PSFont.STIXGenIt",     "/STIXGeneral-Italic" },
      { "Root.PSFont.STIXGenBd",     "/STIXGeneral-Bold" },
      { "Root.PSFont.STIXGenBdIt",   "/STIXGeneral-BoldItalic" },
      { "Root.PSFont.STIXSiz1Sym",   "/STIXSize1Symbols" },
      { "Root.PSFont.STIXSiz1SymBd", "/STIXSize1Symbols-Bold" },
      { "Root.PSFont.STIXSiz2Sym",   "/STIXSize2Symbols" },
      { "Root.PSFont.STIXSiz2SymBd", "/STIXSize2Symbols-Bold" },
      { "Root.PSFont.STIXSiz3Sym",   "/STIXSize3Symbols" },
      { "Root.PSFont.STIXSiz3SymBd", "/STIXSize3Symbols-Bold" },
      { "Root.PSFont.STIXSiz4Sym",   "/STIXSize4Symbols" },
      { "Root.PSFont.STIXSiz4SymBd", "/STIXSize4Symbols-Bold" },
      { "Root.PSFont.STIXSiz5Sym",   "/STIXSize5Symbols" },
      { "Root.PSFont.ME",            "/DroidSansFallback" },
      { "Root.PSFont.CJKMing",       "/DroidSansFallback" },
      { "Root.PSFont.CJKGothic",     "/DroidSansFallback" }
   };

   const double kDEGRAD = TMath::Pi()/180.;
   double x = xx;
   double y = yy;
   if (!gPad) return;

   // Compute the font size. Exit if it is 0
   // The font size is computed from the TTF size to get exactly the same
   // size on the screen and in the PostScript file.
   double wh = (double) gPad->GetPadWidth();
   double hh = (double) gPad->GetPadHeight();
   if (wh <= 0 || hh <= 0)
      return;
   float tsize, ftsize;

   if (wh < hh) {
      tsize         = fTextSize*wh;
      int sizeTTF = (int)(tsize*kScale+0.5); // TTF size
      ftsize        = (sizeTTF*_xPageSize*gPad->GetAbsWNDC())/wh;
   } else {
      tsize         = fTextSize*hh;
      int sizeTTF = (int)(tsize*kScale+0.5); // TTF size
      ftsize        = (sizeTTF*_yPageSize*gPad->GetAbsHNDC())/hh;
   }
   double fontsize = 4*(72*(ftsize)/2.54);
   if( fontsize <= 0) return;

   float tsizex = gPad->AbsPixeltoX(int(tsize))-gPad->AbsPixeltoX(0);
   float tsizey = gPad->AbsPixeltoY(0)-gPad->AbsPixeltoY(int(tsize));

   int font = abs(fTextFont)/10;
   if( font > 31 || font < 1) font = 1;

   // text color.
   setColor(int(fTexColor));

   // text alignment.
   int txalh = fTextAlign/10;
   if (txalh <1) txalh = 1; else if (txalh > 3) txalh = 3;
   int txalv = fTextAlign%10;
   if (txalv <1) txalv = 1; else if (txalv > 3) txalv = 3;
   if (txalv == 3) {
      y -= 0.8*tsizey*TMath::Cos(kDEGRAD*fTextAngle);
      x += 0.8*tsizex*TMath::Sin(kDEGRAD*fTextAngle);
   } else if (txalv == 2) {
      y -= 0.4*tsizey*TMath::Cos(kDEGRAD*fTextAngle);
      x += 0.4*tsizex*TMath::Sin(kDEGRAD*fTextAngle);
   }

   UInt_t w = 0, w0 = 0;
   bool kerning;
   // In order to measure the precise character positions we need to trick
   // FreeType into rendering high-resolution characters otherwise it will
   // stick to the screen pixel grid, which is far worse than we can achieve
   // on print.
   const float scale = 16.0;
   // Save current text attributes.
   text saveAtText;
   saveAtText.TAtText::operator=(*this);
   const int len=strlen(chars);
   int *charWidthsCumul = nullptr;
   text t;
   t.SeTextSize(fTextSize * scale);
   t.SeTextFont(fTextFont);
   t.GeTextAdvance(w, chars);
   t.GeTextAdvance(w0, chars, kFALSE);
   t.TAtText::Modify();
   if (w0-w != 0) kerning = kTRUE;
   else        kerning = kFALSE;
   if (kerning) {
      // Calculate the individual character placements.
      charWidthsCumul = new int[len];
      for (int i = len - 1;i >= 0;i--) {
         UInt_t ww = 0;
         t.GeTextAdvance(ww, chars + i);
         double wwl = (gPad->AbsPixeltoX(ww)-gPad->AbsPixeltoX(0));
         charWidthsCumul[i] = (int)((XtoPS(wwl) - XtoPS(0)) / scale);
      }
   }
   // Restore text attributes.
   saveAtText.TAtText::Modify();

   double charsLength = gPad->AbsPixeltoX(w)-gPad->AbsPixeltoX(0);
   int psCharsLength = (int)((XtoPS(charsLength)-XtoPS(0)) / scale);

   // text angle.
   int psangle = int(0.5 + fTextAngle);

   // Save context.
   printStr("@");
   SaveRestore(1);

   // Clipping
   int xc1 = XtoPS(gPad->GetX1());
   int xc2 = XtoPS(gPad->GetX2());
   int yc1 = YtoPS(gPad->GetY1());
   int yc2 = YtoPS(gPad->GetY2());
   writeInteger(xc2 - xc1);
   writeInteger(yc2 - yc1);
   writeInteger(xc1);
   writeInteger(yc1);
   printStr(" C");

   // Output text position and angle. The text position is computed
   // using double to avoid precision problems.
   double vx  = (x - gPad->GetX1())/(gPad->GetX2()-gPad->GetX1());
   double cmx = _xPageSize*(gPad->GetAbsXlowNDC()+vx*gPad->GetAbsWNDC());
   writeReal((288.*cmx)/2.54);
   double vy  = (y - gPad->GetY1())/(gPad->GetY2()-gPad->GetY1());
   double cmy = _yPageSize*(gPad->GetAbsYlowNDC()+vy*gPad->GetAbsHNDC());
   writeReal((288.*cmy)/2.54);
   printStr(TString::Format(" t %d r ", psangle));
   if(txalh == 2) printStr(TString::Format(" %d 0 t ", -psCharsLength/2));
   if(txalh == 3) printStr(TString::Format(" %d 0 t ", -psCharsLength));
   printStr(gEnv->GetValue(psfont[font-1][0], psfont[font-1][1]));
   if (font != 15) {
      printStr(TString::Format(" findfont %g sf 0 0 m ",fontsize));
   } else {
      printStr(TString::Format(" findfont %g sf 0 0 m ita ",fontsize));
   }

   if (kerning) {
      printStr("@");
      // Output individual character placements
      for (int i = len-1; i >= 1; i--) {
         writeInteger(charWidthsCumul[0] - charWidthsCumul[i]);
      }
      delete [] charWidthsCumul;
      printStr("@");
   }

   // Output text.
   printStr("(");

   // Inside a PostScript string, the new line (if needed to break up long lines) must be escaped by a backslash.
   const char *crsave = fImplicitCREsc;
   fImplicitCREsc = "\\";

   char str[8];
   for (int i=0; i<len;i++) {
      if (chars[i]!='\n') {
         if (chars[i]=='(' || chars[i]==')' || chars[i]=='\\') {
            snprintf(str,8,"\\%c",chars[i]);
            printStr(str);
         } else if ((chars[i]=='-') && (font != 12)) {
            printStr("\\255");
         } else {
            snprintf(str,8,"%c",chars[i]);
            printFast(1,str);
         }
      }
   }
   printStr(")");
   fImplicitCREsc = crsave;

   if (kerning) {
      if (font != 15) printStr(" K NC");
      else            printStr(" K gr NC");
   } else {
      if (font != 15) printStr(" show NC");
      else            printStr(" show gr NC");
   }

   SaveRestore(-1);
}

////////////////////////////////////////////////////////////////////////////////
/// write a string of characters
///
/// This method writes the string chars into a PostScript file
/// at position xx,yy in world coordinates.

void TPostScript::text(double xx, double yy, const wchar_t *chars)
{
   static const char *psfont[31][2] = {
      { "Root.PSFont.1", "/FreeSerifItalic" },
      { "Root.PSFont.2", "/FreeSerifBold" },
      { "Root.PSFont.3", "/FreeSerifBoldItalic" },
      { "Root.PSFont.4", "/FreeSans" },
      { "Root.PSFont.5", "/FreeSansOblique" },
      { "Root.PSFont.6", "/FreeSansBold" },
      { "Root.PSFont.7", "/FreeSansBoldOblique" },
      { "Root.PSFont.8", "/FreeMono" },
      { "Root.PSFont.9", "/FreeMonoOblique" },
      { "Root.PSFont.10", "/FreeMonoBold" },
      { "Root.PSFont.11", "/FreeMonoBoldOblique" },
      { "Root.PSFont.12", "/SymbolMT" },
      { "Root.PSFont.13", "/FreeSerif" },
      { "Root.PSFont.14", "/Wingdings-Regular" },
      { "Root.PSFont.15", "/SymbolMT" },
      { "Root.PSFont.STIXGen", "/STIXGeneral" },
      { "Root.PSFont.STIXGenIt", "/STIXGeneral-Italic" },
      { "Root.PSFont.STIXGenBd", "/STIXGeneral-Bold" },
      { "Root.PSFont.STIXGenBdIt", "/STIXGeneral-BoldItalic" },
      { "Root.PSFont.STIXSiz1Sym", "/STIXSize1Symbols" },
      { "Root.PSFont.STIXSiz1SymBd", "/STIXSize1Symbols-Bold" },
      { "Root.PSFont.STIXSiz2Sym", "/STIXSize2Symbols" },
      { "Root.PSFont.STIXSiz2SymBd", "/STIXSize2Symbols-Bold" },
      { "Root.PSFont.STIXSiz3Sym", "/STIXSize3Symbols" },
      { "Root.PSFont.STIXSiz3SymBd", "/STIXSize3Symbols-Bold" },
      { "Root.PSFont.STIXSiz4Sym", "/STIXSize4Symbols" },
      { "Root.PSFont.STIXSiz4SymBd", "/STIXSize4Symbols-Bold" },
      { "Root.PSFont.STIXSiz5Sym", "/STIXSize5Symbols" },
      { "Root.PSFont.ME", "/DroidSansFallback" },
      { "Root.PSFont.CJKMing", "/DroidSansFallback" },
      { "Root.PSFont.CJKGothic", "/DroidSansFallback" }
   };

   int len = wcslen(chars);
   if (len<=0) return;

   const double kDEGRAD = TMath::Pi()/180.;
   double x = xx;
   double y = yy;
   if (!gPad) return;

   // Compute the font size. Exit if it is 0
   // The font size is computed from the TTF size to get exactly the same
   // size on the screen and in the PostScript file.
   double wh = (double)gPad->GetPadWidth();
   double hh = (double)gPad->GetPadHeight();
   float tsize, ftsize;

   if (wh < hh) {
      tsize         = fTextSize*wh;
      int sizeTTF = (int)(tsize*kScale+0.5); // TTF size
      ftsize        = (sizeTTF*_xPageSize*gPad->GetAbsWNDC())/wh;
   } else {
      tsize         = fTextSize*hh;
      int sizeTTF = (int)(tsize*kScale+0.5); // TTF size
      ftsize        = (sizeTTF*_yPageSize*gPad->GetAbsHNDC())/hh;
   }
   double fontsize = 4*(72*(ftsize)/2.54);
   if(fontsize <= 0) return;

   float tsizex = gPad->AbsPixeltoX(int(tsize))-gPad->AbsPixeltoX(0);
   float tsizey = gPad->AbsPixeltoY(0)-gPad->AbsPixeltoY(int(tsize));

   int font = abs(fTextFont)/10;
   if( font > 29 || font < 1) font = 1;

   // text color.
   setColor(int(fTexColor));

   // text alignment.
   int txalh = fTextAlign/10;
   if (txalh <1) txalh = 1; else if (txalh > 3) txalh = 3;
   int txalv = fTextAlign%10;
   if (txalv <1) txalv = 1; else if (txalv > 3) txalv = 3;
   if (txalv == 3) {
      y -= 0.8*tsizey*TMath::Cos(kDEGRAD*fTextAngle);
      x += 0.8*tsizex*TMath::Sin(kDEGRAD*fTextAngle);
   } else if (txalv == 2) {
      y -= 0.4*tsizey*TMath::Cos(kDEGRAD*fTextAngle);
      x += 0.4*tsizex*TMath::Sin(kDEGRAD*fTextAngle);
   }
   UInt_t w = 0,  h = 0;

   text t;
   t.SeTextSize(fTextSize);
   t.SeTextFont(fTextFont);
   t.GeTextExtent(w, h, chars);
   double charsLength = gPad->AbsPixeltoX(w)-gPad->AbsPixeltoX(0);
   int psCharsLength = XtoPS(charsLength)-XtoPS(0);

   // text angle.
   int psangle = int(0.5 + fTextAngle);

   // Save context.
   printStr("@");
   SaveRestore(1);

   // Clipping
   int xc1 = XtoPS(gPad->GetX1());
   int xc2 = XtoPS(gPad->GetX2());
   int yc1 = YtoPS(gPad->GetY1());
   int yc2 = YtoPS(gPad->GetY2());
   writeInteger(xc2 - xc1);
   writeInteger(yc2 - yc1);
   writeInteger(xc1);
   writeInteger(yc1);
   printStr(" C");

   // Output text position and angle.
   writeInteger(XtoPS(x));
   writeInteger(YtoPS(y));
   printStr(TString::Format(" t %d r ", psangle));
   if(txalh == 2) printStr(TString::Format(" %d 0 t ", -psCharsLength/2));
   if(txalh == 3) printStr(TString::Format(" %d 0 t ", -psCharsLength));
   fMustEmbed[font-1] = kTRUE; // This font will be embedded in the file at EOF time.
   printStr(gEnv->GetValue(psfont[font-1][0], psfont[font-1][1]));
   printStr(TString::Format(" findfont %g sf 0 0 m ",fontsize));

   // Output text.
   if (len > 1) printStr(TString::Format("%d ", len));
   for(int i = 0; i < len; i++) {
      // Adobe Glyph Naming Convention
      // http://www.adobe.com/devnet/opentype/archives/glyph.hpptml
#include "AdobeGlyphList.hpp"
      const wchar_t *lower = std::lower_bound(
                                              adobe_glyph_ucs, adobe_glyph_ucs + nadobe_glyph,
                                              chars[i]);
      if(lower <  adobe_glyph_ucs + nadobe_glyph &&
         *lower == chars[i]) {
         // Named glyph from AGL 1.2
         const unsigned long index =
         lower - adobe_glyph_ucs;
         printStr(TString::Format("/%s ", adobe_glyph_name[index]));
      }
      else if((unsigned int)chars[i] < 0xffff) {
         // Unicode BMP
         printStr(TString::Format("/uni%04X ",
                       (unsigned int)chars[i]));
      }
      else {
         // Unicode supplemental planes
         printStr(TString::Format("/u%04X ",
                       (unsigned int)chars[i]));
      }
   }
   if(len > 1) {
      printStr("{glyphshow} repeat ");
   }
   else {
      printStr("glyphshow ");
   }

   printStr("NC");

   SaveRestore(-1);
}

////////////////////////////////////////////////////////////////////////////////
/// Draw text with URL. Same as text.
///

void TPostScript::textUrl(double x, double y, const char *chars, const char *)
{
   text(x, y, chars);
}

////////////////////////////////////////////////////////////////////////////////
/// write a string of characters in NDC

void TPostScript::textNDC(double u, double v, const char *chars)
{
   double x = gPad->GetX1() + u*(gPad->GetX2() - gPad->GetX1());
   double y = gPad->GetY1() + v*(gPad->GetY2() - gPad->GetY1());
   text(x, y, chars);
}

////////////////////////////////////////////////////////////////////////////////
/// write a string of characters in NDC

void TPostScript::textNDC(double u, double v, const wchar_t *chars)
{
   double x = gPad->GetX1() + u*(gPad->GetX2() - gPad->GetX1());
   double y = gPad->GetY1() + v*(gPad->GetY2() - gPad->GetY1());
   text(x, y, chars);
}

////////////////////////////////////////////////////////////////////////////////
/// Convert U from NDC coordinate to PostScript

int TPostScript::UtoPS(double u)
{
   double cm = _xPageSize*(gPad->GetAbsXlowNDC() + u*gPad->GetAbsWNDC());
   return int(0.5 + 288*cm/2.54);
}

////////////////////////////////////////////////////////////////////////////////
/// Convert V from NDC coordinate to PostScript

int TPostScript::VtoPS(double v)
{
   double cm = _yPageSize*(gPad->GetAbsYlowNDC() + v*gPad->GetAbsHNDC());
   return int(0.5 + 288*cm/2.54);
}

////////////////////////////////////////////////////////////////////////////////
/// Convert X from world coordinate to PostScript

int TPostScript::XtoPS(double x)
{
   double u = (x - gPad->GetX1())/(gPad->GetX2() - gPad->GetX1());
   return  UtoPS(u);
}

////////////////////////////////////////////////////////////////////////////////
/// Convert Y from world coordinate to PostScript

int TPostScript::YtoPS(double y)
{
   double v = (y - gPad->GetY1())/(gPad->GetY2() - gPad->GetY1());
   return  VtoPS(v);
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize the PostScript page in zones

void TPostScript::Zone()
{
   if( !fClear )return;
   fClear = kFALSE;

   // When Zone has been called, fZone is TRUE
   fZone = kTRUE;

   if( fIYzone > fNYzone) {
      fIYzone=1;
      if( fMode != 3) {
         printStr("@showpage");
         SaveRestore(-1);
         fNpages++;
         printStr("@%%Page:");
         writeInteger(fNpages);
         writeInteger(fNpages);
         printStr("@");
      } else {
         printFast(9," showpage");
         SaveRestore(-1);
      }
   }

   // No grestore the first time
   if( fMode != 3) {
      if( fIXzone != 1 || fIYzone != 1) SaveRestore(-1);
      SaveRestore(1);
      printStr("@");
      writeInteger(fIXzone);
      writeInteger(fIYzone);
      printFast(5," Zone");
      printStr("@");
      fIXzone++;
      if( fIXzone > fNXzone) { fIXzone=1; fIYzone++; }
   }

   // Picture Initialisation
   SaveRestore(1);
   if (fgLineJoin) {
      writeInteger(fgLineJoin);
      printFast(12," setlinejoin");
   }
   if (fgLineCap) {
      writeInteger(fgLineCap);
      printFast(11," setlinecap");
   }
   printFast(6," 0 0 t");
   _red     = -1;
   _green   = -1;
   _blue    = -1;
   fPrinted = kFALSE;
   fLineColor  = -1;
   fLineStyle  = -1;
   fLineWidth  = -1;
   fFillColor  = -1;
   fFillStyle  = -1;
   fMarkerSizeCur = -1;
}
