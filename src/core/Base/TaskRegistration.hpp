/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TaskRegistration.hpp
//! @brief Registration of the task classes in the reflection registry, for Task::instantiateTask() (kept out of the
//! ROOT dictionaries).
#ifndef CAP__TaskRegistration
#define CAP__TaskRegistration
#include "Reflection.hpp"
#include "Task.hpp"

namespace CAP
{

  //!
  //! @brief Register a task class, so that workflows can create its tasks by class name (Task::instantiateTask())
  //! and Task::taskClassNames() lists it. Called once per class, in a CAP_REFLECTION block of its library:
  //! @code
  //!   CAP_REFLECTION(ParticlesTasks)
  //!   {
  //!   CAP::registerTask<CAP::EventIterator>("CAP::EventIterator");
  //!   }
  //! @endcode
  //! @tparam T task class (deriving from Task, with a public default constructor) @param name full class name
  //!
  template <typename T>
  void registerTask(const char * name)
  {
  static_assert(std::is_base_of<Task, T>::value, "not a task class");
  REFLECTION::ClassBuilder<T> builder(name);
  if constexpr (!std::is_same<T, Task>::value) builder.template base<Task>();
  builder.template constructor<>();
  }

} // namespace CAP

#endif // !CAP__TaskRegistration
