################################################################################################
# Project CAP/Jets
################################################################################################

###include(${ROOT_USE_FILE})
###include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
###add_definitions(${ROOT_CXX_FLAGS})

ROOT_GENERATE_DICTIONARY(G__Jets JetHistos.hpp JetSingleHistos.hpp JetPairHistos.hpp JetAnalyzer.hpp
JetHistosDerived.hpp JetSingleHistosDerived.hpp JetPairHistosDerived.hpp  LINKDEF JetLinkDef.h)

################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Jets SHARED JetHistos.cpp JetSingleHistos.cpp JetPairHistos.cpp JetHelpers.cpp JetAnalyzer.cpp
JetHistosDerived.cpp JetSingleHistosDerived.cpp  JetPairHistosDerived.cpp G__Jets.cxx)

target_link_libraries(Jets Base Particles FASTJET_LIB ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Jets PUBLIC Jets Particles CAPPythia Base Math ${CAP_FASTJET_INCLUDE_PATH} ${CAP_PYTHIA8_LIB_PATH}  ${EXTRA_INCLUDES} )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/libJets.rootmap" "${CMAKE_CURRENT_BINARY_DIR}/libJets_rdict.pcm" DESTINATION "$ENV{CAP_LIB_PATH}")
install(TARGETS Jets LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

