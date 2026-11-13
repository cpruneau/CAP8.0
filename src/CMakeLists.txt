################################################################################################
#
# CMakeLists.txt for CAP package. Version 6.1
#
# Creates  libraries with dictionary
#
################################################################################################

################################################################################################
# Build options (set with -D<option>=ON|OFF, or with the BuildCAP.py GUI).
# The core modules (Base, Math, Particles, Global, ParticleSingle, ParticlePair, ParticlePair3D, PtPt, NuDyn,
# ParticleFlow, Spherocity, SubSample, Performance, BasicEventGen) are always built.
################################################################################################
cmake_minimum_required(VERSION 3.14)   # 3.14: generator expressions in install(CODE)

option(CAP_WITH_THERMINATOR3 "Build the Therminator3 module (thermal model event generator)" ON)
option(CAP_WITH_LATEX        "Build the Latex module (LaTeX documents of results)"           ON)
option(CAP_WITH_GRAPHICS     "Build the Graphics module (2D drawing; macOS window and demo)"  ON)
option(CAP_WITH_GUI          "Build bin/CapGui, the graphical interface (Dear ImGui; requires Graphics)" ON)
option(CAP_WITH_ROOT         "Use ROOT (off: CAP needs no ROOT): the RootConvert library (.root histogram files in CapGui, bin/CapRootConvert), the readers of ROOT event trees (Epos, Pythia)" OFF)
option(CAP_WITH_JETS         "Build the Jets module (requires FastJet)"                       OFF)
option(CAP_WITH_EPOS         "Build the Epos module (EPOS event reader; requires CAP_WITH_ROOT)" OFF)
option(CAP_WITH_PHSD         "Build the PHSD module (PHSD event reader)"                      ON)
option(CAP_WITH_PYTHIA8      "Build the interface to the Pythia8 event generator (library Pythia)" OFF)
option(CAP_WITH_HERWIG       "Build the interface to the Herwig event generator"              OFF)
option(CAP_WITH_GLAUBER      "Build the Glauber module (Glauber Monte Carlo of nucleus-nucleus collisions)" ON)
option(CAP_WITH_SHERPA       "Build the interface to the Sherpa event generator (placeholder)" OFF)
option(CAP_WITH_THERMALFIST  "Build the interface to Thermal-FIST (placeholder)"              OFF)
# Installation prefixes of the external packages. When empty, a copy distributed with CAP is used if present
# (a directory <CAP>/pythia8*, <CAP>/external/pythia8*, ...), then the environment (CAP_PYTHIA8_PATH, PYTHIA8, ...),
# then the package's configuration tool (pythia8-config --prefix, ...).
set(CAP_PYTHIA8_DIR "" CACHE PATH "Pythia8 installation prefix")
set(CAP_HERWIG_DIR  "" CACHE PATH "Herwig installation prefix")
set(CAP_FASTJET_DIR "" CACHE PATH "FastJet installation prefix")
set(CAP_SHERPA_DIR  "" CACHE PATH "Sherpa installation prefix")
set(CAP_THERMALFIST_DIR "" CACHE PATH "Thermal-FIST directory (the source tree, holding include/HRGBase.h)")

if(APPLE)
    set (CMAKE_OSX_DEPLOYMENT_TARGET "26.0" CACHE STRING "Minimum OS X deployment version" FORCE)
    project(CAP VERSION 1.0 DESCRIPTION "Correlation Analysis Package" LANGUAGES CXX)
#    set (CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Architectures" FORCE)
#    set (CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET[arch=arm64] "11.0" CACHE STRING "arm 64 minimum deployment target" FORCE)
else()
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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Classes register themselves when their library is loaded (CAP_REFLECTION: the tasks of Glauber, Therminator3, ...,
# the classes of CapShell), and the programs reference no symbol of most of these libraries. On Linux, where the
# linker drops such libraries by default (--as-needed), bin/CAP then threw UnknownClassException: link them all.
if(UNIX AND NOT APPLE)
  add_link_options("LINKER:--no-as-needed")
endif()
enable_testing()   # ctest runs the test programs of the modules
# Tests of the model of RunCAP.py (filters, histogram folders), with the templates of DB/Templates
find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
  add_test(NAME RunCAPFilters
           COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${CMAKE_CURRENT_SOURCE_DIR}/../tests/python
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()
################################################################################################
# ROOT is optional (CAP_WITH_ROOT, off by default). Without it, nothing in CAP includes a ROOT header or links a ROOT
# library. With it, ROOT is located through $ROOTSYS (or CMAKE_PREFIX_PATH), and ${ROOT_LIBRARIES} is linked only by
# the targets that need ROOT: RootConvert, CapRootConvert, the Epos module and the Pythia event reader.
################################################################################################
if(CAP_WITH_ROOT)
  list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
  find_package(ROOT REQUIRED COMPONENTS MathCore RIO Hist Tree)
  include(${ROOT_USE_FILE})
  add_compile_definitions(CAP_WITH_ROOT)
  include_directories("core/RootConvert")
else()
  set(ROOT_LIBRARIES "")
endif()
include_directories(${CMAKE_SOURCE_DIR} "core/Base" "core/Math" "core/Math/Basic" "core/Math/LinearAlgebra" "core/Math/Functions" "core/Math/Interpolation" "core/Math/Integral" "core/Math/Derivative" "core/Math/Data" "core/Math/Search" "core/Math/Fitting" "core/Math/StatisticalTests" "core/Math/Random" "core/Interpreter" "core/Graphics" "core/Latex" "analysis/Particles" "analysis/ParticleSingle" "analysis/ParticlePair" "analysis/ParticlePair3D" "analysis/SubSample"  "models/Pythia"  "analysis/Global" "analysis/Jets" "analysis/Spherocity"   "analysis/NuDyn" "analysis/ParticleFlow" "analysis/PtPt" "models/Therminator3" "models/PHSD" "models/Epos" "analysis/Performance"  "models/BasicEventGen" "models/Glauber"
 )
add_compile_options(-Wall -Wextra -pedantic)

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
# cap_install(<target> LIBRARY|RUNTIME|BUNDLE <destination>): install(TARGETS), the installed file removed first.
# CMake does not copy a file whose time equals that of the installed one to the second, but still edits the rpaths
# of the installed copy: a target linked and installed within the same second then makes the next install fail
# ("install_name_tool: ... would duplicate path", "no LC_RPATH load command"). Removed first, it is always copied.
################################################################################################
function(cap_install target kind destination)
  if(kind STREQUAL "BUNDLE")
    set(installed "${destination}/$<TARGET_FILE_NAME:${target}>.app/Contents/MacOS/$<TARGET_FILE_NAME:${target}>")
  else()
    set(installed "${destination}/$<TARGET_FILE_NAME:${target}>")
  endif()
  install(CODE "file(REMOVE \"${installed}\")")
  install(TARGETS ${target} ${kind} DESTINATION "${destination}")
endfunction()

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
add_subdirectory(analysis/Global)
add_subdirectory(analysis/Particles)
add_subdirectory(analysis/ParticleSingle)
add_subdirectory(analysis/ParticlePair)
add_subdirectory(analysis/ParticlePair3D)
add_subdirectory(analysis/Performance)
add_subdirectory(analysis/PtPt)
add_subdirectory(analysis/NuDyn)
add_subdirectory(analysis/ParticleFlow)
add_subdirectory(analysis/Spherocity)
add_subdirectory(analysis/SubSample)
set(CAP_LINKED_MODULES Base Math Particles SubSample Global Spherocity ParticleSingle ParticlePair
                       ParticlePair3D NuDyn ParticleFlow PtPt Performance BasicEventGen)
set(CAP_OPTIONAL_BUILT "")
if(CAP_WITH_ROOT)   # ROOT files and trees: RootFileHelpers, RootConversion, RootFileConversion, RootChainManager
  add_subdirectory(core/RootConvert)
  list(APPEND CAP_LINKED_MODULES RootConvert)
  list(APPEND CAP_OPTIONAL_BUILT "RootConvert (ROOT ${ROOT_VERSION})")
endif()

################################################################################################
# Optional modules
################################################################################################
if(CAP_WITH_GLAUBER)
  add_subdirectory(models/Glauber)
  list(APPEND CAP_LINKED_MODULES Glauber)
  list(APPEND CAP_OPTIONAL_BUILT Glauber)
endif()

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

if(CAP_WITH_EPOS AND NOT CAP_WITH_ROOT)
  message(WARNING "CAP_WITH_EPOS requires CAP_WITH_ROOT (the EPOS events are ROOT trees): the Epos module is not built.")
elseif(CAP_WITH_EPOS)
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
                     BUNDLED "../Fastjet/install" "fastjet*" "external/fastjet*" ENV CAP_FASTJET_PATH FASTJET TOOL fastjet-config)
  find_library(CAP_FASTJET_LIBRARY NAMES fastjet PATHS "${CAP_FASTJET_DIR}/lib" NO_DEFAULT_PATH)
  if(NOT CAP_FASTJET_LIBRARY)
    message(FATAL_ERROR "FastJet: no libfastjet in ${CAP_FASTJET_DIR}/lib (is FastJet built and installed?)")
  endif()
  set(CAP_FASTJET_INCLUDE_PATH "${CAP_FASTJET_DIR}/include")
  include_directories(SYSTEM "${CAP_FASTJET_INCLUDE_PATH}")   # external headers: their warnings are not shown
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
  include_directories(SYSTEM "${CAP_PYTHIA8_INCLUDE_PATH}" "${CAP_PYTHIA8_INCLUDE_PATH}/Pythia8")   # external headers
  add_library(PYTHIA8_LIB SHARED IMPORTED)
  set_target_properties(PYTHIA8_LIB PROPERTIES IMPORTED_LOCATION "${CAP_PYTHIA8_LIBRARY}"
                        INTERFACE_INCLUDE_DIRECTORIES "${CAP_PYTHIA8_INCLUDE_PATH};${CAP_PYTHIA8_INCLUDE_PATH}/Pythia8")
  add_subdirectory(models/Pythia)
  list(APPEND CAP_LINKED_MODULES Pythia)
  list(APPEND CAP_OPTIONAL_BUILT "Pythia (Pythia8: ${CAP_PYTHIA8_DIR})")
endif()

if(CAP_WITH_HERWIG)
  cap_resolve_prefix(CAP_HERWIG_DIR "Herwig" "include/Herwig/API/HerwigAPI.h"
                     BUNDLED "../Herwig/install" "Herwig*" "herwig*" "external/Herwig*" "external/herwig*" ENV CAP_HERWIG_PATH HERWIG_DIR
                     TOOL Herwig-config)
  if(EXISTS "${CMAKE_SOURCE_DIR}/models/Herwig/CMakeLists.txt")
    include_directories(SYSTEM "${CAP_HERWIG_DIR}/include")   # external headers: their warnings are not shown
    add_subdirectory(models/Herwig)
    list(APPEND CAP_LINKED_MODULES Herwig)
    list(APPEND CAP_OPTIONAL_BUILT "Herwig (Herwig: ${CAP_HERWIG_DIR})")
  else()
    message(WARNING "Herwig was found in ${CAP_HERWIG_DIR}, but CAP's interface to Herwig (src/models/Herwig) does not exist "
                    "yet: nothing is built for Herwig.")
  endif()
endif()

# Placeholders: the packages are located (so their location can be checked), but CAP has no interface to them yet.
if(CAP_WITH_SHERPA)
  cap_resolve_prefix(CAP_SHERPA_DIR "Sherpa" "include/SHERPA-MC/SHERPA/Main/Sherpa.H"
                     BUNDLED "../Sherpa/install" "Sherpa*" "sherpa*" "external/Sherpa*" "external/sherpa*"
                     ENV CAP_SHERPA_PATH SHERPA_DIR TOOL Sherpa-config)
  message(WARNING "Sherpa was found in ${CAP_SHERPA_DIR}, but CAP has no interface to Sherpa yet: nothing is built for it.")
endif()

if(CAP_WITH_THERMALFIST)
  cap_resolve_prefix(CAP_THERMALFIST_DIR "Thermal-FIST" "include/HRGBase.h"
                     BUNDLED "../Thermal-FIST" "Thermal-FIST*" "external/Thermal-FIST*" ENV CAP_THERMALFIST_PATH THERMALFIST_DIR)
  message(WARNING "Thermal-FIST was found in ${CAP_THERMALFIST_DIR}, but CAP has no interface to Thermal-FIST yet: "
                  "nothing is built for it.")
endif()

################################################################################################
# CAP executable
################################################################################################
add_executable(CAP CAP.cxx)
target_link_libraries(CAP PUBLIC ${CAP_LINKED_MODULES})
cap_install(CAP RUNTIME "${CAP_INSTALL_BIN_DIR}")

# CapGui (File > New from Data), PlotEditor and PlotHistos (--template) configure the tasks of workflows (without
# running them): they need all the task classes
if(TARGET CapGui)
  cmake_policy(SET CMP0079 NEW)   # the programs are made in gui/
  foreach(program CapGui PlotEditor PlotHistos)
    target_link_libraries(${program} PRIVATE ${CAP_LINKED_MODULES})
  endforeach()
endif()

# The task classes in the reflection registry (run with ctest; not installed)
add_executable(TestTaskRegistry tests/TestTaskRegistry.cpp)
target_link_libraries(TestTaskRegistry PUBLIC ${CAP_LINKED_MODULES})
if(CAP_WITH_GLAUBER)   # the test expects the Glauber tasks only when they are built
  target_compile_definitions(TestTaskRegistry PRIVATE CAP_WITH_GLAUBER)
endif()
add_test(NAME TaskRegistry COMMAND TestTaskRegistry)

# Sources that must compile without ROOT (no ROOT include path): most of Base (strings, files, folders, CAP files,
# XML, configuration, reflection), the random numbers, the interpreter, Graphics (colors included). A ROOT header reaching one of them fails this test.
set(CAP_ROOT_FREE_SOURCES
  core/Base/Accountant.cpp core/Base/BinaryFile.cpp core/Base/CapFile.cpp core/Base/CmdlineParser.cpp
  core/Base/Condition.cpp core/Base/Configuration.cpp core/Base/ConfigurationManager.cpp core/Base/Crc32.cpp
  core/Base/Directory.cpp core/Base/Exceptions.cpp core/Base/Factory.cpp core/Base/File.cpp core/Base/FileSystem.cpp
  core/Base/FileSystemEntry.cpp core/Base/Filter.cpp core/Base/Folder.cpp core/Base/LongParameter.cpp core/Base/StringParameter.cpp
  core/Base/NameHelpers.cpp core/Base/Object.cpp core/Base/Parser.cpp core/Base/PhysicsConstants.cpp
  core/Base/PrintHelpers.cpp core/Base/Properties.cpp core/Base/PropertiesParser.cpp core/Base/Property.cpp
  core/Base/Reflection.cpp core/Base/String.cpp core/Base/TxtFile.cpp core/Base/XmlDocument.cpp
  core/Base/XmlElement.cpp core/Base/XmlFile.cpp core/Base/XmlReader.cpp core/Base/XmlWriter.cpp
  core/Math/Random/MersenneTwister.cpp core/Math/Random/RandomDistribution.cpp core/Math/Random/RandomGenerator.cpp
  core/Interpreter/Interpreter.cpp core/Interpreter/Lexer.cpp core/Interpreter/Parser.cpp core/Interpreter/CapShell.cxx
  core/Graphics/Attributes.cpp core/Graphics/Axis.cpp core/Graphics/AxisRange.cpp core/Graphics/AxisRenderer.cpp
  core/Graphics/Canvas.cpp core/Graphics/CanvasRegistry.cpp core/Graphics/ColorTable.cpp core/Graphics/Drawable2D.cpp core/Graphics/FillAttributes.cpp
  core/Graphics/Geom2D.cpp core/Graphics/Graph.cpp core/Graphics/GraphRenderer.cpp core/Graphics/GraphRenderer3D.cpp
  core/Graphics/Grid2D.cpp core/Graphics/LatexRenderer.cpp core/Graphics/Line.cpp core/Graphics/LineAttributes.cpp
  core/Graphics/LineRenderer.cpp core/Graphics/Margins.cpp core/Graphics/Marker.cpp
  core/Graphics/MarkerAttributes.cpp core/Graphics/MarkerRenderer.cpp core/Graphics/MarkerVector.cpp
  core/Graphics/Painter.cpp core/Graphics/Palette.cpp core/Graphics/Palettes.cpp core/Graphics/PdfPainter.cpp
  core/Graphics/Polygon.cpp core/Graphics/PolygonRenderer.cpp core/Graphics/Projection3D.cpp
  core/Graphics/Renderer.cpp core/Graphics/Shape.cpp core/Graphics/SvgPainter.cpp core/Graphics/TextAttributes.cpp
  core/Graphics/TextRenderer.cpp)
set(CAP_ROOT_FREE_COMMAND ${CMAKE_CXX_COMPILER} -std=c++17 -fsyntax-only)
if(CMAKE_OSX_SYSROOT)
  list(APPEND CAP_ROOT_FREE_COMMAND -isysroot ${CMAKE_OSX_SYSROOT})
endif()
foreach(dir core/Base core/Math core/Math/Basic core/Math/LinearAlgebra core/Math/Functions core/Math/Interpolation
            core/Math/Integral core/Math/Derivative core/Math/Data core/Math/Search
            core/Math/Fitting core/Math/StatisticalTests core/Math/Random core/Interpreter core/Graphics)
  list(APPEND CAP_ROOT_FREE_COMMAND -I${CMAKE_SOURCE_DIR}/${dir})
endforeach()
foreach(source ${CAP_ROOT_FREE_SOURCES})
  list(APPEND CAP_ROOT_FREE_COMMAND ${CMAKE_SOURCE_DIR}/${source})
endforeach()
add_test(NAME RootFreeSources COMMAND ${CAP_ROOT_FREE_COMMAND})

string(REPLACE ";" ", " optional_text "${CAP_OPTIONAL_BUILT}")
if(optional_text STREQUAL "")
  set(optional_text "none")
endif()
message(STATUS "CAP core modules: Base, Math, Particles, Global, ParticleSingle, ParticlePair, ParticlePair3D, "
               "PtPt, NuDyn, ParticleFlow, Spherocity, SubSample, Performance, BasicEventGen")
message(STATUS "CAP optional modules: ${optional_text}")
message(STATUS "CAP libraries install directory: $ENV{CAP_LIB_PATH}")
