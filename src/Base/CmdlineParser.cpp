/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez     
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "CmdlineParser.hpp"

using CAP::CommandLineParser;
using std::ostream;
using std::cout;
using std::endl;

ClassImp(CommandLineParser);

CommandLineParser::CommandLineParser()
:
Parser()
{   }

CommandLineParser::CommandLineParser(const String & _name,
                             const String & _title,
                             const String & _version __attribute__((unused)),
                             const String & _preHash __attribute__((unused)))
:
Parser()
{
  // Parser has no (name,title,version,preHash) constructor; version/preHash are not stored yet.
  setName(_name);
  setTitle(_title);
}


CommandLineParser::CommandLineParser(const CommandLineParser & parser)
:
Parser(parser)
{   }

CommandLineParser & CommandLineParser::operator=(const CommandLineParser & parser)
{
  if (this!=&parser)
    {
    Parser::operator=(parser);
    }
  return *this;
}



