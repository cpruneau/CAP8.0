################################################################################################
# Project Datum/NuDyn
################################################################################################

################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(NuDyn SHARED
NuDynTasks.cpp
NuDynAnalyzer.cpp
NuDynHistos.cpp)

target_link_libraries(NuDyn Base  Particles  ${EXTRA_LIBS} )
target_include_directories(NuDyn  PUBLIC Base  Particles  NuDyn ${EXTRA_INCLUDES} ) 

datum_install(NuDyn LIBRARY "$ENV{DATUM_LIB_PATH}")

