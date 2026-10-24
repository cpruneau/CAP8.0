################################################################################################
# Project CAP/Jets
################################################################################################

###include(${ROOT_USE_FILE})
###include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
###add_definitions(${ROOT_CXX_FLAGS})


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Jets SHARED JetHistos.cpp JetSingleHistos.cpp JetPairHistos.cpp JetHelpers.cpp JetAnalyzer.cpp
JetHistosDerived.cpp JetSingleHistosDerived.cpp  JetPairHistosDerived.cpp)

target_link_libraries(Jets Base Particles FASTJET_LIB ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Jets PUBLIC Jets Particles Pythia Base Math ${CAP_FASTJET_INCLUDE_PATH}  ${EXTRA_INCLUDES} )

install(TARGETS Jets LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

