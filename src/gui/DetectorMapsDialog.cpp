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
#include "DetectorMapsDialog.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <string>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "CapFile.hpp"
#include "Exceptions.hpp"
#include "FileDialog.hpp"
#include "FileSystem.hpp"
#include "FunctionRegistry.hpp"
#include "Histogram.hpp"
#include "ParameterizedComposite.hpp"
#include "ParameterizedFunction.hpp"
#include "ParticleEfficiency.hpp"
#include "ParticleSmear.hpp"

namespace CAP
{
  namespace GUI
  {

  namespace
  {
    const char * const kEfficiency = "PARTICLE_EFFICIENCY";
    const char * const kSmearer    = "PARTICLE_SMEARER";
    const char * const variableLabels[3] = { "pT", "eta", "phi" };
    const char * const modelLabels[5]    = { "None", "Constant", "Function", "Histogram", "Curves" };
    const int kCurvesChoice = 4;   // "Curves": a function made of Bezier curves (see DetectorMapsDialog::isCurves())
    const char * const distributionLabels[4] = { "Gaussian", "Uniform", "Laplace", "Cauchy" };

    std::string key(const KinematicMapFolder & object, unsigned int k)
    { return object.name().str()+"/"+object.mapKey(k).str(); }

    bool isEfficiency(const KinematicMapFolder & object) { return object.kindName()==kEfficiency; }

    //! A histogram of the binning of an object for some variables, filled with a value.
    MATH::Histogram makeHistogram(const KinematicMapFolder & object, const std::vector<KinematicMap::Variable> & v,
                                  double value)
    {
    auto n  = [&](unsigned int a) { return (unsigned int) object.nBins(v[a]); };
    auto lo = [&](unsigned int a) { return object.binMinimum(v[a]); };
    auto hi = [&](unsigned int a) { return object.binMaximum(v[a]); };
    MATH::Histogram h = v.size()==1 ? MATH::Histogram("map","map",n(0),lo(0),hi(0))
                      : v.size()==2 ? MATH::Histogram("map","map",n(0),lo(0),hi(0),n(1),lo(1),hi(1))
                                    : MATH::Histogram("map","map",n(0),lo(0),hi(0),n(1),lo(1),hi(1),n(2),lo(2),hi(2));
    unsigned int nCells = h.binValues().size();
    for (unsigned int c=0; c<nCells; c++) h.setBinValueAtGlobal(c,value);
    return h;
    }

    //! Variables completed (or cut) to a rank, without repeating one.
    std::vector<KinematicMap::Variable> completed(std::vector<KinematicMap::Variable> v, unsigned int rank)
    {
    if (v.size()>rank) v.resize(rank);
    for (int x=0; v.size()<rank && x<3; x++)
      if (std::find(v.begin(),v.end(),KinematicMap::Variable(x))==v.end()) v.push_back(KinematicMap::Variable(x));
    return v;
    }

    String withExtension(const String & name)
    { return FileSystem::hasExtension(name,".cap") ? name : name+".cap"; }
  }

  DetectorMapsDialog::DetectorMapsDialog()
  :
  _open(false),
  _fileName(),
  _modified(false),
  _objects(),
  _selected(-1)
  { }

  DetectorMapsDialog::~DetectorMapsDialog()
  { }

  void DetectorMapsDialog::open(const String & fileName)
  {
  _open = true;
  if (fileName.isNull()) return;
  String error;
  if (!load(fileName,error)) _error = error.str();
  }

  // ---------------------------------------------------------------------------------------------------- files
  bool DetectorMapsDialog::load(const String & fileName, String & error)
  {
  try
    {
    std::unique_ptr<CapFile> file(CapFile::openOldCapFile(fileName));
    const Folder & top = file->folder();
    std::vector<std::unique_ptr<KinematicMapFolder>> objects;
    for (const String & name : KinematicMapFolder::objectNames(top,kEfficiency))
      {
      auto o = std::make_unique<ParticleEfficiency>();
      o->setName(name);
      o->loadFrom(top);
      objects.push_back(std::move(o));
      }
    for (const String & name : KinematicMapFolder::objectNames(top,kSmearer))
      {
      auto o = std::make_unique<ParticleSmear>();
      o->setName(name);
      o->loadFrom(top);
      objects.push_back(std::move(o));
      }
    if (objects.empty())
      {
      error = String("No particle efficiency or smearer in ")+fileName+" (a maps file has a folder per object)";
      return false;
      }
    stopEditing();
    _objects = std::move(objects);
    _fileName = fileName;
    _modified = false;
    _selected = 0;
    _rename = _objects[0]->name().str();
    _shown.clear();
    _error.clear();
    if (setStatus) setStatus(String("Detector maps read from ")+fileName);
    return true;
    }
  catch (const std::exception & e)
    {
    error = String("Cannot read ")+fileName+": "+e.what();
    return false;
    }
  }

  bool DetectorMapsDialog::save(const String & fileName, String & error)
  {
  String name = withExtension(fileName);
  try
    {
    std::unique_ptr<CapFile> file(CapFile::openRecreateCapFile(name));
    for (const auto & o : _objects) o->saveTo(file->folder());
    file->write();
    _fileName = name;
    _modified = false;
    if (setStatus) setStatus(String("Detector maps written to ")+name);
    return true;
    }
  catch (const std::exception & e)
    {
    error = String("Cannot write ")+name+": "+e.what();
    return false;
    }
  }

  KinematicMapFolder & DetectorMapsDialog::add(const String & kind, const String & name)
  {
  String unique = name;
  for (int n=2; std::any_of(_objects.begin(),_objects.end(),[&](const auto & o){ return o->name()==unique; }); n++)
    unique = name+"_"+String(std::to_string(n).c_str());
  std::unique_ptr<KinematicMapFolder> o;
  if (kind==kEfficiency) o = std::make_unique<ParticleEfficiency>();
  else                   o = std::make_unique<ParticleSmear>();
  o->setName(unique);
  _objects.push_back(std::move(o));
  _selected = int(_objects.size())-1;
  _rename = unique.str();
  _modified = true;
  return *_objects.back();
  }

  bool DetectorMapsDialog::attempt(const std::function<void()> & change)
  {
  try
    {
    change();
    _modified = true;
    _error.clear();
    return true;
    }
  catch (const std::exception & e)
    {
    _error = e.what();
    return false;
    }
  }

  const std::vector<unsigned int> & DetectorMapsDialog::variants(const String & name)
  {
  auto found = _variants.find(name.str());
  if (found!=_variants.end()) return found->second;
  std::vector<unsigned int> & v = _variants[name.str()];
  for (unsigned int n=0; n<=30; n++)
    {
    std::unique_ptr<MATH::ParameterizedFunction> f(MATH::FunctionRegistry::create(name,n));
    if (f) v.push_back(n);
    }
  return v;
  }

  const char * DetectorMapsDialog::mapLabel(const String & key)
  {
  static const std::map<std::string,const char *> labels = {
    {"EFFICIENCY","Efficiency"}, {"PT","Resolution of pT"}, {"ETA","Resolution of eta"}, {"PHI","Resolution of phi"},
    {"PT_BIAS","Bias of pT"}, {"ETA_BIAS","Bias of eta"}, {"PHI_BIAS","Bias of phi"} };
  auto found = labels.find(key.str());
  return found==labels.end() ? key.data() : found->second;
  }

  std::string DetectorMapsDialog::summary(const KinematicMap & map)
  {
  char text[160];
  std::string vars = KinematicMap::variablesText(map.variables()).str();
  switch (map.model())
    {
    case KinematicMap::kNone:     return "none";
    case KinematicMap::kConstant: std::snprintf(text,sizeof(text),"constant %g",map.constant()); return text;
    case KinematicMap::kFunction:
      {
      if (isCurves(map)) return "curves of "+vars;
      const auto * f = dynamic_cast<const MATH::ParameterizedFunction *>(map.function());
      std::string name = f ? MATH::FunctionRegistry::typeName(*f).str() : std::string("function");
      return name+" of "+vars;
      }
    case KinematicMap::kHistogram:
      return map.histogram() ? "histogram of "+vars : "histogram (not there yet)";
    }
  return "";
  }

  // ---------------------------------------------------------------------------------------------------- window
  void DetectorMapsDialog::render()
  {
  if (!_open) return;
  ImGui::SetNextWindowSize(ImVec2(1000.0f,640.0f),ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(windowName(),&_open)) { ImGui::End(); return; }
  if (ImGui::Button("New"))
    {
    stopEditing();
    _objects.clear(); _selected = -1; _fileName = String(); _modified = false; _shown.clear(); _error.clear();
    }
  ImGui::SameLine();
  if (ImGui::Button("Open...") && fileDialog)
    fileDialog->open(FileDialog::kOpenFile,"Open a maps file",FileSystem::current(),{".cap"},"",
                     [this](const String & path) { String e; if (!load(path,e)) _error = e.str(); });
  ImGui::SameLine();
  bool saveAs = ImGui::Button("Save As...");
  ImGui::SameLine();
  ImGui::BeginDisabled(_objects.empty());
  if (ImGui::Button("Save"))
    {
    if (_fileName.isNull()) saveAs = true;
    else { String e; if (!save(_fileName,e)) _error = e.str(); }
    }
  ImGui::EndDisabled();
  if (saveAs && fileDialog)
    fileDialog->open(FileDialog::kSaveFile,"Save the maps file",FileSystem::current(),{".cap"},
                     _fileName.isNull() ? String("maps.cap") : FileSystem::name(_fileName),
                     [this](const String & path) { String e; if (!save(path,e)) _error = e.str(); });
  ImGui::SameLine();
  ImGui::TextDisabled("%s%s",_fileName.isNull() ? "(not saved)" : _fileName.data(),_modified ? "  (modified)" : "");
  if (!_error.empty())
    {
    ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.75f,0.1f,0.1f,1.0f));
    ImGui::TextWrapped("%s",_error.c_str());
    ImGui::PopStyleColor();
    ImGui::SameLine();
    if (ImGui::SmallButton("OK")) _error.clear();
    }
  ImGui::Separator();
  ImGui::BeginChild("list",ImVec2(240.0f,0.0f),ImGuiChildFlags_Borders);
  renderList();
  ImGui::EndChild();
  ImGui::SameLine();
  ImGui::BeginChild("object",ImVec2(0.0f,0.0f),ImGuiChildFlags_Borders);
  if (_selected>=0 && _selected<int(_objects.size())) renderObject(*_objects[_selected]);
  else ImGui::TextDisabled("Open a maps file, or add an efficiency or a smearer.");
  ImGui::EndChild();
  ImGui::End();
  }

  void DetectorMapsDialog::renderList()
  {
  ImGui::TextDisabled("Efficiencies and smearers");
  for (unsigned int k=0; k<_objects.size(); k++)
    {
    const KinematicMapFolder & o = *_objects[k];
    std::string label = o.name().str()+(isEfficiency(o) ? "   efficiency" : "   smearer")+"###object"+std::to_string(k);
    if (ImGui::Selectable(label.c_str(),int(k)==_selected)) { _selected = int(k); _rename = o.name().str(); }
    }
  ImGui::Spacing();
  if (ImGui::Button("+ Efficiency")) add(kEfficiency,"Efficiency");
  ImGui::SameLine();
  if (ImGui::Button("+ Smearer")) add(kSmearer,"Smearer");
  if (_selected<0 || _selected>=int(_objects.size())) return;
  ImGui::Separator();
  ImGui::SetNextItemWidth(-1.0f);
  bool renamed = ImGui::InputText("##name",&_rename,ImGuiInputTextFlags_EnterReturnsTrue);
  if (ImGui::Button("Rename")) renamed = true;
  if (renamed && _rename!=_objects[_selected]->name().str())
    {
    bool used = std::any_of(_objects.begin(),_objects.end(),[&](const auto & o){ return o->name().str()==_rename; });
    if (_rename.empty() || _rename.find('/')!=std::string::npos || used)
      _error = "Choose a name not used yet, without '/'";
    else { _objects[_selected]->setName(String(_rename.c_str())); _modified = true; _error.clear(); }
    }
  ImGui::SameLine();
  if (ImGui::Button("Remove"))
    {
    if (_editing.object==_objects[_selected].get()) stopEditing();
    _objects.erase(_objects.begin()+_selected);
    _selected = _objects.empty() ? -1 : 0;
    if (_selected>=0) _rename = _objects[_selected]->name().str();
    _modified = true;
    }
  }

  void DetectorMapsDialog::renderObject(KinematicMapFolder & object)
  {
  ImGui::Text("%s  (%s)",object.name().data(),isEfficiency(object) ? "particle efficiency" : "particle smearer");
  if (auto * eff = dynamic_cast<ParticleEfficiency *>(&object))
    {
    std::string species;
    for (unsigned int k=0; k<eff->species().size(); k++) species += (k ? ", " : "")+std::to_string(eff->species()[k]);
    ImGui::SetNextItemWidth(220.0f);
    if (ImGui::InputText("Species (PDG codes; empty: all particles)",&species,ImGuiInputTextFlags_EnterReturnsTrue))
      attempt([&]() {
        std::vector<int> codes;
        for (double c : KinematicMap::parseNumbers(String(species.c_str()))) codes.push_back(int(c));
        eff->setSpecies(codes); });
    }
  ParticleSmear * smear = dynamic_cast<ParticleSmear *>(&object);
  for (unsigned int k=0; k<object.nMaps(); k++)
    {
    if (smear && k==3) ImGui::SeparatorText("Biases");
    std::string header = std::string(mapLabel(object.mapKey(k)))+":  "+summary(object.map(k))+"###map"+object.mapKey(k).str();
    ImGuiTreeNodeFlags flags = (k<3 || !object.map(k).isNone()) ? ImGuiTreeNodeFlags_DefaultOpen : 0;
    if (!ImGui::CollapsingHeader(header.c_str(),flags)) continue;
    ImGui::PushID(int(k));
    ImGui::Indent();
    renderMap(object,k);
    if (smear && k<3 && smear->smears(ParticleSmear::Observable(k)))
      {
      int d = int(smear->distribution(ParticleSmear::Observable(k)));
      if (d<=int(ParticleSmear::kCauchy))
        {
        ImGui::SetNextItemWidth(140.0f);
        if (ImGui::Combo("Deviates",&d,distributionLabels,4))
          attempt([&]() { smear->setDistribution(ParticleSmear::Observable(k),ParticleSmear::Distribution(d)); });
        }
      else ImGui::TextDisabled("Deviates: a function of the program");
      }
    ImGui::Unindent();
    ImGui::PopID();
    }
  ImGui::SeparatorText("Binning of the histograms made here (Make, To histogram)");
  for (int v=0; v<3; v++)
    {
    ImGui::PushID(100+v);
    int n = object.nBins(KinematicMap::Variable(v));
    double lo = object.binMinimum(KinematicMap::Variable(v)), hi = object.binMaximum(KinematicMap::Variable(v));
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%-4s",variableLabels[v]);
    ImGui::SameLine(60.0f);
    ImGui::SetNextItemWidth(90.0f);
    bool changed = ImGui::InputInt("bins",&n);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(90.0f);
    changed |= ImGui::InputDouble("min",&lo,0.0,0.0,"%g");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(90.0f);
    changed |= ImGui::InputDouble("max",&hi,0.0,0.0,"%g");
    if (changed) attempt([&]() { object.setBinning(KinematicMap::Variable(v),n,lo,hi); });
    ImGui::PopID();
    }
  }

  void DetectorMapsDialog::renderMap(KinematicMapFolder & object, unsigned int k)
  {
  const KinematicMap & m = object.map(k);
  bool efficiency = isEfficiency(object);
  double defaultValue = efficiency ? 1.0 : 0.0;
  int model = isCurves(m) ? kCurvesChoice : int(m.model());
  ImGui::SetNextItemWidth(140.0f);
  if (ImGui::BeginCombo("Source",modelLabels[model]))
    {
    for (int s=efficiency ? 1 : 0; s<5; s++)
      if (ImGui::Selectable(modelLabels[s],s==model) && s!=model)
        attempt([&]() {
          if (_editing.object==&object && _editing.k==k) stopEditing();
          if (s==kCurvesChoice)
            {
            double value = m.model()==KinematicMap::kConstant ? m.constant() : defaultValue;
            setCurves(object,k,{ Curve{KinematicMap::kPt,flatCurve(object,k,KinematicMap::kPt,value)} });
            return;
            }
          switch (s)
            {
            case KinematicMap::kNone: object.setMapNone(k); break;
            case KinematicMap::kConstant: object.setMapConstant(k,defaultValue); break;
            case KinematicMap::kFunction:
              {
              std::shared_ptr<MATH::ParameterizedFunction> f(MATH::FunctionRegistry::create("Polynomial",1));
              f->setParameterValue(0,defaultValue);
              object.setMapFunction(k,f,{});
              break;
              }
            case KinematicMap::kHistogram:
              object.setMapHistogram(k,makeHistogram(object,{KinematicMap::kPt},defaultValue),{KinematicMap::kPt});
              break;
            }
          });
    ImGui::EndCombo();
    }
  switch (object.map(k).model())
    {
    case KinematicMap::kNone:
      ImGui::SameLine();
      ImGui::TextDisabled(object.mapKey(k).endsWith("_BIAS") ? "no bias" : "not smeared");
      break;
    case KinematicMap::kConstant:
      {
      double v = object.map(k).constant();
      ImGui::SameLine();
      ImGui::SetNextItemWidth(120.0f);
      if (ImGui::InputDouble("Value",&v,0.0,0.0,"%g",ImGuiInputTextFlags_EnterReturnsTrue))
        attempt([&]() { object.setMapConstant(k,v); });
      break;
      }
    case KinematicMap::kFunction:  if (isCurves(object.map(k))) renderCurves(object,k); else renderFunction(object,k);  break;
    case KinematicMap::kHistogram: renderHistogram(object,k); break;
    }
  if (object.map(k).isNone()) return;
  if (ImGui::Button("Show")) show(object,k);
  auto shown = _shown.find(key(object,k));
  if (shown!=_shown.end())
    {
    ImGui::SameLine();
    unsigned int nFits = shown->second->nFits();
    ImGui::BeginDisabled(nFits==0);
    std::string label = nFits ? "Adopt fit ("+std::to_string(nFits)+")" : std::string("Adopt fit (fit it: Edit > Fit...)");
    if (ImGui::Button(label.c_str()))
      attempt([&]() {
        std::shared_ptr<const MATH::ParameterizedFunction> f = shown->second->fit(nFits-1).function;
        if (!f) throw Exception("The fit has no function","DetectorMapsDialog::renderMap()");
        std::vector<KinematicMap::Variable> v = object.map(k).variables();
        object.setMapFunction(k,f,v.size()==f->rank() ? v : std::vector<KinematicMap::Variable>{});
        if (setStatus) setStatus("Fitted function adopted");
        });
    ImGui::EndDisabled();
    }
  }

  bool DetectorMapsDialog::renderVariables(const char * id, std::vector<KinematicMap::Variable> & variables,
                                           unsigned int rank)
  {
  std::vector<KinematicMap::Variable> complete = completed(variables,rank);
  bool changed = complete!=variables;
  variables = complete;
  ImGui::PushID(id);
  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted("of");
  for (unsigned int a=0; a<rank; a++)
    {
    ImGui::SameLine();
    ImGui::PushID(int(a));
    int v = int(variables[a]);
    ImGui::SetNextItemWidth(70.0f);
    if (ImGui::Combo("##v",&v,variableLabels,3) && v!=int(variables[a]))
      {
      auto other = std::find(variables.begin(),variables.end(),KinematicMap::Variable(v));
      if (other!=variables.end()) *other = variables[a];               // swap: no variable twice
      variables[a] = KinematicMap::Variable(v);
      changed = true;
      }
    ImGui::PopID();
    }
  ImGui::PopID();
  return changed;
  }

  void DetectorMapsDialog::renderFunction(KinematicMapFolder & object, unsigned int k)
  {
  const KinematicMap & m = object.map(k);
  const auto * f = dynamic_cast<const MATH::ParameterizedFunction *>(m.function());
  if (!f) { ImGui::TextDisabled("A function of the program (it cannot be edited or saved here)"); return; }
  String name = MATH::FunctionRegistry::typeName(*f);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(170.0f);
  if (ImGui::BeginCombo("##function",name.data()))
    {
    std::vector<String> names = MATH::FunctionRegistry::names();
    for (const String & c : MATH::FunctionRegistry::compositeNames()) names.push_back(c);
    for (const String & n : names)
      {
      if (!ImGui::Selectable(n.data(),n==name) || n==name) continue;
      attempt([&]() {
        std::shared_ptr<MATH::ParameterizedFunction> g(MATH::FunctionRegistry::create(n));
        if (!g) throw Exception(String("Cannot make ")+n,"DetectorMapsDialog::renderFunction()");
        if (g->rank()>3) throw Exception(String(n)+" has more than 3 variables","DetectorMapsDialog::renderFunction()");
        object.setMapFunction(k,g,{});
        });
      }
    ImGui::EndCombo();
    }
  f = dynamic_cast<const MATH::ParameterizedFunction *>(object.map(k).function());
  if (!f) return;
  const std::vector<unsigned int> & counts = variants(MATH::FunctionRegistry::typeName(*f));
  if (counts.size()>1)
    {
    ImGui::SameLine();
    int n = int(f->nParameters());
    ImGui::SetNextItemWidth(90.0f);
    if (ImGui::BeginCombo("parameters",std::to_string(n).c_str()))
      {
      for (unsigned int c : counts)
        if (ImGui::Selectable(std::to_string(c).c_str(),int(c)==n) && int(c)!=n)
          attempt([&]() {
            std::shared_ptr<MATH::ParameterizedFunction> g(MATH::FunctionRegistry::create(MATH::FunctionRegistry::typeName(*f),c));
            for (unsigned int p=0; p<std::min(c,f->nParameters()); p++) g->setParameterValue(p,f->parameter(p).value());
            object.setMapFunction(k,g,object.map(k).variables());
            });
      ImGui::EndCombo();
      }
    }
  f = dynamic_cast<const MATH::ParameterizedFunction *>(object.map(k).function());
  std::vector<KinematicMap::Variable> v = object.map(k).variables();
  ImGui::SameLine();
  if (renderVariables("variables",v,f->rank())) attempt([&]() { object.setMapFunction(k,object.map(k).sharedFunction(),v); });
  // parameters by name, 4 per line
  for (unsigned int p=0; p<f->nParameters(); p++)
    {
    if (p%4) ImGui::SameLine();
    ImGui::PushID(int(p));
    double value = f->parameter(p).value();
    ImGui::SetNextItemWidth(100.0f);
    if (ImGui::InputDouble(f->parameter(p).name().data(),&value,0.0,0.0,"%g",ImGuiInputTextFlags_EnterReturnsTrue))
      attempt([&]() {
        std::shared_ptr<MATH::ParameterizedFunction> g(f->clone());
        g->setParameterValue(p,value);
        object.setMapFunction(k,g,object.map(k).variables());
        });
    ImGui::PopID();
    f = dynamic_cast<const MATH::ParameterizedFunction *>(object.map(k).function());
    }
  if (ImGui::Button("To histogram")) toHistogram(object,k);
  ImGui::SameLine();
  }

  void DetectorMapsDialog::renderHistogram(KinematicMapFolder & object, unsigned int k)
  {
  const MATH::Histogram * h = object.map(k).histogram();
  ImGui::SameLine();
  if (h)
    {
    std::string bins;
    for (unsigned int a=0; a<h->rank(); a++)
      bins += (a ? " x " : "")+std::to_string(h->nBins(a));
    std::vector<double> values = h->binValues();
    double lo = values.empty() ? 0.0 : *std::min_element(values.begin(),values.end());
    double hi = values.empty() ? 0.0 : *std::max_element(values.begin(),values.end());
    ImGui::Text("%s bins, values in [%g, %g]",bins.c_str(),lo,hi);
    std::vector<KinematicMap::Variable> v = object.map(k).variables();
    if (renderVariables("axes",v,h->rank()))
      attempt([&]() { MATH::Histogram copy(*h); object.setMapHistogram(k,copy,v); });
    }
  else ImGui::TextDisabled("no histogram yet");
  // drop target: a histogram of an open file
  ImGui::Button("Drop a histogram here from the file browser",ImVec2(-1.0f,0.0f));
  if (ImGui::BeginDragDropTarget())
    {
    if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload(FileBrowser::payloadType()))
      {
      ObjectReference reference;
      const MATH::Histogram * dropped = nullptr;
      if (FileBrowser::decodePayload(payload->Data,payload->DataSize,reference) && resolveHistogram)
        dropped = resolveHistogram(reference);
      if (!dropped) _error = "Only a histogram can be dropped here";
      else attempt([&]() { object.setMapHistogram(k,*dropped,{}); });
      }
    ImGui::EndDragDropTarget();
    }
  // made empty with the binning of the object
  static std::map<std::string,int> dims;
  static std::map<std::string,double> fill;
  std::string id = key(object,k);
  if (!dims.count(id)) dims[id] = 1;
  if (!fill.count(id)) fill[id] = isEfficiency(object) ? 1.0 : 0.0;
  if (ImGui::Button("Make"))
    attempt([&]() {
      std::vector<KinematicMap::Variable> chosen = completed(object.map(k).variables(),unsigned(dims[id]));
      object.setMapHistogram(k,makeHistogram(object,chosen,fill[id]),chosen);
      });
  ImGui::SameLine();
  ImGui::SetNextItemWidth(60.0f);
  int dimIndex = dims[id]-1;
  if (ImGui::Combo("dims",&dimIndex,"1\0" "2\0" "3\0")) dims[id] = dimIndex+1;
  ImGui::SameLine();
  ImGui::SetNextItemWidth(90.0f);
  ImGui::InputDouble("filled with",&fill[id],0.0,0.0,"%g");
  ImGui::SameLine();
  ImGui::TextDisabled("(binning below)");
  if (h)
    {
    bool & edit = _editBins[id];
    ImGui::Checkbox("Edit bins",&edit);
    if (edit) renderBins(object,k);
    }
  }

  void DetectorMapsDialog::renderBins(KinematicMapFolder & object, unsigned int k)
  {
  MATH::Histogram * h = object.mutableMapHistogram(k);
  if (!h) return;
  unsigned int rank = h->rank();
  static std::map<std::string,int> slices;
  int & slice = slices[key(object,k)];
  if (rank==3)
    {
    int n = int(h->nBins(2));
    slice = std::max(0,std::min(slice,n-1));
    ImGui::SetNextItemWidth(200.0f);
    ImGui::SliderInt("slice of the third axis",&slice,0,n-1);
    }
  unsigned int rows = rank==1 ? 1 : h->nBins(0);
  unsigned int columns = rank==1 ? h->nBins(0) : h->nBins(1);
  if (columns>63) { ImGui::TextDisabled("Too many bins along an axis to edit here (63 at most)"); return; }
  ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                          ImGuiTableFlags_SizingFixedFit;
  float height = std::min(300.0f,ImGui::GetFrameHeightWithSpacing()*(rows+2));
  if (!ImGui::BeginTable("bins",int(columns)+1,flags,ImVec2(0.0f,height))) return;
  ImGui::TableSetupScrollFreeze(1,1);
  std::vector<KinematicMap::Variable> vars = object.map(k).variables();
  String corner;
  if (rank==1) corner = vars.size()>0 ? String(KinematicMap::variableName(vars[0])) : String("");
  else if (vars.size()>=2) corner = String(KinematicMap::variableName(vars[0]))+" \\ "+KinematicMap::variableName(vars[1]);
  else corner = "axis 1 \\ 2";
  ImGui::TableSetupColumn(corner.c_str());
  char text[32];
  for (unsigned int c=0; c<columns; c++)
    {
    std::snprintf(text,sizeof(text),"%.3g",h->binCenter(c,rank==1 ? 0 : 1));
    ImGui::TableSetupColumn(text);
    }
  ImGui::TableHeadersRow();
  for (unsigned int r=0; r<rows; r++)
    {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    if (rank==1) ImGui::TextUnformatted("value");
    else ImGui::Text("%.3g",h->binCenter(r,0));
    for (unsigned int c=0; c<columns; c++)
      {
      ImGui::TableSetColumnIndex(int(c)+1);
      ImGui::PushID(int(r*64+c));
      double value = rank==1 ? h->binValueAt(c) : rank==2 ? h->binValueAt(r,c) : h->binValueAt(r,c,unsigned(slice));
      ImGui::SetNextItemWidth(64.0f);
      if (ImGui::InputDouble("##b",&value,0.0,0.0,"%.4g",ImGuiInputTextFlags_EnterReturnsTrue))
        {
        double old = rank==1 ? h->binValueAt(c) : rank==2 ? h->binValueAt(r,c) : h->binValueAt(r,c,unsigned(slice));
        auto set = [&](double x) {
          if (rank==1) h->setBinValueAt(c,x); else if (rank==2) h->setBinValueAt(r,c,x); else h->setBinValueAt(r,c,unsigned(slice),x); };
        set(value);
        if (!attempt([&]() { object.checkRange(k,*h); })) set(old);
        }
      ImGui::PopID();
      }
    }
  ImGui::EndTable();
  }

  void DetectorMapsDialog::toHistogram(KinematicMapFolder & object, unsigned int k)
  {
  attempt([&]() {
    std::vector<KinematicMap::Variable> v = object.map(k).variables();
    MATH::Histogram h = makeHistogram(object,v,0.0);
    unsigned int nCells = h.binValues().size();
    std::vector<unsigned int> n(v.size());
    for (unsigned int a=0; a<v.size(); a++) n[a] = h.nBins(a);
    for (unsigned int c=0; c<nCells; c++)
      {
      double point[3] = { 0.0, 0.0, 0.0 };
      unsigned int rest = c;
      std::vector<unsigned int> index(v.size());
      for (int a=int(v.size())-1; a>=0; a--) { index[a] = rest%n[a]; rest /= n[a]; }
      for (unsigned int a=0; a<v.size(); a++) point[v[a]] = h.binCenter(index[a],a);
      double value = object.mapValue(k,point[0],point[1],point[2]);
      if (v.size()==1) h.setBinValueAt(index[0],value);
      else if (v.size()==2) h.setBinValueAt(index[0],index[1],value);
      else h.setBinValueAt(index[0],index[1],index[2],value);
      }
    object.setMapHistogram(k,h,v);
    if (setStatus) setStatus("Function sampled into a histogram");
    });
  }

  void DetectorMapsDialog::show(KinematicMapFolder & object, unsigned int k)
  {
  const KinematicMap & m = object.map(k);
  std::string label = key(object,k);
  if (m.model()==KinematicMap::kHistogram && m.histogram())
    {
    if (!showData) return;
    MATH::Histogram * copy = new MATH::Histogram(*m.histogram());
    copy->setName(String(label.c_str()));
    copy->setTitle(String(mapLabel(object.mapKey(k))));
    std::vector<KinematicMap::Variable> axes = m.variables();
    for (unsigned int a=0; a<axes.size() && a<copy->rank(); a++)
      copy->setDimensionTitle(a,axes[a]==KinematicMap::kPt ? String("pT") : String(KinematicMap::variableName(axes[a])));
    showData(copy);
    _shown[label] = copy;
    return;
    }
  std::vector<KinematicMap::Variable> v = m.variables();
  std::shared_ptr<const MATH::Function> f = m.sharedFunction();
  MATH::Function * shown = nullptr;
  if (m.model()==KinematicMap::kConstant)
    {
    MATH::ParameterizedFunction * c = MATH::FunctionRegistry::create("Constant");
    c->setParameterValue(0,m.constant());
    shown = c;
    v = { KinematicMap::kPt };
    }
  else if (const auto * p = dynamic_cast<const MATH::ParameterizedFunction *>(f.get())) shown = p->clone();
  if (!shown) { _error = "This function cannot be shown (not a function of MATH::FunctionRegistry)"; return; }
  if (v.size()==1 && showFunction)
    showFunction(shown,object.binMinimum(v[0]),object.binMaximum(v[0]));
  else if (v.size()==2 && showFunction2D)
    showFunction2D(shown,object.binMinimum(v[0]),object.binMaximum(v[0]),object.binMinimum(v[1]),object.binMaximum(v[1]));
  else { delete shown; _error = "A function of 3 variables is shown here once made a histogram (To histogram)"; }
  }

  // ---------------------------------------------------------------------------------------------------- curves

  bool DetectorMapsDialog::isCurves(const KinematicMap & map)
  {
  const MATH::Function * f = map.function();
  if (!f) return false;
  if (dynamic_cast<const MATH::BezierCurve *>(f)) return true;
  const auto * product = dynamic_cast<const MATH::ParameterizedComposite *>(f);
  if (!product || product->operation()!=MATH::ParameterizedComposite::kProduct || product->nComponents()==0) return false;
  for (unsigned int c=0; c<product->nComponents(); c++)
    if (!dynamic_cast<const MATH::BezierCurve *>(&product->component(c))) return false;
  return product->nComponents()==map.variables().size();
  }

  std::vector<DetectorMapsDialog::Curve> DetectorMapsDialog::curvesOf(const KinematicMap & map)
  {
  std::vector<Curve> curves;
  if (!isCurves(map)) return curves;
  const std::vector<KinematicMap::Variable> & v = map.variables();
  if (const auto * curve = dynamic_cast<const MATH::BezierCurve *>(map.function()))
    curves.push_back(Curve{v[0],*curve});
  else
    {
    const auto & product = dynamic_cast<const MATH::ParameterizedComposite &>(*map.function());
    for (unsigned int c=0; c<product.nComponents(); c++)
      curves.push_back(Curve{v[c],dynamic_cast<const MATH::BezierCurve &>(product.component(c))});
    }
  return curves;
  }

  void DetectorMapsDialog::boundsOf(const KinematicMapFolder & object, unsigned int k, double & lower, double & upper)
  {
  lower = MATH::Parameter::defaultMinimum;
  upper = MATH::Parameter::defaultMaximum;
  if (isEfficiency(object)) { lower = 0.0; upper = 1.0; }
  else if (!object.mapKey(k).endsWith("_BIAS")) lower = 0.0;   // a resolution
  }

  MATH::BezierCurve DetectorMapsDialog::flatCurve(const KinematicMapFolder & object, unsigned int k,
                                                  KinematicMap::Variable v, double value)
  {
  double lower, upper;
  boundsOf(object,k,lower,upper);
  double x0 = object.binMinimum(v), x1 = object.binMaximum(v);
  if (!(x1>x0)) x1 = x0+1.0;
  return MATH::BezierCurve({x0,0.5*(x0+x1),x1},{value,value,value},lower,upper);
  }

  void DetectorMapsDialog::setCurves(KinematicMapFolder & object, unsigned int k, std::vector<Curve> curves)
  {
  if (curves.empty()) throw Exception("A map of curves has at least one curve","DetectorMapsDialog::setCurves()");
  std::sort(curves.begin(),curves.end(),[](const Curve & a, const Curve & b) { return a.variable<b.variable; });
  double lower, upper;
  boundsOf(object,k,lower,upper);
  std::vector<KinematicMap::Variable> variables;
  for (Curve & c : curves)
    {
    if (!variables.empty() && variables.back()==c.variable)
      throw Exception("A variable has two curves","DetectorMapsDialog::setCurves()");
    variables.push_back(c.variable);
    if (c.curve.lower()!=lower || c.curve.upper()!=upper) c.curve.setBounds(lower,upper);
    }
  std::shared_ptr<MATH::ParameterizedFunction> f;
  if (curves.size()==1) f.reset(curves[0].curve.clone());
  else
    {
    auto product = std::make_shared<MATH::ParameterizedComposite>(MATH::ParameterizedComposite::kProduct);
    for (const Curve & c : curves) product->add(String(KinematicMap::variableName(c.variable)),c.curve);
    f = product;
    }
  object.setMapFunction(k,f,variables);
  // the curve edited on a canvas follows (or is no longer edited, when its variable is gone)
  if (_editing.curve && _editing.object==&object && _editing.k==k)
    {
    bool found = false;
    for (const Curve & c : curves)
      if (c.variable==_editing.variable) { *_editing.curve = c.curve; found = true; }
    if (!found) stopEditing();
    }
  }

  void DetectorMapsDialog::stopEditing()
  {
  if (_editing.curve && stopEditingCurve) stopEditingCurve(_editing.curve);
  _editing = Editing();
  }

  void DetectorMapsDialog::editOnCanvas(KinematicMapFolder & object, unsigned int k, KinematicMap::Variable v)
  {
  if (!editCurve) return;
  std::vector<Curve> curves = curvesOf(object.map(k));
  const Curve * found = nullptr;
  for (const Curve & c : curves) if (c.variable==v) found = &c;
  if (!found) return;
  stopEditing();
  _editedCurves.push_back(std::make_unique<MATH::BezierCurve>(found->curve));
  MATH::BezierCurve * curve = _editedCurves.back().get();
  std::string title = std::string(mapLabel(object.mapKey(k)))+(curves.size()>1 ? std::string(": factor of ") : std::string(" of "))
                      +variableLabels[v];
  curve->setName(String((key(object,k)+"/"+KinematicMap::variableName(v)).c_str()));
  curve->setTitle(String(title.c_str()));
  _editing.object   = &object;
  _editing.k        = k;
  _editing.variable = v;
  _editing.curve    = curve;
  // x: the binning of the variable (and the knots); y: [0, 1] for an efficiency, room above the knots otherwise
  double x0 = std::min(object.binMinimum(v),curve->knotX(0));
  double x1 = std::max(object.binMaximum(v),curve->knotX(curve->nKnots()-1));
  double lower, upper, largest = 0.0;
  boundsOf(object,k,lower,upper);
  for (unsigned int j=0; j<curve->nKnots(); j++) largest = std::max(largest,std::fabs(curve->knotY(j)));
  if (!(largest>0.0)) largest = 1.0;
  double y0 = lower>MATH::Parameter::defaultMinimum ? lower : -1.5*largest;
  double y1 = upper<MATH::Parameter::defaultMaximum ? upper*1.05 : 1.5*largest;
  KinematicMapFolder * target = &object;
  editCurve(curve,String(title.c_str()),x0,x1,y0,y1,[this,target,k,v]()
    {
    bool exists = false;
    for (const auto & o : _objects) if (o.get()==target) exists = true;
    if (!exists || !_editing.curve || _editing.object!=target || _editing.k!=k || _editing.variable!=v) return;
    std::vector<Curve> curves = curvesOf(target->map(k));
    for (Curve & c : curves) if (c.variable==v) c.curve = *_editing.curve;
    attempt([&]() { setCurves(*target,k,curves); });
    });
  if (setStatus) setStatus("Drag the knots of the curve; click on the curve to add a knot, right-click a knot to remove it");
  }

  void DetectorMapsDialog::renderCurves(KinematicMapFolder & object, unsigned int k)
  {
  std::vector<Curve> curves = curvesOf(object.map(k));
  bool efficiency = isEfficiency(object);
  ImGui::SameLine();
  ImGui::TextDisabled(curves.size()>1 ? "(the product of the curves)" : "(a curve of pT, eta or phi)");
  for (int v=0; v<3; v++)
    {
    ImGui::PushID(1000+v);
    KinematicMap::Variable variable = KinematicMap::Variable(v);
    int index = -1;
    for (unsigned int c=0; c<curves.size(); c++) if (curves[c].variable==variable) index = int(c);
    bool used = index>=0;
    if (ImGui::Checkbox(variableLabels[v],&used))
      attempt([&]() {
        std::vector<Curve> changed = curves;
        if (used)
          {
          // a new factor of 1 (a curve alone: the value of the map)
          double value = curves.empty() ? (efficiency ? 1.0 : 0.0) : 1.0;
          changed.push_back(Curve{variable,flatCurve(object,k,variable,value)});
          }
        else
          {
          if (curves.size()<=1) throw Exception("A map of curves keeps at least one curve","DetectorMapsDialog::renderCurves()");
          changed.erase(changed.begin()+index);
          }
        setCurves(object,k,changed);
        });
    if (used && index>=0)
      {
      const MATH::BezierCurve & curve = curves[std::size_t(index)].curve;
      bool editing = _editing.curve && _editing.object==&object && _editing.k==k && _editing.variable==variable;
      ImGui::SameLine(80.0f);
      ImGui::Text("%u knots",curve.nKnots());
      ImGui::SameLine(160.0f);
      if (editing)
        {
        if (ImGui::Button("Stop editing")) stopEditing();
        }
      else if (ImGui::Button("Edit on canvas")) editOnCanvas(object,k,variable);
      ImGui::SetItemTooltip("Show the curve on the current canvas: drag its knots, click on the curve to add one, "
                            "right-click a knot to remove it");
      ImGui::SameLine();
      std::string tableKey = key(object,k)+"/"+variableLabels[v];
      bool & table = _knotTables[tableKey];
      if (ImGui::Button(table ? "Hide knots" : "Knots")) table = !table;
      ImGui::SameLine();
      if (ImGui::Button("Flat"))
        attempt([&]() {
          std::vector<Curve> changed = curves;
          double mean = curve.integralInRange(curve.knotX(0),curve.knotX(curve.nKnots()-1))
                        /(curve.knotX(curve.nKnots()-1)-curve.knotX(0));
          changed[std::size_t(index)].curve = flatCurve(object,k,variable,mean);
          setCurves(object,k,changed);
          });
      ImGui::SetItemTooltip("Back to 3 knots on a flat line at the mean value");
      if (table && ImGui::BeginTable("knots",3,ImGuiTableFlags_Borders|ImGuiTableFlags_SizingFixedFit))
        {
        ImGui::TableSetupColumn(variableLabels[v]);
        ImGui::TableSetupColumn("value");
        ImGui::TableSetupColumn("");
        ImGui::TableHeadersRow();
        for (unsigned int j=0; j<curve.nKnots(); j++)
          {
          ImGui::PushID(int(j));
          ImGui::TableNextRow();
          double x = curve.knotX(j), y = curve.knotY(j);
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(90.0f);
          bool changedX = ImGui::InputDouble("##x",&x,0.0,0.0,"%g",ImGuiInputTextFlags_EnterReturnsTrue);
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(90.0f);
          bool changedY = ImGui::InputDouble("##y",&y,0.0,0.0,"%g",ImGuiInputTextFlags_EnterReturnsTrue);
          if (changedX || changedY)
            attempt([&]() {
              std::vector<Curve> changed = curves;
              changed[std::size_t(index)].curve.moveKnot(j,x,y);
              setCurves(object,k,changed);
              });
          ImGui::TableNextColumn();
          ImGui::BeginDisabled(curve.nKnots()<=2);
          if (ImGui::SmallButton("x"))
            attempt([&]() {
              std::vector<Curve> changed = curves;
              changed[std::size_t(index)].curve.removeKnot(j);
              setCurves(object,k,changed);
              });
          ImGui::EndDisabled();
          ImGui::PopID();
          }
        ImGui::EndTable();
        }
      }
    ImGui::PopID();
    }
  if (ImGui::Button("To histogram")) toHistogram(object,k);
  ImGui::SameLine();
  }

  } // namespace GUI
} // namespace CAP
