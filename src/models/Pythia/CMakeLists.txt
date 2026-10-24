################################################################################################
# Project CAP/Pythia
################################################################################################


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_library(Pythia SHARED PythiaEventGenerator.cpp PythiaEventReader.cpp)
target_link_libraries(Pythia  Base  Particles  PYTHIA8_LIB  ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
#target_link_libraries(Pythia  Base  Particles    ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Pythia  PUBLIC  Base  Particles Pythia "${CAP_PYTHIA8_INCLUDE_PATH}" "${CAP_PYTHIA8_INCLUDE_PATH}/Pythia8"
 ${EXTRA_INCLUDES})

install(TARGETS Pythia  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}") 
