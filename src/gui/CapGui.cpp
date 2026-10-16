#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include "CapGui.hpp"
#include "SvgPainter.hpp"

#ifndef CAP_SOURCE_ROOT
#define CAP_SOURCE_ROOT ""
#endif

namespace CAP
{
  namespace GUI
  {

  CapGui::CapGui()
  :
  onNewPane(),
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
  _browser(),
  _firstFrame(true),
  _layoutRequested(false),
  _undockRequested(false)
  { }

  CapGui::~CapGui()
  {
  if (_window)
    {
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

  //! First existing file among the candidates, or "".
  std::string firstExisting(const std::vector<std::string> & candidates)
  {
  std::error_code ec;
  for (const auto & c : candidates) if (!c.empty() && std::filesystem::is_regular_file(c, ec)) return c;
  return std::string();
  }
  } // namespace

  bool CapGui::initialize(const GuiSettings & settings)
  {
  _settings = settings;
  glfwSetErrorCallback(glfwError);
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
  setLayout(_settings.columns, _settings.rows);
  return true;
  }

  void CapGui::loadFonts()
  {
  ImGuiIO & io = ImGui::GetIO();
  std::vector<std::string> candidates;
  candidates.push_back(_settings.fontFile.str());
  const char * root = std::getenv("CAP_ROOT_PATH");
  if (root) candidates.push_back(std::string(root) + "/external/imgui/misc/fonts/Roboto-Medium.ttf");
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
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  void CapGui::setLayout(int nColumns, int nRows)
  {
  nColumns = nColumns < 1 ? 1 : (nColumns > 6 ? 6 : nColumns);
  nRows    = nRows    < 1 ? 1 : (nRows    > 6 ? 6 : nRows);
  size_t n = size_t(nColumns*nRows);
  while (_panes.size() > n) _panes.pop_back();
  while (_panes.size() < n)
    {
    char name[32];
    std::snprintf(name, sizeof(name), "Canvas %d", ++_paneCounter);
    _panes.emplace_back(new CanvasPane(name));
    _panes.back()->setShowToolbar(_settings.toolbars);
    if (onNewPane) onNewPane(*_panes.back());
    }
  _columns = nColumns;
  _rows    = nRows;
  if (_currentPane >= nPanes()) _currentPane = 0;
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
    // the layout saved in the ini file is kept if it agrees with the settings
    ImGuiWindowSettings * saved = ImGui::FindWindowSettingsByID(ImHashStr(FileBrowser::windowName()));
    bool savedDocked = saved && saved->DockId != 0;
    if (!ImGui::DockBuilderGetNode(dockspace)) _layoutRequested = true;
    if (_settings.browserDocked && !savedDocked) _layoutRequested = true;
    if (!_settings.browserDocked && savedDocked) _undockRequested = true;
    _firstFrame = false;
    }
  if (_layoutRequested) { buildDockLayout(dockspace); _layoutRequested = false; }
  ImGui::DockSpaceOverViewport(dockspace, viewport, ImGuiDockNodeFlags_NoDockingOverCentralNode);
  ImGuiDockNode * central = ImGui::DockBuilderGetCentralNode(dockspace);

  ImGuiWindowClass canvasClass;
  canvasClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoUndocking;
  ImGui::SetNextWindowClass(&canvasClass);
  ImGui::SetNextWindowDockID(central ? central->ID : dockspace, ImGuiCond_Always);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
  if (ImGui::Begin("Canvases", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse))
    renderCanvases(hoverText);
  ImGui::End();
  ImGui::PopStyleVar();

  renderBrowser();
  _fileDialog.render();
  renderMessage();
  if (onFrame) onFrame();

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
    ImGui::DockBuilderDockWindow(FileBrowser::windowName(), left);
    }
  ImGui::DockBuilderDockWindow("Canvases", canvases);
  ImGui::DockBuilderFinish(dockId);
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
  if (ImGui::Begin(FileBrowser::windowName(), &visible))
    {
    // follow what the user did with the mouse (docking or undocking the window)
    if (!_layoutRequested) _settings.browserDocked = ImGui::IsWindowDocked();
    _browser.render();
    }
  else if (!_layoutRequested) _settings.browserDocked = ImGui::IsWindowDocked();
  ImGui::End();
  _settings.browserVisible = visible;
  }

  void CapGui::renderCanvases(String & hoverText)
  {
  ImVec2 origin = ImGui::GetCursorScreenPos();
  ImVec2 available = ImGui::GetContentRegionAvail();
  _areaX = origin.x; _areaY = origin.y; _areaWidth = available.x; _areaHeight = available.y;
  float spacing = 4.0f;
  float w = (available.x - spacing*(_columns-1))/_columns;
  float h = (available.y - spacing*(_rows-1))/_rows;
  if (w < 40 || h < 40) return;
  for (int row=0; row<_rows; row++)
    for (int column=0; column<_columns; column++)
      {
      int k = row*_columns + column;
      CanvasPane & pane = *_panes[k];
      ImGui::SetCursorScreenPos(ImVec2(origin.x + column*(w + spacing), origin.y + row*(h + spacing)));
      bool current = (k == _currentPane);
      if (current) ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.18f, 0.43f, 0.85f, 1.0f));
      ImGui::BeginChild(pane.name().data(), ImVec2(w, h), ImGuiChildFlags_Borders,
                        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
      if (current) ImGui::PopStyleColor();
      bool clicked = pane.render(w, h, current, _font);
      if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
        clicked = true;
      if (clicked) _currentPane = k;
      if (!pane.hoverText().isNull()) hoverText = pane.hoverText();
      ImGui::EndChild();
      }
  _hoverText = hoverText;
  }

  void CapGui::renderStatusBar(const MenuItem * hovered, const String & /*hoverText*/)
  {
  // left: menu item description, or the last status message (10 s), or a hint
  if (hovered && !hovered->description().isNull())
    ImGui::TextUnformatted(hovered->description().data());
  else if (!_status.isNull() && ImGui::GetTime() - _statusTime < 10.0)
    ImGui::TextUnformatted(_status.data());
  else
    ImGui::TextDisabled("Click a graph to select it; right-click for its menu");
  // right: coordinates under the mouse, current canvas
  char right[160];
  std::snprintf(right, sizeof(right), "%s    %s  (%d x %d)", _hoverText.data(),
                _panes.empty() ? "" : currentPane().name().data(), _columns, _rows);
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
