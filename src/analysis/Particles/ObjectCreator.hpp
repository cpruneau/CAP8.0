/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ObjectCreator.hpp
//! @brief Tasks creating the filters, efficiencies, smearers, events and particle databases of a job, used by the other
//! tasks by name.
#ifndef CAP__ObjectCreator
#define CAP__ObjectCreator
#include "Task.hpp"
#include "ManagedObjects.hpp"
#include "Event.hpp"
#include "ParticleDb.hpp"
#include "EventFilter.hpp"
#include "EventEfficiency.hpp"
#include "EventSmearer.hpp"
#include "ParticleFilter.hpp"
#include "ParticleEfficiency.hpp"
#include "ParticleSmearer.hpp"

namespace CAP
{

//!
//! @brief Task creating all the objects of one type of a job (e.g. all its particle filters): its list of objects
//! (ManagedObjects) owns them, so they go to the store of that type, where the other tasks take them by name
//! (OWNER false in their own lists).
//!
//! Configuration, for creator \<task\> of objects of type \<TYPE\> (see ManagedObjects):
//! @code
//!   <task>:<TYPE>:N, IMPORT, CREATE, EXPORT, IMPORT:..., EXPORT:...
//!   <task>:<TYPE>_<k>:NAME, TITLE, OWNER (true)
//!   <task>:<TYPE>_<k>:CONDITION:N, CONDITION_<i>:...                  // filters
//!   <task>:<TYPE>_<k>:<parameter>                                     // efficiencies and smearers
//! @endcode
//! The objects are created when the creator is configured, so the creators run before the tasks using their objects
//! (see FilterCreator). They are imported or created (IMPORT, CREATE) at initialization and exported (EXPORT) at
//! finalization; they are not cleared at finalization, the tasks using them being finalized after the creator.
//!
//! @tparam MANAGED type of the objects (EventFilter, ParticleFilter, EventEfficiency, ...)
//!
template <class MANAGED>
class ObjectCreator
:
public Task
{
public:

  //! @brief Constructor. @param typeName type of the objects (prefix of their keys, e.g. "PARTICLE_FILTER")
  ObjectCreator(const String & typeName="NONE")
  :
  Task(),
  _typeName(typeName),
  _objects()
  {
  appendClassName("ObjectCreator");
  setMinimumReportLevel(Object::kInfo);
  }
  //! @brief Copy constructor (the objects are shared). @param source task to copy
  ObjectCreator(const ObjectCreator & source)
  :
  Task(source),
  _typeName(source._typeName),
  _objects(source._objects)
  { }
  //! @brief Copy assignment (the objects are shared). @param rhs task to copy @return this task
  ObjectCreator & operator=(const ObjectCreator & rhs)
  {
  if (this!=&rhs)
    {
    Task::operator=(rhs);
    _typeName = rhs._typeName;
    _objects  = rhs._objects;
    }
  return *this;
  }
  //! @brief Destructor.
  virtual ~ObjectCreator() {}

  //! @brief Default configuration: that of the task and of its list of objects.
  void setDefaultConfiguration() override
  {
  Task::setDefaultConfiguration();
  _objects.setDefaultObjectConfiguration(Task::name(),_typeName,_configuration);
  }
  //! @brief Configure the task and create the objects (configured from their keys). @param configuration configuration
  void configure(const Configuration & configuration) override
  {
  Task::configure(configuration);
  _objects.configureObjects(configuration);
  }
  //! @brief Initialize the task and import or create the objects (IMPORT, CREATE).
  void initialize() override
  {
  Task::initialize();
  _objects.initialize();
  }
  //! @brief Export the objects (EXPORT) and finalize the task; the objects are kept for the tasks using them.
  void finalize() override
  {
  if (_objects.exports()) _objects.exportObject();
  Task::finalize();
  }

  //! @brief Type of the objects. @return type (e.g. "PARTICLE_FILTER")
  const String & typeName() const { return _typeName; }
  //! @brief Number of objects created. @return number
  unsigned int nObjects() const { return _objects.size(); }
  //! @brief Objects created. @return objects (owned by the store of their type)
  const std::vector<MANAGED*> & objects() const { return _objects.getObjects(); }

protected:

  String _typeName;                   //!< type of the objects (prefix of their keys)
  ManagedObjects<MANAGED> _objects;   //!< objects created (OWNER true: they go to the store of their type)

};

//! @brief Task creating the event filters of a job (EVENT_FILTER). See ObjectCreator.
class EventFilterCreator : public ObjectCreator<EventFilter>
{
public:
  //! @brief Constructor.
  EventFilterCreator() : ObjectCreator<EventFilter>("EVENT_FILTER") { appendClassName("EventFilterCreator"); setName("EventFilterCreator"); }
  //! @brief Polymorphic copy. @return copy (owned by the caller)
  Task * clone() override { return new EventFilterCreator(*this); }
};

//! @brief Task creating the particle filters of a job (PARTICLE_FILTER). See ObjectCreator.
class ParticleFilterCreator : public ObjectCreator<ParticleFilter>
{
public:
  //! @brief Constructor.
  ParticleFilterCreator() : ObjectCreator<ParticleFilter>("PARTICLE_FILTER") { appendClassName("ParticleFilterCreator"); setName("ParticleFilterCreator"); }
  //! @brief Polymorphic copy. @return copy (owned by the caller)
  Task * clone() override { return new ParticleFilterCreator(*this); }
};

//! @brief Task creating the event efficiencies of a job (EVENT_EFFICIENCY). See ObjectCreator.
class EventEfficiencyCreator : public ObjectCreator<EventEfficiency>
{
public:
  //! @brief Constructor.
  EventEfficiencyCreator() : ObjectCreator<EventEfficiency>("EVENT_EFFICIENCY") { appendClassName("EventEfficiencyCreator"); setName("EventEfficiencyCreator"); }
  //! @brief Polymorphic copy. @return copy (owned by the caller)
  Task * clone() override { return new EventEfficiencyCreator(*this); }
};

//! @brief Task creating the particle efficiencies of a job (PARTICLE_EFFICIENCY). See ObjectCreator.
class ParticleEfficiencyCreator : public ObjectCreator<ParticleEfficiency>
{
public:
  //! @brief Constructor.
  ParticleEfficiencyCreator() : ObjectCreator<ParticleEfficiency>("PARTICLE_EFFICIENCY") { appendClassName("ParticleEfficiencyCreator"); setName("ParticleEfficiencyCreator"); }
  //! @brief Polymorphic copy. @return copy (owned by the caller)
  Task * clone() override { return new ParticleEfficiencyCreator(*this); }
};

//! @brief Task creating the event smearers of a job (EVENT_SMEARER). See ObjectCreator.
class EventSmearerCreator : public ObjectCreator<EventSmearer>
{
public:
  //! @brief Constructor.
  EventSmearerCreator() : ObjectCreator<EventSmearer>("EVENT_SMEARER") { appendClassName("EventSmearerCreator"); setName("EventSmearerCreator"); }
  //! @brief Polymorphic copy. @return copy (owned by the caller)
  Task * clone() override { return new EventSmearerCreator(*this); }
};

//! @brief Task creating the particle smearers of a job (PARTICLE_SMEARER). See ObjectCreator.
class ParticleSmearerCreator : public ObjectCreator<ParticleSmearer>
{
public:
  //! @brief Constructor.
  ParticleSmearerCreator() : ObjectCreator<ParticleSmearer>("PARTICLE_SMEARER") { appendClassName("ParticleSmearerCreator"); setName("ParticleSmearerCreator"); }
  //! @brief Polymorphic copy. @return copy (owned by the caller)
  Task * clone() override { return new ParticleSmearerCreator(*this); }
};

//! @brief Task creating the events of a job (EVENT), filled by the generators or readers. See ObjectCreator.
class EventCreator : public ObjectCreator<Event>
{
public:
  //! @brief Constructor.
  EventCreator() : ObjectCreator<Event>("EVENT") { appendClassName("EventCreator"); setName("EventCreator"); }
  //! @brief Polymorphic copy. @return copy (owned by the caller)
  Task * clone() override { return new EventCreator(*this); }
};

//! @brief Task creating the particle databases of a job (PARTICLE_DB), imported at initialization (IMPORT, from an
//! XML file holding the particles and their decay modes, e.g. ParticleData/ParticleDb.xml). See ObjectCreator.
class ParticleDbCreator : public ObjectCreator<ParticleDb>
{
public:
  //! @brief Constructor.
  ParticleDbCreator() : ObjectCreator<ParticleDb>("PARTICLE_DB") { appendClassName("ParticleDbCreator"); setName("ParticleDbCreator"); }
  //! @brief Polymorphic copy. @return copy (owned by the caller)
  Task * clone() override { return new ParticleDbCreator(*this); }
};

//!
//! @brief Task creating the filters, efficiencies, smearers, events and particle databases of a job, run first: its
//! subtasks, one per type (EventFilterCreator, ParticleFilterCreator, EventEfficiencyCreator,
//! ParticleEfficiencyCreator, EventSmearerCreator, ParticleSmearerCreator, EventCreator, ParticleDbCreator), create all
//! the objects of their type; the tasks of the job then use them by name.
//!
//! @code
//!   <TASK name="Filters" class="CAP::FilterCreator">
//!     <TASK name="ParticleFilters" class="CAP::ParticleFilterCreator">
//!       <GROUP name="PARTICLE_FILTER"> <PARAMETER name="N" type="int">1</PARAMETER> </GROUP>
//!       <GROUP name="PARTICLE_FILTER_0"> NAME, OWNER (true), CONDITION:N, CONDITION_0:... </GROUP>
//!     </TASK>
//!   </TASK>
//!   <TASK name="Analysis" class="CAP::EventIterator"> ... PARTICLE_FILTER_0:NAME, OWNER (false) ... </TASK>
//! @endcode
//! Its subtasks are those of its configuration (nested TASK elements); RunCAP.py writes one per type the job uses.
//!
class FilterCreator : public Task
{
public:
  //! @brief Constructor.
  FilterCreator();
  //! @brief Copy constructor (the subtasks are cloned). @param source task to copy
  FilterCreator(const FilterCreator & source) : Task(source) {}
  //! @brief Destructor.
  virtual ~FilterCreator() {}
  //! @brief Polymorphic copy. @return copy (owned by the caller)
  Task * clone() override { return new FilterCreator(*this); }
};

} // namespace CAP

#endif /* CAP__ObjectCreator */
