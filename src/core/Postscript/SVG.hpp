// @(#)root/postscript:$Id$
// Author: Olivier Couet

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef CAP__TSVG
#define CAP__TSVG


#include "VirtualPS.hpp"
#include "Rtypes.h"   // ClassDef: these classes derive from ROOT classes (TVirtualPS)

class Point2Ds;

class TSVG : public VirtualPS {

protected:
   float      _xPageSize = 0.;           ///< Page size along X
   float      _yPageSize = 0.;           ///< Page size along Y
   int        _type = 0;             ///< Workstation type used to know if the SVG is open
   bool       fCompact = kFALSE;     ///< True when the SVG header is printed
   bool       fBoundingBox = kFALSE; ///< True when the SVG header is printed
   bool       _rangeDefined = kFALSE;       ///< True when a range has been defined
   double     _yPageSizeSVG = 0.;        ///< Page's Y size in SVG units

   static int fgLineJoin;       ///< Appearance of joining lines
   static int fgLineCap;        ///< Appearance of line caps

   void printPath(bool convert, int n, double *xs, double *ys, bool close_path = kTRUE);
   void printLineStyleOnEndOfPath();
   void printLineJointAttributes();
   template<class T>
   void printPolyMarker(int n, T *x, T* y);

public:
   TSVG();
   TSVG(const char *filename, int type=-113, bool compact = kFALSE);
   ~TSVG() override;

   void  cellArrayBegin(int W, int H, double x1, double x2, double y1, double y2) override;
   void  cellArrayFill(int r, int g, int b) override;
   void  cellArrayPng(char *buffer, int size) override;
   void  cellArrayEnd() override;
   void  close(Option_t *opt="") override;
   double CMtoSVG(double u) { return 0.5 + 72*u/2.54; }
   void  drawBox(double x1, double y1,double x2, double  y2) override;
   void  drawFrame(double xl, double yl, double xt, double  yt,
                   int mode, int border, int dark, int light) override;
   void  DrawPolyLine(int, Point2Ds *);
   void  DrawPolyLineNDC(int, Point2Ds *);
   void  drawMarkers(int n, float *x, float *y) override;
   void  drawMarkers(int n, double *x, double *y) override;
   void  drawPS(int n, float *xw, float *yw) override;
   void  drawPS(int n, double *xw, double *yw) override;
   void  DrawSegments(int n, double *xw, double *yw) override;
   bool IsCompact() const { return fCompact; }
   void  Initialize();
   void  MovePS(double x, double y);
   void  newPage() override;
   void  Off();
   void  On();
   void  open(const char *filename, int type=-111) override;
   void  Range(float xrange, float yrange);
   void  setColorAlpha(int color = 1, bool fill = kTRUE, bool stroke = kTRUE);
   void  setColor(int color = 1);
   void  setColor(float r, float g, float b) override;
   void  SetFillColor(Color_t cindex=1) override;
   void  SetLineColor(Color_t cindex=1) override;
   void  SetLineJoin(int linejoin=0);
   void  SetLineCap(int linecap=0);
   void  SetLineStyle(Style_t linestyle = 1) override;
   void  SetLineWidth(Width_t linewidth = 1) override;
   void  SetLineScale(float =3) { }
   void  SetMarkerColor(Color_t cindex=1) override;
   void  SeTexColor(Color_t cindex=1) override;
   void  text(double x, double y, const char *string)  override;
   void  text(double, double, const wchar_t *) override {}
   void  textUrl(double x, double y, const char *string, const char *url) override;
   void  textNDC(double u, double v, const char *string);
   void  textNDC(double, double, const wchar_t *) {}
   void  writeReal(float r, bool space=kTRUE) override;

   double UtoSVG(double u);
   double VtoSVG(double v);
   double XtoSVG(double x);
   double YtoSVG(double y);

   ClassDefOverride(TSVG, 0);  //SVG driver
};

#endif
