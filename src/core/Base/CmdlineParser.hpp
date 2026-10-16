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
#ifndef CAP__CommandLineParser
#define CAP__CommandLineParser
#include "Parser.hpp"

namespace CAP
{

//!
//! @brief Parser of command line arguments (placeholder).
//!
//! CommandLineParser only holds a name and a title for now: it does not parse arguments yet, and the version and
//! pre-hash given to the constructor are not stored. The class is not part of the Base library build (CmdlineParser.cpp
//! is not listed in CMakeLists.txt).
//!
class CommandLineParser : public Parser
{
public:

  //! @brief Default constructor.
  CommandLineParser();
  //! @brief Constructor. @param _name name of the parser @param _title title of the parser
  //! @param _version version of the program (not stored yet) @param _preHash prefix of the options (not stored yet)
  CommandLineParser(const String & _name,
                const String & _title,
                const String & _version,
                const String & _preHash);
  //! @brief Copy constructor. @param parser parser to copy
  CommandLineParser(const CommandLineParser & parser);
  //! @brief Destructor.
  virtual ~CommandLineParser(){}
  //! @brief Copy assignment. @param parser parser to copy @return this parser
  CommandLineParser & operator=(const CommandLineParser & parser);

  ClassDef(CommandLineParser,0)
};

}



#endif /* CAP__CommandLineParser */
