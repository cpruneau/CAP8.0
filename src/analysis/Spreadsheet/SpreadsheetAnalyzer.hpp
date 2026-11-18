/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SpreadsheetAnalyzer.hpp
//! @brief SpreadsheetAnalyzer: task analyzing a table of data (statistics, correlations, plots, user processing).
#ifndef DATUM__SpreadsheetAnalyzer
#define DATUM__SpreadsheetAnalyzer
#include <functional>
#include <vector>
#include "Task.hpp"
#include "Folder.hpp"
#include "Table2D.hpp"

namespace Datum
{

//!
//! @brief Task of the Spreadsheet module: reads a table of data, processes it, computes its statistics and
//! correlations, makes plots of its columns, and saves the results.
//!
//! execute() reads the table (TableReader: text, CSV, binary; INPUT:...), keeps the columns asked for
//! (SELECT:COLUMNS), applies the processing functions (PROCESS:FUNCTIONS, see below), then fills results() with:
//!  - "table": the table, as processed;
//!  - "summary" (SUMMARY): the summary of each column (MATH::TableStatistics::summaryTable());
//!  - "correlations/<method>", ".../<method>_pValues", ".../<method>_map", "correlations/covariance"
//!    (CORRELATIONS:USE): matrices of the columns of CORRELATIONS:COLUMNS (all if empty) and the map of the correlations;
//!  - "plots/<y>_vs_<x>" (PLOTS:XY), "plots/<value>_bars" (PLOTS:BARS), "plots/<column>_distribution"
//!    (PLOTS:DISTRIBUTIONS): see TablePlots.
//! It then writes results() to the Datum file OUTPUT:FILE (if set) and the processed table to OUTPUT:TABLE_FILE (if
//! set; its format from OUTPUT:TABLE_FORMAT or the extension: .csv, .tsv, .txt, .dtb, .xlsx, ...).
//!
//! Columns are given by name (the header row of the file) or by index (0, 1, ...), separated by commas. A plot of
//! PLOTS:XY is "y:x" or "y:x:yError" or "y:x:yError:xError"; a bar chart of PLOTS:BARS is "value" (bars at the row
//! index) or "value:x".
//!
//! Configuration (keys of the task, objectType TABLE), with their defaults:
//! @code
//!   INPUT:PATH            "DEFAULT"   directory of the input (DEFAULT: $DATUM_DATA_IMPORT_PATH, or as given)
//!   INPUT:FILE            "NONE"      file of the table
//!   INPUT:FORMAT          "auto"      auto, blank, tab, comma, csv, semicolon, binary
//!   INPUT:HEADER          "auto"      auto, yes, no: whether the first line of data holds the column names
//!   INPUT:COMMENT         "#"         comment character ("" for none)
//!   INPUT:SKIP_LINES      0           lines skipped at the start of the file
//!   INPUT:TABLE           ""          name (binary file) or path (Datum file) of the table; "": the first one
//!   SELECT:COLUMNS        ""          columns kept ("": all)
//!   PROCESS:FUNCTIONS     ""          processing functions applied in order (registered names)
//!   SUMMARY               true        summary of the columns
//!   CORRELATIONS:USE      true        correlation and covariance matrices
//!   CORRELATIONS:METHOD   "Pearson"   Pearson, Spearman, Kendall
//!   CORRELATIONS:COLUMNS  ""          columns of the matrices ("": all)
//!   PLOTS:XY              ""          e.g. "energy:time, pz:pt:pzError"
//!   PLOTS:BARS            ""          e.g. "counts, yield:centrality"
//!   PLOTS:DISTRIBUTIONS   ""          e.g. "energy, pt"
//!   PLOTS:BINS            0           bins of the distributions (0: chosen from the number of values)
//!   OUTPUT:PATH           "DEFAULT"   directory of the outputs (DEFAULT: $DATUM_HISTOS_EXPORT_PATH, or as given)
//!   OUTPUT:FILE           "NONE"      Datum file of the results
//!   OUTPUT:TABLE_FILE     "NONE"      file of the processed table
//!   OUTPUT:TABLE_FORMAT   "auto"      format of that file (auto: from its extension)
//! @endcode
//!
//! <b>Processing.</b> A processing function changes the table in place (adds columns computed from others, removes
//! rows, rescales, ...). Functions are registered by name, from any library or program, and named in
//! PROCESS:FUNCTIONS; a derived class can instead override process(). Built in:
//!  - "dropMissingRows": remove the rows holding a missing value (NaN);
//!  - "standardize": replace each column by (value - mean)/stdDev.
//! @code
//!   Datum::SpreadsheetAnalyzer::registerProcessor("addRatio", [](Datum::MATH::Table2D & t, Datum::SpreadsheetAnalyzer &)
//!     {
//!     std::vector<double> a = t.column(t.columnIndex("a")), b = t.column(t.columnIndex("b")), r(a.size());
//!     for (size_t k=0; k<a.size(); k++) r[k] = a[k]/b[k];
//!     t.addColumn(r, "a_over_b");
//!     });
//! @endcode
//!
class SpreadsheetAnalyzer
:
public Task
{
public:

  //! @brief Processing function: changes the table in place. @param table table @param task the task running it
  using Processor = std::function<void(MATH::Table2D & table, SpreadsheetAnalyzer & task)>;

  //! @brief Register a processing function (replacing one of the same name). @param name name @param processor function
  static void registerProcessor(const String & name, const Processor & processor);
  //! @brief Whether a processing function is registered. @param name name @return true if so
  static bool hasProcessor(const String & name);
  //! @brief Names of the registered processing functions. @return names, in order of registration
  static std::vector<String> processorNames();
  //! @brief Run a registered processing function. @param name name @param table table @param task task
  //! @throws Exception if there is no function of that name
  static void runProcessor(const String & name, MATH::Table2D & table, SpreadsheetAnalyzer & task);

  //! @brief Constructor.
  SpreadsheetAnalyzer();
  //! @brief Destructor.
  virtual ~SpreadsheetAnalyzer() {}
  //! @brief Default configuration (see the class description).
  virtual void setDefaultConfiguration() override;
  //! @brief Read, process, analyze and save, as configured. @throws Exception if the input cannot be read or the
  //! configuration names columns or functions that do not exist
  virtual void execute() override;

  //!
  //! @brief Analyze a table (without files): keep the columns selected, process, then fill results() as execute().
  //! @param table table (copied)
  //!
  void analyze(const MATH::Table2D & table);
  //! @brief Apply the processing functions of PROCESS:FUNCTIONS in order. @param table table changed in place
  virtual void process(MATH::Table2D & table);

  //! @brief The table analyzed last, as processed. @return table
  const MATH::Table2D & table() const { return _table; }
  //! @brief Results of the last analysis (see the class description). @return folder
  const Folder & results() const { return _results; }
  //! @brief Results of the last analysis (changeable, e.g. by processing functions adding objects). @return folder
  Folder & results() { return _results; }

  //! @brief Index of a column given by name or by index. @param table table @param column name or index
  //! @return index @throws Exception if there is no such column
  static unsigned int columnIndex(const MATH::Table2D & table, const String & column);
  //! @brief Indices of columns given by names or indices separated by commas ("" : none). @param table table
  //! @param columns list @return indices @throws Exception if a column does not exist
  static std::vector<unsigned int> columnIndices(const MATH::Table2D & table, const String & columns);

protected:

  MATH::Table2D _table;   //!< table analyzed last
  Folder _results;        //!< results of the last analysis

  //! @brief Configuration value of the task (objectType TABLE). @param key key @return value
  String stringValue(const char * key) const;
  //! @brief Configuration value of the task (objectType TABLE). @param key key @param defaultValue default @return value
  bool boolValue(const char * key, bool defaultValue) const;
  //! @brief Configuration value of the task (objectType TABLE). @param key key @param defaultValue default @return value
  int intValue(const char * key, int defaultValue) const;
};

} // namespace Datum

#endif // DATUM__SpreadsheetAnalyzer
