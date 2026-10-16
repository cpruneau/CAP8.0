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
#ifndef CAP__CapFile
#define CAP__CapFile
#include "BinaryFile.hpp"
#include "Folder.hpp"
#include "Accountant.hpp"
#include <vector>

namespace CAP
{

//!
//! @brief CAP binary file: one Folder (with its objects and optional sub-folders) stored on disk.
//!
//! A CapFile is the Folder it holds: objects are added, retrieved and organized with the Folder interface. read()
//! loads the folder from the file and write() saves it, as one "CAPO" record of the CAP binary format (see BinaryIO.hpp
//! and Folder). The file itself is a BinaryFile, reached with file(); its path is separate from the name of the folder.
//!
//! CapFile inherits Folder only. The file is a member rather than a second base class, so that the object has a single
//! name (that of the folder) and a single Object base.
//!
//! Classes of the stored objects must be registered with Folder::registerType<T>() to be read back (the Math and
//! Graphics classes register themselves when their libraries are loaded).
//!
//! Example:
//! @code
//!   CAP::CapFile out("run42.cap", CAP::File::kWriteOnly);
//!   out.setName("run42");
//!   out.addFolder("singles")->add(new CAP::MATH::Histogram("pt","p_T",100,0.0,10.0));
//!   out.write();
//!
//!   CAP::CapFile in("run42.cap", CAP::File::kReadOnly);
//!   in.read();
//!   auto * pt = in.get<CAP::MATH::Histogram>("singles/pt");
//! @endcode
//!
class CapFile : public Folder
{
public:

  //! @brief Constructor: an empty folder named "" attached to a file. @param path path of the file
  //! @param access access allowed (read/write by default)
  CapFile(const String & path="", File::Access access=File::kReadWrite);
  //! @brief Destructor (the file is closed if open; the folder is not saved automatically).
  virtual ~CapFile() {}
  CapFile(const CapFile &) = delete;
  CapFile & operator=(const CapFile &) = delete;

  //! @brief Replace the content (name, entries and sub-folders) of the folder by a copy of folder; the file is unchanged.
  //! @param folder folder to copy @return this CapFile
  CapFile & operator=(const Folder & folder);

  //! @name File
  //! @{

  //! @brief The underlying binary file. @return file
  BinaryFile & file() { return _file; }
  //! @brief The underlying binary file. @return file
  const BinaryFile & file() const { return _file; }
  //! @brief Path of the file. @return path
  const String & path() const { return _file.path(); }
  //! @brief Set the path of the file. @param path new path @throws FileException if the file is open
  void setPath(const String & path) { _file.setPath(path); }
  //! @brief Whether the file exists. @return true if it exists
  bool exists() const { return _file.exists(); }

  //! @}

  //! @name Reading and writing
  //! @{

  //!
  //! @brief Load the first folder stored in the file, replacing the content of this folder (its name included).
  //! @throws FileException if the file cannot be read, holds no folder, or is corrupted (this folder is then unchanged)
  //!
  void read();

  //!
  //! @brief Load the folder of the given name stored in the file, replacing the content of this folder.
  //! @param folderName name of the folder to load
  //! @throws FileException if the file cannot be read, holds no such folder, or is corrupted (this folder is then unchanged)
  //!
  void read(const String & folderName);

  //!
  //! @brief Replace the content of the file by this folder.
  //! @param skipUnsavable leave out objects without binary support instead of throwing
  //! @throws FileException if the file cannot be written or an object cannot be saved (and skipUnsavable is false)
  //!
  void write(bool skipUnsavable=false);

  //!
  //! @brief Add this folder at the end of the file (several folders can be stored in one file; see read(folderName)).
  //! @param skipUnsavable leave out objects without binary support instead of throwing
  //! @throws FileException if the file cannot be written or an object cannot be saved (and skipUnsavable is false)
  //!
  void append(bool skipUnsavable=false);

  //! @}

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
  //! Parameters are stored as LongParameter entries of the current folder; an existing entry of the same name is
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

  //! @brief Write this folder to the file. @param mode kWrite (replace the content) or kAppend @param skipUnsavable see write()
  void writeFolder(File::Mode mode, bool skipUnsavable);
  //! @brief Take the content (name, entries and sub-folders) of folder; the class name stays "CapFile". @param folder folder
  void adopt(const Folder & folder);

  BinaryFile _file;  //!<! file holding the folder (not streamed by ROOT)

  ClassDefOverride(CapFile,0)
};

} // namespace CAP

#endif
