################################################################################################
# Project Datum/Jets: jets found with FastJet (optional module, DATUM_WITH_JETS)
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Jets SHARED
JetTasks.cpp
Jet.cpp
JetSettings.cpp
JetClustering.cpp
JetFilter.cpp
JetHistos.cpp
JetAnalyzer.cpp
JetPair.cpp
JetPairHistos.cpp
JetPairAnalyzer.cpp
JetUnderlyingEvent.cpp
JetGroomer.cpp
JetGroomingHistos.cpp
JetGroomingAnalyzer.cpp)

target_link_libraries(Jets Base Math Particles FASTJET_LIB FASTJET_TOOLS_LIB ${EXTRA_LIBS} )
target_include_directories(Jets PUBLIC Jets Particles Base Math ${EXTRA_INCLUDES} )
target_include_directories(Jets SYSTEM PUBLIC ${DATUM_FASTJET_INCLUDE_PATH})

datum_install(Jets LIBRARY "$ENV{DATUM_LIB_PATH}")

################################################################################################
# Tests (run with ctest in the build directory; not installed)
################################################################################################
add_executable(TestJets tests/TestJets.cpp)
target_link_libraries(TestJets Jets Particles Base Math FASTJET_LIB FASTJET_TOOLS_LIB)
add_test(NAME Jets COMMAND TestJets)
