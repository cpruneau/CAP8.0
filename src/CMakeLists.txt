################################################################################################
#
# CMakeLists.txt for CAP package. Version 6.1
#
# Creates  libraries with dictionary
#
################################################################################################

################################################################################################
# Build options (set with -D<option>=ON|OFF, or with the BuildCAP.py GUI).
# The core modules (Base, Math, Helpers, Particles, Global, ParticleSingle, ParticlePair, ParticlePair3D, PtPt, NuDyn,
# Spherocity, SubSample, Performance, BasicEventGen, Glauber) are always built.
################################################################################################
option(CAP_WITH_THERMINATOR3 "Build the Therminator3 module (thermal model event generator)" ON)
option(CAP_WITH_LATEX        "Build the Latex module (LaTeX documents of results)"           ON)
option(CAP_WITH_GRAPHICS     "Build the Graphics module (2D drawing; macOS window and demo)"  ON)
option(CAP_WITH_GUI          "Build bin/CapGui, the graphical interface (Dear ImGui; requires Graphics)" ON)
option(CAP_WITH_JETS         "Build the Jets module (requires FastJet)"                       OFF)
option(CAP_WITH_EPOS         "Build the Epos module (EPOS event reader)"                      OFF)
option(CAP_WITH_PHSD         "Build the PHSD module (PHSD event reader)"                      OFF)
option(CAP_WITH_PYTHIA8      "Build the interface to the Pythia8 event generator (library Pythia)" OFF)
option(CAP_WITH_HERWIG       "Build the interface to the Herwig event generator"              OFF)
# Installation prefixes of the external packages. When empty, a copy distributed with CAP is used if present
# (a directory <CAP>/pythia8*, <CAP>/external/pythia8*, ...), then the environment (CAP_PYTHIA8_PATH, PYTHIA8, ...),
# then the package's configuration tool (pythia8-config --prefix, ...).
set(CAP_PYTHIA8_DIR "" CACHE PATH "Pythia8 installation prefix")
set(CAP_HERWIG_DIR  "" CACHE PATH "Herwig installation prefix")
set(CAP_FASTJET_DIR "" CACHE PATH "FastJet installation prefix")

if(APPLE)
    cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
    set (CMAKE_OSX_DEPLOYMENT_TARGET "26.0" CACHE STRING "Minimum OS X deployment version" FORCE)
    project(CAP VERSION 1.0 DESCRIPTION "Correlation Analysis Package" LANGUAGES CXX)
#    set (CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Architectures" FORCE)
#    set (CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET[arch=arm64] "11.0" CACHE STRING "arm 64 minimum deployment target" FORCE)
else()
 cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
 cmake_policy(SET CMP0048 NEW)
 project(CAP DESCRIPTION "Correlation Analysis Package" LANGUAGES CXX)
endif()

################################################################################################
# Project CAP
################################################################################################
# Optimized build unless another type is asked for (-DCMAKE_BUILD_TYPE=Debug, RelWithDebInfo, ...): without a type,
# CMake compiles without optimization, and CAP runs several times slower.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type: Release, Debug, RelWithDebInfo or MinSizeRel" FORCE)
endif()
set(CMAKE_CXX_STANDARD 14)
####set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
enable_testing()   # ctest runs the test programs of the modules
# Tests of the model of RunCAP.py (filters, histogram folders), with the templates of DB/Templates
find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
  add_test(NAME RunCAPFilters
           COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${CMAKE_CURRENT_SOURCE_DIR}/../tests/python
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()
set(CAP_ROOT_PATH "$ENV{CAP_ROOT_PATH}")

################################################################################################
# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
################################################################################################
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

################################################################################################
#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
################################################################################################
find_package(ROOT REQUIRED COMPONENTS EG MathCore MathMore RIO Hist Tree Net )

################################################################################################
# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
################################################################################################
include(${ROOT_USE_FILE})
include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} "core/Base" "core/Math" "core/Math/Basic" "core/Math/LinearAlgebra" "core/Math/Functions" "core/Math/Data" "core/Math/Search" "core/Math/Fitting" "core/Math/StatisticalTests" "core/Math/Random" "core/Interpreter" "core/Helpers" "core/Graphics" "core/Latex" "analysis/Particles" "analysis/ParticleSingle" "analysis/ParticlePair" "analysis/ParticlePair3D" "analysis/SubSample"  "models/Pythia"  "analysis/Global" "analysis/Jets" "analysis/Spherocity"   "analysis/NuDyn" "analysis/PtPt" "models/Therminator3" "models/PHSD" "models/Epos" "analysis/Performance"  "models/BasicEventGen" "models/Glauber" "$ENV{ROOTSYS}/include"
 )
add_definitions(${ROOT_CXX_FLAGS})
add_compile_options(-Wall -Wextra -pedantic)
add_library(EG SHARED IMPORTED)

get_filename_component(CAP_TOP_DIR "${CMAKE_SOURCE_DIR}" DIRECTORY)   # the CAP8.0 directory

################################################################################################
# Install directories: $CAP_LIB_PATH and $CAP_BIN_PATH (see SetupCAP.sh), by default <CAP>/lib and <CAP>/bin
################################################################################################
if("$ENV{CAP_LIB_PATH}" STREQUAL "")
  set(ENV{CAP_LIB_PATH} "${CAP_TOP_DIR}/lib/")   # read by the install() commands of the modules
  message(WARNING "CAP_LIB_PATH is not set (source SetupCAP.sh): the libraries will be installed in $ENV{CAP_LIB_PATH}")
endif()
if("$ENV{CAP_BIN_PATH}" STREQUAL "")
  set(CAP_INSTALL_BIN_DIR "${CAP_TOP_DIR}/bin")
else()
  set(CAP_INSTALL_BIN_DIR "$ENV{CAP_BIN_PATH}")
endif()

################################################################################################
# cap_resolve_prefix(<var> <label> <file> <bundled glob...> ENV <env vars...> TOOL <config tool>)
# Set <var> (cache) to the installation prefix of an external package, if it is empty: a prefix holding <file> among
# the copies distributed with CAP (globs relative to the CAP directory), the environment variables, and the prefix
# reported by the configuration tool.
################################################################################################
function(cap_resolve_prefix VAR LABEL FILE)
  cmake_parse_arguments(ARG "" "TOOL" "BUNDLED;ENV" ${ARGN})
  if(NOT "${${VAR}}" STREQUAL "")
    if(NOT EXISTS "${${VAR}}/${FILE}")
      message(FATAL_ERROR "${LABEL}: ${VAR}=${${VAR}} does not contain ${FILE}")
    endif()
    message(STATUS "${LABEL}: using ${VAR}=${${VAR}}")
    return()
  endif()
  set(candidates "")
  foreach(pattern ${ARG_BUNDLED})
    file(GLOB found LIST_DIRECTORIES true "${CAP_TOP_DIR}/${pattern}")
    list(APPEND candidates ${found})
  endforeach()
  foreach(env ${ARG_ENV})
    if(NOT "$ENV{${env}}" STREQUAL "")
      list(APPEND candidates "$ENV{${env}}")
    endif()
  endforeach()
  if(ARG_TOOL)
    find_program(tool_path ${ARG_TOOL})
    if(tool_path)
      execute_process(COMMAND ${tool_path} --prefix OUTPUT_VARIABLE prefix OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
      if(prefix)
        list(APPEND candidates "${prefix}")
      endif()
    endif()
    unset(tool_path CACHE)
  endif()
  foreach(candidate ${candidates})
    string(REGEX REPLACE "/+$" "" candidate "${candidate}")
    if(EXISTS "${candidate}/${FILE}")
      set(${VAR} "${candidate}" CACHE PATH "${LABEL} installation prefix" FORCE)
      message(STATUS "${LABEL}: found ${candidate}")
      return()
    endif()
  endforeach()
  message(FATAL_ERROR "${LABEL} was requested but not found (looked for ${FILE} in: ${candidates}). "
                      "Set ${VAR} to its installation prefix, or turn the option off.")
endfunction()

################################################################################################
# Core modules (always built)
################################################################################################
add_subdirectory(core/Base)
add_subdirectory(models/BasicEventGen)
add_subdirectory(core/Math)
add_subdirectory(core/Helpers)
add_subdirectory(models/Glauber)
add_subdirectory(analysis/Global)
add_subdirectory(analysis/Particles)
add_subdirectory(analysis/ParticleSingle)
add_subdirectory(analysis/ParticlePair)
add_subdirectory(analysis/ParticlePair3D)
add_subdirectory(analysis/Performance)
add_subdirectory(analysis/PtPt)
add_subdirectory(analysis/NuDyn)
add_subdirectory(analysis/Spherocity)
add_subdirectory(analysis/SubSample)
set(CAP_LINKED_MODULES Base Math Helpers Glauber Particles SubSample Global Spherocity ParticleSingle ParticlePair
                       ParticlePair3D NuDyn PtPt Performance BasicEventGen)
set(CAP_OPTIONAL_BUILT "")

################################################################################################
# Optional modules
################################################################################################
if(CAP_WITH_THERMINATOR3)
  add_subdirectory(models/Therminator3)
  list(APPEND CAP_LINKED_MODULES Therminator3)
  list(APPEND CAP_OPTIONAL_BUILT Therminator3)
endif()

if(CAP_WITH_LATEX)
  add_subdirectory(core/Latex)
  list(APPEND CAP_LINKED_MODULES Latex)
  list(APPEND CAP_OPTIONAL_BUILT Latex)
endif()

if(CAP_WITH_GRAPHICS)   # not linked into the CAP executable (nothing in CAP uses it yet)
  add_subdirectory(core/Graphics)
  list(APPEND CAP_OPTIONAL_BUILT Graphics)
endif()

add_subdirectory(core/Interpreter)   # bin/CapShell: the CAP command line interpreter (after Graphics, for figures)
list(APPEND CAP_OPTIONAL_BUILT CapShell)

if(CAP_WITH_GUI)        # bin/CapGui: Dear ImGui + GLFW (distributed in <CAP>/external), OpenGL
  if(CAP_WITH_GRAPHICS)
    add_subdirectory(gui)
    list(APPEND CAP_OPTIONAL_BUILT CapGui)
  else()
    message(WARNING "CAP_WITH_GUI requires CAP_WITH_GRAPHICS: bin/CapGui is not built.")
  endif()
endif()

if(CAP_WITH_EPOS)
  add_subdirectory(models/Epos)
  list(APPEND CAP_LINKED_MODULES Epos)
  list(APPEND CAP_OPTIONAL_BUILT Epos)
endif()

if(CAP_WITH_PHSD)
  add_subdirectory(models/PHSD)
  list(APPEND CAP_LINKED_MODULES PHSD)
  list(APPEND CAP_OPTIONAL_BUILT PHSD)
endif()

if(CAP_WITH_JETS)
  cap_resolve_prefix(CAP_FASTJET_DIR "FastJet" "include/fastjet/ClusterSequence.hh"
                     BUNDLED "fastjet*" "external/fastjet*" ENV CAP_FASTJET_PATH FASTJET TOOL fastjet-config)
  find_library(CAP_FASTJET_LIBRARY NAMES fastjet PATHS "${CAP_FASTJET_DIR}/lib" NO_DEFAULT_PATH)
  if(NOT CAP_FASTJET_LIBRARY)
    message(FATAL_ERROR "FastJet: no libfastjet in ${CAP_FASTJET_DIR}/lib (is FastJet built and installed?)")
  endif()
  set(CAP_FASTJET_INCLUDE_PATH "${CAP_FASTJET_DIR}/include")
  include_directories("${CAP_FASTJET_INCLUDE_PATH}")   # also needed by rootcling for the dictionary
  add_library(FASTJET_LIB SHARED IMPORTED)
  set_target_properties(FASTJET_LIB PROPERTIES IMPORTED_LOCATION "${CAP_FASTJET_LIBRARY}"
                        INTERFACE_INCLUDE_DIRECTORIES "${CAP_FASTJET_INCLUDE_PATH}")
  add_subdirectory(analysis/Jets)
  list(APPEND CAP_LINKED_MODULES Jets)
  list(APPEND CAP_OPTIONAL_BUILT "Jets (FastJet: ${CAP_FASTJET_DIR})")
endif()

################################################################################################
# Event generators
################################################################################################
if(CAP_WITH_PYTHIA8)
  cap_resolve_prefix(CAP_PYTHIA8_DIR "Pythia8" "include/Pythia8/Pythia.h"
                     BUNDLED "pythia8*" "external/pythia8*" ENV CAP_PYTHIA8_PATH PYTHIA8 TOOL pythia8-config)
  find_library(CAP_PYTHIA8_LIBRARY NAMES pythia8 PATHS "${CAP_PYTHIA8_DIR}/lib" NO_DEFAULT_PATH)
  if(NOT CAP_PYTHIA8_LIBRARY)
    message(FATAL_ERROR "Pythia8: no libpythia8 in ${CAP_PYTHIA8_DIR}/lib (is Pythia8 built?)")
  endif()
  set(CAP_PYTHIA8_INCLUDE_PATH "${CAP_PYTHIA8_DIR}/include")
  include_directories("${CAP_PYTHIA8_INCLUDE_PATH}" "${CAP_PYTHIA8_INCLUDE_PATH}/Pythia8")   # also for rootcling
  add_library(PYTHIA8_LIB SHARED IMPORTED)
  set_target_properties(PYTHIA8_LIB PROPERTIES IMPORTED_LOCATION "${CAP_PYTHIA8_LIBRARY}"
                        INTERFACE_INCLUDE_DIRECTORIES "${CAP_PYTHIA8_INCLUDE_PATH};${CAP_PYTHIA8_INCLUDE_PATH}/Pythia8")
  add_subdirectory(models/Pythia)
  list(APPEND CAP_LINKED_MODULES Pythia)
  list(APPEND CAP_OPTIONAL_BUILT "Pythia (Pythia8: ${CAP_PYTHIA8_DIR})")
endif()

if(CAP_WITH_HERWIG)
  cap_resolve_prefix(CAP_HERWIG_DIR "Herwig" "include/Herwig/API/HerwigAPI.h"
                     BUNDLED "Herwig*" "herwig*" "external/Herwig*" "external/herwig*" ENV CAP_HERWIG_PATH HERWIG_DIR
                     TOOL Herwig-config)
  if(EXISTS "${CMAKE_SOURCE_DIR}/models/Herwig/CMakeLists.txt")
    include_directories("${CAP_HERWIG_DIR}/include")
    add_subdirectory(models/Herwig)
    list(APPEND CAP_LINKED_MODULES Herwig)
    list(APPEND CAP_OPTIONAL_BUILT "Herwig (Herwig: ${CAP_HERWIG_DIR})")
  else()
    message(WARNING "Herwig was found in ${CAP_HERWIG_DIR}, but CAP's interface to Herwig (src/models/Herwig) does not exist "
                    "yet: nothing is built for Herwig.")
  endif()
endif()

################################################################################################
# CAP executable
################################################################################################
add_executable(CAP CAP.cxx)
target_link_libraries(CAP PUBLIC ${CAP_LINKED_MODULES})
install(TARGETS CAP RUNTIME DESTINATION "${CAP_INSTALL_BIN_DIR}")

string(REPLACE ";" ", " optional_text "${CAP_OPTIONAL_BUILT}")
if(optional_text STREQUAL "")
  set(optional_text "none")
endif()
message(STATUS "CAP core modules: Base, Math, Helpers, Particles, Global, ParticleSingle, ParticlePair, ParticlePair3D, "
               "PtPt, NuDyn, Spherocity, SubSample, Performance, BasicEventGen, Glauber")
message(STATUS "CAP optional modules: ${optional_text}")
message(STATUS "CAP libraries install directory: $ENV{CAP_LIB_PATH}")
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} "Base" "Math" "Helpers" "Latex" "Glauber" "Particles"  "ParticleSingle" "ParticlePair"  "BasicEventGen" "$ENV{ROOTSYS}/include"
#"$ENV{CAP_FASTJET_INCLUDE_PATH}" "$ENV{CAP_PYTHIA8_INCLUDE_PATH}" "$ENV{CAP_PYTHIA8_INCLUDE_PATH}/Pythia8/" )
