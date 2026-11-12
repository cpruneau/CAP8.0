################################################################################################
# Project CAP/Performance
################################################################################################


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Performance SHARED
PerformanceTasks.cpp
CalibrationProducer.cpp
ClosureCalculator.cpp
ClosureIterator.cpp
EventPlaneRandomizerTask.cpp
EventVertexRandomizerTask.cpp
MeasurementSimulator.cpp
ParticlePerformanceHistos.cpp
PerformanceAnalyzer.cpp)

target_link_libraries(Performance Base Math Particles  ${EXTRA_LIBS} )
target_include_directories(Performance  PUBLIC Performance Base  Particles   ${EXTRA_INCLUDES} )

# optimization for big histogram access within the pair inner loop
add_definitions(-DOPTIMIZEADDBINCONTENT)

cap_install(Performance LIBRARY "$ENV{CAP_LIB_PATH}")

################################################################################################
# Tests (run with ctest in the build directory; not installed)
################################################################################################
add_executable(TestPerformance tests/TestPerformance.cpp)
target_link_libraries(TestPerformance Performance Particles Base Math)
add_test(NAME Performance COMMAND TestPerformance)

