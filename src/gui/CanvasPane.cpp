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
//! @file CanvasPane.cpp
//! @brief Implementation of CanvasPane.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "imgui.h"
#include "CanvasPane.hpp"
#include "ImGuiPainter.hpp"
#include "GraphEditor.hpp"
#include "DataPoints.hpp"

namespace CAP
{
  namespace GUI
  {
  using namespace GRAPHICS;

  CanvasPane::CanvasPane(const String & name)
  :
  beforeChange(),
  onDrop(),
  onAddPane(),
  onShowData(),
  _name(name),
  _canvas(name, name),
  _mode(kAdd),
  _selectedGraph(0),
  _showToolbar(true),
  _contextGraph(-1),
  _gridColumns(1),
  _gridRows(1),
  _hoverText(),
  _rectX(0), _rectY(0), _rectW(0), _rectH(0),
  _drag(),
  _editGraph(-1),
  _editPick(),
  _editRecorded(false),
  _openEditor(false),
  _editBefore()
  {
  _canvas.divide(1,1);
  }

  void CanvasPane::setSelectedGraph(int k)
  {
  if (k >= 0 && k < _canvas.nGraphs()) _selectedGraph = k;
  }

  GraphItem & CanvasPane::show(const GraphItem & item)
  {
  changing();
  Graph & graph = currentGraph();
  return _mode == kAdd ? graph.add(item) : graph.replace(item);
  }

  void CanvasPane::divide(int nColumns, int nRows)
  {
  _canvas.divide(nColumns, nRows);
  _gridColumns = _canvas.nColumns();
  _gridRows    = _canvas.nRows();
  if (_selectedGraph >= _canvas.nGraphs()) _selectedGraph = 0;
  }

  void CanvasPane::renderToolbar()
  {
  ImGui::PushID("toolbar");
  int mode = int(_mode);
  ImGui::RadioButton("Add", &mode, kAdd);
  ImGui::SetItemTooltip("Objects shown are added to the selected graph");
  ImGui::SameLine();
  ImGui::RadioButton("Replace", &mode, kReplace);
  ImGui::SetItemTooltip("Objects shown replace the contents of the selected graph");
  _mode = Mode(mode);

  Graph & graph = currentGraph();
  ImGui::SameLine(0, 18);
  bool logX = graph.isLogX(), logY = graph.isLogY();
  if (ImGui::Checkbox("Log X", &logX)) { changing(); graph.setLogX(logX); }
  ImGui::SetItemTooltip("Logarithmic x axis for the selected graph");
  ImGui::SameLine();
  if (ImGui::Checkbox("Log Y", &logY)) { changing(); graph.setLogY(logY); }
  ImGui::SetItemTooltip("Logarithmic y axis for the selected graph");

  ImGui::SameLine(0, 18);
  if (ImGui::Button("Graphs...")) { _gridColumns = _canvas.nColumns(); _gridRows = _canvas.nRows(); ImGui::OpenPopup("graphs"); }
  ImGui::SetItemTooltip("Divide this canvas into n' x m' graphs");
  if (ImGui::BeginPopup("graphs"))
    {
    ImGui::Text("Graphs in this canvas");
    ImGui::SetNextItemWidth(110);
    ImGui::InputInt("columns", &_gridColumns);
    ImGui::SetNextItemWidth(110);
    ImGui::InputInt("rows", &_gridRows);
    _gridColumns = _gridColumns < 1 ? 1 : (_gridColumns > 8 ? 8 : _gridColumns);
    _gridRows    = _gridRows    < 1 ? 1 : (_gridRows    > 8 ? 8 : _gridRows);
    if (ImGui::Button("Apply")) { changing(); divide(_gridColumns, _gridRows); ImGui::CloseCurrentPopup(); }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    }
  ImGui::SameLine();
  if (ImGui::Button("Clear")) { changing(); graph.clearItems(); }
  ImGui::SetItemTooltip("Remove everything from the selected graph");
  if (_canvas.nGraphs() > 1)
    {
    ImGui::SameLine(0, 18);
    ImGui::TextDisabled("graph %d of %d", _selectedGraph+1, _canvas.nGraphs());
    }
  if (onAddPane)
    {
    float buttonWidth = ImGui::CalcTextSize("+").x + 2*ImGui::GetStyle().FramePadding.x;
    ImGui::SameLine(0, 18);
    float available = ImGui::GetContentRegionAvail().x;   // right-aligned
    if (available > buttonWidth) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available - buttonWidth);
    if (ImGui::Button("+")) onAddPane();
    ImGui::SetItemTooltip("New canvas pane, beside this one (close a pane with the x of its tab)");
    }
  ImGui::PopID();
  }

  void CanvasPane::renderContextMenu()
  {
  if (!ImGui::BeginPopup("graphContext")) return;
  if (_contextGraph < 0 || _contextGraph >= _canvas.nGraphs()) { ImGui::EndPopup(); return; }
  Graph & graph = _canvas.graph(_contextGraph);
  ImGui::TextDisabled("%s", graph.title().isNull() ? "Graph" : graph.title().data());
  ImGui::Separator();
  Graph::Pick pick;
  if (ImGui::MenuItem("Edit Title..."))  { pick.part = Graph::kTitle;  edit(_contextGraph, pick); }
  for (int k=0; k<graph.nAxes(); k++)
    {
    char label[48];
    std::snprintf(label, sizeof(label), "Edit %s Axis...", k == 0 ? "X" : (k == 1 ? "Y" : "Z"));
    if (ImGui::MenuItem(label)) { pick.part = Graph::kAxis; pick.axis = k; edit(_contextGraph, pick); }
    std::snprintf(label, sizeof(label), "Edit %s Axis Title...", k == 0 ? "X" : (k == 1 ? "Y" : "Z"));
    if (ImGui::MenuItem(label)) { pick.part = Graph::kAxisTitle; pick.axis = k; edit(_contextGraph, pick); }
    }
  if (ImGui::MenuItem("Edit Legend...")) { pick.part = Graph::kLegend; edit(_contextGraph, pick); }
  if (ImGui::BeginMenu("Edit Item", !graph.items().empty()))
    {
    for (size_t k=0; k<graph.items().size(); k++)
      {
      String label = graph.items()[k].label();
      if (label.isNull()) label = "(unnamed)";
      label += "...##"; label += int(k);
      if (ImGui::MenuItem(label.data())) { pick.part = Graph::kDataPoint; pick.item = int(k); pick.index = -1; edit(_contextGraph, pick); }
      }
    ImGui::EndMenu();
    }
  bool hasData = false;
  for (const auto & item : graph.items()) if (item.kind() == GraphItem::kData && item.data()) hasData = true;
  if (ImGui::BeginMenu("Show Data", hasData && onShowData))
    {
    for (size_t k=0; k<graph.items().size(); k++)
      {
      const GraphItem & item = graph.items()[k];
      if (item.kind() != GraphItem::kData || !item.data()) continue;
      String label = item.label().isNull() ? String("(unnamed)") : item.label();
      String entry = label + String("##data") + String(std::to_string(k));
      if (ImGui::MenuItem(entry.data())) onShowData(item.data(), label);
      }
    ImGui::EndMenu();
    }
  if (!graph.isAutoLegendPosition() || graph.titleOffsetX() != 0 || graph.titleOffsetY() != 0
      || graph.xAxis().titleOffsetX() != 0 || graph.xAxis().titleOffsetY() != 0
      || graph.yAxis().titleOffsetX() != 0 || graph.yAxis().titleOffsetY() != 0)
    if (ImGui::MenuItem("Reset Text Positions"))
      {
      changing();
      graph.setAutoLegendPosition();
      graph.setTitleOffset(0, 0);
      graph.xAxis().setTitleOffset(0, 0);
      graph.yAxis().setTitleOffset(0, 0);
      }
  ImGui::Separator();
  bool logX = graph.isLogX(), logY = graph.isLogY(), legend = graph.showLegend();
  if (ImGui::MenuItem("Log X", nullptr, logX)) { changing(); graph.setLogX(!logX); }
  if (ImGui::MenuItem("Log Y", nullptr, logY)) { changing(); graph.setLogY(!logY); }
  if (ImGui::MenuItem("Legend", nullptr, legend)) { changing(); graph.setShowLegend(!legend); }
  if (ImGui::MenuItem("Automatic ranges", nullptr, graph.isAutoX() && graph.isAutoY())) { changing(); graph.setAutoRange(); }
  if (ImGui::BeginMenu("Remove", !graph.items().empty()))
    {
    for (size_t k=0; k<graph.items().size(); k++)
      {
      String label = graph.items()[k].label();
      if (label.isNull()) label = "(unnamed)";
      label += "##"; label += int(k);
      if (ImGui::MenuItem(label.data())) { changing(); graph.items().erase(graph.items().begin() + k); break; }
      }
    ImGui::EndMenu();
    }
  ImGui::Separator();
  if (ImGui::MenuItem("Clear graph", nullptr, false, !graph.items().empty())) { changing(); graph.clearItems(); }
  ImGui::EndPopup();
  }

  bool CanvasPane::render(float width, float height, bool isCurrent, ImFont * font)
  {
  bool clicked = false;
  ImGui::PushID(_name.data());
  if (_showToolbar) renderToolbar();

  ImVec2 origin = ImGui::GetCursorScreenPos();
  ImVec2 available = ImGui::GetContentRegionAvail();
  float w = available.x > 10 ? available.x : 10;
  float h = available.y > 10 ? available.y : 10;
  (void)width; (void)height;
  _rectX = origin.x; _rectY = origin.y; _rectW = w; _rectH = h;
  ImGui::InvisibleButton("canvas", ImVec2(w, h), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
  bool hovered = ImGui::IsItemHovered();

  ImDrawList * drawList = ImGui::GetWindowDrawList();
  drawList->PushClipRect(origin, ImVec2(origin.x + w, origin.y + h), true);
  ImGuiPainter painter(drawList, font, origin.x, origin.y, w, h);
  _canvas.setSize(int(w), int(h));
  _canvas.draw(painter);

  // selected graph: blue frame around its cell (only when there are several graphs or the pane is current)
  if (_canvas.nGraphs() > 1 || isCurrent)
    {
    const Graph & g = _canvas.graph(_selectedGraph);
    float x0, y0, x1, y1;
    painter.toScreen(Point2D(g.cellXLow(), g.cellYHigh()), x0, y0);
    painter.toScreen(Point2D(g.cellXHigh(), g.cellYLow()), x1, y1);
    ImU32 blue = ImGui::GetColorU32(ImVec4(0.18f, 0.43f, 0.85f, isCurrent ? 0.9f : 0.35f));
    drawList->AddRect(ImVec2(x0+1, y0+1), ImVec2(x1-1, y1-1), blue, 0.0f, 0, isCurrent ? 2.0f : 1.0f);
    }

  // drop target: an object dragged from the file browser goes to the graph under the mouse
  if (ImGui::BeginDragDropTarget())
    {
    const ImGuiPayload * payload = ImGui::AcceptDragDropPayload(FileBrowser::payloadType(),
                                                                ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
    if (payload)
      {
      ImVec2 mouse = ImGui::GetMousePos();
      int k = _canvas.graphAt(painter.toCanvas(mouse.x, mouse.y));
      if (k >= 0)
        {
        const Graph & g = _canvas.graph(k);
        float x0, y0, x1, y1;
        painter.toScreen(Point2D(g.cellXLow(), g.cellYHigh()), x0, y0);
        painter.toScreen(Point2D(g.cellXHigh(), g.cellYLow()), x1, y1);
        drawList->AddRectFilled(ImVec2(x0+2, y0+2), ImVec2(x1-2, y1-2), ImGui::GetColorU32(ImVec4(0.18f, 0.43f, 0.85f, 0.10f)));
        drawList->AddRect(ImVec2(x0+2, y0+2), ImVec2(x1-2, y1-2), ImGui::GetColorU32(ImVec4(0.18f, 0.43f, 0.85f, 1.0f)), 0.0f, 0, 3.0f);
        if (payload->IsDelivery())
          {
          ObjectReference object;
          if (FileBrowser::decodePayload(payload->Data, payload->DataSize, object) && onDrop) onDrop(k, object);
          clicked = true;
          }
        }
      }
    ImGui::EndDragDropTarget();
    }

  // mouse: select, drags, dialogs, coordinates
  if (handleMouse(painter, hovered)) clicked = true;
  drawList->PopClipRect();
  renderContextMenu();
  if (_openEditor) { ImGui::OpenPopup("editor"); _openEditor = false; }
  renderEditor();
  ImGui::PopID();
  return clicked;
  }

  void CanvasPane::edit(int k, const Graph::Pick & pick)
  {
  if (k < 0 || k >= _canvas.nGraphs()) return;
  _editGraph    = k;
  _editPick     = pick;
  _editBefore   = _canvas.graph(k);
  _editRecorded = false;
  _openEditor   = true;
  }

  void CanvasPane::renderEditor()
  {
  if (!ImGui::BeginPopup("editor", ImGuiWindowFlags_AlwaysAutoResize)) return;
  if (_editGraph < 0 || _editGraph >= _canvas.nGraphs()) { ImGui::CloseCurrentPopup(); ImGui::EndPopup(); return; }
  Graph & graph = _canvas.graph(_editGraph);
  bool changed = false;
  switch (_editPick.part)
    {
    case Graph::kTitle:     changed = GraphEditor::editTitle(graph); break;
    case Graph::kAxis:      changed = GraphEditor::editAxis(graph, _editPick.axis); break;
    case Graph::kAxisTitle: changed = GraphEditor::editAxisTitle(graph, _editPick.axis); break;
    case Graph::kLegend:    changed = GraphEditor::editLegend(graph); break;
    case Graph::kDataPoint: changed = GraphEditor::editItem(graph, _editPick.item, _editPick.index); break;
    default: ImGui::CloseCurrentPopup(); break;
    }
  // the first change of the dialog records the undo state of the graph as it was when the dialog was opened
  if (changed && !_editRecorded)
    {
    Graph after = graph;
    graph = _editBefore;
    changing();
    graph = after;
    _editRecorded = true;
    }
  ImGui::EndPopup();
  }

  void CanvasPane::applyMove(float dx, float dy)
  {
  if (_drag.graph < 0 || _drag.graph >= _canvas.nGraphs()) return;
  Graph & g = _canvas.graph(_drag.graph);
  // screen pixels to fractions of the cell, of the frame (y up on the canvas, down on the screen)
  float cellW  = (g.cellXHigh()  - g.cellXLow())*_rectW, cellH  = (g.cellYHigh()  - g.cellYLow())*_rectH;
  float frameW = (g.frameXHigh() - g.frameXLow())*_rectW, frameH = (g.frameYHigh() - g.frameYLow())*_rectH;
  // texts stay within reach: titles within half a cell of their automatic position, the corner of the legend near
  // the frame (a drag released outside the window cannot lose them)
  auto clamp = [](float v, float low, float high) { return std::min(std::max(v, low), high); };
  switch (_drag.kind)
    {
    case kMoveTitle:
      g.setTitleOffset(clamp(_drag.initialX + dx/cellW, -0.5f, 0.5f), clamp(_drag.initialY - dy/cellH, -0.5f, 0.5f));
      break;
    case kMoveAxisTitle:
      g.axis(_drag.axis).setTitleOffset(clamp(_drag.initialX + dx/cellW, -0.5f, 0.5f), clamp(_drag.initialY - dy/cellH, -0.5f, 0.5f));
      break;
    case kMoveLegend:
      g.setLegendPosition(clamp(_drag.initialX + dx/frameW, -0.3f, 0.95f), clamp(_drag.initialY - dy/frameH, 0.05f, 1.3f));
      break;
    case kMoveLabels:
      {
      // away from the frame: down for the x axis, left for the y axis; 20 pixels per unit of the automatic distance
      float away = (_drag.axis == 0) ? dy : -dx;
      g.axis(_drag.axis).setLabelGap(std::min(std::max(_drag.initialX + away/20.0f, 0.0f), 5.0f));
      break;
      }
    default: break;
    }
  }

  bool CanvasPane::handleMouse(ImGuiPainter & painter, bool hovered)
  {
  bool clicked = false;
  ImDrawList * drawList = ImGui::GetWindowDrawList();
  ImVec2 mouse = ImGui::GetMousePos();
  Point2D p = painter.toCanvas(mouse.x, mouse.y);
  const float tolerance = 6.0f;   // pixels
  const float tolX = tolerance/_rectW, tolY = tolerance/_rectH;
  int k = _canvas.graphAt(p);
  Graph::Pick pick;
  if (k >= 0) pick = _canvas.graph(k).pick(p, tolX, tolY);

  // ---- a drag in progress
  if (_drag.kind != kNoDrag)
    {
    float dx = mouse.x - _drag.startX, dy = mouse.y - _drag.startY;
    if (!_drag.moved && std::fabs(dx) + std::fabs(dy) > 3.0f)
      {
      _drag.moved = true;
      if (_drag.kind >= kMoveTitle) changing();
      }
    bool isMove = _drag.kind >= kMoveTitle;
    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
      {
      if (isMove && _drag.moved) applyMove(0, 0);   // back to where it was
      _drag = Drag();
      return false;
      }
    if (_drag.graph >= _canvas.nGraphs()) { _drag = Drag(); return false; }
    Graph & g = _canvas.graph(_drag.graph);
    if (isMove)
      {
      if (_drag.moved) applyMove(dx, dy);
      ImGui::SetMouseCursor(_drag.kind == kMoveLabels ? (_drag.axis == 0 ? ImGuiMouseCursor_ResizeNS : ImGuiMouseCursor_ResizeEW)
                                                      : ImGuiMouseCursor_ResizeAll);
      }
    // zoom: rectangle (or band along an axis) from the start to the mouse, within the frame
    float fx0, fy0, fx1, fy1;
    painter.toScreen(Point2D(g.frameXLow(), g.frameYHigh()), fx0, fy0);
    painter.toScreen(Point2D(g.frameXHigh(), g.frameYLow()), fx1, fy1);
    float x0 = std::min(std::max(_drag.startX, fx0), fx1), x1 = std::min(std::max(mouse.x, fx0), fx1);
    float y0 = std::min(std::max(_drag.startY, fy0), fy1), y1 = std::min(std::max(mouse.y, fy0), fy1);
    if (_drag.kind == kZoomX) { y0 = fy0; y1 = fy1; }
    if (_drag.kind == kZoomY) { x0 = fx0; x1 = fx1; }
    if (!isMove && _drag.moved)
      {
      ImU32 blue = ImGui::GetColorU32(ImVec4(0.18f, 0.43f, 0.85f, 1.0f));
      drawList->AddRectFilled(ImVec2(std::min(x0,x1), std::min(y0,y1)), ImVec2(std::max(x0,x1), std::max(y0,y1)),
                              ImGui::GetColorU32(ImVec4(0.18f, 0.43f, 0.85f, 0.12f)));
      drawList->AddRect(ImVec2(std::min(x0,x1), std::min(y0,y1)), ImVec2(std::max(x0,x1), std::max(y0,y1)), blue, 0, 0, 1.5f);
      }
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
      {
      if (!isMove && _drag.moved && std::fabs(x1 - x0) > 4 && std::fabs(y1 - y0) > 4)
        {
        const Graph::View view = g.view();
        double xa, ya, xb, yb;
        g.fromCanvas(view, painter.toCanvas(std::min(x0,x1), std::max(y0,y1)), xa, ya);   // lower left
        g.fromCanvas(view, painter.toCanvas(std::max(x0,x1), std::min(y0,y1)), xb, yb);   // upper right
        changing();
        if (_drag.kind != kZoomY && xb > xa) g.setXRange(xa, xb);
        if (_drag.kind != kZoomX && yb > ya) g.setYRange(ya, yb);
        }
      _drag = Drag();
      }
    return false;
    }

  if (!hovered) { _hoverText = ""; return false; }

  // ---- coordinates under the mouse
  _hoverText = "";
  if (k >= 0 && !_canvas.graph(k).items().empty() && _canvas.graph(k).frameContains(p))
    {
    const Graph & g = _canvas.graph(k);
    const Graph::View view = g.view();
    double x, y;
    g.fromCanvas(view, p, x, y);
    char text[160];
    std::snprintf(text, sizeof(text), "x = %.5g   y = %.5g", x, y);
    _hoverText = text;
    if (view.hasZ)
      {
      // the value of the map under the mouse (the last drawn, on top), or of the colored point under it
      int nMaps = 0;
      for (const auto & item : g.items()) if (item.isMap()) nMaps++;
      String z;
      if (pick.part == Graph::kDataPoint && pick.item >= 0 && pick.item < int(g.items().size()) && g.items()[pick.item].isMap())
        {
        const GraphItem & item = g.items()[pick.item];
        std::vector<double> c = item.data()->coordinatesAt(pick.index);
        std::snprintf(text, sizeof(text), "point (%.5g, %.5g)   z = %.5g", c[0], c[1], item.data()->valueAt(pick.index));
        z = text;
        if (nMaps > 1) z += String("   (") + item.label() + String(")");
        }
      for (int n=int(g.items().size())-1; n>=0 && z.isNull(); n--)
        {
        const GraphItem & item = g.items()[n];
        if (!item.isMap()) continue;
        std::shared_ptr<const Grid2D> grid = item.grid(view.xMinimum, view.xMaximum, view.yMinimum, view.yMaximum, view.logX, view.logY);
        int i, j;
        if (!grid || !grid->cell(x, y, i, j) || !std::isfinite(grid->value(i, j))) continue;
        std::snprintf(text, sizeof(text), "z = %.5g", grid->value(i, j));
        z = text;
        if (nMaps > 1) z += String("   (") + item.label() + String(")");
        }
      if (!z.isNull()) _hoverText = String(_hoverText.str() + "   " + z.str());
      }
    }
  if (k < 0) return false;
  Graph & g = _canvas.graph(k);
  const bool shift = ImGui::GetIO().KeyShift;

  // ---- cursor: what a drag would do
  switch (pick.part)
    {
    case Graph::kTitle: case Graph::kAxisTitle: case Graph::kLegend: ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll); break;
    case Graph::kAxis:
      ImGui::SetMouseCursor((pick.axis == 0) != shift ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
      break;
    case Graph::kDataPoint: ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); break;
    default: break;
    }

  // ---- double-click in the frame: automatic ranges
  if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && (pick.part == Graph::kFrame || pick.part == Graph::kDataPoint))
    {
    if (!g.isAutoX() || !g.isAutoY()) { changing(); g.setAutoRange(); }
    return true;
    }

  // ---- left button pressed: select the graph and start a drag
  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
    _selectedGraph = k;
    clicked = true;
    Drag drag;
    drag.graph  = k;
    drag.axis   = pick.axis;
    drag.startX = mouse.x;
    drag.startY = mouse.y;
    switch (pick.part)
      {
      case Graph::kTitle:
        drag.kind = kMoveTitle; drag.initialX = g.titleOffsetX(); drag.initialY = g.titleOffsetY(); break;
      case Graph::kAxisTitle:
        drag.kind = kMoveAxisTitle;
        drag.initialX = g.axis(pick.axis).titleOffsetX(); drag.initialY = g.axis(pick.axis).titleOffsetY(); break;
      case Graph::kLegend:
        {
        drag.kind = kMoveLegend;
        const Rect2D & r = g.legendRect();
        float w = g.frameXHigh() - g.frameXLow(), h = g.frameYHigh() - g.frameYLow();
        drag.initialX = (r.x0 - g.frameXLow())/w;   // upper left corner, fractions of the frame
        drag.initialY = (r.y1 - g.frameYLow())/h;
        break;
        }
      case Graph::kAxis:
        if (shift) { drag.kind = kMoveLabels; drag.initialX = g.axis(pick.axis).labelGap(); }
        else if (pick.axis < 2) drag.kind = (pick.axis == 0) ? kZoomX : kZoomY;   // the z axis: not by dragging
        break;
      case Graph::kFrame: case Graph::kDataPoint:
        drag.kind = kZoom; break;
      default: break;
      }
    _drag = drag;
    }

  // ---- right button: dialog editing the part under the mouse, or the context menu of the graph
  if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
    _selectedGraph = k;
    clicked = true;
    switch (pick.part)
      {
      case Graph::kTitle: case Graph::kAxis: case Graph::kAxisTitle: case Graph::kLegend: case Graph::kDataPoint:
        edit(k, pick);
        break;
      default:
        _contextGraph = k;
        ImGui::OpenPopup("graphContext");
        break;
      }
    }
  return clicked;
  }

  } // namespace GUI
} // namespace CAP
