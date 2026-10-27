################################################################################################
# Project CAP/Pythia
################################################################################################


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_library(Pythia SHARED
PythiaTasks.cpp
PythiaEventGenerator.cpp
PythiaEventReader.cpp)
target_link_libraries(Pythia  Base  Helpers  Particles  PYTHIA8_LIB  ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Pythia  PUBLIC  Base  Particles Pythia "${CAP_PYTHIA8_INCLUDE_PATH}" "${CAP_PYTHIA8_INCLUDE_PATH}/Pythia8"
 ${EXTRA_INCLUDES})

# xml directory of the Pythia8 found by cmake: the last fallback when neither xmlInputPath, $PYTHIA8DATA nor
# $CAP_PYTHIA8_PATH is set (e.g. jobs run by RunCAP.py)
target_compile_definitions(Pythia PRIVATE CAP_PYTHIA8_XMLDOC="${CAP_PYTHIA8_DIR}/share/Pythia8/xmldoc")

install(TARGETS Pythia  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}") 
