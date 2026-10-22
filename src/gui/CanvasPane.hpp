/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CanvasPane.hpp
//! @brief CanvasPane: canvas pane of the CapGui window, with its toolbar and mouse interactions.
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

  //! @brief What showing an object does to the selected graph: add it, or replace its contents
  enum Mode { kAdd, kReplace };

  //! @brief Constructor: one graph. @param name name of the pane, unique in the window (e.g. "Canvas 1")
  CanvasPane(const String & name);

  GRAPHICS::Canvas & canvas() { return _canvas; }   //!< The canvas of the pane
  const GRAPHICS::Canvas & canvas() const { return _canvas; }   //!< The canvas of the pane
  const String & name() const { return _name; }   //!< Name of the pane (identifies its window)

  Mode mode() const { return _mode; }   //!< Add or Replace
  void setMode(Mode mode) { _mode = mode; }   //!< Set Add or Replace
  //! @brief Index of the selected graph (the target of Add/Replace, Log X/Y, Copy/Paste).
  int  selectedGraph() const { return _selectedGraph; }
  //! @brief Select graph k (nothing if out of range). @param k index of the graph
  void setSelectedGraph(int k);
  GRAPHICS::Graph & currentGraph() { return _canvas.graph(_selectedGraph); }   //!< The selected graph

  //! @brief Show an item in the selected graph, added or replacing the contents according to mode(). @return item
  GRAPHICS::GraphItem & show(const GRAPHICS::GraphItem & item);
  //! @brief Divide the canvas into nColumns x nRows graphs (keeps the existing graphs, see Canvas::divide()).
  void divide(int nColumns, int nRows);

  bool showToolbar() const { return _showToolbar; }   //!< Whether the toolbar is shown
  void setShowToolbar(bool v) { _showToolbar = v; }   //!< Show or hide the toolbar

  //! @brief Draw the pane (toolbar and canvas) in the current ImGui window. @param width width available
  //! @param height height available @param isCurrent whether this pane is the current pane (highlighted)
  //! @param font font of the plots
  //! @return true if the pane was clicked (it should become the current pane)
  bool render(float width, float height, bool isCurrent, ImFont * font);

  //! @brief Position under the mouse in data coordinates, or empty. @return text
  const String & hoverText() const { return _hoverText; }
  //! @brief Screen rectangle of the canvas at the last render (pixels). @param x left edge
  //! @param y top edge @param w width @param h height
  void canvasRect(float & x, float & y, float & w, float & h) const { x = _rectX; y = _rectY; w = _rectW; h = _rectH; }

  //! Called before the pane changes its canvas (undo).
  std::function<void()> beforeChange;
  //! Called when an object of the file browser is dropped on graph k of the pane.
  std::function<void(int graph, const ObjectReference & object)> onDrop;
  //! Called by the + button of the toolbar (new canvas pane).
  std::function<void()> onAddPane;
  //! Called to show the values of data points shown in a graph as a table (graph menu, Show Data).
  std::function<void(const MATH::DataPoints * data, const String & label)> onShowData;
  //! Called by "Fit..." in the context menu of a graph (made the selected graph), with the index of the item to fit (-1: any).
  std::function<void(int item)> onFit;
  //! Called by "Residual Map" in the context menu of a graph, for fit k of data points of rank 2.
  std::function<void(const MATH::DataPoints * data, unsigned int fit)> onResidualMap;
  //! Called by "Projections" in the context menu of a graph, for a histogram of rank 2 with fits.
  std::function<void(const MATH::DataPoints * data)> onProjections;

  protected:

  String _name;   //!< name of the pane
  GRAPHICS::Canvas _canvas;   //!< the canvas, divided into graphs
  Mode _mode;   //!< Add or Replace
  int  _selectedGraph;   //!< index of the selected graph
  bool _showToolbar;   //!< the toolbar is shown
  int  _contextGraph;   //!< graph whose context menu is open
  int  _gridColumns;   //!< values edited in the Graphs popup
  int  _gridRows;   //!< values edited in the Graphs popup
  String _hoverText;   //!< data coordinates under the mouse, or empty
  float _rectX;   //!< screen rectangle of the canvas at the last render (pixels)
  float _rectY;   //!< screen rectangle of the canvas at the last render (pixels)
  float _rectW;   //!< screen rectangle of the canvas at the last render (pixels)
  float _rectH;   //!< screen rectangle of the canvas at the last render (pixels)

  //! What a drag of the left mouse button does.
  enum DragKind { kNoDrag, kZoom, kZoomX, kZoomY, kMoveTitle, kMoveAxisTitle, kMoveLegend, kMoveLabels, kRotate };
  //! State of a drag of the left mouse button
  struct Drag
  {
  DragKind kind = kNoDrag;   //!< what the drag does
  int   graph = -1;   //!< graph dragged in
  int   axis  = -1;   //!< axis zoomed, or whose title or labels are moved
  float startX = 0;      //!< screen position where the drag started (pixels)
  float startY = 0;      //!< screen position where the drag started (pixels)
  float initialX = 0;  //!< position (offset, legend corner, label gap) before the drag
  float initialY = 0;  //!< position (offset, legend corner, label gap) before the drag
  bool  moved = false;   //!< the mouse moved enough to start the drag
  };
  Drag _drag;   //!< drag in progress (kind kNoDrag if none)
  int  _editGraph;                     //!< graph whose part is edited in the "editor" popup
  GRAPHICS::Graph::Pick _editPick;     //!< part edited
  bool _editRecorded;                  //!< undo state recorded for the current dialog
  bool _openEditor;                    //!< open the editor popup at the next opportunity
  GRAPHICS::Graph _editBefore;         //!< graph when the dialog was opened (for undo)

  void changing() { if (beforeChange) beforeChange(); }   //!< Record the undo state before a change
  //! @brief Draw the toolbar: Add/Replace, Log X/Y, Graphs, Clear, +.
  void renderToolbar();
  //! @brief Draw the context menu of a graph, if open.
  void renderContextMenu();
  //! @brief Draw the dialog editing a part of a graph, if open (see GraphEditor).
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
