/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "Task.hpp"
#include "Exceptions.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"
#include "TFile.h"
#include "EnvironmentVariables.hpp"

ClassImp(CAP::Task);

namespace CAP
{


  Task::Task()
  :
  Object(),
  ConfigurationManager(),
  _parentTask(nullptr),
  _subTasks(),
  _taskExecuted("Executed",1),
  _timer()
  {
  setMinimumReportLevel(Object::kInfo);
  setClassName("Task");
  setName("Task");
  setTitle("Task");
  }

  Task::Task(const Task & source)
  :
  Object(source),
  ConfigurationManager(source),
  _parentTask(source._parentTask),
  _subTasks(source._subTasks),
  _taskExecuted(source._taskExecuted),
  _timer(source._timer)
  {     }

  Task & Task::operator=(const Task & rhs)
  {
  if (this!=&rhs)
    {
    Object::operator=(rhs);
    ConfigurationManager::operator=(rhs);
    _taskExecuted = rhs._taskExecuted;
    _parentTask = rhs._parentTask;
    _subTasks   = rhs._subTasks;
    _timer      = rhs._timer;
    }
  return *this;
  }

  Task::~Task()
  {
  clear();
  }

  void Task::setDefaultConfiguration()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  String taskName = name();
  _configuration.addProperty(taskName+":TASK:NAME", taskName);
  _configuration.addProperty(taskName+":TASK:CLASS",className());
  _configuration.addProperty(taskName+":TASK:SEVERITY",String("INFO"));
  }

  void Task::configure(const Configuration & newConfiguration)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  _configuration.addSelectedProperties(newConfiguration,name());
  String severityName = _configuration.valueString(createKey(name(),"TASK:SEVERITY"));
  setMinimumReportLevel(severityName);
  }

  void Task::initialize()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  _timer.start();
  _taskExecuted.initialize("taskExecuted",1);
  if (hasSubTasks()) initializeSubTasks();
  }

  void Task::execute()
  {
  _taskExecuted.increment();
  }

  void Task::postProcess()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  }

  void Task::finalize()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  if (hasSubTasks()) finalizeSubTasks();
  _timer.stop();
  print();
  }

  void Task::partial()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  }

  void Task::reset()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  _taskExecuted.reset();
  }

  void Task::clear()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  _taskExecuted.clear();
  }

  void Task::print() const
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  printLine();
  Object::print();
  _taskExecuted.print();
  printSubTasks();
  _timer.print();
  printLine();
  }


  void Task::configureSubtasks(Task * parentTask,
                               const String & parentTaskName,
                               const Configuration & configuration)
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  int nSubtasks = configuration.valueInt( createKey(parentTaskName,"SUBTASK","N") );
  for (int k=0; k<nSubtasks; k++)
    {
    String subtaskName  = configuration.valueString( createKey(parentTaskName,"SUBTASK",k,"NAME")   );
    createTask(parentTask,subtaskName,configuration);
    }
  }

  void Task::initializeSubTasks()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  for (auto & subtask : _subTasks) subtask->initialize();
  }

  void Task::executeSubTasks()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  for (auto & subtask : _subTasks) subtask->execute();
  }

  void Task::finalizeSubTasks()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  for (auto & subtask : _subTasks) subtask->finalize();
  }

  void Task::partialSubTasks()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  for (auto & subtask : _subTasks) subtask->partial();
  }

  void Task::resetSubTasks()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  for (auto & subtask : _subTasks) subtask->reset();
  }

  void Task::clearSubTasks()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  for (auto & subtask : _subTasks)
    {
    subtask->clear();
    delete subtask;
    }
  _subTasks.clear();
  }

  void Task::printSubTasks() const
  {
  if (reportDebug(__FUNCTION__))
    {
    printCR();
    printValue("nSubTasks",_subTasks.size());
    };
  for (auto & subTask : _subTasks) subTask->print();
  }


  Task * Task::addSubTask(Task * task)
  {
  if (!task)      throw Exception("Given task pointer is null.",__FUNCTION__);
  if (task==this) throw Exception("Given task pointer is self.",__FUNCTION__);
  _subTasks.push_back( task );
  if (task->_parentTask == nullptr)  task->setParentTask(this);
  if (reportDebug(__FUNCTION__))
    {
    printCR();
    printValue("Added task",task->name());
    printValue("To task",name());
    }
  return task;
  }

  Task * Task::subTaskAt(unsigned int index)
  {
  if (index>=_subTasks.size()) throw IndexOutOfBoundException(index,_subTasks.size(),"index>=subTasks.size()",__FUNCTION__);
  return _subTasks[index];
  }

  String Task::parentName() const
  {
  if (_parentTask!=nullptr)
    return _parentTask->name();
  else
    throw Exception("Task has no parent",__FUNCTION__);
  }

  Task * Task::taskAt(int depth)
  {
  //if (depth == 0) return this;
  Task * task = this;
  Task * aParentTask = task->parentTask();
  for (int k=0; k<depth; k++)
    {
    task = aParentTask;
    if (task==nullptr) break;
    aParentTask = task->parentTask();
    }
  return task;
  }


  const Task * Task::taskAt(int depth) const
  {
  //if (depth == 0) return this;
  const Task * task = this;
  const Task * aParentTask = task->parentTask();
  //std::cout <<  "0 : name: " << task->name();
  for (int k=0; k<depth; k++)
    {
    task = aParentTask;
    if (task==nullptr) break;
    //std::cout << k <<  " : name: " << task->name();
    aParentTask = task->parentTask();
    }
  return task;
  }

  String Task::reverseTaskPath(int depth) const
  {
  String result;
  String work;
  String colon = String(":");
  for (int k=0; k<=depth; k++)
    {
    const Task * task = taskAt(k);
    if (task!=nullptr)
      {
      work = task->name();
      work += colon;
      if (result.Length()>0)
        {
        result = work + result;
        }
      else
        {
        result = work;
        }
      }
    }
  return result;
  }

  std::vector<String> Task::taskPathTokens() const
  {
  std::vector<String> paths;
  int n = nAncestors();
  for (int k=n; k>=0; k--)
    {
    const Task * task= taskAt(k);
    if (task==nullptr)
      {
      String s="task==nullptr at k="; s+=k;
      throw Exception(s,"Task::taskPathTokens()");
      }
    else
      {
      paths.push_back(task->name());
      }
    }
  return paths;
  }


  String Task::taskPath(int depth) const
  {
  String path = "";
  std::vector<String> paths = taskPathTokens();
  for (int k=0; k<=depth; k++)
    {
    path += paths[k];
    path += ":";
    }
  return path;
  }


  String Task::fullTaskPath() const
  {
  String path;
  int n = nAncestors();
  path = reverseTaskPath(n);
  return path;
  }


  int Task::nAncestors() const
  {
  int count = 0;
  if (_parentTask==nullptr) return count;
  Task * aParentTask = _parentTask;
  while (aParentTask!=nullptr)
    {
    count++;
    if (aParentTask!=nullptr)
      aParentTask = aParentTask->parentTask();
    else
      break;
    }
  return count;
  }

  //!
  //!Instantiate a task within the given parent, based on the given taksReferenceName, the given requested configuration, and the given option.
  //!This function configures but does not initialize the created task
  //!
  Task * Task::createTask(Task * parentTask,
                          const String & taskName,
                          const Configuration & configuration)
  {
  String keyword = taskName;
  keyword += ":TASK:CLASS";
  String taskClassName   = configuration.valueString(keyword);
  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printValue(taskName+":TASK:NAME",taskName);
    printValue(taskName+":TASK:CLASS",taskClassName);
    }
  TClass * taskClass = TClass::GetClass(taskClassName);
  if (!taskClass) throw UnknownClassException(taskClassName,__FUNCTION__);
  if (!taskClass->IsLoaded()) throw Exception("!taskClass->IsLoaded()",__FUNCTION__);
  if (!taskClass->GetNew())  throw Exception("does not have the accelerator function/wrapper",__FUNCTION__);
  Task * task = static_cast<Task*>(taskClass->New());
  task->setName(taskName);
  task->setClassName(taskClassName);
  if (parentTask) parentTask->addSubTask(task);
  task->setDefaultConfiguration();
  task->configure(configuration);
  configureSubtasks(task,taskName,configuration);
  return task;
  }




} // namespace CAP


