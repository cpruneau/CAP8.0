################################################################################################
# Project Datum/RootConvert
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

# Conversion of the histograms of ROOT files to Datum files and back (the RootConvert library is built only with DATUM_WITH_ROOT)
add_executable(DatumRootConvert DatumRootConvert.cxx)
target_link_libraries(DatumRootConvert RootConvert Base Math ${ROOT_LIBRARIES})
set_target_properties(DatumRootConvert PROPERTIES INSTALL_RPATH "$ENV{DATUM_LIB_PATH}"   # runs without SetupDatum.sh
                      OUTPUT_NAME DatumRootConvert)                                   # bin/CapRootConvert: a link
datum_install(DatumRootConvert RUNTIME "${DATUM_INSTALL_BIN_DIR}")
datum_install_alias(DatumRootConvert CapRootConvert "${DATUM_INSTALL_BIN_DIR}")

datum_install(RootConvert LIBRARY "$ENV{DATUM_LIB_PATH}")

