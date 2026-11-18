/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TableReader.hpp
//! @brief TableReader: reads tables (MATH::Table2D) from text, CSV and Datum binary files.
#ifndef DATUM__TableReader
#define DATUM__TableReader
#include <string>
#include <vector>
#include "String.hpp"
#include "Table2D.hpp"
#include "TableFormat.hpp"

namespace Datum
{

//!
//! @brief Reads a table of numbers (MATH::Table2D, with its column names) from a file.
//!
//! <b>Text files</b> (blank, tab, comma or semicolon separated, CSV). One row per line. Lines that are blank, or whose
//! first character other than a blank is the comment character ('#' by default), are skipped; so are the first
//! skipLines lines. The separator is that of the format, or detected from the first line of data (kAuto): a tab if the
//! line has one, otherwise a comma, otherwise a semicolon, otherwise blanks. In CSV (and semicolon) files, fields may be
//! quoted with "..." (a quote inside written ""), and a quoted field may hold the separator or a line break.
//!
//! <b>Header row.</b> With header kAutoHeader, the first line of data is taken as the names of the columns if one of its
//! fields is not a number; kHeader always takes it as names, kNoHeader never. Columns without a name (fewer names than
//! columns, empty names) get the default name of Table2D ("c<column>"). Leading and trailing blanks are removed from
//! names and fields.
//!
//! <b>Cells.</b> Each field is read as a number (strtod: 1, -2.5, 1e-3, inf, nan). An empty field, or one of "NA",
//! "N/A", "NaN", "null", "-" (any case), is a missing value, stored as NaN. A field that is not a number is stored as
//! NaN too and counted in Report::nonNumeric (strict: FileException instead). The number of columns is the largest
//! number of fields of a row (empty fields at the end of a row, left by spreadsheet exports, do not count) or of names;
//! shorter rows are completed with NaN (counted in Report::padded). In a semicolon separated file, a decimal comma
//! ("1,5") is read as a decimal point.
//!
//! <b>Binary files.</b> Files written by Table2D::saveBinary() (.dtb): the table of the given name, or the first one.
//! Datum files (.datum, .cap): the table of the given path in the file, or the first one found.
//!
//! The table read replaces the contents of the table given (its name becomes the name of the file without directory
//! and extension, unless a name is set in the options); on error the table is unchanged.
//!
//! @code
//!   Datum::MATH::Table2D table;
//!   Datum::TableReader reader;
//!   Datum::TableReader::Report report = reader.read("measurements.csv", table);
//!   std::cout << report.summary() << std::endl;   // e.g. "1200 rows x 6 columns, header, 3 missing values"
//!   std::vector<double> energy = table.column(table.columnIndex("energy"));
//! @endcode
//!
class TableReader
{
public:

  //! @brief Whether the first line of data holds the names of the columns.
  enum Header { kAutoHeader, kHeader, kNoHeader };

  //! @brief How a file is read.
  struct Options
  {
    TableFormat::Type format = TableFormat::kAuto;   //!< format (kAuto: from the extension and the first line)
    Header header       = kAutoHeader;   //!< header row
    char   comment      = '#';           //!< lines starting with it are skipped (0: no comments)
    unsigned int skipLines = 0;          //!< lines skipped at the start of the file (before comments are looked for)
    bool   strict       = false;         //!< a field that is not a number throws instead of being read as NaN
    String name;                         //!< name of the table (empty: the file name without directory and extension);
                                         //!< for binary files, also the table looked for (empty: the first one)
  };

  //! @brief What was read.
  struct Report
  {
    unsigned int rows     = 0;       //!< rows read
    unsigned int columns  = 0;       //!< columns read
    bool         header   = false;   //!< the first line of data was taken as the column names
    TableFormat::Type format = TableFormat::kAuto;   //!< format used (the separator detected for kAuto)
    unsigned int missing    = 0;     //!< empty or "NA" cells (NaN)
    unsigned int nonNumeric = 0;     //!< cells that are not numbers (NaN)
    unsigned int padded     = 0;     //!< rows completed with NaN (fewer fields than columns)
    String firstNonNumeric;          //!< where the first non-numeric cell is, e.g. "line 12, column 3: 'abc'"
    //! @brief One line describing what was read. @return text
    String summary() const;
  };

  //! @brief Constructor with the default options.
  TableReader();
  //! @brief Constructor. @param options how the files are read
  explicit TableReader(const Options & options) : _options(options) {}

  Options & options() { return _options; }   //!< Options (changeable)
  const Options & options() const { return _options; }   //!< Options

  //!
  //! @brief Read a table from a file, replacing the contents of table.
  //! @param fileName file @param table table filled @return what was read
  //! @throws FileException if the file cannot be read, is of a format that cannot be read (xlsx), holds no table, or
  //! (strict) holds a non-numeric field; the table is then unchanged
  //!
  Report read(const String & fileName, MATH::Table2D & table) const;

  //!
  //! @brief Read a table from text (as the contents of a text file; e.g. pasted from the clipboard).
  //! @param text contents @param format format (kAuto: separator detected; kBinary and kXlsx are not text)
  //! @param table table filled @param source name of the text in the messages (e.g. the file name) @return what was read
  //! @throws FileException as read()
  //!
  Report readText(const std::string & text, TableFormat::Type format, MATH::Table2D & table,
                  const String & source="text") const;

  //! @brief Split a line of text into fields (no line breaks inside quotes). @param line line @param format text
  //! format (kAuto: blank) @return fields, trimmed and unquoted
  static std::vector<std::string> splitLine(const std::string & line, TableFormat::Type format);
  //! @brief Separator detected in a line of data: kTab, kComma, kSemicolon or kBlank. @param line line @return format
  static TableFormat::Type detectFormat(const std::string & line);
  //!
  //! @brief Read a field as a number. @param field text (trimmed) @param value set to the number, or NaN
  //! @return 0 if it is a number, 1 if it is a missing value (empty, NA, ...), 2 if it is not a number
  //!
  static int parseField(const std::string & field, double & value);

protected:

  Options _options;   //!< how files are read

  //! @brief Read a binary table or Datum file. @param fileName file @param table table filled @return report
  Report readBinary(const String & fileName, MATH::Table2D & table) const;
};

} // namespace Datum

#endif // DATUM__TableReader
