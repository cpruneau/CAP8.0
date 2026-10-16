/* ********************************************************************* * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez     
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__Timer
#define CAP__Timer
#include <wchar.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include "TObject.h"
using namespace std;
using namespace std::chrono;

namespace CAP
{
//!
//! @brief Wall-clock timer measuring total and lap durations.
//!
//! start() sets the start time. Each call to stop() measures the time elapsed since start() (total duration) and since
//! the previous stop() (interval duration), and breaks both into days, hours (0-23), minutes (0-59) and seconds. The
//! timer is started by the constructor. Task uses a Timer to report its execution time.
//!
//! Example:
//! @code
//!   CAP::Timer timer;   // started
//!   // ... work ...
//!   timer.stop();
//!   timer.print();      // total and interval durations
//! @endcode
//!
class Timer
{
public:

  //! @brief Constructor: the timer is started.
  Timer();
  //! @brief Copy constructor. @param timer timer to copy
  Timer(const Timer & timer);
  //! @brief Destructor.
  virtual ~Timer() { }
  //! @brief Copy assignment. @param timer timer to copy @return this timer
  Timer & operator=(const Timer & timer);
  //! @brief Start (or restart) the timer: the start time and the time of the last stop are set to now.
  void start();
  //! @brief Record the current time: update the total duration (since start()) and the interval duration (since the
  //! previous stop() or start()), and their breakdowns in days, hours, minutes and seconds.
  void stop();
  //! @brief Print the breakdowns of the total and interval durations measured by the last stop().
  void print() const;

  high_resolution_clock::time_point startTime;  //!< time of the last start()
  high_resolution_clock::time_point stopTime;   //!< time of the last stop()
  high_resolution_clock::time_point oldStop;    //!< time of the stop() before the last one (or of start())
  duration<double> totalDuration;               //!< time from start() to the last stop(), in seconds
  duration<double> intervalDuration;            //!< time between the last two stop() calls, in seconds
  int    days;                                  //!< whole days of the total duration
  int    hours;                                 //!< hours (0-23) of the total duration
  int    minutes;                               //!< minutes (0-59) of the total duration
  double seconds;                               //!< seconds (below 60) of the total duration
  int    deltaDays;                             //!< whole days of the interval duration
  int    deltaHours;                            //!< hours (0-23) of the interval duration
  int    deltaMinutes;                          //!< minutes (0-59) of the interval duration
  double deltaSeconds;                          //!< seconds (below 60) of the interval duration

  ClassDef(Timer,0)
};

}



#endif /* CAP__Timer */
