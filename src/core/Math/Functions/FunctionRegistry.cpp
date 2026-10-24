/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FunctionRegistry.cpp
//! @brief Implementation of FunctionRegistry.
#include <algorithm>
#include <cstdlib>
#include <cxxabi.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include "FunctionRegistry.hpp"
#include "MathExceptions.hpp"
#include "Constant.hpp"
#include "Linear.hpp"
#include "Parabolic.hpp"
#include "Cubic.hpp"
#include "Quartic.hpp"
#include "Polynomial.hpp"
#include "Normal.hpp"
#include "LogNormal.hpp"
#include "Exponential.hpp"
#include "OrthogonalSeries.hpp"
#include "BesselFunction.hpp"
#include "Normal2D.hpp"
#include "Polynomial2D.hpp"
#include "ParameterizedComposite.hpp"
#include "FileSystem.hpp"
#include "XmlDocument.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {

  std::string key(const String & name) { return std::string(name.data(), std::size_t(name.length())); }

  //! The registrations, with the functions of CAP registered at first use.
  struct Registry
  {
    std::mutex mutex;
    std::map<std::string,FunctionRegistry::Factory> factories;   // by name
    std::map<std::type_index,std::string> names;   // by class
    std::map<std::string,std::shared_ptr<const ParameterizedComposite>> composites;   // by name
    std::vector<std::string> compositeOrder;   // names of the composites, in the order of registration

    void add(const std::string & name, const std::type_info & type, FunctionRegistry::Factory factory)
    {
      for (auto it = names.begin(); it != names.end(); )   // a class has one name, a name one class
        it = (it->second == name && it->first != std::type_index(type)) ? names.erase(it) : std::next(it);
      auto previous = names.find(std::type_index(type));
      if (previous != names.end() && previous->second != name) factories.erase(previous->second);
      factories[name] = std::move(factory);
      names[std::type_index(type)] = name;
    }

    template <typename T>
    void add(const std::string & name) { add(name, typeid(T), [](unsigned int) -> ParameterizedFunction * { return new T(); }); }

    Registry()
    {
      add<Constant>("Constant");
      add<Linear>("Linear");
      add<Parabolic>("Parabolic");
      add<Cubic>("Cubic");
      add<Quartic>("Quartic");
      add("Polynomial", typeid(Polynomial), [](unsigned int n) -> ParameterizedFunction *
          { return new Polynomial(n == 0 ? 2 : n-1); });
      add<Normal>("Normal");
      add<LogNormal>("LogNormal");
      add<Exponential>("Exponential");
      addSeries<LegendreSeries>("LegendreSeries", OrthogonalSeries::kLegendre);
      addSeries<ChebyshevSeries>("ChebyshevSeries", OrthogonalSeries::kChebyshev);
      addSeries<HermiteSeries>("HermiteSeries", OrthogonalSeries::kHermite);
      addSeries<LaguerreSeries>("LaguerreSeries", OrthogonalSeries::kLaguerre);
      add<BesselJFunction>("BesselJ");
      add<BesselYFunction>("BesselY");
      add<BesselIFunction>("BesselI");
      add<BesselKFunction>("BesselK");
      add<Normal2D>("Normal2D");
      add("Polynomial2D", typeid(Polynomial2D), [](unsigned int n) -> ParameterizedFunction *
          {
          if (n == 0) return new Polynomial2D(2);
          for (unsigned int d=0; d<=9; d++) if (Polynomial2D::nCoefficients(d) + 4 == n) return new Polynomial2D(d);
          return nullptr;
          });
    }

    //! A series made with a number of parameters: N+1 coefficients and its mapping (0: degree 3).
    template <typename T>
    void addSeries(const std::string & name, OrthogonalSeries::Family family)
    {
      add(name, typeid(T), [family](unsigned int n) -> ParameterizedFunction *
          {
          unsigned int extra = OrthogonalSeries::nMappingParameters(family);
          if (n == 0) return new T(3);
          if (n < extra + 1) return nullptr;
          return new T(n - extra - 1);
          });
    }
  };

  Registry & registry()
  {
    static Registry instance;
    return instance;
  }

  } // namespace

  void FunctionRegistry::registerType(const String & name, const std::type_info & type, Factory factory)
  {
  if (name.length() == 0) throw MathException("A function class must be registered with a name", __FUNCTION__);
  if (!factory) throw MathException("A function class must be registered with a factory", __FUNCTION__);
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  r.add(key(name), type, std::move(factory));
  }

  bool FunctionRegistry::isRegistered(const String & name)
  {
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  return r.factories.count(key(name)) > 0;
  }

  bool FunctionRegistry::isRegistered(const ParameterizedFunction & function)
  {
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  return r.names.count(std::type_index(typeid(function))) > 0;
  }

  String FunctionRegistry::typeName(const ParameterizedFunction & function)
  {
  if (!function.registryName().isNull()) return function.registryName();   // e.g. a composite
  Registry & r = registry();
  {
  std::lock_guard<std::mutex> lock(r.mutex);
  auto found = r.names.find(std::type_index(typeid(function)));
  if (found != r.names.end()) return String(found->second.c_str());
  }
  int status = 0;
  char * demangled = abi::__cxa_demangle(typeid(function).name(), nullptr, nullptr, &status);
  std::string name = (status == 0 && demangled) ? demangled : typeid(function).name();
  std::free(demangled);
  return String(name.c_str());
  }

  ParameterizedFunction * FunctionRegistry::create(const String & name, unsigned int nParameters)
  {
  Factory factory;
  {
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  auto found = r.factories.find(key(name));
  if (found == r.factories.end()) return nullptr;
  factory = found->second;
  }
  std::unique_ptr<ParameterizedFunction> function;
  try { function.reset(factory(nParameters)); }
  catch (...) { return nullptr; }
  if (!function || function->nParameters() != nParameters) return nullptr;
  return function.release();
  }

  ParameterizedFunction * FunctionRegistry::create(const String & name)
  {
  Factory factory;
  {
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  auto found = r.factories.find(key(name));
  if (found == r.factories.end()) return nullptr;
  factory = found->second;
  }
  try { return factory(0); }
  catch (...) { return nullptr; }
  }

  std::vector<String> FunctionRegistry::names()
  {
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  std::vector<String> result;
  for (const auto & entry : r.factories) result.push_back(String(entry.first.c_str()));
  return result;
  }


  void FunctionRegistry::registerComposite(const String & name, const ParameterizedComposite & prototype)
  {
  if (name.length() == 0) throw MathException("A composite must be registered with a name", __FUNCTION__);
  if (prototype.nComponents() == 0) throw MathException("A composite to register needs components", __FUNCTION__);
  auto copy = std::make_shared<ParameterizedComposite>(prototype);
  copy->setRegistryName(name);
  std::shared_ptr<const ParameterizedComposite> stored = copy;
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  std::string k = key(name);
  if (r.factories.count(k) && !r.composites.count(k))
    throw MathException(String("The name ") + name + String(" is that of a class of functions"), __FUNCTION__);
  if (!r.composites.count(k)) r.compositeOrder.push_back(k);
  r.composites[k] = stored;
  r.factories[k] = [stored](unsigned int) -> ParameterizedFunction * { return stored->clone(); };
  }

  bool FunctionRegistry::isComposite(const String & name)
  {
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  return r.composites.count(key(name)) > 0;
  }

  std::shared_ptr<const ParameterizedComposite> FunctionRegistry::composite(const String & name)
  {
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  auto found = r.composites.find(key(name));
  return found == r.composites.end() ? nullptr : found->second;
  }

  std::vector<String> FunctionRegistry::compositeNames()
  {
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  std::vector<String> result;
  for (const auto & name : r.compositeOrder) result.push_back(String(name.c_str()));
  return result;
  }

  bool FunctionRegistry::removeComposite(const String & name)
  {
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  std::string k = key(name);
  if (!r.composites.erase(k)) return false;
  r.factories.erase(k);
  r.compositeOrder.erase(std::remove(r.compositeOrder.begin(), r.compositeOrder.end(), k), r.compositeOrder.end());
  return true;
  }

  bool FunctionRegistry::writeComposites(const String & fileName)
  {
  try
    {
    XmlDocument document("Functions");
    XmlElement & root = document.root();
    root.setAttribute("version", 1);
    for (const auto & name : compositeNames())
      {
      std::shared_ptr<const ParameterizedComposite> c = composite(name);
      if (!c) continue;
      XmlElement & element = root.addChild("Composite");
      element.setAttribute("name", name);
      if (c->operation() == ParameterizedComposite::kProduct) element.setAttribute("operation", "product");
      for (unsigned int k=0; k<c->nComponents(); k++)
        {
        const ParameterizedFunction & f = c->component(k);
        XmlElement & component = element.addChild("Component");
        component.setAttribute("label", c->label(k));
        component.setAttribute("type", typeName(f));
        component.setAttribute("parameters", int(f.nParameters()));
        for (unsigned int j=0; j<f.nParameters(); j++)
          {
          const Parameter & p = c->parameter(c->firstParameter(k) + j);
          XmlElement & parameter = component.addChild("Parameter");
          parameter.setAttribute("value", p.value());
          parameter.setAttribute("type", int(p.type()));
          parameter.setAttribute("minimum", p.minimum());
          parameter.setAttribute("maximum", p.maximum());
          }
        }
      }
    document.write(fileName);
    return true;
    }
  catch (const std::exception &)
    {
    return false;
    }
  }

  int FunctionRegistry::readComposites(const String & fileName, String * problems)
  {
  XmlDocument document;
  try { document = XmlDocument::read(fileName); }
  catch (const std::exception &) { return -1; }
  const XmlElement & root = document.root();
  if (!root.isNamed("Functions")) return -1;
  int count = 0;
  for (const XmlElement * element : root.children("Composite"))
    {
    String name = element->attribute("name", "");
    try
      {
      ParameterizedComposite model(element->attribute("operation", "sum") == "product" ? ParameterizedComposite::kProduct
                                                                                          : ParameterizedComposite::kSum);
      for (const XmlElement * component : element->children("Component"))
        {
        String type = component->attribute("type", "");
        unsigned int n = (unsigned int)(component->attributeInt("parameters", 0));
        std::unique_ptr<ParameterizedFunction> f(create(type, n));
        if (!f) throw MathException(String("the function ") + type + String(" is not registered"), __FUNCTION__);
        std::vector<const XmlElement *> parameters = component->children("Parameter");
        for (unsigned int j=0; j<parameters.size() && j<f->nParameters(); j++)
          {
          long t = parameters[j]->attributeInt("type", 0);
          if (t < 0 || t > long(Parameter::High)) t = 0;
          f->setParameter(j, Parameter(f->parameter(j).name(), parameters[j]->attributeDouble("value", 0.0), Parameter::ParameterType(t),
                                       parameters[j]->attributeDouble("minimum", Parameter::defaultMinimum),
                                       parameters[j]->attributeDouble("maximum", Parameter::defaultMaximum)));
          }
        model.add(component->attribute("label", ""), *f);
        }
      registerComposite(name, model);
      count++;
      }
    catch (const std::exception & e)
      {
      if (problems) *problems += name + String(": ") + String(e.what()) + String("\n");
      }
    }
  return count;
  }

  String FunctionRegistry::defaultCompositesFile()
  {
  return FileSystem::join(FileSystem::home(), ".CapFunctions.xml");
  }

  } // namespace MATH
} // namespace CAP
