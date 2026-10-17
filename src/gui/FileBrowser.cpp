#include <algorithm>
#include <cctype>
#include <cstdio>
#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include "FileBrowser.hpp"
#include "FileSystem.hpp"
#include "CapFile.hpp"
#include "Folder.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "DataPoints.hpp"


namespace CAP
{
  namespace GUI
  {

  namespace
  {
  //! Whether path is directory or below it.
  bool isBelow(const std::string & path, const std::string & directory)
  {
  if (directory.empty()) return false;
  std::string d = directory;
  if (d.back() != '/') d += '/';
  return path.compare(0, d.size(), d) == 0;
  }


  //! "CAP::MATH::Matrix" -> "Matrix".
  String shortTypeName(const std::string & typeName)
  {
  size_t colon = typeName.rfind("::");
  return String(colon == std::string::npos ? typeName : typeName.substr(colon+2));
  }

  //! Path of the object currently pressed in the tree (to tell a click from the start of a drag).
  std::string pressedObject;
  } // namespace

  FileBrowser::FileBrowser()
  :
  onOpenFile(),
  onShowObject(),
  onShowFolder(),
  onCloseFile(),
  onReloadFile(),
  onCommand(),
  _files(nullptr),
  _folders(),
  _listings(),
  _openFailed(),
  _reveal(),
  _filter(),
  _selected()
  { }

  bool FileBrowser::decodePayload(const void * data, int size, ObjectReference & reference)
  {
  if (!data || size < 3) return false;
  std::string s(static_cast<const char *>(data), size_t(size));
  if (!s.empty() && s.back() == '\0') s.pop_back();
  size_t newline = s.find('\n');
  if (newline == std::string::npos) return false;
  try { reference.file = std::stoi(s.substr(0, newline)); }
  catch (...) { return false; }
  reference.path = String(s.substr(newline+1));
  return true;
  }

  void FileBrowser::addFolder(const String & folder)
  {
  std::string f = folder.str();
  while (f.size() > 1 && f.back() == '/') f.pop_back();
  for (const auto & existing : _folders) if (existing.str() == f) return;
  _folders.push_back(String(f));
  }

  void FileBrowser::removeFolder(const String & folder)
  {
  _folders.erase(std::remove_if(_folders.begin(), _folders.end(), [&](const String & f) { return f.str() == folder.str(); }),
                 _folders.end());
  }

  void FileBrowser::reveal(const String & path)
  {
  _reveal = path.str();
  }

  const BrowserFile * FileBrowser::fileByPath(const std::string & path) const
  {
  if (!_files) return nullptr;
  for (const auto & f : *_files) if (f.path.str() == path) return &f;
  return nullptr;
  }

  const BrowserFile * FileBrowser::fileById(int id) const
  {
  if (!_files) return nullptr;
  for (const auto & f : *_files) if (f.id == id) return &f;
  return nullptr;
  }

  bool FileBrowser::insideFolders(const std::string & path) const
  {
  for (const auto & folder : _folders) if (isBelow(path, folder.str())) return true;
  return false;
  }

  const FileBrowser::Listing & FileBrowser::listing(const std::string & directory)
  {
  auto found = _listings.find(directory);
  if (found != _listings.end()) return found->second;
  Listing & l = _listings[directory];
  // sub-directories and CAP files, directories first, by name (case-insensitive); hidden entries left out
  FileSystem::ListOptions options;
  options.extensions = { ".cap" };
  try
    {
    for (const auto & entry : FileSystem::list(String(directory), options))
      (entry.isDirectory() ? l.directories : l.files).push_back(entry.name.str());
    }
  catch (const FileException & e) { l.error = e.what(); }
  return l;
  }

  bool FileBrowser::matches(const String & name) const
  {
  return _filter.empty() || name.contains(_filter.c_str(), true);
  }

  bool FileBrowser::folderMatches(const Folder & folder) const
  {
  if (_filter.empty()) return true;
  for (const auto & name : folder.names())
    {
    if (matches(name)) return true;
    if (const Folder * sub = folder.folder(name)) if (folderMatches(*sub)) return true;
    }
  return false;
  }

  bool FileBrowser::describe(const Folder & folder, const String & name, String & kind, String & details)
  {
  char text[256];
  if (const MATH::Histogram * h = folder.get<MATH::Histogram>(name))
    {
    unsigned int rank = h->rank();
    bool profile = dynamic_cast<const MATH::Profile *>(h) != nullptr;
    std::snprintf(text, sizeof(text), "%s%u", profile ? "P" : "H", rank);
    kind = text;
    details = h->title().isNull() ? name : h->title();
    for (unsigned int k=0; k<rank; k++)
      {
      std::snprintf(text, sizeof(text), "\naxis %u: %u bins in [%g, %g]%s%s", k, h->nBins(k), h->minimum(k), h->maximum(k),
                    h->dimensionTitle(k).isNull() ? "" : "  ", h->dimensionTitle(k).data());
      details += text;
      }
    std::snprintf(text, sizeof(text), "\nentries: %.10g", h->totalEntries());
    details += text;
    if (rank != 1) details += "\n(2D and 3D graphs: stage 5)";
    return rank == 1;
    }
  if (const MATH::DataPoints * d = folder.get<MATH::DataPoints>(name))
    {
    kind = "Points";
    std::snprintf(text, sizeof(text), "%s\n%u points", d->title().isNull() ? name.data() : d->title().data(), d->size());
    details = text;
    return d->rank() == 1;
    }
  kind = shortTypeName(folder.typeName(name));
  details = String(folder.typeName(name));
  details += "\n(cannot be shown in a graph)";
  return false;
  }

  void FileBrowser::renderToolbar()
  {
  float buttons = ImGui::CalcTextSize("Open...Folder...Refresh").x + 6*ImGui::GetStyle().FramePadding.x
                  + 3*ImGui::GetStyle().ItemSpacing.x;
  ImGui::SetNextItemWidth(std::max(80.0f, ImGui::GetContentRegionAvail().x - buttons));
  ImGui::InputTextWithHint("##filter", "Filter objects", &_filter);
  ImGui::SetItemTooltip("Show the objects whose name contains this text");
  ImGui::SameLine();
  if (ImGui::Button("Open...") && onCommand) onCommand("file.open");
  ImGui::SetItemTooltip("Open a CAP file");
  ImGui::SameLine();
  if (ImGui::Button("Folder...") && onCommand) onCommand("file.openFolder");
  ImGui::SetItemTooltip("Browse a folder of the file system");
  ImGui::SameLine();
  if (ImGui::Button("Refresh")) { refresh(); _openFailed.clear(); }
  ImGui::SetItemTooltip("Read the folders again");
  }

  void FileBrowser::render()
  {
  renderToolbar();
  ImGui::Separator();
  ImGui::BeginChild("##tree", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

  // files outside the folders
  std::vector<BrowserFile> loose;                     // copies: a file may be closed while the list is drawn
  if (_files) for (const auto & f : *_files) if (!insideFolders(f.path.str())) loose.push_back(f);

  if (loose.empty() && _folders.empty())
    {
    ImGui::Spacing();
    ImGui::TextWrapped("Use File > Open to open a CAP file, or File > Open Folder to browse a folder.");
    ImGui::Spacing();
    ImGui::TextWrapped("Click an object to show it in the selected graph, or drag it onto a graph.");
    }

  if (!loose.empty())
    {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Files"))
      for (const BrowserFile & f : loose) renderFile(f, FileSystem::name(f.path));
    }
  if (!_folders.empty())
    {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Folders"))
      {
      std::vector<String> folders = _folders;          // the list may change from a context menu
      for (const auto & folder : folders) renderDirectory(folder.str(), folder.str(), true);
      }
    }
  ImGui::EndChild();
  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) pressedObject.clear();
  }

  void FileBrowser::renderFile(const BrowserFile & file, const String & label)
  {
  ImGui::PushID(file.id);
  bool reveal = (_reveal == file.path.str());
  if (reveal) ImGui::SetNextItemOpen(true);
  else        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  bool open = ImGui::TreeNodeEx("##file", ImGuiTreeNodeFlags_SpanAvailWidth, "%s", label.data());
  if (reveal) { ImGui::SetScrollHereY(0.2f); _reveal.clear(); }
  ImGui::SetItemTooltip("%s", file.path.data());
  ImGuiID nodeId = ImGui::GetItemID();
  bool close = false;
  if (ImGui::BeginPopupContextItem("##fileMenu"))
    {
    if (ImGui::MenuItem("Show All")    && onShowFolder) onShowFolder(file.id, "");
    ImGui::Separator();
    if (ImGui::MenuItem("Reload")      && onReloadFile) onReloadFile(file.id);
    if (ImGui::MenuItem("Close File")) close = true;
    ImGui::EndPopup();
    }
  if (close)
    {
    ImGui::TreeNodeSetOpen(nodeId, false);           // in the storage of this window, not of the popup
    if (open) ImGui::TreePop();
    ImGui::PopID();
    if (onCloseFile) onCloseFile(file.id);           // file is no longer valid after this
    return;
    }
  if (open)
    {
    if (fileById(file.id) && file.file) renderFolder(file.id, *file.file, "");
    ImGui::TreePop();
    }
  ImGui::PopID();
  }

  void FileBrowser::renderDirectory(const std::string & directory, const std::string & label, bool isRoot)
  {
  ImGui::PushID(directory.c_str());
  if (isRoot) ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  if (!_reveal.empty() && isBelow(_reveal, directory)) ImGui::SetNextItemOpen(true);
  bool open = ImGui::TreeNodeEx("##dir", ImGuiTreeNodeFlags_SpanAvailWidth, "%s%s", label.c_str(), isRoot ? "" : "/");
  ImGui::SetItemTooltip("%s", directory.c_str());
  if (ImGui::BeginPopupContextItem("##dirMenu"))
    {
    if (ImGui::MenuItem("Refresh")) { _listings.erase(directory); _openFailed.clear(); }
    if (isRoot && ImGui::MenuItem("Remove from Browser")) removeFolder(String(directory));
    ImGui::EndPopup();
    }
  if (open)
    {
    const Listing & l = listing(directory);
    if (!l.error.empty()) ImGui::TextDisabled("%s", l.error.c_str());
    else if (l.directories.empty() && l.files.empty()) ImGui::TextDisabled("(no CAP files)");
    Listing copy = l;                                   // the cache may change while rendering the children
    for (const auto & d : copy.directories) renderDirectory(directory + "/" + d, d, false);
    for (const auto & f : copy.files)       renderCapFileInTree(directory + "/" + f, f);
    ImGui::TreePop();
    }
  ImGui::PopID();
  }

  void FileBrowser::renderCapFileInTree(const std::string & path, const std::string & label)
  {
  const BrowserFile * file = fileByPath(path);
  ImGui::PushID(path.c_str());
  bool reveal = (_reveal == path);
  if (reveal) ImGui::SetNextItemOpen(true);
  bool loaded = (file != nullptr);
  if (loaded) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Text));
  else        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
  bool open = ImGui::TreeNodeEx("##capfile", ImGuiTreeNodeFlags_SpanAvailWidth, "%s", label.c_str());
  ImGui::PopStyleColor();
  if (reveal) { ImGui::SetScrollHereY(0.2f); _reveal.clear(); }
  ImGuiID nodeId = ImGui::GetItemID();
  ImGui::SetItemTooltip("%s%s", path.c_str(), loaded ? "" : "\nclick to open");
  if (ImGui::IsItemToggledOpen() && open && !file && onOpenFile)
    {
    int id = onOpenFile(String(path));
    if (id < 0) _openFailed.insert(path); else _openFailed.erase(path);
    file = fileByPath(path);
    }
  bool close = false;
  if (file && ImGui::BeginPopupContextItem("##fileMenu"))
    {
    if (ImGui::MenuItem("Show All")   && onShowFolder) onShowFolder(file->id, "");
    ImGui::Separator();
    if (ImGui::MenuItem("Reload")     && onReloadFile) onReloadFile(file->id);
    if (ImGui::MenuItem("Close File")) close = true;
    ImGui::EndPopup();
    }
  if (close)
    {
    ImGui::TreeNodeSetOpen(nodeId, false);           // in the storage of this window, not of the popup
    if (onCloseFile) onCloseFile(file->id);
    if (open) ImGui::TreePop();
    ImGui::PopID();
    return;
    }
  if (open)
    {
    file = fileByPath(path);
    if (file && file->file) renderFolder(file->id, *file->file, "");
    else if (_openFailed.count(path)) ImGui::TextDisabled("(cannot be read)");
    else ImGui::TextDisabled("(not open)");
    ImGui::TreePop();
    }
  ImGui::PopID();
  }

  void FileBrowser::renderFolder(int fileId, const Folder & folder, const std::string & prefix)
  {
  std::vector<String> names = folder.names();
  if (names.empty()) { ImGui::TextDisabled("(empty)"); return; }
  for (const auto & name : names)
    {
    std::string path = prefix.empty() ? name.str() : prefix + "/" + name.str();
    if (const Folder * sub = folder.folder(name))
      {
      if (!folderMatches(*sub) && !matches(name)) continue;
      ImGui::PushID(name.data());
      if (!_filter.empty()) ImGui::SetNextItemOpen(true);
      bool open = ImGui::TreeNodeEx("##folder", ImGuiTreeNodeFlags_SpanAvailWidth, "%s/", name.data());
      if (ImGui::BeginPopupContextItem("##folderMenu"))
        {
        if (ImGui::MenuItem("Show All") && onShowFolder) onShowFolder(fileId, String(path));
        ImGui::EndPopup();
        }
      ImGui::SetItemTooltip("%u entries; right-click: show all", sub->size());
      if (open) { renderFolder(fileId, *sub, path); ImGui::TreePop(); }
      ImGui::PopID();
      }
    else if (matches(name))
      renderObject(fileId, folder, name, path);
    }
  }

  void FileBrowser::renderObject(int fileId, const Folder & folder, const String & name, const std::string & path)
  {
  String kind, details;
  bool showable = describe(folder, name, kind, details);
  ImGui::PushID(name.data());
  bool selected = (_selected.file == fileId && _selected.path.str() == path);
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
  if (selected) flags |= ImGuiTreeNodeFlags_Selected;
  if (!showable) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
  ImGui::TreeNodeEx("##object", flags, "%s", name.data());
  if (!showable) ImGui::PopStyleColor();

  // click (press and release on the object without dragging): show it
  std::string key = std::to_string(fileId) + "\n" + path;
  if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) pressedObject = key;
  if (showable && pressedObject == key && ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)
      && ImGui::GetIO().MouseDragMaxDistanceSqr[0] < 16.0f)
    {
    _selected.file = fileId;
    _selected.path = String(path);
    if (onShowObject) onShowObject(_selected);
    }

  // drag onto a graph
  if (showable && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
    ImGui::SetDragDropPayload(payloadType(), key.c_str(), key.size()+1);
    ImGui::Text("%s", name.data());
    ImGui::SameLine();
    ImGui::TextDisabled("%s", kind.data());
    ImGui::EndDragDropSource();
    }
  else if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("%s", details.data());

  if (ImGui::BeginPopupContextItem("##objectMenu"))
    {
    if (ImGui::MenuItem("Show", nullptr, false, showable) && onShowObject)
      {
      _selected.file = fileId;
      _selected.path = String(path);
      onShowObject(_selected);
      }
    ImGui::EndPopup();
    }
  ImGui::SameLine();
  ImGui::TextDisabled("%s", kind.data());
  ImGui::PopID();
  }

  } // namespace GUI
} // namespace CAP
