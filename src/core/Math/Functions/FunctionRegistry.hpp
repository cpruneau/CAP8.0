/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
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
#include <memory>
#include <typeinfo>
#include <vector>
#include "Aliases.hpp"

namespace CAP
{
  namespace MATH
  {

  class ParameterizedFunction;
  class ParameterizedComposite;

  //!
  //! @brief Registry of the classes of ParameterizedFunction, by name: a function stored in a file (the function of a
  //! FitRecord, see FitList::encodeBinary()) is written with the name of its class and recreated by it when read.
  //!
  //! The functions of CAP are registered under their class names, without namespaces: Constant, Linear, Parabolic,
  //! Cubic, Quartic, Polynomial, Normal, LogNormal, Exponential; the series of orthogonal polynomials LegendreSeries,
  //! ChebyshevSeries, HermiteSeries, LaguerreSeries (see OrthogonalSeries); and the Bessel functions BesselJ, BesselY,
  //! BesselI, BesselK (see BesselFunction); and, of two variables, Normal2D and Polynomial2D. A class of a user is
  //! registered once, before its fits are written or read:
  //! @code
  //!   CAP::MATH::FunctionRegistry::registerType<MyFunction>("MyFunction");
  //! @endcode
  //! The factory makes a function with a given number of parameters (whose values, bounds and names are then set from
  //! the file); the default one, for registerType<T>(name), uses the default constructor of T.
  //!
  //! Composites (ParameterizedComposite: sums of registered functions, e.g. a signal over a background, or products
  //! such as f(x) g(y)) are registered
  //! by name with registerComposite(); their definitions can be written to a file and read back (writeComposites(),
  //! readComposites()). CapGui reads defaultCompositesFile() when it starts and writes the composites made in its Fit
  //! window there, so the composites a program saves to that file are offered by CapGui, and the other way around.
  //! @code
  //!   CAP::MATH::ParameterizedComposite model;
  //!   model.add("signal", CAP::MATH::Normal(100.0, 0.0, 1.0));
  //!   model.add("background", CAP::MATH::Linear(10.0, 0.0));
  //!   CAP::MATH::FunctionRegistry::registerComposite("Gauss+Linear", model);
  //!   CAP::MATH::FunctionRegistry::writeComposites(CAP::MATH::FunctionRegistry::defaultCompositesFile());
  //! @endcode
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

  //! @name Composites
  //! @{

  //!
  //! @brief Register a composite under a name (replacing a composite of that name): create(name) then makes copies
  //! of it, with its parameters (values, types, bounds) as starting values. @param name name (not empty)
  //! @param prototype composite (copied; its registryName() becomes name)
  //! @throws MathException if the name is empty or that of a class, or the composite has no component
  //!
  static void registerComposite(const String & name, const ParameterizedComposite & prototype);
  //! @brief Whether a name is that of a composite. @param name name @return true if so
  static bool isComposite(const String & name);
  //! @brief The composite of a name. @param name name @return composite, or null if it is not one
  static std::shared_ptr<const ParameterizedComposite> composite(const String & name);
  //! @brief Names of the composites, in the order they were registered. @return names
  static std::vector<String> compositeNames();
  //! @brief Remove a composite. @param name name @return false if it is not a composite
  static bool removeComposite(const String & name);
  //!
  //! @brief Write the definitions of the composites to an XML file: for each, its name, and for each component its
  //! label, registered name, and parameters (values, types, bounds). @param fileName file
  //! @return false if it cannot be written
  //!
  static bool writeComposites(const String & fileName);
  //!
  //! @brief Register the composites defined in an XML file written by writeComposites(). A composite whose
  //! components are not all registered (e.g. a class of a user not registered yet) is skipped.
  //! @param fileName file @param problems if given, set to the composites skipped and why
  //! @return number of composites registered, or -1 if the file cannot be read
  //!
  static int readComposites(const String & fileName, String * problems=nullptr);
  //! @brief File of the composites shared by programs and CapGui: $HOME/.CapFunctions.xml. @return path
  static String defaultCompositesFile();
  //! @}
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__FunctionRegistry
