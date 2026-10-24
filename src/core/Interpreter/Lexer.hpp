/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Lexer.hpp
//! @brief Tokens of the CAP command language, and the exception of the interpreter.
#ifndef CAP__INTERPRETER__Lexer
#define CAP__INTERPRETER__Lexer
#include <string>
#include <vector>
#include "Exceptions.hpp"

namespace CAP
{
  namespace INTERPRETER
  {

  //! @brief Error found while reading or running a program: a message and the line where it happened.
  class InterpreterException : public Exception
  {
  public:
  //! @brief Error. @param message what happened @param line line of the program (0: unknown)
  InterpreterException(const String & message, int line) : Exception(message, "Interpreter"), _line(line) { }
  int line() const { return _line; }   //!< Line of the program
  private:
  int _line;   //!< line of the program
  };

  //! @brief A token: a word, a number, a text, a symbol, or the end of a line.
  struct Token
  {
  //! Kinds of tokens
  enum Kind { kEnd, kNewline, kInteger, kReal, kText, kName, kKeyword, kSymbol };
  Kind kind = kEnd;   //!< kind
  std::string text;   //!< characters of a name, keyword or symbol; value of a text
  long integer = 0;   //!< value of an integer
  double real = 0.0;   //!< value of a real
  int line = 0;   //!< line of the program
  //! @brief Whether the token is a symbol or keyword. @param s characters @return true if so
  bool is(const char * s) const { return (kind == kSymbol || kind == kKeyword) && text == s; }
  };

  //!
  //! @brief Tokens of a program. Newlines inside parentheses and brackets are dropped (an expression may continue on
  //! the next line); comments run from '#' or "//" to the end of the line.
  //! @param source program @param firstLine number of its first line
  //! @return tokens, ending with kEnd
  //! @throws InterpreterException for an unknown character or a text not closed
  //!
  std::vector<Token> tokenize(const std::string & source, int firstLine=1);

  //!
  //! @brief Whether a program is complete: no parenthesis, bracket or brace left open, no text left open (to read
  //! more lines before running what was typed).
  //! @param source program @return true if complete
  //!
  bool isComplete(const std::string & source);

  } // namespace INTERPRETER
} // namespace CAP

#endif // !CAP__INTERPRETER__Lexer
