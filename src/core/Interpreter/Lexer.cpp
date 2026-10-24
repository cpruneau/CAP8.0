/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <cctype>
#include <cstdlib>
#include <set>
#include "Lexer.hpp"

namespace CAP
{
  namespace INTERPRETER
  {

  namespace
  {
  const std::set<std::string> & keywords()
  {
  static const std::set<std::string> words =
    { "if", "else", "while", "for", "in", "fun", "return", "break", "continue", "true", "false", "none",
      "and", "or", "not", "help" };
  return words;
  }

  //! Symbols, the longest first.
  const char * const symbols[] =
    { "..", "**", "==", "!=", "<=", ">=", "&&", "||", "+=", "-=", "*=", "/=",
      "+", "-", "*", "/", "%", "^", "<", ">", "=", "!", "(", ")", "{", "}", "[", "]", ",", ".", ";", ":" };
  } // namespace

  std::vector<Token> tokenize(const std::string & source, int firstLine)
  {
  std::vector<Token> tokens;
  int line = firstLine;
  int depth = 0;   // open parentheses and brackets
  std::size_t k = 0, n = source.size();
  auto push = [&](Token::Kind kind, const std::string & text)
    {
    Token t;
    t.kind = kind;
    t.text = text;
    t.line = line;
    tokens.push_back(t);
    return &tokens.back();
    };
  while (k < n)
    {
    char c = source[k];
    if (c == '\n')
      {
      if (depth == 0 && !tokens.empty() && tokens.back().kind != Token::kNewline) push(Token::kNewline, "\n");
      line++;
      k++;
      continue;
      }
    if (std::isspace((unsigned char) c)) { k++; continue; }
    if (c == '#' || (c == '/' && k + 1 < n && source[k+1] == '/'))
      {
      while (k < n && source[k] != '\n') k++;
      continue;
      }
    if (std::isdigit((unsigned char) c) || (c == '.' && k + 1 < n && std::isdigit((unsigned char) source[k+1])))
      {
      std::size_t start = k;
      bool real = false;
      while (k < n && std::isdigit((unsigned char) source[k])) k++;
      // a decimal point, unless it starts ".." (a range)
      if (k < n && source[k] == '.' && !(k + 1 < n && source[k+1] == '.'))
        {
        real = true;
        k++;
        while (k < n && std::isdigit((unsigned char) source[k])) k++;
        }
      if (k < n && (source[k] == 'e' || source[k] == 'E'))
        {
        std::size_t e = k + 1;
        if (e < n && (source[e] == '+' || source[e] == '-')) e++;
        if (e < n && std::isdigit((unsigned char) source[e]))
          {
          real = true;
          k = e;
          while (k < n && std::isdigit((unsigned char) source[k])) k++;
          }
        }
      std::string text = source.substr(start, k - start);
      Token * t = push(real ? Token::kReal : Token::kInteger, text);
      if (real) t->real = std::strtod(text.c_str(), nullptr);
      else
        {
        t->integer = std::strtol(text.c_str(), nullptr, 10);
        t->real = double(t->integer);
        }
      continue;
      }
    if (std::isalpha((unsigned char) c) || c == '_')
      {
      std::size_t start = k;
      while (k < n && (std::isalnum((unsigned char) source[k]) || source[k] == '_' || source[k] == ':'))
        {
        // "::" inside a name (CAP::MATH::Histogram); a single ':' ends it
        if (source[k] == ':' && !(k + 1 < n && source[k+1] == ':' && k + 2 < n && std::isalpha((unsigned char) source[k+2]))) break;
        k += source[k] == ':' ? 2 : 1;
        }
      std::string word = source.substr(start, k - start);
      push(keywords().count(word) ? Token::kKeyword : Token::kName, word);
      continue;
      }
    if (c == '"' || c == '\'')
      {
      char quote = c;
      std::string text;
      int startLine = line;
      k++;
      while (k < n && source[k] != quote)
        {
        char d = source[k++];
        if (d == '\n') line++;
        if (d == '\\' && k < n)
          {
          char e = source[k++];
          switch (e)
            {
            case 'n': d = '\n'; break;
            case 't': d = '\t'; break;
            default:  d = e; break;
            }
          }
        text += d;
        }
      if (k >= n) throw InterpreterException("Text not closed", startLine);
      k++;
      Token * t = push(Token::kText, text);
      t->line = startLine;
      continue;
      }
    bool found = false;
    for (const char * s : symbols)
      {
      std::size_t length = std::char_traits<char>::length(s);
      if (source.compare(k, length, s) == 0)
        {
        push(Token::kSymbol, s);
        if (*s == '(' || *s == '[') depth++;
        if ((*s == ')' || *s == ']') && depth > 0) depth--;
        k += length;
        found = true;
        break;
        }
      }
    if (!found) throw InterpreterException(String("Unexpected character '") + String(std::string(1, c)) + String("'"), line);
    }
  Token end;
  end.kind = Token::kEnd;
  end.line = line;
  tokens.push_back(end);
  return tokens;
  }

  bool isComplete(const std::string & source)
  {
  try
    {
    int depth = 0;
    for (const Token & t : tokenize(source))
      {
      if (t.is("(") || t.is("[") || t.is("{")) depth++;
      if (t.is(")") || t.is("]") || t.is("}")) depth--;
      }
    return depth <= 0;
    }
  catch (const InterpreterException & e)
    {
    return std::string(e.what()).find("Text not closed") == std::string::npos;   // other errors: run it, to report them
    }
  }

  } // namespace INTERPRETER
} // namespace CAP
