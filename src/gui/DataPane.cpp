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
//! @file DataPane.cpp
//! @brief Implementation of DataPane.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "imgui.h"
#include "DataPane.hpp"
#include "DataPoints.hpp"
#include "FitRecord.hpp"
#include "Graph.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"

namespace CAP
{
  namespace GUI
  {

  namespace
  {
  std::vector<std::string> strings(const std::vector<String> & texts)
  {
  std::vector<std::string> result;
  for (const auto & text : texts) result.push_back(text.data());
  return result;
  }
  } // namespace

  DataPane::DataPane(const String & name)
  :
  _name(name),
  _data(nullptr),
  _label()
  { }

  void DataPane::setData(const MATH::DataPoints * data, const String & label)
  {
  _data  = data;
  _label = label;
  _shown.clear();
  if (_data) _shown = _data->columnsShown();   // uncertainties zero for every point are left out
  }

  String DataPane::windowTitle() const
  {
  // "###name" keeps the window (and its place in the layout) when the label changes
  return (_label.isNull() ? _name : _label) + String("###") + _name;
  }

  String DataPane::text() const
  {
  return _data ? _data->table(true, _showPulls) : String();
  }

  void DataPane::render()
  {
  if (!_data) { ImGui::TextDisabled("No data"); return; }
  const MATH::DataPoints & data = *_data;

  // heading: name, kind and size, title; Copy on the right
  ImGui::TextUnformatted(_label.data());
  ImGui::SameLine();
  ImGui::TextDisabled("%s", data.description().data());
  if (!data.title().isNull() && !data.title().equalTo(_label))
    {
    ImGui::SameLine();
    ImGui::TextDisabled("— %s", data.title().data());
    }
  float buttonWidth = ImGui::CalcTextSize("Copy").x + 2*ImGui::GetStyle().FramePadding.x;
  ImGui::SameLine();
  float available = ImGui::GetContentRegionAvail().x;
  if (available > buttonWidth) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available - buttonWidth);
  if (ImGui::Button("Copy")) ImGui::SetClipboardText(text().data());
  ImGui::SetItemTooltip("Copy the table (tab-separated, with its header row) to the clipboard");
  if (data.nFits() > 0)
    {
    // the fits: label and quality of each, and whether the pulls are shown
    for (unsigned int f=0; f<data.nFits(); f++)
      {
      const MATH::FitRecord & fit = data.fit(f);
      if (f > 0) { ImGui::SameLine(); ImGui::TextDisabled("|"); ImGui::SameLine(); }
      ImGui::TextDisabled("%s: %s, p = %.3g", fit.label.isNull() ? "fit" : fit.label.data(),
                          GRAPHICS::GraphItem::fitSummary(fit).data(), fit.result.pValue);
      }
    ImGui::SameLine(0, 18);
    ImGui::Checkbox("pulls", &_showPulls);
    ImGui::SetItemTooltip("Show the pull of each fit: (value - f(x)) / statistical uncertainty");
    }

  // table: header row and index column frozen; only the visible rows are drawn
  std::vector<std::string> all = strings(data.columnNames()), names;
  for (std::size_t c=0; c<all.size(); c++) if (_shown[c]) names.push_back(all[c]);
  for (const auto & name : data.fitColumnNames(_showPulls)) names.push_back(name.data());
  ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter
                        | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;
  // the number of columns is in the identifier: a table whose columns change (fits, pulls) starts afresh
  std::string id = "##data" + std::to_string(names.size());
  if (!ImGui::BeginTable(id.c_str(), int(names.size()), flags, ImVec2(0.0f, 0.0f))) return;
  ImGui::TableSetupScrollFreeze(1, 1);
  float digits = ImGui::CalcTextSize("-1.23457e+06").x;
  for (std::size_t c=0; c<names.size(); c++)
    {
    float width = (c == 0) ? ImGui::CalcTextSize("0000000").x : std::max(digits, ImGui::CalcTextSize(names[c].c_str()).x);
    ImGui::TableSetupColumn(names[c].c_str(), ImGuiTableColumnFlags_WidthFixed, width);
    }
  ImGui::TableHeadersRow();
  ImGuiListClipper clipper;
  clipper.Begin(int(data.size()));
  while (clipper.Step())
    for (int k=clipper.DisplayStart; k<clipper.DisplayEnd; k++)
      {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextDisabled("%d", k);
      std::vector<std::string> cells = strings(data.rowValues(unsigned(k)));
      for (std::size_t c=0; c<cells.size(); c++)
        {
        if (!_shown[c+1]) continue;
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(cells[c].c_str());
        }
      std::vector<bool> dimmed;
      std::vector<std::string> fitCells = strings(data.fitValues(unsigned(k), _showPulls, &dimmed));
      for (std::size_t c=0; c<fitCells.size(); c++)
        {
        ImGui::TableNextColumn();
        if (dimmed[c]) ImGui::TextDisabled("%s", fitCells[c].c_str());
        else ImGui::TextUnformatted(fitCells[c].c_str());
        }
      }
  ImGui::EndTable();
  }

  } // namespace GUI
} // namespace CAP
