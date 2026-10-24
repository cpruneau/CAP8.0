/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <cctype>
#include "CanvasRegistry.hpp"
#include "FileSystem.hpp"
#include "PdfPainter.hpp"
#include "SvgPainter.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  CanvasRegistry & CanvasRegistry::session()
  {
  static CanvasRegistry registry;
  return registry;
  }

  CanvasRegistry::~CanvasRegistry()
  {
  for (auto & e : _entries) if (e.canvas->_registry == this) e.canvas->_registry = nullptr;
  }

  String CanvasRegistry::uniqueName(const String & name, const Canvas * except) const
  {
  String base = name.str().empty() ? String("Canvas") : name;
  auto taken = [&](const String & n)
    {
    for (const auto & e : _entries) if (e.canvas != except && e.name == n) return true;
    return false;
    };
  if (!taken(base)) return base;
  for (int k=2;; k++)
    {
    String candidate = base + String("_") + String(std::to_string(k));
    if (!taken(candidate)) return candidate;
    }
  }

  String CanvasRegistry::add(Canvas & canvas, const String & name, const String & group)
  {
  if (canvas._registry && canvas._registry != this) canvas._registry->remove(canvas);
  String given = uniqueName(name.str().empty() ? canvas.name() : name, &canvas);
  int k = indexOf(canvas);
  if (k >= 0) { _entries[size_t(k)].name = given; _entries[size_t(k)].group = group; return given; }
  _entries.push_back(Entry{ &canvas, given, group });
  canvas._registry = this;
  return given;
  }

  void CanvasRegistry::remove(const Canvas & canvas)
  {
  for (auto it = _entries.begin(); it != _entries.end(); ++it)
    if (it->canvas == &canvas)
      {
      if (it->canvas->_registry == this) it->canvas->_registry = nullptr;
      _entries.erase(it);
      return;
      }
  }

  String CanvasRegistry::rename(const Canvas & canvas, const String & name)
  {
  int k = indexOf(canvas);
  if (k < 0) return String();
  _entries[size_t(k)].name = uniqueName(name, &canvas);
  return _entries[size_t(k)].name;
  }

  void CanvasRegistry::setGroup(const Canvas & canvas, const String & group)
  {
  int k = indexOf(canvas);
  if (k >= 0) _entries[size_t(k)].group = group;
  }

  Canvas * CanvasRegistry::find(const String & name) const
  {
  for (const auto & e : _entries) if (e.name == name) return e.canvas;
  return nullptr;
  }

  int CanvasRegistry::indexOf(const Canvas & canvas) const
  {
  for (std::size_t k=0; k<_entries.size(); k++) if (_entries[k].canvas == &canvas) return int(k);
  return -1;
  }

  String CanvasRegistry::nameOf(const Canvas & canvas) const
  {
  int k = indexOf(canvas);
  return k < 0 ? String() : _entries[size_t(k)].name;
  }

  std::vector<String> CanvasRegistry::groups() const
  {
  std::vector<String> list;
  for (const auto & e : _entries)
    {
    bool seen = false;
    for (const auto & g : list) if (g == e.group) seen = true;
    if (!seen) list.push_back(e.group);
    }
  return list;
  }

  std::vector<Canvas *> CanvasRegistry::canvases(const String & group) const
  {
  std::vector<Canvas *> list;
  for (const auto & e : _entries) if (group.str().empty() || e.group == group) list.push_back(e.canvas);
  return list;
  }

  String CanvasRegistry::fileName(const String & text)
  {
  std::string name;
  for (char c : text.str()) name += (std::isalnum((unsigned char) c) || c == '-' || c == '_' || c == '.') ? c : '_';
  return String(name.empty() ? std::string("canvas") : name);
  }

  std::vector<String> CanvasRegistry::saveAll(const String & directory, const String & extension, bool perGraph,
                                              const String & group) const
  {
  std::vector<String> written;
  if (!directory.str().empty() && !FileSystem::isDirectory(directory)) FileSystem::createDirectory(directory);
  bool pdf = FileSystem::hasExtension(String("x") + extension, ".pdf");
  String ext = pdf ? String(".pdf") : String(".svg");
  for (const auto & e : _entries)
    {
    if (!group.str().empty() && e.group != group) continue;
    const Canvas & canvas = *e.canvas;
    float w = float(canvas.width()), h = float(canvas.height());
    if (!perGraph)
      {
      String path = FileSystem::join(directory, fileName(e.name) + ext);
      bool ok;
      if (pdf) { PdfPainter p(w, h); canvas.draw(p); ok = p.write(path); }
      else     { SvgPainter p(w, h); canvas.draw(p); ok = p.write(path); }
      if (ok) written.push_back(path);
      continue;
      }
    for (int k=0; k<canvas.nGraphs(); k++)
      {
      const Graph & g = canvas.graph(k);
      float gw = (g.cellXHigh() - g.cellXLow())*w, gh = (g.cellYHigh() - g.cellYLow())*h;
      String path = FileSystem::join(directory, fileName(e.name) + String("_") + String(std::to_string(k + 1)) + ext);
      bool ok;
      if (pdf) { PdfPainter p(gw, gh); drawGraph(p, canvas, k, w, h); ok = p.write(path); }
      else     { SvgPainter p(gw, gh); drawGraph(p, canvas, k, w, h); ok = p.write(path); }
      if (ok) written.push_back(path);
      }
    }
  return written;
  }

  bool CanvasRegistry::writeBook(const String & fileName, const String & group) const
  {
  PdfPainter * painter = nullptr;
  for (const auto & e : _entries)
    {
    if (!group.str().empty() && e.group != group) continue;
    float w = float(e.canvas->width()), h = float(e.canvas->height());
    if (!painter) painter = new PdfPainter(w, h);
    else painter->newPage(w, h);
    e.canvas->draw(*painter);
    }
  if (!painter) return false;
  bool ok = painter->write(fileName);
  delete painter;
  return ok;
  }

  } // namespace GRAPHICS
} // namespace CAP
