/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ManagedObjects
#define CAP__ManagedObjects
#include <vector>
#include "String.hpp"
#include "FileSystem.hpp"
#include "Exceptions.hpp"
#include "Configuration.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"
#include "StreamHelpers.hpp"
#include "CapFile.hpp"
#include "Object.hpp"
#include "EnvironmentVariables.hpp"
#include "Object.hpp"
#include "ManagedObject.hpp"
#include <memory>
#include <type_traits>
#include <utility>
#include <iostream>

namespace CAP
{

//! @brief Whether a type has a member function loadFromXml(const String &) (used by ManagedObjects::loadFromXml()).
template <typename T, typename = void>
struct HasLoadFromXml : std::false_type {};
//! @brief Whether a type has a member function loadFromXml(const String &) (specialization: it has).
template <typename T>
struct HasLoadFromXml<T, std::void_t<decltype(std::declval<T&>().loadFromXml(std::declval<const String &>()))>> : std::true_type {};
  
  //!
  //! @brief List of managed objects (events, filters, histogram groups, particle databases, ...) of one type, used by a
  //! task: configuration, creation or import at initialization, export at finalization.
  //!
  //! Configuration (keys added by setDefaultObjectConfiguration(), read by configureObjects()), for task \<task\> and
  //! object type \<TYPE\> (e.g. PARTICLE_FILTER, HISTOGRAM_1):
  //! @code
  //!   <task>:<TYPE>:USE, N, IMPORT, CREATE                            // flags and number of objects
  //!   <task>:<TYPE>:EXPORT                                            // not for HISTOGRAM types
  //!   <task>:<TYPE>:IMPORT:BASE_NAME, IMPORT:PATH, IMPORT:FILE_NAME   // not for HISTOGRAM types
  //!   <task>:<TYPE>:EXPORT:BASE_NAME, EXPORT:PATH, EXPORT:FILE_NAME, EXPORT:FORCE_REWRITE   // not for HISTOGRAM types
  //!   <task>:<TYPE>_<k>:NAME, TITLE, OWNER                            // object k
  //!   <task>:<TYPE>_<k>:<param>                                       // histogram folder k: its own binning
  //! @endcode
  //! The TITLE of an object, unless empty or NONE, is set after its configuration. A histogram folder whose keys include
  //! binning parameters of its own uses them instead of those of the HISTOGRAM group of the task (see
  //! objectConfiguration()).
  //! An object whose OWNER flag is true is created (and configured) by this list and added to a static store shared by
  //! all lists of this type; otherwise the object of that name is taken from the store (shared between tasks).
  //!
  //! Histograms (HISTOGRAM types) have no files of their own: the job writes those of all its tasks to one output file,
  //! a folder per task and a sub-folder per object (saveToFolders()), and imports them (IMPORT) from one input file in
  //! the same way (loadFromFolders(); see CAP.cxx). Other types: for PARTICLE_DB, relative import paths are taken in
  //! $CAP_DATABASE_PATH.
  //! Files ending in .cap (or .root, read as .cap) are CAP files; a PARTICLE_DB is read from an XML file (.xml) holding
  //! the particles and their decay modes (see ParticleDbXml).
    //!
  //! @tparam MANAGED type of the managed objects (a ManagedObject)
  //!
  template <class MANAGED>
  class ManagedObjects
  :
  public Object
  {
  protected:
  
  //! Whether the list owns its objects.
  bool   _owner;
  //! Whether the objects are used (USE).
  bool   _use;
  //! Whether the objects are imported at initialization (IMPORT).
  bool   _import;
  //! Whether the objects are created at initialization (CREATE).
  bool   _create;
  //! Whether the objects are exported at finalization (EXPORT).
  bool   _export;
  //! Whether the objects are scaled.
  bool   _scale;
  //! Whether an existing export file is overwritten (EXPORT:FORCE_REWRITE).
  bool   _forceRewrite;
  //! Import base name.
  String _importBaseName;
  //! Import directory.
  String _importPath;
  //! Import file name.
  String _importFileName;
  //! Export base name.
  String _exportBaseName;
  //! Export directory.
  String _exportPath;
  //! Export file name.
  String _exportFileName;
  //! Name of the task using the list.
  String _taskName;
  //! Name of the owner.
  String _ownerName;
  //! Object type (prefix of the configuration keys).
  String _typeName;
  //! Objects of the list.
  std::vector<MANAGED*> _objects;
  
  //! Store of the objects of this type created by all lists (shared between tasks).
  static std::vector<MANAGED*> * objectStore;
  
  public:
  
  //! @brief Default constructor: empty list.
  ManagedObjects()
  :
  _owner(false),
  _use(false),
  _import(false),
  _create(false),
  _export(false),
  _scale(false),
  _forceRewrite(true),
  _importBaseName("BASE"),
  _importPath("DEFAULT"),
  _importFileName("NONE"),
  _exportBaseName("BASE"),
  _exportPath("DEFAULT"),
  _exportFileName("NONE"),
  _taskName("UNKNOWN"),
  _ownerName("UNKNOWN"),
  _typeName("UNKNOWN"),
  _objects()
  {
  setClassName("ManagedObjects");
  setName("UNKNOWN:UNKNOWN");       // <task>:<TYPE> once known (see updateName())
  setMinimumReportLevel(Object::kInfo);
  }
  
  //  ManagedObjects(const ManagedObjects & manager)
  //  ManagedObjects & operator=(const ManagedObjects & manager);
  //! @brief Destructor.
  virtual ~ManagedObjects()
  {
  clear();
  }
  
  //! @brief Set the task and object type and add the default configuration of the list (see the class description).
  //! @param taskName name of the task @param typeName object type @param configuration configuration completed
  void setDefaultObjectConfiguration(const String & taskName,
                                     const String & typeName,
                                     Configuration & configuration)
  {
  _taskName = taskName;
  _typeName = typeName;
  updateName();
  if (reportStart(__FUNCTION__)) { printCR(); }
  configuration.addProperty(createKey(_taskName,_typeName,"USE"),    false);
  configuration.addProperty(createKey(_taskName,_typeName,"N"),      0);
  configuration.addProperty(createKey(_taskName,_typeName,"IMPORT"), false);
  configuration.addProperty(createKey(_taskName,_typeName,"CREATE"), false);
  if (isHistogramList()) return;    // histograms: read from and written to the files of the job (see CAP.cxx)
  configuration.addProperty(createKey(_taskName,_typeName,"EXPORT"), false);
  configuration.addProperty(createKey(_taskName,_typeName,"IMPORT:BASE_NAME"),"NONE");
  configuration.addProperty(createKey(_taskName,_typeName,"IMPORT:PATH"),"DEFAULT");
  configuration.addProperty(createKey(_taskName,_typeName,"IMPORT:FILE_NAME"),"NONE");
  configuration.addProperty(createKey(_taskName,_typeName,"EXPORT:BASE_NAME"),"NONE");
  configuration.addProperty(createKey(_taskName,_typeName,"EXPORT:PATH"),"DEFAULT");
  configuration.addProperty(createKey(_taskName,_typeName,"EXPORT:FILE_NAME"),"NONE");
  configuration.addProperty(createKey(_taskName,_typeName,"EXPORT:FORCE_REWRITE"),true);
  }
  
  
  //!
  //! @brief Whether a file name designates a CAP file: names ending with ".cap", and names ending with ".root" (from
  //! configurations written for the ROOT version, read and written as CAP files, see capFileName()).
  //! @param fileName file name @return true for a CAP file
  //!
  static bool isCapFileName(const String & fileName)
  {
  return fileName.endsWith(".cap") || fileName.endsWith(".root");
  }

  //!
  //! @brief Name of the CAP file used for a configured file name: a ".root" extension is replaced by ".cap".
  //! @param fileName configured file name @return CAP file name
  //!
  static String capFileName(const String & fileName)
  {
  if (!fileName.endsWith(".root")) return fileName;
  String name(fileName.data(),fileName.length()-5);
  name += ".cap";
  return name;
  }

  //! @brief Set whether the list owns its objects. @param v flag
  virtual void setOwner(bool v)  { _owner = v; }
  //! @brief Set whether the objects are imported. @param option flag
  virtual void setImport(bool option) { _import = option; }
  //! @brief Set whether the objects are created. @param option flag
  virtual void setCreate(bool option) { _create = option; }
  //! @brief Set whether the objects are exported. @param option flag
  virtual void setExport(bool option) { _export = option; }
  //! @brief Set whether an existing export file is overwritten. @param option flag
  virtual void setForceRewrite(bool option) { _forceRewrite = option; }
  //! @brief Whether the list owns its objects. @return flag
  virtual bool isOwner() const   { return _owner; }
  //! @brief Whether the objects are imported. @return flag
  virtual bool imports() const { return _import; }
  //! @brief Whether the objects are created. @return flag
  virtual bool create() const { return _create; }
  //! @brief Whether the objects are exported. @return flag
  virtual bool exports() const { return _export; }
  //! @brief Whether an existing export file is overwritten. @return flag
  virtual bool forceRewrite() const { return _forceRewrite; }
  //! @brief Set the import base name. @param option name
  virtual void setImportBaseName(const String & option) { _importBaseName = option;}
  //! @brief Set the import directory. @param option directory
  virtual void setImportPath(const String & option)     { _importPath = option;}
  //! @brief Set the import file name. @param option file name
  virtual void setImportFileName(const String & option) { _importFileName = option;}
  //! @brief Set the export base name. @param option name
  virtual void setExportBaseName(const String & option) { _exportBaseName = option;}
  //! @brief Set the export directory. @param option directory
  virtual void setExportPath(const String & option)     { _exportPath = option;}
  //! @brief Set the export file name. @param option file name
  virtual void setExportFileName(const String & option) { _exportFileName = option;}
  //! @brief Set the name of the task. @param option name
  virtual void setParentTaskName(const String & option) { _taskName = option; updateName(); }
  //! @brief Set the name of the owner. @param option name
  virtual void setOwnerName(const String & option)      { _ownerName = option;}
  //! @brief Set the object type. @param option type
  virtual void setTypeName(const String & option)       { _typeName  = option; updateName(); }

  //! @brief Name the list after its task and object type (\<task\>:\<TYPE\>, e.g. RunAnalysis:EVENT_FILTER), so that
  //! its messages say which list they come from.
  void updateName() { setName(createKey(_taskName,_typeName)); }
  
  
  
  //! @brief Import base name. @return name
  virtual const String & importBaseName() const { return _importBaseName;}
  //! @brief Import directory. @return directory
  virtual const String & importPath() const     { return _importPath;}
  //! @brief Import file name. @return file name
  virtual const String & importFileName() const { return _importFileName;}
  //! @brief Export base name. @return name
  virtual const String & getExportBaseName() const { return _exportBaseName;}
  //! @brief Export directory. @return directory
  virtual const String & getExportPath() const     { return _exportPath;}
  //! @brief Export file name. @return file name
  virtual const String & getExportFileName() const { return _exportFileName;}
  //! @brief Name of the owner. @return name
  virtual const String & getOwnerName() const      { return _ownerName; }
  //! @brief Name of the task. @return name
  virtual const String & taskName() const       { return _taskName; }
  //! @brief Object type. @return type
  virtual const String & typeName() const       { return _typeName; }
  
  //! @brief Create an object, add it to the store and to the list. @param objectName name @return object
  //! @throws InvalidNameException if the store already has an object of that name
  virtual MANAGED* instantiateObject(const String & objectName)
  {
  if (storeContainsObjectNamed(objectName))
    throw InvalidNameException(objectName,"Name already found in object store",__FUNCTION__);
  MANAGED * object = new MANAGED();
  object->setName(objectName);
  appendToStore(object);
  append(object);
  return object;
  }
  
  //! @brief Add the object of a given name from the store to the list. @param objectName name @return object
  //! @throws InvalidNameException if the store has no object of that name
  virtual MANAGED* useObject(const  String & objectName)
  {
  if (!storeContainsObjectNamed(objectName))
    throw InvalidNameException(objectName,"Name NOT found in object store",__FUNCTION__);
  MANAGED * object = getStoreObjectNamed(objectName);
  append(object);
  return object;
  }
  
  //! @brief Objects of the list. @return objects
  virtual std::vector<MANAGED*> & getObjects()
  {
  return _objects;
  }
  
  //! @brief Objects of the list. @return objects
  virtual const std::vector<MANAGED*> & getObjects() const
  {
  return _objects;
  }
  
  //! @brief Add an object to the list. @param object object
  virtual void append(MANAGED * object)
  {
  _objects.push_back(object);
  //  if (!storeContainsObjectNamed(object->name()))
  //    appendToStore(object);
  }
  
  //! @brief Whether the list has an object of a given name. @param name name @return true if so
  virtual bool containsObjectNamed(const String & name)
  {
  for (auto object : _objects) if (object->isNamed(name)) return true;
  return false;
  }
  
  //! @brief Object at an index. @param index index @return object
  virtual MANAGED * objectAt(unsigned int index)
  {
  if (index>=_objects.size()) throw IndexOutOfBoundException(index,_objects.size(),"Index out of bound",__FUNCTION__);
  return _objects[index];
  }
  
  //! @brief Object at an index. @param index index @return object
  virtual const MANAGED * objectAt(unsigned int index) const
  {
  if (index>=_objects.size()) throw IndexOutOfBoundException(index,_objects.size(),"Index out of bound",__FUNCTION__);
  return _objects[index];
  }
  
  //! @brief Object of a given name. @param name name @return object
  virtual MANAGED * getObjectNamed(const String & name)
  {
  for (auto object : _objects) if (object->isNamed(name)) return object;
  throw InvalidNameException(name,"Name not found in _objects",__FUNCTION__);
  }
  
  //! @brief Number of objects. @return number
  virtual unsigned int size() const
  {
  return _objects.size();
  }
  
  //! @brief Number of objects. @return number
  virtual unsigned int size()
  {
  return _objects.size();
  }
  
  

  
  //!
  //!Returns a pointer to a store of type std::vector<MANAGED*>
  //!
  static std::vector<MANAGED*> & getObjectStore()
  {
  if (!objectStore)
    {
    objectStore = new std::vector<MANAGED*>();
    }
  return *objectStore;
  }
  
  //! @brief Add an object to the store. @param object object
  virtual void appendToStore(MANAGED * object)
  {
  getObjectStore().push_back(object);
  }
  
  //! @brief Whether the store has an object of a given name. @param name name @return true if so
  virtual bool storeContainsObjectNamed(const String & name)
  {
  for (auto object : getObjectStore()) if (object->isNamed(name)) return true;
  return false;
  }
  
  //! @brief Object of the store at an index. @param index index @return object
  //! @throws IndexOutOfBoundException if index>=objectStoreSize()
  static MANAGED * getStoreObjectAt(unsigned int index)
  {
  std::vector<MANAGED*> & store = getObjectStore();
  if (index>=store.size()) throw IndexOutOfBoundException(index,store.size(),"index>=objectStoreSize()",__FUNCTION__);
  return store[index];
  }
  
  //! @brief Object of the store of a given name. @param name name @return object
  static MANAGED * getStoreObjectNamed(const String & name)
  {
  for (auto & object : getObjectStore()) if (object->isNamed(name)) return object;
  throw InvalidNameException(name,"Name not found in _objects",__FUNCTION__);
  }
  
  //! @brief Number of objects in the store. @return number
  static unsigned int objectStoreSize()
  {
  return getObjectStore().size();
  }
  
  //! @brief Empty the store.
  static void clearStore()
  {
  for (auto object : getObjectStore()) delete object;
  delete objectStore;
  }
  
  
  //! @brief Read the configuration of the list and create (OWNER) or take from the store the configured objects (see
  //! the class description). @param configuration configuration
  virtual void configureObjects(const Configuration & configuration)
  {
  //if (reportDebug(__FUNCTION__)) { printCR(); }
  String name;
  String title;
  bool   owner;
  String importPath;
  String exportPath;
  unsigned int nObjects;
  _use            = configuration.valueBool(  createKey(_taskName,_typeName,"USE"));
  _create         = configuration.valueBool(  createKey(_taskName,_typeName,"CREATE"));
  _import         = configuration.valueBool(  createKey(_taskName,_typeName,"IMPORT"));
  nObjects        = configuration.valueInt(   createKey(_taskName,_typeName,"N") );
  if (!isHistogramList())
    {
  _importBaseName = configuration.valueString(createKey(_taskName,_typeName,"IMPORT:BASE_NAME"));
  importPath      = configuration.valueString(createKey(_taskName,_typeName,"IMPORT:PATH"));
  _importFileName = configuration.valueString(createKey(_taskName,_typeName,"IMPORT:FILE_NAME"));
  _export         = configuration.valueBool(  createKey(_taskName,_typeName,"EXPORT"));
  _exportBaseName = configuration.valueString(createKey(_taskName,_typeName,"EXPORT:BASE_NAME"));
  exportPath      = configuration.valueString(createKey(_taskName,_typeName,"EXPORT:PATH"));
  _exportFileName = configuration.valueString(createKey(_taskName,_typeName,"EXPORT:FILE_NAME"));
  _forceRewrite   = configuration.valueBool(  createKey(_taskName,_typeName,"EXPORT:FORCE_REWRITE"));
    // "DEFAULT", absolute, "./..." or relative to the directory of the environment variable
    if (_typeName.beginsWith("PARTICLE_DB"))
      _importPath = FileSystem::configuredPath(importPath,"CAP_DATABASE_PATH");
    }
  bool verbose = true;
  if (reportInfo(__FUNCTION__))
    {
    verbose = true;
    printCR();
    printValue(createKey(_taskName,_typeName,"USE"),   _use);
    printValue(createKey(_taskName,_typeName,"CREATE"),_create);
    printValue(createKey(_taskName,_typeName,"IMPORT"),_import);
    if (!isHistogramList())
      {
    printValue(createKey(_taskName,_typeName,"IMPORT:PATH"),_importPath);
    printValue(createKey(_taskName,_typeName,"IMPORT:FILE_NAME"),_importFileName);
    printValue(createKey(_taskName,_typeName,"EXPORT"),_export);
    printValue(createKey(_taskName,_typeName,"EXPORT:PATH"),_exportPath);
    printValue(createKey(_taskName,_typeName,"EXPORT:FILE_NAME"),_exportFileName);
    printValue(createKey(_taskName,_typeName,"EXPORT:FORCE_REWRITE"),_forceRewrite);
      }
    printValue(createKey(_taskName,_typeName,"N"),nObjects);
    }
  MANAGED * object;
  
  for (unsigned int k=0; k<nObjects; k++)
    {
    name  = configuration.valueString(createKey(_taskName,_typeName,k,"NAME"));
    title = configuration.valueString(createKey(_taskName,_typeName,k,"TITLE"));
    owner = configuration.valueBool(  createKey(_taskName,_typeName,k,"OWNER"));
    if (verbose)
      {
      printValue(createKey(_taskName,_typeName,k,"NAME"),name);
      printValue(createKey(_taskName,_typeName,k,"TITLE"),title);
      printValue(createKey(_taskName,_typeName,k,"OWNER"),owner);
      }
    if (owner)
      {
      object = instantiateObject(name);
      Configuration scoped;
      bool own = objectConfiguration(configuration,k,scoped);
      object->configure(_taskName,_typeName,own ? scoped : configuration,k);
      if (!title.isEmpty() && title!="NONE") object->setTitle(title);
      }
    else
      object = useObject(name);
    }
  }

  //!
  //! @brief Configuration of histogram folder k, when it has its own binning: its keys \<task\>:\<TYPE\>_\<k\>:\<param\>
  //! (other than NAME, TITLE, OWNER and BASE_NAME) replace the values of \<task\>:HISTOGRAM:\<param\> and
  //! \<task\>:\<TYPE\>:\<param\>, the keys the folders read their binning from; the parameters a folder does not set are
  //! those of the HISTOGRAM group of the task. Only for HISTOGRAM types.
  //! @param configuration configuration of the task @param k index of the folder @param scoped configuration of the
  //! folder (set when the folder has keys of its own) @return true if the folder has keys of its own
  //!
  bool objectConfiguration(const Configuration & configuration, unsigned int k, Configuration & scoped) const
  {
  if (!_typeName.beginsWith("HISTOGRAM")) return false;
  String prefix = createKey(_taskName,_typeName,k,"");
  std::vector<std::pair<String,const Property *>> overrides;
  for (const Property & p : configuration.properties())
    {
    const String & key = p.name();
    if (!key.beginsWith(prefix)) continue;
    String parameter = key.subString(prefix.length());
    if (parameter.isEmpty() || parameter.contains(":") || parameter=="NAME" || parameter=="TITLE" || parameter=="OWNER"
        || parameter=="BASE_NAME") continue;
    overrides.push_back({parameter,&p});
    }
  if (overrides.empty()) return false;
  scoped = configuration;
  for (auto & o : overrides)
    {
    for (const String & group : { String("HISTOGRAM"), _typeName })
      {
      Property p(*o.second);
      p.setName(createKey(_taskName,group,o.first));
      scoped.addProperty(p);
      }
    int group = -1;
    String groupKey = createKey(_taskName,"HISTOGRAM",o.first);
    for (unsigned int i=0; i<configuration.properties().size() && group<0; i++)
      if (configuration.properties()[i].name()==groupKey) group = int(i);
    bool differs = group<0 || configuration.properties()[group].string()!=o.second->string();
    if (differs && reportInfo(__FUNCTION__)) printValue(o.second->name(),o.second->string());
    }
  return true;
  }
  
  //! @brief Import (IMPORT) and/or create (CREATE) the objects. Histograms are imported by their task from the input
  //! file of the job (loadFromFolders()).
  virtual void initialize()
  {
  if (reportInfo(__FUNCTION__)) { printCR(); }
  if (_import && !isHistogramList()) load();
  if (_create) create();
  }
  
  //! @brief Export the objects (EXPORT) and clear the list. Histograms are kept: the job writes them to its output
  //! file after the tasks are finalized (saveToFolders()), and they are cleared with the list.
  virtual void finalize()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  if (isHistogramList()) return;
  if (_export) exportObject();
  clear();
  }

  //! @brief Whether the list holds histogram folders (types HISTOGRAM...): no files of their own.
  bool isHistogramList() const { return _typeName.beginsWith("HISTOGRAM"); }

  //!
  //! @brief Save each object of the list in a sub-folder of a folder (that of its task in the output file of the job).
  //! @param parent folder @param folderName name of the sub-folder of object k (created if needed)
  //!
  template <class NAMER>
  void saveToFolders(Folder & parent, NAMER folderName) const
  {
  for (unsigned int k=0; k<_objects.size(); k++) _objects[k]->saveTo(*parent.addFolder(folderName(k)));
  }

  //!
  //! @brief Load each object of the list from a sub-folder of a folder (that of its task in the input file of the job).
  //! @param parent folder @param folderName name of the sub-folder of object k
  //! @throws Exception if a sub-folder is missing
  //!
  template <class NAMER>
  void loadFromFolders(const Folder & parent, NAMER folderName)
  {
  for (unsigned int k=0; k<_objects.size(); k++)
    {
    const String name = folderName(k);
    Folder * folder = parent.folder(name);
    if (!folder)
      throw Exception(String("Folder ")+name+" of "+_taskName+" (" +_typeName+") not found in the input file",__FUNCTION__);
    _objects[k]->loadFrom(*folder);
    }
  }
  
  //! @brief Create the content of each object.
  virtual void create()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  for (auto & object : _objects) object->create();
  }
  
  //! @brief Import the objects from the configured directory and file.
  virtual void load()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  load(_importPath,_importFileName);
  }
  
  //! @brief Import the objects from a file: a CAP file, or for PARTICLE_DB an XML file (particles and decay modes).
  //! @param importPath directory @param importFileName file name @throws Exception for another file
  virtual void load(const String & importPath,
                            const String & importFileName)
  {
  if (reportDebug(__FUNCTION__))
    {
    printCR();
    printValue("importPath",importPath);
    printValue("importFileName",importFileName);
    };
  if (isCapFileName(importFileName))
    {
    std::unique_ptr<CapFile> file(CapFile::openOldCapFile(importPath,capFileName(importFileName)));
    loadFrom(*file);
    }
  else
    {
    if (_typeName.equalTo("PARTICLE_DB") && importFileName.endsWith(".xml",true))
      {
      // particles and their decay modes in one XML file (see ParticleDbXml)
      loadFromXml(FileSystem::join(importPath,importFileName));
      }
    else
      {
      throw Exception(String("Cannot import ")+_typeName+" objects from "+importFileName
                      +": a CAP file (.cap, .root) or, for PARTICLE_DB, an XML file (.xml) is expected",__FUNCTION__);
      }
    
    }
  }
  
  //! @brief Load each object from a CAP file. @param file file
  virtual void loadFrom(CapFile & file)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  for (auto & object : _objects) object->loadFrom(file);
  }
  
  //! @brief Load each object from an ASCII file. @param file file
  virtual void loadFromAscii1(std::ifstream & file)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  for (auto & object : _objects) object->loadFromAscii(file);
  }
  
  //! @brief Load each object from an XML file. @param fileName file name
  //! @throws Exception for object types that cannot be read from XML (no loadFromXml(), see HasLoadFromXml)
  virtual void loadFromXml(const String & fileName)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  // Only types with a loadFromXml(const String &) function (ManagedObject and its derived classes, e.g. ParticleDb)
  // can be read from XML; the call must not be compiled for the other types (e.g. histogram folders), which the
  // instantiation of this virtual function would otherwise require.
  if constexpr (HasLoadFromXml<MANAGED>::value)
    {
    for (auto & object : _objects) object->loadFromXml(fileName);
    }
  else
    {
    throw Exception(String("Objects of type ")+_typeName+" cannot be read from XML",__FUNCTION__);
    }
  }
  
  //! @brief Save the objects to the configured export file.
  virtual void exportObject()
  {
  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printValue("EXPORT:PATH",_exportPath);
    printValue("EXPORT:FILE_NAME",_exportFileName);
    };
  if (isCapFileName(_exportFileName))
    {
    std::unique_ptr<CapFile> outputFile(CapFile::openRecreateCapFile(_exportPath,capFileName(_exportFileName)));
    for (auto & object : _objects) object->saveTo(*outputFile);
    outputFile->write();
    }
  else
    {
    String fileName = FileSystem::join(_exportPath,_exportFileName);
    std::ofstream & outputFile = openOutputAsciiFile(fileName);
    for (auto & object : _objects) object->saveToAscii(outputFile);
    outputFile.close();
    }
  }
  
  //! @brief Reset each object.
  virtual void reset()
  {
  for (auto & object : _objects) object->reset();
  }

  //! @brief Clear each object and empty the list.
  virtual void clear()
  {
  for (auto & object : _objects) object->clear();
  _objects.clear();
  if (_owner) clearStore();
  }

  //! @brief Print the objects.
  virtual  void print() const
  {
  for (auto & object : _objects) object->print();
  }
  
  //! @brief Scale each object by its factor. @param scaleFactors factors
  virtual  void scaleObject(const std::vector<double> & scaleFactors)
  {
  unsigned int k=0;
  for (auto & object : _objects) object->scaleObject(scaleFactors[k++]);
  }
  
  };
  
  template <class MANAGED>
  std::vector<MANAGED*> * ManagedObjects<MANAGED>::objectStore = nullptr;
  
} // namespace CAP

#endif /* CAP_Manager */
