################################################################################################
# Project Datum/Spherocity
################################################################################################



add_compile_options(-Wall -Wextra -pedantic)
add_library(Spherocity SHARED
SpherocityTasks.cpp
SpherocityHistos.cpp
SpherocityAnalyzer.cpp)

target_link_libraries(Spherocity Base  Particles  ${EXTRA_LIBS} )
target_include_directories(Spherocity  PUBLIC Spherocity Base  Particles  ${EXTRA_INCLUDES} )

# optimization for big histogram access within the pair inner loop
# WARNING: the sumw2 structure is not used -> estimate uncertainties using sub-samples
add_definitions(-DOPTIMIZEADDBINCONTENT)


datum_install(Spherocity LIBRARY "$ENV{DATUM_LIB_PATH}")

