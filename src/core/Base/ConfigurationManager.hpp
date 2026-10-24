/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ConfigurationManager
#define CAP__ConfigurationManager
#include "Object.hpp"
#include "Configuration.hpp"

namespace CAP
{

class XmlElement;

//!
//! @brief Holder of a Configuration, with access to its parameters relative to a configuration path.
//!
//! ConfigurationManager is a base class of Task. The single-key functions (valueInt(key), addProperty(name,value), ...)
//! work on keys relative to the configuration path (configurationPath(), e.g. the task name): valueInt("nBins") reads
//! "<configurationPath>:nBins". The (path, key) overloads give the path explicitly.
//!
//! <b>XML mapping.</b> The parameters of the manager (those whose keys begin with the key path xmlKeyPath()) map to an
//! XML element (see XmlElement). Each colon-separated component of a key but the last becomes a GROUP element and the
//! last one a PARAMETER element holding the value and its type:
//! @code
//!   <CONFIGURATION name="Ana">                         <!-- createXmlElement() -->
//!     <PARAMETER name="nEvents" type="long">1000</PARAMETER>        <!-- Ana:nEvents -->
//!     <GROUP name="HISTOGRAM">
//!       <PARAMETER name="pt_nbins" type="int">100</PARAMETER>       <!-- Ana:HISTOGRAM:pt_nbins -->
//!     </GROUP>
//!   </CONFIGURATION>
//! @endcode
//! The dictionary is not fixed: each class defines its own part of it through setDefaultConfiguration(), which acts as
//! the template of its element.
//! - createXmlElement(): the empty element of the object;
//! - defineXmlElement(): fill it with the template, i.e., the default parameters and their types;
//! - fillXmlElement(): fill it with the current parameters;
//! - writeXml(), exportXmlDictionary(): write the filled or defined element to a file (root element CAP);
//! - readXmlElement(), readXml(): update the parameters from an element or a file, and configure() the object.
//!
//! Values read are checked against the type of the template parameter (a ConfigurationException is thrown for a value
//! of the wrong type). A parameter absent from the template is added (its type is then given by its "type" attribute,
//! string by default) and reported by unknownXmlParameter(). Derived classes can handle elements of their own by
//! overriding readXmlChild() (reading) and fillXmlElement() (writing).
//!
class ConfigurationManager
{

public:
  //! @brief Default constructor: empty configuration and configuration path.
  ConfigurationManager();
  //! @brief Copy constructor. @param source manager to copy
  ConfigurationManager(const ConfigurationManager & source);
  //! @brief Destructor.
  virtual ~ConfigurationManager();
  //! @brief Copy assignment. @param rhs manager to copy @return this manager
  ConfigurationManager & operator=(const ConfigurationManager & rhs);

  //!
  //! Returns a reference to a constant Configuration object controlling the behavior of this task instance.
  //! @return reference to the Configuration instance controlling this task instance.
  //!
  const Configuration & configuration() const
  {
  return _configuration;
  }

  //!
  //! Returns a reference to a Configuration object controlling the behavior of this task instance.
  //! @return reference to the Configuration instance controlling this task instance.
  //!
  Configuration & configuration()
  {
  return _configuration;
  }

  //! @brief Add the default parameters to the configuration (nothing here; derived classes override it).
  virtual void setDefaultConfiguration()
  {
  // no ops
  }

  //!
  //! Add all the parameters in the given requested configuration to the configuration of this manager.
  //!
  virtual void setConfiguration(const Configuration & reqConfiguration)
  {
  _configuration.addProperties(reqConfiguration);
  }

  //!
  //! Add all the parameters in the given requested configuration to the configuration of this manager.
  //!
  virtual void configure(const Configuration & reqConfiguration)
  {
  _configuration.addProperties(reqConfiguration);
  }

  //!
  //! @brief Write the default configuration (see setDefaultConfiguration()) to the file path/name.Dict, one
  //! "name : TYPE : value" line per parameter.
  //! @param path directory @param name file name (without extension) @param verbose print what is done
  //! @throws FileException if the file cannot be opened
  //!
  virtual void exportDictionary(const String & path, const String & name, bool verbose=true);

  //! @name XML mapping
  //! @{
  //! @brief Empty XML element of this object: <CONFIGURATION name="path"> here; Task makes <TASK name="" class="">.
  //! @return element
  virtual XmlElement createXmlElement() const;
  //!
  //! @brief Fill an element with the template of the configuration: the parameters added by setDefaultConfiguration()
  //! with their default values and types. The current configuration is left unchanged.
  //! @param element element made by createXmlElement()
  //!
  virtual void defineXmlElement(XmlElement & element);
  //! @brief Fill an element with the current parameters. @param element element made by createXmlElement()
  virtual void fillXmlElement(XmlElement & element) const;
  //!
  //! @brief Update the parameters from an element and configure this object with the result (configure() receives the
  //! current configuration updated with the values read).
  //! @param element element of this object (as written by fillXmlElement())
  //! @throws ConfigurationException for a value of the wrong type, an unknown type or an unexpected element
  //!
  virtual void readXmlElement(const XmlElement & element);
  //! @brief Write the current parameters to the XML file path/name(.xml), as an element of the root element CAP.
  //! @param path directory @param name file name @throws FileException if the file cannot be written
  virtual void writeXml(const String & path, const String & name) const;
  //! @brief Write the template of the configuration (see defineXmlElement()) to the XML file path/name(.xml).
  //! @param path directory @param name file name @throws FileException if the file cannot be written
  virtual void exportXmlDictionary(const String & path, const String & name);
  //!
  //! @brief Read the XML file path/name(.xml) and update the parameters from the element of this object (the root
  //! element or its first descendant with the tag and name attribute of createXmlElement()).
  //! @param path directory @param name file name
  //! @throws FileException, XmlException if the file cannot be read; ConfigurationException if it has no element
  //! of this object (see also readXmlElement())
  //!
  virtual void readXml(const String & path, const String & name);
  //! @brief Element of this object in a tree (see readXml()). @param root root element @return element, or nullptr
  const XmlElement * findXmlElement(const XmlElement & root) const;
  //! @}

  //!
  //! Get the bool value of the given parameter named key.
  //!
  bool valueBool(const char *  key)   const
  {
  return _configuration.valueBool(_configurationPath,key);
  }

  //!
  //! Get the int value of the given parameter named key.
  //!
  int valueInt(const char *  key)    const
  {
  return _configuration.valueInt(_configurationPath,key);
  }

  //!
  //! Get the long value of the given parameter named key.
  //!
  long valueLong(const char *  key)    const
  {
  return _configuration.valueLong(_configurationPath,key);
  }

  //!
  //! Get the double value of the given parameter named key.
  //!
  double valueDouble(const char *  key) const
  {
  return _configuration.valueDouble(_configurationPath,key);
  }

  //!
  //! Get the String value of the given parameter named key.
  //!
  String valueString(const char *  key) const
  {
  return _configuration.valueString(_configurationPath,key);
  }

  //!
  //! Get the bool value of the given parameter named key.
  //!
  bool valueBool(const char * path, const char *  key)   const
  {
  return _configuration.valueBool(path,key);
  }

  //!
  //! Get the int value of the given parameter named key at the given path.
  //!
  int valueInt(const char * path, const char *  key)    const
  {
  return _configuration.valueInt(path,key);
  }

  //!
  //! Get the long value of the given parameter named key at the given path.
  //!
  long valueLong(const char * path, const char *  key)    const
  {
  return _configuration.valueLong(path,key);
  }

  //!
  //! Get the double value of the given parameter named key at the given path.
  //!
  double valueDouble(const char * path, const char *  key) const
  {
  return _configuration.valueDouble(path,key);
  }

  //!
  //! Get the String value of the given parameter named key at the given path.
  //!
  String valueString(const char * path, const char *  key) const
  {
  return _configuration.valueString(path,key);
  }

  //!
  //! Add a property with the given name and value
  //!
  void addProperty(const char *  name, bool value)
  {
  _configuration.addProperty(_configurationPath,name,value);
  }

  //!
  //! Add a property with the given name and value
  //!
  void addProperty(const char *  name, int value)
  {
  _configuration.addProperty(_configurationPath,name,value);
  }

  //!
  //! Add a property with the given name and value
  //!
  void addProperty(const char *  name, long value)
  {
  _configuration.addProperty(_configurationPath,name,value);
  }

  //!
  //! Add a property with the given name and value
  //!
  void addProperty(const char *  name, double value)
  {
  _configuration.addProperty(_configurationPath,name,value);
  }

  //!
  //! Add a property with the given name and value
  //!
  void addProperty(const char *  name, const char *  value)
  {
  _configuration.addProperty(_configurationPath,name,value);
  }

  //!
  //! Add a property with the given name and value
  //!
  void addProperty(const char * path, const char * name, bool value)
  {
  _configuration.addProperty(path,name,value);
  }

  //!
  //! Add a property with the given name and value
  //!
  void addProperty(const char * path, const char * name, int value)
  {
  _configuration.addProperty(path,name,value);
  }

  //!
  //! Add a property with the given name and value
  //!
  void addProperty(const char * path, const char * name, long value)
  {
  _configuration.addProperty(path,name,value);
  }

  //!
  //! Add a property with the given name and value
  //!
  void addProperty(const char * path, const char * name, double value)
  {
  _configuration.addProperty(path,name,value);
  }

  //!
  //! Add a property with the given name and value
  //!
  void addProperty(const char * path, const char * name, const char * value)
  {
  _configuration.addProperty(path,name,value);
  }

  //!
  //! print the properties held by this configuration manager.
  //!
  virtual void print() const;

  //! @brief Print one parameter. @param keyword key of the parameter @throws PropertyException if not found
  void printProperty(const char * keyword) const;

  //!
  //!Generates and stores in the configuration of this task a list of key,value properties based on the given properties.value
  //!The keys generated have the form keyBaseName## where keyBaseName is the base name of the keys and ## is an integer k
  //!from 0 to nKeysToGenerate-1 (inclusively)
  //!
  //!@param keyBaseName base name of the keys
  //!@param defaultValue default value given for all key,value pairs
  //!@param nKeysToGenerate number of key,value pairs to generate and add to this task configuration.
  //!
  void generateKeyValuePairs(const char *  keyBaseName, const char *  defaultValue, int nKeysToGenerate);

  //! @brief Same as generateKeyValuePairs(keyBaseName,defaultValue,nKeysToGenerate), with an explicit path.
  //! @param path path of the keys @param keyBaseName base name of the keys @param defaultValue value of all pairs
  //! @param nKeysToGenerate number of pairs
  void generateKeyValuePairs(const char *  path, const char *  keyBaseName, const char *  defaultValue, int nKeysToGenerate);

  //!
  //!Parse the configuration associated with task to find the key,value pairs that DO NOT feature the defaultValue
  //!
  //!@param keyBaseName base name of the keys
  //!@param defaultValue default value that is selected against
  //!
  std::vector<String>  selectedValues(const char *  keyBaseName, const char *  defaultValue) const
  {  return _configuration.selectedValues(_configurationPath,keyBaseName,defaultValue); }

  //! @brief Same as selectedValues(keyBaseName,defaultValue), with an explicit path.
  //! @param path path of the keys @param keyBaseName base name of the keys @param defaultValue value selected against
  //! @return selected values
  std::vector<String>  selectedValues(const char *  path, const char *  keyBaseName, const char *  defaultValue) const
  { return _configuration.selectedValues(path,keyBaseName,defaultValue);}

  //!
  //!Get  the number of key,value pairs with the given key base name  that DO NOT feature the defaultValue
  //!
  //!@param keyBaseName base name of the keys to be tested against.
  //!@param defaultValue default value that is selected against
  //!
  int nSelectedValues(const char *  keyBaseName, const char *  defaultValue)  const {return _configuration.nSelectedValues(_configurationPath,keyBaseName,defaultValue); }
  //! @brief Same as nSelectedValues(keyBaseName,defaultValue), with an explicit path.
  //! @param path path of the keys @param keyBaseName base name of the keys @param defaultValue value selected against
  //! @return number of selected values
  int nSelectedValues(const char *  path, const char *  keyBaseName, const char *  defaultValue) const {  return _configuration.nSelectedValues(path,keyBaseName,defaultValue); }
  //! @brief Set the path prepended to the keys of the single-key functions. @param configurationPath path
  void setConfigurationPath(const String & configurationPath) {  _configurationPath = configurationPath; }
  //! @brief Path prepended to the keys of the single-key functions. @return path
  const String & configurationPath() const  {  return _configurationPath; }

protected:

  //! @name XML mapping helpers
  //! @{
  //!
  //! @brief Default configuration of this object: the parameters added by setDefaultConfiguration() to an empty
  //! configuration. The current configuration is left unchanged (it is restored if setDefaultConfiguration() throws).
  //! Note: classes whose setDefaultConfiguration() takes defaults from data members return the current values of
  //! those members, i.e., the defaults are the true defaults only for an object that has not been configured.
  //! @return default configuration
  //!
  Configuration defaultConfiguration();
  //! @brief Path of the keys mapped to XML (without end colon): the configuration path here, the task name for a Task.
  //! @return path
  virtual String xmlKeyPath() const;
  //! @brief Key relative to xmlKeyPath(). @param key full key @param relativeKey relative key
  //! @return false if the key is not under xmlKeyPath()
  bool relativeXmlKey(const String & key, String & relativeKey) const;
  //! @brief Full key of a key relative to xmlKeyPath(). @param relativeKey relative key @return key
  String absoluteXmlKey(const String & relativeKey) const;
  //! @brief Whether a parameter is written as a PARAMETER element (all here; Task excludes those mapped to
  //! attributes and subtask elements). @param relativeKey key relative to xmlKeyPath() @return true if so
  virtual bool isXmlParameter(const String & relativeKey) const;
  //! @brief Handle a child element other than GROUP and PARAMETER while reading. @param child element
  //! @return true if handled (nothing is handled here: an exception is then thrown)
  virtual bool readXmlChild(const XmlElement & child);
  //! @brief Called for each parameter read that is not in the template (nothing here; Task reports a warning).
  //! @param key full key of the parameter
  virtual void unknownXmlParameter(const String & key);
  //! @brief Add the parameters of a configuration as GROUP and PARAMETER elements. @param element element
  //! @param configuration parameters
  void fillXmlParameters(XmlElement & element, const Configuration & configuration) const;
  //! @brief Update a configuration with the GROUP and PARAMETER children of an element (see readXmlElement()).
  //! @param element element @param configuration configuration updated
  void readXmlParameters(const XmlElement & element, Configuration & configuration);
  //! @brief Recursive part of readXmlParameters(). @param element element @param prefix relative path of the element
  //! @param configuration configuration updated @param topLevel whether the element is the object's element
  void readXmlParameters(const XmlElement & element, const String & prefix, Configuration & configuration, bool topLevel);
  //! @}

  Configuration _configuration;  //!< parameters
  String _configurationPath;  //!< path prepended to the keys of the single-key functions
};

} // namespace CAP

#endif /* ConfigurationManager_hpp */
