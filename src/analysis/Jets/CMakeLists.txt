################################################################################################
# Project CAP/Jets: jets found with FastJet (optional module, CAP_WITH_JETS)
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Jets SHARED
JetTasks.cpp
Jet.cpp
JetClustering.cpp
JetFilter.cpp
JetHistos.cpp
JetAnalyzer.cpp
JetPair.cpp
JetPairHistos.cpp
JetPairAnalyzer.cpp
JetUnderlyingEvent.cpp
JetFluctuationHistos.cpp
JetFluctuationAnalyzer.cpp)

target_link_libraries(Jets Base Math Particles FASTJET_LIB ${EXTRA_LIBS} )
target_include_directories(Jets PUBLIC Jets Particles Base Math ${EXTRA_INCLUDES} )
target_include_directories(Jets SYSTEM PUBLIC ${CAP_FASTJET_INCLUDE_PATH})

cap_install(Jets LIBRARY "$ENV{CAP_LIB_PATH}")

################################################################################################
# Tests (run with ctest in the build directory; not installed)
################################################################################################
add_executable(TestJets tests/TestJets.cpp)
target_link_libraries(TestJets Jets Particles Base Math FASTJET_LIB)
add_test(NAME Jets COMMAND TestJets)
