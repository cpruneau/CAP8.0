/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Parser.cpp
//! @brief Parser of the CAP command language (recursive descent).
//!
//! Grammar, from the lowest precedence to the highest:
//! @code
//!   program    : statement*                     (separated by newlines or ';')
//!   statement  : 'fun' name '(' names ')' block | 'if' expr block ('else' (if | block))? | 'while' expr block
//!              | 'for' name 'in' expr block | 'return' expr? | 'break' | 'continue' | 'help' name?
//!              | target ('=' | '+=' | '-=' | '*=' | '/=') expr | expr
//!   block      : '{' statement* '}'
//!   expr       : or ;  or : and ('or' | '||') ... ;  and : not ('and' | '&&') ... ;  not : ('not' | '!') not | comparison
//!   comparison : range (('==' | '!=' | '<' | '<=' | '>' | '>=') range)?
//!   range      : sum ('..' sum)?
//!   sum        : product (('+' | '-') product)* ;  product : unary (('*' | '/' | '%') unary)*
//!   unary      : '-' unary | '+' unary | power ;  power : postfix (('^' | '**') unary)?
//!   postfix    : primary ('(' args ')' | '.' name ('(' args ')')? | '[' expr ']')*
//!   primary    : number | text | 'true' | 'false' | 'none' | name | '[' args ']' | '(' expr ')'
//! @endcode
#include "Ast.hpp"
#include "Lexer.hpp"

namespace CAP
{
  namespace INTERPRETER
  {

  namespace
  {
  class Parser
  {
  public:
  explicit Parser(std::vector<Token> tokens) : _tokens(std::move(tokens)), _at(0) { }

  std::shared_ptr<Block> program()
  {
  auto block = std::make_shared<Block>();
  block->line = peek().line;
  skipSeparators();
  while (peek().kind != Token::kEnd)
    {
    block->statements.push_back(statement());
    endOfStatement();
    }
  return block;
  }

  private:
  std::vector<Token> _tokens;
  std::size_t _at;

  const Token & peek(std::size_t ahead=0) const { return _tokens[std::min(_at + ahead, _tokens.size() - 1)]; }
  const Token & next() { const Token & t = peek(); if (_at < _tokens.size() - 1) _at++; return t; }
  bool accept(const char * s) { if (peek().is(s)) { next(); return true; } return false; }
  [[noreturn]] void error(const std::string & message) const
  {
  const Token & t = peek();
  std::string found = t.kind == Token::kEnd ? "the end" : t.kind == Token::kNewline ? "the end of the line"
                      : t.kind == Token::kText ? "\"" + t.text + "\"" : "'" + t.text + "'";
  throw InterpreterException(String(message + ", found " + found), t.line);
  }
  void expect(const char * s) { if (!accept(s)) error(std::string("Expected '") + s + "'"); }
  std::string name()
  {
  if (peek().kind != Token::kName) error("Expected a name");
  return next().text;
  }
  void skipSeparators() { while (peek().kind == Token::kNewline || peek().is(";")) next(); }
  void skipNewlines() { while (peek().kind == Token::kNewline) next(); }
  void endOfStatement()
  {
  if (peek().kind == Token::kEnd || peek().is("}")) return;
  if (peek().kind != Token::kNewline && !peek().is(";")) error("Expected the end of the statement");
  skipSeparators();
  }

  template <typename T> std::unique_ptr<T> make(int line) { auto p = std::make_unique<T>(); p->line = line; return p; }

  std::shared_ptr<Block> block()
  {
  skipNewlines();
  int line = peek().line;
  expect("{");
  auto b = std::make_shared<Block>();
  b->line = line;
  skipSeparators();
  while (!peek().is("}"))
    {
    if (peek().kind == Token::kEnd) error("Expected '}'");
    b->statements.push_back(statement());
    endOfStatement();
    }
  next();
  return b;
  }

  //! A block as a statement (shared pointer to unique pointer: a copy of the node list is not needed)
  StatementPtr blockStatement()
  {
  std::shared_ptr<Block> b = block();
  auto s = std::make_unique<Block>();
  s->line = b->line;
  s->statements = std::move(b->statements);
  return s;
  }

  StatementPtr statement()
  {
  const Token & t = peek();
  int line = t.line;
  if (accept("fun"))
    {
    auto f = make<FunctionDefinition>(line);
    f->name = name();
    expect("(");
    if (!peek().is(")"))
      do f->parameters.push_back(name()); while (accept(","));
    expect(")");
    f->body = block();
    return f;
    }
  if (accept("if")) return ifStatement(line);
  if (accept("while"))
    {
    auto w = make<While>(line);
    w->condition = expression();
    w->body = blockStatement();
    return w;
    }
  if (accept("for"))
    {
    auto f = make<For>(line);
    f->name = name();
    expect("in");
    f->values = expression();
    f->body = blockStatement();
    return f;
    }
  if (accept("return"))
    {
    auto r = make<Return>(line);
    if (peek().kind != Token::kNewline && peek().kind != Token::kEnd && !peek().is(";") && !peek().is("}")) r->value = expression();
    return r;
    }
  if (accept("break")) return make<Break>(line);
  if (accept("continue")) return make<Continue>(line);
  if (peek().is("help") && !peek(1).is("("))
    {
    next();
    auto h = make<Help>(line);
    if (peek().kind == Token::kName || peek().kind == Token::kKeyword) h->topic = next().text;
    return h;
    }
  ExpressionPtr e = expression();
  for (const char * op : { "=", "+=", "-=", "*=", "/=" })
    if (peek().is(op))
      {
      if (!dynamic_cast<Variable *>(e.get()) && !dynamic_cast<Member *>(e.get()) && !dynamic_cast<Index *>(e.get()))
        error("Only a variable, a member or an element can be assigned");
      next();
      auto a = make<Assignment>(line);
      a->target = std::move(e);
      a->op = op;
      a->value = expression();
      return a;
      }
  auto s = make<ExpressionStatement>(line);
  s->expression = std::move(e);
  return s;
  }

  StatementPtr ifStatement(int line)
  {
  auto s = make<If>(line);
  s->condition = expression();
  s->then = blockStatement();
  // "else" may follow on the next line
  std::size_t save = _at;
  skipNewlines();
  if (accept("else"))
    {
    int elseLine = peek().line;
    if (accept("if")) s->otherwise = ifStatement(elseLine);
    else s->otherwise = blockStatement();
    }
  else _at = save;
  return s;
  }

  ExpressionPtr expression() { return orExpression(); }

  ExpressionPtr binary(ExpressionPtr left, const std::string & op, ExpressionPtr right, int line)
  {
  auto b = make<Binary>(line);
  b->op = op;
  b->left = std::move(left);
  b->right = std::move(right);
  return b;
  }

  ExpressionPtr orExpression()
  {
  ExpressionPtr e = andExpression();
  while (peek().is("or") || peek().is("||")) { int line = next().line; e = binary(std::move(e), "or", andExpression(), line); }
  return e;
  }
  ExpressionPtr andExpression()
  {
  ExpressionPtr e = notExpression();
  while (peek().is("and") || peek().is("&&")) { int line = next().line; e = binary(std::move(e), "and", notExpression(), line); }
  return e;
  }
  ExpressionPtr notExpression()
  {
  if (peek().is("not") || peek().is("!"))
    {
    auto u = make<Unary>(next().line);
    u->op = "not";
    u->operand = notExpression();
    return u;
    }
  return comparison();
  }
  ExpressionPtr comparison()
  {
  ExpressionPtr e = range();
  for (const char * op : { "==", "!=", "<=", ">=", "<", ">" })
    if (peek().is(op)) { int line = next().line; return binary(std::move(e), op, range(), line); }
  return e;
  }
  ExpressionPtr range()
  {
  ExpressionPtr e = sum();
  if (peek().is(".."))
    {
    auto r = make<Range>(next().line);
    r->first = std::move(e);
    r->last = sum();
    return r;
    }
  return e;
  }
  ExpressionPtr sum()
  {
  ExpressionPtr e = product();
  while (peek().is("+") || peek().is("-")) { const Token & t = next(); std::string op = t.text; e = binary(std::move(e), op, product(), t.line); }
  return e;
  }
  ExpressionPtr product()
  {
  ExpressionPtr e = unary();
  while (peek().is("*") || peek().is("/") || peek().is("%")) { const Token & t = next(); std::string op = t.text; e = binary(std::move(e), op, unary(), t.line); }
  return e;
  }
  ExpressionPtr unary()
  {
  if (peek().is("-") || peek().is("+"))
    {
    const Token & t = next();
    auto u = make<Unary>(t.line);
    u->op = t.text;
    u->operand = unary();
    return u;
    }
  return power();
  }
  ExpressionPtr power()
  {
  ExpressionPtr e = postfix();
  if (peek().is("^") || peek().is("**")) { int line = next().line; return binary(std::move(e), "^", unary(), line); }
  return e;
  }
  std::vector<ExpressionPtr> arguments(const char * close)
  {
  std::vector<ExpressionPtr> list;
  if (!peek().is(close))
    do list.push_back(expression()); while (accept(","));
  expect(close);
  return list;
  }
  ExpressionPtr postfix()
  {
  ExpressionPtr e = primary();
  while (true)
    {
    int line = peek().line;
    if (accept("("))
      {
      Variable * v = dynamic_cast<Variable *>(e.get());
      if (!v) error("Only named functions can be called");
      auto c = make<Call>(line);
      c->name = v->name;
      c->arguments = arguments(")");
      e = std::move(c);
      }
    else if (accept("."))
      {
      std::string member = name();
      if (accept("("))
        {
        auto m = make<MethodCall>(line);
        m->object = std::move(e);
        m->name = member;
        m->arguments = arguments(")");
        e = std::move(m);
        }
      else
        {
        auto m = make<Member>(line);
        m->object = std::move(e);
        m->name = member;
        e = std::move(m);
        }
      }
    else if (accept("["))
      {
      auto i = make<Index>(line);
      i->object = std::move(e);
      i->index = expression();
      expect("]");
      e = std::move(i);
      }
    else return e;
    }
  }
  ExpressionPtr primary()
  {
  const Token & t = peek();
  int line = t.line;
  auto literal = [&](const Value & v) { auto l = make<Literal>(line); l->value = v; next(); return l; };
  switch (t.kind)
    {
    case Token::kInteger: return literal(Value(t.integer));
    case Token::kReal:    return literal(Value(t.real));
    case Token::kText:    return literal(Value(t.text));
    case Token::kName:
      {
      auto v = make<Variable>(line);
      v->name = next().text;
      return v;
      }
    default: break;
    }
  if (t.is("true"))  return literal(Value(true));
  if (t.is("false")) return literal(Value(false));
  if (t.is("none"))  return literal(Value());
  if (t.is("help"))   // help(x): the built-in function
    {
    auto v = make<Variable>(line);
    v->name = next().text;
    return v;
    }
  if (accept("["))
    {
    auto l = make<ListLiteral>(line);
    l->items = arguments("]");
    return l;
    }
  if (accept("("))
    {
    ExpressionPtr e = expression();
    expect(")");
    return e;
    }
  error("Expected a value");
  }
  };
  } // namespace

  std::shared_ptr<Block> parse(const std::string & source, int firstLine)
  {
  Parser parser(tokenize(source, firstLine));
  return parser.program();
  }

  } // namespace INTERPRETER
} // namespace CAP
