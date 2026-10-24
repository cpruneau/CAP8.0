/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Ast.hpp
//! @brief Syntax tree of the CAP command language: expressions and statements, evaluated by the Interpreter.
#ifndef CAP__INTERPRETER__Ast
#define CAP__INTERPRETER__Ast
#include <memory>
#include <string>
#include <vector>
#include "Reflection.hpp"

namespace CAP
{
  namespace INTERPRETER
  {

  using REFLECTION::Value;
  class Interpreter;

  //! @brief Node of the syntax tree, with the line of the program it comes from.
  struct Node
  {
  int line = 0;   //!< line of the program
  virtual ~Node() { }
  };

  //! @brief Expression: evaluated to a value.
  struct Expression : Node
  {
  //! @brief Value of the expression. @param interpreter interpreter (variables, functions) @return value
  virtual Value evaluate(Interpreter & interpreter) const = 0;
  };
  using ExpressionPtr = std::unique_ptr<Expression>;

  //! @brief Statement: executed.
  struct Statement : Node
  {
  //! @brief Execute the statement. @param interpreter interpreter
  virtual void execute(Interpreter & interpreter) const = 0;
  };
  using StatementPtr = std::unique_ptr<Statement>;

  // ---------------------------------------------------------------- expressions

  //! A literal: number, text, true, false, none.
  struct Literal : Expression { Value value; Value evaluate(Interpreter &) const override { return value; } };
  //! A variable.
  struct Variable : Expression { std::string name; Value evaluate(Interpreter & interpreter) const override; };
  //! A list: [a, b, c].
  struct ListLiteral : Expression { std::vector<ExpressionPtr> items; Value evaluate(Interpreter & interpreter) const override; };
  //! An operator with one operand: -a, not a.
  struct Unary : Expression { std::string op; ExpressionPtr operand; Value evaluate(Interpreter & interpreter) const override; };
  //! An operator with two operands: a + b, a < b, a and b...
  struct Binary : Expression { std::string op; ExpressionPtr left, right; Value evaluate(Interpreter & interpreter) const override; };
  //! A range of integers, both ends included: a..b.
  struct Range : Expression { ExpressionPtr first, last; Value evaluate(Interpreter & interpreter) const override; };
  //! A call of a function by name: a function of the program, a built-in function, or a constructor.
  struct Call : Expression { std::string name; std::vector<ExpressionPtr> arguments; Value evaluate(Interpreter & interpreter) const override; };
  //! A member function of an object, list or text: x.name(arguments).
  struct MethodCall : Expression
  {
  ExpressionPtr object; std::string name; std::vector<ExpressionPtr> arguments;
  Value evaluate(Interpreter & interpreter) const override;
  };
  //! A member of an object: x.name.
  struct Member : Expression { ExpressionPtr object; std::string name; Value evaluate(Interpreter & interpreter) const override; };
  //! An element of a list or text: x[i].
  struct Index : Expression { ExpressionPtr object, index; Value evaluate(Interpreter & interpreter) const override; };

  // ---------------------------------------------------------------- statements

  //! A sequence of statements.
  struct Block : Statement { std::vector<StatementPtr> statements; void execute(Interpreter & interpreter) const override; };
  //! An expression evaluated for its effect (its value is the result shown at the prompt).
  struct ExpressionStatement : Statement { ExpressionPtr expression; void execute(Interpreter & interpreter) const override; };
  //! An assignment: target = value, or target op= value (target: a variable, a member, an element).
  struct Assignment : Statement
  {
  ExpressionPtr target; std::string op; ExpressionPtr value;
  void execute(Interpreter & interpreter) const override;
  };
  //! if condition { ... } else ...
  struct If : Statement { ExpressionPtr condition; StatementPtr then, otherwise; void execute(Interpreter & interpreter) const override; };
  //! while condition { ... }
  struct While : Statement { ExpressionPtr condition; StatementPtr body; void execute(Interpreter & interpreter) const override; };
  //! for name in values { ... }
  struct For : Statement { std::string name; ExpressionPtr values; StatementPtr body; void execute(Interpreter & interpreter) const override; };
  //! fun name(parameters) { ... }
  struct FunctionDefinition : Statement
  {
  std::string name; std::vector<std::string> parameters; std::shared_ptr<Block> body;
  void execute(Interpreter & interpreter) const override;
  };
  //! return value
  struct Return : Statement { ExpressionPtr value; void execute(Interpreter & interpreter) const override; };
  //! break
  struct Break : Statement { void execute(Interpreter & interpreter) const override; };
  //! continue
  struct Continue : Statement { void execute(Interpreter & interpreter) const override; };
  //! help, help name
  struct Help : Statement { std::string topic; void execute(Interpreter & interpreter) const override; };

  //!
  //! @brief Syntax tree of a program. @param source program @param firstLine number of its first line
  //! @return the statements of the program
  //! @throws InterpreterException for a syntax error
  //!
  std::shared_ptr<Block> parse(const std::string & source, int firstLine=1);

  } // namespace INTERPRETER
} // namespace CAP

#endif // !CAP__INTERPRETER__Ast
