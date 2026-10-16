#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include "imgui.h"
#include "FileDialog.hpp"

namespace fs = std::filesystem;

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
  std::error_code ec;
  _directory = fs::current_path(ec).string();
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
  std::error_code ec;
  std::string start = directory.str();
  if (start.empty() || !fs::is_directory(start, ec)) start = _directory;
  if (start.empty() || !fs::is_directory(start, ec)) start = fs::current_path(ec).string();
  enter(start);
  }

  bool FileDialog::accepts(const std::string & name) const
  {
  if (_extensions.empty()) return true;
  for (const auto & e : _extensions)
    if (name.size() >= e.size() && name.compare(name.size()-e.size(), e.size(), e) == 0) return true;
  return false;
  }

  void FileDialog::enter(const std::string & directory)
  {
  std::error_code ec;
  fs::path p = fs::weakly_canonical(fs::path(directory), ec);
  if (ec || !fs::is_directory(p, ec)) { _error = "Not a folder: " + directory; return; }
  _directory = p.string();
  std::snprintf(_pathField, sizeof(_pathField), "%s", _directory.c_str());
  _selected = -1;
  _error.clear();
  list();
  }

  void FileDialog::list()
  {
  _entries.clear();
  std::error_code ec;
  for (fs::directory_iterator it(_directory, fs::directory_options::skip_permission_denied, ec), end; !ec && it != end; it.increment(ec))
    {
    std::string name = it->path().filename().string();
    if (!_showHidden && !name.empty() && name[0] == '.') continue;
    std::error_code ec2;
    bool folder = it->is_directory(ec2);
    if (!folder && (_mode == kOpenFolder || !accepts(name))) continue;
    std::uintmax_t size = folder ? 0 : it->file_size(ec2);
    _entries.push_back({name, folder, ec2 ? 0 : size});
    }
  std::sort(_entries.begin(), _entries.end(), [](const Entry & a, const Entry & b)
    {
    if (a.folder != b.folder) return a.folder;
    return a.name < b.name;
    });
  if (ec) _error = "Cannot list " + _directory;
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
  if (ImGui::Button("Up")) enter(fs::path(_directory).parent_path().string());
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
    std::error_code ec;
    if (fs::is_directory(_pathField, ec)) enter(_pathField);
    else if (_mode == kOpenFile && fs::is_regular_file(_pathField, ec)) { confirm(_pathField); ImGui::EndPopup(); return; }
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
          if (e.folder) enterFolder = (fs::path(_directory)/e.name).string();
          else          chosen      = (fs::path(_directory)/e.name).string();
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
    if (!chosen.empty() && _mode == kSaveFile) std::snprintf(_fileName, sizeof(_fileName), "%s", fs::path(chosen).filename().string().c_str());
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
        chosen = (fs::path(_directory)/_entries[_selected].name).string();
      confirm(chosen);
      }
    else if (_mode == kOpenFile)
      {
      if (_selected >= 0 && _selected < int(_entries.size()))
        {
        const Entry & e = _entries[_selected];
        if (e.folder) enter((fs::path(_directory)/e.name).string());
        else confirm((fs::path(_directory)/e.name).string());
        }
      else _error = "Select a file";
      }
    else
      {
      if (std::strlen(_fileName) == 0) _error = "Enter a file name";
      else confirm((fs::path(_directory)/_fileName).string());
      }
    }
  if (!keepOpen) _open = false;
  ImGui::EndPopup();
  }

  } // namespace GUI
} // namespace CAP
