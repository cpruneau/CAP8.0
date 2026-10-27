/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include <algorithm>
#include "EventIterator.hpp"
#include "Exceptions.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"


namespace CAP
{

EventIterator::EventIterator()
:
EventProcessor(),
_nEventsRequested(1),
_nEventsReport(10)
{
  appendClassName("EventIterator");
  setMinimumReportLevel(Object::kInfo);
  setName("EventIterator");
  setTitle("EventIterator");
}

EventIterator::EventIterator(const EventIterator & source)
:
EventProcessor(source),
_nEventsRequested(source._nEventsRequested),
_nEventsReport(source._nEventsReport)
{  /* no ops */ }

EventIterator EventIterator::operator=(const EventIterator & rhs)
{
  if (this!=&rhs)
    {
    EventProcessor::operator=(rhs);
    _nEventsRequested    = rhs._nEventsRequested;
    _nEventsReport       = rhs._nEventsReport;
    }
  return *this;
}

void EventIterator::setDefaultConfiguration()
{
  EventProcessor::setDefaultConfiguration();
  const String & taskName = Task::name();
  _configuration.addProperty( createKey(taskName,"EVENT:REQUESTED:N"),_nEventsRequested);
  _configuration.addProperty( createKey(taskName,"EVENT:REPORT:N"),   _nEventsReport);
}

void EventIterator::configure(const Configuration & configuration)
{
  EventProcessor::configure(configuration);
  const String & taskName = Task::name();
  _nEventsRequested = configuration.valueInt(createKey(taskName,"EVENT:REQUESTED:N"));
  _nEventsReport    = configuration.valueInt(createKey(taskName,"EVENT:REPORT:N"));
  if (reportDebug(__FUNCTION__))
    {
    printCR();
    printValue(createKey(taskName,"EVENT:REQUESTED:N"),_nEventsRequested);
    printValue(createKey(taskName,"EVENT:REPORT:N"),   _nEventsReport);
    }
}

void EventIterator::initialize()
{
  // the event lists are set by configure(): checked before the subtasks initialize (PYTHIA's takes a while)
  for (auto & subTask : subTasks()) checkEventsMadeBy(*subTask);
  EventProcessor::initialize();
}

void EventIterator::checkEventsMadeBy(const Task & task) const
{
  if (dynamic_cast<const EventIterator*>(&task)) return;     // a nested iterator resets its own events
  const EventProcessor * processor = dynamic_cast<const EventProcessor*>(&task);
  if (processor && processor->makesEvents())
    {
    for (const Event * made : processor->events())
      {
      if (std::find(events().begin(),events().end(),made)!=events().end()) continue;
      String message = "Events ";
      message += made->name();
      message += " made by task ";
      message += task.name();
      message += " are not used by the event iterator ";
      message += name();
      message += ": the iterator must use them (it resets them before each event)";
      throw ConfigurationException(createKey(name(),"EVENT"),message,__FUNCTION__);
      }
    }
  for (auto & subTask : task.subTasks()) checkEventsMadeBy(*subTask);
}

void EventIterator::execute()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  _timer.start();
  long iEvent = 0;
  try
  {
  while(iEvent<_nEventsRequested)
    {
    Particle::factory().reset();
    for (auto & event : events()) event->reset();
    for (auto & subTask : _subTasks) subTask->execute();
    if (iEvent%_nEventsReport == 0  &&  reportInfo(__FUNCTION__)) printValue("iEvent",iEvent);
    iEvent++;
    }
  }
  catch (CAP::EndOfDataException & exception)
  {
  if (reportWarning(__FUNCTION__))
    {
    printCR();
    printValue("EVENT:REQUESTED:N",_nEventsRequested);
    printValue("EVENT:INDEX",iEvent);
    printString("File Stream Ended");
    printLine();
    }
  _timer.stop();
  printCR();
  printLine();
  _timer.print();
  printLine();
  printCR();
  }
}


} // namespace CAP
