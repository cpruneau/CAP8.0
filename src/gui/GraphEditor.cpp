#include <cmath>
#include <cstdio>
#include <string>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "GraphEditor.hpp"
#include "ImGuiPainter.hpp"
#include "DataPoints.hpp"

namespace CAP
{
  namespace GUI
  {
  namespace GraphEditor
  {
  using namespace GRAPHICS;

  namespace
  {
  ImVec4 rgba(int color)
  {
  float r, g, b;
  Painter::rgb(color, r, g, b);
  return ImVec4(r, g, b, 1.0f);
  }

  //! Edit a String through a std::string buffer. @return true if the text changed
  bool textEdit(const char * label, String & text)
  {
  std::string buffer = text.str();
  if (!ImGui::InputText(label, &buffer)) return false;
  text = String(buffer);
  return true;
  }

  //! Section header inside a dialog.
  void section(const char * name)
  {
  ImGui::Spacing();
  ImGui::SeparatorText(name);
  }

  //! Position of a text: displacement from the automatic position, in fractions of the cell. @return true if changed
  bool offsetEdit(float & dx, float & dy)
  {
  float offset[2] = { dx, dy };
  bool changed = false;
  ImGui::SetNextItemWidth(160);
  if (ImGui::DragFloat2("offset", offset, 0.002f, -1.0f, 1.0f, "%.3f")) { dx = offset[0]; dy = offset[1]; changed = true; }
  ImGui::SetItemTooltip("Displacement from the automatic position, in fractions of the graph (or drag the text with the mouse)");
  ImGui::SameLine();
  if (ImGui::Button("Reset position")) { dx = 0; dy = 0; changed = true; }
  return changed;
  }

  //! Common part of the title editors: text, color, font, size.
  bool textStyleEdit(String & title, TextAttributes & attributes, float & points)
  {
  bool changed = false;
  ImGui::SetNextItemWidth(260);
  changed |= textEdit("text", title);
  int color = attributes.color();
  if (colorEdit("color", color)) { attributes.setColor(color, attributes.alpha()); changed = true; }
  ImGui::SameLine(0, 20);
  int font = attributes.font();
  if (fontEdit("font", font)) { attributes.setFont(font); changed = true; }
  changed |= pointSizeEdit("size", points);
  return changed;
  }

  const char * axisName(int axis) { return axis == 0 ? "x" : (axis == 1 ? "y" : "z"); }
  } // namespace

  // ------------------------------------------------------------------------------------------------ widgets

  bool colorEdit(const char * label, int & color)
  {
  bool changed = false;
  ImGui::PushID(label);
  if (ImGui::ColorButton("##swatch", rgba(color), ImGuiColorEditFlags_NoTooltip, ImVec2(ImGui::GetFrameHeight()*1.6f, ImGui::GetFrameHeight())))
    ImGui::OpenPopup("palette");
  ImGui::SetItemTooltip("ROOT color %d: click to choose", color);
  ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
  ImGui::TextUnformatted(label);
  if (ImGui::BeginPopup("palette"))
    {
    // ROOT's basic colors, then each named hue (kRed, kPink, ...) from light (-9) to dark (+3)
    static const int basic[] = { 0, 1, 920, 921, 922, 923, 2, 3, 4, 5, 6, 7, 8, 9 };
    static const int hues[]  = { 632, 900, 616, 880, 600, 860, 432, 840, 416, 820, 400, 800 };
    static const int shades[] = { -9, -7, -4, -2, 0, 1, 2, 3 };
    const float size = ImGui::GetFrameHeight();
    auto swatch = [&](int c, bool sameLine)
      {
      if (sameLine) ImGui::SameLine(0, 2);
      ImGui::PushID(c);
      if (ImGui::ColorButton("##c", rgba(c), ImGuiColorEditFlags_NoTooltip, ImVec2(size, size))) { color = c; changed = true; ImGui::CloseCurrentPopup(); }
      ImGui::SetItemTooltip("%d", c);
      if (c == color) ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(40, 110, 220, 255), 0, 0, 2.0f);
      ImGui::PopID();
      };
    for (size_t k=0; k<sizeof(basic)/sizeof(int); k++) swatch(basic[k], k > 0);
    for (int hue : hues)
      for (size_t k=0; k<sizeof(shades)/sizeof(int); k++) swatch(hue + shades[k], k > 0);
    ImGui::Separator();
    ImGui::SetNextItemWidth(100);
    int index = color;
    if (ImGui::InputInt("ROOT color index", &index) && index >= 0) { color = index; changed = true; }
    ImGui::EndPopup();
    }
  ImGui::PopID();
  return changed;
  }

  bool fontEdit(const char * label, int & font)
  {
  static const char * families[] = { "Sans (Helvetica)", "Serif (Times)", "Monospace (Courier)", "Symbol" };
  TextAttributes::FontFamily family = TextAttributes::fontFamily(font);
  bool bold = TextAttributes::isBold(font), italic = TextAttributes::isItalic(font);
  bool changed = false;
  ImGui::PushID(label);
  int f = int(family);
  ImGui::SetNextItemWidth(170);
  if (ImGui::Combo("##family", &f, families, 4)) { family = TextAttributes::FontFamily(f); changed = true; }
  ImGui::SameLine();
  changed |= ImGui::Checkbox("bold", &bold);
  ImGui::SameLine();
  changed |= ImGui::Checkbox("italic", &italic);
  ImGui::SameLine();
  ImGui::TextUnformatted(label);
  if (changed) font = TextAttributes::fontCode(family, bold, italic);
  ImGui::PopID();
  return changed;
  }

  bool markerEdit(const char * label, int & style, int color)
  {
  struct Marker { int style; const char * name; };
  static const Marker markers[] = {
    {20, "full circle"}, {21, "full square"}, {22, "full triangle up"}, {23, "full triangle down"},
    {33, "full diamond"}, {29, "full star"}, {24, "open circle"}, {25, "open square"}, {26, "open triangle up"},
    {32, "open triangle down"}, {27, "open diamond"}, {30, "open star"}, {2, "plus"}, {5, "cross"}, {3, "asterisk"},
    {1, "dot"} };
  const char * current = "other";
  for (const auto & m : markers) if (m.style == style) current = m.name;
  bool changed = false;
  // preview of a marker drawn by the plot painter in a small square
  auto preview = [&](int s)
    {
    float h = ImGui::GetTextLineHeight();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGuiPainter painter(ImGui::GetWindowDrawList(), nullptr, p.x, p.y, h, h);
    MarkerAttributes marker;
    marker.set(color, 1.0, s, 1.2);
    painter.drawMarker(Point2D(0.5, 0.5), marker);
    ImGui::Dummy(ImVec2(h, h));
    ImGui::SameLine();
    };
  ImGui::PushID(label);
  preview(style);
  ImGui::SetNextItemWidth(170);
  if (ImGui::BeginCombo(label, current))
    {
    for (const auto & m : markers)
      {
      preview(m.style);
      if (ImGui::Selectable(m.name, m.style == style)) { style = m.style; changed = true; }
      }
    ImGui::EndCombo();
    }
  ImGui::PopID();
  return changed;
  }

  bool pointSizeEdit(const char * label, float & points)
  {
  bool changed = false;
  ImGui::PushID(label);
  bool automatic = points <= 0;
  if (ImGui::Checkbox("automatic", &automatic)) { points = automatic ? 0.0f : 14.0f; changed = true; }
  ImGui::SetItemTooltip("Size following the size of the graph");
  if (!automatic)
    {
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    changed |= ImGui::DragFloat("##points", &points, 0.2f, 4.0f, 72.0f, "%.1f pt");
    }
  ImGui::SameLine();
  ImGui::TextUnformatted(label);
  ImGui::PopID();
  return changed;
  }

  bool lineStyleEdit(const char * label, int & style, bool allowNone)
  {
  static const char * names[] = { "none", "solid", "dashed", "dotted", "dash-dot" };
  int s = (style >= 0 && style <= 4) ? style : 1;
  ImGui::SetNextItemWidth(110);
  bool changed;
  if (allowNone) changed = ImGui::Combo(label, &s, names, 5);
  else { int t = s - 1; if (t < 0) t = 0; changed = ImGui::Combo(label, &t, names + 1, 4); s = t + 1; }
  if (changed) style = s;
  return changed;
  }

  // ------------------------------------------------------------------------------------------------ editors

  bool editAxis(Graph & graph, int k)
  {
  Axis & axis = graph.axis(k);
  bool changed = false;
  ImGui::Text("%s axis", axisName(k));
  if (!axis.title().isNull()) { ImGui::SameLine(); ImGui::TextDisabled("(%s)", axis.title().data()); }

  section("Range");
  const Graph::View view = graph.view();
  bool automatic = (k == 0) ? graph.isAutoX() : graph.isAutoY();
  float range[2] = { k == 0 ? view.xMinimum : view.yMinimum, k == 0 ? view.xMaximum : view.yMaximum };
  bool log = (k == 0) ? graph.isLogX() : graph.isLogY();
  if (ImGui::Checkbox("automatic", &automatic))
    {
    if (automatic) { if (k == 0) graph.setAutoX(); else graph.setAutoY(); }
    else if (k == 0) graph.setXRange(range[0], range[1]); else graph.setYRange(range[0], range[1]);
    changed = true;
    }
  ImGui::SameLine(0, 20);
  if (ImGui::Checkbox("logarithmic", &log)) { if (k == 0) graph.setLogX(log); else graph.setLogY(log); changed = true; }
  ImGui::SetNextItemWidth(240);
  if (ImGui::InputFloat2("minimum, maximum", range, "%.5g", ImGuiInputTextFlags_EnterReturnsTrue) && range[1] > range[0]
      && (!log || range[0] > 0))
    {
    if (k == 0) graph.setXRange(range[0], range[1]); else graph.setYRange(range[0], range[1]);
    changed = true;
    }
  ImGui::SetItemTooltip("Press Enter to apply (or drag along the axis with the mouse; double-click the frame for automatic ranges)");

  section("Line and ticks");
  int lineColor = axis.lineAttributes().color();
  if (colorEdit("line color", lineColor)) { axis.lineAttributes().setColor(lineColor, axis.lineAttributes().alpha()); changed = true; }
  ImGui::SameLine(0, 20);
  float width = axis.lineAttributes().width();
  ImGui::SetNextItemWidth(90);
  if (ImGui::DragFloat("width", &width, 0.05f, 0.5f, 6.0f, "%.1f")) { axis.lineAttributes().setWidth(width); changed = true; }
  bool major = axis.showMajorTicks(), minor = axis.showMinorTicks();
  if (ImGui::Checkbox("major ticks", &major)) { axis.setShowMajorTicks(major); changed = true; }
  ImGui::SameLine();
  if (ImGui::Checkbox("minor ticks", &minor)) { axis.setShowMinorTicks(minor); changed = true; }

  section("Labels");
  bool labels = axis.showLabels();
  if (ImGui::Checkbox("show labels", &labels)) { axis.setShowLabels(labels); changed = true; }
  int labelColor = axis.textAttributes().color();
  if (colorEdit("label color", labelColor)) { axis.textAttributes().setColor(labelColor, axis.textAttributes().alpha()); changed = true; }
  ImGui::SameLine(0, 20);
  int font = axis.textAttributes().font();
  if (fontEdit("font", font)) { axis.textAttributes().setFont(font); changed = true; }
  float size = axis.labelPointSize();
  if (pointSizeEdit("label size", size)) { axis.setLabelPointSize(size); changed = true; }
  float gap = axis.labelGap();
  ImGui::SetNextItemWidth(160);
  if (ImGui::SliderFloat("distance from the axis", &gap, 0.0f, 5.0f, "%.2f x")) { axis.setLabelGap(gap); changed = true; }
  ImGui::SetItemTooltip("Relative to the automatic distance (or Shift + drag the labels with the mouse)");
  return changed;
  }

  bool editAxisTitle(Graph & graph, int k)
  {
  Axis & axis = graph.axis(k);
  ImGui::Text("Title of the %s axis", axisName(k));
  ImGui::Separator();
  String title = axis.title();
  float points = axis.titlePointSize();
  bool changed = textStyleEdit(title, axis.titleAttributes(), points);
  if (changed) { axis.setTitle(title); axis.setTitlePointSize(points); }
  float dx = axis.titleOffsetX(), dy = axis.titleOffsetY();
  if (offsetEdit(dx, dy)) { axis.setTitleOffset(dx, dy); changed = true; }
  return changed;
  }

  bool editTitle(Graph & graph)
  {
  ImGui::TextUnformatted("Title of the graph");
  ImGui::Separator();
  String title = graph.title();
  float points = graph.titlePointSize();
  bool changed = textStyleEdit(title, graph.titleAttributes(), points);
  if (changed) { graph.setTitle(title); graph.setTitlePointSize(points); }
  float dx = graph.titleOffsetX(), dy = graph.titleOffsetY();
  if (offsetEdit(dx, dy)) { graph.setTitleOffset(dx, dy); changed = true; }
  return changed;
  }

  bool editLegend(Graph & graph)
  {
  bool changed = false;
  ImGui::TextUnformatted("Legend");
  ImGui::Separator();
  bool show = graph.showLegend();
  if (ImGui::Checkbox("show", &show)) { graph.setShowLegend(show); changed = true; }
  float points = graph.legendPointSize();
  if (pointSizeEdit("text size", points)) { graph.setLegendPointSize(points); changed = true; }

  section("Position");
  bool automatic = graph.isAutoLegendPosition();
  if (ImGui::Checkbox("upper right corner of the frame", &automatic))
    {
    if (automatic) graph.setAutoLegendPosition();
    else
      {
      // start from where the legend is now
      const Rect2D & r = graph.legendRect();
      float w = graph.frameXHigh() - graph.frameXLow(), h = graph.frameYHigh() - graph.frameYLow();
      if (!r.empty && w > 0 && h > 0) graph.setLegendPosition((r.x0 - graph.frameXLow())/w, (r.y1 - graph.frameYLow())/h);
      else graph.setLegendPosition(graph.legendX(), graph.legendY());
      }
    changed = true;
    }
  if (!automatic)
    {
    float position[2] = { graph.legendX(), graph.legendY() };
    ImGui::SetNextItemWidth(160);
    if (ImGui::DragFloat2("upper left corner", position, 0.005f, -0.5f, 1.5f, "%.3f"))
      { graph.setLegendPosition(position[0], position[1]); changed = true; }
    ImGui::SetItemTooltip("In fractions of the frame (or drag the legend with the mouse)");
    }

  section("Box");
  bool filled = graph.legendFill().style() != 0;
  if (ImGui::Checkbox("background", &filled)) { graph.legendFill().setStyle(filled ? 1001 : 0); changed = true; }
  if (filled)
    {
    ImGui::SameLine();
    int color = graph.legendFill().color();
    if (colorEdit("##background", color)) { graph.legendFill().setColor(color, graph.legendFill().alpha()); changed = true; }
    }
  int style = graph.legendLine().style();
  if (lineStyleEdit("border", style)) { graph.legendLine().setStyle(style); changed = true; }
  if (style != 0)
    {
    ImGui::SameLine();
    int color = graph.legendLine().color();
    if (colorEdit("##border", color)) { graph.legendLine().setColor(color, graph.legendLine().alpha()); changed = true; }
    }
  return changed;
  }

  bool editItem(Graph & graph, int k, int point)
  {
  if (k < 0 || k >= int(graph.items().size())) return false;
  GraphItem & item = graph.items()[k];
  bool changed = false;
  ImGui::TextUnformatted(item.kind() == GraphItem::kData ? "Data points" : "Function");
  if (item.kind() == GraphItem::kData && item.data() && point >= 0 && point < int(item.data()->size()))
    {
    MATH::DataPoint2D p = item.data()->point(point);
    ImGui::TextDisabled("point %d:  x = %.5g  y = %.5g  \xC2\xB1 %.3g (stat.)  \xC2\xB1 %.3g (syst.)",
                        point, p.x(), p.y(), p.yStatError(), p.ySysError());
    }
  ImGui::Separator();
  String label = item.label();
  ImGui::SetNextItemWidth(260);
  if (textEdit("legend label", label)) { item.setLabel(label); changed = true; }

  if (item.kind() == GraphItem::kData)
    {
    section("Markers");
    bool markers = item.showMarkers();
    if (ImGui::Checkbox("show markers", &markers)) { item.setShowMarkers(markers); changed = true; }
    MarkerAttributes & marker = item.markerAttributes();
    int style = marker.style();
    if (markerEdit("symbol", style, marker.color())) { marker.setStyle(style); changed = true; }
    float size = marker.size();
    ImGui::SetNextItemWidth(160);
    if (ImGui::SliderFloat("size", &size, 0.2f, 4.0f, "%.2f")) { marker.setSize(size); changed = true; }
    int color = marker.color();
    if (colorEdit("color", color)) { marker.setColor(color, marker.alpha()); changed = true; }
    ImGui::SetItemTooltip("Color of the markers and error bars");

    section("Uncertainties");
    bool stat = item.showErrors();
    if (ImGui::Checkbox("statistical (and x) error bars", &stat)) { item.setShowErrors(stat); changed = true; }
    bool sys = item.showSysErrors();
    if (ImGui::Checkbox("systematic uncertainties", &sys)) { item.setShowSysErrors(sys); changed = true; }
    if (sys)
      {
      ImGui::Indent();
      static const char * styles[] = { "boxes", "brackets", "band" };
      int s = int(item.sysStyle());
      ImGui::SetNextItemWidth(120);
      if (ImGui::Combo("drawn as", &s, styles, 3)) { item.setSysStyle(GraphItem::SysStyle(s)); changed = true; }
      bool same = item.sysUsesMarkerColor();
      if (ImGui::Checkbox("color of the markers", &same)) { item.setSysUsesMarkerColor(same); changed = true; }
      if (!same)
        {
        int fillColor = item.sysFillAttributes().color(), lineColor = item.sysLineAttributes().color();
        if (colorEdit("fill", fillColor)) { item.sysFillAttributes().setColor(fillColor, item.sysFillAttributes().alpha()); changed = true; }
        ImGui::SameLine(0, 20);
        if (colorEdit("outline", lineColor)) { item.sysLineAttributes().setColor(lineColor, item.sysLineAttributes().alpha()); changed = true; }
        }
      float opacity = item.sysFillAttributes().alpha();
      bool filled = item.sysFillAttributes().style() != 0;
      if (ImGui::Checkbox("filled", &filled)) { item.sysFillAttributes().setStyle(filled ? 1001 : 0); changed = true; }
      if (filled)
        {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120);
        if (ImGui::SliderFloat("opacity", &opacity, 0.05f, 1.0f, "%.2f")) { item.sysFillAttributes().setAlpha(opacity); changed = true; }
        }
      int outline = item.sysLineAttributes().style();
      if (lineStyleEdit("outline style", outline)) { item.sysLineAttributes().setStyle(outline); changed = true; }
      float width = item.sysWidth();
      ImGui::SetNextItemWidth(160);
      if (ImGui::SliderFloat("half width", &width, 0.002f, 0.05f, "%.3f")) { item.setSysWidth(width); changed = true; }
      ImGui::SetItemTooltip("For points without x uncertainty, in fractions of the frame width");
      ImGui::Unindent();
      }
    }

  section(item.kind() == GraphItem::kData ? "Line through the points" : "Curve");
  bool line = item.showLine();
  if (ImGui::Checkbox("show line", &line)) { item.setShowLine(line); changed = true; }
  LineAttributes & attributes = item.lineAttributes();
  int lineColor = attributes.color();
  if (colorEdit("line color", lineColor)) { attributes.setColor(lineColor, attributes.alpha()); changed = true; }
  ImGui::SameLine(0, 20);
  int lineStyle = attributes.style();
  if (lineStyleEdit("style", lineStyle, false)) { attributes.setStyle(lineStyle); changed = true; }
  float lineWidth = attributes.width();
  ImGui::SetNextItemWidth(160);
  if (ImGui::SliderFloat("width", &lineWidth, 0.5f, 6.0f, "%.1f")) { attributes.setWidth(lineWidth); changed = true; }
  if (item.kind() == GraphItem::kFunction)
    {
    float range[2] = { item.xMinimum(), item.xMaximum() };
    ImGui::SetNextItemWidth(240);
    if (ImGui::InputFloat2("x range of the curve", range, "%.5g", ImGuiInputTextFlags_EnterReturnsTrue) && range[1] > range[0])
      { item.setXRange(range[0], range[1]); changed = true; }
    }

  ImGui::Spacing();
  ImGui::Separator();
  if (ImGui::Button("Remove from the graph")) { graph.items().erase(graph.items().begin() + k); changed = true; ImGui::CloseCurrentPopup(); }
  return changed;
  }

  } // namespace GraphEditor
  } // namespace GUI
} // namespace CAP
