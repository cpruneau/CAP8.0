/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ConfigurationManager.hpp"
#include "StreamHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Exceptions.hpp"
#include "XmlDocument.hpp"
#include <cstdlib>
#include <iomanip>

ClassImp(CAP::ConfigurationManager);

namespace CAP
{
ConfigurationManager::ConfigurationManager()
:
_configuration(),
_configurationPath("")
{
  // Note: setDefaultConfiguration() is virtual; calling it from this constructor could only
  // ever reach the no-op base version, never a derived override. Derived classes (e.g., Task)
  // are configured explicitly via setDefaultConfiguration()/configure() after construction.
}

ConfigurationManager::ConfigurationManager(const ConfigurationManager & source)
:
_configuration(source._configuration),
_configurationPath(source._configurationPath)
{  }

ConfigurationManager::~ConfigurationManager()
{  }

ConfigurationManager & ConfigurationManager::operator=(const ConfigurationManager & rhs)
{
  if (this!=&rhs)
    {
    _configuration     = rhs._configuration;
    _configurationPath = rhs._configurationPath;
    }
  return *this;
}


void ConfigurationManager::exportDictionary(const String & path, const String & name, bool verbose)
{
  setDefaultConfiguration();
  std::ofstream & outputFile = openOutputAsciiFile(path,name,".Dict",verbose);
  _configuration.saveTo(outputFile); // was _configuration.print(), which went to stdout and left the file empty
  outputFile.close();
}



void ConfigurationManager::print() const
{
  printCR();
  _configuration.print();
}

void ConfigurationManager::printProperty(const char * keyword) const
{
  const Property & property = _configuration.property(keyword);
  property.print();
}

void ConfigurationManager::generateKeyValuePairs(const char *  keyBaseName, const char *  defaultValue, int nKeysToGenerate)
{
  _configuration.generateKeyValuePairs(_configurationPath,keyBaseName,defaultValue,nKeysToGenerate);
}

void ConfigurationManager::generateKeyValuePairs(const char *  path, const char *  keyBaseName, const char *  defaultValue, int nKeysToGenerate)
{
  _configuration.generateKeyValuePairs(path,keyBaseName,defaultValue,nKeysToGenerate);
}


// ---------------------------------------------------------------- XML mapping

namespace
{
  //! Index of the property named exactly key, or -1.
  int exactIndex(const Configuration & configuration, const String & key)
  {
  const std::vector<Property> & properties = configuration.properties();
  for (unsigned int k=0; k<properties.size(); k++) if (properties[k].name()==key) return int(k);
  return -1;
  }

  //! Path components of a key (empty components are dropped).
  std::vector<String> keyComponents(const String & key)
  {
  std::vector<String> components;
  const std::string & s = key.str();
  std::size_t start = 0;
  while (start<=s.size())
    {
    std::size_t colon = s.find(':',start);
    if (colon==std::string::npos) colon = s.size();
    if (colon>start) components.push_back(String(s.substr(start,colon-start)));
    start = colon+1;
    }
  return components;
  }

  //! Child of an element with a given tag and name attribute, created if absent.
  XmlElement & namedChild(XmlElement & element, const String & tag, const String & name)
  {
  for (XmlElement * c : element.children())
    if (c->isNamed(tag) && c->attribute("name","")==name) return *c;
  XmlElement & c = element.addChild(tag);
  c.setAttribute("name",name);
  return c;
  }

  //! Value of a property as written to XML: booleans as true/false, numbers with the shortest exact representation.
  String xmlValue(const Property & property)
  {
  const String & v = property.value();
  switch (property.type())
    {
    case Property::kBool:
      return Property::isValidValue(v,Property::kBool) ? String(property.getBool() ? "true" : "false") : v;
    case Property::kDouble:
      return Property::isValidValue(v,Property::kDouble) ? XmlElement::formatNumber(std::strtod(v.data(),nullptr)) : v;
    default:
      return v;
    }
  }
}

String ConfigurationManager::xmlKeyPath() const
{
  String path = _configurationPath;
  while (path.endsWith(":")) path = path(0,path.length()-1);
  return path;
}

bool ConfigurationManager::relativeXmlKey(const String & key, String & relativeKey) const
{
  String path = xmlKeyPath();
  if (path.isNull()) { relativeKey = key; return true; }
  String prefix = path + ":";
  if (!key.beginsWith(prefix)) return false;
  relativeKey = key(prefix.length(),key.length()-prefix.length());
  return true;
}

String ConfigurationManager::absoluteXmlKey(const String & relativeKey) const
{
  String path = xmlKeyPath();
  return path.isNull() ? relativeKey : path + ":" + relativeKey;
}

bool ConfigurationManager::isXmlParameter(const String & /*relativeKey*/) const
{
  return true;
}

bool ConfigurationManager::readXmlChild(const XmlElement & /*child*/)
{
  return false;
}

void ConfigurationManager::unknownXmlParameter(const String & /*key*/)
{
  // no ops
}

XmlElement ConfigurationManager::createXmlElement() const
{
  XmlElement element("CONFIGURATION");
  String path = xmlKeyPath();
  if (!path.isNull()) element.setAttribute("name",path);
  return element;
}

void ConfigurationManager::fillXmlParameters(XmlElement & element, const Configuration & configuration) const
{
  for (const Property & property : configuration.properties())
    {
    String relativeKey;
    if (!relativeXmlKey(property.name(),relativeKey) || !isXmlParameter(relativeKey)) continue;
    std::vector<String> components = keyComponents(relativeKey);
    if (components.empty()) continue;
    XmlElement * parent = &element;
    for (unsigned int k=0; k+1<components.size(); k++) parent = &namedChild(*parent,"GROUP",components[k]);
    XmlElement & parameter = namedChild(*parent,"PARAMETER",components.back());
    parameter.setAttribute("type",property.typeName());
    if (!property.title().isNull() && property.title()!=property.name())
      parameter.setAttribute("description",property.title());
    parameter.setText(xmlValue(property));
    }
}

Configuration ConfigurationManager::defaultConfiguration()
{
  Configuration current = _configuration;
  _configuration.clear();
  try
    {
    setDefaultConfiguration();
    }
  catch (...)
    {
    _configuration = current;
    throw;
    }
  Configuration defaults = _configuration;
  _configuration = current;
  return defaults;
}

void ConfigurationManager::defineXmlElement(XmlElement & element)
{
  fillXmlParameters(element,defaultConfiguration());
}

void ConfigurationManager::fillXmlElement(XmlElement & element) const
{
  fillXmlParameters(element,_configuration);
}

void ConfigurationManager::readXmlParameters(const XmlElement & element, Configuration & configuration)
{
  readXmlParameters(element,"",configuration,true);
}

void ConfigurationManager::readXmlParameters(const XmlElement & element, const String & prefix,
                                             Configuration & configuration, bool topLevel)
{
  for (const XmlElement * child : element.children())
    {
    if (child->isNamed("GROUP"))
      {
      if (!child->hasAttribute("name"))
        throw ConfigurationException(absoluteXmlKey(prefix),"GROUP element without name attribute",__FUNCTION__);
      readXmlParameters(*child,prefix+child->attribute("name")+":",configuration,false);
      }
    else if (child->isNamed("PARAMETER"))
      {
      if (!child->hasAttribute("name"))
        throw ConfigurationException(absoluteXmlKey(prefix),"PARAMETER element without name attribute",__FUNCTION__);
      String key   = absoluteXmlKey(prefix+child->attribute("name"));
      String value = child->hasAttribute("value") ? child->attribute("value") : child->text();
      int index = exactIndex(configuration,key);
      Property::Type type = Property::kString;
      if (index>=0)
        type = configuration.properties()[index].type();
      else if (child->hasAttribute("type") && !Property::typeFromName(child->attribute("type"),type))
        throw ConfigurationException(key,String("Unknown type '")+child->attribute("type")+"'",__FUNCTION__);
      if (!Property::isValidValue(value,type))
        throw ConfigurationException(key,String("Invalid ")+Property::typeName(type)+" value '"+value+"'",__FUNCTION__);
      value = Property::normalizedValue(value,type);
      if (index>=0)
        {
        configuration.properties()[index].setValue(value);
        }
      else
        {
        Property property(key.data(),value);
        property.setType(type);
        if (child->hasAttribute("description")) property.setTitle(child->attribute("description"));
        configuration.addProperty(property);
        unknownXmlParameter(key);
        }
      }
    else if (!(topLevel && readXmlChild(*child)))
      {
      throw ConfigurationException(absoluteXmlKey(prefix),String("Unexpected element <")+child->name()+">",__FUNCTION__);
      }
    }
}

void ConfigurationManager::readXmlElement(const XmlElement & element)
{
  Configuration updated = _configuration;
  readXmlParameters(element,updated);
  configure(updated);
}

void ConfigurationManager::writeXml(const String & path, const String & name) const
{
  XmlDocument document("CAP");
  document.root().setAttribute("version",1);
  XmlElement & element = document.root().addChild(createXmlElement());
  fillXmlElement(element);
  document.write(path,name);
}

void ConfigurationManager::exportXmlDictionary(const String & path, const String & name)
{
  XmlDocument document("CAP");
  document.root().setAttribute("version",1);
  XmlElement & element = document.root().addChild(createXmlElement());
  defineXmlElement(element);
  document.write(path,name);
}

const XmlElement * ConfigurationManager::findXmlElement(const XmlElement & root) const
{
  XmlElement model = createXmlElement();
  String name = model.attribute("name","");
  if (root.isNamed(model.name()) && root.attribute("name","")==name) return &root;
  return name.isNull() ? root.findFirst(model.name()) : root.findFirst(model.name(),"name",name);
}

void ConfigurationManager::readXml(const String & path, const String & name)
{
  XmlDocument document = XmlDocument::read(path,name);
  const XmlElement * element = findXmlElement(document.root());
  if (!element)
    {
    XmlElement model = createXmlElement();
    throw ConfigurationException(xmlKeyPath(),String("No <")+model.name()+"> element named '"+model.attribute("name","")
                                 +"' in "+XmlDocument::fileName(path,name),__FUNCTION__);
    }
  readXmlElement(*element);
}

} // namespace CAP

