/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PptxPainter.hpp
//! @brief PptxPainter: a Painter writing PowerPoint presentations (.pptx), one slide per canvas, as native shapes.
#ifndef CAP__PptxPainter
#define CAP__PptxPainter
#include <sstream>
#include <string>
#include <vector>
#include "Painter.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Canvas;

  //!
  //! @brief Painter writing a PowerPoint presentation (Office Open XML, .pptx), with no library.
  //!
  //! Each canvas becomes one slide, fitted to the slide (keeping its proportions) and centred on it. What the
  //! renderers draw becomes native PowerPoint shapes, grouped per slide: lines and polygons are free-form shapes,
  //! markers are preset shapes (circles, squares, triangles, diamonds, stars) or crosses, and texts are text boxes,
  //! so that the slides are vector graphics that can be edited in PowerPoint (and read by Keynote, LibreOffice and
  //! Google Slides). Texts typeset by LaTeX elsewhere are written as plain (Unicode) text, see
  //! LatexRenderer::plainText().
  //! @code
  //!   PptxPainter pptx;                         // 16:9 slides, 13.33 x 7.5 inches
  //!   for (const Canvas * c : canvases) pptx.addCanvas(*c);
  //!   pptx.write("plots.pptx");
  //! @endcode
  //!
  class PptxPainter : public Painter
  {
  public:

  //! @brief Presentation with slides of the given size in points (default 960 x 540: 16:9).
  //! @param slideWidth width of the slides @param slideHeight height of the slides
  PptxPainter(float slideWidth=960.0f, float slideHeight=540.0f);
  //! @brief Destructor.
  virtual ~PptxPainter() { }

  //! @brief Start a slide on which a drawing area of the given proportions is fitted and centred.
  //! @param width width of the drawing (e.g. of the canvas, points) @param height height of the drawing
  void newSlide(float width, float height);
  //! @brief Draw a canvas on a new slide. @param canvas canvas
  void addCanvas(const Canvas & canvas);
  //! @brief Number of slides. @return number
  int nSlides() const { return int(_slides.size()) + (_open ? 1 : 0); }

  virtual float width()  const override { return _regionWidth;  }   //!< Width of the drawing area of the slide (points)
  virtual float height() const override { return _regionHeight; }   //!< Height of the drawing area of the slide (points)
  virtual void drawLine(const std::vector<Point2D> & points, const LineAttributes & line) override;
  virtual void drawPolygon(const std::vector<Point2D> & points, const FillAttributes & fill, const LineAttributes & line) override;
  virtual void fillPolygon(const std::vector<Point2D> & points, const Color & color) override;
  virtual void fillRectangle(float x0, float y0, float x1, float y1, const Color & color) override;
  virtual void drawMarker(const Point2D & position, const MarkerAttributes & marker) override;
  virtual void drawText(const Point2D & position, const String & text, const TextAttributes & attributes, float angle=0.0) override;

  //! @brief Write the presentation. @param fileName file (.pptx) @return false if it cannot be written
  bool write(const String & fileName);
  //! @brief The bytes of the presentation (a ZIP archive). @return bytes
  std::string package();

  //! A run of text: its characters (UTF-8) and its position (0 normal, 1 superscript, -1 subscript)
  struct Run { std::string text; int script = 0; };
  //!
  //! @brief The runs of a text as PowerPoint shows it: LaTeX ($...$, \command) and ROOT syntax (#pi, ^{}, _{}) turned
  //! into Unicode characters (Greek letters, symbols, accents) and superscript and subscript runs.
  //! @param text text @return runs
  //!
  static std::vector<Run> textRuns(const String & text);
  //! @brief The bytes of a ZIP archive of files stored without compression (as PowerPoint accepts).
  //! @param files names and contents of the files, in order @return archive
  static std::string zip(const std::vector<std::pair<std::string,std::string>> & files);

  protected:
  float _slideWidth;    //!< size of the slides (points)
  float _slideHeight;   //!< size of the slides (points)
  float _regionX;       //!< drawing area of the current slide (points, y down)
  float _regionY;       //!< drawing area of the current slide (points, y down)
  float _regionWidth;   //!< drawing area of the current slide (points)
  float _regionHeight;  //!< drawing area of the current slide (points)
  bool  _open;          //!< a slide is being drawn
  int   _nextId;        //!< next shape id of the slide
  std::ostringstream _shapes;   //!< shapes of the current slide
  std::vector<std::string> _slides;   //!< XML of the slides finished

  //! @brief Finish the current slide.
  void closeSlide();
  //! @brief Slide x of a canvas point (points). @param p point @return x
  float px(const Point2D & p) const { return _regionX + p.x()*_regionWidth; }
  //! @brief Slide y of a canvas point (points, y down). @param p point @return y
  float py(const Point2D & p) const { return _regionY + (1.0f - p.y())*_regionHeight; }
  //! @brief Free-form shape through points (slide points, y down). @param xs abscissae @param ys ordinates
  //! @param close closed path @param fill fill XML ("" for none) @param line line XML
  void freeform(const std::vector<float> & xs, const std::vector<float> & ys, bool close, const std::string & fill,
                const std::string & line, const char * name);
  //! @brief Preset shape in a box (slide points). @param preset preset geometry @param x left @param y top
  //! @param w width @param h height @param fill fill XML @param line line XML @param flipV flipped vertically
  void preset(const char * preset, float x, float y, float w, float h, const std::string & fill, const std::string & line,
              bool flipV=false);
  //! @brief Fill XML of a color. @param r red @param g green @param b blue in [0,1] @param alpha opacity
  static std::string solidFill(float r, float g, float b, float alpha);
  //! @brief Line XML. @param color ROOT color number @param alpha opacity @param style line style @param width points
  static std::string lineXml(int color, float alpha, int style, float width);
  //! @brief Text with the XML special characters escaped. @param text text @return escaped text
  static std::string escape(const std::string & text);
  //! @brief Slide XML of a group of shapes. @param shapes shapes @return slide
  std::string slideXml(const std::string & shapes) const;
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__PptxPainter
