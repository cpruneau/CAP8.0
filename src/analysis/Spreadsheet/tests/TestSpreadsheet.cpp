/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestSpreadsheet.cpp
//! @brief Tests of the Spreadsheet module: reading and writing every format, statistics, plots, the task.
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include "DataPointsVector.hpp"
#include "DatumFile.hpp"
#include "Histogram.hpp"
#include "SpreadsheetAnalyzer.hpp"
#include "Table2D.hpp"
#include "TablePlots.hpp"
#include "TableReader.hpp"
#include "TableStatistics.hpp"
#include "TableWriter.hpp"

using namespace Datum;

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

void writeText(const std::string & fileName, const std::string & text)
{
  std::ofstream out(fileName,std::ios::binary);
  out << text;
}

MATH::Table2D sample()
{
  MATH::Table2D t("sample");
  t.addRow({ 1.0, 2.0,  10.0 });
  t.addRow({ 2.0, 4.1,  20.0 });
  t.addRow({ 3.0, 5.9,  std::nan("") });
  t.addRow({ 4.0, 8.2,  40.0 });
  t.addRow({ 5.0, 9.8,  50.0 });
  t.setColumnNames({ "time", "energy, GeV", "count \"n\"" });
  return t;
}

bool same(const MATH::Table2D & a, const MATH::Table2D & b, double tolerance, bool names=true)
{
  if (a.nRows()!=b.nRows() || a.nColumns()!=b.nColumns()) return false;
  for (unsigned int i=0; i<a.nRows(); i++)
    for (unsigned int j=0; j<a.nColumns(); j++)
      if (!near(a(i,j),b(i,j),tolerance)) return false;
  if (names) for (unsigned int j=0; j<a.nColumns(); j++) if (!a.columnName(j).equalTo(b.columnName(j))) return false;
  return true;
}

void testTable2D()
{
  MATH::Table2D t = sample();
  expect(t.columnIndex("time")==0 && t.columnIndex("count \"n\"")==2, "columnIndex");
  t.addColumn({ 0, 1, 2, 3, 4 },"k");
  expect(t.nColumns()==4 && t.columnName(3).equalTo("k") && t(4,3)==4, "addColumn");
  t.removeColumn(1);
  expect(t.nColumns()==3 && t.columnName(1).equalTo("count \"n\"") && t(1,0)==2, "removeColumn");
  MATH::Table2D u("u", 2, 2, 1.0);
  expect(u.columnName(1).equalTo("c1") && !u.hasColumnNames(), "default names");
  t.saveBinary("test_spreadsheet.dtb");
  MATH::Table2D back;
  back.loadBinary("test_spreadsheet.dtb");
  expect(same(t,back,0.0), "binary round trip with names (v2)");
}

void testFormats()
{
  MATH::Table2D t = sample();
  TableWriter writer;
  TableReader reader;
  const char * files[] = { "test_spreadsheet.csv", "test_spreadsheet.tsv", "test_spreadsheet.ssv", "test_spreadsheet.txt",
                           "test_spreadsheet.dtb", "test_spreadsheet.datum" };
  for (const char * file : files)
    {
    writer.write(file,t);
    MATH::Table2D back;
    TableReader::Report report = reader.read(file,back);
    // blank separated: blanks of the names become '_'
    bool blank = std::string(file).find(".txt")!=std::string::npos;
    bool ok = same(t,back,1e-9,!blank);
    if (blank) ok = ok && back.columnName(1).equalTo("energy,_GeV");
    expect(ok, std::string("round trip ") + file + " (" + report.summary().data() + ")");
    expect(report.header, std::string("header found in ") + file);
    }
  // xlsx: a zip file starting with the local header
  writer.write("test_spreadsheet.xlsx",t);
  std::ifstream in("test_spreadsheet.xlsx",std::ios::binary);
  char magic[4] = { 0, 0, 0, 0 };
  in.read(magic,4);
  expect(magic[0]=='P' && magic[1]=='K' && magic[2]==3 && magic[3]==4, "xlsx is a zip archive");
  bool thrown = false;
  try { MATH::Table2D x; reader.read("test_spreadsheet.xlsx",x); } catch (const Exception &) { thrown = true; }
  expect(thrown, "xlsx is not read");
}

void testReaderCases()
{
  TableReader reader;
  MATH::Table2D t;
  // comments, blank lines, quoted fields with commas and line breaks, missing values, trailing separators, BOM
  writeText("test_case1.csv", "\xEF\xBB\xBF# a comment\n\n\"x\",\"y, with comma\",\"multi\nline\"\n1,2,3,\n4,,NA\n7,8,9\n");
  TableReader::Report r = reader.read("test_case1.csv",t);
  expect(t.nRows()==3 && t.nColumns()==3, "csv size " + std::string(r.summary().data()));
  expect(t.columnName(1).equalTo("y, with comma") && t.columnName(2).equalTo("multi\nline"), "csv quoted names");
  expect(std::isnan(t(1,1)) && std::isnan(t(1,2)) && t(2,2)==9 && r.missing==2, "csv missing values");
  // blank separated without header, ragged rows
  writeText("test_case2.dat", "  1  2   3\n4 5\n\t6\t7  8 \n");
  r = reader.read("test_case2.dat",t);
  expect(t.nRows()==3 && t.nColumns()==3 && !r.header && std::isnan(t(1,2)) && r.padded==1 && t(2,1)==7, "blank ragged");
  expect(r.format==TableFormat::kBlank, "blank detected");
  // tab separated, empty fields kept
  writeText("test_case3.txt", "a\tb\tc\n1\t\t3\n");
  r = reader.read("test_case3.txt",t);
  expect(r.format==TableFormat::kTab && t.nColumns()==3 && std::isnan(t(0,1)) && t(0,2)==3, "tab detected, empty field");
  // semicolons and decimal commas
  writeText("test_case4.csv", "a;b\n1,5;2,25\n");
  TableReader semicolon;
  semicolon.options().format = TableFormat::kAuto;
  writeText("test_case4.txt", "a;b\n1,5;2,25\n");
  r = semicolon.read("test_case4.txt",t);
  expect(r.format==TableFormat::kSemicolon && near(t(0,0),1.5) && near(t(0,1),2.25), "semicolon with decimal commas");
  // non-numeric cells
  writeText("test_case5.csv", "a,b\n1,abc\n2,3\n");
  r = reader.read("test_case5.csv",t);
  expect(r.nonNumeric==1 && std::isnan(t(0,1)), "non-numeric cell: " + std::string(r.firstNonNumeric.data()));
  TableReader strict;
  strict.options().strict = true;
  bool thrown = false;
  try { strict.read("test_case5.csv",t); } catch (const Exception &) { thrown = true; }
  expect(thrown && t.nRows()==2, "strict throws, table unchanged");
  // forced header of numeric names
  writeText("test_case6.csv", "2019,2020\n1,2\n");
  TableReader forced;
  forced.options().header = TableReader::kHeader;
  forced.read("test_case6.csv",t);
  expect(t.nRows()==1 && t.columnName(0).equalTo("2019"), "forced header");
}

void testStatistics()
{
  MATH::Table2D t("lin");
  for (int i=0; i<20; i++) t.addRow({ double(i), 2.0*i+1.0, double((i*7)%11), std::exp(0.2*i) });
  t.setColumnNames({ "x", "y", "z", "e" });
  TableStatistics::Result r = TableStatistics::correlation(t,0,1);
  expect(near(r.value,1.0) && r.n==20 && r.pValue<1e-10, "pearson of a line");
  expect(near(TableStatistics::correlation(t,0,3,TableStatistics::kSpearman).value,1.0), "spearman of a monotonic function");
  expect(near(TableStatistics::correlation(t,0,3,TableStatistics::kKendall).value,1.0), "kendall of a monotonic function");
  expect(TableStatistics::correlation(t,0,3).value<0.95, "pearson of an exponential below 1");
  MATH::Table2D m = TableStatistics::correlationMatrix(t);
  expect(m.nRows()==4 && near(m(2,2),1.0) && near(m(1,2),m(2,1)) && m.columnName(3).equalTo("e"), "correlation matrix");
  MATH::Table2D c = TableStatistics::covarianceMatrix(t,{ 0, 1 });
  expect(near(c(0,0),35.0) && near(c(0,1),70.0), "covariance");
  TableStatistics::Summary s = TableStatistics::summarize(sample(),2);
  expect(s.n==4 && s.missing==1 && near(s.mean,30.0) && near(s.median,30.0), "summary with a missing value");
  std::vector<double> ranks = TableStatistics::ranks({ 3, 1, 3, 2 });
  expect(near(ranks[0],3.5) && near(ranks[1],1) && near(ranks[2],3.5) && near(ranks[3],2), "ranks with ties");
}

void testPlots()
{
  MATH::Table2D t = sample();
  MATH::DataPointsVector * xy = TablePlots::xy(t,0,2);
  expect(xy->size()==4 && xy->name().equalTo("count \"n\"_vs_time"), "xy skips missing values");
  delete xy;
  MATH::DataPointsVector * bars = TablePlots::bars(t,1);
  expect(bars->size()==5 && near(bars->point(0).xError(),0.4,1e-6), "bars at the row index");
  delete bars;
  MATH::Histogram * h = TablePlots::distribution(t,1,4);
  expect(h->size()==4, "distribution bins");
  delete h;
  MATH::Histogram * map = TablePlots::matrixMap(TableStatistics::correlationMatrix(t));
  expect(map->rank()==2 && map->size()==9, "matrix map");
  delete map;
}

void testTask()
{
  writeText("test_task.csv", "a,b,c\n1,2,3\n2,4,1\n3,6,2\n4,8,,\n5,10,5\n");
  SpreadsheetAnalyzer::registerProcessor("addSum", [](MATH::Table2D & table, SpreadsheetAnalyzer &)
    {
    std::vector<double> sum(table.nRows());
    for (unsigned int i=0; i<table.nRows(); i++) sum[i] = table(i,0)+table(i,1);
    table.addColumn(sum,"sum");
    });
  SpreadsheetAnalyzer task;
  task.setName("Sheet");
  task.setConfigurationPath("Sheet");
  task.setDefaultConfiguration();
  Configuration & c = task.configuration();
  c.addProperty("Sheet:TABLE:INPUT:PATH","./");
  c.addProperty("Sheet:TABLE:INPUT:FILE","test_task.csv");
  c.addProperty("Sheet:TABLE:PROCESS:FUNCTIONS","dropMissingRows, addSum");
  c.addProperty("Sheet:TABLE:PLOTS:XY","b:a, sum:a:c");
  c.addProperty("Sheet:TABLE:PLOTS:BARS","c");
  c.addProperty("Sheet:TABLE:PLOTS:DISTRIBUTIONS","a");
  c.addProperty("Sheet:TABLE:OUTPUT:PATH","./");
  c.addProperty("Sheet:TABLE:OUTPUT:FILE","test_task.datum");
  c.addProperty("Sheet:TABLE:OUTPUT:TABLE_FILE","test_task_out.xlsx");
  task.execute();
  const MATH::Table2D & t = task.table();
  expect(t.nRows()==4 && t.nColumns()==4 && t.columnName(3).equalTo("sum") && t(3,3)==15, "task processed the table");
  expect(task.results().get<MATH::Table2D>("correlations/Pearson")!=nullptr, "task correlations");
  expect(task.results().get<MATH::DataPointsVector>("plots/sum_vs_a")!=nullptr, "task xy plot");
  expect(task.results().get<MATH::DataPointsVector>("plots/c_bars")!=nullptr, "task bars");
  DatumFile in("test_task.datum",File::kReadOnly);
  in.read();
  expect(in.get<MATH::Table2D>("summary")!=nullptr && in.get<MATH::Histogram>("plots/a_distribution")!=nullptr, "task output file");
  MATH::Table2D back;
  TableReader().read("test_task.datum",back);   // the first table of the file: the processed table
  expect(back.nColumns()==4, "table read back from the Datum file");
}
} // namespace

int main()
{
  try
    {
    testTable2D();
    testFormats();
    testReaderCases();
    testStatistics();
    testPlots();
    testTask();
    }
  catch (const std::exception & e)
    {
    std::cout << "Exception: " << e.what() << std::endl;
    failures++;
    }
  std::cout << checks-failures << "/" << checks << " checks passed" << std::endl;
  return failures==0 ? 0 : 1;
}
