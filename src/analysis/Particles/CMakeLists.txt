################################################################################################
# Project CAP/Particles
################################################################################################



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

target_link_libraries(Particles Base Math   ${EXTRA_LIBS} )
target_include_directories(Particles  PUBLIC Base Math Particles  ${EXTRA_INCLUDES} )

cap_install(Particles LIBRARY "$ENV{CAP_LIB_PATH}")


################################################################################################
# Tests (run with ctest in the build directory; not installed)
################################################################################################
add_executable(TestParticleSelection tests/TestParticleSelection.cpp)
target_link_libraries(TestParticleSelection Particles Base Math)
add_test(NAME ParticleSelection COMMAND TestParticleSelection)
add_executable(TestParticleEfficiency tests/TestParticleEfficiency.cpp)
target_link_libraries(TestParticleEfficiency Particles Base Math)
add_test(NAME ParticleEfficiency COMMAND TestParticleEfficiency)
add_executable(TestParticleSmear tests/TestParticleSmear.cpp)
target_link_libraries(TestParticleSmear Particles Base Math)
add_test(NAME ParticleSmear COMMAND TestParticleSmear)
