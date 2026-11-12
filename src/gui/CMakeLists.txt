################################################################################################
# Project CAP/gui : bin/CapGui, the CAP graphical interface (Dear ImGui + GLFW + OpenGL 3, no ROOT GUI classes)
#
# Dear ImGui (docking branch) and GLFW are distributed with CAP in <CAP>/external/imgui and <CAP>/external/glfw.
# A GLFW installed on the system (e.g. brew install glfw) is used instead when CMake finds it.
################################################################################################

set(CAP_IMGUI_DIR "${CAP_TOP_DIR}/external/imgui")
set(CAP_GLFW_DIR  "${CAP_TOP_DIR}/external/glfw")

if(NOT EXISTS "${CAP_IMGUI_DIR}/imgui.cpp")
  message(FATAL_ERROR "CapGui: Dear ImGui not found in ${CAP_IMGUI_DIR}")
endif()

# ---- GLFW: system package if available, otherwise the copy distributed with CAP (static library)
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
  add_subdirectory("${CAP_GLFW_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/glfw" EXCLUDE_FROM_ALL)
  target_compile_options(glfw PRIVATE -w)                  # third-party code: no warnings
  message(STATUS "CapGui: using the GLFW distributed with CAP (${CAP_GLFW_DIR})")
else()
  message(STATUS "CapGui: using the system GLFW")
endif()

find_package(OpenGL REQUIRED)

# ---- Dear ImGui: core, GLFW and OpenGL3 backends, std::string helpers
add_library(CapImGui STATIC
  ${CAP_IMGUI_DIR}/imgui.cpp
  ${CAP_IMGUI_DIR}/imgui_draw.cpp
  ${CAP_IMGUI_DIR}/imgui_tables.cpp
  ${CAP_IMGUI_DIR}/imgui_widgets.cpp
  ${CAP_IMGUI_DIR}/imgui_demo.cpp
  ${CAP_IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${CAP_IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
  ${CAP_IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp)
target_include_directories(CapImGui PUBLIC ${CAP_IMGUI_DIR} ${CAP_IMGUI_DIR}/backends ${CAP_IMGUI_DIR}/misc/cpp)
target_link_libraries(CapImGui PUBLIC glfw OpenGL::GL)
set_target_properties(CapImGui PROPERTIES CXX_STANDARD 17 POSITION_INDEPENDENT_CODE ON)
target_compile_options(CapImGui PRIVATE -w)                # third-party code: no warnings
if(APPLE)
  target_compile_definitions(CapImGui PUBLIC GL_SILENCE_DEPRECATION)
endif()

# ---- the interface, as a library used by bin/CapGui and by the programs built on it (bin/PlotHistos)
add_library(CapGuiCore STATIC
  CapApp.cpp
  CapGui.cpp
  CapGuiApp.cpp
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
set_target_properties(CapGuiCore PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(CapGuiCore PUBLIC CAP_SOURCE_ROOT="${CAP_TOP_DIR}")
target_include_directories(CapGuiCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} PRIVATE ${CAP_TOP_DIR}/external/nanosvg)
set_source_files_properties(NanoSvg.cpp PROPERTIES COMPILE_OPTIONS "-w")   # nanosvg: third-party code, no warnings
target_link_libraries(CapGuiCore PUBLIC CapImGui Graphics Particles Math Base)
if(CAP_WITH_LATEX)   # File > Make Presentation (Latex/CanvasPresentation)
  target_link_libraries(CapGuiCore PUBLIC Latex)
  target_compile_definitions(CapGuiCore PUBLIC CAP_GUI_PRESENTATIONS)
endif()
if(CAP_WITH_ROOT)   # .root files in the browser: their histograms converted (Helpers/RootFileConversion)
  target_link_libraries(CapGuiCore PUBLIC Helpers)
  target_compile_definitions(CapGuiCore PUBLIC CAP_WITH_ROOT)
endif()

# ---- CapGui: the canvases, the plot files and the browser (see CapGuiApp.hpp); the analysis libraries are linked
# in src/CMakeLists.txt (File > New from Data configures workflows, see PlotCatalog.hpp)
add_executable(CapGui CapGuiMain.cpp PlotCatalog.cpp)
set_target_properties(CapGui PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(CapGui PRIVATE CapGuiCore)

# ---- PlotHistos: the outputs of a plot file in batch mode, or CapGui on it (see PlotHistos.C)
add_executable(PlotHistos PlotHistos.C PlotCatalog.cpp)
set_source_files_properties(PlotHistos.C PROPERTIES LANGUAGE CXX)
set_target_properties(PlotHistos PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON LINKER_LANGUAGE CXX
                      INSTALL_RPATH "$ENV{CAP_LIB_PATH}")
target_link_libraries(PlotHistos PRIVATE CapGuiCore)
install(TARGETS PlotHistos RUNTIME DESTINATION "${CAP_INSTALL_BIN_DIR}")

# ---- PlotEditor: another name of CapGui (PlotEditor plots.xml opens the plot file in CapGui)
add_executable(PlotEditor CapGuiMain.cpp PlotCatalog.cpp)
set_target_properties(PlotEditor PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON
                      INSTALL_RPATH "$ENV{CAP_LIB_PATH}")
target_link_libraries(PlotEditor PRIVATE CapGuiCore)
install(TARGETS PlotEditor RUNTIME DESTINATION "${CAP_INSTALL_BIN_DIR}")

if(APPLE)
  # macOS: CapGui.app, an application bundle that can be opened from the Finder or the Dock. Started that way it gets
  # none of the environment of SetupCAP.sh, so it finds the CAP libraries through its rpath. bin/CapGui is a symbolic
  # link to the executable in the bundle, for use from a terminal.
  set(MACOSX_BUNDLE_BUNDLE_NAME          "CapGui")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER       "CAP.CapGui")
  set(CAP_GUI_VERSION "${PROJECT_VERSION}")
  if(CAP_GUI_VERSION STREQUAL "")
    set(CAP_GUI_VERSION "1.0")
  endif()
  set(MACOSX_BUNDLE_BUNDLE_VERSION       "${CAP_GUI_VERSION}")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${CAP_GUI_VERSION}")
  set_target_properties(CapGui PROPERTIES
    MACOSX_BUNDLE            TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    INSTALL_RPATH            "$ENV{CAP_LIB_PATH}")
  install(TARGETS CapGui BUNDLE DESTINATION "${CAP_INSTALL_BIN_DIR}")
  install(CODE "
    file(REMOVE \"${CAP_INSTALL_BIN_DIR}/CapGui\")
    file(CREATE_LINK \"CapGui.app/Contents/MacOS/CapGui\" \"${CAP_INSTALL_BIN_DIR}/CapGui\" SYMBOLIC)
    message(STATUS \"Installing: ${CAP_INSTALL_BIN_DIR}/CapGui -> CapGui.app/Contents/MacOS/CapGui\")")
else()
  set_target_properties(CapGui PROPERTIES INSTALL_RPATH "$ENV{CAP_LIB_PATH}")
  install(TARGETS CapGui RUNTIME DESTINATION "${CAP_INSTALL_BIN_DIR}")
endif()
