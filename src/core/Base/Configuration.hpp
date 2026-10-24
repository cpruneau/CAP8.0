/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__Configuration
#define CAP__Configuration
#include "Properties.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Set of properties configuring tasks (see Task and ConfigurationManager).
//!
//! Configuration adds to Properties the handling of numbered keys: lists of values stored under keys keyBaseName0,
//! keyBaseName1, ... For instance, the particle types analyzed by a task can be listed as "Ana:Particle0",
//! "Ana:Particle1", ..., unused slots holding a default value such as "none".
//!
//! Example:
//! @code
//!   CAP::Configuration c;
//!   c.generateKeyValuePairs("Ana","Particle","none",4);   // Ana:Particle0 ... Ana:Particle3 = none
//!   c.addProperty("Ana:Particle0","pi+");
//!   std::vector<String> used = c.selectedValues("Ana","Particle","none");   // {"pi+"}
//! @endcode
//!
//! @note Keys are matched with Object::nameContains(): a key base name also selects keys of which it is a part.
//!
class Configuration : public Properties
{
 public:
  //! @brief Default constructor: no properties.
  Configuration();
  //! @brief Copy constructor. @param _configuration configuration to copy
  Configuration(const Configuration & _configuration);
  //! @brief Destructor.
  virtual ~Configuration() {}
  //! @brief Copy assignment. @param _configuration configuration to copy @return this configuration
  Configuration & operator=(const Configuration & _configuration);

 
  //!
  //!Generates and stores in the configuration of this task a list of key,value properties based on the given properties.value
  //!The keys generated have the form keyBaseName## where keyBaseName is the base name of the keys and ## is an integer k
  //!from 0 to nKeysToGenerate-1 (inclusively)
  //!
  //!@param keyBaseName base name of the keys
  //!@param defaultValue default value given for all key,value pairs
  //!@param nKeysToGenerate number of key,value pairs to generate and add to this task configuration.
  //!
  virtual void generateKeyValuePairs(const char *  keyBaseName, const char *  defaultValue, int nKeysToGenerate);

  //! @brief Same as generateKeyValuePairs(keyBaseName,defaultValue,nKeysToGenerate) with keys "path:keyBaseName##".
  //! @param path path of the keys @param keyBaseName base name of the keys @param defaultValue value of all pairs
  //! @param nKeysToGenerate number of pairs
  virtual void generateKeyValuePairs(const char *  path, const char *  keyBaseName, const char *  defaultValue, int nKeysToGenerate);

  //!
  //!Parse the configuration associated with task to find the key,value pairs that DO NOT feature the defaultValue
  //!
  //!@param keyBaseName base name of the keys
  //!@param defaultValue default value that is selected against
  //!@return values of the matching keys whose value does not contain defaultValue
  //!
  virtual std::vector<String>  selectedValues(const char *  keyBaseName, const char *  defaultValue) const;

  //! @brief Same as selectedValues(keyBaseName,defaultValue) with key base name "path:keyBaseName".
  //! @param path path of the keys @param keyBaseName base name of the keys @param defaultValue value selected against
  //! @return selected values
  virtual std::vector<String>  selectedValues(const char *  path, const char *  keyBaseName, const char *  defaultValue) const;

  //!
  //!Get  the number of key,value pairs with the given key base name  that DO NOT feature the defaultValue
  //!
  //!@param keyBaseName base name of the keys to be tested against.
  //!@param defaultValue default value that is selected against
  //!@return number of selected values
  //!
  virtual int nSelectedValues(const char *  keyBaseName, const char *  defaultValue)  const;

  //! @brief Same as nSelectedValues(keyBaseName,defaultValue) with key base name standardize(path,keyBaseName).
  //! @param path path of the keys @param keyBaseName base name of the keys @param defaultValue value selected against
  //! @return number of selected values
  virtual int nSelectedValues(const char *  path, const char *  keyBaseName, const char *  defaultValue) const
  {
  return nSelectedValues(standardize(path,keyBaseName), defaultValue);
  }

  //! @brief Number of keys containing a key base name, whatever their value. @param keyBaseName base name of the keys
  //! @return number of keys
  virtual int nPossibleValues(const char *  keyBaseName) const;

  //! @brief Same as nPossibleValues(keyBaseName) with key base name "path:keyBaseName".
  //! @param path path of the keys @param keyBaseName base name of the keys @return number of keys
  virtual int nPossibleValues(const char *  path, const char *  keyBaseName) const;

  //!
  //!Add key,value pairs with the given key base name  and values provided in the selectedValues array.
  //!The new keys are numbered after the nSelectedValues(keyBaseName,defaultValue) keys already selected.
  //!
  //!@param keyBaseName base name of the keys to be used
  //!@param defaultValue default value that is selected against
  //!@param selectedValues  values to add
  //!
  virtual void addSelectedValues(const char *  keyBaseName, const char *  defaultValue, const std::vector<String> & selectedValues);

  //! @brief Same as addSelectedValues(keyBaseName,defaultValue,selectedValues) with key base name
  //! standardize(path,keyBaseName).
  //! @param path path of the keys @param keyBaseName base name of the keys @param defaultValue value selected against
  //! @param selectedValues values to add
  virtual void addSelectedValues(const char * path, const char * keyBaseName, const char * defaultValue, const std::vector<String> & selectedValues)
  {
  addSelectedValues(standardize(path,keyBaseName), defaultValue, selectedValues);
  }

protected:
  ClassDef(Configuration,0)
  
};

} // namespace CAP

#endif /* Configuration_hpp */
