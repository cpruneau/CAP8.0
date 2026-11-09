################################################################################################
# Project CAP/Particles
################################################################################################

#include(${ROOT_USE_FILE})
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
#add_definitions(${ROOT_CXX_FLAGS})


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Particles SHARED
ParticlesTasks.cpp
BaseHistos.cpp
BaseDerivedHistos.cpp
Event.cpp
EventEfficiency.cpp
EventFilter.cpp
EventIterator.cpp
EventMerger.cpp
AnalysisTask.cpp
WorkflowHistograms.cpp
EventProcessor.cpp
EventSmearer.cpp
FileIterator.cpp
HistogramFolder.cpp
RootHistogramGroup.cpp
ManagedObject.cpp
ObjectCreator.cpp
Particle.cpp
ParticleDb.cpp
ParticleDbXml.cpp
ParticleDecayer.cpp
ParticleDbExceptions.cpp
ParticleDecayerTask.cpp
ParticleDecayMode.cpp
KinematicMap.cpp
KinematicMapFolder.cpp
ParticleEfficiency.cpp
ParticleFilter.cpp
ParticleSelection.cpp
ParticlePairFilter.cpp
ParticleSmear.cpp
ParticleType.cpp)

target_link_libraries(Particles Base Math Helpers   ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Particles  PUBLIC Base Math Helpers Particles  ${EXTRA_INCLUDES} )

install(TARGETS Particles  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")


################################################################################################
# Tests (run with ctest in the build directory; not installed)
################################################################################################
add_executable(TestParticleSelection tests/TestParticleSelection.cpp)
target_link_libraries(TestParticleSelection Particles Base Math ${ROOT_LIBRARIES})
add_test(NAME ParticleSelection COMMAND TestParticleSelection)
add_executable(TestParticleEfficiency tests/TestParticleEfficiency.cpp)
target_link_libraries(TestParticleEfficiency Particles Base Math ${ROOT_LIBRARIES})
add_test(NAME ParticleEfficiency COMMAND TestParticleEfficiency)
add_executable(TestParticleSmear tests/TestParticleSmear.cpp)
target_link_libraries(TestParticleSmear Particles Base Math ${ROOT_LIBRARIES})
add_test(NAME ParticleSmear COMMAND TestParticleSmear)
