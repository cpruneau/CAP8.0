################################################################################################
# Project CAP/ParticleFlow
################################################################################################

#include(${ROOT_USE_FILE})
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
#add_definitions(${ROOT_CXX_FLAGS})



################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(ParticleFlow SHARED   ParticleFlowHistos.cpp ParticleFlowDerivedHistos.cpp  ParticleFlowAnalyzer.cpp)

target_link_libraries(ParticleFlow Base  Particles  ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(ParticleFlow  PUBLIC Base   Particles ParticleFlow ${EXTRA_INCLUDES} )

# optimization for big histogram access within the pair inner loop
# WARNING: the sumw2 structure is not used -> estimate uncertainties using sub-samples
add_definitions(-DOPTIMIZEADDBINCONTENT)


install(TARGETS ParticleFlow  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

