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

  class ImGuiPainter;

  //!
  //! @brief One canvas pane of the CapGui window: a GRAPHICS::Canvas (divided into graphs) with a toolbar.
  //!
  //! Toolbar: Add / Replace (what showing an object does to the selected graph), Log X, Log Y (scales of the
  //! selected graph), Graphs (divide the canvas into n' x m' graphs), Clear (empty the selected graph), + (new pane).
  //! Clicking a graph selects it (blue frame); the mouse position is reported in data coordinates (hoverText()).
  //! Objects dragged from the file browser can be dropped on any graph (onDrop): the graph under the mouse is
  //! highlighted while dragging.
  //!
  //! Mouse on a graph:
  //!  - drag in the frame: zoom on the rectangle; drag along an axis: zoom on that axis; double-click: automatic ranges
  //!  - drag the title, an axis title or the legend: move it; Shift + drag an axis: move its labels
  //!  - right-click the title, an axis, an axis title, the legend or a data point: dialog editing it (GraphEditor);
  //!    elsewhere: context menu of the graph
  //!  - Escape cancels a drag
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
  //! Called by the + button of the toolbar (new canvas pane).
  std::function<void()> onAddPane;
  //! Called to show the values of data points shown in a graph as a table (graph menu, Show Data).
  std::function<void(const MATH::DataPoints * data, const String & label)> onShowData;

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

  //! What a drag of the left mouse button does.
  enum DragKind { kNoDrag, kZoom, kZoomX, kZoomY, kMoveTitle, kMoveAxisTitle, kMoveLegend, kMoveLabels };
  struct Drag
  {
  DragKind kind = kNoDrag;
  int   graph = -1;
  int   axis  = -1;
  float startX = 0, startY = 0;      //!< screen position where the drag started (pixels)
  float initialX = 0, initialY = 0;  //!< position (offset, legend corner, label gap) before the drag
  bool  moved = false;
  };
  Drag _drag;
  int  _editGraph;                     //!< graph whose part is edited in the "editor" popup
  GRAPHICS::Graph::Pick _editPick;     //!< part edited
  bool _editRecorded;                  //!< undo state recorded for the current dialog
  bool _openEditor;                    //!< open the editor popup at the next opportunity
  GRAPHICS::Graph _editBefore;         //!< graph when the dialog was opened (for undo)

  void changing() { if (beforeChange) beforeChange(); }
  void renderToolbar();
  void renderContextMenu();
  void renderEditor();
  //! Open the dialog editing the part pick of graph k.
  void edit(int k, const GRAPHICS::Graph::Pick & pick);
  //! Mouse buttons on the canvas: drags (zoom, move), double-click, right-click. @return true if the pane was clicked
  bool handleMouse(ImGuiPainter & painter, bool hovered);
  //! Apply the move of the current drag (screen displacement dx, dy in pixels).
  void applyMove(float dx, float dy);
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__CanvasPane
