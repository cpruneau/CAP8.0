/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
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
#include "MathExceptions.hpp"
#include "OrthogonalSeries.hpp"
#include "ParameterizedComposite.hpp"
#include "ParameterizedFunction.hpp"
#include "Polynomial.hpp"
#include "Polynomial2D.hpp"
#include <map>

namespace CAP
{
  namespace GUI
  {

  using GRAPHICS::Graph;
  using GRAPHICS::GraphItem;

  namespace
  {
  String itemName(const GraphItem & item, int k)
  {
  String name = item.label().isNull() ? String("(unnamed)") : item.label();
  name += String(item.kind() == GraphItem::kData ? (dynamic_cast<const MATH::Histogram*>(item.data()) ? "  (histogram)" : "  (data points)")
                                                 : "  (function)");
  return name + String("##item") + String(std::to_string(k));
  }

  //! A function of a registered name, with a degree for a Polynomial or a series. @return function, or null
  MATH::ParameterizedFunction * makeNamed(const String & name, int degree)
  {
  if (name == "Polynomial") return MATH::FunctionRegistry::create(name, (unsigned int)(degree + 1));
  MATH::ParameterizedFunction * f = MATH::FunctionRegistry::create(name);
  if (MATH::OrthogonalSeries * series = dynamic_cast<MATH::OrthogonalSeries*>(f)) series->setDegree((unsigned int)degree);
  if (MATH::Polynomial2D * polynomial = dynamic_cast<MATH::Polynomial2D*>(f)) polynomial->setDegree((unsigned int)std::min(degree, 9));
  return f;
  }

  //! Whether the functions of a name have a degree (a Polynomial, a Polynomial2D, a series).
  bool hasDegree(const String & name)
  {
  return name == "Polynomial" || name == "Polynomial2D" || name.endsWith("Series");
  }

  //! Number of variables of the functions of a registered name (0 if none can be made).
  unsigned int rankOf(const String & name)
  {
  if (std::shared_ptr<const MATH::ParameterizedComposite> c = MATH::FunctionRegistry::composite(name)) return c->rank();
  static std::map<std::string, unsigned int> ranks;                    // classes keep their rank
  auto found = ranks.find(name.data());
  if (found != ranks.end()) return found->second;
  std::unique_ptr<MATH::ParameterizedFunction> f;
  try { f.reset(MATH::FunctionRegistry::create(name)); } catch (...) { }
  unsigned int rank = f ? f->rank() : 0;
  ranks[name.data()] = rank;
  return rank;
  }

  //! The function offered first for a rank.
  String defaultFunction(unsigned int rank) { return rank == 2 ? "Normal2D" : "Normal"; }

  //! One line on the functions of a registered name.
  String describe(const String & name)
  {
  static const std::vector<std::pair<const char *, const char *>> texts = {
    { "Constant", "c0" }, { "Linear", "c0 + c1 x" }, { "Parabolic", "c0 + c1 x + c2 x^2" },
    { "Cubic", "c0 + ... + c3 x^3" }, { "Quartic", "c0 + ... + c4 x^4" }, { "Polynomial", "c0 + c1 x + ... + cN x^N" },
    { "Normal", "A exp(-(x - mean)^2 / (2 width^2))" }, { "LogNormal", "log-normal density of area A (mean and width of ln x)" },
    { "Exponential", "A exp(b x)" },
    { "LegendreSeries", "c0 P0(t) + ... + cN PN(t), Legendre; x in [low, high] mapped onto t in [-1, 1]" },
    { "ChebyshevSeries", "c0 T0(t) + ... + cN TN(t), Chebyshev; x in [low, high] mapped onto t in [-1, 1]" },
    { "HermiteSeries", "c0 H0(t) + ... + cN HN(t), Hermite; t = (x - x0)/scale" },
    { "LaguerreSeries", "c0 L0(t) + ... + cN LN(t), Laguerre of parameter alpha; t = (x - x0)/scale" },
    { "BesselJ", "A J_n(scale (x - shift)), Bessel of the first kind; order n fixed" },
    { "BesselY", "A Y_n(scale (x - shift)), Bessel of the second kind; order n fixed" },
    { "BesselI", "A I_n(scale (x - shift)), modified Bessel of the first kind; order n fixed" },
    { "BesselK", "A K_n(scale (x - shift)), modified Bessel of the second kind; order n fixed" },
    { "Normal2D", "A exp(-(u^2 - 2 rho u v + v^2)/(2 (1 - rho^2))), u = (x - meanX)/widthX, v = (y - meanY)/widthY" },
    { "Polynomial2D", "sum of cij t^i s^j, i + j <= N; t, s: x, y of the range mapped onto [-1, 1]" } };
  for (const auto & text : texts) if (name == text.first) return String(text.second);
  if (std::shared_ptr<const MATH::ParameterizedComposite> c = MATH::FunctionRegistry::composite(name))
    {
    bool product = c->operation() == MATH::ParameterizedComposite::kProduct;
    String text = product ? "product: " : "composite: ";
    for (unsigned int k=0; k<c->nComponents(); k++)
      text += String(k ? (product ? " x " : " + ") : "") + c->label(k) + String(" (") + MATH::FunctionRegistry::typeName(c->component(k)) + String(")");
    return text;
    }
  return "function registered by a program";
  }
  } // namespace

  FitDialog::FitDialog()
  :
  _open(false),
  _openRequested(false),
  _requestedItem(-1),
  _item(-1),
  _object(nullptr),
  _functionName("Normal"),
  _degree(2),
  _function(),
  _statistic(0),
  _search(false),
  _low{ 0.0 },
  _high{ 1.0 },
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
  if (!MATH::FunctionRegistry::isRegistered(_functionName) || rankOf(_functionName) != _rank) _functionName = defaultFunction(_rank);
  try { _function.reset(makeNamed(_functionName, _degree)); }
  catch (...) { _function.reset(); }
  if (_function) _function->adaptToRange(_low, _high);   // e.g. the mapping of a series
  }

  void FitDialog::saveComposites()
  {
  String file = MATH::FunctionRegistry::defaultCompositesFile();
  if (!MATH::FunctionRegistry::writeComposites(file)) { _message = String("Cannot write ") + file; _error = true; }
  }

  void FitDialog::renderCompositeBuilder()
  {
  if (_builderRequested) { ImGui::OpenPopup("Composite"); _builderRequested = false; }
  ImGui::SetNextWindowSize(ImVec2(560, 0), ImGuiCond_Appearing);
  if (!ImGui::BeginPopupModal("Composite", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
  ImGui::TextWrapped("A sum of registered functions of the same variables, or a product of functions of one variable "
                     "each (f(x) g(y)), fitted with all their parameters (named label.parameter). It is saved in %s, "
                     "read when CapGui starts.", MATH::FunctionRegistry::defaultCompositesFile().data());
  ImGui::SetNextItemWidth(240);
  ImGui::InputText("name", &_compositeName.str());
  ImGui::RadioButton("sum", &_operation, 0);
  ImGui::SameLine();
  ImGui::RadioButton("product f(x) g(y)", &_operation, 1);
  ImGui::SetItemTooltip("Each component a function of its own variable, in order: x for the first, y for the second");
  std::vector<String> names = MATH::FunctionRegistry::names();
  int removed = -1;
  if (ImGui::BeginTable("##components", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerH))
    {
    ImGui::TableSetupColumn("function");
    ImGui::TableSetupColumn("degree");
    ImGui::TableSetupColumn("label");
    ImGui::TableSetupColumn("");
    ImGui::TableHeadersRow();
    for (std::size_t k=0; k<_drafts.size(); k++)
      {
      ComponentDraft & draft = _drafts[k];
      ImGui::PushID(int(k));
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(170);
      if (ImGui::BeginCombo("##type", draft.type.data()))
        {
        for (const auto & name : names)
          {
          if (name == _compositeName || (_operation == 1 && rankOf(name) != 1)) continue;   // a product of functions of one variable
          if (ImGui::Selectable(name.data(), name == draft.type)) draft.type = name;
          }
        ImGui::EndCombo();
        }
      ImGui::SetItemTooltip("%s", describe(draft.type).data());
      ImGui::TableNextColumn();
      if (hasDegree(draft.type))
        {
        ImGui::SetNextItemWidth(80);
        if (ImGui::InputInt("##degree", &draft.degree)) draft.degree = std::min(std::max(draft.degree, 0), 10);
        }
      else ImGui::TextDisabled("-");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(130);
      ImGui::InputText("##label", &draft.label.str());
      ImGui::TableNextColumn();
      if (ImGui::SmallButton("x")) removed = int(k);
      ImGui::PopID();
      }
    ImGui::EndTable();
    }
  if (removed >= 0) _drafts.erase(_drafts.begin() + removed);
  if (ImGui::Button("Add component"))
    {
    ComponentDraft draft;
    draft.type = _drafts.empty() ? "Normal" : "Linear";
    draft.label = _drafts.empty() ? "signal" : (_drafts.size() == 1 ? "background" : String(("f" + std::to_string(_drafts.size() + 1)).c_str()));
    _drafts.push_back(draft);
    }
  if (!_builderMessage.isNull()) ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.1f, 1.0f), "%s", _builderMessage.data());
  ImGui::Separator();
  if (ImGui::Button("Create"))
    {
    try
      {
      if (_compositeName.isNull()) throw MATH::MathException("Give the composite a name", "FitDialog");
      if (MATH::FunctionRegistry::isRegistered(_compositeName) && !MATH::FunctionRegistry::isComposite(_compositeName))
        throw MATH::MathException(String("The name ") + _compositeName + String(" is that of a class of functions"), "FitDialog");
      MATH::ParameterizedComposite composite(_operation == 1 ? MATH::ParameterizedComposite::kProduct : MATH::ParameterizedComposite::kSum);
      for (const auto & draft : _drafts)
        {
        std::unique_ptr<MATH::ParameterizedFunction> f(makeNamed(draft.type, draft.degree));
        if (!f) throw MATH::MathException(String("Cannot make ") + draft.type, "FitDialog");
        composite.add(draft.label, *f);
        }
      MATH::FunctionRegistry::registerComposite(_compositeName, composite);
      saveComposites();
      if (composite.rank() == _rank) { _functionName = _compositeName; makeFunction(); }
      else
        {
        _message = _compositeName + String(" registered: of ") + String(std::to_string(composite.rank()).c_str())
                   + String(" variables, it is offered for objects of as many");
        _error = false;
        }
      _builderMessage = "";
      ImGui::CloseCurrentPopup();
      }
    catch (const std::exception & e) { _builderMessage = e.what(); }
    }
  ImGui::SameLine();
  if (ImGui::Button("Cancel")) { _builderMessage = ""; ImGui::CloseCurrentPopup(); }
  ImGui::EndPopup();
  }

  void FitDialog::selectItem(Graph & graph, int k)
  {
  _item = -1;
  _object = nullptr;
  if (k < 0 || k >= int(graph.items().size()) || !graph.items()[std::size_t(k)].isFittable())
    {
    for (std::size_t i=0; i<graph.items().size(); i++) if (graph.items()[i].isFittable()) { k = int(i); break; }
    if (k < 0 || k >= int(graph.items().size()) || !graph.items()[std::size_t(k)].isFittable()) return;
    }
  const GraphItem & item = graph.items()[std::size_t(k)];
  _item = k;
  _object = item.object();
  _rank = item.rank();
  _low.assign(_rank, 0.0);
  _high.assign(_rank, 1.0);
  if (item.kind() == GraphItem::kData)
    {
    for (unsigned int a=0; a<_rank; a++)                            // the whole object
      {
      try { _low[a] = item.data()->minimum(a); _high[a] = item.data()->maximum(a); }
      catch (...) { _low[a] = 0; _high[a] = 1; }
      }
    if (!dynamic_cast<const MATH::Histogram*>(item.data())) { _statistic = 0; _binAveraging = false; }   // counts, bins
    }
  else
    {
    _low[0] = item.xMinimum(); _high[0] = item.xMaximum();
    if (_rank == 2) { _low[1] = item.yMinimum(); _high[1] = item.yMaximum(); }
    _statistic = 0;
    _binAveraging = false;
    }
  if (rankOf(_functionName) != _rank) makeFunction();                   // a function of the rank of the object
  else if (_function) _function->adaptToRange(_low, _high);
  }

  void FitDialog::estimate(Graph & graph)
  {
  if (!_function || _item < 0) return;
  const GraphItem & item = graph.items()[std::size_t(_item)];
  bool done = false;
  try
    {
    if (item.kind() == GraphItem::kData) done = MATH::Fitter::estimate(*_function, *item.data(), _low, _high);
    else
      {
      MATH::DataPointsVector samples = _rank == 1 ? MATH::Fitter::samples(*item.function(), _low[0], _high[0], (unsigned int)_samples)
        : MATH::Fitter::samples(*item.function(), _low[0], _high[0], (unsigned int)_samples, _low[1], _high[1], (unsigned int)_samples);
      done = MATH::Fitter::estimate(*_function, samples, _low, _high);
      }
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
    // the fit is made and attached by MATH::Fitter::fitAndAttach(), as a program would; the objects shown belong
    // to the application or its files: the fit is attached to them
    MATH::Fitter::FitOptions options;
    options.statistic = _statistic == 1 ? MATH::Fitter::kPoisson : MATH::Fitter::kChiSquare;
    options.search = _search;
    options.rangeLow = _low;
    options.rangeHigh = _high;
    options.binAveraging = _binAveraging;
    options.samples = (unsigned int)_samples;
    options.label = _label;
    MATH::FitRecord record = item.kind() == GraphItem::kData
      ? MATH::Fitter::fitAndAttach(*_function, *const_cast<MATH::DataPoints*>(item.data()), options)
      : MATH::Fitter::fitAndAttach(*_function, *const_cast<MATH::Function*>(item.function()), _low, _high, options);
    item.setFitShown(item.nFits() - 1, true);
    const MATH::FitResult & r = record.result;
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
    ImGui::TextDisabled("The selected graph shows no data points or function of one or two variables to fit.");
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
      if (!candidate.isFittable()) continue;
      if (ImGui::Selectable(itemName(candidate, int(k)).data(), int(k) == _item)) selectItem(*graph, int(k));
      }
    ImGui::EndCombo();
    }
  const bool histogram = item.kind() == GraphItem::kData && dynamic_cast<const MATH::Histogram*>(item.data());

  // the function: those of CAP and of programs, then the composites
  std::vector<String> names = MATH::FunctionRegistry::names(), composites = MATH::FunctionRegistry::compositeNames();
  if (!MATH::FunctionRegistry::isRegistered(_functionName) || rankOf(_functionName) != _rank) makeFunction();
  ImGui::SetNextItemWidth(180);
  if (ImGui::BeginCombo("function", _functionName.data(), ImGuiComboFlags_HeightLarge))
    {
    ImGui::SeparatorText(_rank == 2 ? "Functions of x and y" : "Functions of x");
    for (const auto & name : names)
      {
      if (MATH::FunctionRegistry::isComposite(name) || rankOf(name) != _rank) continue;
      if (ImGui::Selectable(name.data(), name == _functionName)) { _functionName = name; makeFunction(); }
      ImGui::SetItemTooltip("%s", describe(name).data());
      }
    if (!composites.empty())
      {
      ImGui::SeparatorText("Composites");
      for (const auto & name : composites)
        {
        if (rankOf(name) != _rank) continue;
        if (ImGui::Selectable(name.data(), name == _functionName)) { _functionName = name; makeFunction(); }
        ImGui::SetItemTooltip("%s", describe(name).data());
        }
      }
    ImGui::EndCombo();
    }
  if (hasDegree(_functionName))
    {
    ImGui::SameLine(0, 16);
    ImGui::SetNextItemWidth(90);
    if (ImGui::InputInt("degree", &_degree))
      {
      _degree = std::min(std::max(_degree, 0), _functionName == "Polynomial2D" ? 9 : 10);
      if (MATH::OrthogonalSeries * series = dynamic_cast<MATH::OrthogonalSeries*>(_function.get())) series->setDegree((unsigned int)_degree);
      else if (MATH::Polynomial2D * polynomial = dynamic_cast<MATH::Polynomial2D*>(_function.get())) polynomial->setDegree((unsigned int)_degree);
      else makeFunction();
      }
    }
  ImGui::SameLine(0, 16);
  if (ImGui::Button("Composite..."))
    {
    _compositeName = "";
    _drafts.clear();
    _drafts.push_back(ComponentDraft());
    _drafts.back().label = "signal";
    _operation = 0;
    _builderMessage = "";
    _builderRequested = true;
    }
  ImGui::SetItemTooltip("Make a composite: a sum of registered functions (e.g. a signal over a background), or a product f(x) g(y)");
  bool composite = MATH::FunctionRegistry::isComposite(_functionName);
  if (composite)
    {
    ImGui::SameLine();
    if (ImGui::Button("Keep values") && _function)
      {
      if (const MATH::ParameterizedComposite * c = dynamic_cast<const MATH::ParameterizedComposite*>(_function.get()))
        {
        MATH::FunctionRegistry::registerComposite(_functionName, *c);
        saveComposites();
        _message = String("The parameters of ") + _functionName + String(" are now its starting values");
        _error = false;
        }
      }
    ImGui::SetItemTooltip("Keep the current parameters (values, fixed, bounds) as the starting values of this composite");
    ImGui::SameLine();
    if (ImGui::Button("Remove"))
      {
      MATH::FunctionRegistry::removeComposite(_functionName);
      saveComposites();
      _message = String("Composite ") + _functionName + String(" removed");
      _error = false;
      _functionName = defaultFunction(_rank);
      makeFunction();
      }
    ImGui::SetItemTooltip("Remove this composite (its fits already made are kept)");
    }
  ImGui::TextDisabled("%s", describe(_functionName).data());
  if (!_function) ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.1f, 1.0f), "This function cannot be made");
  renderCompositeBuilder();

  // range, statistic, method
  ImGui::SeparatorText("Range and statistic");
  for (unsigned int a=0; a<_rank; a++)
    {
    double range[2] = { _low[a], _high[a] };
    ImGui::SetNextItemWidth(240);
    if (ImGui::InputScalarN(a == 0 ? "x range" : "y range", ImGuiDataType_Double, range, 2, nullptr, nullptr, "%.6g") && range[0] <= range[1])
      { _low[a] = range[0]; _high[a] = range[1]; if (_function) _function->adaptToRange(_low, _high); }
    if (a + 1 < _rank) continue;
    ImGui::SameLine();
    if (ImGui::Button("View"))
      {
      Graph::View view = graph->view();
      _low[0] = view.xMinimum; _high[0] = view.xMaximum;
      if (_rank == 2) { _low[1] = view.yMinimum; _high[1] = view.yMaximum; }
      if (_function) _function->adaptToRange(_low, _high);
      }
    ImGui::SetItemTooltip("Take the range shown in the graph");
    ImGui::SameLine();
    if (ImGui::Button("All")) selectItem(*graph, _item);
    ImGui::SetItemTooltip("Take the whole range of the object");
    }
  if (item.kind() == GraphItem::kFunction)
    {
    ImGui::SetNextItemWidth(120);
    if (ImGui::InputInt("samples", &_samples)) _samples = std::min(std::max(_samples, 2), _rank == 2 ? 300 : 100000);
    ImGui::SetItemTooltip(_rank == 2 ? "Points of the function fitted along each axis, on a grid over the range (each with an uncertainty of 1)"
                                     : "Points of the function fitted, equally spaced over the range (each with an uncertainty of 1)");
    }
  ImGui::RadioButton("chi square", &_statistic, 0);
  ImGui::SetItemTooltip("Least squares weighted by the statistical uncertainties (points without one are left out)");
  ImGui::SameLine();
  ImGui::BeginDisabled(!histogram);
  ImGui::RadioButton("Poisson likelihood", &_statistic, 1);
  ImGui::EndDisabled();
  ImGui::SetItemTooltip("For counts (histograms): unbiased where bins have few entries, empty bins included");
  if (!histogram) { _statistic = 0; _binAveraging = false; }
  ImGui::BeginDisabled(!histogram);
  ImGui::Checkbox("average over the bins", &_binAveraging);
  ImGui::EndDisabled();
  ImGui::SetItemTooltip("Compare each bin to the average of the function over it, not to its value at the center: no bias "
                        "for narrow peaks in wide bins (3 or 9 evaluations per bin)");
  ImGui::Checkbox("global search first (differential evolution)", &_search);
  ImGui::SetItemTooltip("When the starting values are poor or the statistic has several minima; needs bounds on the free parameters");

  // parameters
  ImGui::SeparatorText("Parameters");
  renderParameters();
  if (ImGui::Button("Estimate")) estimate(*graph);
  ImGui::SetItemTooltip("Starting values from the data in the range (Normal, LogNormal, Exponential, polynomials, series, Normal2D, Polynomial2D)");

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
