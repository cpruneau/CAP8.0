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
//! @brief Tests of the task classes in the reflection registry: all registered, each made by the registry with its
//! own class, and Task::taskClassNames() listing them.
#include <algorithm>
#include <cstdio>
#include <set>
#include <string>
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
  CHECK(registered.size() >= 34);                                   // the task classes of the default build
  for (const char * name : { "CAP::Task", "CAP::BasicEventGen", "CAP::EventIterator", "CAP::FilterCreator",
                             "CAP::ParticleSingleAnalyzer", "CAP::GlauberGenerator", "CAP::PtPtAnalyzer" })
    CHECK(registered.count(name) == 1);

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
