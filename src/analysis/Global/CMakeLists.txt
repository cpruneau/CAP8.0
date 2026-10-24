################################################################################################
# Project CAP/Global
################################################################################################



add_compile_options(-Wall -Wextra -pedantic)
add_library(Global SHARED
GlobalTasks.cpp
GlobalAnalyzer.cpp
GlobalDerivedHistos.cpp
GlobalHistos.cpp)

target_link_libraries(Global Base  Particles  ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Global  PUBLIC Global Base  Particles  ${EXTRA_INCLUDES} )

# optimization for big histogram access within the pair inner loop
# WARNING: the sumw2 structure is not used -> estimate uncertainties using sub-samples
add_definitions(-DOPTIMIZEADDBINCONTENT)


install(TARGETS Global  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

