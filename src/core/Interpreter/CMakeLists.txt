################################################################################################
# Project Datum/Interpreter: the Datum command language and its interpreter, bin/DatumShell
# (no ROOT code of its own: classes are reached through the reflection registry of Base)
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Interpreter SHARED
Builtins.cpp
Interpreter.cpp
Lexer.cpp
Parser.cpp)
target_link_libraries(Interpreter Math Base)
if(TARGET Graphics)   # save(): figures as SVG or PDF; the canvases of the session
  target_link_libraries(Interpreter Graphics)
  target_compile_definitions(Interpreter PRIVATE DATUM_INTERPRETER_GRAPHICS)
  if(TARGET Latex)    # presentation()
    target_link_libraries(Interpreter Latex)
    target_compile_definitions(Interpreter PRIVATE DATUM_INTERPRETER_LATEX)
  endif()
endif()
datum_install(Interpreter LIBRARY "$ENV{DATUM_LIB_PATH}")

# bin/DatumShell, with line editing and history when the system has libedit (macOS, Linux with libedit-dev)
add_executable(DatumShell DatumShell.cxx)
target_link_libraries(DatumShell Interpreter Math Base)
find_path(EDITLINE_INCLUDE_DIR editline/readline.h)
find_library(EDITLINE_LIBRARY edit)
if(EDITLINE_INCLUDE_DIR AND EDITLINE_LIBRARY)
  target_include_directories(DatumShell PRIVATE ${EDITLINE_INCLUDE_DIR})
  target_link_libraries(DatumShell ${EDITLINE_LIBRARY})
  target_compile_definitions(DatumShell PRIVATE DATUM_HAVE_EDITLINE)
endif()
set_target_properties(DatumShell PROPERTIES INSTALL_RPATH "$ENV{DATUM_LIB_PATH}"   # runs without SetupDatum.sh
                      OUTPUT_NAME DatumShell)                                   # bin/DatumShell (bin/CapShell: a link)
datum_install(DatumShell RUNTIME "${DATUM_INSTALL_BIN_DIR}")
datum_install_alias(DatumShell CapShell "${DATUM_INSTALL_BIN_DIR}")

# Tests (run with ctest in the build directory; not installed)
add_executable(TestInterpreter tests/TestInterpreter.cpp)
target_link_libraries(TestInterpreter Interpreter Math Base)
add_test(NAME Interpreter COMMAND TestInterpreter)
