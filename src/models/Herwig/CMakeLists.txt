################################################################################################
# Project Datum/Herwig: the interface to the Herwig 7 event generator (HerwigEventGenerator)
#
# Herwig and ThePEG are found in DATUM_HERWIG_DIR (see src/CMakeLists.txt): libThePEG (lib/ThePEG) and libHerwigAPI
# (lib/Herwig); ThePEG loads the modules of Herwig (lib/Herwig/*.so) at run time, from the paths Herwig was built with.
################################################################################################
find_library(DATUM_THEPEG_LIBRARY NAMES ThePEG PATHS "${DATUM_HERWIG_DIR}/lib/ThePEG" NO_DEFAULT_PATH)
find_library(DATUM_HERWIG_API_LIBRARY NAMES HerwigAPI PATHS "${DATUM_HERWIG_DIR}/lib/Herwig" NO_DEFAULT_PATH)
if(NOT DATUM_THEPEG_LIBRARY OR NOT DATUM_HERWIG_API_LIBRARY)
  message(FATAL_ERROR "Herwig: libThePEG (lib/ThePEG) or libHerwigAPI (lib/Herwig) not found in ${DATUM_HERWIG_DIR} "
                      "(is Herwig built and installed?)")
endif()
message(STATUS "Herwig: ${DATUM_HERWIG_API_LIBRARY}, ${DATUM_THEPEG_LIBRARY}")

# the headers of ThePEG include those of GSL (and Boost): their directories, as ThePEG was configured with them
set(DATUM_HERWIG_INCLUDE_DIRS "${DATUM_HERWIG_DIR}/include")
if(EXISTS "${DATUM_HERWIG_DIR}/bin/thepeg-config")
  execute_process(COMMAND sh "${DATUM_HERWIG_DIR}/bin/thepeg-config" --cppflags OUTPUT_VARIABLE thepeg_cppflags
                  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
  separate_arguments(thepeg_cppflags UNIX_COMMAND "${thepeg_cppflags}")
  foreach(flag ${thepeg_cppflags})
    if(flag MATCHES "^-I(.+)$" AND EXISTS "${CMAKE_MATCH_1}")
      list(APPEND DATUM_HERWIG_INCLUDE_DIRS "${CMAKE_MATCH_1}")
    endif()
  endforeach()
endif()
find_path(DATUM_GSL_INCLUDE_DIR gsl/gsl_rng.h PATHS ${DATUM_HERWIG_INCLUDE_DIRS} "$ENV{GSL_DIR}/include"
          /usr/local/opt/gsl/include /opt/homebrew/opt/gsl/include /usr/local/include /usr/include)
if(NOT DATUM_GSL_INCLUDE_DIR)
  message(FATAL_ERROR "Herwig: the headers of GSL (gsl/gsl_rng.h), which ThePEG includes, are not found: set "
                      "DATUM_GSL_INCLUDE_DIR (e.g. $(brew --prefix gsl)/include)")
endif()
list(APPEND DATUM_HERWIG_INCLUDE_DIRS "${DATUM_GSL_INCLUDE_DIR}")
list(REMOVE_DUPLICATES DATUM_HERWIG_INCLUDE_DIRS)

add_library(Herwig SHARED
HerwigTasks.cpp
HerwigEventGenerator.cpp)
target_link_libraries(Herwig Base Particles "${DATUM_HERWIG_API_LIBRARY}" "${DATUM_THEPEG_LIBRARY}"
                      ${EXTRA_LIBS})
target_include_directories(Herwig PUBLIC Base Particles Herwig ${EXTRA_INCLUDES})
# ThePEG, Herwig, GSL and Boost headers: external, their warnings are not shown
target_include_directories(Herwig SYSTEM PUBLIC ${DATUM_HERWIG_INCLUDE_DIRS})
# ThePEG/Config/std.h has "using std::mem_fun;", which C++17 removed: libc++ (macOS) declares it again with this macro
# (libstdc++ still has it, deprecated). ThePEG and Herwig themselves are compiled as C++14.
target_compile_definitions(Herwig PRIVATE _LIBCPP_ENABLE_CXX17_REMOVED_BINDERS)

if(APPLE)
  # As Herwig and ThePEG themselves (~/Developer/Herwig/build_herwig.sh): Apple clang 21 (macOS 26) uses typed operator
  # new by default, which aborts when ThePEG allocates in static initializers; and the library depends on libc++
  # explicitly, so that libc++ is initialized before ThePEG and Herwig.
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-fno-typed-cxx-new-delete DATUM_HAVE_NO_TYPED_NEW)
  if(DATUM_HAVE_NO_TYPED_NEW)
    target_compile_options(Herwig PRIVATE -fno-typed-cxx-new-delete)
  endif()
  # -lc++ for libHerwig only (not passed on to the programs linking it); the compiler adds -lc++ as well, and Apple's
  # linker would warn about the duplicate
  target_link_options(Herwig PRIVATE -lc++)
  include(CheckCXXSourceCompiles)
  set(CMAKE_REQUIRED_LINK_OPTIONS "-Wl,-no_warn_duplicate_libraries")
  check_cxx_source_compiles("int main() { return 0; }" DATUM_HAVE_NO_WARN_DUPLICATE_LIBRARIES)
  unset(CMAKE_REQUIRED_LINK_OPTIONS)
  if(DATUM_HAVE_NO_WARN_DUPLICATE_LIBRARIES)
    target_link_options(Herwig PRIVATE -Wl,-no_warn_duplicate_libraries)
  endif()
  # the libraries of Herwig are found where they are installed, from the build and the installed Datum
  set_target_properties(Herwig PROPERTIES BUILD_RPATH "${DATUM_HERWIG_DIR}/lib/ThePEG;${DATUM_HERWIG_DIR}/lib/Herwig;${DATUM_HERWIG_DIR}/lib"
                                          INSTALL_RPATH "${DATUM_HERWIG_DIR}/lib/ThePEG;${DATUM_HERWIG_DIR}/lib/Herwig;${DATUM_HERWIG_DIR}/lib")
else()
  set_target_properties(Herwig PROPERTIES BUILD_RPATH "${DATUM_HERWIG_DIR}/lib/ThePEG;${DATUM_HERWIG_DIR}/lib/Herwig;${DATUM_HERWIG_DIR}/lib"
                                          INSTALL_RPATH "${DATUM_HERWIG_DIR}/lib/ThePEG;${DATUM_HERWIG_DIR}/lib/Herwig;${DATUM_HERWIG_DIR}/lib")
endif()

datum_install(Herwig LIBRARY "$ENV{DATUM_LIB_PATH}")
