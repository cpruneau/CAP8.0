################################################################################################
# Project CAP/NuDyn
################################################################################################

################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(NuDyn SHARED
NuDynTasks.cpp
NuDynAnalyzer.cpp
NuDynDerivedCalculator.cpp
NuDynHistos.cpp)

target_link_libraries(NuDyn Base  Particles  ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(NuDyn  PUBLIC Base  Particles  NuDyn ${EXTRA_INCLUDES} ) 

install(TARGETS NuDyn  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

