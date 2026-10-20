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
//! @file FitPane.cpp
//! @brief Implementation of FitPane.
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "imgui.h"
#include "FitPane.hpp"
#include "FitRecord.hpp"
#include "FunctionRegistry.hpp"
#include "Graph.hpp"
#include "GraphEditor.hpp"
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace GUI
  {

  using GRAPHICS::Graph;
  using GRAPHICS::GraphItem;
  using GRAPHICS::LineAttributes;

  namespace
  {
  std::string number(double v, const char * format="%.6g")
  {
  char text[48];
  std::snprintf(text, sizeof(text), format, v);
  return text;
  }

  //! Name of the class of the function of a fit (see MATH::FunctionRegistry).
  std::string functionType(const MATH::FitRecord & fit)
  {
  return fit.function ? std::string(MATH::FunctionRegistry::typeName(*fit.function).data()) : std::string("(no function)");
  }
  } // namespace

  String FitPane::text(const GraphItem & item, const MATH::FitRecord & fit)
  {
  return String("fitted to\t") + (item.label().isNull() ? String("(unnamed)") : item.label()) + String("\n") + fit.text();
  }

  void FitPane::render(Graph * graph, const std::function<void()> & beforeChange)
  {
  auto changing = [&]() { if (beforeChange) beforeChange(); };
  if (!graph) { ImGui::TextDisabled("No graph selected"); return; }

  // the fits of the graph: every item of rank 1 with fits attached
  std::vector<std::pair<int,unsigned int>> fits;   // (item, fit)
  for (std::size_t i=0; i<graph->items().size(); i++)
    {
    const GraphItem & item = graph->items()[i];
    if (item.isMap()) continue;
    for (unsigned int k=0; k<item.nFits(); k++) fits.push_back({ int(i), k });
    }
  if (fits.empty())
    {
    ImGui::TextDisabled("No fit in the selected graph.");
    if (onNewFit) { ImGui::SameLine(); if (ImGui::Button("Fit...")) onNewFit(); }
    return;
    }

  bool show = graph->showFits();
  if (ImGui::Checkbox("show the fits", &show)) { changing(); graph->setShowFits(show); }
  ImGui::SetItemTooltip("Draw the fits of the selected graph");
  ImGui::SameLine();
  bool statistics = graph->showFitStatistics();
  if (ImGui::Checkbox("quality in the legend", &statistics)) { changing(); graph->setShowFitStatistics(statistics); }
  ImGui::SetItemTooltip("Give the chi square per degree of freedom of each fit after its label in the legend");
  ImGui::SameLine();
  bool residuals = graph->showResiduals();
  if (ImGui::Checkbox("residuals panel", &residuals)) { changing(); graph->setShowResiduals(residuals); }
  ImGui::SetItemTooltip("Panel below the graph with the residuals of the fits, in their colors");
  if (residuals)
    {
    ImGui::SameLine();
    int mode = int(graph->residualMode());
    ImGui::SetNextItemWidth(130);
    if (ImGui::Combo("##residualMode", &mode, "data \xE2\x88\x92 fit\0pulls\0")) { changing(); graph->setResidualMode(Graph::ResidualMode(mode)); }
    ImGui::SetItemTooltip("data - fit with the statistical error bars, or the pulls (data - fit)/uncertainty");
    }
  if (onNewFit)
    {
    ImGui::SameLine(0, 18);
    if (ImGui::Button("Fit...")) onNewFit();
    ImGui::SetItemTooltip("Fit a function to an object of the selected graph");
    }

  if (!ImGui::BeginTabBar("##fits", ImGuiTabBarFlags_FittingPolicyScroll)) return;
  for (const auto & entry : fits)
    {
    GraphItem & item = graph->items()[std::size_t(entry.first)];
    const MATH::FitRecord * fit = item.fit(entry.second);
    if (!fit) continue;
    std::string label = fit->label.isNull() ? "fit " + std::to_string(entry.second + 1) : fit->label.data();
    label += "###fit" + std::to_string(entry.first) + "_" + std::to_string(entry.second);
    if (ImGui::BeginTabItem(label.c_str()))
      {
      renderFit(item, entry.second, beforeChange);
      ImGui::EndTabItem();
      }
    }
  ImGui::EndTabBar();
  }

  void FitPane::renderFit(GraphItem & item, unsigned int k, const std::function<void()> & beforeChange)
  {
  auto changing = [&]() { if (beforeChange) beforeChange(); };
  const MATH::FitRecord & fit = *item.fit(k);
  ImGui::PushID(int(k));

  // what was fitted to what; Copy on the right
  ImGui::Text("%s", functionType(fit).c_str());
  ImGui::SameLine();
  ImGui::TextDisabled("fitted to %s", item.label().isNull() ? "(unnamed)" : item.label().data());
  float buttonWidth = ImGui::CalcTextSize("Copy").x + 2*ImGui::GetStyle().FramePadding.x;
  ImGui::SameLine();
  float available = ImGui::GetContentRegionAvail().x;
  if (available > buttonWidth) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available - buttonWidth);
  if (ImGui::Button("Copy")) ImGui::SetClipboardText(text(item, fit).data());
  ImGui::SetItemTooltip("Copy the results of the fit (tab-separated) to the clipboard");

  // the curve: shown, color, style, width, range
  bool shown = item.isFitShown(k);
  if (ImGui::Checkbox("show", &shown)) { changing(); item.setFitShown(k, shown); }
  LineAttributes line = item.fitLineAttributes(k);
  ImGui::SameLine(0, 16);
  int color = line.color();
  if (GraphEditor::colorEdit("color", color)) { changing(); line.setColor(color, line.alpha()); item.setFitLineAttributes(k, line); }
  ImGui::SameLine(0, 16);
  int style = line.style();
  if (GraphEditor::lineStyleEdit("style", style, false)) { changing(); line.setStyle(style); item.setFitLineAttributes(k, line); }
  ImGui::SameLine(0, 16);
  float width = line.width();
  ImGui::SetNextItemWidth(120);
  bool widthChanged = ImGui::SliderFloat("width", &width, 0.5f, 6.0f, "%.1f");
  if (ImGui::IsItemActivated()) changing();
  if (widthChanged) { line.setWidth(width); item.setFitLineAttributes(k, line); }
  ImGui::SameLine(0, 16);
  bool extend = item.extendFits();
  if (ImGui::Checkbox("whole x range", &extend)) { changing(); item.setExtendFits(extend); }
  ImGui::SetItemTooltip("Draw the fits of this object over the whole x range of the graph, not only the range of each fit");

  // quality (left) and parameters (right), side by side when there is room
  ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingFixedFit;
  float qualityWidth = ImGui::CalcTextSize("degrees of freedom  differential evolution + Levenberg-Marquardt").x;
  bool sideBySide = ImGui::GetContentRegionAvail().x > 2.2f*qualityWidth;
  if (ImGui::BeginTable("##quality", 2, flags, ImVec2(sideBySide ? qualityWidth : 0.0f, 0.0f)))
    {
    for (const auto & row : fit.quality())
      {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextDisabled("%s", row.first.data());
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(row.second.data());
      }
    ImGui::EndTable();
    }
  if (sideBySide) ImGui::SameLine(0, 16);
  else ImGui::Spacing();
  ImGui::BeginGroup();
  if (fit.function && ImGui::BeginTable("##parameters", 5, flags))
    {
    const MATH::ParameterizedFunction & f = *fit.function;
    for (const char * name : { "parameter", "value", "uncertainty", "bounds", "" })
      ImGui::TableSetupColumn(name, ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
    for (unsigned int p=0; p<f.nParameters(); p++)
      {
      double value, error;
      fit.parameterAt(p, value, error);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(fit.parameterName(p).data());
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(number(value).c_str());
      ImGui::TableNextColumn();
      if (f.parameter(p).type() == MATH::Parameter::Fixed) ImGui::TextDisabled("-");
      else ImGui::Text("\xC2\xB1 %s", number(error, "%.4g").c_str());
      ImGui::TableNextColumn();
      ImGui::TextDisabled("%s", f.parameter(p).boundsText().data());
      ImGui::TableNextColumn();
      if (p < fit.result.atLimit.size() && fit.result.atLimit[p])
        {
        ImGui::TextColored(ImVec4(0.85f, 0.45f, 0.0f, 1.0f), "at a bound");
        ImGui::SetItemTooltip("The parameter ended at one of its bounds: its uncertainty is unreliable");
        }
      }
    ImGui::EndTable();
    }

  // correlations of the free parameters
  const MATH::Matrix & c = fit.result.correlation;
  const std::vector<unsigned int> & free = fit.result.freeParameters;
  if (fit.function && free.size() > 1 && c.nRows() == free.size() && c.nColumns() == free.size()
      && ImGui::TreeNodeEx("Correlations", ImGuiTreeNodeFlags_DefaultOpen))
    {
    if (ImGui::BeginTable("##correlations", int(free.size()) + 1, flags))
      {
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
      for (unsigned int j : free) ImGui::TableSetupColumn(fit.parameterName(j).data(), ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableHeadersRow();
      for (std::size_t i=0; i<free.size(); i++)
        {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(fit.parameterName(free[i]).data());
        for (std::size_t j=0; j<free.size(); j++)
          {
          ImGui::TableNextColumn();
          double v = c.values()[i*free.size() + j];
          if (i == j) ImGui::TextDisabled("1");
          else if (std::fabs(v) > 0.9) ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.1f, 1.0f), "%s", number(v, "%.3f").c_str());
          else ImGui::TextUnformatted(number(v, "%.3f").c_str());
          }
        }
      ImGui::EndTable();
      }
    ImGui::TreePop();
    }
  ImGui::EndGroup();
  ImGui::PopID();
  }

  } // namespace GUI
} // namespace CAP
