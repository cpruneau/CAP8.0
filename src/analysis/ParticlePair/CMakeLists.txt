################################################################################################
# Project CAP/ParticlePair
################################################################################################

#include(${ROOT_USE_FILE})
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
#add_definitions(${ROOT_CXX_FLAGS})


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(ParticlePair SHARED
ParticlePairTasks.cpp
ParticlePairAnalyzer.cpp
ParticlePairBfHistos.cpp
ParticlePairHistos.cpp
ParticlePairDerivedCalculator.cpp)

target_link_libraries(ParticlePair Base   Particles ParticleSingle ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(ParticlePair  PUBLIC Base Particles ParticleSingle ParticlePair ${EXTRA_INCLUDES} )

# optimization for big histogram access within the pair inner loop
# WARNING: the sumw2 structure is not used -> estimate uncertainties using sub-samples
add_definitions(-DOPTIMIZEADDBINCONTENT)


install(TARGETS ParticlePair  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

