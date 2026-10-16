#include <cstdio>
#include "imgui.h"
#include "CanvasPane.hpp"
#include "ImGuiPainter.hpp"

namespace CAP
{
  namespace GUI
  {
  using namespace GRAPHICS;

  CanvasPane::CanvasPane(const String & name)
  :
  beforeChange(),
  onDrop(),
  _name(name),
  _canvas(name, name),
  _mode(kAdd),
  _selectedGraph(0),
  _showToolbar(true),
  _contextGraph(-1),
  _gridColumns(1),
  _gridRows(1),
  _hoverText(),
  _rectX(0), _rectY(0), _rectW(0), _rectH(0)
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
  ImGui::PopID();
  }

  void CanvasPane::renderContextMenu()
  {
  if (!ImGui::BeginPopup("graphContext")) return;
  if (_contextGraph < 0 || _contextGraph >= _canvas.nGraphs()) { ImGui::EndPopup(); return; }
  Graph & graph = _canvas.graph(_contextGraph);
  ImGui::TextDisabled("%s", graph.title().isNull() ? "Graph" : graph.title().data());
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
  drawList->PopClipRect();

  // mouse: select, context menu, coordinates
  _hoverText = "";
  if (hovered)
    {
    ImVec2 mouse = ImGui::GetMousePos();
    Point2D p = painter.toCanvas(mouse.x, mouse.y);
    int k = _canvas.graphAt(p);
    if (k >= 0)
      {
      const Graph & g = _canvas.graph(k);
      if (g.frameContains(p))
        {
        double x, y;
        g.fromCanvas(g.view(), p, x, y);
        char text[96];
        std::snprintf(text, sizeof(text), "x = %.5g   y = %.5g", x, y);
        _hoverText = text;
        }
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))  { _selectedGraph = k; clicked = true; }
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) { _selectedGraph = k; _contextGraph = k; clicked = true; ImGui::OpenPopup("graphContext"); }
      }
    }
  renderContextMenu();
  ImGui::PopID();
  return clicked;
  }

  } // namespace GUI
} // namespace CAP
