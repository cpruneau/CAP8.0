################################################################################################
# Project Datum/PHSD
################################################################################################


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_library(PHSD SHARED PHSDTasks.cpp PHSDEventReader.cpp)
target_link_libraries(PHSD Base Particles ${EXTRA_LIBS} )
target_include_directories(PHSD  PUBLIC Base  Particles  Cluster PHSD ${EXTRA_INCLUDES})

datum_install(PHSD LIBRARY "$ENV{DATUM_LIB_PATH}")

