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
#ifndef CAP__Task
#define CAP__Task
#include "ConfigurationManager.hpp"
#include "Accountant.hpp"
#include "Object.hpp"
#include "Timer.hpp"
#include "XmlDocument.hpp"
#include <iostream>
namespace CAP
{

  //!
  //! @brief Base class of the analysis and simulation tasks, organized as a tree of subtasks.
  //!
  //! <b>Life cycle.</b> A task is configured, initialized once, executed repeatedly (e.g., once per event), and
  //! finalized:
  //! - setDefaultConfiguration(): add the default values of the task's parameters to its configuration;
  //! - configure(): take from a configuration the properties of this task (those whose keys contain the task name as a
  //!   path component, e.g. "PtPt:nBins" for task PtPt) and set the report level from "<name>:TASK:SEVERITY";
  //! - initialize(): start the timer, reset the execution counter, initialize the subtasks;
  //! - execute(): count the call and execute the subtasks;
  //! - partial(), postProcess(): intermediate saves and post-processing (derived classes);
  //! - finalize(): finalize the subtasks and stop the timer.
  //!
  //! Derived classes override these functions and call the base version to keep the subtask calls (doing their own
  //! work first, so that it precedes that of the subtasks).
  //!
  //! <b>Subtasks.</b> A task owns its subtasks (addSubTask()) and deletes them in clearSubTasks(); each subtask knows its
  //! parent. Tasks can be created by name from a configuration with createTask(), which uses ROOT's dictionary: the
  //! class of task "X" is read from "X:TASK:CLASS", and its subtasks from "X:SUBTASK:N" and "X:SUBTASK_<k>:NAME".
  //!
  //! <b>Copies.</b> Copies clone the subtasks with clone(): every derived class must override clone(). The parent is not
  //! copied.
  //!
  //! Example of configuration:
  //! @code
  //!   Analysis:TASK:CLASS = CAP::EventIterator
  //!   Analysis:SUBTASK:N = 1
  //!   Analysis:SUBTASK_0:NAME = PtPt
  //!   PtPt:TASK:CLASS = CAP::PtPtAnalyzer
  //! @endcode
  //!
  //! <b>XML configuration.</b> A task maps to a TASK element (see ConfigurationManager for the parameters): its name,
  //! class and severity are attributes, its parameters PARAMETER and GROUP elements, and its subtasks nested TASK
  //! elements. The keys <name>:SUBTASK:N and <name>:SUBTASK_<k>:NAME are implied by the nested TASK elements.
  //! @code
  //!   <CAP version="1">
  //!     <TASK name="Analysis" class="CAP::EventIterator" severity="INFO">
  //!       <PARAMETER name="nEventsRequested" type="long">1000</PARAMETER>
  //!       <TASK name="PtPt" class="CAP::PtPtAnalyzer" severity="INFO">
  //!         <GROUP name="HISTOGRAM">
  //!           <PARAMETER name="pt_nbins" type="int">100</PARAMETER>
  //!         </GROUP>
  //!       </TASK>
  //!     </TASK>
  //!   </CAP>
  //! @endcode
  //! createTask(parent, element) creates a task from its element: it instantiates the class, builds the template of
  //! its configuration with setDefaultConfiguration(), updates it with the values of the element and configures the
  //! task (readXmlElement()), and creates the subtasks the same way. Parameters that a task does not define in
  //! setDefaultConfiguration() are accepted with a warning. writeXml() writes the configuration of a task tree, and
  //! exportXmlDictionary() the template of the tree (default values of all tasks).
  //!
  class Task
  :
  public Object,
  public ConfigurationManager
  {
  public:

  //! @brief Default constructor: name, title and class name "Task", report level kInfo, no subtasks.
  Task();
  //! @brief Copy constructor: the subtasks are cloned; the copy has no parent. @param task task to copy
  Task(const Task & task);
  //! @brief Copy assignment: the subtasks are replaced by clones of those of task; the parent is unchanged.
  //! @param task task to copy @return this task
  Task & operator=(const Task & task);
  //! @brief Destructor: the subtasks are deleted.
  virtual ~Task();

  //! @brief Add the default configuration of the task: <name>:TASK:NAME, <name>:TASK:CLASS and <name>:TASK:SEVERITY (INFO).
  //! Derived classes add their own parameters.
  virtual void setDefaultConfiguration();
  //! @brief Take the properties of this task from a configuration and set the report level from <name>:TASK:SEVERITY.
  //! @param configuration configuration
  virtual void configure(const Configuration & configuration);
  //! @brief Start the timer, reset the execution counter and initialize the subtasks.
  virtual void initialize();
  //! @brief Count the execution and execute the subtasks.
  virtual void execute();
  //! @brief Post-processing after the run (nothing in the base class).
  virtual void postProcess();
  //! @brief Intermediate processing (e.g., partial saves) of the subtasks.
  virtual void partial();
  //! @brief Finalize the subtasks and stop the timer.
  virtual void finalize();
  //! @brief Reset the subtasks and set the execution counter to 0.
  virtual void reset();
  //! @brief Clear the execution counter and delete the subtasks.
  virtual void clear();
  //! @brief Print the task, its execution counter, its subtasks and its timer.
  virtual void print() const;
  //!
  //! @brief Create the subtasks of a task listed in a configuration (<parentName>:SUBTASK:N and
  //! <parentName>:SUBTASK_<k>:NAME), with createTask().
  //! @param task task receiving the subtasks @param parentName name of that task @param configuration configuration
  //!
  virtual void configureSubtasks(Task * task,
                                 const String & parentName,
                                 const Configuration & configuration);
  //! @brief Polymorphic copy. @return new copy (owned by the caller)
  //! @throws NoImplementationException if called on a derived class that does not override it
  virtual Task * clone();

  //! @name Operations on the subtasks
  //! @{
  //! @brief Initialize the subtasks.
  void initializeSubTasks();
  //! @brief Execute the subtasks.
  void executeSubTasks();
  //! @brief Finalize the subtasks.
  void finalizeSubTasks();
  //! @brief Call partial() on the subtasks.
  void partialSubTasks();
  //! @brief Reset the subtasks.
  void resetSubTasks();
  //! @brief Clear and delete the subtasks.
  void clearSubTasks();
  //! @brief Print the subtasks.
  void printSubTasks() const;

  //! @}

  //! @name Subtasks
  //! @{
  //! @brief Whether a task is a direct subtask of this one. @param task task @return true if so
  bool isSubtask(Task * task) const;
  //! @brief Direct subtasks (owned by this task). @return subtasks
  const std::vector<Task*> & subTasks() const  { return _subTasks; }
  //! @brief Whether this task has subtasks. @return true if so
  bool hasSubTasks()  const  { return _subTasks.size()>0;  }
  //! @brief Number of direct subtasks. @return number of subtasks
  unsigned int nSubTasks() const  { return _subTasks.size(); }
  //! @brief Subtask at an index. @param index index @return subtask @throws IndexOutOfBoundException if index>=nSubTasks()
  Task * subTaskAt(unsigned int index);
  //! @brief Add a subtask, which this task then owns. @param task subtask created with new @return task
  //! @throws Exception if task is nullptr, this task, already has a parent or already is a subtask
  Task * addSubTask(Task * task);

  //! @}

  //! @name Parent and task paths
  //! @{
  //! @brief Whether this task has a parent. @return true if so
  bool hasParent() const      {   return _parent!=nullptr; }
  //! @brief Parent task. @return parent, or nullptr
  Task * parent() const   {   return _parent; }
  //! @brief Name of the parent task. @return name @throws Exception if there is no parent
  String parentName() const;
  //! @brief Set the parent task (use addSubTask() on the parent instead). @param parent parent
  void setParent(Task * parent)  { _parent = parent; }
  //! @brief Ancestor at a depth. @param depth 0 for this task, 1 for its parent, ... @return task, or nullptr if too deep
  Task * taskAt(int depth);
  //! @brief Ancestor at a depth. @param depth 0 for this task, 1 for its parent, ... @return task, or nullptr if too deep
  const Task * taskAt(int depth) const;
  //! @brief Path of the ancestors up to a depth, from the oldest one: "ancestor:...:parent:name:".
  //! @param depth number of ancestors included @return path
  String reverseTaskPath(int depth=0) const;
  //! @brief Names of the tasks from the root task to this one. @return names
  std::vector<String> taskPathTokens() const;
  //! @brief Path of the first depth+1 tasks from the root task: "root:child:...:".
  //! @param depth index of the last task included (0 for the root task) @return path
  String taskPath(int depth=0) const;
  //! @brief Path from the root task to this one: "root:...:name:". @return path
  String fullTaskPath() const;
  //! @brief Number of ancestors (0 for a root task). @return number of ancestors
  int nAncestors() const;

  //! @}

  //!
  //! @brief Create a task by name from a configuration, without initializing it: its class is read from
  //! <taskName>:TASK:CLASS and instantiated through ROOT's dictionary; the task is added to the parent, configured, and
  //! its subtasks are created (configureSubtasks()).
  //! @param _parent parent of the new task, or nullptr @param taskName name of the task @param configuration configuration
  //! @return new task (owned by the parent, or by the caller if there is no parent)
  //! @throws UnknownClassException if the class is unknown to ROOT; Exception if it cannot be instantiated
  //!
  Task * createTask(Task * _parent,
                    const String & taskName,
                    const Configuration & configuration);

  //!
  //! @brief Create a task tree from a TASK element (see the XML configuration above), without initializing it.
  //! @param _parent parent of the new task, or nullptr @param element TASK element (attributes name and class required)
  //! @return new task (owned by the parent, or by the caller if there is no parent)
  //! @throws ConfigurationException if the element is not a valid TASK element; UnknownClassException if the class is
  //! unknown to ROOT; ConfigurationException for invalid values
  //!
  Task * createTask(Task * _parent, const XmlElement & element);

  //! @name XML mapping (see ConfigurationManager)
  //! @{
  //! @brief Empty TASK element of this task, with attributes name and class. @return element
  XmlElement createXmlElement() const override;
  //! @brief Fill a TASK element with the template of the configuration of this task and of its subtasks.
  //! @param element element made by createXmlElement()
  void defineXmlElement(XmlElement & element) override;
  //!
  //! @brief Template of this task alone, as a standalone XML document: root element CAP (attribute type="template")
  //! holding the TASK element of this task filled with its default parameters and their types (see
  //! defineXmlElement(); subtasks are not included). Meant for editors of configurations: the template lists every
  //! parameter a task of this class accepts.
  //! @return document
  //!
  XmlDocument createXmlTemplate();
  //! @brief Write the template of this task (see createXmlTemplate()) to path/<class name without namespace>.xml.
  //! @param path directory @return name of the file written @throws FileException if the file cannot be written
  String exportXmlTemplate(const String & path);
  //! @brief Name of the template file of this task: its class name without namespace (e.g. PtPtAnalyzer; template
  //! arguments are joined with underscores). @return name
  String xmlTemplateName() const;
  //!
  //! @brief Write the templates of task classes (see exportXmlTemplate()), each task being named after its class.
  //! @param path directory
  //! @param classNames classes; all the non-abstract classes deriving from CAP::Task known to ROOT (see taskClassNames())
  //! if empty
  //! @return names of the files written (classes that cannot be instantiated are skipped with a warning)
  //!
  static std::vector<String> exportXmlTemplates(const String & path, const std::vector<String> & classNames=std::vector<String>());
  //! @brief Names of the non-abstract, non-template classes deriving from CAP::Task whose dictionaries are loaded.
  //! @return sorted names
  static std::vector<String> taskClassNames();
  //! @brief Fill a TASK element with the configuration of this task (severity attribute, parameters) and its subtasks.
  //! @param element element made by createXmlElement()
  void fillXmlElement(XmlElement & element) const override;
  //!
  //! @brief Update the configuration from a TASK element, configure() the task, and do the same for the subtasks
  //! (subtasks of the element that this task does not have are created with createTask()).
  //! @param element TASK element of this task
  //! @throws ConfigurationException if the element is not the TASK element of this task or holds invalid values
  //!
  void readXmlElement(const XmlElement & element) override;
  //! @}

  //! @brief Counter of the calls to execute(). @return accountant
  Accountant & taskExecuted() { return _taskExecuted; }
  //! @brief Counter of the calls to execute(). @return accountant
  const Accountant & taskExecuted() const { return _taskExecuted; }
  //! @brief Add one to the counter of the calls to execute().
  void incrementTaskExecuted() { _taskExecuted.increment(); }

  protected:

  //! @brief Instantiate a task of a class through ROOT's dictionary. @param taskClassName class name @return new task
  //! @throws UnknownClassException if the class is unknown; Exception if it cannot be instantiated
  static Task * instantiateTask(const String & taskClassName);
  //! @brief Name of the task (the keys of a task begin with its name). @return name
  String xmlKeyPath() const override;
  //! @brief Fill a TASK element with the template of this task only (see defineXmlElement()). @param element element
  void defineOwnXmlElement(XmlElement & element);
  //! @brief False for the keys mapped to attributes (TASK:NAME, TASK:CLASS, TASK:SEVERITY) and to subtask elements
  //! (SUBTASK:N, SUBTASK_<k>:NAME). @param relativeKey key relative to the task name @return true for the others
  bool isXmlParameter(const String & relativeKey) const override;
  //! @brief Accept nested TASK elements (handled by readXmlElement()). @param child element @return true for TASK
  bool readXmlChild(const XmlElement & child) override;
  //! @brief Warn that a parameter read is not defined by setDefaultConfiguration(). @param key key of the parameter
  void unknownXmlParameter(const String & key) override;

  //!
  //! Pointer to parent task if any
  //!
  Task * _parent;

  //!
  //! Array of pointers to subTasks called by this task instance, once per event analyzed (or iteration generated by EventIterator task). If this instance carries out
  //! initialize, finalize, execute type operations, these are performed BEFORE the corresponding operations by the subTasks.
  //!
  std::vector<Task*> _subTasks;

  //!
  //! Use an accountant instance to keep track of the number of times the "execute" function is called.
  //!
  Accountant _taskExecuted;

  //!
  //! Use time to keep track of executioin time
  //!
  Timer _timer;

  ClassDef(Task,0)
  };

}

#endif /* CAP__Task */
