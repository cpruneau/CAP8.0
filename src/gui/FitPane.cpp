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

  //! Whether the statistic is a chi square, or behaves as one (binned Poisson likelihood).
  bool chiSquareLike(const MATH::FitResult & r) { return !(r.statistic == "unbinned likelihood"); }

  //! The quality of a fit, as (name, value) rows.
  std::vector<std::pair<std::string,std::string>> quality(const MATH::FitRecord & fit)
  {
  const MATH::FitResult & r = fit.result;
  std::vector<std::pair<std::string,std::string>> rows;
  bool chi2 = chiSquareLike(r);
  rows.push_back({ chi2 ? "\xCF\x87\xC2\xB2" : "-2 ln L", number(r.chiSquare) });
  rows.push_back({ "degrees of freedom", std::to_string(r.nDegreesOfFreedom) });
  if (chi2)
    {
    rows.push_back({ "\xCF\x87\xC2\xB2/ndf", r.nDegreesOfFreedom > 0 ? number(r.chiSquarePerDegreeOfFreedom, "%.4g") : "-" });
    rows.push_back({ "p-value", r.nDegreesOfFreedom > 0 ? number(r.pValue, "%.4g") : "-" });
    }
  rows.push_back({ "points", std::to_string(r.nPoints) });
  rows.push_back({ "converged", r.converged ? "yes" : "no" });
  rows.push_back({ "uncertainties", r.validErrors ? "valid" : "not available" });
  if (!r.status.isNull()) rows.push_back({ "status", r.status.data() });
  rows.push_back({ "statistic", r.statistic.isNull() ? "-" : r.statistic.data() });
  rows.push_back({ "method", r.method.isNull() ? "-" : r.method.data() });
  rows.push_back({ "evaluations", std::to_string(r.nEvaluations) });
  rows.push_back({ "iterations", std::to_string(r.nIterations) });
  if (!fit.low.empty())
    {
    std::string range;
    for (std::size_t i=0; i<fit.low.size(); i++)
      range += (i ? " x " : "") + std::string("[") + number(fit.low[i]) + ", " + number(fit.high[i]) + "]";
    rows.push_back({ "range", range });
    }
  return rows;
  }

  //! Bounds of a parameter, as a text.
  std::string bounds(const MATH::Parameter & p)
  {
  switch (p.type())
    {
    case MATH::Parameter::Fixed:   return "fixed";
    case MATH::Parameter::Low:     return ">= " + number(p.minimum());
    case MATH::Parameter::High:    return "<= " + number(p.maximum());
    case MATH::Parameter::LowHigh: return "[" + number(p.minimum()) + ", " + number(p.maximum()) + "]";
    default:                       return "free";
    }
  }

  //! Name of parameter k (its index if it has none).
  std::string parameterName(const MATH::ParameterizedFunction & f, unsigned int k)
  {
  const String & name = f.parameter(k).name();
  return name.isNull() ? "p" + std::to_string(k) : std::string(name.data());
  }

  //! Value and uncertainty of parameter k: those of the result, or the values of the function.
  void parameterValue(const MATH::FitRecord & fit, unsigned int k, double & value, double & error)
  {
  const MATH::FitResult & r = fit.result;
  value = k < r.values.size() ? r.values[k] : fit.function->parameter(k).value();
  error = k < r.errors.size() ? r.errors[k] : 0.0;
  }

  //! Name of the class of the function of a fit (see MATH::FunctionRegistry).
  std::string functionType(const MATH::FitRecord & fit)
  {
  return fit.function ? std::string(MATH::FunctionRegistry::typeName(*fit.function).data()) : std::string("(no function)");
  }
  } // namespace

  String FitPane::text(const GraphItem & item, const MATH::FitRecord & fit)
  {
  std::string text = std::string(fit.label.data()) + "\t" + functionType(fit) + "\tfitted to\t" + item.label().data() + "\n";
  for (const auto & row : quality(fit)) text += row.first + "\t" + row.second + "\n";
  if (fit.function)
    {
    const MATH::ParameterizedFunction & f = *fit.function;
    text += "parameter\tvalue\tuncertainty\tbounds\n";
    for (unsigned int k=0; k<f.nParameters(); k++)
      {
      double value, error;
      parameterValue(fit, k, value, error);
      text += parameterName(f, k) + "\t" + number(value) + "\t" + number(error) + "\t" + bounds(f.parameter(k)) + "\n";
      }
    const MATH::Matrix & c = fit.result.correlation;
    const std::vector<unsigned int> & free = fit.result.freeParameters;
    if (c.nRows() == free.size() && c.nColumns() == free.size() && !free.empty())
      {
      text += "correlations";
      for (unsigned int j : free) text += "\t" + parameterName(f, j);
      text += "\n";
      for (std::size_t i=0; i<free.size(); i++)
        {
        text += parameterName(f, free[i]);
        for (std::size_t j=0; j<free.size(); j++) text += "\t" + number(c.values()[i*free.size() + j], "%.4f");
        text += "\n";
        }
      }
    }
  return String(text.c_str());
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
    ImGui::TextDisabled("No fit in the selected graph (fits are attached to data points and functions by MATH::Fitter::record())");
    return;
    }

  bool show = graph->showFits();
  if (ImGui::Checkbox("show the fits", &show)) { changing(); graph->setShowFits(show); }
  ImGui::SetItemTooltip("Draw the fits of the selected graph");
  ImGui::SameLine();
  bool statistics = graph->showFitStatistics();
  if (ImGui::Checkbox("quality in the legend", &statistics)) { changing(); graph->setShowFitStatistics(statistics); }
  ImGui::SetItemTooltip("Give the chi square per degree of freedom of each fit after its label in the legend");

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
    for (const auto & row : quality(fit))
      {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextDisabled("%s", row.first.c_str());
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(row.second.c_str());
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
      parameterValue(fit, p, value, error);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(parameterName(f, p).c_str());
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(number(value).c_str());
      ImGui::TableNextColumn();
      if (f.parameter(p).type() == MATH::Parameter::Fixed) ImGui::TextDisabled("-");
      else ImGui::Text("\xC2\xB1 %s", number(error, "%.4g").c_str());
      ImGui::TableNextColumn();
      ImGui::TextDisabled("%s", bounds(f.parameter(p)).c_str());
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
    const MATH::ParameterizedFunction & f = *fit.function;
    if (ImGui::BeginTable("##correlations", int(free.size()) + 1, flags))
      {
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
      for (unsigned int j : free) ImGui::TableSetupColumn(parameterName(f, j).c_str(), ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableHeadersRow();
      for (std::size_t i=0; i<free.size(); i++)
        {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(parameterName(f, free[i]).c_str());
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
