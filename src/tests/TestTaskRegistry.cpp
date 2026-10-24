/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestTaskRegistry.cpp
//! @brief Tests of the task classes in the reflection registry: the same classes as ROOT's dictionaries list, each
//! made by the registry with its own class, and Task::taskClassNames() listing them.
#include <algorithm>
#include <cstdio>
#include <set>
#include <string>
#include "TClass.h"
#include "TClassTable.h"
#include "Reflection.hpp"
#include "Task.hpp"

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

//! The task classes ROOT's dictionaries know (what Task::taskClassNames() returned before the registry).
std::set<std::string> rootTaskClasses()
{
  std::set<std::string> names;
  TClass * taskClass = TClass::GetClass("CAP::Task");
  TClassTable::Init();
  while (const char * className = TClassTable::Next())
    {
    TClass * c = TClass::GetClass(className);
    if (!c || !c->InheritsFrom(taskClass) || !c->GetNew() || (c->Property() & kIsAbstract)) continue;
    if (std::string(className).find('<') != std::string::npos) continue;
    names.insert(className);
    }
  return names;
}

//! A task of a class, made as a workflow makes it (Task::instantiateTask() is protected).
struct Factory : public Task
{
  static Task * make(const String & name) { return instantiateTask(name); }
};
} // namespace

#define CHECK(condition) check((condition), #condition, __LINE__)

int main()
{
  Registry & registry = Registry::instance();
  const ClassInfo * taskInfo = registry.find(typeid(Task));
  CHECK(taskInfo != nullptr);
  if (!taskInfo) return 1;
  std::set<std::string> registered;
  for (const auto & name : registry.names(taskInfo)) registered.insert(name);
  std::set<std::string> root = rootTaskClasses();
  for (const auto & name : root)
    if (!registered.count(name)) { std::printf("  not registered: %s\n", name.c_str()); check(false, "registered " + name, __LINE__); }
  for (const auto & name : registered)
    if (!root.count(name)) std::printf("  registered, unknown to ROOT: %s\n", name.c_str());
  CHECK(registered.size() >= 30);

  // each task made by the registry is of its class
  int made = 0;
  for (const auto & name : registered)
    {
    Task * task = Factory::make(String(name));
    const ClassInfo * dynamic = task ? registry.find(typeid(*task)) : nullptr;
    if (!dynamic || dynamic->name() != name) { std::printf("  %s made a %s\n", name.c_str(), dynamic ? dynamic->name().c_str() : "?"); check(false, "class of " + name, __LINE__); }
    else made++;
    delete task;
    }
  CHECK(made == int(registered.size()));

  // taskClassNames(): registered and ROOT classes, once each, sorted
  std::vector<String> names = Task::taskClassNames();
  std::set<std::string> listed;
  for (const auto & n : names) listed.insert(n.str());
  CHECK(listed.size() == names.size());
  CHECK(std::includes(listed.begin(), listed.end(), registered.begin(), registered.end()));
  CHECK(std::is_sorted(names.begin(), names.end(), [](const String & a, const String & b) { return a.str() < b.str(); }));

  std::printf("TestTaskRegistry: %d checks, %d failed (%d task classes)\n", nChecks, nFailures, int(registered.size()));
  return nFailures;
}
