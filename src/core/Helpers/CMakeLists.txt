################################################################################################
# Project CAP/Helpers
################################################################################################

#include(${ROOT_USE_FILE})
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
#add_definitions(${ROOT_CXX_FLAGS})


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Helpers SHARED
CanvasConfiguration.cpp
CanvasList.cpp
GraphConfiguration.cpp
RootChainManager.cpp
RootFileHelpers.cpp
RootHistogramHelpers.cpp
RootConversion.cpp
RootFileConversion.cpp)

target_link_libraries(Helpers Base Math ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Helpers PUBLIC Base Math ${EXTRA_INCLUDES} )

# Tests (run with ctest in the build directory; not installed)
add_executable(TestRootFileHelpers tests/TestRootFileHelpers.cpp)
target_link_libraries(TestRootFileHelpers Helpers Base Math ${ROOT_LIBRARIES})
add_test(NAME RootFileHelpers COMMAND TestRootFileHelpers)
add_executable(TestRootConversion tests/TestRootConversion.cpp)
target_link_libraries(TestRootConversion Helpers Base Math ${ROOT_LIBRARIES})
add_test(NAME RootConversion COMMAND TestRootConversion)

# Conversion of the histograms of ROOT files to CAP files and back (option CAP_WITH_ROOT_FILES)
if(CAP_WITH_ROOT_FILES)
  add_executable(CapRootConvert CapRootConvert.cxx)
  target_link_libraries(CapRootConvert Helpers Base Math ${ROOT_LIBRARIES})
  set_target_properties(CapRootConvert PROPERTIES INSTALL_RPATH "$ENV{CAP_LIB_PATH}")   # runs without SetupCAP.sh
  install(TARGETS CapRootConvert RUNTIME DESTINATION "${CAP_INSTALL_BIN_DIR}")
endif()

install(TARGETS Helpers  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

