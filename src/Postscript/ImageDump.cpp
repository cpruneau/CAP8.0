/**
 \class ImageDump
 \ingroup PS
 \brief Save canvas as an image (GIF, JPEG, PNG, XPM, TIFF etc.).

 Example:
 ~~~ {.cpp}
 $ root -b
 root [0] .x hsimple.C
 root [1] c1->print("c1.gif");
 ~~~
 ImageDump can be used in any mode (batch, interactive) as follows
 ~~~ {.cpp}
 TCanvas *c1;
 ImageDump *imgdump = new ImageDump("test.png");
 c1->Paint();
 imgdump->close();
 ~~~
 */

#include "ImageDump.hpp"
#include "Image.hpp"
#include "BasicMath.hpp"
#include "Point2D.hpp"
#include "Color.hpp"
#include "VirtualPad.hpp"
#include "VirtualX.hpp"
#include "Text.hpp"

namespace CAP
{

  ImageDump::ImageDump() : VirtualPS()
  {
  gVirtualPS = this;
  setTitle("IMG");
  setName("");
  setType(kImageUnknown);
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Initialize batch image interface
  ///
  /// fname : image file name
  ///
  /// The possible workstation types are:
  ///  - 111 - Portrait
  ///  - 112 - Landscape
  ///  - 114 - preview, keep in memory (do not write on delete)

  ImageDump::ImageDump(const String & filename, int type)
  : VirtualPS(filename, type)
  {
  open(filename, type);
  gVirtualPS = this;
  setTitle("IMG");
  setName(filename);
  setName(type);
  }

  void ImageDump::open(const String & filename)
  {
  _image  = Image::Create();
  }

  ImageDump::~ImageDump()
  {
  close();
  delete _image;
  _image = nullptr;
  gVirtualPS = nullptr;
  }

  void ImageDump::close()
  {
  if (!_image || (_type == kImagePreview)) return;
  _image->writeImage(name());
  }


  void ImageDump::drawBox(const Point2D & point1, const Point2D & point2)
  {
  if (!gPad || !_image) return;
  _image->beginPaint();

  double x[4], y[4];
  int ix1 = x1 < x2 ? XtoPixel(x1) : XtoPixel(x2);
  int ix2 = x1 < x2 ? XtoPixel(x2) : XtoPixel(x1);
  int iy1 = y1 < y2 ? YtoPixel(y1) : YtoPixel(y2);
  int iy2 = y1 < y2 ? YtoPixel(y2) : YtoPixel(y1);

  if (ix1<0 || ix2 <0 || iy1 < 0 || iy2 <0) return; // box is not visible

  if (Math::absolute(ix2-ix1) < 1) ix2 = ix1+1;
  if (Math::absolute(iy1-iy2) < 1) iy1 = iy2+1;

  int fillis = fFillStyle/1000;
  int fillsi = fFillStyle%1000;
  Color * fillColor = _fillAttributes->color();
  Color * lineColor = _lineAttributes->color();
  if ((fillis == 3) || (fillis == 2))
    {
    if (fillsi > 99) {
      x[0] = x1;   y[0] = y1;
      x[1] = x2;   y[1] = y1;
      x[2] = x2;   y[2] = y2;
      x[3] = x1;   y[3] = y2;
      return;
    }
    if ((fillsi > 0) && (fillsi < 26)) {
      x[0] = x1;   y[0] = y1;
      x[1] = x2;   y[1] = y1;
      x[2] = x2;   y[2] = y2;
      x[3] = x1;   y[3] = y2;
      drawPS(-4, &x[0], &y[0]);
    }
    }
  if (fillis == 1) _image->drawBox(ix1, iy1, ix2, iy2, fillColor->asHexString(), 1, VirtualX::kFilled);
  if (fillis == 0)
    {
  if (_lineAttributes->width()<=0) return;
    _image->drawBox(ix1, iy1, ix2, iy2, lineColor->asHexString(), _lineAttributes->width(), VirtualX::kHollow);
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// draw a Frame around a box
  ///
  ///  - mode = -1  the box looks as it is behind the screen
  ///  - mode =  1  the box looks as it is in front of the screen
  /// border is the border size in already pre-computed dark is the
  /// color for the dark part of the frame light is the color for the light
  /// part of the frame

  void ImageDump::drawFrame(const Point2D & point1, const Point2D & point2,
                            int mode, int bordersize, int dark, int light)
  {
  if (!gPad || !_image) return;
  _image->BeginPaint();
  bordersize = bordersize < 1 ? 1 : bordersize;
  Color *col;
  Color *lo = gROOT->color(dark);
  if (!lo) {
    lo = gROOT->color(10);
  }
  Color *hi = gROOT->color(light);
  if (!hi) {
    hi = gROOT->color(10);
  }

  Short_t pxl,pyl,pxt,pyt,px1,py1,px2,py2;

  px1 = XtoPixel(x1);   py1 = YtoPixel(y1);
  px2 = XtoPixel(x2);   py2 = YtoPixel(y2);
  if (px1 < px2) {pxl = px1; pxt = px2;}
  else           {pxl = px2; pxt = px1;}
  if (py1 > py2) {pyl = py1; pyt = py2;}
  else           {pyl = py2; pyt = py1;}

  if (bordersize == 1) {
    col = gROOT->color(fLineColor);
    if (!col) {
      fLineColor = 1;
      col = gROOT->color(fLineColor);
      if (!col) return;
    }
    _image->drawBox(pxl, pyl, pxt, pyt-1, col->asHexString(),  VirtualX::kFilled);
    return;
  }

  if (!_image->IsValid()) {
    col = gROOT->color(light);
    if (!col) {
      col = gROOT->color(10);
      if (!col) return;
    }
    _image->drawBox(pxl, pyl, pxt, pyt, // force image creation and resizing
                    "#ffffffff", 1,  VirtualX::kFilled);
  }

  Point2D frame[6];

  frame[0].fX = pxl;                 frame[0].fY = pyl;
  frame[1].fX = pxl + bordersize;    frame[1].fY = pyl - bordersize;
  frame[2].fX = pxl + bordersize;    frame[2].fY = pyt + bordersize;
  frame[3].fX = pxt - bordersize;    frame[3].fY = pyt + bordersize;
  frame[4].fX = pxt;                 frame[4].fY = pyt;
  frame[5].fX = pxl;                 frame[5].fY = pyt;

  if (mode == -1) col = lo;
  else            col = hi;

  _image->drawFillArea(6, frame, col->asHexString());
  frame[0].fX = pxl;                 frame[0].fY = pyl;
  frame[1].fX = pxl + bordersize;    frame[1].fY = pyl - bordersize;
  frame[2].fX = pxt - bordersize;    frame[2].fY = frame[1].fY;
  frame[3].fX = frame[2].fX;         frame[3].fY = pyt + bordersize;
  frame[4].fX = pxt;                 frame[4].fY = pyt;
  frame[5].fX = pxt;                 frame[5].fY = pyl;

  if (mode == -1) col = hi;
  else            col = lo;

  _image->drawFillArea(6, frame, col->asHexString());
  }

  void ImageDump::drawMarkers(int n, double *xw, double *yw)
  {
  if (!gPad || !_image) return;
  _image->BeginPaint();

  fMarkerStyle = TMath::absolute(fMarkerStyle);
  int ms = TAttMarker::GetMarkerStyleBase(fMarkerStyle);
  Point2D pt[20];

  if (ms == 4)
    ms = 24;
  else if (ms == 8)
    ms = 20;
  else if (ms >= 9 && ms <= 19)
    ms = 1;

  // Define the marker size
  const int kBASEMARKER = 8;
  double msize = (fMarkerSize - TMath::Floor(TAttMarker::GetMarkerLineWidth(fMarkerStyle)/2.)/4.) * kBASEMARKER * gStyle->GeImageScaling();
  if (ms == 6) msize *= 0.2;
  if (ms == 7) msize *= 0.3;
  double m  = msize;
  double m2 = m/2;
  double m3 = m/3;
  double m6 = m/6;
  double m4 = m/4;
  double m8 = m/8;
  double m0 = m*0.1;

  Color *col = gROOT->color(fMarkerColor);
  if (!col) { // no color
    fMarkerColor = 1;
    col = gROOT->color(fMarkerColor);
    if (!col) return;
  }
  if (col->GetAlpha()<1.) {
    if (ms==8)  ms = 108;
    if (ms==20) ms = 120;
  }

  Width_t mlinewidth = TAttMarker::GetMarkerLineWidth(fMarkerStyle);

  // draw the marker according to the type
  Short_t ix,iy;
  for (int i=0;i<n;i++) {
    ix = XtoPixel(xw[i]);
    iy = YtoPixel(yw[i]);

    switch (ms) {
        // Dots (.) big, medium and small
      case 7:
        _image->PutPixel((UInt_t)ix-1, (UInt_t)iy-1, col->asHexString());
        _image->PutPixel((UInt_t)ix-1, (UInt_t)iy+1, col->asHexString());
        _image->PutPixel((UInt_t)ix+1, (UInt_t)iy+1, col->asHexString());
        _image->PutPixel((UInt_t)ix+1, (UInt_t)iy-1, col->asHexString());
      case 6:
        _image->PutPixel((UInt_t)ix,   (UInt_t)iy-1, col->asHexString());
        _image->PutPixel((UInt_t)ix,   (UInt_t)iy+1, col->asHexString());
        _image->PutPixel((UInt_t)ix-1, (UInt_t)iy,   col->asHexString());
        _image->PutPixel((UInt_t)ix+1, (UInt_t)iy,   col->asHexString());
      case 1:
        _image->PutPixel((UInt_t)ix,   (UInt_t)iy,   col->asHexString());
        break;
        // Plus (+)
      case 2:
        _image->drawLine(UInt_t(ix-m2), UInt_t(iy), UInt_t(ix+m2), UInt_t(iy), col->asHexString(), mlinewidth);
        _image->drawLine(UInt_t(ix), UInt_t(iy-m2), UInt_t(ix), UInt_t(iy+m2), col->asHexString(), mlinewidth);
        break;
        // X shape (X)
      case 5:
        _image->drawLine(UInt_t(ix-m2*0.707), UInt_t(iy-m2*0.707), UInt_t(ix+m2*0.707), UInt_t(iy+m2*0.707), col->asHexString(), mlinewidth);
        _image->drawLine(UInt_t(ix-m2*0.707), UInt_t(iy+m2*0.707), UInt_t(ix+m2*0.707), UInt_t(iy-m2*0.707), col->asHexString(), mlinewidth);
        break;
        // Asterisk shape (*)
      case 3:
      case 31:
        _image->drawLine(UInt_t(ix-m2), UInt_t(iy), UInt_t(ix+m2), UInt_t(iy), col->asHexString(), mlinewidth);
        _image->drawLine(UInt_t(ix), UInt_t(iy-m2), UInt_t(ix), UInt_t(iy+m2), col->asHexString(), mlinewidth);
        _image->drawLine(UInt_t(ix-m2*0.707), UInt_t(iy-m2*0.707), UInt_t(ix+m2*0.707), UInt_t(iy+m2*0.707), col->asHexString(), mlinewidth);
        _image->drawLine(UInt_t(ix-m2*0.707), UInt_t(iy+m2*0.707), UInt_t(ix+m2*0.707), UInt_t(iy-m2*0.707), col->asHexString(), mlinewidth);
        break;
        // Circle
      case 4:
      case 24:
        _image->drawCircle(ix, iy, int(msize/2), col->asHexString(), mlinewidth);
        break;
        // Circle
      case 8:
      case 20:
        _image->drawCircle(ix, iy, int(msize/2), col->asHexString(), -1);
        break;
      case 108:
      case 120:
        for (int idx=int(msize/2); idx>0; idx--) _image->drawCircle(ix, iy, idx, col->asHexString(), 1);
        _image->PutPixel((UInt_t)ix, (UInt_t)iy, col->asHexString());
        break;
        // Square
      case 21:
        _image->FillRectangle(col->asHexString(), UInt_t(ix-m2), UInt_t(iy-m2), UInt_t(m), UInt_t(m));
        break;
      case 25:
        _image->drawRectangle(UInt_t(ix-m2), UInt_t(iy-m2), UInt_t(m), UInt_t(m), col->asHexString(), mlinewidth);
        break;
        // Down triangle
      case 23:
      case 32:
        pt[0].fX = Short_t(ix-m2); pt[0].fY = Short_t(iy-m2);
        pt[1].fX = Short_t(ix+m2); pt[1].fY = Short_t(iy-m2);
        pt[2].fX = Short_t(ix);    pt[2].fY = Short_t(iy+m2);
        pt[3].fX = Short_t(ix-m2); pt[3].fY = Short_t(iy-m2);
        ms == 32 ? _image->drawPolyLine(4, pt, col->asHexString(), mlinewidth) :
        _image->FillPolygon(3, pt, col->asHexString());
        break;
        // Up triangle
      case 22:
      case 26:
        pt[0].fX = Short_t(ix);    pt[0].fY = Short_t(iy-m2);
        pt[1].fX = Short_t(ix+m2); pt[1].fY = Short_t(iy+m2);
        pt[2].fX = Short_t(ix-m2); pt[2].fY = Short_t(iy+m2);
        pt[3].fX = Short_t(ix);    pt[3].fY = Short_t(iy-m2);
        ms == 26 ? _image->drawPolyLine(4, pt, col->asHexString(), mlinewidth) :
        _image->FillPolygon(3, pt, col->asHexString());
        break;
      case 27:
      case 33:
        pt[0].fX = Short_t(ix);    pt[0].fY = Short_t(iy-m2);
        pt[1].fX = Short_t(ix+m3); pt[1].fY = Short_t(iy);
        pt[2].fX = Short_t(ix);    pt[2].fY = Short_t(iy+m2);
        pt[3].fX = Short_t(ix-m3); pt[3].fY = Short_t(iy);
        pt[4].fX = Short_t(ix);    pt[4].fY = Short_t(iy-m2);
        ms == 27 ? _image->drawPolyLine(5, pt, col->asHexString(), mlinewidth) :
        _image->FillPolygon(4, pt, col->asHexString());
        break;
      case 28:
      case 34:
        pt[0].fX = Short_t(ix-m6);  pt[0].fY = Short_t(iy-m6);
        pt[1].fX = Short_t(ix-m6);  pt[1].fY = Short_t(iy-m2);
        pt[2].fX = Short_t(ix+m6);  pt[2].fY = Short_t(iy-m2);
        pt[3].fX = Short_t(ix+m6);  pt[3].fY = Short_t(iy-m6);
        pt[4].fX = Short_t(ix+m2);  pt[4].fY = Short_t(iy-m6);
        pt[5].fX = Short_t(ix+m2);  pt[5].fY = Short_t(iy+m6);
        pt[6].fX = Short_t(ix+m6);  pt[6].fY = Short_t(iy+m6);
        pt[7].fX = Short_t(ix+m6);  pt[7].fY = Short_t(iy+m2);
        pt[8].fX = Short_t(ix-m6);  pt[8].fY = Short_t(iy+m2);
        pt[9].fX = Short_t(ix-m6);  pt[9].fY = Short_t(iy+m6);
        pt[10].fX = Short_t(ix-m2); pt[10].fY = Short_t(iy+m6);
        pt[11].fX = Short_t(ix-m2); pt[11].fY = Short_t(iy-m6);
        pt[12].fX = Short_t(ix-m6); pt[12].fY = Short_t(iy-m6);
        ms == 28 ? _image->drawPolyLine(13, pt, col->asHexString(), mlinewidth) :
        _image->FillPolygon(12, pt, col->asHexString());
        break;
      case 29:
      case 30:
        pt[0].fX = Short_t(ix);             pt[0].fY = Short_t(iy+m2);
        pt[1].fX = Short_t(ix+0.112255*m);  pt[1].fY = Short_t(iy+0.15451*m);
        pt[2].fX = Short_t(ix+0.47552*m);   pt[2].fY = Short_t(iy+0.15451*m);
        pt[3].fX = Short_t(ix+0.181635*m);  pt[3].fY = Short_t(iy-0.05902*m);
        pt[4].fX = Short_t(ix+0.29389*m);   pt[4].fY = Short_t(iy-0.40451*m);
        pt[5].fX = Short_t(ix);             pt[5].fY = Short_t(iy-0.19098*m);
        pt[6].fX = Short_t(ix-0.29389*m);   pt[6].fY = Short_t(iy-0.40451*m);
        pt[7].fX = Short_t(ix-0.181635*m);  pt[7].fY = Short_t(iy-0.05902*m);
        pt[8].fX = Short_t(ix-0.47552*m);   pt[8].fY = Short_t(iy+0.15451*m);
        pt[9].fX = Short_t(ix-0.112255*m);  pt[9].fY = Short_t(iy+0.15451*m);
        pt[10].fX = Short_t(ix);             pt[10].fY = Short_t(iy+m2);
        ms == 30 ? _image->drawPolyLine(11, pt, col->asHexString(), mlinewidth) :
        _image->drawFillArea(10, pt, col->asHexString());
        break;
      case 35:
        pt[0].fX = Short_t(ix-m2);  pt[0].fY = Short_t(iy   );
        pt[1].fX = Short_t(ix   );  pt[1].fY = Short_t(iy-m2);
        pt[2].fX = Short_t(ix+m2);  pt[2].fY = Short_t(iy   );
        pt[3].fX = Short_t(ix   );  pt[3].fY = Short_t(iy+m2);
        pt[4].fX = Short_t(ix-m2);  pt[4].fY = Short_t(iy   );
        pt[5].fX = Short_t(ix+m2);  pt[5].fY = Short_t(iy   );
        pt[6].fX = Short_t(ix   );  pt[6].fY = Short_t(iy+m2);
        pt[7].fX = Short_t(ix   );  pt[7].fY = Short_t(iy-m2);
        _image->drawPolyLine(8, pt, col->asHexString(), mlinewidth) ;
        break;
      case 36:
        pt[0].fX = Short_t(ix-m2);  pt[0].fY = Short_t(iy-m2);
        pt[1].fX = Short_t(ix+m2);  pt[1].fY = Short_t(iy-m2);
        pt[2].fX = Short_t(ix+m2);  pt[2].fY = Short_t(iy+m2);
        pt[3].fX = Short_t(ix-m2);  pt[3].fY = Short_t(iy+m2);
        pt[4].fX = Short_t(ix-m2);  pt[4].fY = Short_t(iy-m2);
        pt[5].fX = Short_t(ix+m2);  pt[5].fY = Short_t(iy+m2);
        pt[6].fX = Short_t(ix-m2);  pt[6].fY = Short_t(iy+m2);
        pt[7].fX = Short_t(ix+m2);  pt[7].fY = Short_t(iy-m2);
        _image->drawPolyLine(8, pt, col->asHexString(), mlinewidth) ;
        break;
      case 37:
      case 39:
        pt[0].fX = Short_t(ix   );  pt[0].fY = Short_t(iy   );
        pt[1].fX = Short_t(ix-m4);  pt[1].fY = Short_t(iy-m2);
        pt[2].fX = Short_t(ix-m2);  pt[2].fY = Short_t(iy   );
        pt[3].fX = Short_t(ix+m2);  pt[3].fY = Short_t(iy   );
        pt[4].fX = Short_t(ix+m4);  pt[4].fY = Short_t(iy-m2);
        pt[5].fX = Short_t(ix-m4);  pt[5].fY = Short_t(iy+m2);
        pt[6].fX = Short_t(ix+m4);  pt[6].fY = Short_t(iy+m2);
        pt[7].fX = Short_t(ix   );  pt[7].fY = Short_t(iy   );
        ms == 37 ? _image->drawPolyLine(8, pt, col->asHexString(), mlinewidth) :
        _image->drawFillArea(7, pt, col->asHexString());
        break;
      case 38:
        pt[0].fX = Short_t(ix-m2);  pt[0].fY = Short_t(iy   );
        pt[1].fX = Short_t(ix-m2);  pt[1].fY = Short_t(iy-m4);
        pt[2].fX = Short_t(ix-m4);  pt[2].fY = Short_t(iy-m2);
        pt[3].fX = Short_t(ix+m4);  pt[3].fY = Short_t(iy-m2);
        pt[4].fX = Short_t(ix+m2);  pt[4].fY = Short_t(iy-m4);
        pt[5].fX = Short_t(ix+m2);  pt[5].fY = Short_t(iy+m4);
        pt[6].fX = Short_t(ix+m4);  pt[6].fY = Short_t(iy+m2);
        pt[7].fX = Short_t(ix-m4);  pt[7].fY = Short_t(iy+m2);
        pt[8].fX = Short_t(ix-m2);  pt[8].fY = Short_t(iy+m4);
        pt[9].fX = Short_t(ix-m2);  pt[9].fY = Short_t(iy   );
        pt[10].fX = Short_t(ix+m2);  pt[10].fY = Short_t(iy   );
        pt[11].fX = Short_t(ix   );  pt[11].fY = Short_t(iy   );
        pt[12].fX = Short_t(ix   );  pt[12].fY = Short_t(iy-m2);
        pt[13].fX = Short_t(ix   );  pt[13].fY = Short_t(iy+m2);
        pt[14].fX = Short_t(ix   );  pt[14].fY = Short_t(iy   );
        _image->drawPolyLine(15, pt, col->asHexString(), mlinewidth) ;
        break;
      case 40:
      case 41:
        pt[0].fX = Short_t(ix   );  pt[0].fY = Short_t(iy   );
        pt[1].fX = Short_t(ix+m4);  pt[1].fY = Short_t(iy+m2);
        pt[2].fX = Short_t(ix+m2);  pt[2].fY = Short_t(iy+m4);
        pt[3].fX = Short_t(ix   );  pt[3].fY = Short_t(iy   );
        pt[4].fX = Short_t(ix+m2);  pt[4].fY = Short_t(iy-m4);
        pt[5].fX = Short_t(ix+m4);  pt[5].fY = Short_t(iy-m2);
        pt[6].fX = Short_t(ix   );  pt[6].fY = Short_t(iy   );
        pt[7].fX = Short_t(ix-m4);  pt[7].fY = Short_t(iy-m2);
        pt[8].fX = Short_t(ix-m2);  pt[8].fY = Short_t(iy-m4);
        pt[9].fX = Short_t(ix   );  pt[9].fY = Short_t(iy   );
        pt[10].fX = Short_t(ix-m2);  pt[10].fY = Short_t(iy+m4);
        pt[11].fX = Short_t(ix-m4);  pt[11].fY = Short_t(iy+m2);
        pt[12].fX = Short_t(ix   );  pt[12].fY = Short_t(iy   );
        ms == 40 ? _image->drawPolyLine(13, pt, col->asHexString(), mlinewidth) :
        _image->drawFillArea(12, pt, col->asHexString());
        break;
      case 42:
      case 43:
        pt[0].fX = Short_t(ix   );  pt[0].fY = Short_t(iy+m2);
        pt[1].fX = Short_t(ix-m8);  pt[1].fY = Short_t(iy+m8);
        pt[2].fX = Short_t(ix-m2);  pt[2].fY = Short_t(iy   );
        pt[3].fX = Short_t(ix-m8);  pt[3].fY = Short_t(iy-m8);
        pt[4].fX = Short_t(ix   );  pt[4].fY = Short_t(iy-m2);
        pt[5].fX = Short_t(ix+m8);  pt[5].fY = Short_t(iy-m8);
        pt[6].fX = Short_t(ix+m2);  pt[6].fY = Short_t(iy   );
        pt[7].fX = Short_t(ix+m8);  pt[7].fY = Short_t(iy+m8);
        pt[8].fX = Short_t(ix   );  pt[8].fY = Short_t(iy+m2);
        ms == 42 ? _image->drawPolyLine(9, pt, col->asHexString(), mlinewidth) :
        _image->drawFillArea(8, pt, col->asHexString());
        break;
      case 44:
        pt[0].fX = Short_t(ix   );  pt[0].fY = Short_t(iy   );
        pt[1].fX = Short_t(ix+m4);  pt[1].fY = Short_t(iy+m2);
        pt[2].fX = Short_t(ix-m4);  pt[2].fY = Short_t(iy+m2);
        pt[3].fX = Short_t(ix+m4);  pt[3].fY = Short_t(iy-m2);
        pt[4].fX = Short_t(ix-m4);  pt[4].fY = Short_t(iy-m2);
        pt[5].fX = Short_t(ix   );  pt[5].fY = Short_t(iy   );
        pt[6].fX = Short_t(ix+m2);  pt[6].fY = Short_t(iy+m4);
        pt[7].fX = Short_t(ix+m2);  pt[7].fY = Short_t(iy-m4);
        pt[8].fX = Short_t(ix-m2);  pt[8].fY = Short_t(iy+m4);
        pt[9].fX = Short_t(ix-m2);  pt[9].fY = Short_t(iy-m4);
        pt[10].fX = Short_t(ix   );  pt[10].fY = Short_t(iy   );
        _image->drawPolyLine(11, pt, col->asHexString(), mlinewidth) ;
        break;
      case 45:
        pt[0].fX = Short_t(ix+m0);  pt[0].fY = Short_t(iy+m0);
        pt[1].fX = Short_t(ix+m4);  pt[1].fY = Short_t(iy+m2);
        pt[2].fX = Short_t(ix-m4);  pt[2].fY = Short_t(iy+m2);
        pt[3].fX = Short_t(ix-m0);  pt[3].fY = Short_t(iy+m0);
        pt[4].fX = Short_t(ix-m2);  pt[4].fY = Short_t(iy+m4);
        pt[5].fX = Short_t(ix-m2);  pt[5].fY = Short_t(iy-m4);
        pt[6].fX = Short_t(ix-m0);  pt[6].fY = Short_t(iy-m0);
        pt[7].fX = Short_t(ix-m4);  pt[7].fY = Short_t(iy-m2);
        pt[8].fX = Short_t(ix+m4);  pt[8].fY = Short_t(iy-m2);
        pt[9].fX = Short_t(ix+m0);  pt[9].fY = Short_t(iy-m0);
        pt[10].fX = Short_t(ix+m2);  pt[10].fY = Short_t(iy-m4);
        pt[11].fX = Short_t(ix+m2);  pt[11].fY = Short_t(iy+m4);
        pt[12].fX = Short_t(ix+m0);  pt[12].fY = Short_t(iy+m0);
        _image->drawFillArea(13, pt, col->asHexString());
        break;
      case 46:
      case 47:
        pt[0].fX = Short_t(ix   );  pt[0].fY = Short_t(iy+m4);
        pt[1].fX = Short_t(ix-m4);  pt[1].fY = Short_t(iy+m2);
        pt[2].fX = Short_t(ix-m2);  pt[2].fY = Short_t(iy+m4);
        pt[3].fX = Short_t(ix-m4);  pt[3].fY = Short_t(iy   );
        pt[4].fX = Short_t(ix-m2);  pt[4].fY = Short_t(iy-m4);
        pt[5].fX = Short_t(ix-m4);  pt[5].fY = Short_t(iy-m2);
        pt[6].fX = Short_t(ix   );  pt[6].fY = Short_t(iy-m4);
        pt[7].fX = Short_t(ix+m4);  pt[7].fY = Short_t(iy-m2);
        pt[8].fX = Short_t(ix+m2);  pt[8].fY = Short_t(iy-m4);
        pt[9].fX = Short_t(ix+m4);  pt[9].fY = Short_t(iy   );
        pt[10].fX = Short_t(ix+m2);  pt[10].fY = Short_t(iy+m4);
        pt[11].fX = Short_t(ix+m4);  pt[11].fY = Short_t(iy+m2);
        pt[12].fX = Short_t(ix   );  pt[12].fY = Short_t(iy+m4);
        ms == 46 ? _image->drawPolyLine(13, pt, col->asHexString(), mlinewidth) :
        _image->drawFillArea(12, pt, col->asHexString());
        break;
      case 48:
        pt[0].fX = Short_t(ix   );  pt[0].fY = Short_t(iy+m4*1.005);
        pt[1].fX = Short_t(ix-m4);  pt[1].fY = Short_t(iy+m2);
        pt[2].fX = Short_t(ix-m2);  pt[2].fY = Short_t(iy+m4);
        pt[3].fX = Short_t(ix-m4);  pt[3].fY = Short_t(iy   );
        pt[4].fX = Short_t(ix-m2);  pt[4].fY = Short_t(iy-m4);
        pt[5].fX = Short_t(ix-m4);  pt[5].fY = Short_t(iy-m2);
        pt[6].fX = Short_t(ix   );  pt[6].fY = Short_t(iy-m4);
        pt[7].fX = Short_t(ix+m4);  pt[7].fY = Short_t(iy-m2);
        pt[8].fX = Short_t(ix+m2);  pt[8].fY = Short_t(iy-m4);
        pt[9].fX = Short_t(ix+m4);  pt[9].fY = Short_t(iy   );
        pt[10].fX = Short_t(ix+m2);  pt[10].fY = Short_t(iy+m4);
        pt[11].fX = Short_t(ix+m4);  pt[11].fY = Short_t(iy+m2);
        pt[12].fX = Short_t(ix   );  pt[12].fY = Short_t(iy+m4*0.995);
        pt[13].fX = Short_t(ix+m4*0.995);  pt[13].fY = Short_t(iy   );
        pt[14].fX = Short_t(ix   );  pt[14].fY = Short_t(iy-m4*0.995);
        pt[15].fX = Short_t(ix-m4*0.995);  pt[15].fY = Short_t(iy   );
        pt[16].fX = Short_t(ix   );  pt[16].fY = Short_t(iy+m4*0.995);
        _image->drawFillArea(17, pt, col->asHexString());
        break;
      case 49:
        pt[0].fX = Short_t(ix-m6);  pt[0].fY = Short_t(iy-m6*1.005);
        pt[1].fX = Short_t(ix-m6);  pt[1].fY = Short_t(iy-m2);
        pt[2].fX = Short_t(ix+m6);  pt[2].fY = Short_t(iy-m2);
        pt[3].fX = Short_t(ix+m6);  pt[3].fY = Short_t(iy-m6);
        pt[4].fX = Short_t(ix+m2);  pt[4].fY = Short_t(iy-m6);
        pt[5].fX = Short_t(ix+m2);  pt[5].fY = Short_t(iy+m6);
        pt[6].fX = Short_t(ix+m6);  pt[6].fY = Short_t(iy+m6);
        pt[7].fX = Short_t(ix+m6);  pt[7].fY = Short_t(iy+m2);
        pt[8].fX = Short_t(ix-m6);  pt[8].fY = Short_t(iy+m2);
        pt[9].fX = Short_t(ix-m6);  pt[9].fY = Short_t(iy+m6);
        pt[10].fX = Short_t(ix-m2);  pt[10].fY = Short_t(iy+m6);
        pt[11].fX = Short_t(ix-m2);  pt[11].fY = Short_t(iy-m6);
        pt[12].fX = Short_t(ix-m6);  pt[12].fY = Short_t(iy-m6*0.995);
        pt[13].fX = Short_t(ix-m6);  pt[13].fY = Short_t(iy+m6);
        pt[14].fX = Short_t(ix+m6);  pt[14].fY = Short_t(iy+m6);
        pt[15].fX = Short_t(ix+m6);  pt[15].fY = Short_t(iy-m6);
        pt[16].fX = Short_t(ix-m6);  pt[16].fY = Short_t(iy-m6*1.005);
        _image->drawFillArea(17, pt, col->asHexString());
        break;
      default:
        _image->PutPixel(UInt_t(ix), UInt_t(iy), col->asHexString());
        break;
    }
  }
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// This function defines a path with xw and yw and draw it according the
  /// value of nn:
  ///
  ///  - If nn > 0 a line is drawn.
  ///  - If nn < 0 a closed polygon is drawn.

  void ImageDump::drawPS(int nn, double *x, double *y)
  {
  if (!gPad || !_image || !nn) return;
  _image->BeginPaint();

  bool line = nn > 1;
  UInt_t n = TMath::absolute(nn);
  int fais = fFillStyle / 1000;
  int fasi = fFillStyle % 1000;

  // SetLineStyle
  std::vector<char> dashList;

  Color *fcol = gROOT->color(fFillColor);
  if (!fcol) { // no color, set it white
    fFillColor = 10;
    fcol = gROOT->color(fFillColor);
  }

  Color *lcol = gROOT->color(fLineColor);
  if (!lcol) { // no color, make it black
    fLineColor = 1;
    lcol = gROOT->color(fLineColor);
  }

  if (line) {
    if (fLineWidth <= 0)
      return;
    // dash lines
    if (fLineStyle > 1) {
      TString st = gStyle->GetLineStyleString(fLineStyle);
      std::unique_ptr<TObjArray> tokens(st.Tokenize(" "));

      if (tokens)
        for (int j = 0; j < tokens->GetEntries(); j++) {
          int it;
          sscanf(tokens->At(j)->GetName(), "%d", &it);
          dashList.emplace_back((char)(it/4));
        }
    }
  }

  if (n == 1) {  // point
    auto px1 = XtoPixel(x[0]);
    auto py1 = YtoPixel(y[0]);
    if (fcol)
      _image->PutPixel(px1, py1, fcol->asHexString());
    return;
  }

  if (n == 2) {  // line
    auto px1 = XtoPixel(x[0]);
    auto py1 = YtoPixel(y[0]);
    auto px2 = XtoPixel(x[1]);
    auto py2 = YtoPixel(y[1]);

    // SetLineColor
    if (lcol) {
      if (fLineStyle < 2) {
        _image->drawLine(px1, py1, px2, py2, lcol->asHexString(), fLineWidth);
      } else {
        _image->drawDashLine(px1, py1, px2, py2, dashList.size(), dashList.data(),
                             lcol->asHexString(), fLineWidth);
      }
    }
    return;
  }

  if (!line && ((fais == 3) || (fais == 2)) && (fasi > 100))
    return;

  std::vector<Point2D> pt(n+1);

  for (UInt_t i = 0; i < n; i++) {
    pt[i].fX = XtoPixel(x[i]);
    pt[i].fY = YtoPixel(y[i]);
  }
  pt[n].fX = pt[0].fX;
  pt[n].fY = pt[0].fY;

  const char *stipple = (fais == 3) && (fasi > 0) && (fasi < 26) ? (const char*)gStipples[fasi] : nullptr;

  // filled polygon
  if (!line && fFillStyle && (fFillStyle != 4000) && fcol) {
    if (n < 5)    // convex
      _image->FillPolygon(n, pt.data(), fcol->asHexString(), stipple);
    else        // non-convex fill area
      _image->drawFillArea(n, pt.data(), fcol->asHexString(), stipple);
  }

  // hollow polygon or polyline is drawn
  if (line || !fFillStyle || (fFillStyle == 4000)) {
    if (!line) {
      if (fcol)
        _image->drawPolyLine(n+1, pt.data(), fcol->asHexString(), 1);
    } else if (lcol) {
      if (fLineStyle < 2) { // solid
        _image->drawPolyLine(n, pt.data(), lcol->asHexString(), fLineWidth);
      } else { // dashed
        drawDashPolyLine(n, pt.data(), dashList.size(), dashList.data(),
                         lcol->asHexString(), fLineWidth);
      }
    }
  }
  }

  void ImageDump::drawDashPolyLine(int nn, Point2D *xy, UInt_t nDash,
                                   const char* pDash, const char* col, UInt_t thick)
  {
  int x0 = xy[0].GetX();
  int y0 = xy[0].GetY();

  for (int i = 1; i < nn; i++)
    {
    int x = xy[i].GetX();
    int y = xy[i].GetY();
    _image->drawDashLine(x0, y0, x, y, nDash, pDash, col, thick);
    x0 = x;
    y0 = y;
    }
  }

  void ImageDump::newPage()
  {
  fX0 = fY0 = 0;
  if (gPad && _image)
    {
    UInt_t w = gPad->GetWw() * gStyle->GeImageScaling();
    UInt_t h = gPad->GetWh() * gStyle->GeImageScaling();
    if (gPad != gPad->GetMother()) {
      fX0 = gPad->XtoabsolutePixel(gPad->GetX1())* gStyle->GeImageScaling();
      fY0 = gPad->YtoabsolutePixel(gPad->GetY2())* gStyle->GeImageScaling();
      w = w * gPad->GetabsoluteWNDC();
      h = h * gPad->GetabsoluteHNDC();
    }
    _image->drawRectangle(0, 0, w, h, "#ffffffff");
  }
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// draw text
  ///
  ///  - x: x position of the text
  ///  - y: y position of the text

  void ImageDump::text(double x, double y, const char *chars)
  {
  if (!gPad || !_image)
    return;

  _image->BeginPaint();

  text t(x, y, chars);
  t.SeTextSize(fTextSize*gStyle->GeImageScaling());
  t.SeTextFont(fTextFont);
  t.SeTextAlign(fTextAlign);
  t.SeTextAngle(fTextAngle);
  t.SeTexColor(fTexColor);
  _image->drawTextOnPad(&t, XtoPixel(x), YtoPixel(y), gPad, fX0, fY0);
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// draw text
  ///
  ///  - x: x position of the text
  ///  - y: y position of the text

  void ImageDump::text(double x, double y, const wchar_t *chars)
  {
  if (!gPad || !_image)
    return;

  _image->BeginPaint();

  text t(x, y, chars);
  t.SeTextSize(fTextSize*gStyle->GeImageScaling());
  t.SeTextFont(fTextFont);
  t.SeTextAlign(fTextAlign);
  t.SeTextAngle(fTextAngle);
  t.SeTexColor(fTexColor);
  _image->drawTextOnPad(&t, XtoPixel(x), YtoPixel(y), gPad, fX0, fY0);
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// draw text with URL. Same as text.
  ///

  void ImageDump::textUrl(double x, double y, const char *chars, const char *)
  {
  text(x, y, chars);
  }

  ////////////////////////////////////////////////////////////////////////////////
  ///cell array begin

  void ImageDump::cellArrayBegin(int w, int h, double x1, double x2,
                                 double y1, double y2)
  {
  if (!gPad || !_image || (w <= 0) || (h <= 0))
    return;

  _image->BeginPaint();

  _cellArrayW = w;
  _cellArrayH = h;
  _cellArrayColors.resize(w * h);
  _cellArrayIdx = 0;

  _cellArrayX1 = x1 < x2 ? XtoPixel(x1) : XtoPixel(x2);
  _cellArrayX2 = x1 > x2 ? XtoPixel(x2) : XtoPixel(x1);
  _cellArrayY1 = y1 < y2 ? YtoPixel(y1) : YtoPixel(y2);
  _cellArrayY2 = y1 < y2 ? YtoPixel(y2) : YtoPixel(y1);
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// cell array fill

  void ImageDump::cellArrayFill(int r, int g, int b)
  {
  if (_cellArrayIdx >= _cellArrayColors.size())
    return;

  _image->BeginPaint();

  _cellArrayColors[_cellArrayIdx++] = ((r & 0xFF) << 16) + ((g & 0xFF) << 8) + (b & 0xFF);
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// cell array end

  void ImageDump::cellArrayEnd()
  {
  if (!_image || _cellArrayColors.empty() || !_cellArrayW || !_cellArrayH) return;
  _image->BeginPaint();
  _image->drawCellArray(_cellArrayX1, _cellArrayX2, _cellArrayY1, _cellArrayY2,
                        _cellArrayW, _cellArrayH, _cellArrayColors.data());
  _cellArrayColors.clear();
  _cellArrayIdx = 0;
  _cellArrayW   = 0;
  _cellArrayH   = 0;
  _cellArrayX1  = 0;
  _cellArrayX2  = 0;
  _cellArrayY1  = 0;
  _cellArrayY2  = 0;
  }


} //  namespace CAP
