/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TableFormat.cpp
//! @brief Implementation of TableFormat.
#include <algorithm>
#include <cctype>
#include <string>
#include "TableFormat.hpp"

namespace Datum
{

namespace
{
std::string lower(const std::string & s)
{
  std::string r(s);
  for (char & c : r) c = char(std::tolower((unsigned char)c));
  return r;
}

std::string extensionOf(const String & fileName)
{
  std::string name(fileName.data());
  std::size_t slash = name.find_last_of("/\\");
  std::size_t dot = name.find_last_of('.');
  if (dot==std::string::npos || (slash!=std::string::npos && dot<slash)) return "";
  return lower(name.substr(dot));
}
} // namespace

TableFormat::Type TableFormat::fromFileName(const String & fileName)
{
  std::string e = extensionOf(fileName);
  if (e==".csv")                              return kCsv;
  if (e==".tsv" || e==".tab")                 return kTab;
  if (e==".ssv")                              return kSemicolon;
  if (e==".dtb" || e==".datum" || e==".cap")  return kBinary;
  if (e==".xlsx")                             return kXlsx;
  return kAuto;
}

TableFormat::Type TableFormat::fromName(const String & name, bool * ok)
{
  static const char * names[] = { "auto", "blank", "tab", "comma", "csv", "semicolon", "binary", "xlsx" };
  std::string n = lower(name.data());
  if (ok) *ok = true;
  for (int k=0; k<8; k++) if (n==names[k]) return Type(k);
  if (n=="text" || n=="txt" || n=="space") return kBlank;
  if (n=="tsv")   return kTab;
  if (n=="excel") return kXlsx;
  if (n=="dtb" || n=="datum") return kBinary;
  if (ok) *ok = false;
  return kAuto;
}

String TableFormat::name(Type type)
{
  static const char * names[] = { "auto", "blank", "tab", "comma", "csv", "semicolon", "binary", "xlsx" };
  int k = int(type);
  return (k>=0 && k<8) ? String(names[k]) : String("auto");
}

char TableFormat::separator(Type type)
{
  switch (type)
    {
    case kBlank:     return ' ';
    case kTab:       return '\t';
    case kComma:
    case kCsv:       return ',';
    case kSemicolon: return ';';
    default:         return 0;
    }
}

std::vector<String> TableFormat::readableExtensions()
{
  return { ".csv", ".tsv", ".tab", ".ssv", ".txt", ".dat", ".dtb", ".datum", ".cap" };
}

std::vector<String> TableFormat::writableExtensions()
{
  return { ".csv", ".tsv", ".ssv", ".txt", ".dat", ".dtb", ".datum", ".xlsx" };
}

String TableFormat::extension(Type type)
{
  switch (type)
    {
    case kTab:       return ".tsv";
    case kCsv:       return ".csv";
    case kSemicolon: return ".ssv";
    case kBinary:    return ".dtb";
    case kXlsx:      return ".xlsx";
    default:         return ".txt";
    }
}

} // namespace Datum
