/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TableWriter.hpp
//! @brief TableWriter: writes tables (MATH::Table2D) to text, CSV, Datum binary and Excel files.
#ifndef DATUM__TableWriter
#define DATUM__TableWriter
#include <string>
#include "String.hpp"
#include "Table2D.hpp"
#include "TableFormat.hpp"

namespace Datum
{

//!
//! @brief Writes a table of numbers (MATH::Table2D, with its column names) to a file.
//!
//! <b>Text</b> (kBlank, kTab, kComma, kCsv, kSemicolon): a header row with the column names (unless header is false),
//! then one line per row; numbers in the shortest form that keeps precision significant digits ("%.*g"). Missing
//! values (NaN) are written as empty fields, except in blank separated files, where an empty field cannot be seen:
//! there they are written "nan". In kCsv and kSemicolon, fields holding the separator, a quote, a line break or
//! leading/trailing blanks are quoted ("..."). In blank separated files, blanks in the names are replaced by '_', and
//! the header can be written as a comment line ("# a b c"; see commentHeader) so that programs reading only numbers
//! (Table2D::readText()) still read the file. In kSemicolon files the decimal separator is kept a point.
//!
//! <b>Binary</b>: .dtb (or any extension but .datum and .cap): one Table2D record (Table2D::saveBinary(), exact,
//! names included); .datum and .cap: a Datum file whose folder holds the table.
//!
//! <b>Excel</b> (kXlsx): an Office Open XML workbook with one sheet named after the table, the header row in bold and
//! frozen, numbers written exactly (17 significant digits), missing values as empty cells. The file is a zip archive
//! written without compression (no external library is needed). Excel's limits apply: 1048576 rows and 16384 columns.
//!
//! @code
//!   Datum::TableWriter writer;                      // format from the extension
//!   writer.write("results.xlsx", table);
//!   writer.options().format = Datum::TableFormat::kTab;
//!   writer.write("results.txt", table);             // tab separated, whatever the extension
//! @endcode
//!
class TableWriter
{
public:

  //! @brief How a file is written.
  struct Options
  {
    TableFormat::Type format = TableFormat::kAuto;   //!< format (kAuto: from the extension; .txt, .dat: blank separated)
    bool   header        = true;    //!< write the column names (text and Excel)
    bool   commentHeader = false;   //!< blank separated text: write the names as a comment line "# a b c"
    bool   index         = false;   //!< write the row index (0 to n-1) as a first column named "index" (text and Excel)
    int    precision     = 10;      //!< significant digits of the numbers in text files (1 to 17)
    String sheetName;               //!< name of the Excel sheet (empty: the name of the table)
  };

  //! @brief Constructor with the default options.
  TableWriter();
  //! @brief Constructor. @param options how the files are written
  explicit TableWriter(const Options & options) : _options(options) {}

  Options & options() { return _options; }   //!< Options (changeable)
  const Options & options() const { return _options; }   //!< Options

  //!
  //! @brief Write a table to a file (replacing it).
  //! @param fileName file @param table table
  //! @throws FileException if the file cannot be written, or the table exceeds the limits of the format
  //!
  void write(const String & fileName, const MATH::Table2D & table) const;

  //! @brief The table as text in a text format (e.g. for the clipboard). @param table table @param format text format
  //! (kAuto: kTab) @return text @throws Exception if the format is not text
  std::string text(const MATH::Table2D & table, TableFormat::Type format) const;

  //! @brief The table as the bytes of an Excel workbook (.xlsx). @param table table @return contents of the file
  //! @throws Exception if the table exceeds the limits of Excel
  std::string xlsx(const MATH::Table2D & table) const;

  //! @brief A number as written in text files. @param value value @param precision significant digits @return text
  static std::string formatNumber(double value, int precision);

protected:

  Options _options;   //!< how files are written
};

} // namespace Datum

#endif // DATUM__TableWriter
