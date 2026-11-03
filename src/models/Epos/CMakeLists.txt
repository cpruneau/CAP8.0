################################################################################################
# Project CAP/Epos
################################################################################################


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_library(Epos SHARED EposTasks.cpp EposEventReader.cpp)
target_link_libraries(Epos Base Helpers Particles ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Epos  PUBLIC Base  Particles  Cluster Epos ${EXTRA_INCLUDES})

install(TARGETS Epos  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

