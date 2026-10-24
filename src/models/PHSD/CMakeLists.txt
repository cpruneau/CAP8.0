################################################################################################
# Project CAP/PHSD
################################################################################################


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_library(PHSD SHARED PHSDEventReader.cpp)
target_link_libraries(PHSD Base Particles ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(PHSD  PUBLIC Base  Particles  Cluster PHSD ${EXTRA_INCLUDES})

install(TARGETS PHSD  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

