/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TableFormat.hpp
//! @brief TableFormat: file formats of the tables of the Spreadsheet module.
#ifndef DATUM__TableFormat
#define DATUM__TableFormat
#include <vector>
#include "String.hpp"

namespace Datum
{

//!
//! @brief File formats in which tables (MATH::Table2D) are read and written by TableReader and TableWriter.
//!
//! | Format     | Extensions          | Read | Write | Contents                                                     |
//! |------------|---------------------|------|-------|--------------------------------------------------------------|
//! | kBlank     | .txt .dat           | yes  | yes   | fields separated by blanks (spaces or tabs)                  |
//! | kTab       | .tsv .tab           | yes  | yes   | fields separated by one tab each (empty fields kept)         |
//! | kComma     |                     | yes  | yes   | fields separated by commas, no quoting                       |
//! | kCsv       | .csv                | yes  | yes   | RFC 4180 CSV: commas, fields quoted with " when needed       |
//! | kSemicolon | .ssv                | yes  | yes   | as kCsv with ';' (CSV of locales with a decimal comma)       |
//! | kBinary    | .dtb .datum .cap    | yes  | yes   | Datum binary format (Table2D records, exact; see BinaryIO)   |
//! | kXlsx      | .xlsx               | no   | yes   | Excel workbook (Office Open XML), one sheet                  |
//!
//! kAuto, when reading, takes the format from the extension and, for text files whose extension says nothing about
//! the separator (.txt, .dat, any other), detects the separator from the first line of data.
//!
class TableFormat
{
public:

  //! @brief Formats of table files.
  enum Type
  {
    kAuto,        //!< from the extension of the file (and, for text, the first line of data)
    kBlank,       //!< text, blank separated
    kTab,         //!< text, tab separated
    kComma,       //!< text, comma separated (no quoting)
    kCsv,         //!< CSV (comma separated, quoted fields)
    kSemicolon,   //!< CSV with ';' as separator
    kBinary,      //!< Datum binary records
    kXlsx         //!< Excel workbook (write only)
  };

  //! @brief Format given by the extension of a file name (kAuto for .txt, .dat and unknown extensions).
  //! @param fileName file name @return format
  static Type fromFileName(const String & fileName);
  //! @brief Format of a name: "auto", "blank", "tab", "comma", "csv", "semicolon", "binary", "xlsx" (any case).
  //! @param name name @param ok set to false if the name is unknown (may be null) @return format (kAuto if unknown)
  static Type fromName(const String & name, bool * ok=nullptr);
  //! @brief Name of a format (see fromName()). @param type format @return name
  static String name(Type type);
  //! @brief Whether a format is text. @param type format @return true for kBlank, kTab, kComma, kCsv, kSemicolon
  static bool isText(Type type) { return type>=kBlank && type<=kSemicolon; }
  //! @brief Separator of a text format (' ' for kBlank, '\\t', ',', ';'). @param type format @return separator, 0 if not text
  static char separator(Type type);
  //! @brief Whether a text format quotes its fields (kCsv, kSemicolon). @param type format @return true if so
  static bool quotes(Type type) { return type==kCsv || type==kSemicolon; }
  //! @brief Extensions of the files that TableReader reads (e.g. for a file dialog). @return extensions with the dot
  static std::vector<String> readableExtensions();
  //! @brief Extensions of the files that TableWriter writes. @return extensions with the dot
  static std::vector<String> writableExtensions();
  //! @brief Extension of the files of a format (".txt", ".tsv", ".txt", ".csv", ".ssv", ".dtb", ".xlsx"). @param type
  //! format @return extension
  static String extension(Type type);
};

} // namespace Datum

#endif // DATUM__TableFormat
