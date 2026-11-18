/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TableWriter.cpp
//! @brief Implementation of TableWriter (text, binary, and Excel workbooks written as stored zip archives).
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <vector>
#include "TableWriter.hpp"
#include "DatumFile.hpp"
#include "Exceptions.hpp"

namespace Datum
{

namespace
{

//////////////////////////////////////////////////////////////////////////
// Zip archive, entries stored without compression
//////////////////////////////////////////////////////////////////////////

uint32_t crc32(const std::string & data)
{
  static uint32_t table[256];
  static bool ready = false;
  if (!ready)
    {
    for (uint32_t n=0; n<256; n++)
      {
      uint32_t c = n;
      for (int k=0; k<8; k++) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
      table[n] = c;
      }
    ready = true;
    }
  uint32_t c = 0xFFFFFFFFu;
  for (unsigned char b : data) c = table[(c ^ b) & 0xFF] ^ (c >> 8);
  return c ^ 0xFFFFFFFFu;
}

void put16(std::string & out, uint32_t v) { out += char(v & 0xFF); out += char((v >> 8) & 0xFF); }
void put32(std::string & out, uint32_t v) { put16(out, v & 0xFFFF); put16(out, (v >> 16) & 0xFFFF); }

class ZipWriter
{
public:
  ZipWriter()
  {
    std::time_t now = std::time(nullptr);
    std::tm * t = std::localtime(&now);
    if (t && t->tm_year>=80)
      {
      _time = uint32_t((t->tm_hour << 11) | (t->tm_min << 5) | (t->tm_sec / 2));
      _date = uint32_t(((t->tm_year - 80) << 9) | ((t->tm_mon + 1) << 5) | t->tm_mday);
      }
  }

  void add(const std::string & name, const std::string & data)
  {
    if (_archive.size() + data.size() + name.size() + 30 > 0xFFFFFFFFu)
      throw Exception("The workbook would exceed 4 GB (zip archives without the zip64 extension)","TableWriter::xlsx()");
    Entry e{ name, crc32(data), uint32_t(data.size()), uint32_t(_archive.size()) };
    put32(_archive, 0x04034b50);           // local file header
    put16(_archive, 20);                   // version needed: 2.0
    put16(_archive, 0);                    // flags
    put16(_archive, 0);                    // method: stored
    put16(_archive, _time);
    put16(_archive, _date);
    put32(_archive, e.crc);
    put32(_archive, e.size);               // compressed size
    put32(_archive, e.size);               // uncompressed size
    put16(_archive, uint32_t(name.size()));
    put16(_archive, 0);                    // extra field length
    _archive += name;
    _archive += data;
    _entries.push_back(e);
  }

  std::string finish()
  {
    std::string out = _archive;
    uint32_t start = uint32_t(out.size());
    for (const Entry & e : _entries)
      {
      put32(out, 0x02014b50);              // central directory header
      put16(out, 20);                      // version made by
      put16(out, 20);                      // version needed
      put16(out, 0);
      put16(out, 0);
      put16(out, _time);
      put16(out, _date);
      put32(out, e.crc);
      put32(out, e.size);
      put32(out, e.size);
      put16(out, uint32_t(e.name.size()));
      put16(out, 0);                       // extra
      put16(out, 0);                       // comment
      put16(out, 0);                       // disk
      put16(out, 0);                       // internal attributes
      put32(out, 0);                       // external attributes
      put32(out, e.offset);
      out += e.name;
      }
    uint32_t size = uint32_t(out.size()) - start;
    put32(out, 0x06054b50);                // end of central directory
    put16(out, 0);
    put16(out, 0);
    put16(out, uint32_t(_entries.size()));
    put16(out, uint32_t(_entries.size()));
    put32(out, size);
    put32(out, start);
    put16(out, 0);
    return out;
  }

private:
  struct Entry { std::string name; uint32_t crc; uint32_t size; uint32_t offset; };
  std::string _archive;
  std::vector<Entry> _entries;
  uint32_t _time = 0;
  uint32_t _date = (1 << 5) | 1;   // 1980-01-01
};

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////

std::string xmlEscape(const std::string & s)
{
  std::string r;
  r.reserve(s.size());
  for (unsigned char c : s)
    {
    switch (c)
      {
      case '&': r += "&amp;";  break;
      case '<': r += "&lt;";   break;
      case '>': r += "&gt;";   break;
      case '"': r += "&quot;"; break;
      default:
        if (c<0x20 && c!='\t' && c!='\n' && c!='\r') break;   // not allowed in XML 1.0
        r += char(c);
      }
    }
  return r;
}

//! Excel column letters of a column index: 0 -> A, 25 -> Z, 26 -> AA, ...
std::string columnLetters(unsigned int column)
{
  std::string letters;
  unsigned int n = column + 1;
  while (n>0) { unsigned int r = (n - 1) % 26; letters.insert(letters.begin(), char('A' + r)); n = (n - 1) / 26; }
  return letters;
}

std::string sheetNameOf(const String & requested)
{
  std::string name;
  for (char c : std::string(requested.data()))
    if (std::string("[]:*?/\\").find(c)==std::string::npos && (unsigned char)c>=0x20) name += c;
  if (name.size()>31) name.resize(31);
  while (!name.empty() && name[0]=='\'') name.erase(0,1);
  return name.empty() ? std::string("Sheet1") : name;
}

std::string quoteField(const std::string & field, char separator)
{
  bool needed = field.find(separator)!=std::string::npos || field.find('"')!=std::string::npos
             || field.find('\n')!=std::string::npos || field.find('\r')!=std::string::npos
             || (!field.empty() && (field.front()==' ' || field.back()==' '));
  if (!needed) return field;
  std::string r = "\"";
  for (char c : field) { if (c=='"') r += '"'; r += c; }
  return r + "\"";
}

void writeFile(const String & fileName, const std::string & contents)
{
  std::ofstream out(fileName.data(),std::ios::binary | std::ios::trunc);
  if (!out) throw FileException(fileName,"Cannot open the file for writing","TableWriter::write()");
  out.write(contents.data(),std::streamsize(contents.size()));
  out.close();
  if (!out) throw FileException(fileName,"Error while writing the file","TableWriter::write()");
}

std::string extensionOf(const String & fileName)
{
  std::string name(fileName.data());
  std::size_t slash = name.find_last_of("/\\");
  std::size_t dot = name.find_last_of('.');
  if (dot==std::string::npos || (slash!=std::string::npos && dot<slash)) return "";
  std::string e = name.substr(dot);
  for (char & c : e) c = char(std::tolower((unsigned char)c));
  return e;
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// TableWriter
//////////////////////////////////////////////////////////////////////////

TableWriter::TableWriter() : _options() {}

std::string TableWriter::formatNumber(double value, int precision)
{
  if (std::isnan(value)) return "nan";
  if (std::isinf(value)) return value>0 ? "inf" : "-inf";
  if (precision<1) precision = 1;
  if (precision>17) precision = 17;
  char buffer[64];
  std::snprintf(buffer,sizeof(buffer),"%.*g",precision,value);
  return buffer;
}

std::string TableWriter::text(const MATH::Table2D & table, TableFormat::Type format) const
{
  if (format==TableFormat::kAuto) format = TableFormat::kTab;
  if (!TableFormat::isText(format)) throw Exception("Not a text format","TableWriter::text()");
  const bool blank = format==TableFormat::kBlank;
  const bool quotes = TableFormat::quotes(format);
  const std::string separator = blank ? std::string("  ") : std::string(1,TableFormat::separator(format));
  const char sep = TableFormat::separator(format);
  std::string out;
  auto field = [&](const std::string & f) { return quotes ? quoteField(f,sep) : f; };
  if (_options.header)
    {
    std::string line = (blank && _options.commentHeader) ? "# " : "";
    bool first = true;
    auto addName = [&](std::string name)
      {
      if (blank) { for (char & c : name) if (c==' ' || c=='\t') c = '_'; if (name.empty()) name = "_"; }
      else if (!quotes) { for (char & c : name) if (c==sep) c = '_'; }
      if (!first) line += separator;
      line += field(name);
      first = false;
      };
    if (_options.index) addName("index");
    for (unsigned int j=0; j<table.nColumns(); j++) addName(table.columnName(j).data());
    out += line + "\n";
    }
  for (unsigned int i=0; i<table.nRows(); i++)
    {
    std::string line;
    if (_options.index) line += std::to_string(i);
    for (unsigned int j=0; j<table.nColumns(); j++)
      {
      if (j>0 || _options.index) line += separator;
      double v = table(i,j);
      if (std::isnan(v) && !blank) continue;   // empty field
      line += formatNumber(v,_options.precision);
      }
    out += line + "\n";
    }
  return out;
}

std::string TableWriter::xlsx(const MATH::Table2D & table) const
{
  const unsigned int extraRow = _options.header ? 1 : 0;
  const unsigned int extraColumn = _options.index ? 1 : 0;
  if (std::size_t(table.nRows()) + extraRow > 1048576u)
    throw Exception("Too many rows for an Excel sheet (at most 1048576)","TableWriter::xlsx()");
  if (std::size_t(table.nColumns()) + extraColumn > 16384u)
    throw Exception("Too many columns for an Excel sheet (at most 16384)","TableWriter::xlsx()");
  const std::string ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
  const std::string rel = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
  const std::string pkg = "http://schemas.openxmlformats.org/package/2006/relationships";
  const std::string head = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
  String requested = _options.sheetName.isNull() ? table.name() : _options.sheetName;
  std::string sheetName = sheetNameOf(requested);

  // the sheet
  std::string sheet = head + "<worksheet xmlns=\"" + ns + "\" xmlns:r=\"" + rel + "\">";
  if (_options.header)
    sheet += "<sheetViews><sheetView workbookViewId=\"0\"><pane ySplit=\"1\" topLeftCell=\"A2\" activePane=\"bottomLeft\""
             " state=\"frozen\"/></sheetView></sheetViews>";
  sheet += "<sheetData>";
  auto textCell = [&](const std::string & reference, const std::string & text, bool bold)
    {
    std::string t = xmlEscape(text);
    bool preserve = !text.empty() && (text.front()==' ' || text.back()==' ');
    return "<c r=\"" + reference + "\" t=\"inlineStr\"" + (bold ? " s=\"1\"" : "") + "><is><t"
           + (preserve ? " xml:space=\"preserve\"" : "") + ">" + t + "</t></is></c>";
    };
  if (_options.header)
    {
    sheet += "<row r=\"1\">";
    if (_options.index) sheet += textCell("A1","index",true);
    for (unsigned int j=0; j<table.nColumns(); j++)
      sheet += textCell(columnLetters(j+extraColumn) + "1", table.columnName(j).data(), true);
    sheet += "</row>";
    }
  std::vector<std::string> letters(table.nColumns()+extraColumn);
  for (std::size_t j=0; j<letters.size(); j++) letters[j] = columnLetters(unsigned(j));
  for (unsigned int i=0; i<table.nRows(); i++)
    {
    std::string r = std::to_string(i + 1 + extraRow);
    sheet += "<row r=\"" + r + "\">";
    if (_options.index) sheet += "<c r=\"A" + r + "\"><v>" + std::to_string(i) + "</v></c>";
    for (unsigned int j=0; j<table.nColumns(); j++)
      {
      double v = table(i,j);
      if (!std::isfinite(v)) continue;   // missing (NaN) and infinite values: empty cells
      sheet += "<c r=\"" + letters[j+extraColumn] + r + "\"><v>" + formatNumber(v,17) + "</v></c>";
      }
    sheet += "</row>";
    }
  sheet += "</sheetData></worksheet>";

  std::string contentTypes = head +
    "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
    "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
    "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
    "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
    "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
    "<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
    "</Types>";
  std::string rootRels = head + "<Relationships xmlns=\"" + pkg + "\">"
    "<Relationship Id=\"rId1\" Type=\"" + rel + "/officeDocument\" Target=\"xl/workbook.xml\"/></Relationships>";
  std::string workbook = head + "<workbook xmlns=\"" + ns + "\" xmlns:r=\"" + rel + "\"><sheets>"
    "<sheet name=\"" + xmlEscape(sheetName) + "\" sheetId=\"1\" r:id=\"rId1\"/></sheets></workbook>";
  std::string workbookRels = head + "<Relationships xmlns=\"" + pkg + "\">"
    "<Relationship Id=\"rId1\" Type=\"" + rel + "/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
    "<Relationship Id=\"rId2\" Type=\"" + rel + "/styles\" Target=\"styles.xml\"/></Relationships>";
  std::string styles = head + "<styleSheet xmlns=\"" + ns + "\">"
    "<fonts count=\"2\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font><font><b/><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
    "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
    "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
    "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
    "<cellXfs count=\"2\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"
    "<xf numFmtId=\"0\" fontId=\"1\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\"/></cellXfs>"
    "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles></styleSheet>";

  ZipWriter zip;
  zip.add("[Content_Types].xml", contentTypes);
  zip.add("_rels/.rels", rootRels);
  zip.add("xl/workbook.xml", workbook);
  zip.add("xl/_rels/workbook.xml.rels", workbookRels);
  zip.add("xl/styles.xml", styles);
  zip.add("xl/worksheets/sheet1.xml", sheet);
  return zip.finish();
}

void TableWriter::write(const String & fileName, const MATH::Table2D & table) const
{
  TableFormat::Type format = _options.format;
  if (format==TableFormat::kAuto)
    {
    format = TableFormat::fromFileName(fileName);
    if (format==TableFormat::kAuto) format = TableFormat::kBlank;   // .txt, .dat, other extensions
    }
  try
    {
    switch (format)
      {
      case TableFormat::kBinary:
        {
        std::string e = extensionOf(fileName);
        if (e==".datum" || e==".cap")
          {
          DatumFile file(fileName,File::kWriteOnly);
          file.setFolderName(table.name());
          file.add(new MATH::Table2D(table),table.name());
          file.write();
          }
        else
          table.saveBinary(fileName,false);
        break;
        }
      case TableFormat::kXlsx:
        writeFile(fileName,xlsx(table));
        break;
      default:
        writeFile(fileName,text(table,format));
      }
    }
  catch (const FileException &) { throw; }
  catch (const Exception & e)
    {
    throw FileException(fileName,String(e.what()),"TableWriter::write()");
    }
}

} // namespace Datum
