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
#ifndef CAP__BinaryFile
#define CAP__BinaryFile
#include <string>
#include "File.hpp"

namespace CAP
{

//!
//! @brief Binary file: raw bytes, read and written without any conversion.
//!
//! Gives byte-level access (readAll(), writeAll(), appendAll()) and, through File::open() and File::stream(), a binary
//! stream for record-based formats such as the CAP binary records (see BinaryIO.hpp and CapFile).
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

protected:

  //! @brief Write bytes to the open stream, then close it. @param bytes data @param source caller name
  void writeBytes(const std::string & bytes, const char * source);

  ClassDefOverride(BinaryFile,0)
};

} // namespace CAP

#endif
