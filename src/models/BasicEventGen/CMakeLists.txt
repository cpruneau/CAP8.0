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

################################################################################################
# Tests (run with ctest in the build directory; not installed)
################################################################################################
add_executable(TestBasicEventGen tests/TestBasicEventGen.cpp)
target_link_libraries(TestBasicEventGen BasicEventGen Particles Math Base ${ROOT_LIBRARIES})
target_compile_definitions(TestBasicEventGen PRIVATE CAP_PARTICLE_DB="${CMAKE_SOURCE_DIR}/../DB/ParticleData/ParticleDb.xml")
add_test(NAME BasicEventGen COMMAND TestBasicEventGen)
