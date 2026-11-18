/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TableStatistics.hpp
//! @brief TableStatistics: summaries, covariances and correlations of the columns of a table.
#ifndef DATUM_MATH_TableStatistics
#define DATUM_MATH_TableStatistics
#include <vector>
#include "String.hpp"
#include "Table2D.hpp"

namespace Datum
{
namespace MATH
{

//!
//! @brief Statistics of the columns of a table (Table2D): summaries, covariance and correlation matrices.
//!
//! Missing values (NaN) are left out: a column summary uses the finite values of the column, a covariance or
//! correlation of two columns the rows where both are finite (pairwise complete observations), and Report n gives the
//! number of rows used. Infinite values count as missing.
//!
//! Correlations: Pearson (linear), Spearman (Pearson correlation of the ranks, ties given their average rank) or
//! Kendall (tau-b, which corrects for ties). The p-value of a Pearson or Spearman correlation r of n pairs is that of
//! t = r sqrt((n-2)/(1-r^2)) for a Student distribution of n-2 degrees of freedom (two-sided); for Kendall, the
//! normal approximation of tau-b. The uncertainty given is the standard error sqrt((1-r^2)/(n-2)).
//!
//! @code
//!   Datum::Table2D r = Datum::TableStatistics::correlationMatrix(table);   // all the columns, Pearson
//!   double rho = Datum::MATH::TableStatistics::correlation(table, 0, 3, Datum::MATH::TableStatistics::kSpearman).value;
//! @endcode
//!
class TableStatistics
{
public:

  //! @brief Kinds of correlation coefficients.
  enum Method { kPearson, kSpearman, kKendall };

  //! @brief Summary of one column.
  struct Summary
  {
    String name;                  //!< name of the column
    unsigned int n       = 0;     //!< finite values
    unsigned int missing = 0;     //!< NaN or infinite values
    double mean      = 0;         //!< mean
    double stdDev    = 0;         //!< standard deviation (n-1 in the denominator)
    double meanError = 0;         //!< standard error of the mean (stdDev/sqrt(n))
    double minimum   = 0;         //!< smallest value
    double median    = 0;         //!< median
    double maximum   = 0;         //!< largest value
    double sum       = 0;         //!< sum of the values
  };

  //! @brief A correlation (or covariance) of two columns.
  struct Result
  {
    double value  = 0;     //!< coefficient (NaN if it cannot be computed: fewer than 2 pairs, a constant column)
    double error  = 0;     //!< standard error (NaN if fewer than 3 pairs)
    double pValue = 1;     //!< p-value of the hypothesis of no correlation (two-sided)
    unsigned int n = 0;    //!< pairs used
  };

  //! @brief Name of a method ("Pearson", "Spearman", "Kendall"). @param method method @return name
  static String methodName(Method method);
  //! @brief Method of a name (any case; "" or unknown: Pearson). @param name name @return method
  static Method methodFromName(const String & name);

  //! @brief Summary of a column. @param table table @param column column @return summary
  //! @throws MathException if the column does not exist
  static Summary summarize(const Table2D & table, unsigned int column);
  //!
  //! @brief Summaries of the columns as a table: one row per column of the table, in order, and the columns n,
  //! missing, mean, stdDev, meanError, minimum, median, maximum, sum.
  //! @param table table @return summary table (named "<name>_summary")
  //!
  static Table2D summaryTable(const Table2D & table);

  //! @brief Covariance of two series (pairs with a NaN left out; n-1 in the denominator). @param x first @param y second
  //! @return covariance (value, n; no error or p-value)
  static Result covariance(const std::vector<double> & x, const std::vector<double> & y);
  //! @brief Correlation of two series (pairs with a NaN left out). @param x first @param y second @param method method
  //! @return correlation
  static Result correlation(const std::vector<double> & x, const std::vector<double> & y, Method method=kPearson);
  //! @brief Correlation of two columns. @param table table @param a first column @param b second column
  //! @param method method @return correlation @throws MathException if a column does not exist
  static Result correlation(const Table2D & table, unsigned int a, unsigned int b, Method method=kPearson);

  //!
  //! @brief Matrix of the covariances of columns: element (i,j) is the covariance of columns[i] and columns[j].
  //! @param table table @param columns columns (empty: all) @return square table, its columns named as the columns
  //! of the table (named "<name>_covariance")
  //!
  static Table2D covarianceMatrix(const Table2D & table, const std::vector<unsigned int> & columns = {});
  //!
  //! @brief Matrix of the correlations of columns: element (i,j) is the correlation of columns[i] and columns[j]
  //! (1 on the diagonal, NaN where it cannot be computed).
  //! @param table table @param columns columns (empty: all) @param method method @param pValues if not null, set to
  //! the matrix of the p-values @return square table, its columns named as those of the table (named
  //! "<name>_<method>")
  //!
  static Table2D correlationMatrix(const Table2D & table, const std::vector<unsigned int> & columns = {},
                                         Method method=kPearson, Table2D * pValues=nullptr);

  //! @brief Ranks of values (1 to n, ties the average of their ranks; NaN stays NaN). @param values values @return ranks
  static std::vector<double> ranks(const std::vector<double> & values);
};

} // namespace MATH
} // namespace Datum

#endif // DATUM_MATH_TableStatistics
