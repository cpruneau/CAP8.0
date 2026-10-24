################################################################################################
# Project CAP/BasicEventGen
################################################################################################


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_library(BasicEventGen SHARED
BasicEventGenTasks.cpp
BasicEventGen.cpp)
target_link_libraries(BasicEventGen  Base  Particles   ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(BasicEventGen  PUBLIC  Base Math Particles ${EXTRA_INCLUDES})

install(TARGETS BasicEventGen  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}") 
