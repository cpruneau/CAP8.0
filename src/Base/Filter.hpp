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
#ifndef CAP__Filter
#define CAP__Filter
#include "Exceptions.hpp"
#include "Object.hpp"
#include "Condition.hpp"
#include "PrintHelpers.hpp"

namespace CAP
{

class FilterException : public Exception
{
public:
  FilterException(const String & _message,
                  const String & _source);
  virtual ~FilterException() {}
};

template < class T >
class Filter
{
protected:

  std::vector<Condition*> _conditions;

public:

  Filter()
  :
  _conditions()
  { /* no ops */ }

  //!
  //! Deep copy: the filter owns its Condition* objects, so each one is duplicated
  //! polymorphically via Condition::clone() rather than sharing the source's pointers.
  //!
  Filter(const Filter & source)
  :
  _conditions(cloneConditions(source._conditions))
  { /* no ops */ }

  //!
  //! Deep-copy assignment. The new conditions are cloned before the old ones are released,
  //! so if a clone throws, this filter is left unchanged.
  //!
  Filter & operator=(const Filter & rhs)
  {
  if (this!=&rhs)
    {
    std::vector<Condition*> copies = cloneConditions(rhs._conditions);
    deleteConditions();
    _conditions.swap(copies);
    }
  return *this;
  }

  virtual ~Filter()
  {
  deleteConditions();
  }

  unsigned int nConditions() const {  return _conditions.size(); }
  std::vector<Condition*> & conditions() { return _conditions; }
  const std::vector<Condition*> & conditions() const { return _conditions; }

  //!
  //! Test whether the given object is acceptable based on this filter's criteria.
  //!  The base class throws: derived filters must override accept().
  //!
  virtual bool accept(const T & object __attribute__((unused))) const
  {
  throw FilterException("Improper call to accept of base class","Filter::accept(const T & object __attribute__((unused)))");
  }

  //! Test whether the given objects are acceptable based on this filter's criteria.
  //!  The base class throws: derived filters must override accept().
  virtual bool accept(const T & object1 __attribute__((unused)), const T & object2 __attribute__((unused))) const
  {
  throw FilterException("Improper call to accept of base class","Filter::accept(const T & object1 __attribute__((unused)), const T & object2 __attribute__((unused)))");
  }

//  virtual void print(  ) const
//  {
//  for (auto & condition : _conditions) condition->print();
//  }

  void addCondition(int _subtype,
                    bool _requested)
  {
  _conditions.push_back(new ConditionBool(_subtype,_requested));
  }

  void addCondition(int _subtype,
                    int _requested)
  {
  _conditions.push_back(new ConditionInteger(_subtype,_requested));
  }

  void addCondition(int _subtype,
                    int _minimum,
                    int _maximum)
  {
  _conditions.push_back(new ConditionIntegerRange(_subtype,_minimum,_maximum));
  }

  void addCondition(int _subtype,
                    double _minimum,
                    double _maximum)
  {
  _conditions.push_back(new ConditionDoubleRange(_subtype,_minimum,_maximum));
  }

  void addCondition(int _subtype,
                    double _minimum,
                    double _maximum,
                    double _minimum2,
                    double _maximum2)
  {
  _conditions.push_back(new ConditionDoubleDualRange(_subtype,_minimum,_maximum,_minimum2,_maximum2));
  }


protected:

  //!
  //! Clone every condition of the given list. If any clone throws, the copies already made are
  //! deleted before the exception propagates (no leak).
  //!
  static std::vector<Condition*> cloneConditions(const std::vector<Condition*> & source)
  {
  std::vector<Condition*> copies;
  copies.reserve(source.size());
  try
    {
    for (auto & condition : source) copies.push_back(condition ? condition->clone() : nullptr);
    }
  catch (...)
    {
    for (auto & copy : copies) delete copy;
    throw;
    }
  return copies;
  }

  void deleteConditions()
  {
  for (auto & condition : _conditions) delete condition;
  _conditions.clear();
  }

}; // Filter<T>

}// namespace CAP


#endif /* CAP__Filter  */

