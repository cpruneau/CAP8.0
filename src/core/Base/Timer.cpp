/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "Timer.hpp"
#include "PrintHelpers.hpp"


CAP::Timer::Timer()
:
startTime(),
stopTime(),
oldStop(),
totalDuration(),
intervalDuration(),
days(0),
hours(0),
minutes(0),
seconds(0),
deltaDays(0),
deltaHours(0),
deltaMinutes(0),
deltaSeconds(0)
{
  start();
}

CAP::Timer::Timer(const CAP::Timer & timer)
:
startTime(timer.startTime),
stopTime(timer.stopTime),
oldStop(timer.oldStop),
totalDuration(timer.totalDuration),
intervalDuration(timer.intervalDuration),
days(timer.days),
hours(timer.hours),
minutes(timer.minutes),
seconds(timer.seconds),
deltaDays(timer.deltaDays),
deltaHours(timer.deltaHours),
deltaMinutes(timer.deltaMinutes),
deltaSeconds(timer.deltaSeconds)
{   }

CAP::Timer & CAP::Timer::operator=(const CAP::Timer & timer)
{
  if (this!=&timer)
    {
    startTime        = timer.startTime;
    stopTime         = timer.stopTime;
    oldStop          = timer.oldStop;
    totalDuration    = timer.totalDuration;
    intervalDuration = timer.intervalDuration;
    days          = timer.days;
    hours         = timer.hours;
    minutes       = timer.minutes;
    seconds       = timer.seconds;
    deltaDays     = timer.deltaDays;
    deltaHours    = timer.deltaHours;
    deltaMinutes  = timer.deltaMinutes;
    deltaSeconds  = timer.deltaSeconds;
    }
  return *this;
}


void CAP::Timer::start()
{
  startTime = high_resolution_clock::now();
  oldStop   = startTime;
}
void CAP::Timer::stop()
{
  stopTime = high_resolution_clock::now();
  intervalDuration = chrono::duration_cast<duration<double>>(stopTime - oldStop);
  totalDuration    = chrono::duration_cast<duration<double>>(stopTime - startTime);
  // Break durations into days / hours (0-23) / minutes (0-59) / seconds (0-60), each unit
  // computed from what remains after removing the larger ones. Previously hours were not
  // reduced modulo 24, and deltaSeconds subtracted the days twice (negative after 24 h).
  double t   = totalDuration.count();
  days       = (int)(t/86400.0);          t -= 86400.0 * days;
  hours      = (int)(t/3600.0);           t -= 3600.0  * hours;
  minutes    = (int)(t/60.0);             t -= 60.0    * minutes;
  seconds    = t;
  double dt  = intervalDuration.count();
  deltaDays    = (int)(dt/86400.0);       dt -= 86400.0 * deltaDays;
  deltaHours   = (int)(dt/3600.0);        dt -= 3600.0  * deltaHours;
  deltaMinutes = (int)(dt/60.0);          dt -= 60.0    * deltaMinutes;
  deltaSeconds = dt;
  oldStop  = stopTime;
}

void CAP::Timer::print()  const
{
  printValue("days",days);
  printValue("hours",hours);
  printValue("minutes",minutes);
  printValue("seconds",seconds);
  printValue("DeltaDays",deltaDays);
  printValue("DeltaHours",deltaHours);
  printValue("DeltaMinutes",deltaMinutes);
  printValue("DeltaSeconds",deltaSeconds);
}


