/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestReflection.cpp
//! @brief Tests of the run-time reflection: classes found by name, objects made by their constructors, member
//! functions called (overloads, bases, objects as arguments and results), data members and properties.
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include "Reflection.hpp"

using namespace CAP;
using namespace CAP::REFLECTION;

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

bool throws(const std::function<void()> & f) { try { f(); } catch (const ReflectionException &) { return true; } return false; }

// ---- classes described below
class Named
{
public:
  Named() : _name("unnamed") { }
  virtual ~Named() { }
  const String & name() const { return _name; }
  void setName(const String & name) { _name = name; }
  String greeting(const std::string & who) const { return String("hello ") + String(who) + String(" from ") + _name; }
protected:
  String _name;
};

class Point
{
public:
  Point() : x(0), y(0) { }
  Point(double x0, double y0) : x(x0), y(y0) { }
  double norm() const { return std::sqrt(x*x + y*y); }
  double x;
  double y;
};

enum Color { kRed = 2, kBlue = 4 };

class Circle : public Named
{
public:
  Circle() : count(0), _center(), _radius(1), _color(kRed) { }
  Circle(const Point & center, double radius) : count(0), _center(center), _radius(radius), _color(kRed) { }
  double area() const { return M_PI*_radius*_radius; }
  void scale(double f) { _radius *= f; count++; }
  void scale(double fx, double fy) { _radius *= std::sqrt(fx*fy); count++; }   // overload
  Point center() const { return _center; }                                   // an object, by value
  Point & centerRef() { return _center; }                                    // an object, by reference
  void moveTo(const Point & p) { _center = p; }
  bool contains(const Point * p) const { return p && std::hypot(p->x - _center.x, p->y - _center.y) <= _radius; }
  double radius() const { return _radius; }
  void setRadius(double r) { _radius = r; }
  Color color() const { return _color; }
  void setColor(Color c) { _color = c; }
  int count;
private:
  Point _center;
  double _radius;
  Color _color;
};

double perimeter(const Circle & c) { return 2*M_PI*c.radius(); }
} // namespace

CAP_REFLECTION(TestClasses)
{
  ClassBuilder<Named>("Test::Named")
    .constructor<>()
    .property("name", &Named::name, &Named::setName)
    .method("greeting", &Named::greeting);
  ClassBuilder<Point>("Test::Point")
    .constructor<>()
    .constructor<double, double>()
    .field("x", &Point::x)
    .field("y", &Point::y)
    .method("norm", &Point::norm);
  ClassBuilder<Circle>("Test::Circle")
    .base<Named>()
    .constructor<>()
    .constructor<const Point &, double>()
    .method("area", &Circle::area)
    .method("scale", static_cast<void (Circle::*)(double)>(&Circle::scale))
    .method("scale", static_cast<void (Circle::*)(double, double)>(&Circle::scale))
    .method("center", &Circle::center)
    .method("centerRef", &Circle::centerRef)
    .method("moveTo", &Circle::moveTo)
    .method("contains", &Circle::contains)
    .function("perimeter", &perimeter)
    .property("radius", &Circle::radius, &Circle::setRadius)
    .property("color", &Circle::color, &Circle::setColor)
    .property("area", &Circle::area)
    .field("count", &Circle::count);
}

#define CHECK(condition) check((condition), #condition, __LINE__)

int main()
{
  Registry & registry = Registry::instance();

  // ---- classes by name and by type
  const ClassInfo * circleClass = registry.find("Test::Circle");
  CHECK(circleClass && registry.find("Circle") == circleClass && classOf<Circle>() == circleClass);
  CHECK(registry.find("Nothing") == nullptr);
  CHECK(circleClass->inheritsFrom(*registry.find("Test::Named")) && !registry.find("Test::Named")->inheritsFrom(*circleClass));
  CHECK(registry.names(registry.find("Test::Named")).size() == 2);   // Named and Circle

  // ---- objects: constructors, functions, overloads
  Value p = registry.find("Test::Point")->create({ 3.0, 4 });       // an integer for a double
  CHECK(p.call("norm").toReal() == 5.0);
  Value c = circleClass->create({ p, 2.0 });
  CHECK(std::fabs(c.call("area").toReal() - 4*M_PI) < 1e-12);
  c.call("scale", { 2.0 });
  c.call("scale", { 2.0, 8.0 });                                     // the other overload
  CHECK(std::fabs(c.get("radius").toReal() - 16.0) < 1e-12 && c.get("count").toInteger() == 2);
  CHECK(std::fabs(c.call("perimeter").toReal() - 32*M_PI) < 1e-12);
  // objects as results: a copy (by value), the object itself (by reference)
  Value center = c.call("center");
  CHECK(center.typeName() == "Test::Point" && center.get("x").toReal() == 3.0 && center.toInstance().owner != nullptr);
  c.call("centerRef").set("x", 10.0);
  CHECK(c.call("center").get("x").toReal() == 10.0);
  // objects as arguments: by reference, by pointer
  c.call("moveTo", { registry.find("Test::Point")->create({ 1.0, 1.0 }) });
  CHECK(c.call("contains", { registry.find("Test::Point")->create({ 2.0, 2.0 }) }).toBool());
  CHECK(!c.call("contains", { Value() }).toBool());                  // none: a null pointer

  // ---- members of the base, through the derived class
  CHECK(c.get("name").toText() == "unnamed");
  c.set("name", "wheel");
  CHECK(c.call("greeting", { "you" }).toText() == "hello you from wheel");
  // enums as integers
  c.set("color", 4);
  CHECK(c.get("color").toInteger() == 4);

  // ---- errors
  CHECK(throws([&]() { c.call("missing"); }));
  CHECK(throws([&]() { c.call("scale", { "big" }); }));              // no overload takes a text
  CHECK(throws([&]() { c.call("scale", { 1.0, 2.0, 3.0 }); }));
  CHECK(throws([&]() { c.set("area", 1.0); }));                      // read only
  CHECK(throws([&]() { c.call("moveTo", { c }); }));                 // a circle is not a point
  CHECK(throws([&]() { circleClass->create({ "a", "b", "c" }); }));
  CHECK(throws([&]() { Value(2.5).toInteger(); }));
  CHECK(Value(3.0).toInteger() == 3 && Value(true).toReal() == 1.0);

  // ---- description
  std::string d = circleClass->describe();
  CHECK(d.find("class Test::Circle : Test::Named") != std::string::npos);
  CHECK(d.find("scale void (double, double)") != std::string::npos && d.find("greeting") != std::string::npos);
  CHECK(d.find("radius") != std::string::npos && d.find("(read only)") != std::string::npos);

  std::printf("TestReflection: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
