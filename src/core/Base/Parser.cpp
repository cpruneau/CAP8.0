/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "Parser.hpp"
#include "Exceptions.hpp"


namespace CAP
{

  Parser::Parser()
  :
  Object()
  {
  setClassName("Parser");
  setName("Parser");
  }

  Parser::Parser(const Parser & source)
  :
  Object(source)
  { }

  Parser & Parser::operator=(const Parser & rhs)
  {
  if (this!=&rhs)
    {
    Object::operator=(rhs);
    }
  return *this;
  }

} // namespace CAP
