#ifndef CAP__ImageDump
#define CAP__ImageDump
#include "VirtualPS.hpp"
#include "ClassDef.hpp"
class Image;
class Color;
class Point2D;

class ImageDump :
  public VirtualPS
{
public:
  enum imageFileType { kImageUnknown, kImagePortrait, kImageLandascape, kImagePreview };
protected:
   Image           *_image{nullptr};     ///< Image
   int             _type{0};            ///< PostScript workstation type
   int             fX0{0}, fY0{0};      ///< offset of selected pad to canvas

   std::vector<UInt_t> _cellArrayColors;
   int               _cellArrayW{0};
   int               _cellArrayH{0};
   int               _cellArrayX1{0};
   int               _cellArrayX2{0};
   int               _cellArrayY1{0};
   int               _cellArrayY2{0};
   UInt_t              _cellArrayIdx{0};

   int  XtoPixel(double x);
   int  YtoPixel(double y);
   void   DrawDashPolyLine(int npoints, Point2D *pt, UInt_t nDash,
                           const char* pDash, const char* col, UInt_t thick);

public:
   ImageDump();
   ImageDump(const String & filename, int type = -111);
   ~ImageDump() override;

  void setType(int type) { _type = type; }
  int type() const { return _type; }

   void  cellArrayBegin(int W, int H, double x1, double x2, double y1, double y2) override;
   void  cellArrayFill(int r, int g, int b) override;
   void  cellArrayEnd() override;
   void  close() override;
   void  drawBox(const Point2D & point1, const Point2D & point2) override;
   void  drawFrame(double xl, double yl, double xt, double  yt,
                   int mode, int border, int dark, int light) override;
   void  drawMarkers(int n, float *x, float *y) override;
   void  drawMarkers(int n, double *x, double *y) override;
   void  drawPS(int n, float *xw, float *yw) override;
   void  drawPS(int n, double *xw, double *yw) override;
   void  newPage() override;
   void  open(const char *filename, int type = -111) override;
   void  text(double x, double y, const char *string) override;
   void  text(double x, double y, const wchar_t *string) override;
   void  textUrl(double x, double y, const char *string, const char *url) override;
   void  setColor(float r, float g, float b) override;
   void *stream() const override { return (void*)_image; }
   void  setType(int type = -111) override { _type = type; }
   int   type() const override { return _type; }
   Image * image() const { return _image; }

   ClassDefOverride(ImageDump,0)  // create image in batch mode
};

#endif
