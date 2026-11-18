/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestTableStatistics.cpp
//! @brief Tests of TableStatistics: summaries, covariances, Pearson, Spearman and Kendall correlations, ranks.
#include <cmath>
#include <iostream>
#include <string>
#include "Table2D.hpp"
#include "TableStatistics.hpp"

using namespace Datum::MATH;

namespace
{
int failures = 0;
int checks = 0;

void expect(bool condition, const std::string & what)
{
  checks++;
  if (!condition) { failures++; std::cout << "FAILED: " << what << std::endl; }
}

bool near(double a, double b, double tolerance=1e-9)
{
  if (std::isnan(a) && std::isnan(b)) return true;
  return std::fabs(a-b) <= tolerance*(1.0+std::fabs(b));
}

Table2D sample()
{
  Table2D t("sample");
  t.addRow({ 1.0, 2.0,  10.0 });
  t.addRow({ 2.0, 4.1,  20.0 });
  t.addRow({ 3.0, 5.9,  std::nan("") });
  t.addRow({ 4.0, 8.2,  40.0 });
  t.addRow({ 5.0, 9.8,  50.0 });
  t.setColumnNames({ "time", "energy", "count" });
  return t;
}

void testStatistics()
{
  Table2D t("lin");
  for (int i=0; i<20; i++) t.addRow({ double(i), 2.0*i+1.0, double((i*7)%11), std::exp(0.2*i) });
  t.setColumnNames({ "x", "y", "z", "e" });
  TableStatistics::Result r = TableStatistics::correlation(t,0,1);
  expect(near(r.value,1.0) && r.n==20 && r.pValue<1e-10, "pearson of a line");
  expect(near(TableStatistics::correlation(t,0,3,TableStatistics::kSpearman).value,1.0), "spearman of a monotonic function");
  expect(near(TableStatistics::correlation(t,0,3,TableStatistics::kKendall).value,1.0), "kendall of a monotonic function");
  expect(TableStatistics::correlation(t,0,3).value<0.95, "pearson of an exponential below 1");
  Table2D m = TableStatistics::correlationMatrix(t);
  expect(m.nRows()==4 && near(m(2,2),1.0) && near(m(1,2),m(2,1)) && m.columnName(3).equalTo("e"), "correlation matrix");
  Table2D c = TableStatistics::covarianceMatrix(t,{ 0, 1 });
  expect(near(c(0,0),35.0) && near(c(0,1),70.0), "covariance");
  TableStatistics::Summary s = TableStatistics::summarize(sample(),2);
  expect(s.n==4 && s.missing==1 && near(s.mean,30.0) && near(s.median,30.0), "summary with a missing value");
  std::vector<double> ranks = TableStatistics::ranks({ 3, 1, 3, 2 });
  expect(near(ranks[0],3.5) && near(ranks[1],1) && near(ranks[2],3.5) && near(ranks[3],2), "ranks with ties");
}

void testMissingAndConstant()
{
  // pairs with a missing value are left out; a constant column has no correlation
  TableStatistics::Result r = TableStatistics::correlation({ 1, 2, std::nan(""), 4 }, { 2, 4, 6, 8 });
  expect(r.n==3 && near(r.value,1.0), "pairs with NaN left out");
  expect(std::isnan(TableStatistics::correlation({ 1, 2, 3 }, { 5, 5, 5 }).value), "constant column: NaN");
  Table2D s = TableStatistics::summaryTable(sample());
  expect(s.nRows()==3 && s.columnIndex("median")==6 && near(s(1,2),5.999999999999999,1e-9), "summary table");
  expect(TableStatistics::methodFromName("kendall")==TableStatistics::kKendall && TableStatistics::methodName(TableStatistics::kSpearman)=="Spearman", "method names");
}
} // namespace

int main()
{
  try
    {
    testStatistics();
    testMissingAndConstant();
    }
  catch (const std::exception & e)
    {
    std::cout << "Exception: " << e.what() << std::endl;
    failures++;
    }
  std::cout << checks-failures << "/" << checks << " checks passed" << std::endl;
  return failures==0 ? 0 : 1;
}
