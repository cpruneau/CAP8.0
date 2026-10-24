/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__CapFile
#define CAP__CapFile
#include "BinaryFile.hpp"
#include "Folder.hpp"
#include "Accountant.hpp"
#include <vector>

namespace CAP
{

//!
//! @brief CAP binary file: a BinaryFile holding one Folder (with its objects and optional sub-folders).
//!
//! The folder is held in memory: objects are added, retrieved and organized with the Folder interface, reached with
//! folder() or directly on the CapFile (add(), get(), addFolder(), cd(), ...; a CapFile also converts to Folder & where
//! a folder is expected). read() loads the folder from the file and write() saves it, as one "CAPO" record of the CAP
//! binary format (see BinaryIO.hpp and Folder).
//!
//! As a File, the name of the object is the path of the file; the name of the folder is folderName().
//!
//! load(File::Loading::kLazy) reads the file without decoding it: the tree of sub-folders is built at once, with the
//! names and types of all the objects, and each object is decoded when it is first used (get()), or by loadAll(). The
//! file is read once, to check its checksums, so a corrupted file is still refused at once. If the file changes on disk
//! afterwards, objects not decoded yet can no longer be (FileException): open it again.
//!
//! Classes of the stored objects must be registered with Folder::registerType<T>() to be read back (the Math and
//! Graphics classes register themselves when their libraries are loaded).
//!
//! Example:
//! @code
//!   CAP::CapFile out("run42.cap", CAP::File::kWriteOnly);
//!   out.setFolderName("run42");
//!   out.addFolder("singles")->add(new CAP::MATH::Histogram("pt","p_T",100,0.0,10.0));
//!   out.write();
//!
//!   CAP::CapFile in("run42.cap", CAP::File::kReadOnly);
//!   in.read();
//!   auto * pt = in.get<CAP::MATH::Histogram>("singles/pt");
//! @endcode
//!
class CapFile : public BinaryFile
{
public:

  //! @brief Constructor: an empty folder named "" attached to a file. @param path path of the file
  //! @param access access allowed (read/write by default)
  CapFile(const String & path="", Access access=kReadWrite);
  //! @brief Destructor (the file is closed if open; the folder is not saved automatically).
  virtual ~CapFile() {}

  //! @brief Replace the content (name, entries and sub-folders) of the folder by a copy of folder; the file is unchanged.
  //! @param folder folder to copy @return this CapFile
  CapFile & operator=(const Folder & folder);

  //! @name Folder
  //! @{

  //! @brief The folder held. @return folder
  Folder & folder() { return _folder; }
  //! @brief The folder held. @return folder
  const Folder & folder() const { return _folder; }
  //! @brief The folder held, where a Folder is expected.
  operator Folder & () { return _folder; }
  operator const Folder & () const { return _folder; }
  //! @brief Name of the folder. @return name
  const String & folderName() const { return _folder.name(); }
  //! @brief Set the name of the folder. @param name name
  void setFolderName(const String & name) { _folder.setName(name); }

  //! @brief See Folder::add(). @param object object (owned by the folder) @param name name (that of the object if empty)
  template <typename T>
  T * add(T * object, const String & name="") { return _folder.add(object,name); }
  //! @brief See Folder::addFolder(). @param name name of the sub-folder @return sub-folder
  Folder * addFolder(const String & name) { return _folder.addFolder(name); }
  //! @brief See Folder::get(). @param path path of the object @return object, or null
  template <typename T>
  T * get(const String & path) const { return _folder.get<T>(path); }
  //! @brief See Folder::folder(). @param path path of the sub-folder @return sub-folder, or null
  Folder * folder(const String & path) const { return _folder.folder(path); }
  //! @brief See Folder::contains(). @param path path @return true if there is an entry at path
  bool contains(const String & path) const { return _folder.contains(path); }
  //! @brief See Folder::remove(). @param path path @return true if an entry was removed
  bool remove(const String & path) { return _folder.remove(path); }
  //! @brief See Folder::release(). @param path path @return object (now owned by the caller), or null
  template <typename T>
  T * release(const String & path) { return _folder.release<T>(path); }
  //! @brief See Folder::cd(). @param path path of the sub-folder @return new current folder
  Folder * cd(const String & path) { return _folder.cd(path); }
  //! @brief See Folder::cd(): back to the top folder. @return top folder
  Folder * cd() { return _folder.cd(); }
  //! @brief See Folder::current(). @return current folder
  Folder * current() const { return _folder.current(); }
  //! @brief See Folder::pwd(). @return path of the current folder
  String pwd() const { return _folder.pwd(); }
  //! @brief See Folder::ls(). @param out stream @param path path of a sub-folder
  void ls(std::ostream & out, const String & path="") const { _folder.ls(out,path); }
  //! @brief See Folder::names(). @return names of the entries
  std::vector<String> names() const { return _folder.names(); }
  //! @brief See Folder::paths(). @return paths of all the objects
  std::vector<String> paths() const { return _folder.paths(); }
  //! @brief Number of entries of the folder. @return number of entries
  unsigned int size() const { return _folder.size(); }
  //! @brief Whether the folder is empty. @return true if it has no entry
  bool empty() const { return _folder.empty(); }
  //! @brief Remove all the entries of the folder (the file is not changed).
  void clear() override { _folder.clear(); }

  //! @}

  //! @name Reading and writing
  //! @{

  //!
  //! @brief Load the first folder stored in the file, replacing the folder held (its name included).
  //! @throws FileException if the file cannot be read, holds no folder, or is corrupted (the folder is then unchanged)
  //!
  void read();

  //!
  //! @brief Load the folder of the given name stored in the file, replacing the folder held.
  //! @param folderName name of the folder to load
  //! @throws FileException if the file cannot be read, holds no such folder, or is corrupted (the folder is then unchanged)
  //!
  void read(const String & folderName);

  //!
  //! @brief Replace the content of the file by the folder held (objects not decoded are copied as they are, or decoded
  //! first if the file is the one they come from).
  //! @param skipUnsavable leave out objects without binary support instead of throwing
  //! @throws FileException if the file cannot be written or an object cannot be saved (and skipUnsavable is false)
  //!
  void write(bool skipUnsavable=false);

  //!
  //! @brief Add the folder held at the end of the file (several folders can be stored in one file; see read(folderName)).
  //! @param skipUnsavable leave out objects without binary support instead of throwing
  //! @throws FileException if the file cannot be written or an object cannot be saved (and skipUnsavable is false)
  //!
  void append(bool skipUnsavable=false);

  //! @}

  //! @name Content in memory
  //! @{

  //! @brief Load the first folder of the file: decoded (kEager, read()), or with objects decoded when first used
  //! (kLazy). @param loading kEager or kLazy @throws FileException if the file cannot be read, holds no folder, or is
  //! corrupted (the folder held is then unchanged)
  void load(Loading loading = Loading::kEager) override;
  //! @brief Decode the objects not decoded yet (after load(kLazy)). @throws FileException if one cannot be decoded
  void loadAll() { _folder.loadAll(); }
  //! @brief Number of objects not decoded yet. @return number
  unsigned int nNotLoaded() const { return _folder.nNotLoaded(); }
  //! @brief Whether the object at path is decoded. @param path path @return true if so
  bool isLoaded(const String & path) const { return _folder.isLoaded(path); }
  using File::isLoaded;
  //! @brief Empty the folder held, its name included (the file is not changed).
  void unload() override;
  //! @brief Replace the content of the file by the folder held (write()).
  void save() override { write(); }

  //! @}

  //! @brief Print the file (path, format, access, state) and the folder.
  void print() const override;

  //! @name Opening files (replace the TFile helpers openRootFile(), openOldRootFile(), ... of RootFileHelpers)
  //!
  //! The ioOption strings are those of TFile, case-insensitive:
  //! - "READ" or "OLD": the file must exist; it is opened read-only and its folder is loaded.
  //! - "NEW" or "CREATE": the file must not exist; an empty folder is returned, saved by write().
  //! - "RECREATE": an empty folder is returned; write() replaces any existing file.
  //! - "UPDATE": the folder of the file is loaded if the file exists (empty otherwise); write() replaces it.
  //!
  //! Unlike a TFile, a CapFile only touches the disk on read(), write() and append(): call write() to save.
  //! The returned objects are created with new and belong to the caller.
  //! @{

  //!
  //! @brief Full name of a CAP file: name if it starts with '/', otherwise path/name; ".cap" is appended if missing.
  //! @param path directory (may be empty) @param name file name, with or without the ".cap" extension @return file name
  //!
  static String makeFileName(const String & path, const String & name);

  //!
  //! @brief Open a CAP file. @param path directory @param name file name (".cap" appended if missing)
  //! @param ioOption "READ"/"OLD", "NEW"/"CREATE", "RECREATE" or "UPDATE" @param verbose print what is done
  //! @return new CapFile (owned by the caller)
  //! @throws FileException if the option is unknown, a file to read does not exist or cannot be loaded, or a new file exists
  //!
  static CapFile * openCapFile(const String & path, const String & name, const String & ioOption, bool verbose=false);
  //! @brief Open a CAP file (see openCapFile(path,name,ioOption,verbose)). @param name file name @param ioOption option
  //! @param verbose print what is done @return new CapFile (owned by the caller)
  static CapFile * openCapFile(const String & name, const String & ioOption, bool verbose=false);
  //! @brief Open an existing CAP file and load it ("OLD"). @param path directory @param name file name @param verbose print
  //! @return new CapFile (owned by the caller)
  static CapFile * openOldCapFile(const String & path, const String & name, bool verbose=false);
  //! @brief Open an existing CAP file and load it ("OLD"). @param name file name @param verbose print @return new CapFile
  static CapFile * openOldCapFile(const String & name, bool verbose=false);
  //! @brief Create a CAP file that must not exist yet ("NEW"). @param path directory @param name file name
  //! @param verbose print @return new CapFile (owned by the caller)
  static CapFile * openNewCapFile(const String & path, const String & name, bool verbose=false);
  //! @brief Create a CAP file that must not exist yet ("NEW"). @param name file name @param verbose print @return new CapFile
  static CapFile * openNewCapFile(const String & name, bool verbose=false);
  //! @brief Create or replace a CAP file ("RECREATE"). @param path directory @param name file name @param verbose print
  //! @return new CapFile (owned by the caller)
  static CapFile * openRecreateCapFile(const String & path, const String & name, bool verbose=false);
  //! @brief Create or replace a CAP file ("RECREATE"). @param name file name @param verbose print @return new CapFile
  static CapFile * openRecreateCapFile(const String & name, bool verbose=false);

  //! @cond
  // Overloads catching literal arguments. Without them, openOldCapFile("dir","name") would convert "name" to the bool
  // verbose of openOldCapFile(name,verbose) (a standard conversion beats the conversion to String), and
  // openCapFile("dir","name","OLD") would call openCapFile(name,ioOption,verbose).
  static CapFile * openCapFile(const String & path, const String & name, const char * ioOption, bool verbose=false)
  { return openCapFile(path,name,String(ioOption),verbose); }
  static CapFile * openOldCapFile(const String & path, const char * name, bool verbose=false)
  { return openOldCapFile(path,String(name),verbose); }
  static CapFile * openNewCapFile(const String & path, const char * name, bool verbose=false)
  { return openNewCapFile(path,String(name),verbose); }
  static CapFile * openRecreateCapFile(const String & path, const char * name, bool verbose=false)
  { return openRecreateCapFile(path,String(name),verbose); }
  //! @endcond

  //!
  //! @brief Open several CAP files. @param names file names @param ioOption option (see openCapFile) @param verbose print
  //! @return new CapFile objects (owned by the caller)
  //! @throws FileException if the list is empty or a file cannot be opened (the files already opened are then deleted)
  //!
  static std::vector<CapFile*> openCapFiles(const std::vector<String> & names, const String & ioOption, bool verbose=false);
  //! @brief Open several CAP files in a directory (see openCapFiles(names,ioOption,verbose)). @param path directory
  //! @param names file names @param ioOption option @param verbose print @return new CapFile objects (owned by the caller)
  static std::vector<CapFile*> openCapFiles(const String & path, const std::vector<String> & names, const String & ioOption, bool verbose=false);

  //! @}

  //! @name Parameters and accountants (replace exportParameter(), importParameter(), ... of RootFileHelpers)
  //!
  //! Parameters are stored as LongParameter entries of the folder; an existing entry of the same name is
  //! replaced. They reach the disk with the folder, on write().
  //! @{

  //! @brief Store a parameter. @param name name of the parameter @param value value @param verbose print what is done
  //! @throws Exception if the name is empty or contains '/', or an entry of that name that is not a parameter exists
  void exportParameter(const String & name, long value, bool verbose=false);
  //! @brief Value of a stored parameter. @param name name or path of the parameter @param verbose print what is done
  //! @return value @throws FileException if there is no such parameter
  long importParameter(const String & name, bool verbose=false) const;

  //!
  //! @brief Store the counts of an accountant as parameters Accountant_<name>_N (number of counts) and
  //! Accountant_<name>_<k> (count k), as the ROOT version did.
  //! @param accountant accountant to store @param verbose print what is done
  //!
  void exportAccountant(const Accountant & accountant, bool verbose=false);
  //!
  //! @brief Load the counts of the accountant of the same name stored by exportAccountant().
  //! If the accountant is empty, it is resized to the stored number of counts; otherwise that number must match its size.
  //! @param accountant accountant to fill @param verbose print what is done
  //! @throws FileException if a parameter is missing or the sizes differ (the accountant is then unchanged)
  //!
  void importAccountant(Accountant & accountant, bool verbose=false) const;

  //! @}

protected:

  //! @brief Write the folder to the file. @param mode kWrite (replace the content) or kAppend @param skipUnsavable see write()
  void writeFolder(Mode mode, bool skipUnsavable);
  //! @brief Decode the first folder of the file, or the first one of the given name, into folder (read straight from
  //! the file: its records are copied once). @param folderName name, or null for the first folder @param folder output
  //! @return true if found @throws FileException if the file cannot be read or is corrupted
  bool readFolder(const String * folderName, Folder & folder);

  //! @brief Before writing to the file the objects come from: decode them all (the writing changes the file).
  void loadAllBeforeWriting();

  Folder _folder;  //!<! folder held (not streamed by ROOT)
  std::shared_ptr<const LazySource> _source;  //!<! file the objects not decoded come from (lazy loading)

};

} // namespace CAP

#endif
