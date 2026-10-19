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
//! @file FunctionRegistry.cpp
//! @brief Implementation of FunctionRegistry.
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
          { return n == 0 ? nullptr : new Polynomial(n-1); });
      add<Normal>("Normal");
      add<LogNormal>("LogNormal");
      add<Exponential>("Exponential");
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

  std::vector<String> FunctionRegistry::names()
  {
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  std::vector<String> result;
  for (const auto & entry : r.factories) result.push_back(String(entry.first.c_str()));
  return result;
  }

  } // namespace MATH
} // namespace CAP
