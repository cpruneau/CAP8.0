/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <cmath>
#include <fstream>
#include <sstream>
#include "Interpreter.hpp"

namespace CAP
{
  namespace INTERPRETER
  {

  using namespace REFLECTION;

  namespace
  {
  //! Control flow, carried through the evaluation of the statements.
  struct BreakSignal { };
  struct ContinueSignal { };
  struct ReturnSignal { Value value; };

  //! Error of the reflection (or another C++ error) as an error of the program, at a line.
  template <typename F>
  auto atLine(int line, F && f) -> decltype(f())
  {
  try { return f(); }
  catch (const InterpreterException &) { throw; }
  catch (const BreakSignal &) { throw; }
  catch (const ContinueSignal &) { throw; }
  catch (const ReturnSignal &) { throw; }
  catch (const std::exception & e) { throw InterpreterException(String(e.what()), line); }
  }

  bool isNumber(const Value & v) { return v.kind() == Value::kInteger || v.kind() == Value::kReal || v.kind() == Value::kBool; }

  //! Text of a value for print() and concatenation: texts without quotes.
  std::string raw(const Value & v) { return v.kind() == Value::kText ? v.toText() : v.print(); }

  //! Element k of a list or text (negative: from the end).
  std::size_t position(long k, std::size_t size, int line)
  {
  long i = k < 0 ? long(size) + k : k;
  if (i < 0 || i >= long(size)) throw InterpreterException(String("Index ") + String(std::to_string(k)) + String(" out of range (size ") + String(std::to_string(size)) + String(")"), line);
  return std::size_t(i);
  }

  std::vector<Value> evaluateAll(Interpreter & interpreter, const std::vector<ExpressionPtr> & expressions)
  {
  std::vector<Value> values;
  for (const auto & e : expressions) values.push_back(e->evaluate(interpreter));
  return values;
  }

  //! Ends of a range as integers.
  void rangeEnds(Interpreter & interpreter, const Range & r, long & first, long & last)
  {
  Value a = r.first->evaluate(interpreter), b = r.last->evaluate(interpreter);
  atLine(r.line, [&]() { first = a.toInteger(); last = b.toInteger(); return 0; });
  }
  } // namespace

  // ------------------------------------------------------------------------------------------------ interpreter

  Interpreter::Interpreter(std::ostream & out)
  :
  _out(out),
  _globals(),
  _frames(),
  _functions(),
  _builtins(),
  _programs(),
  _last(),
  _exit(false)
  {
  addStandardBuiltins(*this);
  }

  Value Interpreter::run(const std::string & source, int firstLine)
  {
  std::shared_ptr<Block> program = parse(source, firstLine);
  _programs.push_back(program);
  _last = Value();
  std::size_t depth = _frames.size();
  try
    {
    for (const auto & s : program->statements)
      {
      _last = Value();
      s->execute(*this);
      if (_exit) break;
      }
    }
  catch (const BreakSignal &)    { _frames.resize(depth); throw InterpreterException("break outside a loop", 0); }
  catch (const ContinueSignal &) { _frames.resize(depth); throw InterpreterException("continue outside a loop", 0); }
  catch (const ReturnSignal &)   { _frames.resize(depth); throw InterpreterException("return outside a function", 0); }
  catch (...) { _frames.resize(depth); throw; }
  return _last;
  }

  Value Interpreter::runFile(const std::string & path)
  {
  std::ifstream in(path);
  if (!in) throw InterpreterException(String("Cannot read ") + String(path), 0);
  std::stringstream text;
  text << in.rdbuf();
  return run(text.str());
  }

  Value * Interpreter::storage(const std::string & name)
  {
  if (!_frames.empty())
    {
    auto found = _frames.back().find(name);
    if (found != _frames.back().end()) return &found->second;
    }
  auto found = _globals.find(name);
  return found == _globals.end() ? nullptr : &found->second;
  }

  Value Interpreter::variable(const std::string & name, int line) const
  {
  Value * v = const_cast<Interpreter *>(this)->storage(name);
  if (!v) throw InterpreterException(String("Unknown variable ") + String(name), line);
  return *v;
  }

  bool Interpreter::hasVariable(const std::string & name) const { return const_cast<Interpreter *>(this)->storage(name) != nullptr; }

  void Interpreter::setVariable(const std::string & name, const Value & value)
  {
  if (Value * v = storage(name)) { *v = value; return; }
  if (_frames.empty()) _globals[name] = value;
  else _frames.back()[name] = value;
  }

  std::vector<std::string> Interpreter::variableNames() const
  {
  std::vector<std::string> names;
  for (const auto & e : _globals) names.push_back(e.first);
  return names;
  }

  void Interpreter::addBuiltin(const std::string & name, const std::string & usage, const std::string & description, Builtin function)
  {
  _builtins[name] = BuiltinEntry{ usage, description, std::move(function) };
  }

  Value Interpreter::call(const std::string & name, const std::vector<Value> & arguments, int line)
  {
  auto f = _functions.find(name);
  if (f != _functions.end())
    {
    const FunctionDefinition & d = *f->second;
    if (arguments.size() != d.parameters.size())
      throw InterpreterException(String(name) + String(" takes ") + String(std::to_string(d.parameters.size())) + String(" arguments, not ")
                                 + String(std::to_string(arguments.size())), line);
    if (_frames.size() >= 1000) throw InterpreterException("Too many nested calls", line);
    std::map<std::string, Value> frame;
    for (std::size_t k=0; k<arguments.size(); k++) frame[d.parameters[k]] = arguments[k];
    _frames.push_back(frame);
    try { d.body->execute(*this); }
    catch (ReturnSignal & r) { _frames.pop_back(); return r.value; }
    catch (const BreakSignal &)    { _frames.pop_back(); throw InterpreterException("break outside a loop", line); }
    catch (const ContinueSignal &) { _frames.pop_back(); throw InterpreterException("continue outside a loop", line); }
    catch (...) { _frames.pop_back(); throw; }
    _frames.pop_back();
    return Value();
    }
  auto b = _builtins.find(name);
  if (b != _builtins.end()) return atLine(line, [&]() { return b->second.function(*this, arguments, line); });
  if (const ClassInfo * c = Registry::instance().find(name)) return atLine(line, [&]() { return c->create(arguments); });
  throw InterpreterException(String("Unknown function or class ") + String(name), line);
  }

  std::string Interpreter::help(const std::string & topic) const
  {
  std::ostringstream out;
  if (topic.empty())
    {
    out << "CAP command language\n"
           "  x = 3.5; t = \"text\"; l = [1, 2, 3]       variables: numbers, texts, lists, true/false, none\n"
           "  h = Histogram(\"h\", \"title\", 40, -4, 4)    an object of a registered class (classes() lists them)\n"
           "  h.fill(1.2); h.mean(0); h.name = \"g\"      its functions and members (help Histogram)\n"
           "  + - * / % ^   == != < <= > >=   and or not   a..b (integers a to b)   l[0] l[-1]\n"
           "  if c { ... } else { ... }   while c { ... }   for x in 1..10 { ... }   for x in l { ... }\n"
           "  fun f(a, b) { return a + b }              break, continue, return\n"
           "  # comment                                 ';' or a new line between statements\n"
           "Built-in functions:\n";
    for (const auto & e : _builtins) out << "  " << e.second.usage << "\n";
    if (!_functions.empty())
      {
      out << "Functions of the session:";
      for (const auto & e : _functions) out << " " << e.first;
      out << "\n";
      }
    return out.str();
    }
  auto b = _builtins.find(topic);
  if (b != _builtins.end()) return b->second.usage + "\n  " + b->second.description + "\n";
  auto f = _functions.find(topic);
  if (f != _functions.end())
    {
    out << "fun " << topic << "(";
    for (std::size_t k=0; k<f->second->parameters.size(); k++) out << (k ? ", " : "") << f->second->parameters[k];
    out << ")   (line " << f->second->line << ")\n";
    return out.str();
    }
  if (const ClassInfo * c = Registry::instance().find(topic)) return c->describe();
  if (const Value * v = const_cast<Interpreter *>(this)->storage(topic))
    if (v->kind() == Value::kInstance) return v->toInstance().type->describe();
  return "Nothing is known about " + topic + " (help lists the built-in functions, classes() the classes)\n";
  }

  bool Interpreter::truth(const Value & v)
  {
  switch (v.kind())
    {
    case Value::kNone:     return false;
    case Value::kBool:     return v.toBool();
    case Value::kInteger:  return v.toInteger() != 0;
    case Value::kReal:     return v.toReal() != 0.0;
    case Value::kText:     return !v.toText().empty();
    case Value::kInstance: return v.toInstance().pointer != nullptr;
    case Value::kList:     return !v.toList().empty();
    }
  return false;
  }

  namespace
  {
  bool equal(const Value & a, const Value & b)
  {
  if (isNumber(a) && isNumber(b)) return a.toReal() == b.toReal();
  if (a.kind() != b.kind()) return false;
  switch (a.kind())
    {
    case Value::kNone:     return true;
    case Value::kText:     return a.toText() == b.toText();
    case Value::kInstance: return a.toInstance().pointer == b.toInstance().pointer;
    case Value::kList:
      {
      const auto & x = a.toList(), & y = b.toList();
      if (x.size() != y.size()) return false;
      for (std::size_t k=0; k<x.size(); k++) if (!equal(x[k], y[k])) return false;
      return true;
      }
    default: return false;
    }
  }
  } // namespace

  Value Interpreter::binary(const std::string & op, const Value & a, const Value & b, int line)
  {
  auto fail = [&]() -> Value
    { throw InterpreterException(String("Cannot apply '") + String(op) + String("' to ") + String(a.typeName()) + String(" and ") + String(b.typeName()), line); };
  bool integers = (a.kind() == Value::kInteger || a.kind() == Value::kBool) && (b.kind() == Value::kInteger || b.kind() == Value::kBool);
  if (op == "==") return Value(equal(a, b));
  if (op == "!=") return Value(!equal(a, b));
  if (op == "+")
    {
    if (integers) return Value(a.toInteger() + b.toInteger());
    if (isNumber(a) && isNumber(b)) return Value(a.toReal() + b.toReal());
    if (a.kind() == Value::kText || b.kind() == Value::kText) return Value(raw(a) + raw(b));
    if (a.kind() == Value::kList && b.kind() == Value::kList)
      {
      std::vector<Value> list = a.toList();
      list.insert(list.end(), b.toList().begin(), b.toList().end());
      return Value(list);
      }
    return fail();
    }
  if (op == "<" || op == "<=" || op == ">" || op == ">=")
    {
    int c;
    if (isNumber(a) && isNumber(b)) c = a.toReal() < b.toReal() ? -1 : (a.toReal() > b.toReal() ? 1 : 0);
    else if (a.kind() == Value::kText && b.kind() == Value::kText) c = a.toText().compare(b.toText());
    else return fail();
    if (op == "<")  return Value(c < 0);
    if (op == "<=") return Value(c <= 0);
    if (op == ">")  return Value(c > 0);
    return Value(c >= 0);
    }
  if (!isNumber(a) || !isNumber(b)) return fail();
  if (op == "-") return integers ? Value(a.toInteger() - b.toInteger()) : Value(a.toReal() - b.toReal());
  if (op == "*") return integers ? Value(a.toInteger() * b.toInteger()) : Value(a.toReal() * b.toReal());
  if (op == "/")
    {
    if (b.toReal() == 0.0) throw InterpreterException("Division by zero", line);
    return Value(a.toReal()/b.toReal());
    }
  if (op == "%")
    {
    if (b.toReal() == 0.0) throw InterpreterException("Division by zero", line);
    if (integers) { long m = a.toInteger() % b.toInteger(); if (m != 0 && ((m < 0) != (b.toInteger() < 0))) m += b.toInteger(); return Value(m); }
    double m = std::fmod(a.toReal(), b.toReal());
    if (m != 0 && ((m < 0) != (b.toReal() < 0))) m += b.toReal();
    return Value(m);
    }
  if (op == "^")
    {
    if (integers && b.toInteger() >= 0)
      {
      long result = 1, base = a.toInteger();
      for (long e = b.toInteger(); e > 0; e >>= 1) { if (e & 1) result *= base; base *= base; }
      return Value(result);
      }
    return Value(std::pow(a.toReal(), b.toReal()));
    }
  return fail();
  }

  // ------------------------------------------------------------------------------------------------ expressions

  Value Variable::evaluate(Interpreter & interpreter) const { return interpreter.variable(name, line); }

  Value ListLiteral::evaluate(Interpreter & interpreter) const { return Value(evaluateAll(interpreter, items)); }

  Value Unary::evaluate(Interpreter & interpreter) const
  {
  Value v = operand->evaluate(interpreter);
  if (op == "not") return Value(!Interpreter::truth(v));
  if (v.kind() == Value::kInteger) return Value(op == "-" ? -v.toInteger() : v.toInteger());
  if (v.kind() == Value::kReal) return Value(op == "-" ? -v.toReal() : v.toReal());
  throw InterpreterException(String("Cannot apply '") + String(op) + String("' to ") + String(v.typeName()), line);
  }

  Value Binary::evaluate(Interpreter & interpreter) const
  {
  // and, or: the right operand only when needed
  if (op == "and") return Value(Interpreter::truth(left->evaluate(interpreter)) && Interpreter::truth(right->evaluate(interpreter)));
  if (op == "or")  return Value(Interpreter::truth(left->evaluate(interpreter)) || Interpreter::truth(right->evaluate(interpreter)));
  Value a = left->evaluate(interpreter);
  Value b = right->evaluate(interpreter);
  return Interpreter::binary(op, a, b, line);
  }

  Value Range::evaluate(Interpreter & interpreter) const
  {
  long a, b;
  rangeEnds(interpreter, *this, a, b);
  if (b - a > 100000000) throw InterpreterException("Range too long for a list (use it in a for loop)", line);
  std::vector<Value> list;
  for (long k=a; k<=b; k++) list.push_back(Value(k));
  return Value(list);
  }

  Value Call::evaluate(Interpreter & interpreter) const
  {
  std::vector<Value> values = evaluateAll(interpreter, arguments);
  return interpreter.call(name, values, line);
  }

  Value MethodCall::evaluate(Interpreter & interpreter) const
  {
  // a list held by a variable can be changed in place: l.append(x)
  Value * stored = nullptr;
  if (const Variable * v = dynamic_cast<const Variable *>(object.get())) stored = interpreter.storage(v->name);
  Value o = stored ? *stored : object->evaluate(interpreter);
  std::vector<Value> values = evaluateAll(interpreter, arguments);
  if (o.kind() == Value::kList || o.kind() == Value::kText)
    {
    std::size_t size = o.kind() == Value::kList ? o.toList().size() : o.toText().size();
    if (name == "size" && values.empty()) return Value(long(size));
    if (o.kind() == Value::kList && name == "append" && values.size() == 1)
      {
      if (!stored) throw InterpreterException("append: only a list held by a variable can be changed", line);
      std::vector<Value> list = o.toList();
      list.push_back(values[0]);
      *stored = Value(list);
      return Value();
      }
    throw InterpreterException(String(o.typeName()) + String(" has no function ") + String(name) + String(" (size(), and append(x) for lists)"), line);
    }
  if (o.kind() != Value::kInstance) throw InterpreterException(String(o.typeName()) + String(" has no function ") + String(name), line);
  return atLine(line, [&]() { return o.call(name, values); });
  }

  Value Member::evaluate(Interpreter & interpreter) const
  {
  Value o = object->evaluate(interpreter);
  if (o.kind() != Value::kInstance) throw InterpreterException(String(o.typeName()) + String(" has no member ") + String(name), line);
  return atLine(line, [&]() { return o.get(name); });
  }

  Value Index::evaluate(Interpreter & interpreter) const
  {
  Value o = object->evaluate(interpreter);
  Value i = index->evaluate(interpreter);
  long k = atLine(line, [&]() { return i.toInteger(); });
  if (o.kind() == Value::kList) return o.toList()[position(k, o.toList().size(), line)];
  if (o.kind() == Value::kText) return Value(std::string(1, o.toText()[position(k, o.toText().size(), line)]));
  throw InterpreterException(String(o.typeName()) + String(" has no elements"), line);
  }

  // ------------------------------------------------------------------------------------------------ statements

  void Block::execute(Interpreter & interpreter) const
  {
  for (const auto & s : statements) { s->execute(interpreter); if (interpreter.exitRequested()) return; }
  }

  void ExpressionStatement::execute(Interpreter & interpreter) const
  {
  interpreter.setLastValue(expression->evaluate(interpreter));
  }

  void Assignment::execute(Interpreter & interpreter) const
  {
  Value v = value->evaluate(interpreter);
  if (op != "=")
    {
    Value current = target->evaluate(interpreter);
    v = Interpreter::binary(op.substr(0, 1), current, v, line);
    }
  if (const Variable * var = dynamic_cast<const Variable *>(target.get())) { interpreter.setVariable(var->name, v); return; }
  if (const Member * m = dynamic_cast<const Member *>(target.get()))
    {
    Value o = m->object->evaluate(interpreter);
    if (o.kind() != Value::kInstance) throw InterpreterException(String(o.typeName()) + String(" has no member ") + String(m->name), line);
    atLine(line, [&]() { o.set(m->name, v); return 0; });
    return;
    }
  if (const Index * i = dynamic_cast<const Index *>(target.get()))
    {
    const Variable * var = dynamic_cast<const Variable *>(i->object.get());
    Value * stored = var ? interpreter.storage(var->name) : nullptr;
    if (!stored || stored->kind() != Value::kList) throw InterpreterException("Only the elements of a list held by a variable can be assigned", line);
    long k = atLine(line, [&]() { return i->index->evaluate(interpreter).toInteger(); });
    std::vector<Value> list = stored->toList();
    list[position(k, list.size(), line)] = v;
    *stored = Value(list);
    }
  }

  void If::execute(Interpreter & interpreter) const
  {
  if (Interpreter::truth(condition->evaluate(interpreter))) then->execute(interpreter);
  else if (otherwise) otherwise->execute(interpreter);
  }

  void While::execute(Interpreter & interpreter) const
  {
  while (Interpreter::truth(condition->evaluate(interpreter)))
    {
    try { body->execute(interpreter); }
    catch (const BreakSignal &) { break; }
    catch (const ContinueSignal &) { continue; }
    if (interpreter.exitRequested()) return;
    }
  }

  void For::execute(Interpreter & interpreter) const
  {
  auto iteration = [&](const Value & v)
    {
    interpreter.setVariable(name, v);
    try { body->execute(interpreter); }
    catch (const BreakSignal &) { return false; }
    catch (const ContinueSignal &) { }
    return !interpreter.exitRequested();
    };
  // a range is not made into a list
  if (const Range * r = dynamic_cast<const Range *>(values.get()))
    {
    long a, b;
    rangeEnds(interpreter, *r, a, b);
    for (long k=a; k<=b; k++) if (!iteration(Value(k))) return;
    return;
    }
  Value list = values->evaluate(interpreter);
  if (list.kind() == Value::kList) { for (const Value & v : list.toList()) if (!iteration(v)) return; return; }
  if (list.kind() == Value::kText) { for (char c : list.toText()) if (!iteration(Value(std::string(1, c)))) return; return; }
  throw InterpreterException(String("Cannot loop over ") + String(list.typeName()), line);
  }

  void FunctionDefinition::execute(Interpreter & interpreter) const { interpreter.defineFunction(*this); }

  void Return::execute(Interpreter & interpreter) const { throw ReturnSignal{ value ? value->evaluate(interpreter) : Value() }; }
  void Break::execute(Interpreter &) const { throw BreakSignal(); }
  void Continue::execute(Interpreter &) const { throw ContinueSignal(); }
  void Help::execute(Interpreter & interpreter) const { interpreter.out() << interpreter.help(topic); }

  } // namespace INTERPRETER
} // namespace CAP
