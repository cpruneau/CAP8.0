/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FileBrowser.cpp
//! @brief Implementation of FileBrowser.
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
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

  //! Whether an object not read yet (lazily opened file) can probably be shown in a graph, from its type: histograms,
  //! profiles and data points (whether it is 1D is known once it is read).
  bool showableType(const std::string & typeName)
  {
  for (const char * type : {"Histogram", "Profile", "DataPointsVector", "DataPointColumns"})
    {
    std::string t(type);
    if (typeName.size() >= t.size() && typeName.compare(typeName.size()-t.size(), t.size(), t) == 0) return true;
    }
  return false;
  }
  } // namespace

  FileBrowser::FileBrowser(int number)
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
  _selected(),
  _number(number),
  _windowName(number <= 1 ? std::string(mainWindowName()) : std::string(mainWindowName()) + " " + std::to_string(number)),
  _shown(),
  _pressed(),
  _nameRequest()
  { }

  void FileBrowser::showFile(int id)
  {
  if (!showsFile(id)) _shown.push_back(id);
  }

  void FileBrowser::hideFile(int id)
  {
  _shown.erase(std::remove(_shown.begin(), _shown.end(), id), _shown.end());
  }

  bool FileBrowser::showsFile(int id) const
  {
  return std::find(_shown.begin(), _shown.end(), id) != _shown.end();
  }

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
  // sub-directories and CAP files (and ROOT files when CapGui reads them), directories first, by name (case-insensitive); hidden entries left out
  FileSystem::ListOptions options;
#ifdef CAP_WITH_ROOT_FILES
  options.extensions = { ".cap", ".root" };
#else
  options.extensions = { ".cap" };
#endif
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
    if (rank == 2) details += "\n(shown as a color map)";
    if (rank > 2)  details += "\n(histograms of rank 3 and more: 3D views to come)";
    return rank <= 2;
    }
  if (const MATH::DataPoints * d = folder.get<MATH::DataPoints>(name))
    {
    kind = "Points";
    std::snprintf(text, sizeof(text), "%s\n%u points of rank %u%s", d->title().isNull() ? name.data() : d->title().data(), d->size(),
                  d->rank(), d->rank() == 2 ? " (shown as a color map)" : "");
    details = text;
    return d->rank() <= 2;
    }
  kind = shortTypeName(folder.typeName(name));
  details = String(folder.typeName(name));
  details += "\n(cannot be shown in a graph)";
  return false;
  }

  void FileBrowser::renderToolbar()
  {
  if (ImGui::Button("Open...") && onCommand) onCommand("file.open");
  ImGui::SetItemTooltip("Open a CAP file");
  ImGui::SameLine();
  if (ImGui::Button("Folder...") && onCommand) onCommand("file.openFolder");
  ImGui::SetItemTooltip("Browse a folder of the file system");
  ImGui::SameLine();
  if (ImGui::Button("New Tree") && onNewTree) onNewTree();
  ImGui::SetItemTooltip("Make a new, empty tree: add folders to it, drag objects and folders onto it (copies), then "
                        "save it to a CAP file");
  ImGui::SameLine();
  if (ImGui::Button("Refresh")) { refresh(); _openFailed.clear(); }
  ImGui::SetItemTooltip("Read the folders again");
  ImGui::SetNextItemWidth(-FLT_MIN);
  ImGui::InputTextWithHint("##filter", "Filter objects", &_filter);
  ImGui::SetItemTooltip("Show the objects whose name contains this text");
  }

  void FileBrowser::render()
  {
  renderToolbar();
  ImGui::Separator();
  ImGui::BeginChild("##tree", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

  // files outside the folders
  std::vector<BrowserFile> loose;                     // copies: a file may be closed while the list is drawn
  for (int id : _shown)
    if (const BrowserFile * f = fileById(id))
      if (f->editable || !insideFolders(f->path.str())) loose.push_back(*f);

  if (loose.empty() && _folders.empty())
    {
    ImGui::Spacing();
    ImGui::TextWrapped("Use File > Open to open a CAP file, File > Open Folder to browse a folder, or New Tree to "
                       "make a tree of copies.");
    ImGui::Spacing();
    ImGui::TextWrapped("Click an object to show it in the selected graph, or drag it onto a graph.");
    }

  if (!loose.empty())
    {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Files"))
      for (const BrowserFile & f : loose) renderFile(f, f.editable ? f.name : FileSystem::name(f.path));
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
  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) _pressed.clear();
  renderNameDialog();
  }

  void FileBrowser::renderFile(const BrowserFile & file, const String & label)
  {
  ImGui::PushID(file.id);
  bool reveal = (_reveal == file.path.str());
  if (reveal) ImGui::SetNextItemOpen(true);
  else        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  bool open = ImGui::TreeNodeEx("##file", ImGuiTreeNodeFlags_SpanAvailWidth, "%s%s", label.data(),
                                file.editable ? (file.path.isEmpty() ? "  (tree, not saved)" : "  (tree)") : "");
  if (reveal) { ImGui::SetScrollHereY(0.2f); _reveal.clear(); }
  ImGuiID nodeId = ImGui::GetItemID();
  dragAndDrop(file.id, "", label, true, file.editable);
  if (!ImGui::IsDragDropActive())
    ImGui::SetItemTooltip("%s", file.editable ? (file.path.isEmpty() ? "Tree not saved yet: drag objects and folders "
                                                 "onto it, right-click to add folders or save it" : file.path.data())
                                              : file.path.data());
  bool close = false;
  if (ImGui::BeginPopupContextItem("##fileMenu"))
    {
    if (ImGui::MenuItem("Show All")    && onShowFolder) onShowFolder(file.id, "");
    ImGui::Separator();
    if (file.editable)
      {
      editMenu(file.id, "", true);
      if (ImGui::MenuItem("Save Tree As...") && onSaveTree) onSaveTree(file.id);
      if (onExportRoot && ImGui::MenuItem("Export to ROOT...")) onExportRoot(file.id, "");
      if (ImGui::MenuItem("Close Tree")) close = true;
      }
    else
      {
      if (ImGui::MenuItem("Reload")      && onReloadFile) onReloadFile(file.id);
      if (onSaveCopy && ImGui::MenuItem("Save as CAP File...")) onSaveCopy(file.id, "");
      if (onExportRoot && ImGui::MenuItem("Export to ROOT...")) onExportRoot(file.id, "");
      if (ImGui::MenuItem("Close File")) close = true;
      }
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
    if (fileById(file.id) && file.file) renderFolder(file.id, *file.file, "", file.editable);
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
    for (const auto & d : copy.directories) renderDirectory(FileSystem::join(String(directory), String(d)).str(), d, false);
    for (const auto & f : copy.files)       renderCapFileInTree(FileSystem::join(String(directory), String(f)).str(), f);
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
  if (file) dragAndDrop(file->id, "", String(label), true, false);
  if (!ImGui::IsDragDropActive()) ImGui::SetItemTooltip("%s%s", path.c_str(), loaded ? "" : "\nclick to open");
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
    if (onSaveCopy && ImGui::MenuItem("Save as CAP File...")) onSaveCopy(file->id, "");
    if (onExportRoot && ImGui::MenuItem("Export to ROOT...")) onExportRoot(file->id, "");
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

  void FileBrowser::renderFolder(int fileId, const Folder & folder, const std::string & prefix, bool editable)
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
      dragAndDrop(fileId, path, name, true, editable);
      if (ImGui::BeginPopupContextItem("##folderMenu"))
        {
        if (ImGui::MenuItem("Show All") && onShowFolder) onShowFolder(fileId, String(path));
        if (onSaveCopy && ImGui::MenuItem("Save Folder as CAP File...")) onSaveCopy(fileId, String(path));
        if (onExportRoot && ImGui::MenuItem("Export to ROOT...")) onExportRoot(fileId, String(path));
        if (editable) { ImGui::Separator(); editMenu(fileId, path, true); }
        ImGui::EndPopup();
        }
      if (!ImGui::IsDragDropActive())
        ImGui::SetItemTooltip("%u entries; right-click: show all%s", sub->size(),
                              editable ? ", add a folder, rename, remove" : "; drag it onto a tree to copy it");
      if (open) { renderFolder(fileId, *sub, path, editable); ImGui::TreePop(); }
      ImGui::PopID();
      }
    else if (matches(name))
      renderObject(fileId, folder, name, path, editable);
    }
  }

  void FileBrowser::renderObject(int fileId, const Folder & folder, const String & name, const std::string & path,
                                 bool editable)
  {
  // an object of a lazily opened file is read when it is hovered (tooltip) or shown, not when it is listed
  String kind, details;
  bool loaded = folder.isLoaded(name);
  bool showable;
  if (loaded) showable = describe(folder, name, kind, details);
  else
    {
    std::string type = folder.typeName(name);
    kind     = shortTypeName(type);
    details  = String(type) + String("\n(not read yet)");
    showable = showableType(type);
    }
  ImGui::PushID(name.data());
  bool selected = (_selected.file == fileId && _selected.path.str() == path);
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
  if (selected) flags |= ImGuiTreeNodeFlags_Selected;
  if (!showable) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
  ImGui::TreeNodeEx("##object", flags, "%s", name.data());
  if (!showable) ImGui::PopStyleColor();

  // click (press and release on the object without dragging): show it
  std::string key = std::to_string(fileId) + "\n" + path;
  if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) _pressed = key;
  if (showable && _pressed == key && ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)
      && ImGui::GetIO().MouseDragMaxDistanceSqr[0] < 16.0f)
    {
    _selected.file = fileId;
    _selected.path = String(path);
    if (onShowObject) onShowObject(_selected);
    }

  // drag onto a graph (objects that can be shown) or onto a folder of a tree (copy)
  dragAndDrop(fileId, path, name, false, editable);
  if (!ImGui::IsDragDropActive() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
    {
    if (!loaded)
      {
      try { describe(folder, name, kind, details); }
      catch (const std::exception & e) { details = String("Cannot read it: ") + String(e.what()); }
      }
    ImGui::SetTooltip("%s", details.data());
    }

  if (ImGui::BeginPopupContextItem("##objectMenu"))
    {
    if (ImGui::MenuItem("Show", nullptr, false, showable) && onShowObject)
      {
      _selected.file = fileId;
      _selected.path = String(path);
      onShowObject(_selected);
      }
    if (ImGui::MenuItem("Show Data Table", nullptr, false, showable) && onShowData)
      {
      ObjectReference reference;
      reference.file = fileId;
      reference.path = String(path);
      onShowData(reference);
      }
    if (editable) { ImGui::Separator(); editMenu(fileId, path, false); }
    ImGui::EndPopup();
    }
  ImGui::SameLine();
  ImGui::TextDisabled("%s", kind.data());
  ImGui::PopID();
  }

  void FileBrowser::dragAndDrop(int fileId, const std::string & path, const String & name, bool isFolder,
                                bool editable)
  {
  std::string key = std::to_string(fileId) + "\n" + path;
  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
    ImGui::SetDragDropPayload(isFolder ? folderPayloadType() : payloadType(), key.c_str(), key.size()+1);
    ImGui::Text("%s%s", name.data(), isFolder ? "/" : "");
    ImGui::TextDisabled(isFolder ? "drop it on a folder of a tree to copy it"
                                 : "drop it on a graph to show it, or on a folder of a tree to copy it");
    ImGui::EndDragDropSource();
    }
  if (!editable || !isFolder || !ImGui::BeginDragDropTarget()) return;
  for (const char * type : { payloadType(), folderPayloadType() })
    if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload(type))
      {
      ObjectReference source;
      if (decodePayload(payload->Data, payload->DataSize, source) && onCopy)
        onCopy(source, std::strcmp(type, folderPayloadType()) == 0, fileId, String(path));
      }
  ImGui::EndDragDropTarget();
  }

  void FileBrowser::editMenu(int fileId, const std::string & path, bool isFolder)
  {
  if (isFolder && ImGui::MenuItem("New Folder..."))
    _nameRequest = NameRequest{NameRequest::kNewFolder, fileId, path, "folder", false};
  if (path.empty()) return;
  std::string leaf = path.substr(path.rfind('/') == std::string::npos ? 0 : path.rfind('/') + 1);
  if (ImGui::MenuItem("Rename..."))
    _nameRequest = NameRequest{NameRequest::kRename, fileId, path, leaf, false};
  if (ImGui::MenuItem("Remove") && onRemove) onRemove(fileId, String(path));
  }

  void FileBrowser::renderNameDialog()
  {
  if (_nameRequest.kind == NameRequest::kNone) return;
  const char * title = _nameRequest.kind == NameRequest::kNewFolder ? "New Folder##browserName" : "Rename##browserName";
  if (!_nameRequest.opened) { ImGui::OpenPopup(title); _nameRequest.opened = true; }
  if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
    _nameRequest = NameRequest();                    // closed (Escape)
    return;
    }
  ImGui::Text("%s", _nameRequest.kind == NameRequest::kNewFolder ? "Name of the new folder:" : "New name:");
  if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
  bool done = ImGui::InputText("##name", &_nameRequest.text, ImGuiInputTextFlags_EnterReturnsTrue);
  done = ImGui::Button("OK") || done;
  ImGui::SameLine();
  bool cancel = ImGui::Button("Cancel");
  if (done && !_nameRequest.text.empty())
    {
    if (_nameRequest.kind == NameRequest::kNewFolder && onNewFolder)
      onNewFolder(_nameRequest.file, String(_nameRequest.path), String(_nameRequest.text));
    if (_nameRequest.kind == NameRequest::kRename && onRename)
      onRename(_nameRequest.file, String(_nameRequest.path), String(_nameRequest.text));
    }
  if (done || cancel)
    {
    _nameRequest = NameRequest();
    ImGui::CloseCurrentPopup();
    }
  ImGui::EndPopup();
  }

  } // namespace GUI
} // namespace CAP
