/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file AutoReflectionCxx26.cpp
//! @brief Example (not built): C++26 static reflection (P2996) filling the CAP reflection registry (Reflection.hpp)
//! from the declaration of a class, with no hand-written description.
//!
//! Needs GCC 16 (Apple clang and LLVM clang 23 do not have it yet) and code built with it, hence without the ROOT
//! libraries of Homebrew (built with clang and libc++). The files used do not include ROOT. From src/core/Base:
//!   g++-16 -std=c++26 -freflection -I. examples/AutoReflectionCxx26.cpp Reflection.cpp String.cpp \
//!          Exceptions.cpp PrintHelpers.cpp NameHelpers.cpp -o AutoReflection
#include <meta>
#include <cmath>
#include <cstdio>
#include <set>
#include "Reflection.hpp"

using namespace CAP;
using namespace CAP::REFLECTION;

//! Number of member functions of a class with the name of member function m.
consteval int overloads(std::meta::info type, std::meta::info m)
{
  int n = 0;
  for (auto f : std::meta::members_of(type, std::meta::access_context::unchecked()))
    if (std::meta::is_function(f) && std::meta::has_identifier(f) && std::meta::identifier_of(f) == std::meta::identifier_of(m)) n++;
  return n;
}

//! Describe T from its declaration: public data members, and public member functions that are not overloaded.
template <typename T>
void autoRegister(const std::string & name)
{
  ClassBuilder<T> builder(name);
  constexpr auto context = std::meta::access_context::current();
  template for (constexpr auto m : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, context)))
    builder.field(std::string(std::meta::identifier_of(m)), &[:m:]);
  // member functions: a name declared once (an overload set needs a choice of signature)
  template for (constexpr auto m : std::define_static_array(std::meta::members_of(^^T, context)))
    {
    if constexpr (std::meta::is_function(m) && !std::meta::is_special_member_function(m) && !std::meta::is_static_member(m)
                  && !std::meta::is_operator_function(m) && std::meta::has_identifier(m))
      {
      if constexpr (overloads(^^T, m) == 1) builder.method(std::string(std::meta::identifier_of(m)), &[:m:]);
      }
    }
}

struct Particle
{
  double px = 0, py = 0, pz = 0, mass = 0.13957;
  int charge = 1;
  double p() const { return std::sqrt(px*px + py*py + pz*pz); }
  double energy() const { return std::sqrt(p()*p() + mass*mass); }
  void boost(double factor) { px *= factor; py *= factor; pz *= factor; }
};

int main()
{
  autoRegister<Particle>("Particle");                // the whole description, from the declaration
  const ClassInfo * c = Registry::instance().find("Particle");
  std::printf("%s", c->describe().c_str());
  // objects of the class are made in C++ here (no constructor described): use one through the reflection
  Particle particle;
  Instance i; i.pointer = &particle; i.type = c;
  Value v(i);
  v.set("px", 3.0); v.set("py", 4.0);
  v.call("boost", { 2.0 });
  std::printf("p = %g, energy = %.6f, charge = %ld\n", v.call("p").toReal(), v.call("energy").toReal(), v.get("charge").toInteger());
  return 0;
}
