################################################################################################
# Project Datum/Pythia
################################################################################################


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
# PythiaEventReader reads Pythia events stored in ROOT trees: built only with DATUM_WITH_ROOT
set(DATUM_PYTHIA_SOURCES PythiaTasks.cpp PythiaEventGenerator.cpp)
if(DATUM_WITH_ROOT)
  list(APPEND DATUM_PYTHIA_SOURCES PythiaEventReader.cpp)
endif()
add_library(Pythia SHARED ${DATUM_PYTHIA_SOURCES})
target_link_libraries(Pythia  Base  Particles  PYTHIA8_LIB  ${EXTRA_LIBS} )
if(DATUM_WITH_ROOT)
  target_link_libraries(Pythia RootConvert ${ROOT_LIBRARIES})
endif()
target_include_directories(Pythia  PUBLIC  Base  Particles Pythia)
target_include_directories(Pythia  SYSTEM PUBLIC "${DATUM_PYTHIA8_INCLUDE_PATH}" "${DATUM_PYTHIA8_INCLUDE_PATH}/Pythia8"
 ${EXTRA_INCLUDES})

# xml directory of the Pythia8 found by cmake: the last fallback when neither xmlInputPath, $PYTHIA8DATA nor
# $DATUM_PYTHIA8_PATH is set (e.g. jobs run by RunDatum.py)
target_compile_definitions(Pythia PRIVATE DATUM_PYTHIA8_XMLDOC="${DATUM_PYTHIA8_DIR}/share/Pythia8/xmldoc")

datum_install(Pythia LIBRARY "$ENV{DATUM_LIB_PATH}") 
