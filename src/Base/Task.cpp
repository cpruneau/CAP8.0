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
#include <typeinfo>

ClassImp(CAP::Task);

namespace CAP
{


  Task::Task()
  :
  Object(),
  ConfigurationManager(),
  _parent(nullptr),
  _subTasks(),
  _taskExecuted("taskExecuted",1),
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
  _parent(nullptr),
  _subTasks(),
  _taskExecuted(source._taskExecuted),
  _timer(source._timer)
  {
  if (source.hasSubTasks())
    {
    const std::vector<Task*> & subtasks = source.subTasks();
    for (auto & subtask : subtasks)
      {
      addSubTask(subtask->clone());
      }
    }
  }

  Task & Task::operator=(const Task & rhs)
  {
  if (this!=&rhs)
    {
    clear();
    Object::operator=(rhs);
    ConfigurationManager::operator=(rhs);
    _taskExecuted = rhs._taskExecuted;
    // Note: _parent is deliberately NOT copied from rhs, consistent with the
    // copy constructor. Blindly copying rhs's parent pointer would leave
    // `this` pointing at a Task whose _subTasks list does not contain `this`
    // (a dangling reference the moment that parent is destroyed), without
    // actually attaching `this` to that parent's subtask tree. A copied/
    // assigned task keeps whatever parent (if any) it already had; use
    // addSubTask() on the desired parent to reparent it explicitly.
    if (rhs.hasSubTasks())
      {
      const std::vector<Task*> & subtasks = rhs.subTasks();
      for (auto & subtask : subtasks)
        {
        addSubTask(subtask->clone());
        }
      }
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
  // Accountant::clear() (invoked by Task::clear(), and at the top of
  // operator=) empties _taskExecuted's underlying count vector entirely,
  // whereas Accountant::reset() merely zeroes it while keeping its size.
  // Guard here so that execute() called after clear() without an
  // intervening initialize() re-sizes the accountant instead of throwing
  // an IndexOutOfBoundException out of Accountant::increment().
  if (_taskExecuted.size()==0) _taskExecuted.initialize("taskExecuted",1);
  _taskExecuted.increment();
  if (hasSubTasks()) executeSubTasks();
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
  if (reportDebug(__FUNCTION__))  { printCR();  print(); };
  }

  void Task::partial()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  if (hasSubTasks()) partialSubTasks();
  }

  void Task::reset()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  if (hasSubTasks()) resetSubTasks();
  _taskExecuted.reset();
  }

  void Task::clear()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  _taskExecuted.clear();
  if (hasSubTasks()) clearSubTasks();
  }

  void Task::print() const
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  printLine();
  Object::print();
  _taskExecuted.print();
  if (hasSubTasks()) printSubTasks();
  _timer.print();
  printLine();
  }

  Task * Task::clone()
  {
  // Task::clone() is a prototype-pattern hook: Task::operator=/copy-ctor use
  // it to duplicate _subTasks polymorphically. Every concrete derived class
  // MUST override clone() to return `new DerivedClass(*this)`; otherwise a
  // derived subtask copied through this base implementation would be
  // silently sliced down to a plain Task, losing its derived data members
  // and overridden behavior. Fail loudly instead of slicing silently.
  if (typeid(*this)!=typeid(Task))
    {
    throw NoImplementationException(className(),"Task::clone() -- derived class does not override clone()");
    }
  return new Task(*this);
  }

  void Task::configureSubtasks(Task * parent,
                               const String & parentName,
                               const Configuration & configuration)
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  int nSubtasks = configuration.valueInt( createKey(parentName,"SUBTASK","N") );
  for (int k=0; k<nSubtasks; k++)
    {
    String subtaskName  = configuration.valueString( createKey(parentName,"SUBTASK",k,"NAME")   );
    createTask(parent,subtaskName,configuration);
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
    }
  for (auto & subTask : _subTasks) subTask->print();
  }

  bool Task::isSubtask(Task * task) const
  {
  for (auto & subTask : _subTasks)
    {
    if (subTask == task) return true;
    }
  return false;
  }
  
  Task * Task::addSubTask(Task * task)
  {
  if (!task)      throw Exception("Given task pointer is null.",__FUNCTION__);
  if (task==this) throw Exception("Given task pointer is self.",__FUNCTION__);
  if (task->hasParent()) throw Exception("Given task already has a parent.",__FUNCTION__);
  if (isSubtask(task))   throw Exception("Given task already is a subtask.",__FUNCTION__);
  _subTasks.push_back( task );
  task->setParent(this);
  
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
  if (_parent!=nullptr)
    return _parent->name();
  else
    throw Exception("Task has no parent",__FUNCTION__);
  }

  Task * Task::taskAt(int depth)
  {
  //if (depth == 0) return this;
  Task * task = this;
  Task * aparent = task->parent();
  for (int k=0; k<depth; k++)
    {
    task = aparent;
    if (task==nullptr) break;
    aparent = task->parent();
    }
  return task;
  }


  const Task * Task::taskAt(int depth) const
  {
  //if (depth == 0) return this;
  const Task * task = this;
  const Task * aparent = task->parent();
  //std::cout <<  "0 : name: " << task->name();
  for (int k=0; k<depth; k++)
    {
    task = aparent;
    if (task==nullptr) break;
    //std::cout << k <<  " : name: " << task->name();
    aparent = task->parent();
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
  Task * parent = _parent;
  while (parent!=nullptr)
    {
    count++;
    parent = parent->parent();
    }
  return count;
  }

  //!
  //!Instantiate a task within the given parent, based on the given taksReferenceName, the given requested configuration, and the given option.
  //!This function configures but does not initialize the created task
  //!
  Task * Task::createTask(Task * parent,
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
  if (!task) throw NullPointerException("taskClass->New() returned nullptr for class "+taskClassName,__FUNCTION__);
  task->setName(taskName);
  task->setClassName(taskClassName);
  if (parent) parent->addSubTask(task);
  task->setDefaultConfiguration();
  task->configure(configuration);
  configureSubtasks(task,taskName,configuration);
  return task;
  }




} // namespace CAP


