/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,04/01/2024
 *
 * *********************************************************************/
#include "KinematicMapFolder.hpp"
#include "Exceptions.hpp"
#include "FunctionRegistry.hpp"
#include "MathConstants.hpp"
#include "NameHelpers.hpp"
#include "ParameterizedFunction.hpp"
#include "StringParameter.hpp"
#include <cstdio>
#include <cmath>
#include <string>

namespace CAP
{

  KinematicMapFolder::KinematicMapFolder()
  :
  HistogramFolder(),
  _maps()
  {
  appendClassName("KinematicMapFolder");
  setBinning(KinematicMap::kPt,100,0.0,10.0);
  setBinning(KinematicMap::kEta,20,-1.0,1.0);
  setBinning(KinematicMap::kPhi,36,0.0,MATH::twoPi());
  }

  KinematicMapFolder::KinematicMapFolder(const KinematicMapFolder & source)
  :
  HistogramFolder(source),
  _maps(source._maps)
  {
  for (int v=0; v<3; v++) { _nBins[v] = source._nBins[v]; _minimum[v] = source._minimum[v]; _maximum[v] = source._maximum[v]; }
  repoint(source);
  }

  KinematicMapFolder & KinematicMapFolder::operator=(const KinematicMapFolder & rhs)
  {
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    _maps = rhs._maps;
    for (int v=0; v<3; v++) { _nBins[v] = rhs._nBins[v]; _minimum[v] = rhs._minimum[v]; _maximum[v] = rhs._maximum[v]; }
    repoint(rhs);
    }
  return *this;
  }

  void KinematicMapFolder::repoint(const KinematicMapFolder & source)
  {
  for (unsigned int k=0; k<_maps.size(); k++)
    if (const MATH::Histogram * h = source._maps[k].map.histogram())
      _maps[k].map.repointHistogram(histogramLike(h));
  }

  unsigned int KinematicMapFolder::addMap(const String & key, const KinematicMap & initial, double lowest, double highest)
  {
  _maps.push_back({key,initial,lowest,highest,initial.model()==KinematicMap::kConstant ? initial.constant() : 0.0});
  return _maps.size()-1;
  }

  unsigned int KinematicMapFolder::mapIndex(const String & key) const
  {
  for (unsigned int k=0; k<_maps.size(); k++) if (_maps[k].key==key) return k;
  throw Exception(String("No map ")+key,"KinematicMapFolder::mapIndex()");
  }

  namespace
  {
    //! Whether a base name is given (empty and NONE: no base name).
    bool hasBase(const String & base) { return !base.isEmpty() && base!="NONE"; }
  }

  String KinematicMapFolder::folderName() const
  {
  return hasBase(_histogramBaseName) ? _histogramBaseName : String(name());
  }

  std::vector<String> KinematicMapFolder::objectNames(const Folder & file, const String & kind)
  {
  std::vector<String> names;
  for (const String & entry : file.names())
    if (Folder * sub = file.folder(entry))
      {
      String k;
      if (readSetting(*sub,"KIND",k) && k==kind) names.push_back(entry);
      }
  return names;
  }

  void KinematicMapFolder::setBinning(KinematicMap::Variable v, int nBins, double minimum, double maximum)
  {
  if (nBins<1 || !(maximum>minimum))
    throw Exception(String("Empty binning for ")+KinematicMap::variableName(v),"KinematicMapFolder::setBinning()");
  _nBins[v] = nBins; _minimum[v] = minimum; _maximum[v] = maximum;
  }

  double KinematicMapFolder::mapValue(unsigned int k, double pt, double eta, double phi) const
  {
  const MapEntry & e = _maps[k];
  double v = e.map.value(pt,eta,phi);
  if (std::isnan(v)) return std::isfinite(e.lowest) ? e.lowest : 0.0;
  if (v<e.lowest)  return e.lowest;
  if (v>e.highest) return e.highest;
  return v;
  }

  void KinematicMapFolder::setMapNone(unsigned int k)
  {
  _maps[k].map.setNone();
  }

  void KinematicMapFolder::setMapConstant(unsigned int k, double value)
  {
  MapEntry & e = _maps[k];
  if (!(value>=e.lowest && value<=e.highest))
    throw Exception(String("Value of ")+e.key+" outside its allowed range","KinematicMapFolder::setMapConstant()");
  e.map.setConstant(value);
  e.createValue = value;
  }

  void KinematicMapFolder::setMapFunction(unsigned int k, std::shared_ptr<const MATH::Function> function,
                                          const std::vector<KinematicMap::Variable> & variables)
  {
  _maps[k].map.setFunction(function,variables);
  }

  void KinematicMapFolder::checkRange(unsigned int k, const MATH::Histogram & histogram) const
  {
  const MapEntry & e = _maps[k];
  for (double v : histogram.binValues())
    if (!(v>=e.lowest && v<=e.highest))
      throw Exception(String("Histogram ")+histogram.name()+": a value is outside the allowed range of "+e.key,
                      "KinematicMapFolder::checkRange()");
  }

  void KinematicMapFolder::setMapHistogram(unsigned int k, const MATH::Histogram & histogram,
                                           const std::vector<KinematicMap::Variable> & variables)
  {
  checkRange(k,histogram);
  std::vector<KinematicMap::Variable> checked = KinematicMap::checkedVariables(histogram.rank(),variables);
  String name = mapHistogramName(k);
  if (contains(name)) remove(name);
  MATH::Histogram * copy = histogram.clone();
  copy->setName(name);
  append(copy);
  _maps[k].map.setHistogram(copy,checked);
  }

  void KinematicMapFolder::configure(const String & taskName, const String & typeName,
                                     const Configuration & configuration, unsigned int index)
  {
  HistogramFolder::configure(taskName,typeName,configuration,index);
  for (int v=0; v<3; v++)
    {
    String name = KinematicMap::variableName(KinematicMap::Variable(v));
    int    n    = configuration.valueInt(   createKey(taskName,typeName,index,name+"_nbins"),true,_nBins[v]);
    double lo   = configuration.valueDouble(createKey(taskName,typeName,index,name+"_min"),  true,_minimum[v]);
    double hi   = configuration.valueDouble(createKey(taskName,typeName,index,name+"_max"),  true,_maximum[v]);
    setBinning(KinematicMap::Variable(v),n,lo,hi);
    }
  for (unsigned int k=0; k<_maps.size(); k++)
    {
    MapEntry & e = _maps[k];
    String key   = e.key;
    String model = configuration.valueString(createKey(taskName,typeName,index,key+":MODEL"),true,
                                             KinematicMap::modelName(e.map.model()));
    double value = configuration.valueDouble(createKey(taskName,typeName,index,key+":VALUE"),true,e.createValue);
    String variablesText = configuration.valueString(createKey(taskName,typeName,index,key+":VARIABLES"),true,"");
    std::vector<KinematicMap::Variable> variables = KinematicMap::parseVariables(variablesText);
    switch (KinematicMap::parseModel(model))
      {
      case KinematicMap::kNone:     setMapNone(k); break;
      case KinematicMap::kConstant: setMapConstant(k,value); break;
      case KinematicMap::kHistogram:
        {
        if (!(value>=e.lowest && value<=e.highest))
          throw Exception(String("VALUE of ")+key+" outside its allowed range","KinematicMapFolder::configure()");
        e.createValue = value;
        e.map.expectHistogram(variables.empty() ? std::vector<KinematicMap::Variable>{KinematicMap::kPt} : variables);
        break;
        }
      case KinematicMap::kFunction:
        {
        String functionName = configuration.valueString(createKey(taskName,typeName,index,key+":FUNCTION"),true,"NONE");
        String parameters   = configuration.valueString(createKey(taskName,typeName,index,key+":PARAMETERS"),true,"");
        std::vector<double> values = KinematicMap::parseNumbers(parameters);
        if (!MATH::FunctionRegistry::isRegistered(functionName))
          throw Exception(String("Unknown function ")+functionName+" for "+key+" (see MATH::FunctionRegistry)",
                          "KinematicMapFolder::configure()");
        std::shared_ptr<MATH::ParameterizedFunction> f(values.empty()
                                                       ? MATH::FunctionRegistry::create(functionName)
                                                       : MATH::FunctionRegistry::create(functionName,values.size()));
        if (!f) throw Exception(String("Function ")+functionName+" cannot be made with "
                                +String(std::to_string(values.size()).c_str())+" parameters","KinematicMapFolder::configure()");
        if (!values.empty())
          {
          if (f->nParameters()!=values.size())
            throw Exception(String("Function ")+functionName+" has "+String(std::to_string(f->nParameters()).c_str())
                            +" parameters, "+String(std::to_string(values.size()).c_str())+" given for "+key,
                            "KinematicMapFolder::configure()");
          f->setParameterValues(values);
          }
        setMapFunction(k,f,variables);          // empty variables: the default of its rank
        break;
        }
      }
    }
  }

  void KinematicMapFolder::create()
  {
  for (unsigned int k=0; k<_maps.size(); k++)
    {
    MapEntry & e = _maps[k];
    if (e.map.model()!=KinematicMap::kHistogram) continue;
    const std::vector<KinematicMap::Variable> & v = e.map.variables();
    MATH::Histogram * h = nullptr;
    String name = mapHistogramName(k);
    switch (v.size())
      {
      case 1: h = new MATH::Histogram(name,name,_nBins[v[0]],_minimum[v[0]],_maximum[v[0]]); break;
      case 2: h = new MATH::Histogram(name,name,_nBins[v[0]],_minimum[v[0]],_maximum[v[0]],
                                                 _nBins[v[1]],_minimum[v[1]],_maximum[v[1]]); break;
      default: h = new MATH::Histogram(name,name,_nBins[v[0]],_minimum[v[0]],_maximum[v[0]],
                                                  _nBins[v[1]],_minimum[v[1]],_maximum[v[1]],
                                                  _nBins[v[2]],_minimum[v[2]],_maximum[v[2]]); break;
      }
    unsigned int nCells = h->binValues().size();
    for (unsigned int i=0; i<nCells; i++) h->setBinValueAtGlobal(i,e.createValue);
    setMapHistogram(k,*h,v);
    delete h;
    }
  }

  void KinematicMapFolder::writeSetting(Folder & outputFile, const String & name, const String & value)
  {
  if (outputFile.contains(name)) outputFile.remove(name);
  outputFile.add(new StringParameter(name,value),name);
  }

  bool KinematicMapFolder::readSetting(const Folder & inputFile, const String & name, String & value)
  {
  const StringParameter * p = inputFile.get<StringParameter>(name);
  if (!p) return false;
  value = p->value();
  return true;
  }

  void KinematicMapFolder::loadFrom(const Folder & file)
  {
  const Folder * found = file.folder(folderName());
  if (!found) { loadLegacy(file); return; }
  const Folder & inputFile = *found;
  for (unsigned int k=0; k<_maps.size(); k++)
    {
    MapEntry & e = _maps[k];
    String modelText;
    if (readSetting(inputFile,mapSettingName(k,"MODEL"),modelText))
      {
      // self-describing: the settings of the file replace those of the configuration
      String variablesText, valueText;
      readSetting(inputFile,mapSettingName(k,"VARIABLES"),variablesText);
      std::vector<KinematicMap::Variable> variables = KinematicMap::parseVariables(variablesText);
      double value = e.createValue;
      if (readSetting(inputFile,mapSettingName(k,"VALUE"),valueText))
        {
        std::vector<double> numbers = KinematicMap::parseNumbers(valueText);
        if (numbers.size()!=1) throw Exception(String("Invalid ")+mapSettingName(k,"VALUE")+": "+valueText,
                                               "KinematicMapFolder::loadFrom()");
        value = numbers[0];
        }
      switch (KinematicMap::parseModel(modelText))
        {
        case KinematicMap::kNone:     setMapNone(k); break;
        case KinematicMap::kConstant: setMapConstant(k,value); break;
        case KinematicMap::kFunction:
          {
          const MATH::ParameterizedFunction * f = inputFile.get<MATH::ParameterizedFunction>(mapFunctionName(k));
          if (!f) throw Exception(String("No function ")+mapFunctionName(k)+" in the imported folder",
                                  "KinematicMapFolder::loadFrom()");
          setMapFunction(k,std::shared_ptr<const MATH::Function>(f->clone()),variables);
          break;
          }
        case KinematicMap::kHistogram:
          {
          const MATH::Histogram * h = inputFile.get<MATH::Histogram>(mapHistogramName(k));
          if (!h) throw Exception(String("No histogram ")+mapHistogramName(k)+" in the imported folder",
                                  "KinematicMapFolder::loadFrom()");
          e.createValue = value;
          setMapHistogram(k,*h,variables);
          break;
          }
        }
      continue;
      }
    throw Exception(String("No setting ")+mapSettingName(k,"MODEL")+" in folder "+folderName()+" of the imported file",
                    "KinematicMapFolder::loadFrom()");
    }
  String binningText;
  if (readSetting(inputFile,"BINNING",binningText))
    {
    std::vector<double> b = KinematicMap::parseNumbers(binningText);
    if (b.size()!=9) throw Exception(String("Invalid BINNING: ")+binningText,"KinematicMapFolder::loadFrom()");
    for (int v=0; v<3; v++) setBinning(KinematicMap::Variable(v),int(b[3*v]),b[3*v+1],b[3*v+2]);
    }
  loadSettings(inputFile);
  }

  void KinematicMapFolder::loadLegacy(const Folder & inputFile)
  {
  String prefix = hasBase(_histogramBaseName) ? _histogramBaseName+"_" : String("");
  for (unsigned int k=0; k<_maps.size(); k++)
    {
    MapEntry & e = _maps[k];
    if (e.map.model()==KinematicMap::kHistogram)
      {
      String name = prefix+mapHistogramName(k);
      const MATH::Histogram * h = inputFile.get<MATH::Histogram>(name);
      if (!h) throw Exception(String("No folder ")+folderName()+" nor histogram "+name+" in the imported file",
                              "KinematicMapFolder::loadFrom()");
      std::vector<KinematicMap::Variable> v = e.map.variables();
      setMapHistogram(k,*h,v.size()==h->rank() ? v : std::vector<KinematicMap::Variable>{});
      }
    else if (e.map.model()==KinematicMap::kFunction)
      {
      const MATH::ParameterizedFunction * f = inputFile.get<MATH::ParameterizedFunction>(prefix+mapFunctionName(k));
      if (f) setMapFunction(k,std::shared_ptr<const MATH::Function>(f->clone()),e.map.variables());
      }
    }
  }

  void KinematicMapFolder::saveTo(Folder & file) const
  {
  String sub = folderName();
  if (file.contains(sub) && !file.folder(sub)) file.remove(sub);
  Folder & outputFile = *file.addFolder(sub);
  writeSetting(outputFile,"KIND",kindName());
  HistogramFolder::saveTo(outputFile);
  for (unsigned int k=0; k<_maps.size(); k++)
    {
    const MapEntry & e = _maps[k];
    KinematicMap::Model model = e.map.model();
    const auto * f = dynamic_cast<const MATH::ParameterizedFunction *>(e.map.function());
    if (model==KinematicMap::kFunction && !f) model = KinematicMap::kNone;   // a function that cannot be saved
    if (model==KinematicMap::kHistogram && !e.map.histogram()) model = KinematicMap::kNone;
    char value[64];
    std::snprintf(value,sizeof(value),"%.17g",model==KinematicMap::kConstant ? e.map.constant() : e.createValue);
    writeSetting(outputFile,mapSettingName(k,"MODEL"),KinematicMap::modelName(model));
    writeSetting(outputFile,mapSettingName(k,"VARIABLES"),KinematicMap::variablesText(e.map.variables()));
    writeSetting(outputFile,mapSettingName(k,"VALUE"),String(value));
    if (!f) continue;
    String name = mapFunctionName(k);
    if (outputFile.contains(name)) outputFile.remove(name);
    MATH::ParameterizedFunction * copy = f->clone();
    copy->setName(name);
    outputFile.add(copy,name);
    }
  String binning;
  for (int v=0; v<3; v++)
    {
    char text[96];
    std::snprintf(text,sizeof(text),"%s%d %.17g %.17g",v ? " " : "",_nBins[v],_minimum[v],_maximum[v]);
    binning += text;
    }
  writeSetting(outputFile,"BINNING",binning);
  saveSettings(outputFile);
  }

} // namespace CAP
