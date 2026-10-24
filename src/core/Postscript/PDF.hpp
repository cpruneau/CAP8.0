#ifndef CAP__PDF
#define CAP__PDF
#include "VirtualPS.hpp"
#include <vector>
#include "ClassDef.hpp"

class Points2D;

class PDF : public VirtualPS
{
  
protected:
  float _red = 0.;                   ///< Per cent of red
  float _green = 0.;                 ///< Per cent of green
  float _blue = 0.;                  ///< Per cent of blue
  float _alpha = 1.;                 ///< Per cent of transparency
  std::vector<float> _alphas;          ///< List of alpha values used
  float _xPageSize = 0.;                 ///< Page size along X
  float _yPageSize = 0.;                 ///< Page size along Y
  int _type = 0;                     ///< Workstation type used to know if the PDF is open
  int _pageFormat = 0;               ///< Page format (A4, Letter etc ...)
  int _pageOrientation = 0;          ///< Page orientation (Portrait, Landscape)
  int fStartStream = 0;              ///< Stream start
  float _linearScale = 0.;             ///< Line width scale factor
  std::vector<int> _objectPos;          ///< Objects position
  int _pageCount = 0;                   ///< Number of pages
  int _currentPage = 0;              ///< Object number of the current page
  std::vector<int> _pageObjects;       ///< Page object numbers
  std::vector<std::string> _hasURLs;      ///< URLs
  std::vector<float> fRectX1;          ///< x1 /Rect coordinates for url annots
  std::vector<float> fRectY1;          ///< y1 /Rect coordinates for url annots
  std::vector<float> fRectX2;          ///< x2 /Rect coordinates for url annots
  std::vector<float> fRectY2;          ///< y2 /Rect coordinates for url annots
  bool _opened = kFALSE;       ///< True if an object is opened
  bool _pageNotEmpty = kFALSE;       ///< True if the current page is not empty
  bool _compress = kFALSE;           ///< True when fBuffer must be compressed
  bool _rangeDefined = kFALSE;              ///< True when a range has been defined
  bool _hasURL = kFALSE;                ///< True when the text has an URL
  int fNbUrl = 1;                    ///< Number of URLs in the current page
  double fA = 1.;                    ///< "a" value of the Current Transformation Matrix (CTM)
  double fB = 0.;                    ///< "b" value of the Current Transformation Matrix (CTM)
  double fC = 0.;                    ///< "c" value of the Current Transformation Matrix (CTM)
  double fD = 1.;                    ///< "d" value of the Current Transformation Matrix (CTM)
  double fE = 0.;                    ///< "e" value of the Current Transformation Matrix (CTM)
  double fF = 0.;                    ///< "f" value of the Current Transformation Matrix (CTM)
  
  static int fgLineJoin; ///< Appearance of joining lines
  static int fgLineCap;  ///< Appearance of line caps
  
  void ensureBufferSize(int required_size);
  
public:
  PDF();
  PDF(const String & fileName, int type=-111);
  virtual ~PDF() {}
  virtual void close();
  virtual double cmToPDF(double u) { return int(0.5 + 72 * u / 2.54); }
  virtual void computeRect(const char* chars, double fontsize, double a, double b, double c, double d, double e, double f);
  virtual void drawBox(double x1, double y1,double x2, double  y2);
  virtual void drawFrame(double xl, double yl, double xt, double  yt,
                         int mode, int border, int dark, int light);
  virtual void DrawHatch(float dy, float angle, int n, float *x, float *y);
  virtual void DrawHatch(float dy, float angle, int n, double *x, double *y);
  virtual void DrawPolyLine(int n, Point2Ds *xy);
  virtual void DrawPolyLineNDC(int n, Point2Ds *uv);
  virtual void drawMarkers(int n, float *x, float *y);
  virtual void drawMarkers(int n, double *x, double *y);
  virtual  void drawPS(int n, float *xw, float *yw);
  virtual  void drawPS(int n, double *xw, double *yw);
  virtual   void LineTo(double x, double y);
  virtual  void MoveTo(double x, double y);
  virtual  void endObject();
  virtual  void FontEncode();
  virtual  void newObject(int n);
  virtual  void newPage();
  virtual  void Off();
  virtual  void On();
  virtual  void open(const char *filename, int type = -111);
  virtual  void PatternEncode();
  virtual  void printFast(int nch, const char *string = "");
  virtual  void printStr(const char *string = "");
  virtual  void Range(float xrange, float yrange);
  virtual void setAlpha(float alpha = 1.);
  virtual  void setColor(int color = 1);
  virtual  void setColor(float r, float g, float b);
  virtual void setFillColor( Color_t cindex = 1);
  virtual void setFillPatterns(int ipat, int color);
  virtual void setLineColor( Color_t cindex = 1);
  virtual void setLineJoin(int linejoin = 0);
  virtual void setLineCap(int linecap = 0);
  virtual void setLineScale(float scale = 1) {_linearScale = scale;}
  virtual void setLineStyle(Style_t linestyle = 1);
  virtual void setLineWidth(Width_t linewidth = 1);
  virtual void setMarkerColor(Color_t cindex = 1);
  virtual void SeTexColor(Color_t cindex = 1);
  virtual void text(double x, double y, const char *string);
  virtual void text(double, double, const wchar_t *);
  virtual void textUrl(double x, double y, const char *string, const char *url);
  virtual void textNDC(double u, double v, const char *string);
  virtual void textNDC(double, double, const wchar_t *);
  virtual void writeCompressedBuffer();
  virtual void writeCM(double a, double b, double c, double d, double e, double f, bool acc = kTRUE);
  virtual void writeReal(float r, bool space = kTRUE);
  virtual void writeUrlObjects();
  virtual double UtoPDF(double u);
  virtual double VtoPDF(double v);
  virtual double XtoPDF(double x);
  virtual double YtoPDF(double y);
  
  ClassDefOverride(PDF, 1); // PDF driver
};

#endif
