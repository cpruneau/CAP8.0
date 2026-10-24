################################################################################################
# Project CAP/PtPt
################################################################################################

################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(PtPt SHARED
PtPtTasks.cpp
PtPtAnalyzer.cpp
PtPtDerivedHistos.cpp
PtPtHistos.cpp)

target_link_libraries(PtPt Base  Particles  ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(PtPt  PUBLIC Base  Particles  PtPt ${EXTRA_INCLUDES} ) 

install(TARGETS PtPt  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

