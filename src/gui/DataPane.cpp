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
  //! Names of the columns: index, each coordinate and its uncertainty, the value and its two uncertainties.
  std::vector<std::string> columnNames(const MATH::DataPoints & data)
  {
  static const char * axes[] = { "x", "y", "z" };
  const MATH::Histogram * histogram = dynamic_cast<const MATH::Histogram *>(&data);
  unsigned int rank = data.rank();
  std::vector<std::string> names{ "#" };
  for (unsigned int k=0; k<rank; k++)
    {
    std::string name;
    if (histogram && !histogram->dimensionTitle(k).isNull()) name = histogram->dimensionTitle(k).data();
    else if (rank <= 3) name = axes[k];
    else name = "x" + std::to_string(k);
    names.push_back(name);
    names.push_back("± " + name);
    }
  std::string value;
  if (histogram && !histogram->title().isNull()) value = histogram->title().data();
  else value = (rank == 1) ? "y" : "value";
  names.push_back(value);
  names.push_back("± stat.");
  names.push_back("± syst.");
  return names;
  }

  //! Kind and size of the object, e.g. "Histogram (1D), 100 bins" or "Data points, 25 points".
  std::string kindOf(const MATH::DataPoints & data)
  {
  char text[128];
  const MATH::Histogram * histogram = dynamic_cast<const MATH::Histogram *>(&data);
  if (histogram)
    std::snprintf(text, sizeof(text), "%s (%uD), %u bins", dynamic_cast<const MATH::Profile *>(&data) ? "Profile" : "Histogram",
                  data.rank(), data.size());
  else
    std::snprintf(text, sizeof(text), "Data points (%uD), %u points", data.rank(), data.size());
  return text;
  }

  std::string number(double v)
  {
  char text[32];
  std::snprintf(text, sizeof(text), "%.6g", v);
  return text;
  }

  //! Names of the columns of the fits: f(x) of each fit, and its pull.
  std::vector<std::string> fitColumnNames(const MATH::DataPoints & data, bool pulls)
  {
  std::vector<std::string> names;
  for (unsigned int f=0; f<data.nFits(); f++)
    {
    std::string label = data.fit(f).label.isNull() ? "fit " + std::to_string(f + 1) : data.fit(f).label.data();
    names.push_back("f(x) " + label);
    if (pulls) names.push_back("pull " + label);
    }
  return names;
  }

  //! Cells of the fits of row k (f(x) and pulls); dimmed set for the points outside the range of each fit.
  std::vector<std::string> fitValues(const MATH::DataPoints & data, unsigned int k, bool pulls, std::vector<bool> * dimmed=nullptr)
  {
  std::vector<std::string> cells;
  std::vector<double> x = data.coordinatesAt(k);
  for (unsigned int f=0; f<data.nFits(); f++)
    {
    const MATH::FitRecord & fit = data.fit(f);
    double value = fit.valueAt(x);
    bool inRange = fit.low.size() == x.size();
    for (std::size_t i=0; i<x.size() && inRange; i++) inRange = x[i] >= fit.low[i] && x[i] <= fit.high[i];
    cells.push_back(std::isfinite(value) ? number(value) : std::string());
    if (dimmed) dimmed->push_back(!inRange);
    if (!pulls) continue;
    double error = data.statErrorAt(k);
    cells.push_back(std::isfinite(value) && error > 0 ? number((data.valueAt(k) - value)/error) : std::string());
    if (dimmed) dimmed->push_back(!inRange);
    }
  return cells;
  }

  //! The cells of row k, after the index.
  std::vector<std::string> rowValues(const MATH::DataPoints & data, unsigned int k)
  {
  std::vector<std::string> cells;
  std::vector<double> x = data.coordinatesAt(k), ex = data.coordinateErrorsAt(k);
  for (std::size_t i=0; i<x.size(); i++)
    {
    cells.push_back(number(x[i]));
    cells.push_back(number(i < ex.size() ? ex[i] : 0.0));
    }
  cells.push_back(number(data.valueAt(k)));
  cells.push_back(number(data.statErrorAt(k)));
  cells.push_back(number(data.sysErrorAt(k)));
  return cells;
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
  if (!_data) return;
  // columns of uncertainties (coordinates' and the value's) shown only if some point has a non-zero one
  unsigned int rank = _data->rank();
  std::vector<bool> nonZero(rank + 2, false);
  for (unsigned int k=0; k<_data->size(); k++)
    {
    std::vector<double> ex = _data->coordinateErrorsAt(k);
    for (unsigned int i=0; i<rank && i<ex.size(); i++) if (ex[i] != 0.0) nonZero[i] = true;
    if (_data->statErrorAt(k) != 0.0) nonZero[rank]   = true;
    if (_data->sysErrorAt(k)  != 0.0) nonZero[rank+1] = true;
    }
  _shown.push_back(true);                                                     // index
  for (unsigned int i=0; i<rank; i++) { _shown.push_back(true); _shown.push_back(nonZero[i]); }   // coordinate, its error
  _shown.push_back(true);                                                     // value
  _shown.push_back(nonZero[rank]);
  _shown.push_back(nonZero[rank+1]);
  }

  String DataPane::windowTitle() const
  {
  // "###name" keeps the window (and its place in the layout) when the label changes
  return (_label.isNull() ? _name : _label) + String("###") + _name;
  }

  String DataPane::text() const
  {
  if (!_data) return String();
  std::string text;
  std::vector<std::string> names = columnNames(*_data);
  bool first = true;
  for (std::size_t c=0; c<names.size(); c++) if (_shown[c]) { text += (first ? "" : "\t") + names[c]; first = false; }
  for (const auto & name : fitColumnNames(*_data, _showPulls)) text += "\t" + name;
  text += "\n";
  for (unsigned int k=0; k<_data->size(); k++)
    {
    text += std::to_string(k);
    std::vector<std::string> cells = rowValues(*_data, k);
    for (std::size_t c=0; c<cells.size(); c++) if (_shown[c+1]) text += "\t" + cells[c];
    for (const auto & cell : fitValues(*_data, k, _showPulls)) text += "\t" + cell;
    text += "\n";
    }
  return String(text);
  }

  void DataPane::render()
  {
  if (!_data) { ImGui::TextDisabled("No data"); return; }
  const MATH::DataPoints & data = *_data;

  // heading: name, kind and size, title; Copy on the right
  ImGui::TextUnformatted(_label.data());
  ImGui::SameLine();
  ImGui::TextDisabled("%s", kindOf(data).c_str());
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
  std::vector<std::string> all = columnNames(data), names;
  for (std::size_t c=0; c<all.size(); c++) if (_shown[c]) names.push_back(all[c]);
  for (const auto & name : fitColumnNames(data, _showPulls)) names.push_back(name);
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
      std::vector<std::string> cells = rowValues(data, unsigned(k));
      for (std::size_t c=0; c<cells.size(); c++)
        {
        if (!_shown[c+1]) continue;
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(cells[c].c_str());
        }
      std::vector<bool> dimmed;
      std::vector<std::string> fitCells = fitValues(data, unsigned(k), _showPulls, &dimmed);
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
