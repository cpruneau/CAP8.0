################################################################################################
# Project CAP/Latex
################################################################################################

#include(${ROOT_USE_FILE})
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
#add_definitions(${ROOT_CXX_FLAGS})


################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Latex SHARED
LatexAuthor.cpp
LatexCommand.cpp
LatexDocument.cpp
LatexElement.cpp
LatexEquation.cpp
LatexFigure.cpp
LatexFigureGroup.cpp
LatexFrame.cpp
LatexList.cpp
LatexPackage.cpp
LatexParagraph.cpp
LatexSection.cpp
LatexSubsection.cpp
LatexSubsubsection.cpp
LatexText.cpp)

target_link_libraries(Latex Base Math Helpers ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
target_include_directories(Latex PUBLIC Base Math ${EXTRA_INCLUDES} )

install(TARGETS Latex LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

