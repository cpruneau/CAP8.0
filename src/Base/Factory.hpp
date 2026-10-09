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
#ifndef CAPFactory
#define CAPFactory
#include "Exceptions.hpp"

//!
//! Factory
//!
//! Generic base class for factory services.
//! A factory can be used to generate and own a large collection of objects of the same type "T". Objects are allocated on the heap and pointers "T*" to these
//! objects kept in a large array T ** objects. The array is nominally of fixed size but is enlarged whenever more objects are needed. The factory model is useful
//! when objects need to be repeatedly used and discarded (e.g., in data analysis of particles) because no malloc or destroy is required. The memory remains allocated
//! and so there is no time wasted creating and destroying the objects. The reset method is to be called on a factory to indicate that a new "event" is being considered.
//! It must evidently be possible to initialize the objects with "set" methods specific to the class "T". Use the  T * nextObject() method to obtain an used object.
//!

namespace CAP
{

template < class T >
class Factory
{
protected:

  long _currentSize;
  long _defaultSize;
  long _defaultSizeIncrement;
  long _nextServed;
  bool _owner;
  T ** _objects;

  public:

  Factory(long defaultSize=5000, long defaultSizeIncrement=1000, bool owner=true)
  :
  _currentSize(0),
  _defaultSize(defaultSize),
  _defaultSizeIncrement(defaultSizeIncrement),
  _nextServed(0),
  _owner(owner),
  _objects(nullptr)
  {   }

  //!
  //! The factory owns raw T* allocations: copying it would share (and later double-delete) them.
  //!
  Factory(const Factory &) = delete;
  Factory & operator=(const Factory &) = delete;

  virtual ~Factory()
  {
  clear();
  }

  void setDefaultSize(long defaultSize)
  {
  _defaultSize = defaultSize;
  }

  long defaultSize() const
  {
  return _defaultSize;
  }

  void setDefaultSizeIncrement(long defaultSizeIncrement)
  {
  _defaultSizeIncrement = defaultSizeIncrement;
  }

  long defaultSizeIncrement() const
  {
  return _defaultSizeIncrement;
  }

  void setOwner(bool owner)
  {
  _owner = owner;
  }

  long size() const
  {
    return _currentSize;
  }

  void initialize()
  {
  if (_objects) clear();
  _objects = new T*[_defaultSize];
  for (long k=0; k<_defaultSize; k++ ) _objects[k] = new T();
  _currentSize = _defaultSize;
  _nextServed = 0;
  }

  void increaseSize(long sizeIncrement)
  {
  T** hold = _objects;
  long newSize = _currentSize + sizeIncrement;
  _objects = new T*[newSize];
  for (long k=0; k<_currentSize; k++) _objects[k] = hold[k];
  for (long k=_currentSize; k<newSize; k++) _objects[k] = new T();
  delete[] hold;
  _currentSize = newSize;
  }

  void reset()
  {
  _nextServed = 0;
  }

  //!
  //! Release the objects (if owned) and the pointer array, and return the factory to its
  //! empty state so that a later clear() -- e.g., from the destructor -- is a no-op.
  //! Never throws: it is called from the destructor.
  //!
  void clear()
  {
  if (_objects)
    {
    if (_owner)
      {
      for (long k=0; k<_currentSize; k++) delete _objects[k];
      }
    delete[] _objects;
    }
  _objects     = nullptr;
  _currentSize = 0;
  _nextServed  = 0;
  }

  T & objectAt(unsigned int index)
  {
  if ((long)index>=_currentSize) throw IndexOutOfBoundException(index,_currentSize,"index>=size()",__FUNCTION__);
  return *(_objects[index]);
  }

  T & nextObject()
  {
  if (_nextServed>=_currentSize) increaseSize(_defaultSizeIncrement>0 ? _defaultSizeIncrement : 1);
  return *(_objects[_nextServed++]);
  }

  ClassDef(Factory<T>,0)
};

} // namespace CAP

#endif /* Factoryhpp */
