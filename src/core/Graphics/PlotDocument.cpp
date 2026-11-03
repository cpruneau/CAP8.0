/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <exception>
#include <map>
#include <set>
#include <string>
#include "PlotDocument.hpp"
#include "FileSystem.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  namespace
  {
  const std::set<std::string> & styleNames()
  {
  static const std::set<std::string> names = { "Margins", "Axis", "Frame", "Legend", "TitleText", "Background",
                                               "Line", "Marker", "Fill", "SysFill", "SysLine", "ContourLine", "Labels" };
  return names;
  }

  String lastPart(const String & path)
  {
  const std::string & p = path.str();
  std::size_t k = p.find_last_of('/');
  return k == std::string::npos ? path : String(p.substr(k + 1));
  }

  String grid(const XmlElement & e)
  {
  if (!e.hasAttribute("columns") && !e.hasAttribute("rows")) return String("");
  return String("  (") + e.attribute("columns", "1") + String(" x ") + e.attribute("rows", "1") + String(")");
  }

  const size_t kMaxUndo = 100;
  } // namespace

  PlotDocument::PlotDocument()
  :
  _document("PlotHistos")
  {
  _document.root().setAttribute("version", 1);
  XmlElement & output = _document.root().addChild("Output");
  output.setAttribute("directory", "figures");
  output.setAttribute("pdf", "plots.pdf");
  output.setAttribute("pptx", "plots.pptx");
  output.setAttribute("latex", "plots_latex.tex");
  output.setAttribute("title", "Plots");
  }

  bool PlotDocument::read(const String & fileName, String & error)
  {
  try
    {
    XmlDocument document = XmlDocument::read(fileName);
    if (!document.root().isNamed("PlotHistos")) { error = String("not a PlotHistos file (root element ") + document.root().name() + String(")"); return false; }
    _document = document;
    }
  catch (const std::exception & e) { error = String(e.what()); return false; }
  // the comments before the root element
  _prolog.clear();
  if (std::FILE * f = std::fopen(fileName.data(), "rb"))
    {
    std::string text;
    char buffer[4096];
    std::size_t n;
    while ((n = std::fread(buffer, 1, sizeof(buffer), f)) > 0 && text.size() < 1000000) text.append(buffer, n);
    std::fclose(f);
    std::size_t root = text.find("<PlotHistos");
    std::size_t start = text.find("<!--");
    while (start != std::string::npos && start < root)
      {
      std::size_t end = text.find("-->", start);
      if (end == std::string::npos || end > root) break;
      _prolog += text.substr(start, end + 3 - start) + "\n";
      start = text.find("<!--", end);
      }
    }
  _fileName = FileSystem::normalize(fileName);
  _undo.clear();
  _redo.clear();
  _dirty = false;
  _version++;
  return true;
  }

  bool PlotDocument::write(const String & fileName, String & error)
  {
  String target = FileSystem::normalize(fileName);
  String oldDirectory = directory(), newDirectory = FileSystem::parent(target);
  XmlDocument out = _document;
  // inputs: relative to the file when near it; the output directory follows the file when it moves
  bool untitled = _fileName.isNull();
  for (XmlElement * e : out.root().children())
    {
    const char * attribute = e->isNamed("Input") ? "file" : (e->isNamed("Output") ? "directory" : nullptr);
    if (!attribute || !e->hasAttribute(attribute)) continue;
    String value = e->attribute(attribute);
    if (value.isNull() || value.str()[0] == '$' || value.str()[0] == '~') continue;
    bool absolute = value.str()[0] == '/';
    if (e->isNamed("Output") && (untitled || absolute || oldDirectory == newDirectory)) continue;
    String full = absolute ? FileSystem::normalize(value) : FileSystem::normalize(FileSystem::join(oldDirectory, value));
    e->setAttribute(attribute, relativePath(full, newDirectory));
    }
  try
    {
    std::string text = out.toString().str();
    if (!_prolog.empty())
      {
      std::size_t declaration = text.find("?>");
      std::size_t at = declaration == std::string::npos ? 0 : text.find('\n', declaration) + 1;
      text.insert(at, _prolog);
      }
    std::FILE * f = std::fopen(target.data(), "wb");
    if (!f) { error = String("cannot write ") + target; return false; }
    bool ok = std::fwrite(text.data(), 1, text.size(), f) == text.size();
    ok = std::fclose(f) == 0 && ok;
    if (!ok) { error = String("cannot write ") + target; return false; }
    }
  catch (const std::exception & e) { error = String(e.what()); return false; }
  _document = out;
  _fileName = target;
  _dirty = false;
  _version++;
  return true;
  }

  String PlotDocument::directory() const
  {
  return _fileName.isNull() ? FileSystem::current() : FileSystem::parent(_fileName);
  }

  String PlotDocument::displayName() const
  {
  if (_fileName.isNull()) return _untitled;
  const std::string & p = _fileName.str();
  std::size_t k = p.find_last_of('/');
  return String(k == std::string::npos ? p : p.substr(k + 1));
  }

  XmlElement * PlotDocument::element(const Path & path)
  {
  XmlElement * e = &_document.root();
  for (int k : path)
    {
    if (k < 0 || k >= int(e->nChildren())) return nullptr;
    e = &e->child(unsigned(k));
    }
  return e;
  }

  const XmlElement * PlotDocument::element(const Path & path) const
  {
  return const_cast<PlotDocument *>(this)->element(path);
  }

  PlotDocument::Kind PlotDocument::kind(const XmlElement & e)
  {
  const String & n = e.name();
  if (n == "PlotHistos") return kRoot;
  if (n == "Input")      return kInput;
  if (n == "Output")     return kOutput;
  if (n == "Window")     return kWindow;
  if (n == "Canvas")     return kCanvas;
  if (n == "Graph")      return kGraph;
  if (n == "Histogram" || n == "Data" || n == "Profile") return kHistogram;
  if (n == "Function")   return kFunction;
  if (styleNames().count(n.str())) return kStyle;
  return kOther;
  }

  PlotDocument::Kind PlotDocument::kind(const Path & path) const
  {
  const XmlElement * e = element(path);
  return e ? kind(*e) : kOther;
  }

  const char * PlotDocument::kindName(Kind kind)
  {
  switch (kind)
    {
    case kRoot:      return "Plots";
    case kInput:     return "Input";
    case kOutput:    return "Output";
    case kWindow:    return "Window";
    case kCanvas:    return "Canvas";
    case kGraph:     return "Graph";
    case kHistogram: return "Histogram";
    case kFunction:  return "Function";
    case kStyle:     return "Style";
    default:         return "Element";
    }
  }

  String PlotDocument::label(const XmlElement & e)
  {
  switch (kind(e))
    {
    case kRoot:   return String("Plots");
    case kInput:  return e.attribute("id", "") + String("  =  ") + e.attribute("file", "");
    case kOutput: return String("to ") + e.attribute("directory", ".") + String(":  ") + e.attribute("pdf", "") + String("  ")
                         + e.attribute("pptx", "") + String("  ") + e.attribute("latex", "");
    case kWindow: return e.attribute("title", "window") + grid(e);
    case kCanvas:
      {
      String name = e.attribute("name", ""), title = e.attribute("title", "");
      String text = title.isNull() ? name : title;
      if (!name.isNull() && !(name == text)) text += String("  [") + name + String("]");
      return (text.isNull() ? String("canvas") : text) + grid(e);
      }
    case kGraph:
      {
      String title = e.attribute("title", "");
      return title.isNull() ? String("graph") : title;
      }
    case kHistogram:
      {
      String path = e.attribute("path", ""), text = e.attribute("label", "");
      String shown = text.isNull() ? lastPart(path) : text;
      return shown + String("   ") + e.attribute("input", "") + String(":") + path;
      }
    case kFunction:
      return e.attribute("class", "function") + String("(") + e.attribute("parameters", "") + String(")  [")
             + e.attribute("xMin", "") + String(", ") + e.attribute("xMax", "") + String("]");
    default:
      {
      String text = e.name();
      if (e.hasAttribute("which")) text += String(" ") + e.attribute("which");
      int n = 0;
      for (const auto & a : e.attributes())
        {
        if (a.first == "which") continue;
        text += String(n == 0 ? "  " : ", ") + a.first + String("=") + a.second;
        if (++n == 4) { text += String(", ..."); break; }
        }
      return text;
      }
    }
  }

  bool PlotDocument::canContain(Kind parent, const String & child)
  {
  const std::string & c = child.str();
  switch (parent)
    {
    case kRoot:      return c == "Input" || c == "Output" || c == "Window";
    case kWindow:    return c == "Canvas";
    case kCanvas:    return c == "Graph" || c == "Background";
    case kGraph:     return c == "Histogram" || c == "Data" || c == "Profile" || c == "Function" || c == "Margins"
                            || c == "Axis" || c == "Frame" || c == "Legend" || c == "TitleText";
    case kHistogram:
    case kFunction:  return c == "Line" || c == "Marker" || c == "Fill" || c == "SysFill" || c == "SysLine" || c == "ContourLine";
    case kStyle:     return c == "Line" || c == "Labels" || c == "TitleText" || c == "Fill";
    default:         return false;
    }
  }

  PlotDocument::Path PlotDocument::enclosing(const Path & path, Kind wanted, bool & ok) const
  {
  Path p = path;
  while (true)
    {
    const XmlElement * e = element(p);
    if (e && kind(*e) == wanted) { ok = true; return p; }
    if (p.empty()) break;
    p.pop_back();
    }
  ok = false;
  return Path();
  }

  std::vector<PlotDocument::Path> PlotDocument::children(const Path & parent, Kind wanted) const
  {
  std::vector<Path> out;
  const XmlElement * e = element(parent);
  if (!e) return out;
  for (unsigned k=0; k<e->nChildren(); k++)
    if (kind(e->child(k)) == wanted) { Path p = parent; p.push_back(int(k)); out.push_back(p); }
  return out;
  }

  bool PlotDocument::locate(const Path & path, int & window, int & canvas) const
  {
  bool ok = false;
  Path w = enclosing(path, kWindow, ok);
  if (!ok) return false;
  std::vector<Path> windows = children(Path(), kWindow);
  window = int(std::find(windows.begin(), windows.end(), w) - windows.begin());
  bool inCanvas = false;
  Path c = enclosing(path, kCanvas, inCanvas);
  canvas = -1;
  if (inCanvas)
    {
    std::vector<Path> canvases = children(w, kCanvas);
    canvas = int(std::find(canvases.begin(), canvases.end(), c) - canvases.begin());
    }
  return true;
  }

  void PlotDocument::record()
  {
  if (_group > 0 && _groupRecorded) return;
  _undo.push_back(_document.root());
  if (_undo.size() > kMaxUndo) _undo.pop_front();
  _redo.clear();
  if (_group > 0) _groupRecorded = true;
  }

  void PlotDocument::beginEdit()
  {
  if (_group++ == 0) _groupRecorded = false;
  }

  void PlotDocument::endEdit()
  {
  if (_group > 0) _group--;
  }

  bool PlotDocument::undo()
  {
  if (_undo.empty()) return false;
  _redo.push_back(_document.root());
  _document.setRoot(_undo.back());
  _undo.pop_back();
  changed();
  return true;
  }

  bool PlotDocument::redo()
  {
  if (_redo.empty()) return false;
  _undo.push_back(_document.root());
  _document.setRoot(_redo.back());
  _redo.pop_back();
  changed();
  return true;
  }

  bool PlotDocument::setAttribute(const Path & path, const String & name, const String & value)
  {
  XmlElement * e = element(path);
  if (!e) return false;
  if (value.isNull())
    {
    if (!e->hasAttribute(name)) return false;
    record();
    e = element(path);
    e->removeAttribute(name);
    }
  else
    {
    if (e->hasAttribute(name) && e->attribute(name) == value) return false;
    record();
    e = element(path);
    e->setAttribute(name, value);
    }
  changed();
  return true;
  }

  String PlotDocument::style(const Path & path, const String & child, const String & name, const String & which) const
  {
  const XmlElement * e = element(path);
  if (!e) return String("");
  for (const XmlElement * c : e->children(child))
    {
    if (!which.isNull() && !(c->attribute("which", "") == which)) continue;
    return c->attribute(name, "");
    }
  return String("");
  }

  bool PlotDocument::setStyle(const Path & path, const String & child, const String & name, const String & value,
                              const String & which)
  {
  XmlElement * e = element(path);
  if (!e) return false;
  if (style(path, child, name, which) == value) return false;
  record();
  e = element(path);
  int index = -1;
  for (unsigned k=0; k<e->nChildren(); k++)
    {
    const XmlElement & c = e->child(k);
    if (c.isNamed(child) && (which.isNull() || c.attribute("which", "") == which)) { index = int(k); break; }
    }
  if (index < 0)
    {
    if (value.isNull()) return false;
    // style children first, before the items of a graph
    unsigned position = 0;
    while (position < e->nChildren() && kind(e->child(position)) == kStyle) position++;
    XmlElement c(child);
    if (!which.isNull()) c.setAttribute("which", which);
    e->insertChild(position, c);
    index = int(position);
    }
  XmlElement & c = e->child(unsigned(index));
  if (value.isNull()) c.removeAttribute(name); else c.setAttribute(name, value);
  bool empty = !c.hasChildren();
  for (const auto & a : c.attributes()) if (!(a.first == "which")) empty = false;
  if (empty) e->removeChild(unsigned(index));
  changed();
  return true;
  }

  bool PlotDocument::replace(const Path & path, const XmlElement & replacement)
  {
  XmlElement * e = element(path);
  if (!e) return false;
  if (e->toString() == replacement.toString()) return false;
  record();
  e = element(path);
  *e = replacement;
  changed();
  return true;
  }

  std::vector<String> PlotDocument::canvasNames() const
  {
  std::vector<String> names;
  for (const XmlElement * c : root().findAll("Canvas")) names.push_back(c->attribute("name", ""));
  return names;
  }

  String PlotDocument::uniqueCanvasName(const String & wanted) const
  {
  std::vector<String> names = canvasNames();
  std::string base;
  for (char ch : (wanted.isNull() ? std::string("canvas") : wanted.str()))
    base += (std::isalnum((unsigned char)ch) || ch == '_' || ch == '-') ? ch : '_';
  String name(base);
  for (int k=2; std::find(names.begin(), names.end(), name) != names.end(); k++)
    name = String(base + "_" + std::to_string(k));
  return name;
  }

  PlotDocument::Path PlotDocument::addWindow(const String & title)
  {
  record();
  XmlElement w("Window");
  int n = int(children(Path(), kWindow).size());
  w.setAttribute("title", title.isNull() ? String("Window ") + String(std::to_string(n + 1)) : title);
  w.setAttribute("width", 1200);
  w.setAttribute("height", 800);
  _document.root().addChild(w);
  changed();
  return Path{ int(root().nChildren()) - 1 };
  }

  PlotDocument::Path PlotDocument::addCanvas(const Path & window, const String & name)
  {
  XmlElement * w = element(window);
  if (!w || kind(*w) != kWindow) return Path();
  record();
  w = element(window);
  XmlElement c("Canvas");
  String n = uniqueCanvasName(name.isNull() ? String("canvas") : name);
  c.setAttribute("name", n);
  c.setAttribute("title", name.isNull() ? n : name);
  c.setAttribute("file", n + String(".pdf"));
  w->addChild(c);
  // a window whose grid is full gets one more row
  int nCanvases = int(w->children("Canvas").size());
  if (w->hasAttribute("columns") || w->hasAttribute("rows"))
    {
    long columns = std::max(1L, w->attributeInt("columns", 1)), rows = std::max(1L, w->attributeInt("rows", 1));
    if (columns*rows < nCanvases) w->setAttribute("rows", (nCanvases + columns - 1)/columns);
    }
  changed();
  Path p = window;
  p.push_back(int(w->nChildren()) - 1);
  return p;
  }

  PlotDocument::Path PlotDocument::addGraph(const Path & canvas)
  {
  XmlElement * c = element(canvas);
  if (!c || kind(*c) != kCanvas) return Path();
  record();
  c = element(canvas);
  c->addChild("Graph");
  int nGraphs = int(c->children("Graph").size());
  if (c->hasAttribute("columns") || c->hasAttribute("rows"))
    {
    long columns = std::max(1L, c->attributeInt("columns", 1)), rows = std::max(1L, c->attributeInt("rows", 1));
    if (columns*rows < nGraphs) c->setAttribute("rows", (nGraphs + columns - 1)/columns);
    }
  changed();
  Path p = canvas;
  p.push_back(int(c->nChildren()) - 1);
  return p;
  }

  PlotDocument::Path PlotDocument::addFunction(const Path & graph, const String & className, const String & parameters,
                                               double xMin, double xMax)
  {
  XmlElement * g = element(graph);
  if (!g || kind(*g) != kGraph) return Path();
  record();
  g = element(graph);
  XmlElement & f = g->addChild("Function");
  f.setAttribute("class", className);
  f.setAttribute("parameters", parameters);
  f.setAttribute("xMin", XmlElement::formatNumber(xMin));
  f.setAttribute("xMax", XmlElement::formatNumber(xMax));
  changed();
  Path p = graph;
  p.push_back(int(g->nChildren()) - 1);
  return p;
  }

  String PlotDocument::relativePath(const String & path, const String & directory)
  {
  std::string p = FileSystem::normalize(path).str(), d = FileSystem::normalize(directory).str();
  if (d.empty() || p.empty() || p[0] != '/') return String(p);
  if (d.back() != '/') d += '/';
  if (p.compare(0, d.size(), d) == 0) return String(p.substr(d.size()));
  // a common folder at most two levels up
  std::string up;
  std::string base = d.substr(0, d.size() - 1);
  for (int level=1; level<=2; level++)
    {
    std::size_t k = base.find_last_of('/');
    if (k == std::string::npos || k == 0) break;
    base = base.substr(0, k);
    up += "../";
    if (p.compare(0, base.size() + 1, base + "/") == 0) return String(up + p.substr(base.size() + 1));
    }
  return String(p);
  }

  String PlotDocument::input(const String & file)
  {
  String absolute = FileSystem::normalize(file);
  String dir = directory();
  for (const XmlElement * e : root().children("Input"))
    {
    String f = e->attribute("file", "");
    String resolved = f.isNull() ? f : (f.str()[0] == '/' ? FileSystem::normalize(f) : FileSystem::normalize(FileSystem::join(dir, f)));
    if (resolved == absolute) return e->attribute("id", "");
    }
  // a new input, named after the file, after the inputs
  std::string base = FileSystem::stem(absolute).str();
  std::string id = base.empty() ? std::string("data") : base;
  std::set<std::string> ids;
  for (const XmlElement * e : root().children("Input")) ids.insert(e->attribute("id", "").str());
  for (int k=2; ids.count(id); k++) id = base + "_" + std::to_string(k);
  record();
  XmlElement in("Input");
  in.setAttribute("id", String(id));
  in.setAttribute("file", _fileName.isNull() ? absolute : relativePath(absolute, dir));
  unsigned position = 0;
  XmlElement & r = _document.root();
  while (position < r.nChildren() && r.child(position).isNamed("Input")) position++;
  r.insertChild(position, in);
  changed();
  return String(id);
  }

  XmlElement PlotDocument::histogram(const String & inputId, const String & object)
  {
  XmlElement h("Histogram");
  h.setAttribute("input", inputId);
  h.setAttribute("path", object);
  h.setAttribute("label", lastPart(object));
  return h;
  }

  PlotDocument::Path PlotDocument::addData(const Path & target, const String & file, const std::vector<String> & objects)
  {
  if (objects.empty() || !element(target)) return Path();
  beginEdit();
  unsigned before = root().nChildren();
  String id = input(file);
  Path where = target;
  if (root().nChildren() > before && !where.empty())
    {
    // the new input went after the other inputs: the elements after it moved down by one
    int inserted = int(root().children("Input").size()) - 1;
    if (where[0] >= inserted) where[0]++;
    }
  Path result;
  Kind k = kind(where);
  if (k == kHistogram || k == kFunction || k == kStyle)
    {
    bool ok = false;
    Path item = k == kStyle ? enclosing(where, kHistogram, ok) : where;
    if (k == kStyle && !ok) item = enclosing(where, kFunction, ok);
    Path graph = enclosing(where, kGraph, ok);
    if (ok)
      {
      record();
      XmlElement * g = element(graph);
      int position = item.size() > graph.size() ? item[graph.size()] + 1 : int(g->nChildren());
      for (std::size_t j=0; j<objects.size(); j++) g->insertChild(unsigned(position + int(j)), histogram(id, objects[j]));
      result = graph;
      result.push_back(position);
      changed();
      }
    }
  else if (k == kGraph)
    {
    record();
    XmlElement * g = element(where);
    for (const String & o : objects) g->addChild(histogram(id, o));
    result = where;
    result.push_back(int(g->nChildren()) - int(objects.size()));
    changed();
    }
  else if (k == kCanvas)
    {
    for (std::size_t j=0; j<objects.size(); j++)
      {
      Path graph = addGraph(where);
      XmlElement * g = element(graph);
      g->setAttribute("title", lastPart(objects[j]));
      g->addChild(histogram(id, objects[j]));
      if (j == 0) result = graph;
      }
    }
  else
    {
    Path window;
    if (k == kWindow) window = where;
    else
      {
      std::vector<Path> windows = children(Path(), kWindow);
      window = windows.empty() ? addWindow("") : windows.back();
      }
    String name = objects.size() == 1 ? lastPart(objects[0]) : lastPart(FileSystem::parent(objects[0]));
    Path canvas = addCanvas(window, name);
    for (const String & o : objects)
      {
      Path graph = addGraph(canvas);
      XmlElement * g = element(graph);
      g->setAttribute("title", lastPart(o));
      g->addChild(histogram(id, o));
      }
    result = canvas;
    }
  endEdit();
  return result;
  }

  bool PlotDocument::remove(const Path & path)
  {
  if (path.empty() || !element(path)) return false;
  record();
  element(parent(path))->removeChild(unsigned(path.back()));
  changed();
  return true;
  }

  PlotDocument::Path PlotDocument::duplicate(const Path & path)
  {
  const XmlElement * e = element(path);
  if (path.empty() || !e) return Path();
  XmlElement copy = *e;
  if (copy.isNamed("Canvas"))
    {
    String name = uniqueCanvasName(copy.attribute("name", "canvas"));
    copy.setAttribute("name", name);
    if (copy.hasAttribute("file")) copy.setAttribute("file", name + String(".pdf"));
    }
  record();
  element(parent(path))->insertChild(unsigned(path.back() + 1), copy);
  changed();
  Path p = path;
  p.back()++;
  return p;
  }

  PlotDocument::Path PlotDocument::shift(const Path & path, int step)
  {
  if (path.empty() || !element(path)) return path;
  XmlElement * p = element(parent(path));
  int to = path.back() + step;
  if (to < 0 || to >= int(p->nChildren())) return path;
  record();
  p = element(parent(path));
  XmlElement moved = p->child(unsigned(path.back()));
  p->removeChild(unsigned(path.back()));
  p->insertChild(unsigned(to), moved);
  changed();
  Path q = path;
  q.back() = to;
  return q;
  }

  PlotDocument::Path PlotDocument::move(const Path & from, const Path & target, bool & ok)
  {
  ok = false;
  const XmlElement * e = element(from);
  const XmlElement * t = element(target);
  if (from.empty() || !e || !t) return Path();
  // not into itself or its descendants
  if (target.size() >= from.size() && std::equal(from.begin(), from.end(), target.begin())) return Path();
  Path destination;
  int index = -1;
  if (canContain(kind(*t), e->name())) { destination = target; index = int(t->nChildren()); }
  else if (!target.empty() && canContain(kind(parent(target)), e->name())) { destination = parent(target); index = target.back() + 1; }
  else return Path();
  XmlElement moved = *e;
  record();
  // remove first, then fix the destination path if it followed the element removed
  element(parent(from))->removeChild(unsigned(from.back()));
  Path parentFrom = parent(from);
  if (destination.size() > parentFrom.size() && std::equal(parentFrom.begin(), parentFrom.end(), destination.begin())
      && destination[parentFrom.size()] > from.back())
    destination[parentFrom.size()]--;
  if (destination == parentFrom && index > from.back()) index--;
  XmlElement * d = element(destination);
  if (!d) { changed(); return Path(); }
  d->insertChild(unsigned(std::min(index, int(d->nChildren()))), moved);
  changed();
  ok = true;
  Path result = destination;
  result.push_back(std::min(index, int(d->nChildren()) - 1));
  return result;
  }

  String PlotDocument::copy(const Path & path) const
  {
  const XmlElement * e = element(path);
  return e ? e->toString() : String("");
  }

  PlotDocument::Path PlotDocument::paste(const String & text, const Path & target, bool & ok)
  {
  ok = false;
  XmlElement e;
  try { e = XmlDocument::parse(text).root(); }
  catch (...) { return Path(); }
  if (e.name().isNull()) return Path();
  if (e.isNamed("Canvas"))
    {
    String name = uniqueCanvasName(e.attribute("name", "canvas"));
    e.setAttribute("name", name);
    if (e.hasAttribute("file")) e.setAttribute("file", name + String(".pdf"));
    }
  Path destination;
  int index = -1;
  Path p = target;
  if (element(p) && canContain(kind(p), e.name())) { destination = p; index = int(element(p)->nChildren()); }
  else
    {
    while (!p.empty())
      {
      Path up = parent(p);
      if (canContain(kind(up), e.name())) { destination = up; index = p.back() + 1; break; }
      p = up;
      }
    if (index < 0)
      {
      if (!canContain(kRoot, e.name())) return Path();
      destination = Path();
      index = int(root().nChildren());
      }
    }
  record();
  element(destination)->insertChild(unsigned(index), e);
  changed();
  ok = true;
  destination.push_back(index);
  return destination;
  }

  void PlotDocument::mergeDifferences(XmlElement & target, const XmlElement & full, const XmlElement & def)
  {
  for (const auto & a : full.attributes())
    {
    if (a.first == "which") continue;
    bool same = def.hasAttribute(a.first) && def.attribute(a.first) == a.second;
    if (same) target.removeAttribute(a.first); else target.setAttribute(a.first, a.second);
    }
  for (const XmlElement * c : full.children())
    {
    String which = c->attribute("which", "");
    auto matches = [&](const XmlElement & e) { return e.isNamed(c->name()) && e.attribute("which", "") == which; };
    const XmlElement * d = nullptr;
    for (const XmlElement * x : def.children()) if (matches(*x)) { d = x; break; }
    XmlElement empty(c->name());
    int index = -1;
    for (unsigned k=0; k<target.nChildren(); k++) if (matches(target.child(k))) { index = int(k); break; }
    XmlElement child = index >= 0 ? target.child(unsigned(index)) : XmlElement(c->name());
    if (!which.isNull()) child.setAttribute("which", which);
    mergeDifferences(child, *c, d ? *d : empty);
    bool meaningful = child.hasChildren();
    for (const auto & a : child.attributes()) if (!(a.first == "which")) meaningful = true;
    if (index >= 0)
      {
      if (meaningful) target.child(unsigned(index)) = child; else target.removeChild(unsigned(index));
      }
    else if (meaningful)
      {
      unsigned position = 0;
      while (position < target.nChildren() && kind(target.child(position)) == kStyle) position++;
      target.insertChild(position, child);
      }
    }
  }


  namespace
  {
  bool isItem(const XmlElement & e)
  {
  return e.isNamed("Histogram") || e.isNamed("Data") || e.isNamed("Profile") || e.isNamed("Function");
  }

  std::string itemKey(const XmlElement & e)
  {
  if (e.isNamed("Function"))
    return "F|" + e.attribute("class", "").str() + "|" + e.attribute("parameters", "").str();
  return "H|" + e.attribute("input", "").str() + "|" + e.attribute("path", "").str();
  }

  std::vector<const XmlElement *> items(const XmlElement & graph)
  {
  std::vector<const XmlElement *> out;
  for (const XmlElement * c : graph.children()) if (isItem(*c)) out.push_back(c);
  return out;
  }

  bool meaningful(const XmlElement & e)
  {
  if (e.hasChildren()) return true;
  for (const auto & a : e.attributes()) if (!(a.first == "which")) return true;
  return false;
  }
  } // namespace

  bool PlotDocument::applyChanges(XmlElement & target, const XmlElement & before, const XmlElement & after)
  {
  bool changed = false;
  for (const auto & a : after.attributes())
    {
    if (before.hasAttribute(a.first) && before.attribute(a.first) == a.second) continue;
    if (target.hasAttribute(a.first) && target.attribute(a.first) == a.second) continue;
    target.setAttribute(a.first, a.second);
    changed = true;
    }
  for (const auto & a : before.attributes())
    if (!after.hasAttribute(a.first) && target.hasAttribute(a.first)) { target.removeAttribute(a.first); changed = true; }
  for (const XmlElement * c : after.children())
    {
    if (isItem(*c) || c->isNamed("Graph") || c->isNamed("Canvas") || c->isNamed("Window")) continue;
    String which = c->attribute("which", "");
    auto matches = [&](const XmlElement & e) { return e.isNamed(c->name()) && e.attribute("which", "") == which; };
    const XmlElement * b = nullptr;
    for (const XmlElement * x : before.children()) if (matches(*x)) { b = x; break; }
    if (b && b->toString() == c->toString()) continue;
    XmlElement empty(c->name());
    int index = -1;
    for (unsigned k=0; k<target.nChildren(); k++) if (matches(target.child(k))) { index = int(k); break; }
    XmlElement child = index >= 0 ? target.child(unsigned(index)) : XmlElement(c->name());
    if (!which.isNull()) child.setAttribute("which", which);
    if (!applyChanges(child, b ? *b : empty, *c) && index >= 0) continue;
    if (index >= 0)
      {
      if (meaningful(child)) target.child(unsigned(index)) = child; else target.removeChild(unsigned(index));
      changed = true;
      }
    else if (meaningful(child))
      {
      unsigned position = 0;
      while (position < target.nChildren() && kind(target.child(position)) == kStyle) position++;
      target.insertChild(position, child);
      changed = true;
      }
    }
  return changed;
  }

  bool PlotDocument::syncCanvas(XmlElement & canvas, const XmlElement & before, const XmlElement & after)
  {
  String original = canvas.toString();
  applyChanges(canvas, before, after);
  // the graphs of the element by cell (as PlotSpec::build places them)
  std::map<int, unsigned> targetGraphs;
  int next = 0;
  for (unsigned k=0; k<canvas.nChildren(); k++)
    {
    const XmlElement & g = canvas.child(k);
    if (!g.isNamed("Graph")) continue;
    int cell = g.hasAttribute("cell") ? int(g.attributeInt("cell")) : next;
    next = cell + 1;
    targetGraphs[cell] = k;
    }
  auto cellOf = [](const XmlElement & g) { return int(g.attributeInt("cell", 0)); };
  for (const XmlElement * a : after.children("Graph"))
    {
    int cell = cellOf(*a);
    const XmlElement * b = nullptr;
    for (const XmlElement * x : before.children("Graph")) if (cellOf(*x) == cell) { b = x; break; }
    std::vector<const XmlElement *> afterItems = items(*a);
    auto found = targetGraphs.find(cell);
    if (found == targetGraphs.end())
      {
      if (afterItems.empty()) continue;
      XmlElement g("Graph");
      g.setAttribute("cell", cell);
      canvas.addChild(g);
      targetGraphs[cell] = canvas.nChildren() - 1;
      found = targetGraphs.find(cell);
      }
    XmlElement graph = canvas.child(found->second);
    XmlElement emptyGraph("Graph");
    applyChanges(graph, b ? *b : emptyGraph, *a);
    // items: matched by identity to the items as drawn, and these to the items of the element (in order)
    std::vector<const XmlElement *> beforeItems = b ? items(*b) : std::vector<const XmlElement *>();
    std::vector<const XmlElement *> targetItems = items(graph);
    std::vector<XmlElement> kept;
    std::vector<bool> usedBefore(beforeItems.size(), false), usedTarget(targetItems.size(), false);
    bool aligned = beforeItems.size() == targetItems.size();
    for (const XmlElement * ai : afterItems)
      {
      int match = -1;
      for (std::size_t i=0; i<beforeItems.size(); i++)
        if (!usedBefore[i] && itemKey(*beforeItems[i]) == itemKey(*ai)) { match = int(i); break; }
      int t = -1;
      if (match >= 0)
        {
        usedBefore[std::size_t(match)] = true;
        if (aligned) t = match;
        else
          for (std::size_t j=0; j<targetItems.size(); j++)
            if (!usedTarget[j] && itemKey(*targetItems[j]) == itemKey(*ai)) { t = int(j); break; }
        }
      if (t >= 0 && !usedTarget[std::size_t(t)])
        {
        usedTarget[std::size_t(t)] = true;
        XmlElement item = *targetItems[std::size_t(t)];
        applyChanges(item, *beforeItems[std::size_t(match)], *ai);
        kept.push_back(item);
        }
      else
        {
        // an item added on screen: what identifies it, and its colors and styles
        XmlElement item(ai->name());
        for (const char * n : { "input", "path", "label", "class", "parameters", "xMin", "xMax", "yMin", "yMax" })
          if (ai->hasAttribute(n)) item.setAttribute(n, ai->attribute(n));
        for (const char * n : { "Line", "Marker" })
          if (const XmlElement * c = ai->child(n))
            {
            XmlElement & s = item.addChild(n);
            for (const char * k : { "color", "style" }) if (c->hasAttribute(k)) s.setAttribute(k, c->attribute(k));
            }
        kept.push_back(item);
        }
      }
    // items of the element not drawn (e.g. their histograms not found) stay
    for (std::size_t j=0; j<targetItems.size(); j++)
      {
      if (usedTarget[j]) continue;
      bool drawn = false;
      for (std::size_t i=0; i<beforeItems.size(); i++) if (itemKey(*beforeItems[i]) == itemKey(*targetItems[j])) drawn = true;
      if (!drawn) kept.push_back(*targetItems[j]);
      }
    XmlElement rebuilt = graph;
    rebuilt.clear();
    rebuilt.setName("Graph");
    for (const auto & at : graph.attributes()) rebuilt.setAttribute(at.first, at.second);
    for (const XmlElement * c : graph.children()) if (!isItem(*c)) rebuilt.addChild(*c);
    for (const XmlElement & item : kept) rebuilt.addChild(item);
    canvas.child(found->second) = rebuilt;
    }
  return !(canvas.toString() == original);
  }

  } // namespace GRAPHICS
} // namespace CAP
