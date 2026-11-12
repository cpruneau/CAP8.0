################################################################################################
# Project CAP/ParticlePair3D
################################################################################################


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(ParticlePair3D SHARED
ParticlePair3DTasks.cpp
ParticlePair3DAnalyzer.cpp
ParticlePair3DBFHistos.cpp
ParticlePair3DHistos.cpp)

target_link_libraries(ParticlePair3D Base   Particles ParticleSingle ParticlePair ${EXTRA_LIBS} )
target_include_directories(ParticlePair3D  PUBLIC Base Particles ParticleSingle ParticlePair ParticlePair3D ${EXTRA_INCLUDES} )

# optimization for big histogram access within the pair inner loop
# WARNING: the sumw2 structure is not used -> estimate uncertainties using sub-samples
add_definitions(-DOPTIMIZEADDBINCONTENT)


cap_install(ParticlePair3D LIBRARY "$ENV{CAP_LIB_PATH}")

