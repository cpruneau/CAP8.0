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
//! @file DataPane.hpp
//! @brief DataPane: pane of the CapGui window showing data points or a histogram as a table.
#ifndef CAP__GUI__DataPane
#define CAP__GUI__DataPane
#include <vector>
#include "String.hpp"

namespace CAP
{
  namespace MATH
  {
  class DataPoints;
  }

  namespace GUI
  {

  //!
  //! @brief Pane of the CapGui window showing the values of a set of data points (or of a histogram) as a table.
  //!
  //! A heading line gives the name of the object, its kind and its number of points; the table has one row per
  //! point and the columns: index, each coordinate and its uncertainty, the value and its statistical and systematic
  //! uncertainties. The header row of the table holds the names of the variables (the axis titles of a histogram, or
  //! x, y, ...) and stays visible while the table scrolls. Only the visible rows are drawn, so very large tables scroll
  //! as fast as small ones. Columns of uncertainties that are zero for every point are left out. Copy puts the whole
  //! table (the columns shown) on the clipboard, as tab-separated text.
  //!
  //! If fits are attached to the data (MATH::DataPoints::addFit()), the heading gives the quality of each, and the
  //! table has, after the uncertainties, the value f(x) of each fitted function at the point (dimmed outside the
  //! range of the fit) and, unless turned off (setShowPulls()), the pull (value - f(x))/(statistical uncertainty).
  //!
  //! The data points are not owned: they must outlive the pane (in CapApp they belong to open, or retired, files).
  //!
  class DataPane
  {
  public:

  //! @brief Constructor. @param name name of the pane, unique in the window (e.g. "Data 1")
  explicit DataPane(const String & name);

  //! @brief Show data points. @param data data points (not owned; may be null) @param label name shown in the tab and the heading
  void setData(const MATH::DataPoints * data, const String & label);
  const MATH::DataPoints * data() const { return _data; }   //!< Data points shown (not owned), or nullptr
  const String & label() const { return _label; }   //!< Name shown in the tab and the heading
  //! @brief Name of the pane (identifies its window). @return name
  const String & name() const { return _name; }
  //! @brief Title of its window: the label shown in the tab, the name identifying the window. @return title
  String windowTitle() const;

  //! @brief Draw the pane in the current ImGui window.
  void render();
  //! @brief The table as tab-separated text, header row first. @return text
  String text() const;
  //! @brief Whether the pulls of the fits are shown (when there are fits). @return true if so
  bool showPulls() const { return _showPulls; }
  //! @brief Show the pulls of the fits or not. @param v show them
  void setShowPulls(bool v) { _showPulls = v; }

  protected:

  String _name;   //!< name of the pane (identifies its window)
  const MATH::DataPoints * _data;   //!< data points shown (not owned)
  String _label;   //!< name shown in the tab and the heading
  std::vector<bool> _shown;   //!< columns shown (uncertainties zero for every point are not), set by setData()
  bool _showPulls = true;   //!< the pulls of the fits are shown
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__DataPane
