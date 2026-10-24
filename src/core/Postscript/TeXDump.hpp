#ifndef CAP__TexDump
#define CAP__TexDump


#include "VirtualPS.hpp"
#include "Rtypes.h"   // ClassDef: these classes derive from ROOT classes (TVirtualPS)

class Point2Ds;

class TexDump : public VirtualPS {

protected:
   float      _xPageSize = 0.;           ///< Page size along X
   float      _yPageSize = 0.;           ///< Page size along Y
   int        _type = 0;             ///< Workstation type used to know if the Tex is open
   bool       fBoundingBox = kFALSE; ///< True when the TeX header is printed
   bool       _rangeDefined = kFALSE;       ///< True when a range has been defined
   bool       fStandalone = kFALSE;  ///< True when a TeX file should be standalone
   float      fCurrentRed = -1.;     ///< Current Red component
   float      fCurrentGreen = -1.;   ///< Current Green component
   float      fCurrentBlue = -.1;    ///< Current Blue component
   float      fCurrentAlpha = 1.;    ///< Current Alpha value
   float      _linearScale = 0.;       ///< Line width scale factor

public:
   TexDump();
   TexDump(const char *filename, int type=-113);
   ~TexDump() override;

   void    cellArrayBegin(int W, int H, double x1, double x2, double y1, double y2) override;
   void    cellArrayFill(int r, int g, int b) override;
   void    cellArrayEnd() override;
   void    close(Option_t *opt="") override;
   int   CMtoTeX(double u) { return int(0.5 + 72*u/2.54); }
   void    DefineMarkers();
   void    drawBox(double x1, double y1,double x2, double  y2) override;
   void    drawFrame(double xl, double yl, double xt, double  yt,
                     int mode, int border, int dark, int light) override;
   void    DrawPolyLine(int n, Point2Ds *xy);
   void    DrawPolyLineNDC(int n, Point2Ds *uv);
   void    drawMarkers(int n, float *x, float *y) override;
   void    drawMarkers(int n, double *x, double *y) override;
   void    drawPS(int n, float *xw, float *yw) override;
   void    drawPS(int n, double *xw, double *yw) override;
   void    newPage() override;
   void    Off();
   void    On();
   void    open(const char *filename, int type=-111) override;
   void    Range(float xrange, float yrange);
   void    setColor(int color = 1);
   void    setColor(float r, float g, float b) override;
   void    SetFillColor(Color_t cindex=1) override;
   void    SetLineColor(Color_t cindex=1) override;
   void    SetLineStyle(Style_t linestyle = 1) override;
   void    SetLineWidth(Width_t linewidth = 1) override;
   void    SetLineScale(float scale=1) { _linearScale = scale; }
   void    SetMarkerSize(Size_t msize=1) override;
   void    SetMarkerColor(Color_t cindex=1) override;
   void    SeTexColor(Color_t cindex=1) override;
   void    text(double x, double y, const char *string) override;
   void    text(double, double, const wchar_t *) override {}
   void    textUrl(double x, double y, const char *string, const char *url) override;
   void    textNDC(double u, double v, const char *string);
   void    textNDC(double, double, const wchar_t *) {}
   float UtoTeX(double u);
   float VtoTeX(double v);
   float XtoTeX(double x);
   float YtoTeX(double y);

   ClassDefOverride(TexDump, 0);  //Tex driver
};

#endif
