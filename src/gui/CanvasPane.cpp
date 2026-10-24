/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
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
#include "CanvasRegistry.hpp"
#include "ParameterizedFunction.hpp"
#include "Histogram.hpp"
#include "FitRecord.hpp"
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
  _keepShapes(true),
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
  _editBefore(),
  _tool(kSelectTool),
  _shapeStyle(),
  _shapeGraph(-1),
  _shapeIndex(-1),
  _drafting(false),
  _draftGraph(-1),
  _draft(),
  _openShapeEditor(false),
  _openShapeMenu(false)
  {
  _canvas.divide(1,1);
  setTitle(name);
  GRAPHICS::CanvasRegistry::session().add(_canvas, name, "Main window");   // listed until the pane is closed
  }

  void CanvasPane::setTitle(const String & title)
  {
  _title = title;
  if (_canvas.registry()) _canvas.registry()->rename(_canvas, title);
  _windowId = std::string("###") + _name.str();
  _windowLabel = _title.str() + _windowId;
  }

  void CanvasPane::setSelectedGraph(int k)
  {
  if (k >= 0 && k < _canvas.nGraphs()) _selectedGraph = k;
  }

  GraphItem & CanvasPane::show(const GraphItem & item)
  {
  changing();
  if (_mode == kReplace && !_keepShapes) currentGraph().clearShapes();
  return currentGraph().show(item, _mode == kReplace);   // default style; titles from a first object
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
  renderPalette();
  ImGui::SameLine(0, 18);
  ImGui::Checkbox("Keep shapes", &_keepShapes);
  ImGui::SetItemTooltip("Shapes and texts stay in a graph when its contents are replaced (Replace, or a folder shown); "
                        "otherwise they go with them");
  ImGui::SameLine();
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
  if (graph.hasMaps())            // a graph of a function or data of two variables: its z scale, its 3D view
    {
    ImGui::SameLine();
    bool logZ = graph.isLogZ();
    if (ImGui::Checkbox("Log Z", &logZ)) { changing(); graph.setLogZ(logZ); }
    ImGui::SetItemTooltip("Logarithmic z scale for the selected graph (colors, and heights in 3D)");
    ImGui::SameLine();
    bool view3D = graph.dimension() == Graph::k3D;
    if (ImGui::Checkbox("3D", &view3D)) { changing(); graph.setDimension(view3D ? Graph::k3D : Graph::k2D); }
    ImGui::SetItemTooltip("3D view of the selected graph (surface, mesh or lego boxes); drag in it to turn it");
    }

  bool fitted = false, curves = false;   // a graph of objects with fits: show them or not; residuals of curves
  for (const auto & item : graph.items()) if (item.nFits() > 0) { fitted = true; curves = curves || !item.isMap(); }
  if (fitted)
    {
    ImGui::SameLine();
    bool fits = graph.showFits();
    if (ImGui::Checkbox("Fits", &fits)) { changing(); graph.setShowFits(fits); }
    ImGui::SetItemTooltip("Draw the fits of the objects of the selected graph (View > Fit Results for their results)");
    }
  if (curves)
    {
    ImGui::SameLine();
    bool residuals = graph.showResiduals();
    if (ImGui::Checkbox("Residuals", &residuals)) { changing(); graph.setShowResiduals(residuals); }
    ImGui::SetItemTooltip("Panel below the graph with the residuals of the fits (data - fit, or pulls: right-click), in their colors");
    }

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
  if (ImGui::Button("Clear")) { changing(); graph.clearItems(); graph.clearShapes(); }
  ImGui::SetItemTooltip("Remove everything from the selected graph (objects and shapes)");
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
  if (graph.hasMaps())
    {
    ImGui::Separator();
    bool view3D = graph.dimension() == Graph::k3D;
    if (ImGui::MenuItem("3D View", nullptr, view3D)) { changing(); graph.setDimension(view3D ? Graph::k2D : Graph::k3D); }
    if (view3D && ImGui::MenuItem("Reset View Angles")) { changing(); graph.resetViewAngles(); }
    }
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
  // two histograms of the same rank: overlay, difference, compatibility tests
  std::vector<size_t> histograms;
  for (size_t k=0; k<graph.items().size(); k++)
    {
    const GraphItem & item = graph.items()[k];
    if (item.kind() == GraphItem::kData && dynamic_cast<const MATH::Histogram*>(item.data())) histograms.push_back(k);
    }
  if (onCompare && ImGui::BeginMenu("Compare", histograms.size() >= 2))
    {
    for (size_t i=0; i<histograms.size(); i++)
      for (size_t j=i+1; j<histograms.size(); j++)
        {
        const GraphItem & a = graph.items()[histograms[i]];
        const GraphItem & b = graph.items()[histograms[j]];
        if (a.data()->rank() != b.data()->rank()) continue;
        String entry = a.label() + String(" and ") + b.label() + String("##compare")
                       + String(std::to_string(i*1000 + j).c_str());
        if (ImGui::MenuItem(entry.data())) onCompare(a.data(), b.data(), a.label(), b.label());
        }
    ImGui::EndMenu();
    }
  if (onCompare && histograms.size() < 2 && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    ImGui::SetTooltip("Show two histograms in this graph (drag them from the browsers) to compare them");
  bool fittable = false;
  for (const auto & item : graph.items()) if (item.isFittable()) fittable = true;
  if (onFit && ImGui::BeginMenu("Fit", fittable))
    {
    for (size_t k=0; k<graph.items().size(); k++)
      {
      if (!graph.items()[k].isFittable()) continue;
      String label = graph.items()[k].label().isNull() ? String("(unnamed)") : graph.items()[k].label();
      String entry = label + String("...##fit") + String(std::to_string(k));
      if (ImGui::MenuItem(entry.data())) { setSelectedGraph(_contextGraph); onFit(int(k)); }
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
  if (onSaveGraph || onSaveCanvas || onRename) ImGui::Separator();
  if (onSaveGraph && ImGui::MenuItem("Save Graph As...")) onSaveGraph(_contextGraph);
  if (onSaveCanvas && ImGui::MenuItem("Save Canvas As...")) onSaveCanvas();
  if (onRename && ImGui::MenuItem("Rename Canvas...")) onRename();
  if (graph.hasMovedTexts() && ImGui::MenuItem("Reset Text Positions")) { changing(); graph.resetTextPositions(); }
  ImGui::Separator();
  bool logX = graph.isLogX(), logY = graph.isLogY(), legend = graph.showLegend();
  if (ImGui::MenuItem("Log X", nullptr, logX)) { changing(); graph.setLogX(!logX); }
  if (ImGui::MenuItem("Log Y", nullptr, logY)) { changing(); graph.setLogY(!logY); }
  if (graph.hasMaps())
    {
    bool logZ = graph.isLogZ();
    if (ImGui::MenuItem("Log Z", nullptr, logZ)) { changing(); graph.setLogZ(!logZ); }
    }
  if (ImGui::MenuItem("Legend", nullptr, legend)) { changing(); graph.setShowLegend(!legend); }
  if (ImGui::BeginMenu("Palettes"))
    {
    // the palettes of this graph: "Session's" follows the current palettes of the session (Edit > Preferences)
    GRAPHICS::Palettes & palettes = GRAPHICS::Palettes::session();
    auto choose = [&](const char * menu, const String & current, const std::vector<String> & names, const String & sessions,
                      const std::function<void(const String &)> & set)
      {
      if (!ImGui::BeginMenu(menu)) return;
      String label = String("Session's (") + sessions + String(")");
      if (ImGui::MenuItem(label.data(), nullptr, current.str().empty())) set(String());
      ImGui::Separator();
      for (const String & n : names) if (ImGui::MenuItem(n.data(), nullptr, n == current)) set(n);
      ImGui::EndMenu();
      };
    choose("Colors", graph.colorPaletteName(), palettes.colorPaletteNames(), palettes.colorPalette().name(),
           [&](const String & n) { changing(); graph.setColorPaletteName(n); graph.restyleItems(); });
    choose("Markers", graph.markerPaletteName(), palettes.markerPaletteNames(), palettes.markerPalette().name(),
           [&](const String & n) { changing(); graph.setMarkerPaletteName(n); graph.restyleItems(); });
    if (graph.hasMaps() && ImGui::BeginMenu("Shading"))
      {
      for (const String & n : palettes.shadingPaletteNames())
        {
        const GRAPHICS::ShadingPalette * s = palettes.shadingPalette(n);
        bool current = s && s->type() == graph.palette().type();
        if (ImGui::MenuItem(n.data(), nullptr, current) && s) { changing(); graph.setPalette(*s); }
        }
      ImGui::EndMenu();
      }
    ImGui::EndMenu();
    }
  bool fitted = false, curves = false, maps = false;
  for (const auto & item : graph.items())
    if (item.nFits() > 0) { fitted = true; curves = curves || !item.isMap(); maps = maps || item.isMap(); }
  if (fitted)
    {
    bool fits = graph.showFits(), statistics = graph.showFitStatistics();
    if (ImGui::MenuItem("Fits", nullptr, fits)) { changing(); graph.setShowFits(!fits); }
    if (ImGui::MenuItem("Fit Quality in the Legend", nullptr, statistics)) { changing(); graph.setShowFitStatistics(!statistics); }
    }
  if (maps && onResidualMap && ImGui::BeginMenu("Residual Map"))
    {
    for (size_t i=0; i<graph.items().size(); i++)
      {
      const GraphItem & item = graph.items()[i];
      if (!item.isMap() || item.kind() != GraphItem::kData || !item.data()) continue;
      for (unsigned int k=0; k<item.nFits(); k++)
        {
        const MATH::FitRecord * fit = item.fit(k);
        if (!fit || !fit->function || fit->function->rank() != 2) continue;
        String entry = item.label() + String(": ") + (fit->label.isNull() ? String("fit") : fit->label)
                       + String("##map") + String(std::to_string(i*100 + k));
        if (ImGui::MenuItem(entry.data())) onResidualMap(item.data(), k);
        }
      }
    ImGui::EndMenu();
    }
  if (maps && onProjections && ImGui::BeginMenu("Projections"))
    {
    for (size_t i=0; i<graph.items().size(); i++)
      {
      const GraphItem & item = graph.items()[i];
      if (!item.isMap() || item.nFits() == 0 || !dynamic_cast<const MATH::Histogram*>(item.data())) continue;
      String entry = item.label() + String(" on x and y##projection") + String(std::to_string(i));
      if (ImGui::MenuItem(entry.data())) onProjections(item.data());
      }
    ImGui::EndMenu();
    }
  if (curves)
    {
    bool residuals = graph.showResiduals();
    if (ImGui::MenuItem("Residuals Panel", nullptr, residuals)) { changing(); graph.setShowResiduals(!residuals); }
    if (residuals && ImGui::BeginMenu("Residuals"))
      {
      bool pulls = graph.residualMode() == Graph::kPull;
      if (ImGui::MenuItem("Data \xE2\x88\x92 Fit", nullptr, !pulls)) { changing(); graph.setResidualMode(Graph::kDifference); }
      if (ImGui::MenuItem("Pulls (Data \xE2\x88\x92 Fit) / Uncertainty", nullptr, pulls)) { changing(); graph.setResidualMode(Graph::kPull); }
      ImGui::Separator();
      for (float fraction : { 0.2f, 0.3f, 0.4f })
        {
        char label[32];
        std::snprintf(label, sizeof(label), "Height %.0f%%", 100.0f*fraction);
        if (ImGui::MenuItem(label, nullptr, std::fabs(graph.residualFraction() - fraction) < 0.01f)) { changing(); graph.setResidualFraction(fraction); }
        }
      if (ImGui::MenuItem("Automatic Range", nullptr, graph.isAutoResidualRange())) { changing(); graph.setAutoResidualRange(); }
      ImGui::EndMenu();
      }
    }
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
  if (ImGui::MenuItem("Remove Shapes", nullptr, false, !graph.shapes().empty())) { changing(); graph.clearShapes(); }
  if (ImGui::MenuItem("Clear graph", nullptr, false, !graph.items().empty() || !graph.shapes().empty()))
    { changing(); graph.clearItems(); graph.clearShapes(); }
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

  drawShapeOverlay(painter);

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
  renderShapeMenus();
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
    case kRotate:          // half a degree per pixel: right turns the box, up raises the eye
      g.setViewAngles(_drag.initialX - 0.5f*dx, _drag.initialY + 0.5f*dy);
      break;
    case kMoveShape: case kMoveVertex:
      {
      if (_drag.shape < 0 || _drag.shape >= int(g.shapes().size())) break;
      std::vector<Point2D> & points = g.shapes()[size_t(_drag.shape)].points();
      if (points.size() != _drag.points.size()) break;
      for (size_t k=0; k<points.size(); k++)
        if (_drag.kind == kMoveShape || int(k) == _drag.vertex)
          points[k].set(_drag.points[k].x() + dx/cellW, _drag.points[k].y() - dy/cellH);
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
        changing();
        g.zoom(painter.toCanvas(x0, y0), painter.toCanvas(x1, y1), _drag.kind != kZoomY, _drag.kind != kZoomX);
        }
      _drag = Drag();
      }
    return false;
    }

  if (_tool != kSelectTool || _drafting) return handleDrawing(painter, hovered, k, p);
  if (!hovered) { _hoverText = ""; return false; }

  // ---- shapes (over everything else): the handles of the selected one, then the shape under the mouse
  if (k >= 0)
    {
    Graph & g = _canvas.graph(k);
    int vertex = -1;
    if (selectedShape() && _shapeGraph == k)
      {
      const std::vector<Point2D> & points = g.shapes()[size_t(_shapeIndex)].points();
      for (size_t v=0; v<points.size(); v++)
        {
        float sx, sy;
        painter.toScreen(Shape::toCanvas(g, points[v]), sx, sy);
        if (std::fabs(sx - mouse.x) <= tolerance && std::fabs(sy - mouse.y) <= tolerance) vertex = int(v);
        }
      }
    int shape = vertex >= 0 ? _shapeIndex : g.shapeAt(p, tolX, tolY);
    if (shape >= 0)
      {
      ImGui::SetMouseCursor(vertex >= 0 ? ImGuiMouseCursor_ResizeAll : ImGuiMouseCursor_Hand);
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
        _selectedGraph = k;
        _shapeGraph = k;
        _shapeIndex = shape;
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) _openShapeMenu = true;
        else if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) _openShapeEditor = true;
        else
          {
          Drag drag;
          drag.kind   = vertex >= 0 ? kMoveVertex : kMoveShape;
          drag.graph  = k;
          drag.shape  = shape;
          drag.vertex = vertex;
          drag.points = g.shapes()[size_t(shape)].points();
          drag.startX = mouse.x;
          drag.startY = mouse.y;
          _drag = drag;
          }
        return true;
        }
      _hoverText = String("Shape: ") + String(Shape::kindName(g.shapes()[size_t(shape)].kind()))
                   + String(" (drag: move; right-click: menu)");
      return false;
      }
    }
  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) _shapeGraph = -1;

  // ---- coordinates under the mouse
  _hoverText = "";
  if (k >= 0 && _canvas.graph(k).residualContains(p))
    {
    // the residuals panel: x and the residual under the mouse
    const Graph & g = _canvas.graph(k);
    double x, r;
    g.fromResidualCanvas(g.view(), p, x, r);
    char text[160];
    std::snprintf(text, sizeof(text), "x = %.5g   %s = %.5g", x, g.residualMode() == Graph::kPull ? "pull" : "data \xE2\x88\x92 fit", r);
    _hoverText = text;
    }
  else if (k >= 0 && !_canvas.graph(k).items().empty() && _canvas.graph(k).frameContains(p))
    {
    const Graph & g = _canvas.graph(k);
    const Graph::View view = g.view();
    if (g.is3D())
      {
      char view3D[128];
      std::snprintf(view3D, sizeof(view3D), "azimuth %.0f, elevation %.0f (drag to turn the view, double-click for the default)",
                    g.azimuth(), g.elevation());
      _hoverText = view3D;
      }
    else
    {
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
      double value;
      int index;
      if (z.isNull() && g.valueAt(view, x, y, value, index))
        {
        std::snprintf(text, sizeof(text), "z = %.5g", value);
        z = text;
        if (nMaps > 1) z += String("   (") + g.items()[std::size_t(index)].label() + String(")");
        }
      if (!z.isNull()) _hoverText = String(_hoverText.str() + "   " + z.str());
      }
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
    case Graph::kFrame: if (g.is3D()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll); break;
    default: break;
    }

  // ---- double-click in the frame: automatic ranges
  if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && pick.part == Graph::kResiduals)
    {
    changing();
    g.setAutoX();
    g.setAutoResidualRange();
    return true;
    }
  if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && (pick.part == Graph::kFrame || pick.part == Graph::kDataPoint))
    {
    if (g.is3D()) { changing(); g.resetViewAngles(); g.setAutoRange(); return true; }   // 3D: back to the default view
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
        else if (pick.axis < 2 && !g.is3D()) drag.kind = (pick.axis == 0) ? kZoomX : kZoomY;   // the z axis, 3D views: not by dragging
        break;
      case Graph::kResiduals: drag.kind = kZoomX; break;   // zooms in x, as the x axis
      case Graph::kFrame: case Graph::kDataPoint:
        if (g.is3D()) { drag.kind = kRotate; drag.initialX = g.azimuth(); drag.initialY = g.elevation(); }
        else drag.kind = kZoom;
        break;
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

  // ------------------------------------------------------------------------------------------------ shapes

  Shape * CanvasPane::selectedShape()
  {
  if (_shapeGraph < 0 || _shapeGraph >= _canvas.nGraphs()) { _shapeGraph = -1; return nullptr; }
  std::vector<Shape> & shapes = _canvas.graph(_shapeGraph).shapes();
  if (_shapeIndex < 0 || _shapeIndex >= int(shapes.size())) { _shapeGraph = -1; return nullptr; }
  return &shapes[size_t(_shapeIndex)];
  }

  bool CanvasPane::hasSelectedShape() const
  {
  return const_cast<CanvasPane*>(this)->selectedShape() != nullptr;
  }

  void CanvasPane::deleteSelectedShape()
  {
  if (!selectedShape()) return;
  changing();
  std::vector<Shape> & shapes = _canvas.graph(_shapeGraph).shapes();
  shapes.erase(shapes.begin() + _shapeIndex);
  _shapeGraph = -1;
  }

  void CanvasPane::renderPalette()
  {
  static const char * tips[] =
    {
    "Select: click a shape to select it, drag it to move it (or one of its handles to reshape it); "
    "right-click for its menu, double-click for its properties",
    "Rectangle: drag in a graph (Shift: square)",
    "Oval: drag in a graph (Shift: circle)",
    "Triangle: drag in a graph (Shift: as wide as high)",
    "Polygon: click its vertices; double-click, Enter or right-click to close it (Esc: cancel)",
    "Line: drag in a graph (Shift: horizontal, vertical or diagonal); arrows in its properties",
    "Text: click in a graph (or drag its box), then type it; every shape can also hold a text (its properties)",
    };
  const float size = ImGui::GetFrameHeight();
  ImDrawList * drawList = ImGui::GetWindowDrawList();
  for (int t=0; t<7; t++)
    {
    if (t > 0) ImGui::SameLine(0, 2);
    ImGui::PushID(t);
    if (ImGui::InvisibleButton("tool", ImVec2(size, size))) setTool(Tool(t));
    ImGui::SetItemTooltip("%s", tips[t]);
    ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
    bool active = _tool == Tool(t);
    if (active || ImGui::IsItemHovered())
      drawList->AddRectFilled(a, b, ImGui::GetColorU32(active ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered), 3.0f);
    ImU32 c = ImGui::GetColorU32(ImGuiCol_Text);
    float m = 0.25f*size, x0 = a.x + m, y0 = a.y + m, x1 = b.x - m, y1 = b.y - m;
    switch (Tool(t))
      {
      case kSelectTool:
        {
        ImVec2 arrow[] = { ImVec2(x0+1, y0-1), ImVec2(x0+1, y1+1), ImVec2(x0+0.38f*(x1-x0), y0+0.68f*(y1-y0)),
                           ImVec2(x1, y0+0.62f*(y1-y0)) };
        drawList->AddConvexPolyFilled(arrow, 4, c);
        break;
        }
      case kRectangleTool: drawList->AddRect(ImVec2(x0, y0+2), ImVec2(x1, y1-2), c, 0, 0, 1.5f); break;
      case kEllipseTool:   drawList->AddEllipse(ImVec2(0.5f*(x0+x1), 0.5f*(y0+y1)), ImVec2(0.5f*(x1-x0)+1, 0.5f*(y1-y0)-1), c, 0, 0, 1.5f); break;
      case kTriangleTool:  drawList->AddTriangle(ImVec2(x0, y1), ImVec2(x1, y1), ImVec2(0.5f*(x0+x1), y0), c, 1.5f); break;
      case kPolygonTool:
        {
        ImVec2 pentagon[5];
        for (int v=0; v<5; v++)
          {
          float angle = -0.5f*float(M_PI) + 2.0f*float(M_PI)*float(v)/5.0f;
          pentagon[v] = ImVec2(0.5f*(x0+x1) + 0.55f*(x1-x0)*std::cos(angle), 0.5f*(y0+y1) + 0.1f*(y1-y0) + 0.55f*(y1-y0)*std::sin(angle));
          }
        drawList->AddPolyline(pentagon, 5, c, ImDrawFlags_Closed, 1.5f);
        break;
        }
      case kTextTool:      // a T
        drawList->AddLine(ImVec2(x0, y0+1), ImVec2(x1, y0+1), c, 2.0f);
        drawList->AddLine(ImVec2(0.5f*(x0+x1), y0+1), ImVec2(0.5f*(x0+x1), y1), c, 2.0f);
        break;
      case kLineTool:
        drawList->AddLine(ImVec2(x0, y1), ImVec2(x1-2, y0+2), c, 1.5f);
        drawList->AddTriangleFilled(ImVec2(x1+1, y0-1), ImVec2(x1-6, y0+1), ImVec2(x1-1, y0+6), c);
        break;
      }
    ImGui::PopID();
    }
  }

  bool CanvasPane::handleDrawing(ImGuiPainter & painter, bool hovered, int k, const Point2D & p)
  {
  ImGuiIO & io = ImGui::GetIO();
  bool polygon = _tool == kPolygonTool;
  if (ImGui::IsKeyPressed(ImGuiKey_Escape)) { _drafting = false; _tool = kSelectTool; return false; }
  if (!_drafting)
    {
    if (!hovered || k < 0) return false;
    ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
    _hoverText = "Draw a shape (Esc: back to selecting)";
    if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left)) return false;
    static const Shape::Kind kinds[] = { Shape::kRectangle, Shape::kRectangle, Shape::kEllipse, Shape::kTriangle,
                                         Shape::kPolygon, Shape::kLine, Shape::kText };
    _draft = _shapeStyle;
    _draft.setKind(kinds[_tool]);
    if (_tool != kLineTool) _draft.setArrows(Shape::kNoArrow);
    if (_tool == kTextTool)                                      // a text box: no fill or frame unless asked
      {
      _draft.fillAttributes().setStyle(FillAttributes::kHollow);
      _draft.lineAttributes().setStyle(0);
      }
    _draft.setText("");                                          // the style of the last shape edited, not its text
    Point2D q = Shape::fromCanvas(_canvas.graph(k), p);
    _draft.setPoints({ q, q });
    _draftGraph = k;
    _drafting = true;
    _selectedGraph = k;
    return true;
    }
  if (_draftGraph < 0 || _draftGraph >= _canvas.nGraphs()) { _drafting = false; return false; }
  Graph & g = _canvas.graph(_draftGraph);
  std::vector<Point2D> & points = _draft.points();
  Point2D q = Shape::fromCanvas(g, p);
  // Shift: a square box, or a line at a multiple of 45 degrees (in pixels)
  if (io.KeyShift && points.size() >= 2)
    {
    const Point2D & a = points[points.size() - 2];
    float cw = (g.cellXHigh() - g.cellXLow())*_rectW, ch = (g.cellYHigh() - g.cellYLow())*_rectH;
    float dx = (q.x() - a.x())*cw, dy = (q.y() - a.y())*ch;
    if (_draft.kind() == Shape::kLine || polygon)
      {
      float angle = std::round(std::atan2(dy, dx)/(0.25f*float(M_PI)))*0.25f*float(M_PI), d = std::sqrt(dx*dx + dy*dy);
      dx = d*std::cos(angle); dy = d*std::sin(angle);
      }
    else
      {
      float d = std::max(std::fabs(dx), std::fabs(dy));
      dx = dx < 0 ? -d : d; dy = dy < 0 ? -d : d;
      }
    q = Point2D(a.x() + dx/cw, a.y() + dy/ch);
    }
  points.back() = q;
  auto finish = [&]()
    {
    if (polygon && points.size() > 3) points.pop_back();          // the vertex following the mouse
    _drafting = false;
    bool text = _tool == kTextTool;
    _tool = kSelectTool;
    // big enough: at least 3 pixels across (a text box clicked: 160 x 40 pixels around the click)
    float cw = (g.cellXHigh() - g.cellXLow())*_rectW, ch = (g.cellYHigh() - g.cellYLow())*_rectH;
    if (text && points.size() >= 2 && std::fabs(points[1].x() - points[0].x())*cw < 8 && std::fabs(points[1].y() - points[0].y())*ch < 8)
      {
      Point2D c = points[0];
      points = { Point2D(c.x() - 80.0f/cw, c.y() - 20.0f/ch), Point2D(c.x() + 80.0f/cw, c.y() + 20.0f/ch) };
      }
    float xMin = 1e9f, xMax = -1e9f, yMin = 1e9f, yMax = -1e9f;
    for (const auto & v : points) { xMin = std::min(xMin, v.x()*cw); xMax = std::max(xMax, v.x()*cw); yMin = std::min(yMin, v.y()*ch); yMax = std::max(yMax, v.y()*ch); }
    if (!_draft.isValid() || std::max(xMax - xMin, yMax - yMin) < 3.0f) return;
    changing();
    g.addShape(_draft);
    _shapeGraph = _draftGraph;
    _shapeIndex = int(g.shapes().size()) - 1;
    if (text) _openShapeEditor = true;                           // type the text at once
    };
  if (polygon)
    {
    _hoverText = "Click the next vertex; double-click, Enter or right-click to close the polygon (Esc: cancel)";
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)
        || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
      finish();   // a double-click: its first click added the last vertex
    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) points.push_back(q);
    return true;
    }
  if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) finish();
  return true;
  }

  void CanvasPane::drawShapeOverlay(ImGuiPainter & painter)
  {
  ImDrawList * drawList = ImGui::GetWindowDrawList();
  ImU32 blue = ImGui::GetColorU32(ImVec4(0.18f, 0.43f, 0.85f, 1.0f));
  if (_drafting && _draftGraph >= 0 && _draftGraph < _canvas.nGraphs())
    {
    const Graph & g = _canvas.graph(_draftGraph);
    if (_draft.isValid()) _draft.draw(painter, g);
    if (_draft.kind() == Shape::kText && _draft.isValid())       // the box of a text, drawn with nothing yet
      {
      std::vector<ImVec2> box;
      for (const auto & v : _draft.outline())
        {
        float x, y;
        painter.toScreen(Shape::toCanvas(g, v), x, y);
        box.push_back(ImVec2(x, y));
        }
      drawList->AddPolyline(box.data(), int(box.size()), blue, ImDrawFlags_Closed, 1.0f);
      }
    if (_draft.kind() == Shape::kPolygon)                        // the edges so far, even before 3 vertices
      for (size_t v=0; v+1<_draft.points().size(); v++)
        {
        float x0, y0, x1, y1;
        painter.toScreen(Shape::toCanvas(g, _draft.points()[v]), x0, y0);
        painter.toScreen(Shape::toCanvas(g, _draft.points()[v+1]), x1, y1);
        drawList->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), blue, 1.0f);
        }
    }
  Shape * shape = selectedShape();
  if (!shape) return;
  const Graph & g = _canvas.graph(_shapeGraph);
  // the outline, then a handle on each point
  std::vector<Point2D> outline = shape->outline();
  std::vector<ImVec2> screen;
  for (const auto & v : outline)
    {
    float x, y;
    painter.toScreen(Shape::toCanvas(g, v), x, y);
    screen.push_back(ImVec2(x, y));
    }
  if (screen.size() >= 2) drawList->AddPolyline(screen.data(), int(screen.size()), blue, shape->isClosed() ? ImDrawFlags_Closed : 0, 1.0f);
  for (const auto & v : shape->points())
    {
    float x, y;
    painter.toScreen(Shape::toCanvas(g, v), x, y);
    drawList->AddRectFilled(ImVec2(x-3.5f, y-3.5f), ImVec2(x+3.5f, y+3.5f), IM_COL32(255, 255, 255, 255));
    drawList->AddRect(ImVec2(x-3.5f, y-3.5f), ImVec2(x+3.5f, y+3.5f), blue, 0, 0, 1.5f);
    }
  }

  void CanvasPane::renderShapeMenus()
  {
  if (_openShapeMenu)   { ImGui::OpenPopup("shapeMenu"); _openShapeMenu = false; }
  if (_openShapeEditor && selectedShape())
    {
    _editGraph    = _shapeGraph;
    _editBefore   = _canvas.graph(_shapeGraph);
    _editRecorded = false;
    ImGui::OpenPopup("shapeEditor");
    }
  _openShapeEditor = false;
  if (ImGui::BeginPopup("shapeMenu"))
    {
    Shape * shape = selectedShape();
    if (!shape) { ImGui::CloseCurrentPopup(); ImGui::EndPopup(); return; }
    std::vector<Shape> & shapes = _canvas.graph(_shapeGraph).shapes();
    ImGui::TextDisabled("%s", Shape::kindName(shape->kind()));
    ImGui::Separator();
    if (ImGui::MenuItem("Properties...")) _openShapeEditor = true;
    if (ImGui::MenuItem("Duplicate"))
      {
      changing();
      Shape copy = *shape;
      copy.move(0.03f, -0.03f);
      shapes.push_back(copy);
      _shapeIndex = int(shapes.size()) - 1;
      }
    if (ImGui::MenuItem("Bring to Front", nullptr, false, _shapeIndex + 1 < int(shapes.size())))
      {
      changing();
      Shape moved = *shape;
      shapes.erase(shapes.begin() + _shapeIndex);
      shapes.push_back(moved);
      _shapeIndex = int(shapes.size()) - 1;
      }
    if (ImGui::MenuItem("Send to Back", nullptr, false, _shapeIndex > 0))
      {
      changing();
      Shape moved = *shape;
      shapes.erase(shapes.begin() + _shapeIndex);
      shapes.insert(shapes.begin(), moved);
      _shapeIndex = 0;
      }
    if (ImGui::MenuItem("Use for New Shapes")) _shapeStyle = *shape;
    ImGui::SetItemTooltip("New shapes get the colors, patterns and lines of this one");
    ImGui::Separator();
    if (ImGui::MenuItem("Delete", "Delete")) deleteSelectedShape();
    ImGui::EndPopup();
    }
  if (ImGui::BeginPopup("shapeEditor", ImGuiWindowFlags_AlwaysAutoResize))
    {
    Shape * shape = selectedShape();
    if (!shape) { ImGui::CloseCurrentPopup(); ImGui::EndPopup(); return; }
    if (GraphEditor::editShape(*shape))
      {
      _shapeStyle = *shape;                                    // new shapes look like the one edited last
      if (!_editRecorded)                                      // undo: the graph as it was when the dialog opened
        {
        Graph & graph = _canvas.graph(_shapeGraph);
        Graph after = graph;
        graph = _editBefore;
        changing();
        graph = after;
        _editRecorded = true;
        }
      }
    ImGui::EndPopup();
    }
  }

  } // namespace GUI
} // namespace CAP
