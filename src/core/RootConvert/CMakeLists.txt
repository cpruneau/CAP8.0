################################################################################################
# Project CAP/RootConvert
################################################################################################

#include(${ROOT_USE_FILE})
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
#add_definitions(${ROOT_CXX_FLAGS})


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(RootConvert SHARED
RootChainManager.cpp
RootFileHelpers.cpp
RootConversion.cpp
RootFileConversion.cpp)

target_link_libraries(RootConvert Particles Base Math ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(RootConvert PUBLIC Base Math ${EXTRA_INCLUDES} )

# Tests (run with ctest in the build directory; not installed)
add_executable(TestRootFileHelpers tests/TestRootFileHelpers.cpp)
target_link_libraries(TestRootFileHelpers RootConvert Base Math ${ROOT_LIBRARIES})
add_test(NAME RootFileHelpers COMMAND TestRootFileHelpers)
add_executable(TestRootConversion tests/TestRootConversion.cpp)
target_link_libraries(TestRootConversion RootConvert Base Math ${ROOT_LIBRARIES})
add_test(NAME RootConversion COMMAND TestRootConversion)

# Conversion of the histograms of ROOT files to CAP files and back (the RootConvert library is built only with CAP_WITH_ROOT)
add_executable(CapRootConvert CapRootConvert.cxx)
target_link_libraries(CapRootConvert RootConvert Base Math ${ROOT_LIBRARIES})
set_target_properties(CapRootConvert PROPERTIES INSTALL_RPATH "$ENV{CAP_LIB_PATH}")   # runs without SetupCAP.sh
cap_install(CapRootConvert RUNTIME "${CAP_INSTALL_BIN_DIR}")

cap_install(RootConvert LIBRARY "$ENV{CAP_LIB_PATH}")

