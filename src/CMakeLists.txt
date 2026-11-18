################################################################################################
#
# CMakeLists.txt of Datum 1.0 (formerly CAP)
#
# Creates  libraries with dictionary
#
################################################################################################

################################################################################################
# Build options (set with -D<option>=ON|OFF, or with the BuildDatum.py GUI): DATUM_WITH_... and DATUM_..._DIR. The names
# of CAP (CAP_WITH_..., CAP_..._DIR) are still accepted when the new ones are not given.
# The core modules (Base, Math, Particles, Global, ParticleSingle, ParticlePair, ParticlePair3D, PtPt, NuDyn,
# ParticleFlow, Spherocity, SubSample, Performance, Spreadsheet, BasicEventGen) are always built.
################################################################################################
cmake_minimum_required(VERSION 3.14)   # 3.14: generator expressions in install(CODE)

foreach(module THERMINATOR3 LATEX GRAPHICS GUI ROOT JETS EPOS PHSD PYTHIA8 HERWIG GLAUBER SHERPA THERMALFIST)
  if(DEFINED CAP_WITH_${module} AND NOT DEFINED DATUM_WITH_${module})
    set(DATUM_WITH_${module} "${CAP_WITH_${module}}" CACHE BOOL "Set from CAP_WITH_${module}")
  endif()
endforeach()
foreach(prefix PYTHIA8_DIR HERWIG_DIR FASTJET_DIR SHERPA_DIR THERMALFIST_DIR)
  if(DEFINED CAP_${prefix} AND NOT DEFINED DATUM_${prefix})
    set(DATUM_${prefix} "${CAP_${prefix}}" CACHE PATH "Set from CAP_${prefix}")
  endif()
endforeach()

option(DATUM_WITH_THERMINATOR3 "Build the Therminator3 module (thermal model event generator)" ON)
option(DATUM_WITH_LATEX        "Build the Latex module (LaTeX documents of results)"           ON)
option(DATUM_WITH_GRAPHICS     "Build the Graphics module (2D drawing; macOS window and demo)"  ON)
option(DATUM_WITH_GUI          "Build bin/DatumGui, the graphical interface (Dear ImGui; requires Graphics)" ON)
option(DATUM_WITH_ROOT         "Use ROOT (off: Datum needs no ROOT): the RootConvert library (.root histogram files in DatumGui, bin/DatumRootConvert), the readers of ROOT event trees (Epos, Pythia)" OFF)
option(DATUM_WITH_JETS         "Build the Jets module (requires FastJet)"                       OFF)
option(DATUM_WITH_EPOS         "Build the Epos module (EPOS event reader; requires DATUM_WITH_ROOT)" OFF)
option(DATUM_WITH_PHSD         "Build the PHSD module (PHSD event reader)"                      ON)
option(DATUM_WITH_PYTHIA8      "Build the interface to the Pythia8 event generator (library Pythia)" OFF)
option(DATUM_WITH_HERWIG       "Build the interface to the Herwig event generator"              OFF)
option(DATUM_WITH_GLAUBER      "Build the Glauber module (Glauber Monte Carlo of nucleus-nucleus collisions)" ON)
option(DATUM_WITH_SHERPA       "Build the interface to the Sherpa event generator (placeholder)" OFF)
option(DATUM_WITH_THERMALFIST  "Build the interface to Thermal-FIST (placeholder)"              OFF)
# Installation prefixes of the external packages. When empty, a copy distributed with Datum is used if present
# (a directory <Datum>/pythia8*, <Datum>/external/pythia8*, ...), then the environment (DATUM_PYTHIA8_PATH, PYTHIA8, ...),
# then the package's configuration tool (pythia8-config --prefix, ...).
set(DATUM_PYTHIA8_DIR "" CACHE PATH "Pythia8 installation prefix")
set(DATUM_HERWIG_DIR  "" CACHE PATH "Herwig installation prefix")
set(DATUM_FASTJET_DIR "" CACHE PATH "FastJet installation prefix")
set(DATUM_SHERPA_DIR  "" CACHE PATH "Sherpa installation prefix")
set(DATUM_THERMALFIST_DIR "" CACHE PATH "Thermal-FIST directory (the source tree, holding include/HRGBase.h)")

if(APPLE)
    set (CMAKE_OSX_DEPLOYMENT_TARGET "26.0" CACHE STRING "Minimum OS X deployment version" FORCE)
    project(Datum VERSION 1.0 DESCRIPTION "Datum (formerly CAP, the Correlation Analysis Package)" LANGUAGES CXX)
#    set (CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Architectures" FORCE)
#    set (CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET[arch=arm64] "11.0" CACHE STRING "arm 64 minimum deployment target" FORCE)
else()
 project(Datum VERSION 1.0 DESCRIPTION "Datum (formerly CAP, the Correlation Analysis Package)" LANGUAGES CXX)
endif()

################################################################################################
# Project Datum (formerly CAP)
################################################################################################
# Optimized build unless another type is asked for (-DCMAKE_BUILD_TYPE=Debug, RelWithDebInfo, ...): without a type,
# CMake compiles without optimization, and Datum runs several times slower.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type: Release, Debug, RelWithDebInfo or MinSizeRel" FORCE)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Classes register themselves when their library is loaded (DATUM_REFLECTION: the tasks of Glauber, Therminator3, ...,
# the classes of DatumShell), and the programs reference no symbol of most of these libraries. On Linux, where the
# linker drops such libraries by default (--as-needed), bin/Datum then threw UnknownClassException: link them all.
if(UNIX AND NOT APPLE)
  add_link_options("LINKER:--no-as-needed")
endif()
enable_testing()   # ctest runs the test programs of the modules
# Tests of the model of RunDatum.py (filters, histogram folders), with the templates of DB/Templates
find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
  add_test(NAME RunDatumFilters
           COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${CMAKE_CURRENT_SOURCE_DIR}/../tests/python
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()
################################################################################################
# ROOT is optional (DATUM_WITH_ROOT, off by default). Without it, nothing in Datum includes a ROOT header or links a ROOT
# library. With it, ROOT is located through $ROOTSYS (or CMAKE_PREFIX_PATH), and ${ROOT_LIBRARIES} is linked only by
# the targets that need ROOT: RootConvert, DatumRootConvert, the Epos module and the Pythia event reader.
################################################################################################
if(DATUM_WITH_ROOT)
  list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
  find_package(ROOT REQUIRED COMPONENTS MathCore RIO Hist Tree)
  include(${ROOT_USE_FILE})
  add_compile_definitions(DATUM_WITH_ROOT)
  include_directories("core/RootConvert")
else()
  set(ROOT_LIBRARIES "")
endif()
include_directories(${CMAKE_SOURCE_DIR} "core/Base" "core/Math" "core/Math/Basic" "core/Math/LinearAlgebra" "core/Math/Functions" "core/Math/Interpolation" "core/Math/Integral" "core/Math/Derivative" "core/Math/Data" "core/Math/Search" "core/Math/Fitting" "core/Math/StatisticalTests" "core/Math/Random" "core/Interpreter" "core/Graphics" "core/Latex" "analysis/Particles" "analysis/ParticleSingle" "analysis/ParticlePair" "analysis/ParticlePair3D" "analysis/SubSample"  "models/Pythia"  "analysis/Global" "analysis/Jets" "analysis/Spherocity"   "analysis/NuDyn" "analysis/ParticleFlow" "analysis/PtPt" "models/Therminator3" "models/PHSD" "models/Epos" "analysis/Performance" "analysis/Spreadsheet" "models/BasicEventGen" "models/Glauber"
 )
add_compile_options(-Wall -Wextra -pedantic)

get_filename_component(DATUM_TOP_DIR "${CMAKE_SOURCE_DIR}" DIRECTORY)   # the CAP8.0 directory

################################################################################################
# Install directories: $DATUM_LIB_PATH and $DATUM_BIN_PATH (or CAP_..., their former names; see SetupDatum.sh), by default <Datum>/lib and
# <Datum>/bin
################################################################################################
foreach(variable LIB_PATH BIN_PATH)
  if("$ENV{DATUM_${variable}}" STREQUAL "" AND NOT "$ENV{CAP_${variable}}" STREQUAL "")
    set(ENV{DATUM_${variable}} "$ENV{CAP_${variable}}")
  endif()
endforeach()
if("$ENV{DATUM_LIB_PATH}" STREQUAL "")
  set(ENV{DATUM_LIB_PATH} "${DATUM_TOP_DIR}/lib/")   # read by the install() commands of the modules
  message(WARNING "DATUM_LIB_PATH is not set (source SetupDatum.sh): the libraries will be installed in $ENV{DATUM_LIB_PATH}")
endif()
if("$ENV{DATUM_BIN_PATH}" STREQUAL "")
  set(DATUM_INSTALL_BIN_DIR "${DATUM_TOP_DIR}/bin")
else()
  set(DATUM_INSTALL_BIN_DIR "$ENV{DATUM_BIN_PATH}")
endif()

################################################################################################
# datum_install(<target> LIBRARY|RUNTIME|BUNDLE <destination>): install(TARGETS), the installed file removed first.
# CMake does not copy a file whose time equals that of the installed one to the second, but still edits the rpaths
# of the installed copy: a target linked and installed within the same second then makes the next install fail
# ("install_name_tool: ... would duplicate path", "no LC_RPATH load command"). Removed first, it is always copied.
################################################################################################
################################################################################################
# datum_install_alias(<target> <old name> <destination>): the program installed under its former name too (a symbolic
# link to it), so that scripts and habits using the names of CAP keep working (CAP -> Datum, CapShell -> DatumShell...).
################################################################################################
function(datum_install_alias target old destination)
  install(CODE "
    file(REMOVE \"${destination}/${old}\")
    file(CREATE_LINK \"$<TARGET_FILE_NAME:${target}>\" \"${destination}/${old}\" SYMBOLIC)
    message(STATUS \"Installing: ${destination}/${old} -> $<TARGET_FILE_NAME:${target}>\")")
endfunction()

function(datum_install target kind destination)
  if(kind STREQUAL "BUNDLE")
    set(installed "${destination}/$<TARGET_FILE_NAME:${target}>.app/Contents/MacOS/$<TARGET_FILE_NAME:${target}>")
  else()
    set(installed "${destination}/$<TARGET_FILE_NAME:${target}>")
  endif()
  install(CODE "file(REMOVE \"${installed}\")")
  install(TARGETS ${target} ${kind} DESTINATION "${destination}")
endfunction()

################################################################################################
# datum_resolve_prefix(<var> <label> <file> <bundled glob...> ENV <env vars...> TOOL <config tool>)
# Set <var> (cache) to the installation prefix of an external package, if it is empty: a prefix holding <file> among
# the copies distributed with Datum (globs relative to the Datum directory), the environment variables, and the prefix
# reported by the configuration tool.
################################################################################################
function(datum_resolve_prefix VAR LABEL FILE)
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
    file(GLOB found LIST_DIRECTORIES true "${DATUM_TOP_DIR}/${pattern}")
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
add_subdirectory(analysis/Spreadsheet)   # tables of data: text, CSV, binary, Excel; statistics, correlations, plots
set(DATUM_LINKED_MODULES Base Math Particles SubSample Global Spherocity ParticleSingle ParticlePair
                       ParticlePair3D NuDyn ParticleFlow PtPt Performance Spreadsheet BasicEventGen)
set(DATUM_OPTIONAL_BUILT "")
if(DATUM_WITH_ROOT)   # ROOT files and trees: RootFileHelpers, RootConversion, RootFileConversion, RootChainManager
  add_subdirectory(core/RootConvert)
  list(APPEND DATUM_LINKED_MODULES RootConvert)
  list(APPEND DATUM_OPTIONAL_BUILT "RootConvert (ROOT ${ROOT_VERSION})")
endif()

################################################################################################
# Optional modules
################################################################################################
if(DATUM_WITH_GLAUBER)
  add_subdirectory(models/Glauber)
  list(APPEND DATUM_LINKED_MODULES Glauber)
  list(APPEND DATUM_OPTIONAL_BUILT Glauber)
endif()

if(DATUM_WITH_THERMINATOR3)
  add_subdirectory(models/Therminator3)
  list(APPEND DATUM_LINKED_MODULES Therminator3)
  list(APPEND DATUM_OPTIONAL_BUILT Therminator3)
endif()

if(DATUM_WITH_LATEX)
  add_subdirectory(core/Latex)
  list(APPEND DATUM_LINKED_MODULES Latex)
  list(APPEND DATUM_OPTIONAL_BUILT Latex)
endif()

if(DATUM_WITH_GRAPHICS)   # not linked into the Datum executable (nothing in Datum uses it yet)
  add_subdirectory(core/Graphics)
  list(APPEND DATUM_OPTIONAL_BUILT Graphics)
endif()

add_subdirectory(core/Interpreter)   # bin/DatumShell: the Datum command line interpreter (after Graphics, for figures)
list(APPEND DATUM_OPTIONAL_BUILT DatumShell)

if(DATUM_WITH_GUI)        # bin/DatumGui: Dear ImGui + GLFW (distributed in <Datum>/external), OpenGL
  if(DATUM_WITH_GRAPHICS)
    add_subdirectory(gui)
    list(APPEND DATUM_OPTIONAL_BUILT DatumGui)
  else()
    message(WARNING "DATUM_WITH_GUI requires DATUM_WITH_GRAPHICS: bin/DatumGui is not built.")
  endif()
endif()

if(DATUM_WITH_EPOS AND NOT DATUM_WITH_ROOT)
  message(WARNING "DATUM_WITH_EPOS requires DATUM_WITH_ROOT (the EPOS events are ROOT trees): the Epos module is not built.")
elseif(DATUM_WITH_EPOS)
  add_subdirectory(models/Epos)
  list(APPEND DATUM_LINKED_MODULES Epos)
  list(APPEND DATUM_OPTIONAL_BUILT Epos)
endif()

if(DATUM_WITH_PHSD)
  add_subdirectory(models/PHSD)
  list(APPEND DATUM_LINKED_MODULES PHSD)
  list(APPEND DATUM_OPTIONAL_BUILT PHSD)
endif()

if(DATUM_WITH_JETS)
  datum_resolve_prefix(DATUM_FASTJET_DIR "FastJet" "include/fastjet/ClusterSequence.hh"
                     BUNDLED "../Fastjet/install" "fastjet*" "external/fastjet*" ENV DATUM_FASTJET_PATH CAP_FASTJET_PATH FASTJET TOOL fastjet-config)
  find_library(DATUM_FASTJET_LIBRARY NAMES fastjet PATHS "${DATUM_FASTJET_DIR}/lib" NO_DEFAULT_PATH)
  if(NOT DATUM_FASTJET_LIBRARY)
    message(FATAL_ERROR "FastJet: no libfastjet in ${DATUM_FASTJET_DIR}/lib (is FastJet built and installed?)")
  endif()
  # the FastJet tools (fastjet::Pruner, for the jet grooming), built and installed with FastJet
  find_library(DATUM_FASTJET_TOOLS_LIBRARY NAMES fastjettools PATHS "${DATUM_FASTJET_DIR}/lib" NO_DEFAULT_PATH)
  if(NOT DATUM_FASTJET_TOOLS_LIBRARY)
    message(FATAL_ERROR "FastJet: no libfastjettools in ${DATUM_FASTJET_DIR}/lib (is FastJet built and installed?)")
  endif()
  set(DATUM_FASTJET_INCLUDE_PATH "${DATUM_FASTJET_DIR}/include")
  include_directories(SYSTEM "${DATUM_FASTJET_INCLUDE_PATH}")   # external headers: their warnings are not shown
  add_library(FASTJET_LIB SHARED IMPORTED)
  set_target_properties(FASTJET_LIB PROPERTIES IMPORTED_LOCATION "${DATUM_FASTJET_LIBRARY}"
                        INTERFACE_INCLUDE_DIRECTORIES "${DATUM_FASTJET_INCLUDE_PATH}")
  add_library(FASTJET_TOOLS_LIB SHARED IMPORTED)
  set_target_properties(FASTJET_TOOLS_LIB PROPERTIES IMPORTED_LOCATION "${DATUM_FASTJET_TOOLS_LIBRARY}"
                        INTERFACE_LINK_LIBRARIES FASTJET_LIB)
  add_subdirectory(analysis/Jets)
  list(APPEND DATUM_LINKED_MODULES Jets)
  list(APPEND DATUM_OPTIONAL_BUILT "Jets (FastJet: ${DATUM_FASTJET_DIR})")
endif()

################################################################################################
# Event generators
################################################################################################
if(DATUM_WITH_PYTHIA8)
  datum_resolve_prefix(DATUM_PYTHIA8_DIR "Pythia8" "include/Pythia8/Pythia.h"
                     BUNDLED "pythia8*" "external/pythia8*" ENV DATUM_PYTHIA8_PATH CAP_PYTHIA8_PATH PYTHIA8 TOOL pythia8-config)
  find_library(DATUM_PYTHIA8_LIBRARY NAMES pythia8 PATHS "${DATUM_PYTHIA8_DIR}/lib" NO_DEFAULT_PATH)
  if(NOT DATUM_PYTHIA8_LIBRARY)
    message(FATAL_ERROR "Pythia8: no libpythia8 in ${DATUM_PYTHIA8_DIR}/lib (is Pythia8 built?)")
  endif()
  set(DATUM_PYTHIA8_INCLUDE_PATH "${DATUM_PYTHIA8_DIR}/include")
  include_directories(SYSTEM "${DATUM_PYTHIA8_INCLUDE_PATH}" "${DATUM_PYTHIA8_INCLUDE_PATH}/Pythia8")   # external headers
  add_library(PYTHIA8_LIB SHARED IMPORTED)
  set_target_properties(PYTHIA8_LIB PROPERTIES IMPORTED_LOCATION "${DATUM_PYTHIA8_LIBRARY}"
                        INTERFACE_INCLUDE_DIRECTORIES "${DATUM_PYTHIA8_INCLUDE_PATH};${DATUM_PYTHIA8_INCLUDE_PATH}/Pythia8")
  add_subdirectory(models/Pythia)
  list(APPEND DATUM_LINKED_MODULES Pythia)
  list(APPEND DATUM_OPTIONAL_BUILT "Pythia (Pythia8: ${DATUM_PYTHIA8_DIR})")
endif()

if(DATUM_WITH_HERWIG)
  datum_resolve_prefix(DATUM_HERWIG_DIR "Herwig" "include/Herwig/API/HerwigAPI.h"
                     BUNDLED "../Herwig/install" "Herwig*" "herwig*" "external/Herwig*" "external/herwig*" ENV DATUM_HERWIG_PATH CAP_HERWIG_PATH HERWIG_DIR
                     TOOL Herwig-config)
  if(EXISTS "${CMAKE_SOURCE_DIR}/models/Herwig/CMakeLists.txt")
    include_directories(SYSTEM "${DATUM_HERWIG_DIR}/include")   # external headers: their warnings are not shown
    add_subdirectory(models/Herwig)
    list(APPEND DATUM_LINKED_MODULES Herwig)
    list(APPEND DATUM_OPTIONAL_BUILT "Herwig (Herwig: ${DATUM_HERWIG_DIR})")
  else()
    message(WARNING "Herwig was found in ${DATUM_HERWIG_DIR}, but Datum's interface to Herwig (src/models/Herwig) does not exist "
                    "yet: nothing is built for Herwig.")
  endif()
endif()

# Placeholders: the packages are located (so their location can be checked), but Datum has no interface to them yet.
if(DATUM_WITH_SHERPA)
  datum_resolve_prefix(DATUM_SHERPA_DIR "Sherpa" "include/SHERPA-MC/SHERPA/Main/Sherpa.H"
                     BUNDLED "../Sherpa/install" "Sherpa*" "sherpa*" "external/Sherpa*" "external/sherpa*"
                     ENV DATUM_SHERPA_PATH CAP_SHERPA_PATH SHERPA_DIR TOOL Sherpa-config)
  message(WARNING "Sherpa was found in ${DATUM_SHERPA_DIR}, but Datum has no interface to Sherpa yet: nothing is built for it.")
endif()

if(DATUM_WITH_THERMALFIST)
  datum_resolve_prefix(DATUM_THERMALFIST_DIR "Thermal-FIST" "include/HRGBase.h"
                     BUNDLED "../Thermal-FIST" "Thermal-FIST*" "external/Thermal-FIST*" ENV DATUM_THERMALFIST_PATH CAP_THERMALFIST_PATH THERMALFIST_DIR)
  message(WARNING "Thermal-FIST was found in ${DATUM_THERMALFIST_DIR}, but Datum has no interface to Thermal-FIST yet: "
                  "nothing is built for it.")
endif()

################################################################################################
# Datum executable
################################################################################################
add_executable(Datum Datum.cxx)
set_target_properties(Datum PROPERTIES OUTPUT_NAME Datum)          # bin/Datum (bin/CAP: a link to it)
target_link_libraries(Datum PUBLIC ${DATUM_LINKED_MODULES})
datum_install(Datum RUNTIME "${DATUM_INSTALL_BIN_DIR}")
datum_install_alias(Datum CAP "${DATUM_INSTALL_BIN_DIR}")

# DatumGui (File > New from Data), PlotEditor and PlotHistos (--template) configure the tasks of workflows (without
# running them): they need all the task classes
if(TARGET DatumGui)
  cmake_policy(SET CMP0079 NEW)   # the programs are made in gui/
  foreach(program DatumGui PlotEditor PlotHistos)
    target_link_libraries(${program} PRIVATE ${DATUM_LINKED_MODULES})
  endforeach()
endif()

# The task classes in the reflection registry (run with ctest; not installed)
add_executable(TestTaskRegistry tests/TestTaskRegistry.cpp)
target_link_libraries(TestTaskRegistry PUBLIC ${DATUM_LINKED_MODULES})
if(DATUM_WITH_GLAUBER)   # the test expects the Glauber tasks only when they are built
  target_compile_definitions(TestTaskRegistry PRIVATE DATUM_WITH_GLAUBER)
endif()
add_test(NAME TaskRegistry COMMAND TestTaskRegistry)

# Sources that must compile without ROOT (no ROOT include path): most of Base (strings, files, folders, Datum files,
# XML, configuration, reflection), the random numbers, the interpreter, Graphics (colors included). A ROOT header reaching one of them fails this test.
set(DATUM_ROOT_FREE_SOURCES
  core/Base/Accountant.cpp core/Base/BinaryFile.cpp core/Base/DatumFile.cpp core/Base/CmdlineParser.cpp
  core/Base/Condition.cpp core/Base/Configuration.cpp core/Base/ConfigurationManager.cpp core/Base/Crc32.cpp
  core/Base/Directory.cpp core/Base/Exceptions.cpp core/Base/Factory.cpp core/Base/File.cpp core/Base/FileSystem.cpp
  core/Base/FileSystemEntry.cpp core/Base/Filter.cpp core/Base/Folder.cpp core/Base/LongParameter.cpp core/Base/StringParameter.cpp
  core/Base/NameHelpers.cpp core/Base/Object.cpp core/Base/Parser.cpp core/Base/PhysicsConstants.cpp
  core/Base/PrintHelpers.cpp core/Base/Properties.cpp core/Base/PropertiesParser.cpp core/Base/Property.cpp
  core/Base/Reflection.cpp core/Base/String.cpp core/Base/TxtFile.cpp core/Base/XmlDocument.cpp
  core/Base/XmlElement.cpp core/Base/XmlFile.cpp core/Base/XmlReader.cpp core/Base/XmlWriter.cpp
  core/Math/Random/MersenneTwister.cpp core/Math/Random/RandomDistribution.cpp core/Math/Random/RandomGenerator.cpp
  core/Interpreter/Interpreter.cpp core/Interpreter/Lexer.cpp core/Interpreter/Parser.cpp core/Interpreter/DatumShell.cxx
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
set(DATUM_ROOT_FREE_COMMAND ${CMAKE_CXX_COMPILER} -std=c++17 -fsyntax-only)
if(CMAKE_OSX_SYSROOT)
  list(APPEND DATUM_ROOT_FREE_COMMAND -isysroot ${CMAKE_OSX_SYSROOT})
endif()
foreach(dir core/Base core/Math core/Math/Basic core/Math/LinearAlgebra core/Math/Functions core/Math/Interpolation
            core/Math/Integral core/Math/Derivative core/Math/Data core/Math/Search
            core/Math/Fitting core/Math/StatisticalTests core/Math/Random core/Interpreter core/Graphics)
  list(APPEND DATUM_ROOT_FREE_COMMAND -I${CMAKE_SOURCE_DIR}/${dir})
endforeach()
foreach(source ${DATUM_ROOT_FREE_SOURCES})
  list(APPEND DATUM_ROOT_FREE_COMMAND ${CMAKE_SOURCE_DIR}/${source})
endforeach()
add_test(NAME RootFreeSources COMMAND ${DATUM_ROOT_FREE_COMMAND})

string(REPLACE ";" ", " optional_text "${DATUM_OPTIONAL_BUILT}")
if(optional_text STREQUAL "")
  set(optional_text "none")
endif()
message(STATUS "Datum core modules: Base, Math, Particles, Global, ParticleSingle, ParticlePair, ParticlePair3D, "
               "PtPt, NuDyn, ParticleFlow, Spherocity, SubSample, Performance, Spreadsheet, BasicEventGen")
message(STATUS "Datum optional modules: ${optional_text}")
message(STATUS "Datum libraries install directory: $ENV{DATUM_LIB_PATH}")
