/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SpreadsheetPane.cpp
//! @brief Implementation of SpreadsheetPane.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include "imgui.h"
#include "SpreadsheetPane.hpp"
#include "TableWriter.hpp"

namespace Datum
{
  namespace GUI
  {

  namespace
  {
  constexpr unsigned int kMaximumColumns = 511;   // ImGui tables: at most 512 columns, the index column included

  std::string joined(const MATH::Table2D & table, const std::vector<unsigned int> & columns, std::size_t maximum=4)
  {
  std::string text;
  for (std::size_t k=0; k<columns.size() && k<maximum; k++)
    text += (k ? ", " : "") + std::string(table.columnName(columns[k]).data());
  if (columns.size() > maximum) text += ", ... (" + std::to_string(columns.size()) + ")";
  return text;
  }
  } // namespace

  SpreadsheetPane::SpreadsheetPane(const String & name)
  :
  _name(name),
  _table(),
  _label(),
  _fileName(),
  _rowLabels(),
  _selected()
  { }

  void SpreadsheetPane::setTable(std::unique_ptr<MATH::Table2D> table, const String & label, const String & fileName,
                                 const std::vector<String> & rowLabels)
  {
  _table = std::move(table);
  _label = label;
  _fileName = fileName;
  _rowLabels = rowLabels;
  _selected.assign(_table ? _table->nColumns() : 0, false);
  _anchor = _xColumn = _errorColumn = _menuColumn = -1;
  }

  String SpreadsheetPane::windowTitle() const
  {
  // "###name" keeps the window (and its place in the layout) when the label changes
  return (_label.isNull() ? _name : _label) + String("###") + _name;
  }

  std::vector<unsigned int> SpreadsheetPane::selectedColumns() const
  {
  std::vector<unsigned int> columns;
  for (std::size_t c=0; c<_selected.size(); c++) if (_selected[c]) columns.push_back(unsigned(c));
  return columns;
  }

  void SpreadsheetPane::selectColumns(const std::vector<unsigned int> & columns)
  {
  std::fill(_selected.begin(), _selected.end(), false);
  for (unsigned int c : columns) if (c < _selected.size()) _selected[c] = true;
  }

  std::vector<unsigned int> SpreadsheetPane::plottedColumns() const
  {
  std::vector<unsigned int> columns;
  for (unsigned int c : selectedColumns()) if (int(c) != _xColumn && int(c) != _errorColumn) columns.push_back(c);
  if (columns.empty() && _menuColumn >= 0 && _menuColumn != _xColumn) columns.push_back(unsigned(_menuColumn));
  return columns;
  }

  void SpreadsheetPane::clickColumn(int column)
  {
  if (column < 0 || column >= int(_selected.size())) return;
  const ImGuiIO & io = ImGui::GetIO();
  bool toggle = io.KeyCtrl || io.KeySuper;
  if (io.KeyShift && _anchor >= 0)
    {
    if (!toggle) std::fill(_selected.begin(), _selected.end(), false);
    for (int c=std::min(_anchor, column); c<=std::max(_anchor, column); c++) _selected[std::size_t(c)] = true;
    return;   // the anchor stays, so that the range can be changed
    }
  if (toggle) _selected[std::size_t(column)] = !_selected[std::size_t(column)];
  else
    {
    bool only = _selected[std::size_t(column)] && selectedColumns().size() == 1;
    std::fill(_selected.begin(), _selected.end(), false);
    _selected[std::size_t(column)] = !only;   // clicking the only selected column deselects it
    }
  _anchor = column;
  }

  String SpreadsheetPane::text() const
  {
  if (!_table) return String();
  const MATH::Table2D & t = *_table;
  std::string out = _rowLabels.empty() ? "" : "\t";
  for (unsigned int c=0; c<t.nColumns(); c++) out += (c ? "\t" : "") + std::string(t.columnName(c).data());
  out += "\n";
  for (unsigned int i=0; i<t.nRows(); i++)
    {
    if (!_rowLabels.empty()) out += std::string(i < _rowLabels.size() ? _rowLabels[i].data() : "") + "\t";
    for (unsigned int c=0; c<t.nColumns(); c++)
      {
      if (c) out += "\t";
      if (!std::isnan(t(i, c))) out += TableWriter::formatNumber(t(i, c), 10);
      }
    out += "\n";
    }
  return String(out.c_str());
  }

  void SpreadsheetPane::plot(PlotRequest request)
  {
  request.overlay = _overlay;
  if (onPlot) onPlot(*this, request);
  }

  void SpreadsheetPane::render()
  {
  if (!_table) { ImGui::TextDisabled("No table"); return; }
  const MATH::Table2D & t = *_table;
  if (_selected.size() != t.nColumns()) _selected.assign(t.nColumns(), false);

  // heading: name and size, file; Copy and Save As on the right
  ImGui::TextUnformatted(_label.data());
  ImGui::SameLine();
  ImGui::TextDisabled("%u rows x %u columns", t.nRows(), t.nColumns());
  if (!_fileName.isNull()) { ImGui::SameLine(); ImGui::TextDisabled("— %s", _fileName.data()); }
  const ImGuiStyle & style = ImGui::GetStyle();
  float buttons = ImGui::CalcTextSize("Copy").x + ImGui::CalcTextSize("Save As...").x + 4*style.FramePadding.x + style.ItemSpacing.x;
  ImGui::SameLine();
  float available = ImGui::GetContentRegionAvail().x;
  if (available > buttons) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available - buttons);
  if (ImGui::Button("Copy")) ImGui::SetClipboardText(text().data());
  ImGui::SetItemTooltip("Copy the table (tab-separated, with its header row) to the clipboard");
  ImGui::SameLine();
  if (ImGui::Button("Save As...") && onSaveAs) onSaveAs(*this);
  ImGui::SetItemTooltip("Save the table: CSV, tab or blank separated text, Datum binary, or Excel (.xlsx)");

  // what is selected
  std::vector<unsigned int> selected = selectedColumns();
  if (selected.empty() && _xColumn < 0)
    ImGui::TextDisabled("Click the names of columns to select them; right-click for plots, correlations and statistics");
  else
    {
    std::string line;
    if (!selected.empty()) line += "Selected: " + joined(t, selected);
    if (_xColumn >= 0) line += std::string(line.empty() ? "" : "    ") + "X: " + t.columnName(unsigned(_xColumn)).data();
    if (_errorColumn >= 0) line += std::string("    ± y: ") + t.columnName(unsigned(_errorColumn)).data();
    ImGui::TextDisabled("%s", line.c_str());
    }

  // table: header row and index column frozen; only the visible rows are drawn
  unsigned int nShown = std::min(t.nColumns(), kMaximumColumns);
  ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter
                        | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;
  bool openMenu = false;
  // the number of columns is in the identifier: a table whose columns change starts afresh
  std::string id = "##sheet" + std::to_string(nShown);
  if (ImGui::BeginTable(id.c_str(), int(nShown) + 1, flags, ImVec2(0.0f, 0.0f)))
    {
    ImGui::TableSetupScrollFreeze(1, 1);
    float digits = ImGui::CalcTextSize("-1.23457e+06").x;
    float indexWidth = ImGui::CalcTextSize((std::to_string(t.nRows()) + "0").c_str()).x;
    for (const auto & label : _rowLabels) indexWidth = std::max(indexWidth, ImGui::CalcTextSize(label.data()).x);
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide, indexWidth);
    for (unsigned int c=0; c<nShown; c++)
      {
      float width = std::max(digits, ImGui::CalcTextSize(t.columnName(c).data()).x + ImGui::CalcTextSize(" [x]").x);
      ImGui::TableSetupColumn(t.columnName(c).data(), ImGuiTableColumnFlags_WidthFixed, width);
      }
    const ImU32 selectedColor = ImGui::GetColorU32(ImGuiCol_Header, 0.55f);
    const ImU32 xColor = ImGui::GetColorU32(ImGuiCol_PlotHistogram, 0.25f);

    // header row: the names of the columns; a click selects, a right-click selects and opens the menu. The cells are
    // selectables rather than TableHeader(), whose right-click opens the table's own context menu.
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    ImGui::TableSetColumnIndex(0);
    ImGui::TextDisabled("#");
    for (unsigned int c=0; c<nShown; c++)
      {
      ImGui::TableSetColumnIndex(int(c) + 1);
      if (int(c) == _xColumn) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, xColor);
      std::string label = t.columnName(c).data();
      if (int(c) == _xColumn) label += " [x]";
      if (int(c) == _errorColumn) label += " [\xC2\xB1]";
      label += "##column" + std::to_string(c);
      if (ImGui::Selectable(label.c_str(), bool(_selected[c]))) clickColumn(int(c));
      if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        {
        if (!_selected[c]) { std::fill(_selected.begin(), _selected.end(), false); _selected[c] = true; _anchor = int(c); }
        _menuColumn = int(c);
        openMenu = true;
        }
      if (ImGui::IsItemHovered()) ImGui::SetItemTooltip("%s", t.columnName(c).data());
      }

    ImGuiListClipper clipper;
    clipper.Begin(int(t.nRows()));
    char cell[32];
    while (clipper.Step())
      for (int i=clipper.DisplayStart; i<clipper.DisplayEnd; i++)
        {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (std::size_t(i) < _rowLabels.size()) ImGui::TextUnformatted(_rowLabels[std::size_t(i)].data());
        else ImGui::TextDisabled("%d", i);
        for (unsigned int c=0; c<nShown; c++)
          {
          ImGui::TableNextColumn();
          if (_selected[c]) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, selectedColor);
          else if (int(c) == _xColumn) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, xColor);
          double v = t(unsigned(i), c);
          if (std::isnan(v)) continue;   // missing value: empty cell
          std::snprintf(cell, sizeof(cell), "%.6g", v);
          ImGui::TextUnformatted(cell);
          }
        }
    // right-click in the body: the menu, for the column under the mouse
    int hovered = ImGui::TableGetHoveredColumn();
    if (!openMenu && hovered >= 0 && ImGui::IsMouseReleased(ImGuiMouseButton_Right)
        && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
      {
      _menuColumn = (hovered >= 1 && hovered <= int(nShown)) ? hovered - 1 : -1;
      openMenu = true;
      }
    ImGui::EndTable();
    }
  if (t.nColumns() > nShown)
    ImGui::TextDisabled("Only the first %u columns are shown (of %u); plots and statistics use them all", nShown, t.nColumns());
  if (openMenu) ImGui::OpenPopup("##sheetMenu");
  renderMenu();
  }

  void SpreadsheetPane::renderMenu()
  {
  if (!ImGui::BeginPopup("##sheetMenu")) return;
  const MATH::Table2D & t = *_table;
  std::vector<unsigned int> y = plottedColumns();
  std::string ys = y.empty() ? std::string("(select columns)") : joined(t, y, 3);
  bool hasY = !y.empty();

  if (_menuColumn >= 0)
    {
    std::string name = t.columnName(unsigned(_menuColumn)).data();
    ImGui::TextDisabled("Column %s", name.c_str());
    if (_menuColumn != _xColumn)
      {
      if (ImGui::MenuItem(("Use " + name + " as X").c_str()))
        {
        _xColumn = _menuColumn;
        _selected[std::size_t(_menuColumn)] = false;
        if (_errorColumn == _xColumn) _errorColumn = -1;
        }
      }
    else if (ImGui::MenuItem("Do not use as X")) _xColumn = -1;
    if (_menuColumn != _errorColumn)
      {
      if (ImGui::MenuItem(("Use " + name + " as the uncertainty of y").c_str()))
        {
        _errorColumn = _menuColumn;
        _selected[std::size_t(_menuColumn)] = false;
        if (_xColumn == _errorColumn) _xColumn = -1;
        }
      }
    else if (ImGui::MenuItem("Do not use as the uncertainty of y")) _errorColumn = -1;
    ImGui::Separator();
    }

  // y vs x
  ImGui::TextDisabled("Y: %s", ys.c_str());
  if (_xColumn >= 0)
    {
    std::string label = "Plot Y vs " + std::string(t.columnName(unsigned(_xColumn)).data());
    if (ImGui::MenuItem(label.c_str(), nullptr, false, hasY))
      {
      PlotRequest r; r.kind = kXY; r.columns = y; r.x = _xColumn; r.yError = _errorColumn; plot(r);
      }
    }
  if (ImGui::BeginMenu("Plot Y vs", hasY))
    {
    if (ImGui::MenuItem("row index"))
      {
      PlotRequest r; r.kind = kXY; r.columns = y; r.x = -1; r.yError = _errorColumn; plot(r);
      }
    ImGui::Separator();
    for (unsigned int c=0; c<t.nColumns(); c++)
      {
      if (std::find(y.begin(), y.end(), c) != y.end()) continue;
      std::string label = std::string(t.columnName(c).data()) + "##x" + std::to_string(c);
      if (ImGui::MenuItem(label.c_str()))
        {
        PlotRequest r; r.kind = kXY; r.columns = y; r.x = int(c); r.yError = _errorColumn; plot(r);
        }
      }
    ImGui::EndMenu();
    }
  // bars
  if (ImGui::MenuItem("Bar Chart of Y", nullptr, false, hasY))
    {
    PlotRequest r; r.kind = kBars; r.columns = y; r.x = -1; plot(r);
    }
  ImGui::SetItemTooltip("One bar per row, at the row index; several columns side by side");
  if (_xColumn >= 0)
    {
    std::string label = "Bar Chart of Y vs " + std::string(t.columnName(unsigned(_xColumn)).data());
    if (ImGui::MenuItem(label.c_str(), nullptr, false, hasY))
      {
      PlotRequest r; r.kind = kBars; r.columns = y; r.x = _xColumn; plot(r);
      }
    }
  if (ImGui::MenuItem("Distribution of Y", nullptr, false, hasY))
    {
    PlotRequest r; r.kind = kDistribution; r.columns = y; plot(r);
    }
  ImGui::SetItemTooltip("Histogram of the values of each column");
  ImGui::Separator();

  // statistics
  std::vector<unsigned int> selected = selectedColumns();
  std::vector<unsigned int> matrix = selected.size() >= 2 ? selected : std::vector<unsigned int>();
  if (ImGui::BeginMenu("Correlations", t.nColumns() >= 2))
    {
    ImGui::TextDisabled("%s", matrix.empty() ? "of all the columns" : ("of " + joined(t, matrix, 3)).c_str());
    static const char * methods[] = { "Pearson", "Spearman (ranks)", "Kendall (tau-b)" };
    for (int m=0; m<3; m++)
      if (ImGui::MenuItem(methods[m]))
        {
        PlotRequest r; r.kind = kCorrelation; r.columns = matrix; r.method = m; plot(r);
        }
    ImGui::EndMenu();
    }
  if (ImGui::MenuItem("Summary Statistics"))
    {
    PlotRequest r; r.kind = kSummary; r.columns = selected; plot(r);
    }
  ImGui::SetItemTooltip("n, missing values, mean, standard deviation, minimum, median, maximum of each column");
  ImGui::Separator();
  ImGui::MenuItem("Overlay Plots", nullptr, &_overlay);
  ImGui::SetItemTooltip("Add the plots to the graph (otherwise they replace its contents)");
  if (ImGui::MenuItem("Select All Columns")) std::fill(_selected.begin(), _selected.end(), true);
  if (ImGui::MenuItem("Clear Selection")) { std::fill(_selected.begin(), _selected.end(), false); _xColumn = _errorColumn = -1; }
  ImGui::Separator();
  if (ImGui::MenuItem("Copy Table")) ImGui::SetClipboardText(text().data());
  if (ImGui::MenuItem("Save Table As...") && onSaveAs) onSaveAs(*this);
  ImGui::EndPopup();
  }

  } // namespace GUI
} // namespace Datum
