/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__Parser
#define CAP__Parser
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cmath>
#include <iomanip>
#include "Object.hpp"

namespace CAP
{

  //!
  //! @brief Base class of the parsers (PropertiesParser, CommandLineParser).
  //!
  //! Parser adds nothing to Object yet: it gives the parsers a common base, with a name ("Parser" by default) and
  //! the reporting facilities of Object.
  //!
  class Parser
  :
  public Object
  {
  public:

  //! @brief Default constructor: name and class name "Parser".
  Parser();
  //! @brief Copy constructor. @param parser parser to copy
  Parser(const Parser & parser);
  //! @brief Copy assignment. @param parser parser to copy @return this parser
  Parser & operator=(const Parser & parser);
  //! @brief Destructor.
  virtual ~Parser(){}

  protected:

  };

} // namespace CAP

#endif /* Parser_hpp */
