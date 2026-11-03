################################################################################################
# Project CAP/SubSample
################################################################################################
#include(${ROOT_USE_FILE})
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
#add_definitions(${ROOT_CXX_FLAGS})



add_compile_options(-Wall -Wextra -pedantic)
add_library(SubSample SHARED
SubSampleTasks.cpp
SubSampleStatCalculator.cpp)

target_link_libraries(SubSample Base  Particles  ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(SubSample  PUBLIC Base  Particles SubSample ${EXTRA_INCLUDES} )

install(TARGETS SubSample  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")


################################################################################################
# Tests (run with ctest in the build directory; not installed)
################################################################################################
add_executable(TestSubSample tests/TestSubSample.cpp)
target_link_libraries(TestSubSample SubSample Particles Math Base ${ROOT_LIBRARIES})
add_test(NAME SubSample COMMAND TestSubSample)
