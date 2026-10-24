/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CapShell.cxx
//! @brief CapShell: the CAP command line interpreter (see Interpreter.hpp for the language).
//!
//! Usage:
//!   CapShell                  interactive session (line editing and history in ~/.cap_history when libedit is there)
//!   CapShell file.caps ...    run files of commands, then stop
//!   CapShell -i file.caps     run a file, then continue interactively
//!   CapShell -e "commands"    run commands, then stop
//! Commands are also read from the standard input when it is not a terminal.
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include "Interpreter.hpp"
#include "FileSystem.hpp"
#ifdef CAP_HAVE_EDITLINE
#include <editline/readline.h>
#endif

using namespace CAP;
using namespace CAP::INTERPRETER;

namespace
{
//! Read a line (false at the end of the input).
bool readLine(const std::string & prompt, bool terminal, std::string & line)
{
#ifdef CAP_HAVE_EDITLINE
  if (terminal)
    {
    char * text = readline(prompt.c_str());
    if (!text) return false;
    line = text;
    if (!line.empty()) add_history(text);
    std::free(text);
    return true;
    }
#endif
  if (terminal) { std::cout << prompt << std::flush; }
  return bool(std::getline(std::cin, line));
}

//! Run commands and report an error; false if there was one.
bool runReporting(Interpreter & interpreter, const std::string & source, int firstLine, const std::string & where, bool showResult)
{
  try
    {
    Value v = interpreter.run(source, firstLine);
    if (showResult && !v.isNone()) std::cout << "(" << v.typeName() << ") " << v.print() << std::endl;
    return true;
    }
  catch (const InterpreterException & e)
    {
    std::cerr << "error" << (where.empty() ? "" : " in " + where) << (e.line() > 0 ? " (line " + std::to_string(e.line()) + ")" : "")
              << ": " << e.what() << std::endl;
    }
  catch (const std::exception & e)
    {
    std::cerr << "error" << (where.empty() ? "" : " in " + where) << ": " << e.what() << std::endl;
    }
  return false;
}

void usage()
{
  std::cout << "Usage: CapShell [-i] [file.caps ...]   or   CapShell -e \"commands\"\n";
}
} // namespace

int main(int argc, char ** argv)
{
  Interpreter interpreter;
  bool interactive = false, ranSomething = false, ok = true;
  for (int k=1; k<argc; k++)
    {
    std::string a = argv[k];
    if (a == "-h" || a == "--help") { usage(); return 0; }
    if (a == "-i") { interactive = true; continue; }
    if (a == "-e")
      {
      if (++k >= argc) { usage(); return 1; }
      ok = runReporting(interpreter, argv[k], 1, "", true) && ok;
      ranSomething = true;
      continue;
      }
    try { interpreter.runFile(a); }
    catch (const InterpreterException & e)
      {
      std::cerr << "error in " << a << (e.line() > 0 ? " (line " + std::to_string(e.line()) + ")" : "") << ": " << e.what() << std::endl;
      ok = false;
      }
    ranSomething = true;
    if (interpreter.exitRequested()) return ok ? 0 : 1;
    }
  if (ranSomething && !interactive) return ok ? 0 : 1;

  bool terminal = isatty(0);
  std::string history = FileSystem::expand("~/.cap_history").str();
#ifdef CAP_HAVE_EDITLINE
  if (terminal) { using_history(); read_history(history.c_str()); }
#endif
  if (terminal) std::cout << "CAP command line interpreter: help for the language, classes() for the classes, exit() to leave\n";
  int count = 0, lineNumber = 1;
  std::string line;
  while (!interpreter.exitRequested())
    {
    std::string source;
    std::string prompt = "cap [" + std::to_string(count) + "] ";
    int firstLine = lineNumber;
    // read lines until the commands are complete (no parenthesis, bracket or brace left open)
    bool more = readLine(prompt, terminal, line);
    if (!more) break;
    source = line;
    lineNumber++;
    while (!isComplete(source))
      {
      if (!readLine(std::string(prompt.size() - 4, ' ') + "... ", terminal, line)) break;
      source += "\n" + line;
      lineNumber++;
      }
    if (source.find_first_not_of(" \t") == std::string::npos) continue;
    ok = runReporting(interpreter, source, terminal ? 1 : firstLine, "", terminal) && ok;
    count++;
    }
  if (terminal) std::cout << std::endl;
#ifdef CAP_HAVE_EDITLINE
  if (terminal) write_history(history.c_str());
#endif
  return (terminal || ok) ? 0 : 1;
}
