#ifndef CAP__GUI__CanvasPane
#define CAP__GUI__CanvasPane
#include <functional>
#include "Canvas.hpp"
#include "FileBrowser.hpp"

struct ImFont;

namespace CAP
{
  namespace GUI
  {

  //!
  //! @brief One canvas pane of the CapGui window: a GRAPHICS::Canvas (divided into graphs) with a toolbar.
  //!
  //! Toolbar: Add / Replace (what showing an object does to the selected graph), Log X, Log Y (scales of the
  //! selected graph), Graphs (divide the canvas into n' x m' graphs), Clear (empty the selected graph).
  //! Clicking a graph selects it (blue frame); right-clicking opens its context menu; the mouse position is reported
  //! in data coordinates (hoverText()). Objects dragged from the file browser can be dropped on any graph (onDrop):
  //! the graph under the mouse is highlighted while dragging.
  //!
  //! beforeChange is called before any change made from the pane, so that the application can record an undo state.
  //!
  class CanvasPane
  {
  public:

  enum Mode { kAdd, kReplace };

  CanvasPane(const String & name);

  GRAPHICS::Canvas & canvas() { return _canvas; }
  const GRAPHICS::Canvas & canvas() const { return _canvas; }
  const String & name() const { return _name; }

  Mode mode() const { return _mode; }
  void setMode(Mode mode) { _mode = mode; }
  //! @brief Index of the selected graph (the target of Add/Replace, Log X/Y, Copy/Paste).
  int  selectedGraph() const { return _selectedGraph; }
  void setSelectedGraph(int k);
  GRAPHICS::Graph & currentGraph() { return _canvas.graph(_selectedGraph); }

  //! @brief Show an item in the selected graph, added or replacing the contents according to mode(). @return item
  GRAPHICS::GraphItem & show(const GRAPHICS::GraphItem & item);
  //! @brief Divide the canvas into nColumns x nRows graphs (keeps the existing graphs, see Canvas::divide()).
  void divide(int nColumns, int nRows);

  bool showToolbar() const { return _showToolbar; }
  void setShowToolbar(bool v) { _showToolbar = v; }

  //! @brief Draw the pane (toolbar and canvas) in the current ImGui window. @param size size available
  //! @param isCurrent whether this pane is the current pane (highlighted) @param font font of the plots
  //! @return true if the pane was clicked (it should become the current pane)
  bool render(float width, float height, bool isCurrent, ImFont * font);

  //! @brief Position under the mouse in data coordinates, or empty. @return text
  const String & hoverText() const { return _hoverText; }
  //! @brief Screen rectangle of the canvas at the last render (pixels). @param x @param y @param w @param h
  void canvasRect(float & x, float & y, float & w, float & h) const { x = _rectX; y = _rectY; w = _rectW; h = _rectH; }

  //! Called before the pane changes its canvas (undo).
  std::function<void()> beforeChange;
  //! Called when an object of the file browser is dropped on graph k of the pane.
  std::function<void(int graph, const ObjectReference & object)> onDrop;

  protected:

  String _name;
  GRAPHICS::Canvas _canvas;
  Mode _mode;
  int  _selectedGraph;
  bool _showToolbar;
  int  _contextGraph;
  int  _gridColumns, _gridRows;
  String _hoverText;
  float _rectX, _rectY, _rectW, _rectH;

  void changing() { if (beforeChange) beforeChange(); }
  void renderToolbar();
  void renderContextMenu();
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__CanvasPane
