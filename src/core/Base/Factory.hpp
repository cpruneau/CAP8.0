/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAPFactory
#define CAPFactory
#include "Exceptions.hpp"
#include "ClassDef.hpp"

//!
//! @class CAP::Factory
//! @brief Pool of reusable objects of type T.
//!
//! Generic base class for factory services.
//! A factory can be used to generate and own a large collection of objects of the same type "T". Objects are allocated on the heap and pointers "T*" to these
//! objects kept in a large array T ** objects. The array is nominally of fixed size but is enlarged whenever more objects are needed. The factory model is useful
//! when objects need to be repeatedly used and discarded (e.g., in data analysis of particles) because no malloc or destroy is required. The memory remains allocated
//! and so there is no time wasted creating and destroying the objects. The reset method is to be called on a factory to indicate that a new "event" is being considered.
//! It must evidently be possible to initialize the objects with "set" methods specific to the class "T". Use the  T * nextObject() method to obtain an used object.
//!
//! @tparam T type of the objects; it must have a default constructor
//!

namespace CAP
{

template < class T >
class Factory
{
protected:

  long _currentSize;  //!< current number of objects
  long _defaultSize;  //!< number of objects created by initialize()
  long _defaultSizeIncrement;  //!< number of objects added when the pool is exhausted
  long _nextServed;  //!< index of the next object served
  bool _owner;  //!< whether the factory deletes the objects
  T ** _objects;  //!< array of pointers to the objects

  public:

  //! @brief Constructor. The objects are created by initialize() (or by the first nextObject()).
  //! @param defaultSize number of objects created by initialize() @param defaultSizeIncrement number of objects added
  //! when the pool is exhausted @param owner whether the factory deletes the objects
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

  //! @brief Destructor: see clear().
  virtual ~Factory()
  {
  clear();
  }

  //! @brief Set the number of objects created by initialize(). @param defaultSize number of objects
  void setDefaultSize(long defaultSize)
  {
  _defaultSize = defaultSize;
  }

  //! @brief Number of objects created by initialize(). @return number of objects
  long defaultSize() const
  {
  return _defaultSize;
  }

  //! @brief Set the number of objects added when the pool is exhausted. @param defaultSizeIncrement number of objects
  void setDefaultSizeIncrement(long defaultSizeIncrement)
  {
  _defaultSizeIncrement = defaultSizeIncrement;
  }

  //! @brief Number of objects added when the pool is exhausted. @return number of objects
  long defaultSizeIncrement() const
  {
  return _defaultSizeIncrement;
  }

  //! @brief Set whether the factory deletes its objects. @param owner true to delete them in clear()
  void setOwner(bool owner)
  {
  _owner = owner;
  }

  //! @brief Number of objects in the pool (served or not). @return size
  long size() const
  {
    return _currentSize;
  }

  //! @brief Create defaultSize() new objects (with T's default constructor), releasing any previous ones (see clear()).
  void initialize()
  {
  if (_objects) clear();
  _objects = new T*[_defaultSize];
  for (long k=0; k<_defaultSize; k++ ) _objects[k] = new T();
  _currentSize = _defaultSize;
  _nextServed = 0;
  }

  //! @brief Add new objects to the pool. @param sizeIncrement number of objects to add
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

  //! @brief Start a new event: the next nextObject() serves the first object again. The objects are not modified.
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

  //! @brief Object of the pool at an index. @param index index @return object @throws IndexOutOfBoundException if index>=size()
  T & objectAt(unsigned int index)
  {
  if ((long)index>=_currentSize) throw IndexOutOfBoundException(index,_currentSize,"index>=size()",__FUNCTION__);
  return *(_objects[index]);
  }

  //! @brief Next unused object of the pool, which is enlarged if all objects are in use. The object keeps the values set
  //! when it was last served: set them all. @return object
  T & nextObject()
  {
  if (_nextServed>=_currentSize) increaseSize(_defaultSizeIncrement>0 ? _defaultSizeIncrement : 1);
  return *(_objects[_nextServed++]);
  }

};

} // namespace CAP

#endif /* Factoryhpp */
