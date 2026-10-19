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
//! @file FunctionRegistry.hpp
//! @brief FunctionRegistry: the classes of ParameterizedFunction that can be recreated by name (e.g. to read fits).
#ifndef CAP__MATH__FunctionRegistry
#define CAP__MATH__FunctionRegistry
#include <functional>
#include <typeinfo>
#include <vector>
#include "Aliases.hpp"

namespace CAP
{
  namespace MATH
  {

  class ParameterizedFunction;

  //!
  //! @brief Registry of the classes of ParameterizedFunction, by name: a function stored in a file (the function of a
  //! FitRecord, see FitList::encodeBinary()) is written with the name of its class and recreated by it when read.
  //!
  //! The functions of CAP are registered under their class names, without namespaces: Constant, Linear, Parabolic,
  //! Cubic, Quartic, Polynomial, Normal, LogNormal and Exponential. A class of a user is registered once, before its
  //! fits are written or read:
  //! @code
  //!   CAP::MATH::FunctionRegistry::registerType<MyFunction>("MyFunction");
  //! @endcode
  //! The factory makes a function with a given number of parameters (whose values, bounds and names are then set from
  //! the file); the default one, for registerType<T>(name), uses the default constructor of T.
  //!
  class FunctionRegistry
  {
  public:

  //! @brief Factory: a new function (owned by the caller) with nParameters parameters (0: its default number), or null
  //! if it cannot have that many.
  using Factory = std::function<ParameterizedFunction*(unsigned int nParameters)>;

  //!
  //! @brief Register a class (replacing a registration of the same name or class).
  //! @param name name stored in files (not empty) @param type class of the functions made @param factory factory
  //! @throws MathException if the name is empty or the factory is null
  //!
  static void registerType(const String & name, const std::type_info & type, Factory factory);

  //! @brief Register class T, made with its default constructor. @param name name stored in files
  template <typename T>
  static void registerType(const String & name)
  {
    registerType(name, typeid(T), [](unsigned int) -> ParameterizedFunction * { return new T(); });
  }

  //! @brief Whether a name is registered. @param name name @return true if so
  static bool isRegistered(const String & name);
  //! @brief Whether the class of a function is registered. @param function function @return true if so
  static bool isRegistered(const ParameterizedFunction & function);

  //!
  //! @brief Name of the class of a function: its registered name, or else its C++ name with namespaces (which can be
  //! read back only once registered under it). @param function function @return name
  //!
  static String typeName(const ParameterizedFunction & function);

  //!
  //! @brief A new function of a registered class with nParameters parameters.
  //! @param name registered name @param nParameters number of parameters
  //! @return function (owned by the caller), or null if the name is not registered or the function would not have
  //! nParameters parameters
  //!
  static ParameterizedFunction * create(const String & name, unsigned int nParameters);
  //! @brief A new function of a registered class, with its default parameters (a Polynomial: of degree 2).
  //! @param name registered name @return function (owned by the caller), or null if the name is not registered
  static ParameterizedFunction * create(const String & name);

  //! @brief Registered names, in alphabetical order. @return names
  static std::vector<String> names();
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__FunctionRegistry
