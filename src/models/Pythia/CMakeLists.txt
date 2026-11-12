################################################################################################
# Project CAP/Pythia
################################################################################################


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
# PythiaEventReader reads Pythia events stored in ROOT trees: built only with CAP_WITH_ROOT
set(CAP_PYTHIA_SOURCES PythiaTasks.cpp PythiaEventGenerator.cpp)
if(CAP_WITH_ROOT)
  list(APPEND CAP_PYTHIA_SOURCES PythiaEventReader.cpp)
endif()
add_library(Pythia SHARED ${CAP_PYTHIA_SOURCES})
target_link_libraries(Pythia  Base  Particles  PYTHIA8_LIB  ${EXTRA_LIBS} )
if(CAP_WITH_ROOT)
  target_link_libraries(Pythia Helpers ${ROOT_LIBRARIES})
endif()
target_include_directories(Pythia  PUBLIC  Base  Particles Pythia)
target_include_directories(Pythia  SYSTEM PUBLIC "${CAP_PYTHIA8_INCLUDE_PATH}" "${CAP_PYTHIA8_INCLUDE_PATH}/Pythia8"
 ${EXTRA_INCLUDES})

# xml directory of the Pythia8 found by cmake: the last fallback when neither xmlInputPath, $PYTHIA8DATA nor
# $CAP_PYTHIA8_PATH is set (e.g. jobs run by RunCAP.py)
target_compile_definitions(Pythia PRIVATE CAP_PYTHIA8_XMLDOC="${CAP_PYTHIA8_DIR}/share/Pythia8/xmldoc")

cap_install(Pythia LIBRARY "$ENV{CAP_LIB_PATH}") 
