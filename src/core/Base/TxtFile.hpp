/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__TxtFile
#define CAP__TxtFile
#include <vector>
#include "File.hpp"

namespace CAP
{

//!
//! @brief Text (ASCII) file holding its content in memory as lines.
//!
//! The text is kept as a list of lines, without end-of-line characters. read() (or load()) loads the whole file,
//! replacing the lines held, write() (or save()) replaces the file by the lines held, and append() adds them at the end
//! of the file. These functions open and close the file themselves; the stream remains available through File::open() and File::stream()
//! for other uses. Line endings "\n" and "\r\n" are both accepted when reading; "\n" is written.
//!
//! Example:
//! @code
//!   CAP::TxtFile out("summary.txt", CAP::File::kWriteOnly);
//!   out.addLine("run 42");
//!   out.addLine("events 1000");
//!   out.write();
//!
//!   CAP::TxtFile in("summary.txt", CAP::File::kReadOnly);
//!   in.read();
//!   CAP::String first = in.line(0);    // "run 42"
//! @endcode
//!
class TxtFile : public File
{
public:

  //! @brief Constructor. @param path path of the file @param access access allowed (read/write by default)
  TxtFile(const String & path="", Access access=kReadWrite);
  //! @brief Destructor.
  virtual ~TxtFile() {}

  //! @name Text held in memory
  //! @{

  //! @brief Number of lines. @return number of lines
  unsigned int nLines() const { return _lines.size(); }
  //! @brief All lines. @return lines
  const std::vector<String> & lines() const { return _lines; }
  //! @brief Line number index. @param index 0 to nLines()-1 @return line @throws FileException if index>=nLines()
  const String & line(unsigned int index) const;
  //! @brief Append a line (it should not contain end-of-line characters). @param line text of the line
  void addLine(const String & line) { _lines.push_back(line); changed(); }
  //! @brief Replace line number index. @param index 0 to nLines()-1 @param line new text @throws FileException if index>=nLines()
  void setLine(unsigned int index, const String & line);
  //! @brief The whole text: the lines joined with "\n" (no final end of line). @return text
  String text() const;
  //! @brief Replace the lines by those of a text (split at "\n"; "\r\n" accepted). @param text text
  void setText(const String & text);
  //! @brief Remove all lines held (the file is not changed).
  void clear() override { _lines.clear(); changed(); }

  //! @}

  //! @name File operations
  //! @{

  //! @brief Load the whole file, replacing the lines held. @throws FileException if the file cannot be read (lines then unchanged)
  virtual void read();
  //! @brief Replace the content of the file by the lines held. @throws FileException if the file cannot be written
  virtual void write();
  //! @brief Add the lines held at the end of the file (created if needed). @throws FileException if the file cannot be written
  virtual void append();

  //! @brief Load the whole file (read(); text files have no lazy mode). @param loading ignored
  void load(Loading loading = Loading::kEager) override;
  //! @brief Remove the lines held (the file is not changed).
  void unload() override;
  //! @brief Replace the content of the file by the lines held (write()).
  void save() override { write(); }

  //! @}

  //! @brief Print the path, format, access, state and number of lines.
  void print() const override;

protected:

  //! @brief Write the lines held to the open stream. @param source caller name @throws FileException on error
  void writeLines(const char * source);
  //! @brief Called after every change of the lines held (derived classes keep their own structures up to date).
  virtual void changed() {}

  std::vector<String> _lines;  //!< lines of text

};

} // namespace CAP

#endif
