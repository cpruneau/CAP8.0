/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TablePlots.hpp
//! @brief TablePlots: data points and histograms made from the columns of a table, to be drawn in graphs.
#ifndef DATUM__TablePlots
#define DATUM__TablePlots
#include "String.hpp"
#include "Table2D.hpp"

namespace Datum
{
namespace MATH
{
class DataPointsVector;
class Histogram;
}

//!
//! @brief Objects made from the columns of a table (MATH::Table2D) for the graphs of Datum: y versus x points, bars,
//! distributions of a column and maps of correlation matrices.
//!
//! The objects are new (the caller owns them) and named after the columns, so that they can be stored in a Folder or
//! Datum file and shown by DatumGui or by plot files. Rows where a value used is missing (NaN, infinite) are left out.
//! A bar chart is a set of points whose x uncertainty is half the width of the bar: a graph draws it as bars when
//! GRAPHICS::GraphItem::setShowBars() is set (DatumGui does so for the bar charts it makes).
//!
class TablePlots
{
public:

  //!
  //! @brief Points (x,y) of two columns, with optional uncertainties from other columns.
  //! @param table table @param xColumn column of x (-1: the row index, named "row") @param yColumn column of y
  //! @param yErrorColumn column of the uncertainties of y (-1: none) @param xErrorColumn column of the uncertainties
  //! of x (-1: none) @return points named "<y>_vs_<x>", titled "<y> vs <x>"
  //! @throws MathException if a column does not exist
  //!
  static MATH::DataPointsVector * xy(const MATH::Table2D & table, int xColumn, unsigned int yColumn,
                                     int yErrorColumn=-1, int xErrorColumn=-1);

  //!
  //! @brief Bar chart of a column: one bar per row, at x = the row index or the value of another column.
  //! @param table table @param valueColumn column of the heights @param xColumn column of the positions (-1: the row
  //! index) @param width width of the bars as a fraction of the smallest distance between two positions (0 to 1)
  //! @param offset shift of the bars along x, as a fraction of that distance (e.g. to set the bars of several columns
  //! side by side: width 0.8/n and offsets (k - (n-1)/2)*0.8/n for column k of n)
  //! @return points named "<value>_bars" (x uncertainty: half the width of the bars)
  //! @throws MathException if a column does not exist
  //!
  static MATH::DataPointsVector * bars(const MATH::Table2D & table, unsigned int valueColumn, int xColumn=-1,
                                       double width=0.8, double offset=0.0);

  //!
  //! @brief Distribution of the values of a column (histogram of rank 1).
  //! @param table table @param column column @param nBins bins (0: chosen from the number of values, Sturges' rule)
  //! @param minimum lower edge @param maximum upper edge (minimum >= maximum: the range of the values)
  //! @return histogram named "<column>_distribution", its axis titled with the name of the column
  //! @throws MathException if the column does not exist or holds no finite value
  //!
  static MATH::Histogram * distribution(const MATH::Table2D & table, unsigned int column, unsigned int nBins=0,
                                        double minimum=0.0, double maximum=0.0);

  //!
  //! @brief Map of a square matrix (e.g. TableStatistics::correlationMatrix()): a histogram of rank 2 with one bin per
  //! element, bin (i,j) holding element (i,j), on [0,n] x [0,n] (variable i in the bin from i to i+1).
  //! @param matrix square table @param name name of the histogram (empty: that of the table)
  //! @return histogram @throws MathException if the table is not square or empty
  //!
  static MATH::Histogram * matrixMap(const MATH::Table2D & matrix, const String & name="");
};

} // namespace Datum

#endif // DATUM__TablePlots
