################################################################################################
# Project CAP/ParticleFlow: anisotropic flow by multi-particle cumulants and the scalar product method
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(ParticleFlow SHARED
ParticleFlowTasks.cpp
ParticleFlowAnalyzer.cpp
ParticleFlowHistos.cpp
FlowCorrelator.cpp
FlowCumulants.cpp)

target_link_libraries(ParticleFlow Base Math Particles ${EXTRA_LIBS} )
target_include_directories(ParticleFlow PUBLIC Base Particles ParticleFlow ${EXTRA_INCLUDES} )

install(TARGETS ParticleFlow LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

################################################################################################
# Tests (run with ctest in the build directory; not installed)
################################################################################################
add_executable(TestParticleFlow tests/TestParticleFlow.cpp)
target_link_libraries(TestParticleFlow ParticleFlow Particles Base Math)
add_test(NAME ParticleFlow COMMAND TestParticleFlow)
