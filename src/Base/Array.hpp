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
#ifndef CAP__Array
#define CAP__Array
#include "Object.hpp"

//!
//! @class CAP::Array
//! @brief Array of objects of type T, enlarged as needed (work in progress).
//!
//! Intended as a simpler variant of Factory: a collection of objects of type T allocated once and reused from event to
//! event (reset() starts a new event), so that no time is spent creating and destroying objects.
//!
//! @warning This class is unfinished and does not compile (the declaration of the array and the variable last are
//! invalid). It is not used and not part of the Base library build. Use Factory instead.
//!
//! @tparam T type of the objects
//!

namespace CAP
{

template < class T >
class Array
{
protected:

  unsigned int defaultCapacity;  //!< number of objects allocated by initialize()
  unsigned int defaultSizeIncrement;  //!< number of objects added when the array is full
  T[] objects;  //!< objects

  public:

  //! @brief Default constructor: allocates the default capacity.
  Array()
  :
  defaultCapacity(10),
  defaultSizeIncrement(10),
  objects()
  {
    initialize();
  }

  //! @brief Destructor: see clear().
  virtual ~Array()
  {
  clear();
  }
  
  //! @brief Set the number of objects allocated by initialize(). @param _defaultCapacity number of objects
  void setDefaultCapacity(long _defaultCapacity)
  {
    defaultCapacity = _defaultCapacity;
  }

  //! @brief Set the number of objects added when the array is full. @param _defaultSizeIncrement number of objects
  void setDefaultSizeIncrement(long _defaultSizeIncrement)
  {
  defaultSizeIncrement = _defaultSizeIncrement;
  }

  //! @brief Allocate the default capacity.
  void initialize()
  {
  allocate(defaultCapacity);
  }

  //! @brief Allocate an array of objects. @param size number of objects @param initialValue value given to each object
  void allocate(long size, T initialValue)
  {
    objects = new T[size];
    for (long k=0;k<size;k++) objects[k] = initialValue;
  }

  //! @brief Start a new event: reset all objects.
  void reset()
  {
  for (auto & object : objects) object->reset();
  last = 0;
  }

  //! @brief Reset and delete all objects.
  void clear()
  {
  for (auto & object : objects)
    {
    object->reset();
    delete object;
    }
  objects.clear();
  }


  ClassDef(Array<T>,0)
};

} // namespace CAP

#endif /* Array_hpp */
