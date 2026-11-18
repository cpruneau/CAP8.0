/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TableReader.cpp
//! @brief Implementation of TableReader.
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <memory>
#include <sstream>
#include "TableReader.hpp"
#include "DatumFile.hpp"
#include "Exceptions.hpp"

namespace Datum
{

namespace
{
const double kNaN = std::numeric_limits<double>::quiet_NaN();

bool isBlank(char c) { return c==' ' || c=='\t' || c=='\r'; }

std::string trim(const std::string & s)
{
  std::size_t a = 0, b = s.size();
  while (a<b && isBlank(s[a])) a++;
  while (b>a && isBlank(s[b-1])) b--;
  return s.substr(a,b-a);
}

std::string lower(const std::string & s)
{
  std::string r(s);
  for (char & c : r) c = char(std::tolower((unsigned char)c));
  return r;
}

String toString(unsigned long n) { return String(std::to_string(n).c_str()); }

//! Name of a file without its directory and extension.
String stem(const String & fileName)
{
  std::string name(fileName.data());
  std::size_t slash = name.find_last_of("/\\");
  if (slash!=std::string::npos) name = name.substr(slash+1);
  std::size_t dot = name.find_last_of('.');
  if (dot!=std::string::npos && dot>0) name = name.substr(0,dot);
  return String(name.empty() ? "table" : name.c_str());
}

//! One record (line, or several lines in a quoted CSV field) of a text, and the line where it starts.
struct Record { std::string text; unsigned int line; };

//! Split a text into lines (\n, \r\n or \r); with quotes, a line break inside "..." does not end the record.
std::vector<Record> records(const std::string & text, bool quotes)
{
  std::vector<Record> result;
  std::string current;
  unsigned int line = 1, start = 1;
  bool inQuotes = false;
  for (std::size_t k=0; k<text.size(); k++)
    {
    char c = text[k];
    if (quotes && c=='"') inQuotes = !inQuotes;   // a doubled quote "" toggles twice
    if (c=='\r' || c=='\n')
      {
      if (c=='\r' && k+1<text.size() && text[k+1]=='\n') { if (inQuotes) current += '\r'; k++; }
      line++;
      if (inQuotes) { current += '\n'; continue; }
      result.push_back({current,start});
      current.clear();
      start = line;
      continue;
      }
    current += c;
    }
  if (!current.empty()) result.push_back({current,start});
  return result;
}

//! Whether a line is skipped: blank, or a comment.
bool skipped(const std::string & line, char comment)
{
  std::size_t first = line.find_first_not_of(" \t\r");
  return first==std::string::npos || (comment!=0 && line[first]==comment);
}
} // namespace

TableReader::TableReader() : _options() {}

//////////////////////////////////////////////////////////////////////////
// Report
//////////////////////////////////////////////////////////////////////////

String TableReader::Report::summary() const
{
  String s = toString(rows) + String(" rows x ") + toString(columns) + String(" columns");
  s += String(" (") + TableFormat::name(format) + String(")");
  if (header) s += String(", header");
  if (missing>0) s += String(", ") + toString(missing) + String(" missing values");
  if (nonNumeric>0) s += String(", ") + toString(nonNumeric) + String(" non-numeric cells (first: ") + firstNonNumeric + String(")");
  if (padded>0) s += String(", ") + toString(padded) + String(" short rows completed");
  return s;
}

//////////////////////////////////////////////////////////////////////////
// Fields
//////////////////////////////////////////////////////////////////////////

TableFormat::Type TableReader::detectFormat(const std::string & line)
{
  // count outside quotes
  int tabs = 0, commas = 0, semicolons = 0;
  bool inQuotes = false;
  for (char c : line)
    {
    if (c=='"') inQuotes = !inQuotes;
    if (inQuotes) continue;
    if (c=='\t') tabs++;
    else if (c==',') commas++;
    else if (c==';') semicolons++;
    }
  if (tabs>0)       return TableFormat::kTab;
  if (commas>0)     return TableFormat::kCsv;
  if (semicolons>0) return TableFormat::kSemicolon;
  return TableFormat::kBlank;
}

std::vector<std::string> TableReader::splitLine(const std::string & line, TableFormat::Type format)
{
  if (!TableFormat::isText(format)) format = TableFormat::kBlank;
  const char separator = TableFormat::separator(format);
  const bool quotes = format!=TableFormat::kComma;
  const bool blank = format==TableFormat::kBlank;
  std::vector<std::string> fields;
  std::size_t k = 0, n = line.size();
  auto spaces = [&](std::size_t & i) { while (i<n && (line[i]==' ' || line[i]=='\r' || (line[i]=='\t' && separator!='\t'))) i++; };
  if (blank) { spaces(k); if (k>=n) return fields; }
  while (true)
    {
    std::string field;
    spaces(k);
    if (quotes && k<n && line[k]=='"')
      {
      k++;
      while (k<n)
        {
        if (line[k]=='"')
          {
          if (k+1<n && line[k+1]=='"') { field += '"'; k += 2; continue; }
          k++;
          break;
          }
        field += line[k++];
        }
      // anything after the closing quote, up to the separator, is kept (malformed CSV)
      std::string rest;
      while (k<n && line[k]!=separator && !(blank && (line[k]=='\t'))) rest += line[k++];
      field += trim(rest);
      }
    else
      {
      while (k<n && line[k]!=separator && !(blank && line[k]=='\t')) field += line[k++];
      field = trim(field);
      }
    fields.push_back(field);
    if (k>=n) break;
    k++;   // the separator
    if (blank) { spaces(k); if (k>=n) break; }
    }
  return fields;
}

int TableReader::parseField(const std::string & text, double & value)
{
  std::string field = trim(text);
  value = kNaN;
  if (field.empty()) return 1;
  std::string l = lower(field);
  if (l=="na" || l=="n/a" || l=="nan" || l=="null" || l=="-" || l=="#n/a" || l=="none") return 1;
  const char * begin = field.c_str();
  char * end = nullptr;
  errno = 0;
  double v = std::strtod(begin,&end);
  if (end==begin) return 2;
  while (*end==' ' || *end=='\t') end++;
  if (*end!='\0') return 2;
  value = v;   // overflow gives +-inf, underflow a value near 0: kept as strtod gives them
  return 0;
}

//////////////////////////////////////////////////////////////////////////
// Reading
//////////////////////////////////////////////////////////////////////////

TableReader::Report TableReader::read(const String & fileName, MATH::Table2D & table) const
{
  TableFormat::Type format = _options.format;
  if (format==TableFormat::kAuto) format = TableFormat::fromFileName(fileName);
  if (format==TableFormat::kXlsx)
    throw FileException(fileName,"Excel workbooks cannot be read (save the sheet as CSV)","TableReader::read()");
  if (format==TableFormat::kBinary) return readBinary(fileName,table);
  std::ifstream in(fileName.data(),std::ios::binary);
  if (!in) throw FileException(fileName,"Cannot open the file for reading","TableReader::read()");
  std::ostringstream contents;
  contents << in.rdbuf();
  if (in.bad()) throw FileException(fileName,"Error while reading the file","TableReader::read()");
  // a file that does not look like text may be a binary table
  std::string text = contents.str();
  if (text.size()>=4 && (text.compare(0,4,"DTMB")==0 || text.compare(0,4,"DTMO")==0 || text.compare(0,3,"CAP")==0)
      && _options.format==TableFormat::kAuto)
    return readBinary(fileName,table);
  Report report = readText(text,format,table,fileName);
  if (_options.name.isNull()) { table.setName(stem(fileName)); table.setTitle(stem(fileName)); }
  return report;
}

TableReader::Report TableReader::readText(const std::string & input, TableFormat::Type format, MATH::Table2D & table,
                                          const String & source) const
{
  if (format==TableFormat::kBinary || format==TableFormat::kXlsx)
    throw FileException(source,"Not a text format","TableReader::readText()");
  std::string text = input;
  if (text.size()>=3 && text.compare(0,3,"\xEF\xBB\xBF")==0) text.erase(0,3);   // UTF-8 byte order mark (Excel)

  // separator: from the format, or from the first line of data
  if (format==TableFormat::kAuto)
    {
    // the separator of the first line of data, if the second line has it too (a comma in a name of the header
    // row of a blank separated file is not a separator)
    std::vector<Record> lines = records(text,false);
    format = TableFormat::kBlank;
    std::vector<std::string> data;
    for (std::size_t k=_options.skipLines; k<lines.size() && data.size()<2; k++)
      if (!skipped(lines[k].text,_options.comment)) data.push_back(lines[k].text);
    if (!data.empty())
      {
      format = detectFormat(data[0]);
      if (data.size()>1 && format!=TableFormat::kBlank && detectFormat(data[1])!=format)
        {
        char separator = TableFormat::separator(format);
        if (data[1].find(separator)==std::string::npos) format = TableFormat::kBlank;
        }
      }
    }
  Report report;
  report.format = format;
  const bool commaDecimal = format==TableFormat::kSemicolon;   // "1,5" is 1.5 in a semicolon separated file

  std::vector<Record> lines = records(text,TableFormat::quotes(format));
  std::vector<String> names;
  std::vector< std::vector<double> > rows;
  std::size_t nColumns = 0;   // columns seen (last non-empty field of any row, and the names)
  bool first = true;
  for (std::size_t k=_options.skipLines; k<lines.size(); k++)
    {
    const Record & record = lines[k];
    if (skipped(record.text,_options.comment)) continue;
    std::vector<std::string> fields = splitLine(record.text,format);
    // fields after the last non-empty one do not count (trailing separators of spreadsheet exports)
    std::size_t used = fields.size();
    while (used>0 && fields[used-1].empty()) used--;
    if (first)
      {
      first = false;
      bool header = _options.header==kHeader;
      if (_options.header==kAutoHeader)
        for (std::size_t c=0; c<used && !header; c++)
          {
          double v;
          if (parseField(fields[c],v)==2) header = true;
          }
      if (header)
        {
        for (std::size_t c=0; c<used; c++) names.push_back(String(fields[c].c_str()));
        nColumns = used;
        report.header = true;
        continue;
        }
      }
    std::vector<double> values(used,kNaN);
    for (std::size_t c=0; c<used; c++)
      {
      std::string field = fields[c];
      if (commaDecimal && field.find(',')!=std::string::npos && field.find('.')==std::string::npos)
        std::replace(field.begin(),field.end(),',','.');
      int status = parseField(field,values[c]);
      if (status==1) report.missing++;
      else if (status==2)
        {
        String where = String("line ") + toString(record.line) + String(", column ") + toString(c+1)
                     + String(": '") + String(fields[c].c_str()) + String("'");
        if (_options.strict) throw FileException(source,String("Not a number at ")+where,"TableReader::readText()");
        if (report.nonNumeric==0) report.firstNonNumeric = where;
        report.nonNumeric++;
        }
      }
    nColumns = std::max(nColumns,used);
    rows.push_back(std::move(values));
    }
  if (nColumns==0) throw FileException(source,"No data found","TableReader::readText()");
  if (rows.size()>std::numeric_limits<unsigned int>::max())
    throw FileException(source,"Too many rows","TableReader::readText()");

  // the table: short rows completed with NaN (blank rows that hold only separators are not rows)
  MATH::Table2D result(_options.name.isNull() ? table.name() : _options.name, 0, 0);
  if (!rows.empty())
    {
    result.resize(unsigned(rows.size()),unsigned(nColumns),kNaN);
    for (std::size_t i=0; i<rows.size(); i++)
      {
      if (rows[i].size()<nColumns)
        {
        report.padded++;
        report.missing += unsigned(nColumns-rows[i].size());
        }
      for (std::size_t j=0; j<rows[i].size(); j++) result(unsigned(i),unsigned(j)) = rows[i][j];
      }
    }
  else
    result.resize(0,0);
  if (report.header && !rows.empty())
    {
    names.resize(nColumns);
    result.setColumnNames(names);
    }
  report.rows    = result.nRows();
  report.columns = rows.empty() ? unsigned(nColumns) : result.nColumns();
  if (rows.empty()) throw FileException(source,"No rows of data (only a header)","TableReader::readText()");

  String name = _options.name.isNull() ? table.name() : _options.name;
  table.clear();
  table = result;
  table.setName(name);
  table.setTitle(name);
  return report;
}

TableReader::Report TableReader::readBinary(const String & fileName, MATH::Table2D & table) const
{
  std::ifstream probe(fileName.data(),std::ios::binary);
  if (!probe) throw FileException(fileName,"Cannot open the file for reading","TableReader::readBinary()");
  char tag[5] = { 0, 0, 0, 0, 0 };
  probe.read(tag,4);
  probe.close();
  MATH::Table2D result(table.name());
  if (std::string(tag)=="DTMB" || std::string(tag)=="CAPB")
    result.loadBinary(fileName,_options.name);   // records written by Table2D::saveBinary()
  else
    {
    // a Datum file: the table of the given path, or the first one
    DatumFile file(fileName,File::kReadOnly);
    file.read();
    const MATH::Table2D * found = nullptr;
    if (!_options.name.isNull()) found = file.get<MATH::Table2D>(_options.name);
    else
      for (const String & path : file.paths())
        {
        try { found = file.get<MATH::Table2D>(path); }
        catch (...) { found = nullptr; }
        if (found) break;
        }
    if (!found)
      throw FileException(fileName,_options.name.isNull() ? String("The file holds no table")
                                                          : String("No table named ")+_options.name,"TableReader::readBinary()");
    result = *found;
    }
  Report report;
  report.format  = TableFormat::kBinary;
  report.rows    = result.nRows();
  report.columns = result.nColumns();
  report.header  = result.hasColumnNames();
  for (double v : result.values()) if (std::isnan(v)) report.missing++;
  table = result;
  return report;
}

} // namespace Datum
