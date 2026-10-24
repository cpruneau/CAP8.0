/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <set>
#include <sstream>
#include "Reflection.hpp"

namespace CAP
{
  namespace REFLECTION
  {

  // ------------------------------------------------------------------------------------------------ Value

  bool Value::toBool() const
  {
  switch (kind())
    {
    case kBool:    return std::get<bool>(_value);
    case kInteger: return std::get<long>(_value) != 0;
    case kReal:    return std::get<double>(_value) != 0.0;
    default: throw ReflectionException(String("Not a boolean: ") + String(print()), "Value::toBool");
    }
  }

  long Value::toInteger() const
  {
  switch (kind())
    {
    case kBool:    return std::get<bool>(_value) ? 1 : 0;
    case kInteger: return std::get<long>(_value);
    case kReal:
      {
      double v = std::get<double>(_value);
      if (v == std::floor(v) && std::fabs(v) < 9.2e18) return long(v);
      break;
      }
    default: break;
    }
  throw ReflectionException(String("Not an integer: ") + String(print()), "Value::toInteger");
  }

  double Value::toReal() const
  {
  switch (kind())
    {
    case kBool:    return std::get<bool>(_value) ? 1.0 : 0.0;
    case kInteger: return double(std::get<long>(_value));
    case kReal:    return std::get<double>(_value);
    default: throw ReflectionException(String("Not a number: ") + String(print()), "Value::toReal");
    }
  }

  const std::string & Value::toText() const
  {
  if (kind() != kText) throw ReflectionException(String("Not a text: ") + String(print()), "Value::toText");
  return std::get<std::string>(_value);
  }

  const Instance & Value::toInstance() const
  {
  if (kind() != kInstance) throw ReflectionException(String("Not an object: ") + String(print()), "Value::toInstance");
  return std::get<Instance>(_value);
  }

  const std::vector<Value> & Value::toList() const
  {
  if (kind() != kList) throw ReflectionException(String("Not a list: ") + String(print()), "Value::toList");
  return std::get<std::vector<Value>>(_value);
  }

  std::string Value::print() const
  {
  std::ostringstream out;
  switch (kind())
    {
    case kNone:    return "none";
    case kBool:    return std::get<bool>(_value) ? "true" : "false";
    case kInteger: out << std::get<long>(_value); break;
    case kReal:    out.precision(10); out << std::get<double>(_value); break;
    case kText:    return "\"" + std::get<std::string>(_value) + "\"";
    case kInstance:
      {
      const Instance & o = std::get<Instance>(_value);
      out << "<" << (o.type ? o.type->name() : std::string("?")) << " at " << o.pointer << ">";
      break;
      }
    case kList:
      {
      const std::vector<Value> & list = std::get<std::vector<Value>>(_value);
      out << "[";
      for (std::size_t k=0; k<list.size(); k++) out << (k ? ", " : "") << list[k].print();
      out << "]";
      break;
      }
    }
  return out.str();
  }

  std::string Value::typeName() const
  {
  static const char * names[] = { "none", "boolean", "integer", "real", "text", "object", "list" };
  if (kind() == kInstance && std::get<Instance>(_value).type) return std::get<Instance>(_value).type->name();
  return names[kind()];
  }

  Value Value::call(const std::string & name, const std::vector<Value> & arguments) const
  {
  const Instance & o = toInstance();
  if (!o.pointer) throw ReflectionException("Null object", "Value::call");
  return o.type->call(o.pointer, name, arguments);
  }

  Value Value::get(const std::string & name) const
  {
  const Instance & o = toInstance();
  if (!o.pointer) throw ReflectionException("Null object", "Value::get");
  return o.type->get(o.pointer, name);
  }

  void Value::set(const std::string & name, const Value & value) const
  {
  const Instance & o = toInstance();
  if (!o.pointer) throw ReflectionException("Null object", "Value::set");
  o.type->set(o.pointer, name, value);
  }

  // ------------------------------------------------------------------------------------------------ ClassInfo

  std::vector<const ClassInfo *> ClassInfo::bases() const
  {
  std::vector<const ClassInfo *> list;
  for (const auto & b : _bases) if (const ClassInfo * c = Registry::instance().find(*b.type)) list.push_back(c);
  return list;
  }

  bool ClassInfo::inheritsFrom(const ClassInfo & base) const
  {
  if (this == &base) return true;
  for (const ClassInfo * b : bases()) if (b->inheritsFrom(base)) return true;
  return false;
  }

  void * ClassInfo::upcast(void * pointer, const ClassInfo & base) const
  {
  if (this == &base) return pointer;
  for (const auto & b : _bases)
    if (const ClassInfo * c = Registry::instance().find(*b.type))
      if (void * p = c->upcast(b.upcast(pointer), base)) return p;
  return nullptr;
  }

  const Field * ClassInfo::field(const std::string & name) const
  {
  for (const auto & f : _fields) if (f.name == name) return &f;
  for (const ClassInfo * b : bases()) if (const Field * f = b->field(name)) return f;
  return nullptr;
  }

  std::vector<const Method *> ClassInfo::methods(const std::string & name) const
  {
  std::vector<const Method *> list;
  for (const auto & m : _methods) if (m.name == name) list.push_back(&m);
  for (const ClassInfo * b : bases()) for (const Method * m : b->methods(name)) list.push_back(m);
  return list;
  }

  std::vector<const Method *> ClassInfo::allMethods() const
  {
  std::vector<const Method *> list;
  for (const auto & m : _methods) list.push_back(&m);
  for (const ClassInfo * b : bases()) for (const Method * m : b->allMethods()) list.push_back(m);
  return list;
  }

  std::vector<const Field *> ClassInfo::allFields() const
  {
  std::vector<const Field *> list;
  for (const auto & f : _fields) list.push_back(&f);
  for (const ClassInfo * b : bases()) for (const Field * f : b->allFields()) list.push_back(f);
  return list;
  }

  namespace
  {
  //! Message listing the candidates of a call that failed.
  String noMatch(const std::string & what, const std::vector<const Method *> & candidates, const std::vector<Value> & arguments,
                 const std::string & reason)
  {
  std::string s = "No " + what + " accepts (";
  for (std::size_t k=0; k<arguments.size(); k++) s += (k ? ", " : "") + arguments[k].typeName();
  s += ")";
  if (!reason.empty()) s += ": " + reason;
  if (!candidates.empty()) s += "; candidates:";
  for (const Method * m : candidates) s += "\n  " + m->name + " " + m->signature;
  return String(s);
  }
  } // namespace

  Value ClassInfo::create(const std::vector<Value> & arguments) const
  {
  std::string reason;
  std::vector<const Method *> candidates;
  for (std::size_t k=0; k<_constructors.size(); k++)
    {
    candidates.push_back(&_constructors[k]);
    if (_constructors[k].nArguments != int(arguments.size())) continue;
    try { return Value(_creators[k](arguments)); }
    catch (const ReflectionException & e) { reason = e.what(); }
    }
  throw ReflectionException(noMatch("constructor of " + _name, candidates, arguments, reason), "ClassInfo::create");
  }

  Value ClassInfo::call(void * object, const std::string & name, const std::vector<Value> & arguments) const
  {
  // the functions of this class first, then those of the bases (on the base part of the object)
  std::string reason;
  std::vector<const Method *> candidates;
  for (const auto & m : _methods)
    {
    if (m.name != name) continue;
    candidates.push_back(&m);
    if (m.nArguments != int(arguments.size())) continue;
    try { return m.invoke(object, arguments); }
    catch (const ReflectionException & e) { reason = e.what(); }
    }
  std::size_t own = candidates.size();
  std::string baseError;
  for (const auto & b : _bases)
    if (const ClassInfo * c = Registry::instance().find(*b.type))
      if (!c->methods(name).empty())
        {
        try { return c->call(b.upcast(object), name, arguments); }
        catch (const ReflectionException & e)
          {
          if (baseError.empty()) baseError = e.what();
          for (const Method * m : c->methods(name)) candidates.push_back(m);
          }
        }
  if (candidates.empty()) throw ReflectionException(String("No function ") + String(name) + String(" in ") + String(_name), "ClassInfo::call");
  if (own == 0) throw ReflectionException(String(baseError), "ClassInfo::call");   // the functions of a base only: its message
  throw ReflectionException(noMatch("function " + name, candidates, arguments, reason), "ClassInfo::call");
  }

  Value ClassInfo::get(void * object, const std::string & name) const
  {
  for (const auto & f : _fields) if (f.name == name) return f.get(object);
  for (const auto & b : _bases)
    if (const ClassInfo * c = Registry::instance().find(*b.type))
      if (c->field(name)) return c->get(b.upcast(object), name);
  throw ReflectionException(String("No member ") + String(name) + String(" in ") + String(_name), "ClassInfo::get");
  }

  void ClassInfo::set(void * object, const std::string & name, const Value & value) const
  {
  for (const auto & f : _fields)
    if (f.name == name)
      {
      if (!f.set) throw ReflectionException(String("The member ") + String(name) + String(" is read only"), "ClassInfo::set");
      f.set(object, value);
      return;
      }
  for (const auto & b : _bases)
    if (const ClassInfo * c = Registry::instance().find(*b.type))
      if (c->field(name)) { c->set(b.upcast(object), name, value); return; }
  throw ReflectionException(String("No member ") + String(name) + String(" in ") + String(_name), "ClassInfo::set");
  }

  std::string ClassInfo::describe() const
  {
  std::ostringstream out;
  out << "class " << _name;
  std::vector<const ClassInfo *> b = bases();
  for (std::size_t k=0; k<b.size(); k++) out << (k ? ", " : " : ") << b[k]->name();
  out << "\n";
  for (const auto & c : _constructors) out << "  " << _name.substr(_name.rfind(':') == std::string::npos ? 0 : _name.rfind(':') + 1) << c.signature << "\n";
  for (const Field * f : allFields()) out << "  " << f->typeName << " " << f->name << (f->set ? "" : "   (read only)") << "\n";
  std::set<std::string> seen;
  for (const Method * m : allMethods())
    if (seen.insert(m->name + m->signature).second) out << "  " << m->name << " " << m->signature << "\n";
  return out.str();
  }

  // ------------------------------------------------------------------------------------------------ Registry

  Registry & Registry::instance()
  {
  static Registry registry;
  return registry;
  }

  const ClassInfo * Registry::find(const std::string & name) const
  {
  auto found = _byName.find(name);
  if (found != _byName.end()) return found->second.get();
  // the name without its namespaces, if no other class has it
  const ClassInfo * match = nullptr;
  for (const auto & e : _byName)
    {
    const std::string & full = e.first;
    if (full.size() > name.size() && full.compare(full.size() - name.size(), name.size(), name) == 0
        && full[full.size() - name.size() - 1] == ':')
      {
      if (match) return nullptr;   // ambiguous
      match = e.second.get();
      }
    }
  return match;
  }

  const ClassInfo * Registry::find(const std::type_info & type) const
  {
  auto found = _byType.find(std::type_index(type));
  return found == _byType.end() ? nullptr : found->second;
  }

  std::vector<std::string> Registry::names(const ClassInfo * base) const
  {
  std::vector<std::string> list;
  for (const auto & e : _byName) if (!base || e.second->inheritsFrom(*base)) list.push_back(e.first);
  return list;
  }

  ClassInfo & Registry::add(std::unique_ptr<ClassInfo> info)
  {
  ClassInfo * c = info.get();
  _byType[std::type_index(c->type())] = c;
  _byName[c->name()] = std::move(info);
  return *c;
  }

  void * objectPointer(const Value & value, const ClassInfo * target, const char * typeName)
  {
  if (!target)
    {
    int status = 0;
    char * name = abi::__cxa_demangle(typeName, nullptr, nullptr, &status);
    String readable(status == 0 && name ? name : typeName);
    std::free(name);
    throw ReflectionException(String("Unregistered class ") + readable, "Reflection");
    }
  if (value.kind() != Value::kInstance)
    throw ReflectionException(String("Expected ") + String(target->name()) + String(", not ") + String(value.typeName()), "Reflection");
  const Instance & o = value.toInstance();
  void * p = o.type ? o.type->upcast(o.pointer, *target) : nullptr;
  if (!p) throw ReflectionException(String("Expected ") + String(target->name()) + String(", not ") + String(value.typeName()), "Reflection");
  return p;
  }

  namespace Detail
  {
  std::string typeName(const std::type_info & type)
  {
  int status = 0;
  char * name = abi::__cxa_demangle(type.name(), nullptr, nullptr, &status);
  std::string s = (status == 0 && name) ? name : type.name();
  std::free(name);
  // shorter names for the common types
  static const std::pair<const char *, const char *> shorter[] =
    { { "std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >", "std::string" },
      { "std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >", "std::string" } };
  for (const auto & r : shorter) for (std::size_t at; (at = s.find(r.first)) != std::string::npos;) s.replace(at, std::strlen(r.first), r.second);
  return s;
  }
  } // namespace Detail

  } // namespace REFLECTION
} // namespace CAP
