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
#include "Folder.hpp"
#include "Exceptions.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"
#include "TFile.h"
#include "EnvironmentVariables.hpp"
#include "XmlDocument.hpp"
#include "TClassTable.h"
#include <algorithm>
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

  void Task::saveHistograms(Folder & top)
  {
  for (auto & subtask : _subTasks) subtask->saveHistograms(top);
  }

  void Task::loadHistograms(const Folder & top)
  {
  for (auto & subtask : _subTasks) subtask->loadHistograms(top);
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
      if (result.length()>0)
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
  Task * task = instantiateTask(taskClassName);
  task->setName(taskName);
  task->setClassName(taskClassName);
  if (parent) parent->addSubTask(task);
  task->setDefaultConfiguration();
  task->configure(configuration);
  configureSubtasks(task,taskName,configuration);
  return task;
  }




  Task * Task::instantiateTask(const String & taskClassName)
  {
  TClass * taskClass = TClass::GetClass(taskClassName);
  if (!taskClass) throw UnknownClassException(taskClassName,__FUNCTION__);
  if (!taskClass->IsLoaded()) throw Exception("!taskClass->IsLoaded()",__FUNCTION__);
  if (!taskClass->GetNew())  throw Exception("does not have the accelerator function/wrapper",__FUNCTION__);
  if (!taskClass->InheritsFrom(TClass::GetClass("CAP::Task")))
    throw Exception(String("Class ")+taskClassName+" is not a CAP::Task",__FUNCTION__);
  Task * task = static_cast<Task*>(taskClass->New());
  if (!task) throw NullPointerException("taskClass->New() returned nullptr for class "+taskClassName,__FUNCTION__);
  return task;
  }

  // ---------------------------------------------------------------- XML mapping

  namespace
  {
    //! Whether a relative key is SUBTASK_<k>:NAME.
    bool isSubtaskNameKey(const String & key)
    {
    const std::string & s = key.str();
    const std::string head = "SUBTASK_", tail = ":NAME";
    if (s.size()<=head.size()+tail.size() || s.compare(0,head.size(),head)!=0) return false;
    if (s.compare(s.size()-tail.size(),tail.size(),tail)!=0) return false;
    for (std::size_t k=head.size(); k<s.size()-tail.size(); k++) if (s[k]<'0' || s[k]>'9') return false;
    return true;
    }

    //! Check that an element is a TASK element with a name.
    void checkTaskElement(const XmlElement & element, const char * source)
    {
    if (!element.isNamed("TASK"))
      throw ConfigurationException(element.name(),String("Expected a <TASK> element, got <")+element.name()+">",source);
    if (element.attribute("name","").isNull())
      throw ConfigurationException("TASK","<TASK> element without name attribute",source);
    }
  }

  String Task::xmlKeyPath() const
  {
  return name();
  }

  bool Task::isXmlParameter(const String & relativeKey) const
  {
  return !(relativeKey=="TASK:NAME" || relativeKey=="TASK:CLASS" || relativeKey=="TASK:SEVERITY"
           || relativeKey=="SUBTASK:N" || isSubtaskNameKey(relativeKey));
  }

  bool Task::readXmlChild(const XmlElement & child)
  {
  return child.isNamed("TASK");
  }

  void Task::unknownXmlParameter(const String & key)
  {
  // Files of the job (<task>:JOB:OUTPUT_FILE, INPUT_FILE, read by CAP.cxx for the main task) are expected.
  if (key.contains(":JOB:")) return;
  // Keys of the objects of managed lists (<task>:<TYPE>_<k>:..., the list having <task>:<TYPE>:N) are not in the
  // default configuration, whose lists are empty: they are expected.
  std::string text = key.data();
  std::size_t start = 0;
  while (start < text.size())
    {
    std::size_t end = text.find(':',start);
    if (end == std::string::npos) break;
    std::string part = text.substr(start,end-start);
    std::size_t underscore = part.rfind('_');
    if (underscore != std::string::npos && underscore+1 < part.size()
        && part.find_first_not_of("0123456789",underscore+1) == std::string::npos)
      {
      String list = String(text.substr(0,start).c_str()) + String(part.substr(0,underscore).c_str()) + String(":N");
      for (const Property & p : configuration().properties()) if (p.name() == list) return;
      }
    start = end + 1;
    }
  if (reportWarning(__FUNCTION__))
    std::cout << "Parameter " << key << " is not defined by " << className() << "::setDefaultConfiguration()" << std::endl;
  }

  XmlElement Task::createXmlElement() const
  {
  XmlElement element("TASK");
  element.setAttribute("name",name());
  element.setAttribute("class",className());
  return element;
  }

  void Task::fillXmlElement(XmlElement & element) const
  {
  int index = _configuration.findIndex(createKey(name(),"TASK:SEVERITY"));
  if (index>=0) element.setAttribute("severity",_configuration.properties()[index].value());
  ConfigurationManager::fillXmlElement(element);
  for (const Task * subtask : _subTasks)
    {
    XmlElement & child = element.addChild(subtask->createXmlElement());
    subtask->fillXmlElement(child);
    }
  }

  void Task::defineOwnXmlElement(XmlElement & element)
  {
  // template of this task: its default configuration only (the current configuration is unchanged)
  Configuration defaults = defaultConfiguration();
  int index = defaults.findIndex(createKey(name(),"TASK:SEVERITY"));
  if (index>=0) element.setAttribute("severity",defaults.properties()[index].value());
  fillXmlParameters(element,defaults);
  }

  void Task::defineXmlElement(XmlElement & element)
  {
  defineOwnXmlElement(element);
  for (Task * subtask : _subTasks)
    {
    XmlElement & child = element.addChild(subtask->createXmlElement());
    subtask->defineXmlElement(child);
    }
  }

  String Task::xmlTemplateName() const
  {
  // class name without the namespaces, made a valid file name (e.g. CAP::PtPtAnalyzer -> PtPtAnalyzer)
  String shortName = className();
  shortName.replaceAll("CAP::","");
  shortName.replaceAll("::","_");
  shortName.replaceAll("<","_");
  shortName.replaceAll(",","_");
  shortName.replaceAll(">","");
  shortName.replaceAll(" ","");
  return shortName;
  }

  XmlDocument Task::createXmlTemplate()
  {
  XmlDocument document("CAP");
  document.root().setAttribute("version",1);
  document.root().setAttribute("type","template");
  XmlElement & element = document.root().addChild(createXmlElement());
  defineOwnXmlElement(element);
  return document;
  }

  String Task::exportXmlTemplate(const String & path)
  {
  XmlDocument document = createXmlTemplate();
  String fileName = XmlDocument::fileName(path,xmlTemplateName());
  document.write(fileName);
  return fileName;
  }

  std::vector<String> Task::taskClassNames()
  {
  std::vector<String> names;
  TClass * taskClass = TClass::GetClass("CAP::Task");
  TClassTable::Init();
  while (const char * className = TClassTable::Next())
    {
    TClass * c = TClass::GetClass(className);
    if (!c || !c->InheritsFrom(taskClass) || !c->GetNew()) continue;
    if (c->Property() & kIsAbstract) continue;
    if (String(className).contains("<")) continue;   // class templates are bases of the analysis tasks
    names.push_back(className);
    }
  std::sort(names.begin(),names.end(),[](const String & a, const String & b) { return a.str()<b.str(); });
  names.erase(std::unique(names.begin(),names.end()),names.end());
  return names;
  }

  std::vector<String> Task::exportXmlTemplates(const String & path, const std::vector<String> & classNames)
  {
  std::vector<String> fileNames;
  std::vector<String> names = classNames.empty() ? taskClassNames() : classNames;
  for (const String & className : names)
    {
    Task * task = nullptr;
    try
      {
      task = instantiateTask(className);
      task->setClassName(className);
      task->setName(task->xmlTemplateName());
      fileNames.push_back(task->exportXmlTemplate(path));
      }
    catch (const std::exception & exception)   // CAP::Exception derives from std::exception
      {
      std::cout << "<WARNING> Task::exportXmlTemplates() no template for " << className << ": " << exception.what() << std::endl;
      }
    delete task;
    }
  return fileNames;
  }

  void Task::readXmlElement(const XmlElement & element)
  {
  checkTaskElement(element,__FUNCTION__);
  if (element.attribute("name")!=name())
    throw ConfigurationException(name(),String("<TASK> element of task ")+element.attribute("name")+" given to task "+name(),__FUNCTION__);
  if (element.hasAttribute("class") && element.attribute("class")!=className())
    throw ConfigurationException(name(),String("Class ")+element.attribute("class")+" given to task of class "+className(),__FUNCTION__);

  Configuration updated = _configuration;
  updated.addProperty(createKey(name(),"TASK:NAME"),name());
  updated.addProperty(createKey(name(),"TASK:CLASS"),className());
  if (element.hasAttribute("severity"))
    updated.addProperty(createKey(name(),"TASK:SEVERITY"),element.attribute("severity"));
  readXmlParameters(element,updated);
  std::vector<const XmlElement *> subtaskElements = element.children("TASK");
  for (const XmlElement * child : subtaskElements) checkTaskElement(*child,__FUNCTION__);
  // Subtask keys: the existing subtasks (they are kept, whether or not the element lists them), then the subtasks
  // of the element that will be created below, so that SUBTASK:N matches the resulting tree.
  std::vector<String> subtaskNames;
  for (const Task * subtask : _subTasks) subtaskNames.push_back(subtask->name());
  for (const XmlElement * child : subtaskElements)
    {
    const String childName = child->attribute("name");
    if (std::find(subtaskNames.begin(),subtaskNames.end(),childName)==subtaskNames.end()) subtaskNames.push_back(childName);
    }
  updated.addProperty(createKey(name(),"SUBTASK","N"),int(subtaskNames.size()));
  for (unsigned int k=0; k<subtaskNames.size(); k++)
    updated.addProperty(createKey(name(),"SUBTASK",k,"NAME"),subtaskNames[k]);
  configure(updated);

  for (const XmlElement * child : subtaskElements)
    {
    Task * existing = nullptr;
    for (Task * subtask : _subTasks) if (subtask->name()==child->attribute("name")) { existing = subtask; break; }
    if (existing)
      existing->readXmlElement(*child);
    else
      createTask(this,*child);
    }
  }

  Task * Task::createTask(Task * parent, const XmlElement & element)
  {
  checkTaskElement(element,__FUNCTION__);
  String taskName      = element.attribute("name");
  String taskClassName = element.attribute("class","");
  if (taskClassName.isNull()) throw ConfigurationException(taskName,"<TASK> element without class attribute",__FUNCTION__);
  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printValue(taskName+":TASK:NAME",taskName);
    printValue(taskName+":TASK:CLASS",taskClassName);
    }
  Task * task = instantiateTask(taskClassName);
  task->setName(taskName);
  task->setClassName(taskClassName);
  try
    {
    if (parent) parent->addSubTask(task);
    }
  catch (...)
    {
    delete task;
    throw;
    }
  try
    {
    task->setDefaultConfiguration();
    task->readXmlElement(element);
    }
  catch (...)
    {
    // do not leave a half-configured subtask in the parent's tree: detach it, then delete it (with its subtasks)
    if (parent)
      {
      std::vector<Task*> & siblings = parent->_subTasks;
      siblings.erase(std::remove(siblings.begin(),siblings.end(),task),siblings.end());
      }
    delete task;
    throw;
    }
  return task;
  }

} // namespace CAP


