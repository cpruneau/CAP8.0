/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CanvasPresentation.hpp
//! @brief CanvasPresentation: a Beamer presentation of canvases (LatexDocument), one frame per canvas or per graph.
#ifndef CAP__CanvasPresentation
#define CAP__CanvasPresentation
#include <vector>
#include "CanvasRegistry.hpp"
#include "String.hpp"

namespace CAP
{

  //!
  //! @brief A presentation of canvases: each canvas (or each of its graphs) is written as a PDF figure and shown on its
  //! own frame of a Beamer document (LatexDocument), titled after the canvas; the document can be compiled with
  //! pdflatex.
  //! @code
  //!   CanvasPresentation talk("Two-particle correlations");
  //!   talk.setAuthor("C. Pruneau", "", "Wayne State University");
  //!   talk.addCanvases(GRAPHICS::CanvasRegistry::session());       // every canvas of the session
  //!   String tex = talk.write("talk", "correlations");             // talk/correlations.tex, talk/figures/*.pdf
  //!   talk.compile(tex);                                           // talk/correlations.pdf
  //! @endcode
  //!
  class CanvasPresentation
  {
  public:
  //! @brief Presentation of a title. @param title title
  explicit CanvasPresentation(const String & title="");

  void setTitle(const String & title) { _title = title; }   //!< Title (first frame)
  void setSubtitle(const String & subtitle) { _subtitle = subtitle; }   //!< Subtitle
  //! @brief Author shown on the title frame. @param name name @param email email @param affiliation affiliation
  void setAuthor(const String & name, const String & email="", const String & affiliation="")
  { _author = name; _email = email; _affiliation = affiliation; }
  void setTheme(const String & theme) { _theme = theme; }   //!< Beamer theme (default "Madrid")
  //! @brief One frame per graph rather than per canvas, for the canvases added after. @param v one per graph
  void setOnePerGraph(bool v) { _onePerGraph = v; }

  //! @brief Frame(s) of a canvas. @param canvas canvas @param title title of the frame (the canvas's title if empty)
  void addCanvas(const GRAPHICS::Canvas & canvas, const String & title="");
  //! @brief Frames of the canvases of a registry (of a group, or all), titled after their names in it.
  //! @param registry registry @param group group ("" for all)
  void addCanvases(const GRAPHICS::CanvasRegistry & registry, const String & group="");
  //! @brief Frame of text (a list of items, one per line of the text). @param title title @param text text
  void addText(const String & title, const String & text);
  //! @brief Number of frames (the title frame not counted). @return number
  int nFrames() const { return int(_frames.size()); }

  //!
  //! @brief Write the presentation: its figures as PDF files in directory/figures, and directory/name.tex.
  //! @param directory directory (created if needed) @param name name of the .tex file (without extension)
  //! @return path of the .tex file, or "" if it cannot be written
  //!
  String write(const String & directory, const String & name);
  //!
  //! @brief Compile a .tex file with pdflatex (twice, for the references), in its directory.
  //! @param texPath file written by write() @param log if not null, receives the output of pdflatex
  //! @return true if the PDF was made
  //!
  static bool compile(const String & texPath, String * log=nullptr);

  private:
  //! A frame: its title, and the canvas (and graph, -1 for all) shown, or a text
  struct Frame { String title; const GRAPHICS::Canvas * canvas; int graph; String text; };
  String _title;   //!< title
  String _subtitle;   //!< subtitle
  String _author;   //!< author
  String _email;   //!< email of the author
  String _affiliation;   //!< affiliation of the author
  String _theme;   //!< Beamer theme
  bool _onePerGraph;   //!< one frame per graph
  std::vector<Frame> _frames;   //!< frames, in order
  };

} // namespace CAP

#endif // !CAP__CanvasPresentation
