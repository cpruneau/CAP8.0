/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SpreadsheetPane.hpp
//! @brief SpreadsheetPane: pane of the DatumGui window showing a table of data (Spreadsheet module), with its plots.
#ifndef DATUM__GUI__SpreadsheetPane
#define DATUM__GUI__SpreadsheetPane
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "String.hpp"
#include "Table2D.hpp"

namespace Datum
{
  namespace GUI
  {

  //!
  //! @brief Pane of the DatumGui window showing a table of data (MATH::Table2D, e.g. read from a CSV file by
  //! Datum::TableReader) and making plots of its columns.
  //!
  //! The table scrolls vertically and horizontally; its top row (the names of the fields, or variables) and its left
  //! column (the row index, 0 to n-1, or the names of the rows of a derived table such as a correlation matrix) stay in
  //! view. Only the visible rows are drawn, so tables of millions of rows scroll as fast as small ones. Missing values
  //! (NaN) show as empty cells.
  //!
  //! <b>Columns.</b> Clicking the name of a column selects it (Ctrl/Cmd-click adds or removes one, Shift-click a range);
  //! selected columns are highlighted. One column can be marked as X ("Use as X" in the menu), shown with "[x]"; another
  //! as the uncertainty of y, shown with "[±]".
  //!
  //! <b>Right-click menu.</b> Over the table (a right-click on a column name selects it first):
  //!  - Plot ... vs X, vs another column or vs the row index: points y(x), one set per selected column;
  //!  - Bar chart: one bar per row (at the row index, or at X), several columns side by side;
  //!  - Distribution: histogram of the values of each selected column;
  //!  - Correlations (Pearson, Spearman, Kendall) of the selected columns (all if fewer than two are selected): the
  //!    matrix in a new table pane and its map in the canvas;
  //!  - Summary statistics: a new table pane with one row per column (n, mean, standard deviation, ...);
  //!  - Overlay: plots are added to the graph instead of replacing its contents.
  //! The plots go to the canvas pane of the table (canvasWindow()), docked beside it the first time, with all the
  //! functions of the canvases (zoom, styles, fits, saving to SVG or PDF, undo).
  //!
  //! The pane only asks for the plots (onPlot) and for saving (onSaveAs); DatumApp makes them.
  //!
  class SpreadsheetPane
  {
  public:

  //! @brief What a plot request asks for
  enum PlotKind { kXY, kBars, kDistribution, kCorrelation, kSummary };

  //! @brief A plot (or derived table) asked for from the menu.
  struct PlotRequest
  {
    PlotKind kind = kXY;                 //!< what to make
    std::vector<unsigned int> columns;   //!< columns plotted (y), or of the correlation matrix
    int  x       = -1;                   //!< column of x (-1: the row index)
    int  yError  = -1;                   //!< column of the uncertainties of y (-1: none)
    int  method  = 0;                    //!< correlation: Datum::MATH::TableStatistics::Method
    bool overlay = false;                //!< add to the graph rather than replace its contents
  };

  //! @brief Constructor. @param name name of the pane, unique in the window (e.g. "Table 1")
  explicit SpreadsheetPane(const String & name);

  //!
  //! @brief Show a table (owned by the pane from now on).
  //! @param table table @param label name shown in the tab and the heading @param fileName file it was read from
  //! ("" if none) @param rowLabels names of the rows shown instead of the index (empty: the index)
  //!
  void setTable(std::unique_ptr<MATH::Table2D> table, const String & label, const String & fileName="",
                const std::vector<String> & rowLabels = {});
  //! @brief The table shown, or nullptr. @return table
  const MATH::Table2D * table() const { return _table.get(); }
  const String & label() const { return _label; }   //!< Name shown in the tab and the heading
  //! @brief Set the name shown in the tab and the heading. @param label name
  void setLabel(const String & label) { _label = label; }
  const String & fileName() const { return _fileName; }   //!< File the table was read from or saved to ("" if none)
  void setFileName(const String & fileName) { _fileName = fileName; }   //!< Set the file of the table
  const String & name() const { return _name; }   //!< Name of the pane (identifies its window)
  //! @brief Title of its window: the label shown in the tab, the name identifying the window. @return title
  String windowTitle() const;
  //! @brief "###name": identifier of its window, whatever its title. @return identifier
  std::string windowId() const { return std::string("###") + _name.data(); }
  //! @brief Identifier of the window of the canvas pane receiving its plots ("" before the first plot). @return id
  const std::string & canvasWindow() const { return _canvasWindow; }
  //! @brief Set the canvas pane receiving its plots. @param windowId identifier of its window
  void setCanvasWindow(const std::string & windowId) { _canvasWindow = windowId; }

  //! @brief Columns selected, in order. @return indices
  std::vector<unsigned int> selectedColumns() const;
  //! @brief Select columns (the others are deselected). @param columns indices
  void selectColumns(const std::vector<unsigned int> & columns);
  int  xColumn() const { return _xColumn; }   //!< Column used as x (-1: none)
  void setXColumn(int column) { _xColumn = column; }   //!< Use a column as x (-1: none)
  int  errorColumn() const { return _errorColumn; }   //!< Column of the uncertainties of y (-1: none)
  void setErrorColumn(int column) { _errorColumn = column; }   //!< Use a column as the uncertainties of y (-1: none)
  bool overlay() const { return _overlay; }   //!< Whether plots are added to the graph
  void setOverlay(bool v) { _overlay = v; }   //!< Add plots to the graph, or replace its contents

  //! @brief Draw the pane in the current ImGui window.
  void render();
  //! @brief The table as tab-separated text, header row first (the row labels as a first column, if any). @return text
  String text() const;

  //! Called for a plot asked from the menu: the pane and the request
  std::function<void(SpreadsheetPane & pane, const PlotRequest & request)> onPlot;
  //! Called by "Save Table As...": the pane
  std::function<void(SpreadsheetPane & pane)> onSaveAs;

  protected:

  String _name;                            //!< name of the pane (identifies its window)
  std::unique_ptr<MATH::Table2D> _table;   //!< table shown (owned)
  String _label;                           //!< name shown in the tab and the heading
  String _fileName;                        //!< file of the table
  std::vector<String> _rowLabels;          //!< names of the rows (empty: the index)
  std::vector<bool> _selected;             //!< selected columns
  int  _anchor = -1;                       //!< column clicked last (Shift-click selects from it)
  int  _xColumn = -1;                      //!< column used as x
  int  _errorColumn = -1;                  //!< column of the uncertainties of y
  int  _menuColumn = -1;                   //!< column under the mouse when the menu opened (-1: none)
  bool _overlay = false;                   //!< plots added to the graph
  std::string _canvasWindow;               //!< window of the canvas pane receiving the plots

  //! @brief Click on the name of a column: select it (Ctrl/Cmd: toggle, Shift: range). @param column column
  void clickColumn(int column);
  //! @brief Columns plotted: the selected ones but x, or the column under the mouse. @return indices
  std::vector<unsigned int> plottedColumns() const;
  //! @brief Draw the right-click menu.
  void renderMenu();
  //! @brief Send a request to onPlot. @param request request
  void plot(PlotRequest request);
  };

  } // namespace GUI
} // namespace Datum

#endif // !DATUM__GUI__SpreadsheetPane
