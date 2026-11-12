/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PlotDocument.hpp
//! @brief PlotDocument: a PlotHistos XML file being edited (PlotEditor): its tree of inputs, outputs, windows,
//! canvases, graphs, histograms and functions, the edits that keep it valid, and their undo and redo.
#ifndef CAP__PlotDocument
#define CAP__PlotDocument
#include <deque>
#include <vector>
#include "String.hpp"
#include "XmlDocument.hpp"
#include "XmlElement.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! @brief A PlotHistos file (see PlotSpec) being edited.
  //!
  //! The document is the XML tree of the file. Its elements are reached by their paths, the indices of the children
  //! from the root down (paths stay valid across undo and redo, pointers do not). The edits keep the structure of the
  //! file: Input and Output elements and windows at the top, canvases in windows, graphs in canvases, histograms and
  //! functions in graphs; the elements of style (Margins, Axis, Marker, Line, ...) in the elements they style.
  //!
  //! Data are added where they are dropped (addData()): on a graph, as one more histogram of it; on a histogram or
  //! function, after it in its graph; on a canvas, as a new graph; on a window, as a new canvas holding one graph; on
  //! the top of the document, in a new canvas of the last window (a new window if there is none). The input file of
  //! the data is added to the inputs if needed (input()).
  //!
  //! Every edit records the state before it (undo(), redo()); setAttribute() and the others are edits too. A group
  //! of changes is one step of undo when made between beginEdit() and endEdit().
  //!
  class PlotDocument
  {
  public:
  //! Kinds of elements
  enum Kind { kRoot, kInput, kOutput, kWindow, kCanvas, kGraph, kHistogram, kFunction, kStyle, kOther };
  //! Path of an element: the indices of the children from the root down (empty: the root)
  using Path = std::vector<int>;

  //! @brief A new document: one output and no window.
  PlotDocument();

  //! @name File
  //! @{
  //! @brief Read a file (the comments before its root element are kept; those inside it are not).
  //! @param fileName XML file @param error set to the reason of a failure @return false on failure
  bool read(const String & fileName, String & error);
  //!
  //! @brief Write the document to a file. Written in another directory, its relative paths (inputs, output
  //! directory) are rewritten relative to the new one, so that they still point to the same files.
  //! @param fileName file @param error set to the reason of a failure @return false on failure
  //!
  bool write(const String & fileName, String & error);
  //! @brief File of the document ("" for a new document). @return path
  const String & fileName() const { return _fileName; }
  //! @brief Directory of the relative paths: that of the file (the current directory for a new document). @return directory
  String directory() const;
  //! @brief Whether the document changed since it was read or written. @return true if so
  bool isDirty() const { return _dirty; }
  //! @brief Set the document unchanged or changed. @param v flag
  void setDirty(bool v) { _dirty = v; }
  //! @brief Name shown for the document (the name of its file, or the given name of a new document). @return name
  String displayName() const;
  //! @brief Name of a new document (e.g. "Untitled 2"). @param name name
  void setUntitledName(const String & name) { _untitled = name; }
  //! @brief Number of changes made (to redraw what depends on the document). @return number
  unsigned long version() const { return _version; }
  //! @}

  //! @name Tree
  //! @{
  XmlDocument & document() { return _document; }   //!< The XML document
  const XmlElement & root() const { return _document.root(); }   //!< Root element (PlotHistos)
  //! @brief Element of a path. @param path path @return element, or null if the path is not valid
  XmlElement * element(const Path & path);
  //! @brief Element of a path. @param path path @return element, or null
  const XmlElement * element(const Path & path) const;
  //! @brief Kind of an element by its name and place. @param e element @return kind
  static Kind kind(const XmlElement & e);
  //! @brief Kind of the element of a path. @param path path @return kind (kOther if the path is not valid)
  Kind kind(const Path & path) const;
  //! @brief Name of a kind, e.g. "Canvas". @param kind kind @return name
  static const char * kindName(Kind kind);
  //! @brief Short description of an element, as shown in a tree, e.g. "Spectra  (2 x 1)". @param e element @return text
  static String label(const XmlElement & e);
  //! @brief Whether an element of a kind can hold an element of a name (e.g. a Canvas holds Graph and Background).
  //! @param parent kind of the parent @param child name of the child @return true if so
  static bool canContain(Kind parent, const String & child);
  //! @brief Parent path. @param path path (not empty) @return path of the parent
  static Path parent(const Path & path) { return Path(path.begin(), path.end() - (path.empty() ? 0 : 1)); }
  //! @brief Path of the closest element of a kind holding (or being) the element of a path. @param path path
  //! @param kind kind @return path, or an empty path with ok false
  Path enclosing(const Path & path, Kind kind, bool & ok) const;
  //! @brief Paths of the windows, of the canvases of a window, ... : the children of an element of a kind.
  //! @param parent path of the parent @param kind kind @return paths
  std::vector<Path> children(const Path & parent, Kind kind) const;
  //! @brief Index of the window and of the canvas in it holding the element of a path (for the preview).
  //! @param path path @param window set to the index of the window among the windows @param canvas set to the index
  //! of the canvas among the canvases of the window (-1 for the window itself) @return false if the element is not
  //! in a window
  bool locate(const Path & path, int & window, int & canvas) const;
  //! @}

  //! @name Edits (each is a step of undo)
  //! @{
  //! @brief Set an attribute of an element (removed if the value is empty). @param path element @param name name
  //! @param value value @return false if the path is not valid or nothing changed
  bool setAttribute(const Path & path, const String & name, const String & value);
  //! @brief Set an attribute of a child of style of an element (e.g. the color of the Marker of a histogram), the
  //! child made if needed (and removed when it has nothing left). @param path element @param child name of the child
  //! @param name attribute @param value value ("" removes it) @param which for an Axis child, which axis (x, y, z)
  //! @return false if nothing changed
  bool setStyle(const Path & path, const String & child, const String & name, const String & value,
                const String & which="");
  //! @brief Value of an attribute of a child of style ("" if not set). @param path element @param child child
  //! @param name attribute @param which axis for an Axis child @return value
  String style(const Path & path, const String & child, const String & name, const String & which="") const;
  //! @brief Replace an element by another (e.g. edited as a whole). @param path element @param e new element
  //! @return false if the path is not valid
  bool replace(const Path & path, const XmlElement & e);
  //! @brief Add a window (at the end). @param title title @return its path
  Path addWindow(const String & title="");
  //! @brief Add a canvas to a window. @param window path of a window @param name name ("" for a unique one)
  //! @return its path
  Path addCanvas(const Path & window, const String & name="");
  //! @brief Add a graph to a canvas. @param canvas path of a canvas @return its path
  Path addGraph(const Path & canvas);
  //! @brief Add a function to a graph. @param graph path of a graph @param className function class (e.g. Normal)
  //! @param parameters its parameters @param xMin lower end @param xMax upper end @return its path
  Path addFunction(const Path & graph, const String & className="Normal", const String & parameters="1, 0, 1",
                   double xMin=-3.0, double xMax=3.0);
  //! @brief Add an input (or find it). @param file path of the CAP file @return its identifier
  String input(const String & file);
  //!
  //! @brief Add data points of an input file where dropped (see the class description). @param target path of the
  //! element dropped on @param file CAP file @param objects paths of the objects in the file (several: a folder) —
  //! overlaid in one graph when dropped on a graph or an item, one graph each otherwise
  //! @return path of the first element added (histogram, graph or canvas)
  //!
  Path addData(const Path & target, const String & file, const std::vector<String> & objects);
  //! @brief Remove an element (not the root). @param path path @return false if it cannot be removed
  bool remove(const Path & path);
  //! @brief Copy an element after itself (a canvas under a new name, a window with its canvases under new names).
  //! @param path path @return path of the copy
  Path duplicate(const Path & path);
  //!
  //! @brief Move an element into another, at an index (or into the parent of the target, after it, when the target
  //! cannot hold it: dropping a histogram on a histogram puts it after that one). @param from element moved
  //! @param target element dropped on @return new path, or an empty path with ok false if it cannot go there
  //!
  Path move(const Path & from, const Path & target, bool & ok);
  //! @brief Move an element among its siblings. @param path path @param step -1 up, +1 down @return new path
  Path shift(const Path & path, int step);
  //! @brief XML text of an element (what Copy puts in the clipboard). @param path path @return text
  String copy(const Path & path) const;
  //!
  //! @brief Paste the XML text of an element: into the target if it can hold it, otherwise after the target in its
  //! parent, otherwise into the closest element that can hold it. @param text XML of one element @param target
  //! selected element @param ok set to false if it cannot be pasted there @return path of the element added
  //!
  Path paste(const String & text, const Path & target, bool & ok);
  //! @brief Start a group of changes undone in one step.
  void beginEdit();
  //! @brief End a group of changes.
  void endEdit();
  //! @}

  //! @name Undo
  //! @{
  bool canUndo() const { return !_undo.empty(); }   //!< Whether an edit can be undone
  bool canRedo() const { return !_redo.empty(); }   //!< Whether an edit can be redone
  bool undo();   //!< Undo the last edit @return false if there is none
  bool redo();   //!< Redo the edit undone last @return false if there is none
  //! @}

  //! @brief Unique name of a new canvas, from a wanted name. @param wanted wanted name @return name
  String uniqueCanvasName(const String & wanted) const;
  //!
  //! @brief Write to an element the settings of a full description that differ from the defaults (e.g. a graph edited
  //! as an object, written with GraphXml::writeGraph(), against a default graph written the same way): attributes
  //! equal to their defaults are removed, the others set; children (matched by name and "which") likewise, made or
  //! removed as needed; what the full description does not give is kept. @param target element changed
  //! @param full full description @param defaults description of the defaults
  //!
  static void mergeDifferences(XmlElement & target, const XmlElement & full, const XmlElement & defaults);
  //!
  //! @brief Write to an element the changes from one description to another (e.g. of a canvas before and after an
  //! edit on screen): attributes changed or added are set, attributes removed are removed, children of style
  //! (matched by name and "which") likewise; what did not change is left as it is, so that the element keeps its
  //! own, compact, form. Graphs and items are not touched (see syncCanvas()). @param target element changed
  //! @param before description before @param after description after @return true if the element changed
  //!
  static bool applyChanges(XmlElement & target, const XmlElement & before, const XmlElement & after);
  //!
  //! @brief Record in a Canvas element of the file the changes made to its canvas on screen. before and after are
  //! full descriptions of the canvas (PlotSpec::toXml()) when it was drawn from the element and now: the changes of the
  //! canvas, of each graph (matched by cell) and of each item (matched by input and path, or class and parameters) are
  //! written; items added on screen are added, items removed are removed. @param canvas Canvas element
  //! @param before canvas as drawn @param after canvas now @return true if the element changed
  //!
  static bool syncCanvas(XmlElement & canvas, const XmlElement & before, const XmlElement & after);
  //! @brief Path relative to a directory if it is below it (or near it: "../a/b"), absolute otherwise.
  //! @param path absolute path @param directory directory @return path
  static String relativePath(const String & path, const String & directory);

  private:
  XmlDocument _document;   //!< the file
  String _fileName;   //!< file read or written last
  String _untitled = "Untitled";   //!< name of a new document
  std::string _prolog;   //!< comments before the root element of the file read (written back)
  bool _dirty = false;   //!< changed since read or written
  unsigned long _version = 0;   //!< number of changes
  int _group = 0;   //!< depth of beginEdit()
  bool _groupRecorded = false;   //!< the state before the group was recorded
  std::deque<XmlElement> _undo;   //!< states before the last edits
  std::deque<XmlElement> _redo;   //!< states undone
  //! @brief Record the state before an edit (once per group).
  void record();
  //! @brief Mark a change.
  void changed() { _dirty = true; _version++; }
  //! @brief The canvas names of the document. @return names
  std::vector<String> canvasNames() const;
  //! @brief A histogram element for an object. @param input input id @param object path in the file @return element
  static XmlElement histogram(const String & input, const String & object);
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__PlotDocument
