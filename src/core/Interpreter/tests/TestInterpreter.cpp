/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestInterpreter.cpp
//! @brief Tests of the CAP command language: values and operators, control flow, functions, lists, objects of the
//! registered classes, built-in functions, CAP files and figures, error messages with their lines.
#include <cmath>
#include <cstdio>
#include <sstream>
#include <string>
#include <unistd.h>
#include "Interpreter.hpp"
#include "Directory.hpp"

using namespace CAP;
using namespace CAP::INTERPRETER;

namespace
{
int nChecks = 0, nFailures = 0;

void check(bool ok, const std::string & what, int line)
{
  nChecks++;
  if (ok) return;
  nFailures++;
  std::printf("FAILED (line %d): %s\n", line, what.c_str());
}

std::ostringstream output;
Interpreter interpreter(output);

//! Value of a program (none if it fails, with the message printed).
Value run(const std::string & source)
{
  try { return interpreter.run(source); }
  catch (const std::exception & e) { std::printf("  error: %s\n", e.what()); return Value(); }
}

//! Line of the error of a program (-1 if it runs), and its message.
int errorLine(const std::string & source, std::string * message=nullptr)
{
  try { interpreter.run(source); }
  catch (const InterpreterException & e) { if (message) *message = e.what(); return e.line(); }
  return -1;
}
} // namespace

#define CHECK(condition) check((condition), #condition, __LINE__)

int main()
{
  // ---- values and operators
  CHECK(run("1 + 2*3").toInteger() == 7);
  CHECK(run("7/2").toReal() == 3.5 && run("7 % 3").toInteger() == 1 && run("-7 % 3").toInteger() == 2);
  CHECK(run("2^10").toInteger() == 1024 && run("2**-1").toReal() == 0.5 && run("-2^2").toInteger() == -4);
  CHECK(std::fabs(run("sqrt(2)^2").toReal() - 2.0) < 1e-12 && std::fabs(run("cos(pi)").toReal() + 1.0) < 1e-12);
  CHECK(run("\"a\" + 1 + \"b\"").toText() == "a1b");
  CHECK(run("3 > 2 and not (1 == 2)").toBool() && run("none == none").toBool() && !run("[1, 2] == [1, 3]").toBool());
  CHECK(run("1.5e3").toReal() == 1500.0 && run("type(1)").toText() == "integer" && run("type(1.0)").toText() == "real");

  // ---- variables, lists, ranges
  run("x = 10; x += 5; x *= 2");
  CHECK(run("x").toInteger() == 30);
  run("l = [1, 2, 3]\nl.append(4)\nl[0] = 10");
  CHECK(run("l").toList().size() == 4 && run("l[0] + l[-1]").toInteger() == 14 && run("len(l)").toInteger() == 4);
  CHECK(run("sum(1..100)").toReal() == 5050.0 && run("len(range(2, 10, 3))").toInteger() == 3);
  CHECK(run("max([3, 9, 2])").toReal() == 9.0 && run("\"hello\"[1]").toText() == "e");

  // ---- control flow
  run("s = 0\nfor k in 1..1000000 { s += k }");                   // a range is not made into a list
  CHECK(run("s").toInteger() == 500000500000L);
  run("n = 0\nwhile true {\n  n += 1\n  if n % 2 == 0 { continue }\n  if n > 9 { break }\n}");
  CHECK(run("n").toInteger() == 11);
  run("t = \"\"\nfor w in [\"a\", \"b\"] { t = t + w }\nif t == \"ab\" { r = 1 }\nelse { r = 2 }");
  CHECK(run("r").toInteger() == 1);
  run("if false { q = 1 } else if true { q = 2 } else { q = 3 }");
  CHECK(run("q").toInteger() == 2);

  // ---- functions (recursion, locals, globals)
  run("fun fib(n) {\n  if n < 2 { return n }\n  return fib(n - 1) + fib(n - 2)\n}");
  CHECK(run("fib(20)").toInteger() == 6765);
  run("counter = 0\nfun bump(by) { local = by; counter += by }\nbump(3); bump(4)");
  CHECK(run("counter").toInteger() == 7 && !interpreter.hasVariable("local"));
  CHECK(errorLine("fun f(a) { return a }\nf(1, 2)") == 2);

  // ---- objects of the registered classes
  run("g = MersenneTwister(12345)\nh = Histogram(\"h\", \"Gaussian numbers\", 40, -4, 4)\n"
      "for k in 1..20000 { h.fill(g.gauss(0.5, 1)) }");
  CHECK(std::fabs(run("h.mean(0)").toReal() - 0.5) < 0.03);
  CHECK(run("h.name").toText() == "h" && run("h.nBins(0)").toInteger() == 40);
  run("h.name = \"gauss\"");
  CHECK(run("h.name").toText() == "gauss" && run("type(h)").toText() == "CAP::MATH::Histogram");
  CHECK(run("len(classes(\"CAP::Object\"))").toInteger() >= 3);
  output.str("");
  run("help Histogram");
  CHECK(output.str().find("fill void (double, double)") != std::string::npos);
  output.str("");
  run("help");
  CHECK(output.str().find("Built-in functions") != std::string::npos && output.str().find("save(file") != std::string::npos);
  output.str("");
  run("print(\"mean\", 1, [1, \"a\"])");
  CHECK(output.str() == "mean 1 [1, \"a\"]\n");

  // ---- CAP files and figures
  char buffer[] = "/tmp/CapTestInterpreter.XXXXXX";
  Directory top{String(mkdtemp(buffer))};
  std::string dir = top.path().str();
  run("write(\"" + dir + "/out.cap\", h)");
  CHECK(run("ls(\"" + dir + "/out.cap\")").toList().size() == 1);
  run("back = read(\"" + dir + "/out.cap\", \"gauss\")");
  CHECK(run("type(back)").toText() == "CAP::MATH::Histogram" && run("back.integral() == h.integral()").toBool());
  run("save(\"" + dir + "/gauss.pdf\", h)\nsave(\"" + dir + "/gauss.svg\", h, back)");
  CHECK(top.contains("gauss.pdf") && top.contains("gauss.svg"));
  // a file of commands
  {
    FILE * f = std::fopen((dir + "/script.caps").c_str(), "w");
    std::fprintf(f, "# a script\nfun cube(x) { return x^3 }\nscripted = cube(3)\n");
    std::fclose(f);
  }
  run("run(\"" + dir + "/script.caps\")");
  CHECK(run("scripted").toInteger() == 27);
  top.remove(true);

  // ---- errors: message and line
  std::string message;
  CHECK(errorLine("a = 1\nb = undefinedName + 1", &message) == 2 && message.find("undefinedName") != std::string::npos);
  CHECK(errorLine("h.fill(\"x\")", &message) == 1 && message.find("No function fill accepts") != std::string::npos);
  CHECK(errorLine("x = (1 + 2", &message) == 1);
  CHECK(errorLine("h.missing()", &message) == 1 && message.find("missing") != std::string::npos);
  CHECK(errorLine("1 / 0", &message) == 1 && message.find("Division by zero") != std::string::npos);
  CHECK(errorLine("Nothing(1)", &message) == 1 && message.find("Unknown function or class") != std::string::npos);
  CHECK(errorLine("break") >= 0);
  CHECK(isComplete("for k in 1..3 {") == false && isComplete("for k in 1..3 { print(k) }") && !isComplete("print(\"open"));
  CHECK(run("exit()").isNone() && interpreter.exitRequested());

  std::printf("TestInterpreter: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
