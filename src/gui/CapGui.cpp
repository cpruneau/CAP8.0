/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CapGui.cpp
//! @brief Implementation of CapGui.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include "imgui.h"
#include "FileSystem.hpp"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
//! Silence the deprecation warnings of OpenGL on macOS.
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include "CapGui.hpp"
#include "ImGuiPainter.hpp"
#include "LatexRenderer.hpp"
#include "LatexTextures.hpp"
#include "SvgPainter.hpp"

#ifndef CAP_SOURCE_ROOT
//! Top directory of the CAP sources (set by the build; empty if not).
#define CAP_SOURCE_ROOT ""
#endif

namespace CAP
{
  namespace GUI
  {

  CapGui::CapGui()
  :
  onNewPane(),
  beforePanesChange(),
  onFrame(),
  onCloseRequest(),
  _window(nullptr),
  _settings(),
  _font(nullptr),
  _menuBar(),
  _fileDialog(),
  _panes(),
  _columns(0), _rows(0), _currentPane(0),
  _paneCounter(0),
  _status(),
  _statusTime(0),
  _messageTitle(), _messageText(),
  _messageRequested(false),
  _exitRequested(false),
  _areaX(0), _areaY(0), _areaWidth(0), _areaHeight(0),
  _iniFile(),
  _hoverText(),
  _browsers(),
  _activeBrowser(0),
  _browserCounter(1),
  _firstFrame(true),
  _layoutRequested(false),
  _undockRequested(false),
  _gridColumns(1),
  _dockBeside(),
  _closeRequests(),
  _dataPanes(),
  _dataCounter(0),
  _addRequested(false),
  _focusedPane(-1),
  _focusRequested(false)
  {
  _fitPane.onNewFit = [this]() { openFitDialog(); };
  _browsers.emplace_back(new FileBrowser(1));          // the first browser, always there
  }

  CapGui::~CapGui()
  {
  if (_window)
    {
    GRAPHICS::LatexRenderer::instance().setReadyCallback(nullptr);
    LatexTextures::instance().clear();                  // while the OpenGL context exists
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(_window);
    glfwTerminate();
    }
  }

  namespace
  {
  void glfwError(int code, const char * description)
  {
  std::fprintf(stderr, "CapGui: GLFW error %d: %s\n", code, description);
  }

  //! First existing file among the candidates, or "" (see FileSystem::firstExisting()).
  std::string firstExisting(const std::vector<std::string> & candidates)
  {
  return FileSystem::firstExisting(std::vector<String>(candidates.begin(), candidates.end())).str();
  }
  } // namespace

  bool CapGui::initialize(const GuiSettings & settings)
  {
  _settings = settings;
  glfwSetErrorCallback(glfwError);
#ifdef __APPLE__
  glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);   // keep the current folder (CapGui.app starts in the home folder)
#endif
  if (!glfwInit()) return false;

#if defined(__APPLE__)
  const char * glslVersion = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
  const char * glslVersion = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
  _window = glfwCreateWindow(_settings.width, _settings.height, _settings.title.data(), nullptr, nullptr);
  if (!_window) { glfwTerminate(); return false; }
  glfwMakeContextCurrent(_window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO & io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
  if (_settings.viewports) io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  _iniFile = _settings.iniFile.str();
  io.IniFilename = _iniFile.empty() ? nullptr : _iniFile.c_str();
  setTheme(_settings.theme);
  loadFonts();
  ImGui_ImplGlfw_InitForOpenGL(_window, true);
  ImGui_ImplOpenGL3_Init(glslVersion);
  // texts typeset by LaTeX in the background: wake up the event loop to draw them
  GRAPHICS::LatexRenderer::instance().setReadyCallback([]() { glfwPostEmptyEvent(); });
  setLayout(_settings.columns, _settings.rows);
  return true;
  }

  void CapGui::loadFonts()
  {
  ImGuiIO & io = ImGui::GetIO();
  std::vector<std::string> candidates;
  candidates.push_back(_settings.fontFile.str());
  candidates.push_back(FileSystem::expand("$CAP_ROOT_PATH/external/imgui/misc/fonts/Roboto-Medium.ttf").str());
  candidates.push_back(std::string(CAP_SOURCE_ROOT) + "/external/imgui/misc/fonts/Roboto-Medium.ttf");
  std::string file = firstExisting(candidates);
  _font = nullptr;
  if (!file.empty()) _font = io.Fonts->AddFontFromFileTTF(file.c_str(), _settings.fontSize);
  if (!_font)
    {
    ImFontConfig config;
    config.SizePixels = _settings.fontSize;
    _font = io.Fonts->AddFontDefault(&config);
    }

  // fonts of the plots, by ROOT font index (see TextAttributes): Helvetica is the interface font, its other styles
  // and the Times and Courier families come from the system (macOS, or Liberation fonts on Linux) when available
  std::vector<std::string> directories{ "/System/Library/Fonts/Supplemental/", "/Library/Fonts/",
                                        "/usr/share/fonts/truetype/liberation/", "/usr/share/fonts/liberation-sans/",
                                        "/usr/share/fonts/liberation-serif/", "/usr/share/fonts/liberation-mono/",
                                        "/usr/share/fonts/truetype/msttcorefonts/" };
  struct PlotFont { int index; std::vector<std::string> names; };
  const std::vector<PlotFont> plotFonts = {
    {  5, { "Arial Italic.ttf",                "LiberationSans-Italic.ttf" } },
    {  6, { "Arial Bold.ttf",                  "LiberationSans-Bold.ttf" } },
    {  7, { "Arial Bold Italic.ttf",           "LiberationSans-BoldItalic.ttf" } },
    { 13, { "Times New Roman.ttf",             "LiberationSerif-Regular.ttf" } },
    {  1, { "Times New Roman Italic.ttf",      "LiberationSerif-Italic.ttf" } },
    {  2, { "Times New Roman Bold.ttf",        "LiberationSerif-Bold.ttf" } },
    {  3, { "Times New Roman Bold Italic.ttf", "LiberationSerif-BoldItalic.ttf" } },
    {  8, { "Courier New.ttf",                 "LiberationMono-Regular.ttf" } },
    {  9, { "Courier New Italic.ttf",          "LiberationMono-Italic.ttf" } },
    { 10, { "Courier New Bold.ttf",            "LiberationMono-Bold.ttf" } },
    { 11, { "Courier New Bold Italic.ttf",     "LiberationMono-BoldItalic.ttf" } } };
  ImGuiPainter::setFont(4, _font);
  for (const auto & plotFont : plotFonts)
    {
    std::vector<std::string> files;
    for (const auto & name : plotFont.names) for (const auto & directory : directories) files.push_back(directory + name);
    std::string fontFile = firstExisting(files);
    ImFont * font = fontFile.empty() ? nullptr : io.Fonts->AddFontFromFileTTF(fontFile.c_str(), _settings.fontSize);
    ImGuiPainter::setFont(plotFont.index, font);
    }
  // Courier without a system font: the monospaced font distributed with Dear ImGui
  if (!ImGuiPainter::fontFor(82))
    {
    std::string cousine = firstExisting({ FileSystem::expand("$CAP_ROOT_PATH/external/imgui/misc/fonts/Cousine-Regular.ttf").str(),
                                          std::string(CAP_SOURCE_ROOT) + "/external/imgui/misc/fonts/Cousine-Regular.ttf" });
    if (!cousine.empty()) ImGuiPainter::setFont(8, io.Fonts->AddFontFromFileTTF(cousine.c_str(), _settings.fontSize));
    }
  }

  void CapGui::setTheme(const String & theme)
  {
  _settings.theme = theme;
  if      (theme == "dark")    ImGui::StyleColorsDark();
  else if (theme == "classic") ImGui::StyleColorsClassic();
  else                         ImGui::StyleColorsLight();
  ImGuiStyle & style = ImGui::GetStyle();
  style.WindowRounding = 0.0f;
  style.FrameRounding  = 3.0f;
  style.ChildBorderSize = 1.0f;
  style.FrameBorderSize = 1.0f;   // check boxes and input fields stay visible on white backgrounds
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  CanvasPane & CapGui::newPane()
  {
  char name[32];
  std::snprintf(name, sizeof(name), "Canvas %d", ++_paneCounter);
  _panes.emplace_back(new CanvasPane(name));
  CanvasPane & pane = *_panes.back();
  pane.setShowToolbar(_settings.toolbars);
  pane.onAddPane = [this]() { _addRequested = true; };
  if (onNewPane) onNewPane(pane);
  return pane;
  }

  void CapGui::setLayout(int nColumns, int nRows)
  {
  nColumns = nColumns < 1 ? 1 : (nColumns > 6 ? 6 : nColumns);
  nRows    = nRows    < 1 ? 1 : (nRows    > 6 ? 6 : nRows);
  size_t n = size_t(nColumns*nRows);
  while (_panes.size() > n) _panes.pop_back();
  while (_panes.size() < n) newPane();
  _columns = nColumns;
  _rows    = nRows;
  _gridColumns = nColumns;
  _layoutRequested = true;
  if (_currentPane >= nPanes()) _currentPane = 0;
  }

  void CapGui::setPaneCount(int n)
  {
  n = n < 1 ? 1 : (n > 36 ? 36 : n);
  if (n == nPanes()) return;
  while (nPanes() > n) _panes.pop_back();
  while (nPanes() < n) newPane();
  _gridColumns = int(std::ceil(std::sqrt(double(n))));
  _columns = _gridColumns;
  _rows    = (n + _gridColumns - 1)/_gridColumns;
  _layoutRequested = true;
  if (_currentPane >= nPanes()) _currentPane = 0;
  }

  CanvasPane & CapGui::addPane()
  {
  std::string beside = _panes.empty() ? std::string() : currentPane().name().str();
  CanvasPane & pane = newPane();
  _dockBeside.push_back(std::make_pair(pane.name().str(), beside));
  setCurrentPane(nPanes() - 1);
  return pane;
  }

  DataPane & CapGui::addDataPane()
  {
  char name[32];
  std::snprintf(name, sizeof(name), "Data %d", ++_dataCounter);
  _dataPanes.emplace_back(new DataPane(name));
  DataPane & pane = *_dataPanes.back();
  std::string beside = _panes.empty() ? std::string() : currentPane().name().str();
  _dockBeside.push_back(std::make_pair(std::string("###") + name, beside));
  return pane;
  }

  void CapGui::closePane(int k)
  {
  if (k < 0 || k >= nPanes()) return;
  if (nPanes() == 1)
    {
    // the last canvas (moved out of the main window): an empty one takes its place in the main window
    ImGuiWindow * window = ImGui::GetCurrentContext() ? ImGui::FindWindowByName(_panes[k]->name().data()) : nullptr;
    if (!window || !window->Viewport || window->Viewport == ImGui::GetMainViewport()) return;
    newPane();
    _gridColumns = 1;
    _layoutRequested = true;
    }
  // a dock space keeps its central node even when empty: lay the panes out again rather than leave a hole
  ImGuiWindow * window = ImGui::GetCurrentContext() ? ImGui::FindWindowByName(_panes[k]->name().data()) : nullptr;
  if (window && window->DockNode && window->DockNode->IsCentralNode())
    {
    _gridColumns = int(std::ceil(std::sqrt(double(nPanes() - 1))));
    _layoutRequested = true;
    }
  _panes.erase(_panes.begin() + k);
  if (_currentPane > k || _currentPane >= nPanes()) _currentPane = std::max(0, _currentPane - 1);
  _focusedPane = -1;
  }

  void CapGui::setFitPaneVisible(bool visible)
  {
  if (visible && !_fitsVisible)
    {
    // docked below the canvases the first time; afterwards where the user left it
    ImGuiWindow * window = ImGui::GetCurrentContext() ? ImGui::FindWindowByName(FitPane::windowName()) : nullptr;
    if (!window) _fitsDockRequested = true;
    _fitsShownOnce = true;
    }
  _fitsVisible = visible;
  }

  void CapGui::dockFitPane()
  {
  // the node of the canvases: the largest one holding the first canvas pane but not the docked file browser
  ImGuiID dockspace = ImHashStr("CapDockSpace");
  ImGuiWindow * first = _panes.empty() ? nullptr : ImGui::FindWindowByName(_panes[0]->name().data());
  ImGuiWindow * browser = ImGui::FindWindowByName(FileBrowser::mainWindowName());
  ImGuiDockNode * browserNode = browser ? browser->DockNode : nullptr;
  auto holdsBrowser = [browserNode](ImGuiDockNode * node)
    {
    for (ImGuiDockNode * n = browserNode; n; n = n->ParentNode) if (n == node) return true;
    return false;
    };
  ImGuiDockNode * node = first ? first->DockNode : nullptr;
  if (!node) node = ImGui::DockBuilderGetCentralNode(dockspace);
  if (!node) return;
  while (node->ParentNode && !holdsBrowser(node->ParentNode)) node = node->ParentNode;
  ImGuiID below = 0, above = 0;
  ImGui::DockBuilderSplitNode(node->ID, ImGuiDir_Down, 0.3f, &below, &above);
  ImGui::DockBuilderDockWindow(FitPane::windowName(), below);
  ImGui::DockBuilderFinish(dockspace);
  }

  void CapGui::renderFitPane()
  {
  if (!_fitsShownOnce && !_panes.empty())
    for (const auto & item : currentPane().currentGraph().items())
      if (!item.isMap() && item.nFits() > 0) { setFitPaneVisible(true); break; }
  if (!_fitsVisible) return;
  if (_fitsDockRequested) { dockFitPane(); _fitsDockRequested = false; }
  bool open = true;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
  bool visible = ImGui::Begin(FitPane::windowName(), &open, ImGuiWindowFlags_NoCollapse);
  ImGui::PopStyleVar();
  if (visible && !_panes.empty())
    {
    CanvasPane & pane = currentPane();
    _fitPane.render(&pane.currentGraph(), pane.beforeChange);
    }
  ImGui::End();
  _fitsVisible = open;
  }

  void CapGui::setShowToolbars(bool v)
  {
  _settings.toolbars = v;
  for (auto & pane : _panes) pane->setShowToolbar(v);
  }

  void CapGui::setStatus(const String & message)
  {
  _status = message;
  _statusTime = ImGui::GetCurrentContext() ? ImGui::GetTime() : 0.0;
  }

  void CapGui::showMessage(const String & title, const String & text)
  {
  _messageTitle = title;
  _messageText  = text;
  _messageRequested = true;
  }

  void CapGui::requestExit()
  {
  _exitRequested = true;
  }


  void CapGui::run()
  {
  if (!_window) return;
  ImGuiIO & io = ImGui::GetIO();
  while (true)
    {
    if (glfwWindowShouldClose(_window) || _exitRequested)
      {
      bool close = !onCloseRequest || onCloseRequest();
      if (close) break;
      glfwSetWindowShouldClose(_window, GLFW_FALSE);
      _exitRequested = false;
      }
    glfwWaitEventsTimeout(0.05);                       // low CPU use when idle
    if (glfwGetWindowAttrib(_window, GLFW_ICONIFIED)) { ImGui_ImplGlfw_Sleep(10); continue; }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    renderFrame();
    ImGui::Render();

    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);
    glViewport(0, 0, width, height);
    ImVec4 bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    glClearColor(bg.x, bg.y, bg.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
      {
      GLFWwindow * context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(context);
      }
    glfwSwapBuffers(_window);
    LatexTextures::instance().collect(ImGui::GetFrameCount());
    }
  }

  void CapGui::renderFrame()
  {
  const MenuItem * hovered = _menuBar.render();
  String hoverText;

  // main area: a dock space (the file browser and other tools dock around the canvases)
  ImGuiViewport * viewport = ImGui::GetMainViewport();
  float statusHeight = ImGui::GetFrameHeight();
  ImGuiWindowFlags sideFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
  if (ImGui::BeginViewportSideBar("##statusbar", viewport, ImGuiDir_Down, statusHeight, sideFlags))
    {
    if (ImGui::BeginMenuBar()) { renderStatusBar(hovered, hoverText); ImGui::EndMenuBar(); }
    }
  ImGui::End();
  ImGuiID dockspace = ImHashStr("CapDockSpace");
  if (_firstFrame)
    {
    // the canvas panes are laid out at startup; the browser follows the settings
    _layoutRequested = true;
    _firstFrame = false;
    }
  if (_layoutRequested) { buildDockLayout(dockspace); _layoutRequested = false; }
  ImGui::DockSpaceOverViewport(dockspace, viewport);
  renderCanvases(hoverText);

  renderBrowser();
  _fileDialog.render();
  renderMessage();
  if (onFrame) onFrame();

  }

  FileBrowser & CapGui::activeBrowser()
  {
  if (_activeBrowser < 0 || _activeBrowser >= int(_browsers.size())) _activeBrowser = 0;
  return *_browsers[_activeBrowser];
  }

  FileBrowser & CapGui::addBrowser()
  {
  _browsers.emplace_back(new FileBrowser(++_browserCounter));
  FileBrowser & browser = *_browsers.back();
  if (onNewBrowser) onNewBrowser(browser);
  _activeBrowser = int(_browsers.size()) - 1;
  _settings.browserVisible = true;
  return browser;
  }

  void CapGui::setBrowserDocked(bool docked)
  {
  if (docked == _settings.browserDocked) return;
  _settings.browserDocked = docked;
  _settings.browserVisible = true;
  if (docked) _layoutRequested = true; else _undockRequested = true;
  }

  void CapGui::buildDockLayout(unsigned int dockId)
  {
  ImGuiViewport * viewport = ImGui::GetMainViewport();
  ImGui::DockBuilderRemoveNode(dockId);
  ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockId, viewport->WorkSize);
  ImGuiID canvases = dockId;
  if (_settings.browserDocked)
    {
    ImGuiID left = 0;
    ImGui::DockBuilderSplitNode(dockId, ImGuiDir_Left, 0.22f, &left, &canvases);
    ImGui::DockBuilderDockWindow(FileBrowser::mainWindowName(), left);
    }
  buildPaneGrid(canvases);
  ImGui::DockBuilderFinish(dockId);
  }

  void CapGui::buildPaneGrid(unsigned int nodeId)
  {
  // rows from the top, then the panes of each row from the left (the last row may have fewer panes)
  int n = nPanes();
  if (n == 0) return;
  int columns = std::min(std::max(_gridColumns, 1), n);
  int rows = (n + columns - 1)/columns;
  std::vector<ImGuiID> rowNodes;
  ImGuiID remaining = nodeId;
  for (int r=0; r<rows-1; r++)
    {
    ImGuiID top = 0;
    ImGui::DockBuilderSplitNode(remaining, ImGuiDir_Up, 1.0f/float(rows - r), &top, &remaining);
    rowNodes.push_back(top);
    }
  rowNodes.push_back(remaining);
  for (int r=0; r<rows; r++)
    {
    int count = std::min(columns, n - r*columns);
    ImGuiID rest = rowNodes[r];
    for (int c=0; c<count-1; c++)
      {
      ImGuiID left = 0;
      ImGui::DockBuilderSplitNode(rest, ImGuiDir_Left, 1.0f/float(count - c), &left, &rest);
      ImGui::DockBuilderDockWindow(_panes[r*columns + c]->name().data(), left);
      }
    ImGui::DockBuilderDockWindow(_panes[r*columns + count - 1]->name().data(), rest);
    }
  }

  void CapGui::renderBrowser()
  {
  if (!_settings.browserVisible) return;
  ImGuiViewport * viewport = ImGui::GetMainViewport();
  if (_undockRequested)
    {
    // a window of its own: next to the left edge of the main window, outside it if the screen allows
    ImGui::SetNextWindowDockID(0, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 40, viewport->WorkPos.y + 60), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(380, viewport->WorkSize.y*0.75f), ImGuiCond_Always);
    _undockRequested = false;
    }
  else
    ImGui::SetNextWindowSize(ImVec2(380, viewport->WorkSize.y*0.75f), ImGuiCond_FirstUseEver);
  ImGuiWindowClass browserClass;
  if (!_settings.browserDocked) browserClass.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
  ImGui::SetNextWindowClass(&browserClass);
  bool visible = true;
  if (ImGui::Begin(FileBrowser::mainWindowName(), &visible))
    {
    // follow what the user did with the mouse (docking or undocking the window)
    if (!_layoutRequested) _settings.browserDocked = ImGui::IsWindowDocked();
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) _activeBrowser = 0;
    _browsers.front()->render();
    }
  else if (!_layoutRequested) _settings.browserDocked = ImGui::IsWindowDocked();
  ImGui::End();
  _settings.browserVisible = visible;

  // the other browsers: tabs beside the first one when they are made, then wherever they are moved
  ImGuiWindow * first = ImGui::FindWindowByName(FileBrowser::mainWindowName());
  for (std::size_t k=1; k<_browsers.size(); )
    {
    FileBrowser & browser = *_browsers[k];
    if (first && first->DockId) ImGui::SetNextWindowDockID(first->DockId, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(380, viewport->WorkSize.y*0.75f), ImGuiCond_FirstUseEver);
    bool open = true;
    if (ImGui::Begin(browser.windowName(), &open))
      {
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) _activeBrowser = int(k);
      browser.render();
      }
    ImGui::End();
    if (open) { k++; continue; }
    if (onCloseBrowser) onCloseBrowser(browser);
    _browsers.erase(_browsers.begin() + long(k));
    if (_activeBrowser >= int(k)) _activeBrowser = 0;
    }
  }

  void CapGui::renderCanvases(String & hoverText)
  {
  // panes added since the last frame: docked beside the pane that was current, splitting its area
  ImGuiID dockspace = ImHashStr("CapDockSpace");
  for (const auto & request : _dockBeside)
    {
    ImGuiWindow * target = request.second.empty() ? nullptr : ImGui::FindWindowByName(request.second.c_str());
    ImGuiDockNode * node = target ? target->DockNode : nullptr;
    if (!node) node = ImGui::DockBuilderGetCentralNode(dockspace);
    if (!node) continue;
    ImGuiID atDir = 0, other = 0;
    ImGuiDir direction = node->Size.x >= node->Size.y ? ImGuiDir_Right : ImGuiDir_Down;
    ImGui::DockBuilderSplitNode(node->ID, direction, 0.5f, &atDir, &other);
    ImGui::DockBuilderDockWindow(request.first.c_str(), atDir);
    ImGui::DockBuilderFinish(dockspace);
    }
  _dockBeside.clear();

  float areaX0 = 1e9f, areaY0 = 1e9f, areaX1 = -1e9f, areaY1 = -1e9f;
  for (int k=0; k<nPanes(); k++)
    {
    CanvasPane & pane = *_panes[k];
    bool open = true;
    if (_focusRequested && k == _currentPane) ImGui::SetNextWindowFocus();
    // a close button when there are other canvases, or when the canvas was moved out of the main window (closing the
    // last canvas then puts an empty one in the main window, see closePane())
    ImGuiWindow * window = ImGui::FindWindowByName(pane.name().data());
    bool outside = window && window->Viewport && window->Viewport != ImGui::GetMainViewport();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    bool visible = ImGui::Begin(pane.name().data(), (nPanes() > 1 || outside) ? &open : nullptr,
                                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);
    ImGui::PopStyleVar();
    if (visible)
      {
      // the pane whose window gets the focus (click, tab) becomes current
      bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
      if (focused && _focusedPane != k) { _currentPane = k; _focusedPane = k; }
      ImVec2 size = ImGui::GetContentRegionAvail();
      bool clicked = pane.render(size.x, size.y, k == _currentPane, _font);
      if (clicked) _currentPane = k;
      if (!pane.hoverText().isNull()) hoverText = pane.hoverText();
      float x, y, w, h;
      pane.canvasRect(x, y, w, h);
      areaX0 = std::min(areaX0, x); areaY0 = std::min(areaY0, y);
      areaX1 = std::max(areaX1, x + w); areaY1 = std::max(areaY1, y + h);
      }
    ImGui::End();
    if (!open) _closeRequests.push_back(k);
    }
  _focusRequested = false;
  _hoverText = hoverText;

  // data panes: tables of data points, closed with the x of their tab
  for (std::size_t k=0; k<_dataPanes.size(); )
    {
    DataPane & pane = *_dataPanes[k];
    bool open = true;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
    bool visible = ImGui::Begin(pane.windowTitle().data(), &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();
    if (visible) pane.render();
    ImGui::End();
    if (open) k++;
    else _dataPanes.erase(_dataPanes.begin() + k);
    }
  renderFitPane();
  if (_fitDialog.render(_panes.empty() ? nullptr : &currentPane().currentGraph())) setFitPaneVisible(true);
  if (areaX1 > areaX0) { _areaX = areaX0; _areaY = areaY0; _areaWidth = areaX1 - areaX0; _areaHeight = areaY1 - areaY0; }

  // panes added (+ button) or closed (x of their tab) by the user
  if (_addRequested)
    {
    if (beforePanesChange) beforePanesChange();
    addPane();
    _addRequested = false;
    }
  if (!_closeRequests.empty())
    {
    if (beforePanesChange) beforePanesChange();
    std::sort(_closeRequests.rbegin(), _closeRequests.rend());
    for (int k : _closeRequests) closePane(k);
    _closeRequests.clear();
    }
  }

  void CapGui::renderStatusBar(const MenuItem * hovered, const String & /*hoverText*/)
  {
  // left: menu item description, or the last status message (10 s), or a hint
  if (hovered && !hovered->description().isNull())
    ImGui::TextUnformatted(hovered->description().data());
  else if (!_status.isNull() && ImGui::GetTime() - _statusTime < 10.0)
    ImGui::TextUnformatted(_status.data());
  else
    ImGui::TextDisabled("Drag to zoom (double-click: automatic ranges); drag titles and legends to move them; right-click to edit");
  // right: coordinates under the mouse, current canvas
  char right[160];
  std::snprintf(right, sizeof(right), "%s    %s  (%d canvas%s)", _hoverText.data(),
                _panes.empty() ? "" : currentPane().name().data(), nPanes(), nPanes() > 1 ? "es" : "");
  float textWidth = ImGui::CalcTextSize(right).x;
  ImGui::SameLine(ImGui::GetWindowWidth() - textWidth - 12.0f);
  ImGui::TextUnformatted(right);
  }

  void CapGui::renderMessage()
  {
  if (_messageRequested) { ImGui::OpenPopup("##message"); _messageRequested = false; }
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("##message", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
    {
    ImGui::TextUnformatted(_messageTitle.data());
    ImGui::Separator();
    ImGui::PushTextWrapPos(ImGui::GetFontSize()*35.0f);
    ImGui::TextUnformatted(_messageText.data());
    ImGui::PopTextWrapPos();
    if (ImGui::Button("OK") || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape))
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    }
  }

  bool CapGui::savePageSvg(const String & fileName)
  {
  if (_areaWidth <= 0 || _areaHeight <= 0) return false;
  GRAPHICS::SvgPainter svg(_areaWidth, _areaHeight);
  for (auto & pane : _panes)        // typeset the LaTeX texts of all the canvases in one run
    {
    float x, y, w, h;
    pane->canvasRect(x, y, w, h);
    if (w > 0 && h > 0) GRAPHICS::LatexRenderer::instance().prepare(pane->canvas(), w, h);
    }
  for (auto & pane : _panes)
    {
    float x, y, w, h;
    pane->canvasRect(x, y, w, h);
    if (w <= 0 || h <= 0) continue;
    svg.beginRegion(x - _areaX, y - _areaY, w, h);
    pane->canvas().draw(svg);
    }
  svg.endRegion();
  return svg.write(fileName);
  }

  } // namespace GUI
} // namespace CAP
