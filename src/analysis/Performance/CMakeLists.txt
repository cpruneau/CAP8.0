################################################################################################
# Project CAP/Performance
################################################################################################
#include(${ROOT_USE_FILE})
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
#add_definitions(${ROOT_CXX_FLAGS})


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Performance SHARED
PerformanceTasks.cpp
ClosureCalculator.cpp
ClosureIterator.cpp
EventPlaneRandomizerTask.cpp
EventVertexRandomizerTask.cpp
MeasurementSimulator.cpp
ParticlePerformanceHistos.cpp
PerformanceAnalyzer.cpp
RunClosureAnalysis.cpp)

target_link_libraries(Performance Base  Particles  ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Performance  PUBLIC Performance Base  Particles   ${EXTRA_INCLUDES} )

# optimization for big histogram access within the pair inner loop
add_definitions(-DOPTIMIZEADDBINCONTENT)

install(TARGETS Performance  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

