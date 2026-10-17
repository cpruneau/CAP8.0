#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "imgui.h"
#include "FileDialog.hpp"
#include "FileSystem.hpp"


namespace CAP
{
  namespace GUI
  {

  FileDialog::FileDialog()
  :
  _mode(kOpenFile),
  _title("Open"),
  _directory(),
  _extensions(),
  _callback(),
  _open(false),
  _requested(false),
  _showHidden(false),
  _selected(-1),
  _entries(),
  _error()
  {
  _fileName[0] = 0;
  _pathField[0] = 0;
  _directory = FileSystem::current().str();
  }

  void FileDialog::open(Mode mode, const String & title, const String & directory, const std::vector<String> & extensions,
                        const String & fileName, const Callback & callback)
  {
  _mode = mode;
  _title = title.str();
  _extensions.clear();
  for (const auto & e : extensions) _extensions.push_back(e.str());
  std::snprintf(_fileName, sizeof(_fileName), "%s", fileName.data());
  _callback = callback;
  _requested = true;
  std::string start = directory.str();
  if (!FileSystem::isDirectory(String(start))) start = _directory;
  if (!FileSystem::isDirectory(String(start))) start = FileSystem::current().str();
  enter(start);
  }

  void FileDialog::enter(const std::string & directory)
  {
  if (!FileSystem::isDirectory(String(directory))) { _error = "Not a folder: " + directory; return; }
  _directory = FileSystem::normalize(String(directory)).str();
  std::snprintf(_pathField, sizeof(_pathField), "%s", _directory.c_str());
  _selected = -1;
  _error.clear();
  list();
  }

  void FileDialog::list()
  {
  _entries.clear();
  FileSystem::ListOptions options;
  options.hidden = _showHidden;
  options.files  = (_mode != kOpenFolder);
  for (const auto & e : _extensions) options.extensions.push_back(String(e));
  try
    {
    for (const auto & entry : FileSystem::list(String(_directory), options))
      _entries.push_back({entry.name.str(), entry.isDirectory(), entry.size});
    }
  catch (const FileException &) { _error = "Cannot list " + _directory; }
  }

  void FileDialog::confirm(const std::string & path)
  {
  _open = false;
  ImGui::CloseCurrentPopup();
  if (_callback) _callback(String(path));
  }

  void FileDialog::render()
  {
  if (_requested)
    {
    ImGui::OpenPopup(_title.c_str());
    _requested = false;
    _open = true;
    }
  if (!_open) return;
  ImGuiViewport * viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowSize(ImVec2(std::min(760.0f, viewport->WorkSize.x*0.9f), std::min(520.0f, viewport->WorkSize.y*0.9f)), ImGuiCond_Appearing);
  ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  bool keepOpen = true;
  if (!ImGui::BeginPopupModal(_title.c_str(), &keepOpen))
    {
    _open = ImGui::IsPopupOpen(_title.c_str());
    return;
    }

  // folder path: buttons to go up, editable path
  if (ImGui::Button("Up")) enter(FileSystem::parent(String(_directory)).str());
  ImGui::SameLine();
  if (ImGui::Button("Home"))
    {
    const char * home = std::getenv("HOME");
    if (home) enter(home);
    }
  ImGui::SameLine();
  ImGui::SetNextItemWidth(-130);
  if (ImGui::InputText("##path", _pathField, sizeof(_pathField), ImGuiInputTextFlags_EnterReturnsTrue))
    {
      if (FileSystem::isDirectory(String(_pathField))) enter(_pathField);
    else if (_mode == kOpenFile && FileSystem::isFile(String(_pathField))) { confirm(_pathField); ImGui::EndPopup(); return; }
    else _error = std::string("No such folder: ") + _pathField;
    }
  ImGui::SameLine();
  if (ImGui::Checkbox("Hidden", &_showHidden)) list();

  // entries
  float footer = ImGui::GetFrameHeightWithSpacing()*(_mode == kSaveFile ? 2.2f : 1.2f) + (_error.empty() ? 0 : ImGui::GetTextLineHeightWithSpacing());
  if (ImGui::BeginTable("entries", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter,
                        ImVec2(0, -footer)))
    {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 90.0f);
    ImGui::TableHeadersRow();
    std::string enterFolder, chosen;
    for (int k=0; k<int(_entries.size()); k++)
      {
      const Entry & e = _entries[k];
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      std::string label = (e.folder ? "[folder]  " : "") + e.name;
      if (ImGui::Selectable(label.c_str(), _selected == k, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
        {
        _selected = k;
        if (!e.folder && _mode == kSaveFile) std::snprintf(_fileName, sizeof(_fileName), "%s", e.name.c_str());
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
          {
          if (e.folder) enterFolder = FileSystem::join(String(_directory), String(e.name)).str();
          else          chosen      = FileSystem::join(String(_directory), String(e.name)).str();
          }
        }
      ImGui::TableNextColumn();
      if (!e.folder)
        {
        double kb = e.size/1024.0;
        if (kb < 1024) ImGui::Text("%.1f kB", kb); else ImGui::Text("%.1f MB", kb/1024.0);
        }
      }
    ImGui::EndTable();
    if (!enterFolder.empty()) enter(enterFolder);
    if (!chosen.empty() && _mode != kSaveFile) { confirm(chosen); ImGui::EndPopup(); return; }
    if (!chosen.empty() && _mode == kSaveFile) std::snprintf(_fileName, sizeof(_fileName), "%s", FileSystem::name(String(chosen)).data());
    }
  if (!_error.empty()) ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.2f, 1.0f), "%s", _error.c_str());

  // file name (save) and buttons
  bool accept = false;
  if (_mode == kSaveFile)
    {
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputTextWithHint("##name", "file name", _fileName, sizeof(_fileName), ImGuiInputTextFlags_EnterReturnsTrue)) accept = true;
    }
  const char * okLabel = _mode == kSaveFile ? "Save" : (_mode == kOpenFolder ? "Choose this folder" : "Open");
  if (ImGui::Button(okLabel)) accept = true;
  ImGui::SameLine();
  if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) { _open = false; ImGui::CloseCurrentPopup(); ImGui::EndPopup(); return; }
  if (accept)
    {
    if (_mode == kOpenFolder)
      {
      std::string chosen = _directory;
      if (_selected >= 0 && _selected < int(_entries.size()) && _entries[_selected].folder)
        chosen = FileSystem::join(String(_directory), String(_entries[_selected].name)).str();
      confirm(chosen);
      }
    else if (_mode == kOpenFile)
      {
      if (_selected >= 0 && _selected < int(_entries.size()))
        {
        const Entry & e = _entries[_selected];
        if (e.folder) enter(FileSystem::join(String(_directory), String(e.name)).str());
        else confirm(FileSystem::join(String(_directory), String(e.name)).str());
        }
      else _error = "Select a file";
      }
    else
      {
      if (std::strlen(_fileName) == 0) _error = "Enter a file name";
      else confirm(FileSystem::join(String(_directory), String(_fileName)).str());
      }
    }
  if (!keepOpen) _open = false;
  ImGui::EndPopup();
  }

  } // namespace GUI
} // namespace CAP
