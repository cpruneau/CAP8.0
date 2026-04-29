################################################################################################
#
# CMakeLists.txt for CAP package. Version 6.1
#
# Creates  libraries with dictionary
#
################################################################################################

set(Include_JETS "NO")
set(Include_PYTHIA "NO")
set(Include_HERWIG "NO")
set(Include_HIJING "NO")
set(Include_AMPT "NO")
set(Include_EPOS "NO")
set(Include_PHSD "NO")
set(Include_THERMINATOR "YES")
set(Include_GLAUBER "YES")
set(Include_IDENTITY "NO")

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
set(CMAKE_CXX_STANDARD 14)
####set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
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
include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} "Base" "Math" "Helpers" "Xml" "Latex" "Particles" "ParticleSingle" "ParticlePair" "ParticlePair3D" "SubSample"  "CAPPythia"  "Global" "Jets" "Spherocity"   "NuDyn" "PtPt" "Therminator3" "PHSD" "Performance"  "BasicEventGen" "Glauber" "$ENV{ROOTSYS}/include"
 "$ENV{CAP_FASTJET_INCLUDE_PATH}" "$ENV{CAP_PYTHIA8_INCLUDE_PATH}" "$ENV{CAP_PYTHIA8_INCLUDE_PATH}/Pythia8/" )
add_definitions(${ROOT_CXX_FLAGS})
add_compile_options(-Wall -Wextra -pedantic)
add_library(EG SHARED IMPORTED)

add_subdirectory(Base)
add_subdirectory(BasicEventGen)
add_subdirectory(Math)
add_subdirectory(Helpers)
add_subdirectory(Glauber)
add_subdirectory(Global)
add_subdirectory(Particles)
add_subdirectory(ParticleSingle)
add_subdirectory(ParticlePair)
add_subdirectory(ParticlePair3D)
add_subdirectory(Performance)
add_subdirectory(PtPt)
add_subdirectory(NuDyn)
add_subdirectory(Spherocity)
add_subdirectory(SubSample)
add_subdirectory(Therminator3)
add_subdirectory(Xml)
add_subdirectory(Latex)

if (Include_PYTHIA OR Include_JETS)
  find_library(PYTHIA8_LIB pythia8 PATHS $ENV{CAP_PYTHIA8_LIB_PATH})
  add_library(PYTHIA8_LIB SHARED IMPORTED)
  set_target_properties(PYTHIA8_LIB PROPERTIES IMPORTED_LOCATION ${PYTHIA8_LIB})
  add_subdirectory(CAPPythia)
endif()

if (Include_HERWIG)
  add_subdirectory(Herwig)
endif()


if (Include_JETS)
  find_library(FASTJET_LIB fastjet PATHS $ENV{CAP_FASTJET_LIB_PATH} )
  add_library(FASTJET_LIB SHARED IMPORTED)
  set_target_properties(FASTJET_LIB PROPERTIES IMPORTED_LOCATION ${FASTJET_LIB})
  add_subdirectory(Jets)
endif()

if (Include_AMPT)
  add_subdirectory(Ampt)
endif()

if (Include_EPOS)
  add_subdirectory(Epos)
endif()

if (Include_PHSD)
  add_subdirectory(PHSD)
endif()


if (Include_HIJING)
  add_subdirectory(Hijing)
endif()

if (Include_IDENTITY)
  add_subdirectory(Identity)
endif()

add_executable(CAP CAP.cxx)
target_link_libraries(CAP PUBLIC  Base Math Glauber Particles  SubSample  Global Spherocity ParticleSingle ParticlePair ParticlePair3D NuDyn PtPt Performance Therminator3  BasicEventGen Xml Latex)

install(TARGETS CAP  LIBRARY DESTINATION "/Users/aa7526/Documents/GitHub/CAP8.0/lib")
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} "Base" "Math" "Helpers" "Xml" "Latex" "Glauber" "Particles"  "ParticleSingle" "ParticlePair"  "BasicEventGen" "$ENV{ROOTSYS}/include"
#"$ENV{CAP_FASTJET_INCLUDE_PATH}" "$ENV{CAP_PYTHIA8_INCLUDE_PATH}" "$ENV{CAP_PYTHIA8_INCLUDE_PATH}/Pythia8/" )
