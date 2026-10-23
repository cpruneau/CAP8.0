################################################################################################
# Project CAP/Graphics
################################################################################################

#include(${ROOT_USE_FILE})
#include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
#add_definitions(${ROOT_CXX_FLAGS})

# MarkerErrors is left out of the build until it is rewritten (MarkerErrors.hpp/.cpp do not compile yet).
# Graphs of functions and data of rank 2 (color maps) are drawn by Graph, with Grid2D and Palette.
set(GRAPHICS_HEADERS
Attributes.hpp
Axis.hpp
AxisRange.hpp
AxisRenderer.hpp
Canvas.hpp
Color.hpp
Drawable2D.hpp
FillAttributes.hpp
Geom2D.hpp
Graph.hpp
GraphRenderer.hpp
Grid2D.hpp
Line.hpp
LineAttributes.hpp
LineRenderer.hpp
Margins.hpp
Marker.hpp
MarkerAttributes.hpp
MarkerRenderer.hpp
MarkerVector.hpp
Painter.hpp
Palette.hpp
Point2D.hpp
Point3D.hpp
Rect2D.hpp
Polygon.hpp
Projection3D.hpp
PolygonRenderer.hpp
Renderer.hpp
SvgPainter.hpp
PdfPainter.hpp
TextAttributes.hpp
TextRenderer.hpp)

set(GRAPHICS_SOURCES
Attributes.cpp
Axis.cpp
AxisRange.cpp
AxisRenderer.cpp
Canvas.cpp
Drawable2D.cpp
FillAttributes.cpp
Geom2D.cpp
Graph.cpp
GraphRenderer.cpp
GraphRenderer3D.cpp
Grid2D.cpp
LatexRenderer.cpp
Line.cpp
LineAttributes.cpp
LineRenderer.cpp
Margins.cpp
Marker.cpp
MarkerAttributes.cpp
MarkerRenderer.cpp
MarkerVector.cpp
Painter.cpp
Palette.cpp
Polygon.cpp
Projection3D.cpp
PolygonRenderer.cpp
Renderer.cpp
SvgPainter.cpp
PdfPainter.cpp
TextAttributes.cpp
TextRenderer.cpp)

# macOS window backend: Objective-C++ (AppKit, CoreGraphics, CoreText), compiled with ARC
if(APPLE)
  list(APPEND GRAPHICS_HEADERS MacOsCanvas.hpp)
  list(APPEND GRAPHICS_SOURCES MacOsCanvas.mm)
  set_source_files_properties(MacOsCanvas.mm PROPERTIES LANGUAGE CXX COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
endif()

ROOT_GENERATE_DICTIONARY(G__Graphics
${GRAPHICS_HEADERS}
LINKDEF GraphicsLinkDef.h)

################################################################################################
# Create a shared library with geneated dictionary
################################################################################################
add_compile_options(-Wall -Wextra -pedantic)
add_library(Graphics SHARED
${GRAPHICS_SOURCES}
G__Graphics.cxx)

target_link_libraries(Graphics Base Math ${ROOT_LIBRARIES} ${EXTRA_LIBS} )
if(APPLE)
  target_link_libraries(Graphics "-framework Cocoa" "-framework CoreText")
endif()
target_include_directories(Graphics PUBLIC Base Math Graphics ${EXTRA_INCLUDES} )


# Tests (run with ctest in the build directory; not installed)
add_executable(TestLatexRenderer tests/TestLatexRenderer.cpp)
target_link_libraries(TestLatexRenderer Graphics Math Base ${ROOT_LIBRARIES})
add_test(NAME LatexRenderer COMMAND TestLatexRenderer)
add_executable(TestGraph2D tests/TestGraph2D.cpp)
target_link_libraries(TestGraph2D Graphics Math Base ${ROOT_LIBRARIES})
add_test(NAME Graph2D COMMAND TestGraph2D)
add_executable(TestPdfPainter tests/TestPdfPainter.cpp)
target_link_libraries(TestPdfPainter Graphics Math Base ${ROOT_LIBRARIES})
add_test(NAME PdfPainter COMMAND TestPdfPainter)
add_executable(TestGraph3D tests/TestGraph3D.cpp)
target_link_libraries(TestGraph3D Graphics Math Base ${ROOT_LIBRARIES})
add_test(NAME Graph3D COMMAND TestGraph3D)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/libGraphics.rootmap" "${CMAKE_CURRENT_BINARY_DIR}/libGraphics_rdict.pcm" DESTINATION "$ENV{CAP_LIB_PATH}")
install(TARGETS Graphics  LIBRARY DESTINATION "$ENV{CAP_LIB_PATH}")

# macOS demo program: bin/MacOsCanvasDemo opens a window showing a sample canvas
if(APPLE)
  add_executable(MacOsCanvasDemo MacOsCanvasDemo.C)
  set_source_files_properties(MacOsCanvasDemo.C PROPERTIES LANGUAGE CXX)
  target_link_libraries(MacOsCanvasDemo Graphics Math Base ${ROOT_LIBRARIES})
  install(TARGETS MacOsCanvasDemo RUNTIME DESTINATION "${CAP_INSTALL_BIN_DIR}")
endif()
