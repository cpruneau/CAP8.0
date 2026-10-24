/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Builtins.cpp
//! @brief Built-in functions of the CAP command language: output, mathematics, lists, classes, CAP files, figures.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <sstream>
#include "Interpreter.hpp"
#include "CapFile.hpp"
#include "FileSystem.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#ifdef CAP_INTERPRETER_GRAPHICS
#include "Canvas.hpp"
#include "PdfPainter.hpp"
#include "SvgPainter.hpp"
#endif

namespace CAP
{
  namespace INTERPRETER
  {

  using namespace REFLECTION;

  namespace
  {
  //! Check the number of arguments of a built-in function.
  void count(const char * name, const std::vector<Value> & a, std::size_t low, std::size_t high, int line)
  {
  if (a.size() >= low && a.size() <= high) return;
  std::string expected = low == high ? std::to_string(low) : std::to_string(low) + " to " + std::to_string(high);
  throw InterpreterException(String(name) + String(" takes ") + String(expected) + String(" arguments, not ")
                             + String(std::to_string(a.size())), line);
  }

  std::string raw(const Value & v) { return v.kind() == Value::kText ? v.toText() : v.print(); }

  //! The numbers of the arguments, or of a single list argument.
  std::vector<double> numbers(const std::vector<Value> & a)
  {
  std::vector<double> x;
  const std::vector<Value> & list = (a.size() == 1 && a[0].kind() == Value::kList) ? a[0].toList() : a;
  for (const Value & v : list) x.push_back(v.toReal());
  return x;
  }

  //! A function of one real.
  void math(Interpreter & interpreter, const char * name, const char * description, double (*f)(double))
  {
  interpreter.addBuiltin(name, std::string(name) + "(x)", description,
                         [name, f](Interpreter &, const std::vector<Value> & a, int line) { count(name, a, 1, 1, line); return Value(f(a[0].toReal())); });
  }

  //! Address of an object of class T (or deriving from it), or null.
  template <typename T> T * as(const Value & v)
  {
  if (v.kind() != Value::kInstance || !classOf<T>()) return nullptr;
  const Instance & o = v.toInstance();
  return o.type ? static_cast<T *>(o.type->upcast(o.pointer, *classOf<T>())) : nullptr;
  }
  } // namespace

  void addStandardBuiltins(Interpreter & interpreter)
  {
  interpreter.setVariable("pi", Value(M_PI));
  interpreter.setVariable("e", Value(M_E));

  // ---- output, session
  interpreter.addBuiltin("print", "print(a, b, ...)", "Write the values, separated by spaces (texts without their quotes).",
    [](Interpreter & in, const std::vector<Value> & a, int)
    {
    for (std::size_t k=0; k<a.size(); k++) in.out() << (k ? " " : "") << raw(a[k]);
    in.out() << "\n";
    return Value();
    });
  interpreter.addBuiltin("help", "help(x)", "Help on a function or class (by name), or on the class of an object; help alone: the language.",
    [](Interpreter & in, const std::vector<Value> & a, int line)
    {
    count("help", a, 0, 1, line);
    if (a.empty()) in.out() << in.help("");
    else if (a[0].kind() == Value::kInstance) in.out() << a[0].toInstance().type->describe();
    else in.out() << in.help(a[0].toText());
    return Value();
    });
  interpreter.addBuiltin("type", "type(x)", "Name of the type of a value: integer, real, text, list, none, boolean, or the class of an object.",
    [](Interpreter &, const std::vector<Value> & a, int line) { count("type", a, 1, 1, line); return Value(a[0].typeName()); });
  interpreter.addBuiltin("classes", "classes(base)", "Names of the classes that can be used (deriving from base, if given).",
    [](Interpreter &, const std::vector<Value> & a, int line)
    {
    count("classes", a, 0, 1, line);
    const ClassInfo * base = nullptr;
    if (a.size() == 1 && !(base = Registry::instance().find(a[0].toText())))
      throw InterpreterException(String("Unknown class ") + String(a[0].toText()), line);
    std::vector<Value> names;
    for (const auto & n : Registry::instance().names(base)) names.push_back(Value(n));
    return Value(names);
    });
  interpreter.addBuiltin("variables", "variables()", "Names of the variables of the session.",
    [](Interpreter & in, const std::vector<Value> & a, int line)
    {
    count("variables", a, 0, 0, line);
    std::vector<Value> names;
    for (const auto & n : in.variableNames()) names.push_back(Value(n));
    return Value(names);
    });
  interpreter.addBuiltin("run", "run(file)", "Run a file of commands.",
    [](Interpreter & in, const std::vector<Value> & a, int line) { count("run", a, 1, 1, line); return in.runFile(FileSystem::expand(a[0].toText()).str()); });
  interpreter.addBuiltin("exit", "exit()", "Leave the session (also quit()).",
    [](Interpreter & in, const std::vector<Value> &, int) { in.requestExit(); return Value(); });
  interpreter.addBuiltin("quit", "quit()", "Leave the session (also exit()).",
    [](Interpreter & in, const std::vector<Value> &, int) { in.requestExit(); return Value(); });
  interpreter.addBuiltin("clock", "clock()", "Seconds since the start of the session (to time commands).",
    [](Interpreter &, const std::vector<Value> &, int)
    {
    static const auto start = std::chrono::steady_clock::now();
    return Value(std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count());
    });

  // ---- conversions, lists
  interpreter.addBuiltin("int", "int(x)", "Integer of a number (rounded toward zero) or of a text.",
    [](Interpreter &, const std::vector<Value> & a, int line)
    {
    count("int", a, 1, 1, line);
    if (a[0].kind() == Value::kText) return Value(std::stol(a[0].toText()));
    return Value(long(std::trunc(a[0].toReal())));
    });
  interpreter.addBuiltin("real", "real(x)", "Real of a number or of a text.",
    [](Interpreter &, const std::vector<Value> & a, int line)
    { count("real", a, 1, 1, line); return a[0].kind() == Value::kText ? Value(std::stod(a[0].toText())) : Value(a[0].toReal()); });
  interpreter.addBuiltin("text", "text(x)", "Text of a value (as print() writes it).",
    [](Interpreter &, const std::vector<Value> & a, int line) { count("text", a, 1, 1, line); return Value(raw(a[0])); });
  interpreter.addBuiltin("len", "len(x)", "Number of elements of a list, or of characters of a text.",
    [](Interpreter &, const std::vector<Value> & a, int line)
    {
    count("len", a, 1, 1, line);
    return Value(long(a[0].kind() == Value::kText ? a[0].toText().size() : a[0].toList().size()));
    });
  interpreter.addBuiltin("range", "range(n), range(first, end, step)", "List of the integers from 0 (or first) up to n (or end) excluded.",
    [](Interpreter &, const std::vector<Value> & a, int line)
    {
    count("range", a, 1, 3, line);
    long first = a.size() > 1 ? a[0].toInteger() : 0, end = a.size() > 1 ? a[1].toInteger() : a[0].toInteger();
    long step = a.size() > 2 ? a[2].toInteger() : 1;
    if (step == 0) throw InterpreterException("range: the step is 0", line);
    std::vector<Value> list;
    for (long k=first; step > 0 ? k < end : k > end; k += step) list.push_back(Value(k));
    return Value(list);
    });
  interpreter.addBuiltin("sum", "sum(list), sum(a, b, ...)", "Sum of the numbers.",
    [](Interpreter &, const std::vector<Value> & a, int)
    { double s = 0; for (double x : numbers(a)) s += x; return Value(s); });
  interpreter.addBuiltin("min", "min(list), min(a, b, ...)", "Smallest of the numbers.",
    [](Interpreter &, const std::vector<Value> & a, int line)
    {
    std::vector<double> x = numbers(a);
    if (x.empty()) throw InterpreterException("min of nothing", line);
    return Value(*std::min_element(x.begin(), x.end()));
    });
  interpreter.addBuiltin("max", "max(list), max(a, b, ...)", "Largest of the numbers.",
    [](Interpreter &, const std::vector<Value> & a, int line)
    {
    std::vector<double> x = numbers(a);
    if (x.empty()) throw InterpreterException("max of nothing", line);
    return Value(*std::max_element(x.begin(), x.end()));
    });

  // ---- mathematics
  math(interpreter, "sqrt", "Square root.", [](double x) { return std::sqrt(x); });
  math(interpreter, "exp", "Exponential.", [](double x) { return std::exp(x); });
  math(interpreter, "log", "Natural logarithm.", [](double x) { return std::log(x); });
  math(interpreter, "log10", "Logarithm in base 10.", [](double x) { return std::log10(x); });
  math(interpreter, "sin", "Sine (radians).", [](double x) { return std::sin(x); });
  math(interpreter, "cos", "Cosine (radians).", [](double x) { return std::cos(x); });
  math(interpreter, "tan", "Tangent (radians).", [](double x) { return std::tan(x); });
  math(interpreter, "asin", "Arc sine.", [](double x) { return std::asin(x); });
  math(interpreter, "acos", "Arc cosine.", [](double x) { return std::acos(x); });
  math(interpreter, "atan", "Arc tangent.", [](double x) { return std::atan(x); });
  math(interpreter, "floor", "Largest whole number not above x.", [](double x) { return std::floor(x); });
  math(interpreter, "ceil", "Smallest whole number not below x.", [](double x) { return std::ceil(x); });
  math(interpreter, "round", "Nearest whole number.", [](double x) { return std::round(x); });
  interpreter.addBuiltin("abs", "abs(x)", "Absolute value (an integer for an integer).",
    [](Interpreter &, const std::vector<Value> & a, int line)
    { count("abs", a, 1, 1, line); return a[0].kind() == Value::kInteger ? Value(std::labs(a[0].toInteger())) : Value(std::fabs(a[0].toReal())); });
  interpreter.addBuiltin("atan2", "atan2(y, x)", "Angle of the point (x, y), in (-pi, pi].",
    [](Interpreter &, const std::vector<Value> & a, int line) { count("atan2", a, 2, 2, line); return Value(std::atan2(a[0].toReal(), a[1].toReal())); });
  interpreter.addBuiltin("pow", "pow(x, y)", "x to the power y.",
    [](Interpreter &, const std::vector<Value> & a, int line) { count("pow", a, 2, 2, line); return Value(std::pow(a[0].toReal(), a[1].toReal())); });

  // ---- CAP files
  interpreter.addBuiltin("ls", "ls(file)", "Paths of the objects of a CAP file.",
    [](Interpreter &, const std::vector<Value> & a, int line)
    {
    count("ls", a, 1, 1, line);
    CapFile file(FileSystem::expand(a[0].toText()), File::kReadOnly);
    file.load(File::Loading::kLazy);
    std::vector<Value> paths;
    for (const auto & p : file.folder().paths()) paths.push_back(Value(p.str()));
    return Value(paths);
    });
  interpreter.addBuiltin("read", "read(file, path)", "Object of a CAP file (see ls()), e.g. h = read(\"run.cap\", \"singles/pt\").",
    [](Interpreter &, const std::vector<Value> & a, int line)
    {
    count("read", a, 2, 2, line);
    String path = FileSystem::expand(a[0].toText());
    CapFile file(path, File::kReadOnly);
    file.read();
    Object * object = file.folder().release<Object>(String(a[1].toText()));
    if (!object)
      {
      if (file.folder().contains(String(a[1].toText())))
        throw InterpreterException(String(a[1].toText()) + String(" is a ") + String(file.folder().typeName(String(a[1].toText()))) + String(", not an object that can be used"), line);
      throw InterpreterException(String("No object ") + String(a[1].toText()) + String(" in ") + path, line);
      }
    return owned<Object>(object);
    });
  interpreter.addBuiltin("write", "write(file, a, b, ...)", "Write histograms and profiles to a new CAP file, under their names.",
    [](Interpreter &, const std::vector<Value> & a, int line)
    {
    if (a.size() < 2) throw InterpreterException("write takes a file and objects", line);
    CapFile file(FileSystem::expand(a[0].toText()), File::kWriteOnly);
    for (std::size_t k=1; k<a.size(); k++)
      {
      if (MATH::Profile * p = as<MATH::Profile>(a[k])) file.add(new MATH::Profile(*p), p->name());
      else if (MATH::Histogram * h = as<MATH::Histogram>(a[k])) file.add(new MATH::Histogram(*h), h->name());
      else throw InterpreterException(String("write: cannot write ") + String(a[k].typeName()), line);
      }
    file.write();
    return Value();
    });

#ifdef CAP_INTERPRETER_GRAPHICS
  // ---- figures
  interpreter.addBuiltin("save", "save(file, a, b, ...)",
    "Draw histograms and data points in one graph, and write it to an SVG or PDF file (by the extension), 800 x 600 points.",
    [](Interpreter &, const std::vector<Value> & a, int line)
    {
    if (a.size() < 2) throw InterpreterException("save takes a file and objects", line);
    String path = FileSystem::expand(a[0].toText());
    GRAPHICS::Canvas canvas("c", "c", 800, 600);
    canvas.divide(1, 1);
    GRAPHICS::Graph & graph = canvas.graph(0);
    for (std::size_t k=1; k<a.size(); k++)
      {
      MATH::DataPoints * data = as<MATH::DataPoints>(a[k]);
      if (!data) throw InterpreterException(String("save: cannot draw ") + String(a[k].typeName()), line);
      graph.add(GRAPHICS::GraphItem(data, data->name()));
      if (k == 1) graph.setTitlesFrom(graph.items().back());
      }
    bool ok;
    if (FileSystem::hasExtension(path, ".pdf")) { GRAPHICS::PdfPainter painter(800, 600); canvas.draw(painter); ok = painter.write(path); }
    else { GRAPHICS::SvgPainter painter(800, 600); canvas.draw(painter); ok = painter.write(path); }
    if (!ok) throw InterpreterException(String("Cannot write ") + path, line);
    return Value();
    });
#endif
  }

  } // namespace INTERPRETER
} // namespace CAP
