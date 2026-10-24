/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "PropertiesParser.hpp"
#include "PrintHelpers.hpp"
#include "StreamHelpers.hpp"

ClassImp(CAP::PropertiesParser);

namespace CAP
{

PropertiesParser::PropertiesParser()
:
Parser()
{
  appendClassName("PropertiesParser");
  setName("PropertiesParser");
}


void  PropertiesParser::loadFrom(Properties & _properties,
                                     const String & _inputPath,
                                     const String & _inputFileName,
                                     const String & _extension)
{
  std::ifstream & inputFile = openInputAsciiFile(_inputPath,_inputFileName,_extension);
  if (reportDebug(__FUNCTION__))
    {
    printCR();
    printValue("Parsing file",_inputPath + _inputFileName);
    }
  // Read whole lines with std::getline: the previous fixed 200-char buffer set the failbit on
  // any longer line, after which eof() never became true and the loop never terminated.
  auto trim = [](const std::string & str)
    {
    const char * ws = " \t\r\n";
    size_t first = str.find_first_not_of(ws);
    if (first==std::string::npos) return std::string();
    size_t last = str.find_last_not_of(ws);
    return str.substr(first,last-first+1);
    };
  std::string line;
  int lineCount = 0;
  while (std::getline(inputFile,line))
    {
    lineCount++;
    std::string text = trim(line);
    // blank lines, comments (# or ;) and [section] headers are ignored.
    if (text.empty() || text[0]=='#' || text[0]==';' || text[0]=='[') continue;
    size_t equal = text.find('=');
    if (equal==std::string::npos)
      {
      if (reportWarning(__FUNCTION__))
        {
        printCR();
        printValue("lineCount",lineCount);
        printValue("Ignoring line without '='",text.c_str());
        }
      continue;
      }
    // Keys never contain white space; values keep their internal spaces and are only trimmed.
    std::string key;
    for (char c : text.substr(0,equal)) if (c!=' ' && c!='\t') key += c;
    std::string val = trim(text.substr(equal+1));
    String keyword = key.c_str();
    String value   = val.c_str();
    _properties.addProperty(keyword,value);
    if (reportDebug(__FUNCTION__))
      {
      printCR();
      printValue("lineCount",lineCount);
      printValue("Property name",keyword);
      printValue("Property value",value);
      }
    }
  inputFile.close();
}

//!
//! Write the properties as "name = value" lines, i.e., in the format read back by loadFrom().
//!
void  PropertiesParser::saveTo(Properties & _properties,
                                 const String & _path,
                                 const String & _fileName,
                                 const String & _extension)
{
  std::ofstream & outputFile = openOutputAsciiFile(_path,_fileName,_extension);
  for (auto & property : _properties.properties())
    outputFile << property.name() << " = " << property.value() << std::endl;
  outputFile.close();
}

}
