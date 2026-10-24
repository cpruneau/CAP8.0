################################################################################################
# Project CAP/Interpreter: the CAP command language and its interpreter, bin/CapShell
# (no ROOT code of its own: classes are reached through the reflection registry of Base)
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Interpreter SHARED
Builtins.cpp
Interpreter.cpp
Lexer.cpp
Parser.cpp)
target_link_libraries(Interpreter Math Base ${ROOT_LIBRARIES})
if(TARGET Graphics)   # save(): figures as SVG or PDF
  target_link_libraries(Interpreter Graphics)
  target_compile_definitions(Interpreter PRIVATE CAP_INTERPRETER_GRAPHICS)
endif()
install(TARGETS Interpreter LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

# bin/CapShell, with line editing and history when the system has libedit (macOS, Linux with libedit-dev)
add_executable(CapShell CapShell.cxx)
target_link_libraries(CapShell Interpreter Math Base ${ROOT_LIBRARIES})
find_path(EDITLINE_INCLUDE_DIR editline/readline.h)
find_library(EDITLINE_LIBRARY edit)
if(EDITLINE_INCLUDE_DIR AND EDITLINE_LIBRARY)
  target_include_directories(CapShell PRIVATE ${EDITLINE_INCLUDE_DIR})
  target_link_libraries(CapShell ${EDITLINE_LIBRARY})
  target_compile_definitions(CapShell PRIVATE CAP_HAVE_EDITLINE)
endif()
set_target_properties(CapShell PROPERTIES INSTALL_RPATH "$ENV{CAP_LIB_PATH}")   # runs without SetupCAP.sh
install(TARGETS CapShell RUNTIME DESTINATION "${CAP_INSTALL_BIN_DIR}")

# Tests (run with ctest in the build directory; not installed)
add_executable(TestInterpreter tests/TestInterpreter.cpp)
target_link_libraries(TestInterpreter Interpreter Math Base ${ROOT_LIBRARIES})
add_test(NAME Interpreter COMMAND TestInterpreter)
