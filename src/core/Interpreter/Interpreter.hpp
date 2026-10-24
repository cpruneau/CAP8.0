/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Interpreter.hpp
//! @brief Interpreter of the CAP command language: variables, functions, and the classes of the reflection registry.
#ifndef CAP__INTERPRETER__Interpreter
#define CAP__INTERPRETER__Interpreter
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "Ast.hpp"
#include "Lexer.hpp"

namespace CAP
{
  namespace INTERPRETER
  {

  //!
  //! @brief Interpreter of the CAP command language.
  //!
  //! The language has variables (made by assignment), numbers (integers and reals), texts, booleans, none, lists,
  //! and the objects of the classes registered for the reflection (see Reflection.hpp): an object is made by calling
  //! its class (h = Histogram("h", "title", 40, -4, 4)), and used through its member functions and members
  //! (h.fill(1.5), h.name, h.name = "other").
  //! @code
  //!   g = MersenneTwister(12345)
  //!   h = Histogram("h", "Gaussian numbers", 40, -4, 4)
  //!   for k in 1..10000 { h.fill(g.gauss(0, 1)) }
  //!   fun square(x) { return x*x }
  //!   if h.mean(0) > 0 { print("positive mean", h.mean(0)) } else { print("negative mean") }
  //!   save("gauss.pdf", h)
  //! @endcode
  //!
  //! Integers stay integers through +, -, *, % and ^ (with a non-negative exponent); / always gives a real.
  //! A function sees its parameters and the variables it assigns, and reads (and assigns) the global variables
  //! that exist. Built-in functions (print, sqrt, range, read, save, ...) are listed by help.
  //!
  class Interpreter
  {
  public:

  //! @brief A built-in function: called with the values of its arguments and the line of the call.
  using Builtin = std::function<Value(Interpreter &, const std::vector<Value> &, int line)>;

  //! @brief Interpreter writing to out (print, help). @param out stream of the output
  explicit Interpreter(std::ostream & out=std::cout);

  //!
  //! @brief Run a program. @param source program @param firstLine number of its first line (for the messages)
  //! @return value of the last statement if it is an expression, none otherwise
  //! @throws InterpreterException for a syntax error or an error while running (with its line)
  //!
  Value run(const std::string & source, int firstLine=1);
  //! @brief Run a file of the language. @param path file @return value of the last statement
  //! @throws InterpreterException if it cannot be read, or for an error in it
  Value runFile(const std::string & path);

  //! @brief Value of a variable. @param name variable @return value @throws InterpreterException if it is not defined
  Value variable(const std::string & name, int line=0) const;
  //! @brief Whether a variable is defined. @param name variable @return true if so
  bool hasVariable(const std::string & name) const;
  //! @brief Set a variable (local in a function, if it is not a global variable). @param name name @param value value
  void setVariable(const std::string & name, const Value & value);
  //! @brief Names of the global variables. @return names
  std::vector<std::string> variableNames() const;

  //! @brief Add (or replace) a built-in function. @param name name @param usage signature shown by help
  //! @param description description shown by help @param function function
  void addBuiltin(const std::string & name, const std::string & usage, const std::string & description, Builtin function);
  //! @brief Call a function by name: of the program, built in, or a constructor. @param name name
  //! @param arguments arguments @param line line of the call @return result
  Value call(const std::string & name, const std::vector<Value> & arguments, int line);
  //! @brief Text of help: general (empty topic), or of a built-in function, a function of the program, or a class.
  //! @param topic topic @return text
  std::string help(const std::string & topic) const;
  //! @brief Stream of the output. @return stream
  std::ostream & out() { return _out; }
  //! @brief Whether exit() was called. @return true if so
  bool exitRequested() const { return _exit; }
  //! @brief Ask to stop (exit()). @param v true to stop
  void requestExit(bool v=true) { _exit = v; }

  //! @name Used by the syntax tree
  //! @{
  void defineFunction(const FunctionDefinition & f) { _functions[f.name] = &f; }   //!< Define a function
  void setLastValue(const Value & v) { _last = v; }   //!< Value of the last expression statement
  //! @brief Value of a binary operator. @param op operator @param a,b operands @param line line @return value
  static Value binary(const std::string & op, const Value & a, const Value & b, int line);
  //! @brief Whether a value counts as true (none, false, 0, "", [] are false). @param v value @return true if so
  static bool truth(const Value & v);
  //! @brief Reference to the storage of a variable (null if undefined). @param name variable @return storage
  Value * storage(const std::string & name);
  //! @}

  private:
  //! A built-in function and its help
  struct BuiltinEntry { std::string usage; std::string description; Builtin function; };
  std::ostream & _out;   //!< output
  std::map<std::string, Value> _globals;   //!< global variables
  std::vector<std::map<std::string, Value>> _frames;   //!< variables of the functions being called
  std::map<std::string, const FunctionDefinition *> _functions;   //!< functions of the programs
  std::map<std::string, BuiltinEntry> _builtins;   //!< built-in functions
  std::vector<std::shared_ptr<Block>> _programs;   //!< programs run (they own the functions defined)
  Value _last;   //!< value of the last expression statement
  bool _exit;   //!< exit() was called
  };

  //! @brief Add the standard built-in functions (see help): output, mathematics, lists, classes, CAP files, figures.
  //! Called by the constructor of Interpreter. @param interpreter interpreter
  void addStandardBuiltins(Interpreter & interpreter);

  } // namespace INTERPRETER
} // namespace CAP

#endif // !CAP__INTERPRETER__Interpreter
