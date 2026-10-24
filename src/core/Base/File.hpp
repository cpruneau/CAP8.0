/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__File
#define CAP__File
#include <cstdint>
#include <fstream>
#include "String.hpp"
#include "Exceptions.hpp"
#include "FileSystemEntry.hpp"

namespace CAP
{

//!
//! @brief Proxy for a file of the operating system.
//!
//! File handles what all files share: opening and closing, the stream, and checks. It makes no assumption on the
//! content. The path and the questions to the operating system (size, date, permissions, ...) come from
//! FileSystemEntry. Derived classes set, at construction,
//!  - the format: text (kText) or binary (kBinary);
//!  - the access the file allows: read only (kReadOnly), write only (kWriteOnly) or read and write (kReadWrite);
//! and provide the operations on their type of content:
//!
//!     File ─┬─ TxtFile ──── XmlFile     text: lines held in memory; XML
//!           └─ BinaryFile ── CapFile    raw bytes; one Folder of CAP objects
//!
//! A file is opened in one of four modes, which must be allowed by its access (FileException otherwise):
//!  - kRead   : read an existing file;
//!  - kWrite  : create the file, or empty an existing one, and write;
//!  - kAppend : create the file if needed and write at its end;
//!  - kUpdate : read and write an existing file (created if missing), without emptying it; requires kReadWrite.
//!
//! The name of the object (Object::name()) is the path given at construction. Files cannot be copied: each File
//! owns its stream. The stream is closed by close() or when the File is destroyed.
//!
//! Example (with a derived class):
//! @code
//!   CAP::TxtFile notes("notes.txt");            // read and write allowed
//!   notes.open(CAP::File::kAppend);
//!   notes.stream() << "one more line" << std::endl;
//!   notes.close();
//! @endcode
//!
class File : public FileSystemEntry
{
public:

  //! @brief Format of the content.
  enum Format { kText, kBinary };
  //! @brief Access allowed by the file type.
  enum Access { kReadOnly, kWriteOnly, kReadWrite };
  //! @brief Mode of an open file.
  enum Mode   { kRead, kWrite, kAppend, kUpdate };
  //! @brief How load() brings the content into memory: all of it (kEager), or only what is needed to reach it, the
  //! rest being read on demand (kLazy; classes without a lazy mode load everything).
  enum class Loading { kEager, kLazy };

protected:

  //!
  //! @brief Constructor for derived classes.
  //! @param path path of the file @param format content format @param access access allowed by the file type
  //! @param className name of the derived class
  //!
  File(const String & path, Format format, Access access, const String & className);

public:

  //! @brief Destructor: closes the file if it is open.
  virtual ~File();
  File(const File &) = delete;
  File & operator=(const File &) = delete;

  //! @name Path and type
  //! @{

  //! @brief Set the path of the file (the name of the object is unchanged). @param path new path
  //! @throws FileException if the file is open
  void setPath(const String & path) override;
  //! @brief Content format. @return kText or kBinary
  Format format() const { return _format; }
  //! @brief Whether the content is text. @return true for kText
  bool isText() const { return _format==kText; }
  //! @brief Whether the content is binary. @return true for kBinary
  bool isBinary() const { return _format==kBinary; }
  //! @brief Access allowed by the file type. @return kReadOnly, kWriteOnly or kReadWrite
  Access access() const { return _access; }
  //! @brief Whether the file may be read. @return true for kReadOnly and kReadWrite
  bool canRead() const { return _access!=kWriteOnly; }
  //! @brief Whether the file may be written. @return true for kWriteOnly and kReadWrite
  bool canWrite() const { return _access!=kReadOnly; }
  //! @brief Whether the file allows a mode. @param mode mode @return true if allowed by access()
  bool allows(Mode mode) const;

  //! @}

  //! @name File system
  //! @{

  //! @brief Whether the file exists. @return true if a regular file exists at path() (a directory does not count)
  bool exists() const override { return exists(_path); }
  //! @brief Whether a regular file exists. @param path path of the file @return true if it exists
  static bool exists(const String & path);

  //! @}

  //! @name Opening and closing
  //! @{

  //!
  //! @brief Open the file (an open file is closed first).
  //! @param mode kRead, kWrite, kAppend or kUpdate
  //! @throws FileException if the mode is not allowed by access(), the path is empty, or the file cannot be opened
  //!
  virtual void open(Mode mode);
  //! @brief Close the file (nothing happens if it is not open).
  virtual void close();
  //! @brief Whether the file is open. @return true if open
  bool isOpen() const { return _stream.is_open(); }
  //! @brief Mode of the open file. @return mode @throws FileException if the file is not open
  Mode mode() const;
  //! @brief Stream of the open file. @return stream @throws FileException if the file is not open
  std::fstream & stream();

  //! @}

  //! @name Content in memory
  //!
  //! Each type of file has its structure in memory: lines (TxtFile), an XML document (XmlFile), bytes (BinaryFile),
  //! a Folder of objects (CapFile). load() fills it from the file and save() writes it back; both open and close the
  //! file themselves. isLoaded() is true after load() or save(), until unload() (which frees the structure; the file is
  //! not changed) or a change of path.
  //! @{

  //! @brief Fill the structure from the file. @param loading kEager or kLazy
  //! @throws FileException if the file cannot be read (or the class holds no structure)
  virtual void load(Loading loading = Loading::kEager);
  //! @brief Free the structure (the file is not changed).
  virtual void unload() { _loaded = false; }
  //! @brief Whether the structure was filled from the file (or saved to it), and not unloaded since. @return true if so
  virtual bool isLoaded() const { return _loaded; }
  //! @brief Write the structure to the file, replacing its content.
  //! @throws FileException if the file cannot be written (or the class holds no structure)
  virtual void save();

  //! @}

  //! @brief Print the path, format, access and state.
  void print() const override;

protected:

  //! @brief Throw unless the file is open in a mode that can read. @param source caller name @throws FileException otherwise
  void requireReadable(const char * source);
  //! @brief Throw unless the file is open in a mode that can write. @param source caller name @throws FileException otherwise
  void requireWritable(const char * source);
  //! @brief Throw unless the file is open and its stream is in a good state. @param source caller name @param what operation
  void checkStream(const char * source, const char * what);

  Format       _format;  //!< content format
  Access       _access;  //!< access allowed by the file type
  Mode         _mode;    //!< mode of the open file
  bool         _loaded;  //!< the structure was loaded from the file, or saved to it
  std::fstream _stream;  //!<! stream of the open file (not streamed by ROOT)

};

} // namespace CAP

#endif
