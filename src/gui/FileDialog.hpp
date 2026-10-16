#ifndef CAP__GUI__FileDialog
#define CAP__GUI__FileDialog
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "String.hpp"

namespace CAP
{
  namespace GUI
  {

  //!
  //! @brief Modal dialog to choose a file to open, a folder, or a file to save to, drawn with ImGui (no native or
  //! ROOT dialog needed).
  //!
  //! open() shows the dialog; render() must be called every frame; when the user confirms, the callback receives
  //! the chosen path. Double-click a folder to enter it, a file to choose it; the path field accepts a typed path.
  //!
  class FileDialog
  {
  public:

  enum Mode { kOpenFile, kOpenFolder, kSaveFile };
  using Callback = std::function<void(const String & path)>;

  FileDialog();

  //! @brief Show the dialog. @param mode what is chosen @param title window title @param directory starting folder
  //! @param extensions file name endings shown (e.g. {".cap", ".root"}; empty: all files)
  //! @param fileName initial file name (kSaveFile) @param callback called with the chosen path
  void open(Mode mode, const String & title, const String & directory, const std::vector<String> & extensions,
            const String & fileName, const Callback & callback);
  bool isOpen() const { return _open || _requested; }
  //! @brief Draw the dialog if open; call once per frame.
  void render();
  //! @brief Folder shown last (to start the next dialog there).
  const std::string & directory() const { return _directory; }

  protected:

  struct Entry { std::string name; bool folder; std::uintmax_t size; };

  Mode _mode;
  std::string _title;
  std::string _directory;
  std::vector<std::string> _extensions;
  char _fileName[512];
  char _pathField[1024];
  Callback _callback;
  bool _open;
  bool _requested;
  bool _showHidden;
  int  _selected;
  std::vector<Entry> _entries;
  std::string _error;

  void list();
  void enter(const std::string & directory);
  void confirm(const std::string & path);
  bool accepts(const std::string & name) const;
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__FileDialog
