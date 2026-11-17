################################################################################################
# Project Datum/gui : bin/DatumGui, the Datum graphical interface (Dear ImGui + GLFW + OpenGL 3, no ROOT GUI classes)
#
# Dear ImGui (docking branch) and GLFW are distributed with Datum in <Datum>/external/imgui and <Datum>/external/glfw.
# A GLFW installed on the system (e.g. brew install glfw) is used instead when CMake finds it.
################################################################################################

set(DATUM_IMGUI_DIR "${DATUM_TOP_DIR}/external/imgui")
set(DATUM_GLFW_DIR  "${DATUM_TOP_DIR}/external/glfw")

if(NOT EXISTS "${DATUM_IMGUI_DIR}/imgui.cpp")
  message(FATAL_ERROR "DatumGui: Dear ImGui not found in ${DATUM_IMGUI_DIR}")
endif()

# ---- GLFW: system package if available, otherwise the copy distributed with Datum (static library)
find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND)
  set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS   OFF)
  if(UNIX AND NOT APPLE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)        # X11 (also works under XWayland); needs no wayland-scanner
  endif()
  add_subdirectory("${DATUM_GLFW_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/glfw" EXCLUDE_FROM_ALL)
  target_compile_options(glfw PRIVATE -w)                  # third-party code: no warnings
  message(STATUS "DatumGui: using the GLFW distributed with Datum (${DATUM_GLFW_DIR})")
else()
  message(STATUS "DatumGui: using the system GLFW")
endif()

find_package(OpenGL REQUIRED)

# ---- Dear ImGui: core, GLFW and OpenGL3 backends, std::string helpers
add_library(DatumImGui STATIC
  ${DATUM_IMGUI_DIR}/imgui.cpp
  ${DATUM_IMGUI_DIR}/imgui_draw.cpp
  ${DATUM_IMGUI_DIR}/imgui_tables.cpp
  ${DATUM_IMGUI_DIR}/imgui_widgets.cpp
  ${DATUM_IMGUI_DIR}/imgui_demo.cpp
  ${DATUM_IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${DATUM_IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
  ${DATUM_IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp)
target_include_directories(DatumImGui PUBLIC ${DATUM_IMGUI_DIR} ${DATUM_IMGUI_DIR}/backends ${DATUM_IMGUI_DIR}/misc/cpp)
target_link_libraries(DatumImGui PUBLIC glfw OpenGL::GL)
set_target_properties(DatumImGui PROPERTIES CXX_STANDARD 17 POSITION_INDEPENDENT_CODE ON)
target_compile_options(DatumImGui PRIVATE -w)                # third-party code: no warnings
if(APPLE)
  target_compile_definitions(DatumImGui PUBLIC GL_SILENCE_DEPRECATION)
endif()

# ---- the interface, as a library used by bin/DatumGui and by the programs built on it (bin/PlotHistos)
add_library(DatumGuiCore STATIC
  DatumApp.cpp
  DatumGui.cpp
  DatumGuiApp.cpp
  PlotOutputs.cpp
  CanvasPane.cpp
  DataPane.cpp
  FitDialog.cpp
  DetectorMapsDialog.cpp
  FitPane.cpp
  FileBrowser.cpp
  FileDialog.cpp
  GraphEditor.cpp
  ImGuiPainter.cpp
  LatexTextures.cpp
  NanoSvg.cpp
  Menu.cpp
  MenuBar.cpp
  MenuItem.cpp)
set_target_properties(DatumGuiCore PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(DatumGuiCore PUBLIC DATUM_SOURCE_ROOT="${DATUM_TOP_DIR}")
target_include_directories(DatumGuiCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} PRIVATE ${DATUM_TOP_DIR}/external/nanosvg)
set_source_files_properties(NanoSvg.cpp PROPERTIES COMPILE_OPTIONS "-w")   # nanosvg: third-party code, no warnings
target_link_libraries(DatumGuiCore PUBLIC DatumImGui Graphics Particles Math Base)
if(DATUM_WITH_LATEX)   # File > Make Presentation (Latex/CanvasPresentation)
  target_link_libraries(DatumGuiCore PUBLIC Latex)
  target_compile_definitions(DatumGuiCore PUBLIC DATUM_GUI_PRESENTATIONS)
endif()
if(DATUM_WITH_ROOT)   # .root files in the browser: their histograms converted (RootConvert/RootFileConversion)
  target_link_libraries(DatumGuiCore PUBLIC RootConvert)
  target_compile_definitions(DatumGuiCore PUBLIC DATUM_WITH_ROOT)
endif()

# ---- DatumGui: the canvases, the plot files and the browser (see DatumGuiApp.hpp); the analysis libraries are linked
# in src/CMakeLists.txt (File > New from Data configures workflows, see PlotCatalog.hpp)
add_executable(DatumGui DatumGuiMain.cpp PlotCatalog.cpp)
set_target_properties(DatumGui PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON
                      OUTPUT_NAME DatumGui)                  # bin/DatumGui (bin/CapGui: a link to it)
target_link_libraries(DatumGui PRIVATE DatumGuiCore)

# ---- PlotHistos: the outputs of a plot file in batch mode, or DatumGui on it (see PlotHistos.C)
add_executable(PlotHistos PlotHistos.C PlotCatalog.cpp)
set_source_files_properties(PlotHistos.C PROPERTIES LANGUAGE CXX)
set_target_properties(PlotHistos PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON LINKER_LANGUAGE CXX
                      INSTALL_RPATH "$ENV{DATUM_LIB_PATH}")
target_link_libraries(PlotHistos PRIVATE DatumGuiCore)
datum_install(PlotHistos RUNTIME "${DATUM_INSTALL_BIN_DIR}")

# ---- PlotEditor: another name of DatumGui (PlotEditor plots.xml opens the plot file in DatumGui)
add_executable(PlotEditor DatumGuiMain.cpp PlotCatalog.cpp)
set_target_properties(PlotEditor PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON
                      INSTALL_RPATH "$ENV{DATUM_LIB_PATH}")
target_link_libraries(PlotEditor PRIVATE DatumGuiCore)
datum_install(PlotEditor RUNTIME "${DATUM_INSTALL_BIN_DIR}")

if(APPLE)
  # macOS: DatumGui.app, an application bundle that can be opened from the Finder or the Dock. Started that way it
  # gets none of the environment of SetupDatum.sh, so it finds the libraries through its rpath. bin/DatumGui (and
  # bin/CapGui, its former name) are symbolic links to the executable in the bundle, for use from a terminal.
  set(MACOSX_BUNDLE_BUNDLE_NAME          "DatumGui")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER       "Datum.DatumGui")
  set(DATUM_GUI_VERSION "${PROJECT_VERSION}")
  if(DATUM_GUI_VERSION STREQUAL "")
    set(DATUM_GUI_VERSION "1.0")
  endif()
  set(MACOSX_BUNDLE_BUNDLE_VERSION       "${DATUM_GUI_VERSION}")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${DATUM_GUI_VERSION}")
  set_target_properties(DatumGui PROPERTIES
    MACOSX_BUNDLE            TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    INSTALL_RPATH            "$ENV{DATUM_LIB_PATH}")
  datum_install(DatumGui BUNDLE "${DATUM_INSTALL_BIN_DIR}")
  foreach(link DatumGui CapGui)
    install(CODE "
      file(REMOVE \"${DATUM_INSTALL_BIN_DIR}/${link}\")
      file(CREATE_LINK \"DatumGui.app/Contents/MacOS/DatumGui\" \"${DATUM_INSTALL_BIN_DIR}/${link}\" SYMBOLIC)
      message(STATUS \"Installing: ${DATUM_INSTALL_BIN_DIR}/${link} -> DatumGui.app/Contents/MacOS/DatumGui\")")
  endforeach()
else()
  set_target_properties(DatumGui PROPERTIES INSTALL_RPATH "$ENV{DATUM_LIB_PATH}")
  datum_install(DatumGui RUNTIME "${DATUM_INSTALL_BIN_DIR}")
  datum_install_alias(DatumGui CapGui "${DATUM_INSTALL_BIN_DIR}")
endif()
