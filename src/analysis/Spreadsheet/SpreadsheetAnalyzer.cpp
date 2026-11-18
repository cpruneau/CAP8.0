/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SpreadsheetAnalyzer.cpp
//! @brief Implementation of SpreadsheetAnalyzer.
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include "SpreadsheetAnalyzer.hpp"
#include "DataPointsVector.hpp"
#include "DatumFile.hpp"
#include "Exceptions.hpp"
#include "FileSystem.hpp"
#include "Histogram.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "TablePlots.hpp"
#include "TableReader.hpp"
#include "TableStatistics.hpp"
#include "TableWriter.hpp"

namespace Datum
{

namespace
{
const char * kObjectType = "TABLE";

struct Registry
{
  std::mutex mutex;
  std::vector<String> names;
  std::map<std::string,SpreadsheetAnalyzer::Processor> processors;
};

Registry & registry()
{
  static Registry r;
  return r;
}

//! Items of a list separated by commas or semicolons, trimmed, empty items left out.
std::vector<std::string> items(const String & list)
{
  std::vector<std::string> result;
  std::string current;
  auto flush = [&]()
    {
    std::size_t a = current.find_first_not_of(" \t");
    std::size_t b = current.find_last_not_of(" \t");
    if (a!=std::string::npos) result.push_back(current.substr(a,b-a+1));
    current.clear();
    };
  for (char c : std::string(list.data())) { if (c==',' || c==';') flush(); else current += c; }
  flush();
  return result;
}

//! Parts of an item separated by ':'.
std::vector<std::string> parts(const std::string & item)
{
  std::vector<std::string> result;
  std::string current;
  for (char c : item)
    {
    if (c==':') { result.push_back(current); current.clear(); }
    else current += c;
    }
  result.push_back(current);
  for (auto & p : result)
    {
    std::size_t a = p.find_first_not_of(" \t"), b = p.find_last_not_of(" \t");
    p = a==std::string::npos ? std::string() : p.substr(a,b-a+1);
    }
  return result;
}

bool isNone(const String & s) { return s.isNull() || s.equalTo("NONE",true); }

//! Built-in processing functions, registered once.
bool registerBuiltIns()
{
  SpreadsheetAnalyzer::registerProcessor("dropMissingRows", [](MATH::Table2D & table, SpreadsheetAnalyzer &)
    {
    MATH::Table2D kept(table.name());
    for (unsigned int i=0; i<table.nRows(); i++)
      {
      std::vector<double> row = table.row(i);
      bool complete = true;
      for (double v : row) if (std::isnan(v)) { complete = false; break; }
      if (complete) kept.addRow(row);
      }
    if (kept.nRows()==0) { kept.resize(0,0); }
    else if (table.hasColumnNames()) kept.setColumnNames(table.columnNames());
    kept.setTitle(table.title());
    table = kept;
    });
  SpreadsheetAnalyzer::registerProcessor("standardize", [](MATH::Table2D & table, SpreadsheetAnalyzer &)
    {
    for (unsigned int c=0; c<table.nColumns(); c++)
      {
      TableStatistics::Summary s = TableStatistics::summarize(table,c);
      if (!(s.stdDev>0)) continue;
      for (unsigned int i=0; i<table.nRows(); i++) table(i,c) = (table(i,c)-s.mean)/s.stdDev;
      }
    });
  return true;
}
} // namespace

//////////////////////////////////////////////////////////////////////////
// Processing functions
//////////////////////////////////////////////////////////////////////////

void SpreadsheetAnalyzer::registerProcessor(const String & name, const Processor & processor)
{
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  std::string key(name.data());
  if (r.processors.find(key)==r.processors.end()) r.names.push_back(name);
  r.processors[key] = processor;
}

bool SpreadsheetAnalyzer::hasProcessor(const String & name)
{
  static bool builtIns = registerBuiltIns();
  (void)builtIns;
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  return r.processors.find(std::string(name.data()))!=r.processors.end();
}

std::vector<String> SpreadsheetAnalyzer::processorNames()
{
  static bool builtIns = registerBuiltIns();
  (void)builtIns;
  Registry & r = registry();
  std::lock_guard<std::mutex> lock(r.mutex);
  return r.names;
}

void SpreadsheetAnalyzer::runProcessor(const String & name, MATH::Table2D & table, SpreadsheetAnalyzer & task)
{
  static bool builtIns = registerBuiltIns();
  (void)builtIns;
  Processor processor;
    {
    Registry & r = registry();
    std::lock_guard<std::mutex> lock(r.mutex);
    auto found = r.processors.find(std::string(name.data()));
    if (found==r.processors.end())
      throw Exception(String("No processing function named ")+name,"SpreadsheetAnalyzer::runProcessor()");
    processor = found->second;
    }
  processor(table,task);
}

//////////////////////////////////////////////////////////////////////////
// Task
//////////////////////////////////////////////////////////////////////////

SpreadsheetAnalyzer::SpreadsheetAnalyzer()
:
Task(),
_table("table"),
_results("results")
{
  appendClassName("SpreadsheetAnalyzer");
  setMinimumReportLevel(Object::kInfo);
}

void SpreadsheetAnalyzer::setDefaultConfiguration()
{
  Task::setDefaultConfiguration();
  const String & taskName = name();
  auto add = [&](const char * key, auto value) { _configuration.addProperty(createKey(taskName,kObjectType,key),value); };
  add("INPUT:PATH",           "DEFAULT");
  add("INPUT:FILE",           "NONE");
  add("INPUT:FORMAT",         "auto");
  add("INPUT:HEADER",         "auto");
  add("INPUT:COMMENT",        "#");
  add("INPUT:SKIP_LINES",     int(0));
  add("INPUT:TABLE",          "");
  add("SELECT:COLUMNS",       "");
  add("PROCESS:FUNCTIONS",    "");
  add("SUMMARY",              true);
  add("CORRELATIONS:USE",     true);
  add("CORRELATIONS:METHOD",  "Pearson");
  add("CORRELATIONS:COLUMNS", "");
  add("PLOTS:XY",             "");
  add("PLOTS:BARS",           "");
  add("PLOTS:DISTRIBUTIONS",  "");
  add("PLOTS:BINS",           int(0));
  add("OUTPUT:PATH",          "DEFAULT");
  add("OUTPUT:FILE",          "NONE");
  add("OUTPUT:TABLE_FILE",    "NONE");
  add("OUTPUT:TABLE_FORMAT",  "auto");
}

String SpreadsheetAnalyzer::stringValue(const char * key) const
{
  return _configuration.valueString(createKey(name(),kObjectType,key));
}

bool SpreadsheetAnalyzer::boolValue(const char * key, bool defaultValue) const
{
  return _configuration.valueBool(createKey(name(),kObjectType,key),true,defaultValue);
}

int SpreadsheetAnalyzer::intValue(const char * key, int defaultValue) const
{
  return _configuration.valueInt(createKey(name(),kObjectType,key),true,defaultValue);
}

unsigned int SpreadsheetAnalyzer::columnIndex(const MATH::Table2D & table, const String & column)
{
  int k = table.columnIndex(column);
  if (k>=0) return unsigned(k);
  std::string text(column.data());
  char * end = nullptr;
  long index = std::strtol(text.c_str(),&end,10);
  if (!text.empty() && end && *end=='\0' && index>=0 && index<long(table.nColumns())) return unsigned(index);
  throw Exception(String("No column named ")+column+String(" in table ")+table.name(),"SpreadsheetAnalyzer::columnIndex()");
}

std::vector<unsigned int> SpreadsheetAnalyzer::columnIndices(const MATH::Table2D & table, const String & columns)
{
  std::vector<unsigned int> result;
  for (const std::string & item : items(columns)) result.push_back(columnIndex(table,String(item.c_str())));
  return result;
}

void SpreadsheetAnalyzer::execute()
{
  if (reportInfo(__FUNCTION__)) { printCR(); }
  if (_taskExecuted.size()==0) _taskExecuted.initialize("taskExecuted",1);
  _taskExecuted.increment();

  // input
  String inputFile = stringValue("INPUT:FILE");
  if (isNone(inputFile)) throw Exception("Set the input file: TABLE:INPUT:FILE",__FUNCTION__);
  String inputPath = FileSystem::configuredPath(stringValue("INPUT:PATH"),"DATUM_DATA_IMPORT_PATH");
  String fileName = inputPath.isNull() ? inputFile : FileSystem::join(inputPath,inputFile);
  TableReader::Options options;
  bool ok = true;
  options.format = TableFormat::fromName(stringValue("INPUT:FORMAT"),&ok);
  if (!ok) throw Exception(String("Unknown format: ")+stringValue("INPUT:FORMAT"),__FUNCTION__);
  String header = stringValue("INPUT:HEADER");
  options.header = header.equalTo("yes",true) || header.equalTo("true",true) ? TableReader::kHeader
                 : (header.equalTo("no",true) || header.equalTo("false",true) ? TableReader::kNoHeader : TableReader::kAutoHeader);
  String comment = stringValue("INPUT:COMMENT");
  options.comment = comment.isNull() ? 0 : comment.data()[0];
  int skip = intValue("INPUT:SKIP_LINES",0);
  options.skipLines = skip>0 ? unsigned(skip) : 0u;
  options.name = stringValue("INPUT:TABLE");
  MATH::Table2D table;
  TableReader::Report report = TableReader(options).read(fileName,table);
  if (reportInfo(__FUNCTION__)) std::cout << "Read " << fileName << ": " << report.summary() << std::endl;

  analyze(table);

  // outputs
  String outputPath = FileSystem::configuredPath(stringValue("OUTPUT:PATH"),"DATUM_HISTOS_EXPORT_PATH");
  auto outputName = [&](const String & file) { return outputPath.isNull() ? file : FileSystem::join(outputPath,file); };
  String outputFile = stringValue("OUTPUT:FILE");
  if (!isNone(outputFile))
    {
    DatumFile out(outputName(outputFile),File::kWriteOnly);
    out = _results;
    out.setFolderName(_table.name());
    out.write();
    if (reportInfo(__FUNCTION__)) std::cout << "Results written to " << outputName(outputFile) << std::endl;
    }
  String tableFile = stringValue("OUTPUT:TABLE_FILE");
  if (!isNone(tableFile))
    {
    TableWriter writer;
    writer.options().format = TableFormat::fromName(stringValue("OUTPUT:TABLE_FORMAT"),&ok);
    if (!ok) throw Exception(String("Unknown format: ")+stringValue("OUTPUT:TABLE_FORMAT"),__FUNCTION__);
    writer.write(outputName(tableFile),_table);
    if (reportInfo(__FUNCTION__)) std::cout << "Table written to " << outputName(tableFile) << std::endl;
    }
}

void SpreadsheetAnalyzer::process(MATH::Table2D & table)
{
  for (const std::string & function : items(stringValue("PROCESS:FUNCTIONS")))
    runProcessor(String(function.c_str()),table,*this);
}

void SpreadsheetAnalyzer::analyze(const MATH::Table2D & input)
{
  // columns kept
  MATH::Table2D table(input);
  std::vector<unsigned int> selected = columnIndices(table,stringValue("SELECT:COLUMNS"));
  if (!selected.empty())
    {
    MATH::Table2D kept(table.name());
    for (unsigned int c : selected) kept.addColumn(table.column(c),table.columnName(c));
    kept.setTitle(table.title());
    table = kept;
    }
  process(table);
  _table = table;
  _results.clear();
  _results.add(new MATH::Table2D(_table),"table");

  if (boolValue("SUMMARY",true))
    _results.add(new MATH::Table2D(TableStatistics::summaryTable(_table)),"summary");

  if (boolValue("CORRELATIONS:USE",true) && _table.nColumns()>0)
    {
    TableStatistics::Method method = TableStatistics::methodFromName(stringValue("CORRELATIONS:METHOD"));
    std::vector<unsigned int> columns = columnIndices(_table,stringValue("CORRELATIONS:COLUMNS"));
    MATH::Table2D pValues;
    MATH::Table2D r = TableStatistics::correlationMatrix(_table,columns,method,&pValues);
    String methodName = TableStatistics::methodName(method);
    Folder * folder = _results.addFolder("correlations");
    folder->add(new MATH::Table2D(r),methodName);
    folder->add(new MATH::Table2D(pValues),methodName+String("_pValues"));
    folder->add(TablePlots::matrixMap(r,methodName+String("_map")),methodName+String("_map"));
    folder->add(new MATH::Table2D(TableStatistics::covarianceMatrix(_table,columns)),"covariance");
    }

  Folder * plots = nullptr;
  auto plotFolder = [&]() { if (!plots) plots = _results.addFolder("plots"); return plots; };
  for (const std::string & item : items(stringValue("PLOTS:XY")))
    {
    std::vector<std::string> p = parts(item);
    if (p.size()<2) throw Exception(String("A plot of PLOTS:XY is y:x, not ")+String(item.c_str()),__FUNCTION__);
    unsigned int y = columnIndex(_table,String(p[0].c_str()));
    unsigned int x = columnIndex(_table,String(p[1].c_str()));
    int ye = p.size()>2 && !p[2].empty() ? int(columnIndex(_table,String(p[2].c_str()))) : -1;
    int xe = p.size()>3 && !p[3].empty() ? int(columnIndex(_table,String(p[3].c_str()))) : -1;
    MATH::DataPointsVector * points = TablePlots::xy(_table,x,y,ye,xe);
    plotFolder()->add(points,points->name());
    }
  for (const std::string & item : items(stringValue("PLOTS:BARS")))
    {
    std::vector<std::string> p = parts(item);
    unsigned int v = columnIndex(_table,String(p[0].c_str()));
    int x = p.size()>1 && !p[1].empty() ? int(columnIndex(_table,String(p[1].c_str()))) : -1;
    MATH::DataPointsVector * points = TablePlots::bars(_table,v,x);
    plotFolder()->add(points,points->name());
    }
  int bins = intValue("PLOTS:BINS",0);
  for (const std::string & item : items(stringValue("PLOTS:DISTRIBUTIONS")))
    {
    MATH::Histogram * h = TablePlots::distribution(_table,columnIndex(_table,String(item.c_str())),bins>0 ? unsigned(bins) : 0u);
    plotFolder()->add(h,h->name());
    }
}

} // namespace Datum
