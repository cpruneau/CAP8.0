/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file BinaryFile.hpp
//! @brief BinaryFile: file of raw bytes, read whole, by ranges, or lazily.
#ifndef CAP__BinaryFile
#define CAP__BinaryFile
#include <string>
#include "File.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Binary file: raw bytes, read and written without any conversion.
//!
//! Gives byte-level access (readAll(), writeAll(), appendAll(), readRange()) and, through File::open() and
//! File::stream(), a binary stream for record-based formats such as the CAP binary records (see BinaryIO.hpp and
//! CapFile).
//!
//! In memory, the content is bytes(): load() reads the whole file into it and save() writes it back. load(kLazy) keeps
//! nothing in memory: the content is then read on demand, by ranges, with readRange().
//!
//! Example:
//! @code
//!   CAP::BinaryFile f("data.bin");
//!   f.writeAll(std::string("\x01\x02\x03",3));
//!   std::string bytes = f.readAll();     // 3 bytes
//! @endcode
//!
class BinaryFile : public File
{
public:

  //! @brief Constructor. @param path path of the file @param access access allowed (read/write by default)
  BinaryFile(const String & path="", Access access=kReadWrite);
  //! @brief Destructor.
  virtual ~BinaryFile() {}

  //! @brief Read the whole file. @return its bytes @throws FileException if the file cannot be read
  std::string readAll();
  //! @brief Replace the content of the file by bytes. @param bytes content @throws FileException if the file cannot be written
  void writeAll(const std::string & bytes);
  //! @brief Add bytes at the end of the file (created if needed). @param bytes data @throws FileException if the file cannot be written
  void appendAll(const std::string & bytes);
  //!
  //! @brief Read part of the file (whatever is loaded). @param offset position of the first byte
  //! @param length number of bytes @return the bytes (fewer than length at the end of the file)
  //! @throws FileException if the file cannot be read or offset is beyond its end
  //!
  std::string readRange(std::uintmax_t offset, std::size_t length);

  //! @name Content in memory
  //! @{

  //! @brief Read the whole file into bytes() (kEager), or check that it can be read and keep nothing (kLazy: use
  //! readRange()). @param loading kEager or kLazy @throws FileException if the file cannot be read
  void load(Loading loading = Loading::kEager) override;
  //! @brief Free the bytes held (the file is not changed).
  void unload() override;
  //! @brief Replace the content of the file by bytes(). @throws FileException if the file cannot be written, or the file
  //! was loaded lazily (nothing is held)
  void save() override;
  //! @brief Whether the file was loaded lazily (bytes() is then empty). @return true if so
  bool isLazy() const { return _lazy; }
  //! @brief Bytes held. @return bytes
  const std::string & bytes() const { return _bytes; }
  //! @brief Replace the bytes held (the file is not changed until save()). @param bytes bytes
  void setBytes(const std::string & bytes) { _bytes = bytes; _lazy = false; }

  //! @}

protected:

  //! @brief Write bytes to the open stream, then close it. @param bytes data @param source caller name
  void writeBytes(const std::string & bytes, const char * source);

  std::string _bytes;  //!<! bytes held (not streamed by ROOT)
  bool        _lazy;   //!< loaded lazily: nothing held

  ClassDefOverride(BinaryFile,0)
};

} // namespace CAP

#endif
