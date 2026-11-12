################################################################################################
# Project CAP/ParticleSingle
################################################################################################




################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(ParticleSingle SHARED
ParticleSingleTasks.cpp   ParticleSingleHistos.cpp  ParticleSingleAnalyzer.cpp )

message(STATUS "EXTRA_LIBS: ${EXTRA_LIBS}")
message(STATUS "EXTRA_INCLUDES: ${EXTRA_INCLUDES}")
target_link_libraries(ParticleSingle Base   Particles  ${EXTRA_LIBS} )
target_include_directories(ParticleSingle  PUBLIC Base    Particles ParticleSingle ${EXTRA_INCLUDES} )

# optimization for big histogram access within the pair inner loop
# WARNING: the sumw2 structure is not used -> estimate uncertainties using sub-samples
add_definitions(-DOPTIMIZEADDBINCONTENT)


cap_install(ParticleSingle LIBRARY "$ENV{CAP_LIB_PATH}")

