/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CanvasRegistry.hpp
//! @brief CanvasRegistry: the canvases of a session, by index, name and group (not owned).
#ifndef CAP__CanvasRegistry
#define CAP__CanvasRegistry
#include <vector>
#include "Canvas.hpp"
#include "String.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Painter;

  //!
  //! @brief The canvases of a session, listed (not owned) by index, by name, and by group (a window, a step of an
  //! analysis...): the top level from which every canvas, and through it every graph, can be reached.
  //!
  //! A canvas listed here removes itself when it is destroyed; its copies are not listed (undo states, for instance).
  //! Names are unique: a name already taken gets a suffix (_2, _3...).
  //! @code
  //!   CanvasRegistry & canvases = CanvasRegistry::session();
  //!   canvases.add(canvas, "pt spectra", "Window 1");
  //!   for (Canvas & c : canvases) ...                         // every canvas, in the order added
  //!   Canvas * c = canvases.find("pt spectra");
  //!   canvases.saveAll("figures", ".pdf");                    // one file per canvas
  //!   canvases.writeBook("all.pdf");                          // one page per canvas, to print
  //! @endcode
  //!
  class CanvasRegistry
  {
  public:
  //! @brief The canvases of the session. @return registry
  static CanvasRegistry & session();
  //! @brief Empty registry (usually the one of the session is used, see session()).
  CanvasRegistry() : _entries() { }
  //! @brief Destructor: the canvases still listed are told so.
  ~CanvasRegistry();
  CanvasRegistry(const CanvasRegistry &) = delete;
  CanvasRegistry & operator=(const CanvasRegistry &) = delete;

  //! @name Listing
  //! @{
  //! @brief List a canvas (or rename and regroup it if it is listed). A canvas is listed by one registry at a time.
  //! @param canvas canvas @param name name (the canvas's name if empty; made unique) @param group group ("" for none)
  //! @return the name given
  String add(Canvas & canvas, const String & name="", const String & group="");
  //! @brief Stop listing a canvas (nothing if it is not listed). @param canvas canvas
  void remove(const Canvas & canvas);
  //! @brief Whether a canvas is listed. @param canvas canvas @return true if so
  bool contains(const Canvas & canvas) const { return indexOf(canvas) >= 0; }
  //! @brief Rename a listed canvas (made unique). @param canvas canvas @param name new name @return the name given
  String rename(const Canvas & canvas, const String & name);
  //! @brief Move a listed canvas to a group. @param canvas canvas @param group group
  void setGroup(const Canvas & canvas, const String & group);
  //! @}

  //! @name Access
  //! @{
  int size() const { return int(_entries.size()); }   //!< Number of canvases listed
  Canvas & canvas(int k) const { return *_entries.at(size_t(k)).canvas; }   //!< Canvas k, in the order added
  const String & name(int k) const { return _entries.at(size_t(k)).name; }   //!< Name of canvas k
  const String & group(int k) const { return _entries.at(size_t(k)).group; }   //!< Group of canvas k
  //! @brief Canvas of a name. @param name name @return canvas, or null
  Canvas * find(const String & name) const;
  //! @brief Index of a canvas. @param canvas canvas @return index, or -1 if not listed
  int indexOf(const Canvas & canvas) const;
  //! @brief Name of a listed canvas. @param canvas canvas @return name, or "" if not listed
  String nameOf(const Canvas & canvas) const;
  //! @brief Groups, in the order of their first canvas. @return names
  std::vector<String> groups() const;
  //! @brief Canvases of a group (all of them if group is empty). @param group group @return canvases
  std::vector<Canvas *> canvases(const String & group="") const;
  //! @}

  //! Iteration over the canvases (Canvas &), in the order added
  class Iterator
  {
  public:
  Iterator(const CanvasRegistry * r, int k) : _r(r), _k(k) { }   //!< Iterator at canvas k
  Canvas & operator*() const { return _r->canvas(_k); }   //!< The canvas
  Iterator & operator++() { _k++; return *this; }   //!< Next canvas
  bool operator!=(const Iterator & o) const { return _k != o._k; }   //!< Different position
  private:
  const CanvasRegistry * _r;   //!< registry
  int _k;   //!< position
  };
  Iterator begin() const { return Iterator(this, 0); }   //!< First canvas
  Iterator end() const { return Iterator(this, size()); }   //!< Past the last canvas

  //! @name Output of all the canvases
  //! @{
  //!
  //! @brief Save every canvas (of a group, or all), or every graph of every canvas, to its own file, named after the
  //! canvas (and the number of the graph): SVG or PDF by the extension, the size of the canvas.
  //! @param directory directory (created if needed) @param extension ".pdf" or ".svg" @param perGraph one file per graph
  //! @param group group ("" for all the canvases)
  //! @return paths of the files written
  //!
  std::vector<String> saveAll(const String & directory, const String & extension=".pdf", bool perGraph=false,
                              const String & group="") const;
  //! @brief Write every canvas (of a group, or all) as a page of one PDF file, to print or to share.
  //! @param fileName PDF file @param group group ("" for all) @return true on success
  bool writeBook(const String & fileName, const String & group="") const;
  //! @}

  //!
  //! @brief Draw graph k of a canvas alone, on a page the size of its cell (the canvas drawn at width x height, moved
  //! so that the cell fills the page, its background under it).
  //! @param painter painter with regions (SvgPainter, PdfPainter) @param canvas canvas @param k graph
  //! @param width width of the canvas @param height height
  //!
  template <typename PAINTER>
  static void drawGraph(PAINTER & painter, const Canvas & canvas, int k, float width, float height)
  {
  const Graph & graph = canvas.graph(k);
  painter.beginRegion(-graph.cellXLow()*width, -(1.0f - graph.cellYHigh())*height, width, height);
  std::vector<Point2D> cell = { Point2D(graph.cellXLow(), graph.cellYLow()), Point2D(graph.cellXHigh(), graph.cellYLow()),
                                Point2D(graph.cellXHigh(), graph.cellYHigh()), Point2D(graph.cellXLow(), graph.cellYHigh()) };
  LineAttributes noLine;
  noLine.setStyle(0);
  painter.drawPolygon(cell, canvas.background(), noLine);
  graph.draw(painter);
  painter.endRegion();
  }
  //! @brief Text usable as a file name (letters, digits, '-', '_', '.'; the rest as '_'). @param text text @return name
  static String fileName(const String & text);

  private:
  //! A canvas listed
  struct Entry { Canvas * canvas; String name; String group; };
  std::vector<Entry> _entries;   //!< canvases, in the order added

  //! @brief A name not taken by another canvas. @param name name wanted @param except canvas allowed to have it
  String uniqueName(const String & name, const Canvas * except) const;
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__CanvasRegistry
