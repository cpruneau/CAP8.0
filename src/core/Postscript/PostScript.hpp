// @(#)root/postscript:$Id$
// Author: O.Couet   16/07/99

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef CAP__TPostScript
#define CAP__TPostScript


#include "VirtualPS.hpp"
#include "ClassDef.hpp"

class Point2Ds;

class TPostScript : public VirtualPS {

protected:
   float fX1v = 0.;             ///< X bottom left corner of paper
   float fY1v = 0.;             ///< Y bottom left corner of paper
   float fX2v = 0.;             ///< X top right corner of paper
   float fY2v = 0.;             ///< Y top right corner of paper
   float fX1w = 0.;             ///<
   float fY1w = 0.;             ///<
   float fX2w = 0.;             ///<
   float fY2w = 0.;             ///<
   float fDXC = 0.;             ///<
   float fDYC = 0.;             ///<
   float fXC = 0.;              ///<
   float fYC = 0.;              ///<
   float fFX = 0.;              ///<
   float fFY = 0.;              ///<
   float fXVP1 = 0.;            ///<
   float fXVP2 = 0.;            ///<
   float fYVP1 = 0.;            ///<
   float fYVP2 = 0.;            ///<
   float fXVS1 = 0.;            ///<
   float fXVS2 = 0.;            ///<
   float fYVS1 = 0.;            ///<
   float fYVS2 = 0.;            ///<
   float _xPageSize = 0.;           ///< Page size along X
   float _yPageSize = 0.;           ///< Page size along Y
   float fMaxsize = 0.;         ///< Largest dimension of X and Y
   float _red = 0.;             ///< Per cent of red
   float _green = 0.;           ///< Per cent of green
   float _blue = 0.;            ///< Per cent of blue
   float fWidth = 0.;           ///< Current line width
   int   fStyle = 1;            ///< Current line style
   float _linearScale = 0.;       ///< Line width scale factor
   int   fSave = 0;             ///< Number of gsave for restore
   int   fNXzone = 0;           ///< Number of zones along X
   int   fNYzone = 0;           ///< Number of zones along Y
   int   fIXzone = 0;           ///< Current zone along X
   int   fIYzone = 0;           ///< Current zone along Y
   float fMarkerSizeCur = 0.;   ///< current transformed value of marker size
   int   fNpages = 0;           ///< number of pages
   int   _type = 0;             ///< PostScript workstation type
   int   fMode = 0;             ///< PostScript mode
   int   fClip = 0;             ///< Clipping mode
   bool  fBoundingBox = kFALSE; ///< True for Encapsulated PostScript
   bool  fClear = kFALSE;       ///< True when page must be cleared
   bool  fClipStatus = kFALSE;  ///< Clipping Indicator
   bool  _rangeDefined = kFALSE;       ///< True when a range has been defined
   bool  fZone = kFALSE;        ///< Zone indicator
   char    fPatterns[32];         ///< Indicate if pattern n is defined
   int   fNbinCT = 0;           ///< Number of entries in the current cell Array
   int   fNbCellW = 0;          ///< Number of boxes per line
   int   fNbCellLine = 0;       ///< Number of boxes in the current line
   int   fMaxLines = 0;         ///< Maximum number of lines in a PS array
   int   fLastCellRed = 0;      ///< Last red value
   int   fLastCellGreen = 0;    ///< Last green value
   int   fLastCellBlue = 0;     ///< Last blue value
   int   fNBSameColorCell = 0;  ///< Number of boxes with the same color
   TString fFileName;             ///< PS file name
   bool  fFontEmbed = kFALSE;   ///< True is FontEmbed has been called
   bool  fMustEmbed[29];        ///< flag to embed font

   static int fgLineJoin;       ///< Appearance of joining lines
   static int fgLineCap;        ///< Appearance of line caps

public:
   TPostScript();
   TPostScript(const char *filename, int type=-111);
   ~TPostScript() override;

   void  cellArrayBegin(int W, int H, double x1, double x2,
                                          double y1, double y2) override;
   void  cellArrayFill(int r, int g, int b) override;
   void  cellArrayEnd() override;
   void  close(Option_t *opt="") override;
   int CMtoPS(double u) {return int(0.5 + 72*u/2.54);}
   void  DefineMarkers();
   void  drawBox(double x1, double y1, double x2, double y2) override;
   void  drawFrame(double xl, double yl, double xt, double yt, int mode, int border, int dark,
                  int light) override;
   void  DrawHatch(float dy, float angle, int n, float *x, float *y);
   void  DrawHatch(float dy, float angle, int n, double *x, double *y);
   void  DrawPolyLine(int n, Point2Ds *xy);
   void  DrawPolyLineNDC(int n, Point2Ds *uv);
   void  drawMarkers(int n, float *x, float *y) override;
   void  drawMarkers(int n, double *x, double *y) override;
   void  drawPS(int n, float *xw, float *yw) override;
   void  drawPS(int n, double *xw, double *yw) override;
   bool  FontEmbedType1(const char *filename);
   bool  FontEmbedType2(const char *filename);
   bool  FontEmbedType42(const char *filename);
   void  FontEmbed();
   void  FontEncode();
   void  Initialize();
   void  newPage() override;
   void  Off();
   void  On();
   void  open(const char *filename, int type=-111) override;
   void  SaveRestore(int flag);
   void  SetFillColor(Color_t cindex=1) override;
   void  SetFillPatterns(int ipat, int color);
   void  SetLineColor(Color_t cindex=1) override;
   void  SetLineJoin(int linejoin=0);
   void  SetLineCap(int linecap=0);
   void  SetLineStyle(Style_t linestyle = 1) override;
   void  SetLineWidth(Width_t linewidth = 1) override;
   void  SetLineScale(float scale=3) {_linearScale = scale;}
   void  SetMarkerColor(Color_t cindex=1) override;
   void  SeTexColor(Color_t cindex=1) override;
   void  SetWidth(Width_t linewidth = 1);
   void  SetStyle(Style_t linestyle = 1);
   void  MovePS(int x, int y);
   void  Range(float xrange, float yrange);
   void  setColor(int color = 1);
   void  setColor(float r, float g, float b) override;
   void  text(double x, double y, const char *string) override;
   void  textUrl(double x, double y, const char *string, const char *url) override;
   void  text(double x, double y, const wchar_t *string) override;
   void  textNDC(double u, double v, const char *string);
   void  textNDC(double u, double v, const wchar_t *string);
   int UtoPS(double u);
   int VtoPS(double v);
   int XtoPS(double x);
   int YtoPS(double y);
   void  Zone();

   ClassDefOverride(TPostScript,1)  //PostScript driver
};

#endif
