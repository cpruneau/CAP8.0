################################################################################################
# Project CAP/Glauber
################################################################################################

#include(${ROOT_USE_FILE})
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
#add_definitions(${ROOT_CXX_FLAGS})


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Glauber SHARED
GlauberTasks.cpp
GlauberAnalyzer.cpp
GlauberDb.cpp
GlauberEvent.cpp
GlauberEventFilter.cpp
GlauberEventMoments.cpp
GlauberExceptions.cpp 
GlauberGenerator.cpp
GlauberHistos.cpp
GlauberNucleon.cpp
GlauberNucleusType.cpp
GlauberNucleus.cpp
GlauberPoint.cpp
NNXSectVsSqrtS.cpp
NNProfile.cpp
NucleonProfile.cpp
NuclearDistribution.cpp
NNXSectVsSqrtS.cpp
PTotProfile.cpp
SmearingProfile.cpp)

target_link_libraries(Glauber Base Math Helpers Particles  ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Glauber PUBLIC Base Math Glauber Helpers ParticleDb Particles ${EXTRA_INCLUDES} )

install(TARGETS Glauber  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

