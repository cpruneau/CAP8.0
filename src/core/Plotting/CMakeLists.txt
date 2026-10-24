################################################################################################
# Project CAP/Plotting
################################################################################################
#BalFctPlotter.hpp

################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Plotting SHARED CanvasList.cpp CanvasConfiguration.cpp  DataGraph.cpp   FunctionList.cpp  GraphConfiguration.cpp Plotter.cpp LegendConfiguration.cpp)
 
 # BalFctPlotter.cpp

target_link_libraries(Plotting Base  ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Plotting  PUBLIC Plotting Base  ${EXTRA_INCLUDES} )

# optimization for big histogram access within the pair inner loop
# WARNING: the sumw2 structure is not used -> estimate uncertainties using sub-samples
add_definitions(-DOPTIMIZEADDBINCONTENT)


install(TARGETS Plotting  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

