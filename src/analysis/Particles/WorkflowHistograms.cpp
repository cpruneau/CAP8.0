/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <cstdio>
#include <exception>
#include <fstream>
#if defined(__unix__) || defined(__APPLE__)
#define CAP_WORKFLOW_FORK
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#endif
#include <functional>
#include "WorkflowHistograms.hpp"
#include "AnalysisTask.hpp"
#include "CapFile.hpp"
#include "Exceptions.hpp"
#include "Task.hpp"

namespace CAP
{

namespace
{
//! Make one stage in this process: its main task and subtasks configured, then their histograms made empty in top.
//! @param element TASK element @param stage stage completed (outputFile, problems) @param top folder of the histograms
void makeStage(const XmlElement & element, WorkflowHistograms::Stage & stage, Folder & top)
{
  Task creator;
  Task * main = nullptr;
  try
    {
    main = creator.createTask(nullptr, element);
    stage.outputFile = main->configuration().valueString(createKey(main->name(),"JOB:OUTPUT_FILE"), true,
                                                          main->name() + String(".cap"));
    std::function<void(Task &)> visit = [&](Task & task)
      {
      if (AnalysisTask * analysis = dynamic_cast<AnalysisTask *>(&task))
        {
        try { analysis->makeEmptyHistograms(top); }
        catch (const std::exception & e) { stage.problems += task.name() + String(": ") + String(e.what()) + String("\n"); }
        }
      for (Task * subtask : task.subTasks()) visit(*subtask);
      };
    visit(*main);
    }
  catch (const std::exception & e) { stage.problems += stage.task + String(": ") + String(e.what()) + String("\n"); }
  delete main;
}

#ifdef CAP_WORKFLOW_FORK
//! Make one stage in a child process (the stores of the managed objects, filters and histograms included, are those
//! of one job, as when RunCAP.py runs it; its messages are not shown): the histograms come back in a CAP file, the
//! output file and the problems in a text file. @param element TASK element @param stage stage completed
//! @return false if the child could not be run (the stage is then to be made here)
bool makeStageApart(const XmlElement & element, WorkflowHistograms::Stage & stage)
{
  char capName[] = "/tmp/cap_workflow_XXXXXX";
  int capFd = ::mkstemp(capName);
  if (capFd < 0) return false;
  ::close(capFd);
  std::string textName = std::string(capName) + ".txt";
  std::fflush(stdout);
  std::fflush(stderr);
  pid_t pid = ::fork();
  if (pid < 0) { ::unlink(capName); return false; }
  if (pid == 0)
    {
    int null = ::open("/dev/null", O_WRONLY);
    if (null >= 0) { ::dup2(null, 1); ::dup2(null, 2); }
    int status = 0;
    try
      {
      CapFile out{String(capName)};
      out.folder().setName(stage.task);
      makeStage(element, stage, out.folder());
      out.write(true);
      std::ofstream text(textName);
      text << stage.outputFile.str() << "\n" << stage.problems.str();
      }
    catch (...) { status = 1; }
    ::_exit(status);
    }
  int status = 0;
  ::waitpid(pid, &status, 0);
  bool ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;
  if (ok)
    {
    try
      {
      CapFile in(String(capName), File::kReadOnly);
      in.load(File::Loading::kEager);
      *stage.folder = std::move(in.folder());
      std::ifstream text(textName);
      std::string line, problems;
      std::getline(text, line);
      stage.outputFile = String(line);
      while (std::getline(text, line)) problems += line + "\n";
      stage.problems = String(problems);
      }
    catch (const std::exception & e) { stage.problems += stage.task + String(": ") + String(e.what()) + String("\n"); }
    }
  else stage.problems += stage.task + String(": the stage could not be made (its tasks failed)\n");
  ::unlink(capName);
  ::unlink(textName.c_str());
  return true;
}
#endif
} // namespace

std::vector<WorkflowHistograms::Stage> WorkflowHistograms::make(const XmlDocument & workflow, String & error)
{
  std::vector<Stage> stages;
  const XmlElement & root = workflow.root();
  std::vector<const XmlElement *> elements = root.children("TASK");
  if (root.isNamed("TASK")) elements.insert(elements.begin(), &root);
  if (elements.empty()) { error = "no TASK element in the workflow"; return stages; }
  for (const XmlElement * element : elements)
    {
    Stage stage;
    stage.task = element->attribute("name", "");
    stage.name = element->attribute("stage", stage.task);
    stage.folder = std::make_shared<Folder>(stage.task);
#ifdef CAP_WORKFLOW_FORK
    if (makeStageApart(*element, stage)) { stages.push_back(stage); continue; }
#endif
    makeStage(*element, stage, *stage.folder);
    stages.push_back(stage);
    }
  return stages;
}

namespace
{
//! The stages run in this process. @return false with error set on failure
bool runStagesHere(const XmlDocument & workflow, const std::vector<String> & stages, const Folder & input,
                   Folder & output, String & error)
{
  const XmlElement & root = workflow.root();
  std::vector<const XmlElement *> elements = root.children("TASK");
  if (root.isNamed("TASK")) elements.insert(elements.begin(), &root);
  Folder current = input;
  for (const String & name : stages)
    {
    const XmlElement * element = nullptr;
    for (const XmlElement * e : elements)
      if (e->attribute("stage", "") == name || e->attribute("name", "") == name) { element = e; break; }
    if (!element) { error = String("no stage ") + name + String(" in the workflow"); return false; }
    Task creator;
    Task * task = nullptr;
    try
      {
      task = creator.createTask(nullptr, *element);
      task->initialize();
      task->loadHistograms(current);
      task->execute();
      task->finalize();
      Folder next(name);
      task->saveHistograms(next);
      current = next;
      }
    catch (const std::exception & e)
      {
      error = name + String(": ") + String(e.what());
      delete task;
      return false;
      }
    delete task;
    }
  output = current;
  return true;
}
} // namespace

bool WorkflowHistograms::runStages(const XmlDocument & workflow, const std::vector<String> & stages, const Folder & input,
                                   Folder & output, String & error, bool verbose)
{
  if (stages.empty()) { error = "no stage to run"; return false; }
#ifdef CAP_WORKFLOW_FORK
  char capName[] = "/tmp/cap_stages_XXXXXX";
  int capFd = ::mkstemp(capName);
  if (capFd >= 0)
    {
    ::close(capFd);
    std::string textName = std::string(capName) + ".txt";
    std::fflush(stdout);
    std::fflush(stderr);
    pid_t pid = ::fork();
    if (pid == 0)
      {
      if (!verbose)
        {
        int null = ::open("/dev/null", O_WRONLY);
        if (null >= 0) { ::dup2(null, 1); ::dup2(null, 2); }
        }
      int status = 0;
      try
        {
        CapFile out{String(capName)};
        String problem;
        if (runStagesHere(workflow, stages, input, out.folder(), problem)) out.write(true);
        else { std::ofstream text(textName); text << problem.str(); status = 2; }
        }
      catch (const std::exception & e) { std::ofstream text(textName); text << e.what(); status = 1; }
      catch (...) { status = 1; }
      std::fflush(stdout);
      ::_exit(status);
      }
    if (pid > 0)
      {
      int status = 0;
      ::waitpid(pid, &status, 0);
      bool ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;
      if (ok)
        {
        try
          {
          CapFile in(String(capName), File::kReadOnly);
          in.load(File::Loading::kEager);
          output = in.folder();
          }
        catch (const std::exception & e) { error = String(e.what()); ok = false; }
        }
      else
        {
        std::ifstream text(textName);
        std::string line, all;
        while (std::getline(text, line)) all += line;
        error = all.empty() ? String("the stages failed (child process)") : String(all);
        }
      ::unlink(capName);
      ::unlink(textName.c_str());
      return ok;
      }
    ::unlink(capName);
    }
#endif
  (void)verbose;
  return runStagesHere(workflow, stages, input, output, error);
}

} // namespace CAP
