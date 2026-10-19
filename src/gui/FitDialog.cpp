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
//! @file FitDialog.cpp
//! @brief Implementation of FitDialog.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "FitDialog.hpp"
#include "DataPoints.hpp"
#include "DataPointsVector.hpp"
#include "Exceptions.hpp"
#include "Fitter.hpp"
#include "FunctionRegistry.hpp"
#include "Graph.hpp"
#include "Histogram.hpp"
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace GUI
  {

  using GRAPHICS::Graph;
  using GRAPHICS::GraphItem;

  namespace
  {
  //! Whether an item can be fitted: data points or a function, of rank 1.
  bool fittable(const GraphItem & item)
  {
  if (item.isMap()) return false;
  if (item.kind() == GraphItem::kData) return item.data() != nullptr && item.data()->size() > 0;
  return item.function() != nullptr && item.function()->rank() == 1;
  }

  String itemName(const GraphItem & item, int k)
  {
  String name = item.label().isNull() ? String("(unnamed)") : item.label();
  name += String(item.kind() == GraphItem::kData ? (dynamic_cast<const MATH::Histogram*>(item.data()) ? "  (histogram)" : "  (data points)")
                                                 : "  (function)");
  return name + String("##item") + String(std::to_string(k));
  }

  //! Extent in x of data points.
  bool extent(const MATH::DataPoints & data, double & low, double & high)
  {
  low = 1E300; high = -1E300;
  for (unsigned int k=0; k<data.size(); k++)
    {
    double x = data.point(k).x();
    if (!std::isfinite(x)) continue;
    low = std::min(low, x); high = std::max(high, x);
    }
  return high >= low;
  }

  //! Index of a name in the registry, or 0.
  int indexOf(const std::vector<String> & names, const char * name)
  {
  for (std::size_t k=0; k<names.size(); k++) if (names[k] == name) return int(k);
  return 0;
  }
  } // namespace

  FitDialog::FitDialog()
  :
  _open(false),
  _openRequested(false),
  _requestedItem(-1),
  _item(-1),
  _object(nullptr),
  _functionIndex(indexOf(MATH::FunctionRegistry::names(), "Normal")),
  _degree(2),
  _function(),
  _statistic(0),
  _search(false),
  _low(0.0),
  _high(1.0),
  _samples(100),
  _label(),
  _message(),
  _error(false)
  {
  makeFunction();
  }

  FitDialog::~FitDialog() { }

  void FitDialog::open(int item)
  {
  _open = true;
  _openRequested = true;
  _requestedItem = item;
  _message = "";
  ImGui::SetWindowFocus(windowName());
  }

  void FitDialog::makeFunction()
  {
  std::vector<String> names = MATH::FunctionRegistry::names();
  if (_functionIndex < 0 || _functionIndex >= int(names.size())) _functionIndex = 0;
  if (names.empty()) { _function.reset(); return; }
  const String & name = names[std::size_t(_functionIndex)];
  if (name == "Polynomial") _function.reset(MATH::FunctionRegistry::create(name, (unsigned int)(_degree + 1)));
  else _function.reset(MATH::FunctionRegistry::create(name));
  }

  void FitDialog::selectItem(Graph & graph, int k)
  {
  _item = -1;
  _object = nullptr;
  if (k < 0 || k >= int(graph.items().size()) || !fittable(graph.items()[std::size_t(k)]))
    {
    for (std::size_t i=0; i<graph.items().size(); i++) if (fittable(graph.items()[i])) { k = int(i); break; }
    if (k < 0 || k >= int(graph.items().size()) || !fittable(graph.items()[std::size_t(k)])) return;
    }
  const GraphItem & item = graph.items()[std::size_t(k)];
  _item = k;
  _object = item.object();
  if (item.kind() == GraphItem::kData)
    {
    if (!extent(*item.data(), _low, _high)) { _low = 0; _high = 1; }
    if (!dynamic_cast<const MATH::Histogram*>(item.data())) _statistic = 0;   // the Poisson likelihood needs counts
    }
  else
    {
    _low = item.xMinimum(); _high = item.xMaximum();
    _statistic = 0;
    }
  }

  void FitDialog::estimate(Graph & graph)
  {
  if (!_function || _item < 0) return;
  const GraphItem & item = graph.items()[std::size_t(_item)];
  bool done = false;
  try
    {
    if (item.kind() == GraphItem::kData) done = MATH::Fitter::estimate(*_function, *item.data(), _low, _high);
    else done = MATH::Fitter::estimate(*_function, MATH::Fitter::samples(*item.function(), _low, _high, (unsigned int)_samples), _low, _high);
    }
  catch (const Exception & e) { _message = e.what(); _error = true; return; }
  _message = done ? "Starting values estimated from the data in the range"
                  : "No estimate for this function: set the starting values by hand";
  _error = !done;
  }

  bool FitDialog::fit(Graph & graph)
  {
  if (!_function || _item < 0) return false;
  GraphItem & item = graph.items()[std::size_t(_item)];
  try
    {
    // a function is fitted through samples of it, over the range
    MATH::DataPointsVector samples;
    if (item.kind() == GraphItem::kFunction) samples = MATH::Fitter::samples(*item.function(), _low, _high, (unsigned int)_samples);
    const MATH::DataPoints & data = item.kind() == GraphItem::kData ? *item.data() : samples;
    MATH::Fitter fitter(*_function, data, _statistic == 1 ? MATH::Fitter::kPoisson : MATH::Fitter::kChiSquare);
    fitter.setRange(_low, _high);
    if (_search)
      {
      fitter.setMethod(MATH::Fitter::kSearch);
      fitter.search().setMethod(MATH::ExtremumSearch::kDifferentialEvolution);
      }
    MATH::FitResult r = fitter.fit();
    MATH::FitRecord record = fitter.record(_label);
    // the objects shown belong to the application or its files: the fit is attached to them
    unsigned int index;
    if (item.kind() == GraphItem::kData)
      {
      MATH::DataPoints * target = const_cast<MATH::DataPoints*>(item.data());
      target->addFit(record);
      index = target->nFits() - 1;
      }
    else
      {
      MATH::Function * target = const_cast<MATH::Function*>(item.function());
      target->addFit(record);
      index = target->nFits() - 1;
      }
    item.setFitShown(index, true);
    char text[256];
    std::snprintf(text, sizeof(text), "%s attached: %s, %s%s", record.label.data(), GraphItem::fitSummary(record).data(),
                  r.converged ? "converged" : "not converged", r.validErrors ? "" : " (no uncertainties)");
    _message = text;
    _error = !r.converged;
    return true;
    }
  catch (const Exception & e)
    {
    _message = String("The fit failed: ") + String(e.what());
    _error = true;
    }
  catch (const std::exception & e)
    {
    _message = String("The fit failed: ") + String(e.what());
    _error = true;
    }
  return false;
  }

  void FitDialog::renderParameters()
  {
  if (!_function) return;
  MATH::ParameterizedFunction & f = *_function;
  ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingFixedFit;
  if (!ImGui::BeginTable("##parameters", 6, flags)) return;
  ImGui::TableSetupColumn("parameter");
  ImGui::TableSetupColumn("starting value");
  ImGui::TableSetupColumn("fixed");
  ImGui::TableSetupColumn("bounded");
  ImGui::TableSetupColumn("minimum");
  ImGui::TableSetupColumn("maximum");
  ImGui::TableHeadersRow();
  for (unsigned int k=0; k<f.nParameters(); k++)
    {
    const MATH::Parameter & p = f.parameter(k);
    ImGui::PushID(int(k));
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(p.name().isNull() ? ("p" + std::to_string(k)).c_str() : p.name().data());
    ImGui::TableNextColumn();
    double value = p.value();
    ImGui::SetNextItemWidth(130);
    if (ImGui::InputDouble("##value", &value, 0.0, 0.0, "%.6g"))
      {
      bool low = p.type() == MATH::Parameter::Low || p.type() == MATH::Parameter::LowHigh;
      bool high = p.type() == MATH::Parameter::High || p.type() == MATH::Parameter::LowHigh;
      if (low) value = std::max(value, p.minimum());
      if (high) value = std::min(value, p.maximum());
      try { f.setParameterValue(k, value); } catch (...) { }
      }
    ImGui::TableNextColumn();
    bool fixed = p.type() == MATH::Parameter::Fixed;
    if (ImGui::Checkbox("##fixed", &fixed)) f.fixParameter(k, fixed);
    ImGui::TableNextColumn();
    bool hasLow = p.minimum() > MATH::Parameter::defaultMinimum, hasHigh = p.maximum() < MATH::Parameter::defaultMaximum;
    bool bounded = hasLow || hasHigh;
    if (ImGui::Checkbox("##bounded", &bounded))
      {
      if (bounded)
        {
        double span = std::max(1.0, std::fabs(p.value()));
        f.setParameterLimits(k, p.value() - span, p.value() + span);
        }
      else
        {
        bool wasFixed = p.type() == MATH::Parameter::Fixed;
        MATH::Parameter free(p.name(), p.value(), wasFixed ? MATH::Parameter::Fixed : MATH::Parameter::Free);
        f.setParameter(k, free);
        }
      }
    ImGui::TableNextColumn();
    if (bounded)
      {
      double minimum = hasLow ? p.minimum() : -1E300, maximum = hasHigh ? p.maximum() : 1E300;
      ImGui::SetNextItemWidth(110);
      if (ImGui::InputDouble("##minimum", &minimum, 0.0, 0.0, "%.6g") && minimum <= maximum) f.setParameterLimits(k, minimum, maximum);
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(110);
      if (ImGui::InputDouble("##maximum", &maximum, 0.0, 0.0, "%.6g") && minimum <= maximum) f.setParameterLimits(k, minimum, maximum);
      }
    else
      {
      ImGui::TextDisabled("-");
      ImGui::TableNextColumn();
      ImGui::TextDisabled("-");
      }
    ImGui::PopID();
    }
  ImGui::EndTable();
  }

  bool FitDialog::render(Graph * graph)
  {
  if (!_open) return false;
  ImGui::SetNextWindowSize(ImVec2(640, 0), ImGuiCond_FirstUseEver);
  bool attached = false;
  if (!ImGui::Begin(windowName(), &_open, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
    {
    ImGui::End();
    return false;
    }
  if (!graph) { ImGui::TextDisabled("No graph selected"); ImGui::End(); return false; }

  // the item fitted: chosen again when the graph or its contents change
  bool valid = _item >= 0 && _item < int(graph->items().size()) && graph->items()[std::size_t(_item)].object() == _object;
  if (_openRequested || !valid) selectItem(*graph, _openRequested ? _requestedItem : _item);
  _openRequested = false;
  if (_item < 0)
    {
    ImGui::TextDisabled("The selected graph shows no data points or function of one variable to fit.");
    ImGui::End();
    return false;
    }
  GraphItem & item = graph->items()[std::size_t(_item)];

  ImGui::SeparatorText("Fit");
  ImGui::SetNextItemWidth(320);
  if (ImGui::BeginCombo("object", itemName(item, _item).data()))
    {
    for (std::size_t k=0; k<graph->items().size(); k++)
      {
      const GraphItem & candidate = graph->items()[k];
      if (!fittable(candidate)) continue;
      if (ImGui::Selectable(itemName(candidate, int(k)).data(), int(k) == _item)) selectItem(*graph, int(k));
      }
    ImGui::EndCombo();
    }
  const bool histogram = item.kind() == GraphItem::kData && dynamic_cast<const MATH::Histogram*>(item.data());

  // the function
  std::vector<String> names = MATH::FunctionRegistry::names();
  ImGui::SetNextItemWidth(180);
  if (ImGui::BeginCombo("function", _functionIndex < int(names.size()) ? names[std::size_t(_functionIndex)].data() : ""))
    {
    for (std::size_t k=0; k<names.size(); k++)
      if (ImGui::Selectable(names[k].data(), int(k) == _functionIndex)) { _functionIndex = int(k); makeFunction(); }
    ImGui::EndCombo();
    }
  if (_functionIndex < int(names.size()) && names[std::size_t(_functionIndex)] == "Polynomial")
    {
    ImGui::SameLine(0, 16);
    ImGui::SetNextItemWidth(90);
    if (ImGui::InputInt("degree", &_degree)) { _degree = std::min(std::max(_degree, 0), 10); makeFunction(); }
    }
  if (!_function) ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.1f, 1.0f), "This function cannot be made");

  // range, statistic, method
  ImGui::SeparatorText("Range and statistic");
  double range[2] = { _low, _high };
  ImGui::SetNextItemWidth(240);
  if (ImGui::InputScalarN("x range", ImGuiDataType_Double, range, 2, nullptr, nullptr, "%.6g") && range[0] <= range[1])
    { _low = range[0]; _high = range[1]; }
  ImGui::SameLine();
  if (ImGui::Button("View"))
    {
    Graph::View view = graph->view();
    _low = view.xMinimum; _high = view.xMaximum;
    }
  ImGui::SetItemTooltip("Take the x range shown in the graph");
  ImGui::SameLine();
  if (ImGui::Button("All")) selectItem(*graph, _item);
  ImGui::SetItemTooltip("Take the whole x range of the object");
  if (item.kind() == GraphItem::kFunction)
    {
    ImGui::SetNextItemWidth(120);
    if (ImGui::InputInt("samples", &_samples)) _samples = std::min(std::max(_samples, 2), 100000);
    ImGui::SetItemTooltip("Points of the function fitted, equally spaced over the range (each with an uncertainty of 1)");
    }
  ImGui::RadioButton("chi square", &_statistic, 0);
  ImGui::SetItemTooltip("Least squares weighted by the statistical uncertainties (points without one are left out)");
  ImGui::SameLine();
  ImGui::BeginDisabled(!histogram);
  ImGui::RadioButton("Poisson likelihood", &_statistic, 1);
  ImGui::EndDisabled();
  ImGui::SetItemTooltip("For counts (histograms): unbiased where bins have few entries, empty bins included");
  if (!histogram) _statistic = 0;
  ImGui::Checkbox("global search first (differential evolution)", &_search);
  ImGui::SetItemTooltip("When the starting values are poor or the statistic has several minima; needs bounds on the free parameters");

  // parameters
  ImGui::SeparatorText("Parameters");
  renderParameters();
  if (ImGui::Button("Estimate")) estimate(*graph);
  ImGui::SetItemTooltip("Starting values from the data in the range (Normal, LogNormal, Exponential, polynomials)");

  // label and actions
  ImGui::SeparatorText("Result");
  ImGui::SetNextItemWidth(240);
  ImGui::InputText("label", &_label.str());
  ImGui::SetItemTooltip("Label of the fit in the legend and the tabs (empty: the function and \"fit\")");
  ImGui::BeginDisabled(!_function);
  if (ImGui::Button("Fit")) attached = fit(*graph);
  ImGui::EndDisabled();
  ImGui::SetItemTooltip("Fit, and attach the fit to the object (its results are in the Fits pane)");
  ImGui::SameLine();
  if (ImGui::Button("Close")) _open = false;
  if (!_message.isNull())
    {
    if (_error) ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.1f, 1.0f), "%s", _message.data());
    else ImGui::TextWrapped("%s", _message.data());
    }
  ImGui::End();
  return attached;
  }

  } // namespace GUI
} // namespace CAP
