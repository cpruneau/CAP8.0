# Reflection in CAP without ROOT

October 2026. A study of how CAP can find classes by name, create objects, and reach their data members and member
functions at run time, as ROOT's dictionaries allow, without ROOT.

## What CAP uses ROOT's reflection for today

- **Dictionaries everywhere:** 230 classes carry `ClassDef`, and 26 libraries run `rootcling` through
  `ROOT_GENERATE_DICTIONARY` and a `LinkDef.h` file.
- **One essential use, the task factory:** `Task::instantiateTask()` creates the tasks of a workflow from their class
  names in the XML file (`TClass::GetClass(name)->New()`). `Task::taskClassNames()` lists the task classes by walking
  `TClassTable`.
- **Little else:** CAP files no longer need ROOT streaming, since `Folder` and `CapFile` have their own binary format.
  The remaining uses are drawing with ROOT and reading ROOT files (`Helpers`), and `InheritsFrom` checks.
- **ROOT in the basic headers, until October 2026:** `Aliases.hpp`, included by `Exceptions.hpp` and so by nearly
  everything, included ROOT's `Rtypes.h` for the `ClassDef` macros. `Aliases` is gone, and so are `ClassDef` and the
  dictionaries (see below).

So ROOT's reflection is load-bearing in one place (creating tasks by name), and a build-time and header-level cost
everywhere else.

## The options

| Option | What it is | Tools needed | Cost per class | Available |
|---|---|---|---|---|
| ROOT dictionaries (today) | `rootcling` parses headers, generates `TClass` data | ROOT, `rootcling`, LinkDef | `ClassDef` + LinkDef line | yes |
| **Own run-time registry** | classes described once in C++ (`ClassBuilder`), kept by name | none (C++17) | a short description in a `.cpp` file | **yes, prototyped** |
| Own generator (libclang) | a small `rootcling`: parse headers, emit the registry code | libclang at build time | none (generated) | weeks of work, a heavy build dependency |
| **C++26 static reflection** (P2996) | the compiler describes the class; one line fills the registry | GCC 16 (`-freflection`) | one line | **GCC 16 only; prototyped** |
| Header-only libraries | Boost.Describe, refl-cpp, RTTR | Boost / third-party | a macro per class | yes, but adds dependencies (RTTR is unmaintained) |

### C++26 static reflection on this machine

- **GCC 16.2 (Homebrew) has it.** `examples/AutoReflectionCxx26.cpp` fills CAP's registry from a class declaration with
  `autoRegister<Particle>("Particle")`, and the registry code compiles unchanged.
- **Apple clang 21 and LLVM clang 23 do not have it.**
- **GCC can't be mixed with Homebrew ROOT.** ROOT is built with clang and libc++, and code built with GCC uses
  libstdc++, so the two can't share objects in one process.
- **So the C++26 route opens only for code that no longer links ROOT,** or once clang catches up.

## Recommendation

**Use CAP's own run-time registry now** (`Reflection.hpp`, committed), and fill it by C++26 reflection later.

- **It has no dependencies:** plain C++17, no code generator, works with every compiler CAP builds with.
- **It covers what ROOT's dictionaries give CAP:**
  - classes found by name, and listed by base class (for the task factory);
  - objects made by any constructor;
  - member functions called by name, with overloads and inherited functions;
  - data members and get/set properties;
  - objects passed and returned by value, by reference or by pointer;
  - lists, as `std::vector`.
- **It's ready for C++26:** the descriptions written by hand today become one generated line per class, with the
  same registry and the same users.
- **It serves several users at once:**
  - the task factory;
  - a command line interpreter, which is item 5;
  - generic property editors in CapGui;
  - XML configuration by member name;
  - a bridge to Python if wanted.

### What the prototype shows (tests `Reflection` and `MathReflection`)

```cpp
const ClassInfo * h = Registry::instance().find("Histogram");
Value g    = Registry::instance().find("MersenneTwister")->create({ 12345 });
Value hist = h->create({ "h", "Gaussian numbers", 40, -4.0, 4.0 });
for (int k=0; k<10000; k++) hist.call("fill", { g.call("gauss", { 0.5, 1.0 }) });
hist.get("name");           // a property of CAP::Object, the base
hist.call("nBins", { 0 });  // a function of Partition, the other base
std::cout << h->describe(); // bases, constructors, fields, functions with their signatures
```

Registering Histogram takes 25 lines (`MathReflection.cpp`). Overloaded functions need a `static_cast` choosing the
signature, and functions with default arguments need one entry per arity (or a small lambda).

### Limits of this version

- **Calls use the static type:** functions are looked up in the class of the value and its bases. A `Base *` returned
  for a derived object shows the base's functions; finding the dynamic type is a later addition.
- **Only registered classes cross the boundary:** arguments and results of class type must be registered classes,
  `std::string`/`String`, numbers, enums, or vectors of these.
- **No streaming:** object I/O stays with `Folder`/`CapFile`, which already have their own format.

## Path away from ROOT's dictionaries

1. **Done: register the task classes.** All 34 are registered, one line each with `registerTask<T>()`, in a
   `<Library>Tasks.cpp` file per library. `Task::instantiateTask()` and `taskClassNames()` use the registry only.
2. **Register the classes an interpreter or the GUI needs:** histograms, functions, data points, fits, graphs,
   random numbers.
3. **Done (October 2026): no ROOT dictionaries.** `ClassDef`, `ClassImp`, the LinkDef files and
   `ROOT_GENERATE_DICTIONARY` are gone from every library, removed one library at a time with a build, the tests
   and a workflow run after each. Nothing in CAP wrote CAP objects into ROOT files, so no stored data depends on
   them. The ROOT macros using CAP classes interactively (`src/Macros`) need CAP's headers on their include path
   now that the libraries carry no dictionary. The optional modules Jets and Pythia were edited the same way but
   could not be built here (FastJet and Pythia8 are not installed). PHSD fails to build, as it did before, for
   reasons unrelated to the dictionaries. `core/Postscript`, not built, keeps `ClassDef` because its classes derive
   from ROOT's.
4. **Switch to C++26 reflection** once CAP builds without ROOT (or clang supports P2996): generate the descriptions
   instead of writing them.
