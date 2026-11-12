################################################################################################
# Project CAP/SubSample
################################################################################################



add_compile_options(-Wall -Wextra -pedantic)
add_library(SubSample SHARED
SubSampleTasks.cpp
SubSampleStatCalculator.cpp)

target_link_libraries(SubSample Base  Particles  ${EXTRA_LIBS} )
target_include_directories(SubSample  PUBLIC Base  Particles SubSample ${EXTRA_INCLUDES} )

install(TARGETS SubSample  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")


################################################################################################
# Tests (run with ctest in the build directory; not installed)
################################################################################################
add_executable(TestSubSample tests/TestSubSample.cpp)
target_link_libraries(TestSubSample SubSample Particles Math Base)
add_test(NAME SubSample COMMAND TestSubSample)
