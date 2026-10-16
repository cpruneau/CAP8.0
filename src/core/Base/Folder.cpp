#include "Folder.hpp"
#include "LongParameter.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>
#if defined(__GNUG__)
#include <cxxabi.h>
#endif

ClassImp(CAP::Folder);

namespace CAP
{

  std::string demangledTypeName(const std::type_info & type)
  {
#if defined(__GNUG__)
  int status = 0;
  char * name = abi::__cxa_demangle(type.name(),nullptr,nullptr,&status);
  if (status==0 && name)
    {
    std::string result(name);
    std::free(name);
    return result;
    }
#endif
  return type.name();
  }

  // ---------------------------------------------------------------- construction

  Folder::Folder(const String & name)
  :
  Object("","","Folder"),
  _entries(),
  _parent(nullptr),
  _currentPath()
  {
  setName(name);
  }

  Folder::Folder(const Folder & source)
  :
  Object(source),
  _entries(),
  _parent(nullptr),           // a copy is a top folder
  _currentPath()
  {
  _entries.reserve(source._entries.size());
  for (const auto & e : source._entries)
    _entries.push_back(Entry{e.name,std::unique_ptr<FolderItem>(e.item->clone())});
  adoptSubFolders();
  }

  Folder & Folder::operator=(const Folder & rhs)
  {
  if (this!=&rhs)
    {
    Folder copy(rhs);              // copy first: this folder is unchanged if an object cannot be copied
    Object::operator=(copy);
    _entries.swap(copy._entries);
    adoptSubFolders();
    _currentPath.clear();          // the parent of this folder is kept
    }
  return *this;
  }

  Folder::~Folder()
  {
  }

  void Folder::setName(const String & name)
  {
  if (std::string(name.data()).find('/')!=std::string::npos)
    throw Exception("A folder name cannot contain '/'","Folder::setName()");
  _name = name;
  }

  void Folder::print() const
  {
  print(std::cout);
  }

  // ---------------------------------------------------------------- entries

  void Folder::checkName(const String & name, const char * source)
  {
  std::string n(name.data());
  if (n.empty()) throw Exception("An entry needs a name: give one to add()",source);
  if (n=="." || n=="..") throw Exception("\".\" and \"..\" cannot be used as entry names",source);
  if (n.find('/')!=std::string::npos) throw Exception(String("Entry names cannot contain '/': ")+name,source);
  }

  void Folder::addItem(const String & name, FolderItem * item)
  {
  std::unique_ptr<FolderItem> owned(item);   // deleted (with its object) if the name is rejected
  checkName(name,"Folder::add()");
  std::string n(name.data());
  if (findEntry(n)) throw Exception(String("An entry named ")+name+" already exists in folder "+_name,"Folder::add()");
  if (Folder * f = owned->asFolder()) f->_parent = this;
  _entries.push_back(Entry{n,std::move(owned)});
  }

  Folder * Folder::addFolder(const String & name)
  {
  checkName(name,"Folder::addFolder()");
  Folder * target = current();
  if (const Entry * e = target->findEntry(name.data()))
    {
    Folder * existing = e->item->asFolder();
    if (!existing) throw Exception(String("An entry named ")+name+" exists and is not a folder","Folder::addFolder()");
    return existing;
    }
  return target->add(new Folder(name));
  }

  const Folder::Entry * Folder::findEntry(const std::string & name) const
  {
  for (const auto & e : _entries) if (e.name==name) return &e;
  return nullptr;
  }

  Folder * Folder::walk(const Folder * start, const std::string & path)
  {
  const Folder * f = start;
  std::size_t begin = 0;
  while (f && begin<=path.size())
    {
    std::size_t slash = path.find('/',begin);
    std::string component = path.substr(begin,(slash==std::string::npos) ? std::string::npos : slash-begin);
    if (component=="..")
      f = f->_parent;
    else if (!component.empty() && component!=".")
      {
      const Entry * e = f->findEntry(component);
      f = (e) ? e->item->asFolder() : nullptr;
      }
    if (slash==std::string::npos) break;
    begin = slash+1;
    }
  return const_cast<Folder *>(f);
  }

  Folder * Folder::parentOf(const String & path, std::string & leaf) const
  {
  std::string p(path.data());
  const Folder * start = current();
  if (!p.empty() && p[0]=='/') { start = top(); p = p.substr(1); }
  std::size_t slash = p.rfind('/');
  leaf = (slash==std::string::npos) ? p : p.substr(slash+1);
  if (leaf.empty() || leaf=="." || leaf=="..") return nullptr;
  if (slash==std::string::npos) return const_cast<Folder *>(start);
  return walk(start,p.substr(0,slash));
  }

  void Folder::adoptSubFolders()
  {
  for (auto & e : _entries)
    if (Folder * f = e.item->asFolder()) f->_parent = this;
  }

  // ---------------------------------------------------------------- navigation

  Folder * Folder::top() const
  {
  const Folder * f = this;
  while (f->_parent) f = f->_parent;
  return const_cast<Folder *>(f);
  }

  String Folder::path() const
  {
  std::string p;
  for (const Folder * f = this; f->_parent; f = f->_parent)
    {
    // name under which the parent holds f (the entry name, which is what paths use)
    std::string key(f->_name.data());
    for (const auto & e : f->_parent->_entries) if (e.item->asFolder()==f) { key = e.name; break; }
    p = "/" + key + p;
    }
  return String(p.empty() ? "/" : p.c_str());
  }

  Folder * Folder::current() const
  {
  if (_currentPath.empty()) return const_cast<Folder *>(this);
  Folder * f = walk(top(),_currentPath);
  return f ? f : const_cast<Folder *>(this);   // that branch was removed: back to this folder
  }

  Folder * Folder::cd(const String & path)
  {
  std::string p(path.data());
  const Folder * start = current();
  if (!p.empty() && p[0]=='/') { start = top(); p = p.substr(1); }
  Folder * target = walk(start,p);
  if (!target) return nullptr;
  _currentPath = (target==this) ? std::string() : std::string(target->path().data());
  return target;
  }

  Folder * Folder::cd()
  {
  Folder * parentFolder = current()->_parent;
  if (!parentFolder) return nullptr;
  return cd("..");
  }

  void Folder::ls(std::ostream & out, const String & path) const
  {
  const Folder * f = (path.length()==0) ? current() : folder(path);
  if (!f) { out << "No folder " << path.data() << std::endl; return; }
  out << f->path().data() << ":" << std::endl;
  for (const auto & e : f->_entries)
    out << "  " << e.name << (e.item->asFolder() ? "/" : "") << "  [" << e.item->typeName() << "]" << std::endl;
  }

  const Folder::Entry * Folder::findPath(const String & path) const
  {
  std::string leaf;
  Folder * parent = parentOf(path,leaf);
  return parent ? parent->findEntry(leaf) : nullptr;
  }

  Folder * Folder::folder(const String & path) const
  {
  std::string p(path.data());
  const Folder * start = current();
  if (!p.empty() && p[0]=='/') { start = top(); p = p.substr(1); }
  return walk(start,p);
  }

  bool Folder::contains(const String & path) const
  {
  return findPath(path)!=nullptr;
  }

  std::string Folder::typeName(const String & path) const
  {
  const Entry * e = findPath(path);
  return e ? e->item->typeName() : std::string();
  }

  bool Folder::remove(const String & path)
  {
  std::string leaf;
  Folder * parent = parentOf(path,leaf);
  if (!parent) return false;
  for (auto it=parent->_entries.begin(); it!=parent->_entries.end(); ++it)
    if (it->name==leaf) { parent->_entries.erase(it); return true; }
  return false;
  }

  void Folder::clear()
  {
  _entries.clear();
  _currentPath.clear();
  }

  std::vector<String> Folder::names() const
  {
  std::vector<String> result;
  for (const auto & e : current()->_entries) result.push_back(String(e.name.c_str()));
  return result;
  }

  void Folder::collectPaths(const std::string & prefix, std::vector<String> & out, bool unsavableOnly) const
  {
  for (const auto & e : _entries)
    {
    std::string path = prefix.empty() ? e.name : prefix+"/"+e.name;
    if (Folder * f = e.item->asFolder()) { f->collectPaths(path,out,unsavableOnly); continue; }
    if (!unsavableOnly || !e.item->savable()) out.push_back(String(path.c_str()));
    }
  }

  std::vector<String> Folder::paths() const
  {
  std::vector<String> result;
  collectPaths("",result,false);
  return result;
  }

  std::vector<String> Folder::unsavablePaths() const
  {
  std::vector<String> result;
  collectPaths("",result,true);
  return result;
  }

  void Folder::print(std::ostream & out, unsigned int indent) const
  {
  std::string pad(indent,' ');
  out << pad << _name.data() << "/  (" << _entries.size() << " entries)" << std::endl;
  for (const auto & e : _entries)
    {
    if (Folder * f = e.item->asFolder()) { f->print(out,indent+2); continue; }
    out << pad << "  " << e.name << "  [" << e.item->typeName() << (e.item->savable() ? "" : ", not savable") << "]" << std::endl;
    }
  }

  // ---------------------------------------------------------------- arithmetic

  bool Folder::compareStructure(const Folder & other, const std::string & path, std::string * reason) const
  {
  auto fail = [&](const std::string & why) { if (reason) *reason = why; return false; };
  for (const auto & o : other._entries)
    if (!findEntry(o.name)) return fail("Entry "+path+"/"+o.name+" is missing in the first folder");
  for (const auto & e : _entries)
    {
    std::string entryPath = path+"/"+e.name;
    const Entry * o = other.findEntry(e.name);
    if (!o) return fail("Entry "+entryPath+" is missing in the second folder");
    if (e.item->type()!=o->item->type())
      return fail("Entry "+entryPath+": types differ ("+e.item->typeName()+" and "+o->item->typeName()+")");
    if (Folder * f = e.item->asFolder())
      if (!f->compareStructure(*o->item->asFolder(),entryPath,reason)) return false;
    }
  return true;
  }

  bool Folder::sameStructure(const Folder & other, std::string * reason) const
  {
  return compareStructure(other,"",reason);
  }

  Folder Folder::combineFolders(const Folder & left, const Folder & right, bool subtract, const std::string & path)
  {
  std::string reason;
  if (!left.compareStructure(right,path,&reason)) throw Exception(String(reason.c_str()),subtract ? "Folder::subtract()" : "Folder::add()");
  Folder result(left.name());
  for (const auto & e : left._entries)
    {
    const Entry * o = right.findEntry(e.name);
    result.addItem(String(e.name.c_str()),e.item->combine(*o->item,subtract,path+"/"+e.name));
    }
  return result;
  }

  Folder Folder::add(const Folder & left, const Folder & right, const String & name)
  {
  Folder result = combineFolders(left,right,false,"");
  if (name.length()>0) result.setName(name);
  return result;
  }

  Folder Folder::subtract(const Folder & left, const Folder & right, const String & name)
  {
  Folder result = combineFolders(left,right,true,"");
  if (name.length()>0) result.setName(name);
  return result;
  }

  Folder & Folder::operator+=(const Folder & rhs)
  {
  Folder result = combineFolders(*this,rhs,false,"");   // computed first: this folder is unchanged if it fails
  _entries.swap(result._entries);
  adoptSubFolders();
  _currentPath.clear();
  return *this;
  }

  Folder & Folder::operator-=(const Folder & rhs)
  {
  Folder result = combineFolders(*this,rhs,true,"");
  _entries.swap(result._entries);
  adoptSubFolders();
  _currentPath.clear();
  return *this;
  }

  Folder operator+(const Folder & left, const Folder & right) { return Folder::add(left,right); }
  Folder operator-(const Folder & left, const Folder & right) { return Folder::subtract(left,right); }

  // ---------------------------------------------------------------- binary files

  std::map<std::string,Folder::Factory> & Folder::registry()
  {
  static std::map<std::string,Factory> types;
  static bool initialized = false;
  if (!initialized)
    {
    initialized = true;
    registerType<Folder>();
    registerType<LongParameter>();
    // Other libraries register their own classes when they are loaded (Math/FolderTypes.cpp, Graphics/Data2DPoints.cpp, ...).
    }
  return types;
  }

  void Folder::encodeBinary(BinaryWriter & w) const
  {
  encodeBinary(w,false);
  }

  void Folder::encodeBinary(BinaryWriter & w, bool skipUnsavable) const
  {
  // each entry: its name, then the complete record of its object (sub-folders included)
  std::vector<const Entry *> saved;
  for (const auto & e : _entries)
    {
    if (e.item->savable()) { saved.push_back(&e); continue; }
    if (!skipUnsavable)
      throw Exception(String("Entry ")+e.name.c_str()+" of folder "+_name+" (type "+e.item->typeName().c_str()+
                      ") cannot be saved: remove it, or save with skipUnsavable=true","Folder::encodeBinary()");
    }
  w.writeVarUInt(saved.size());
  for (const Entry * e : saved)
    {
    std::ostringstream record;
    e->item->writeRecord(record,skipUnsavable);
    const std::string bytes = record.str();
    w.writeString(String(e->name.c_str()));
    w.writeVarUInt(bytes.size());
    w.writeBytes(bytes.data(),bytes.size());
    }
  }

  void Folder::decodeBinary(BinaryReader & r, const String & folderName, unsigned int /*version*/)
  {
  setName(folderName);
  _entries.clear();
  _currentPath.clear();
  uint64_t n = r.readVarUInt();
  if (n > r.remaining()) throw Exception("More entries than data","Folder::decodeBinary()");
  for (uint64_t i=0; i<n; i++)
    {
    String entryName = r.readString();
    uint64_t length = r.readVarUInt();
    if (length > r.remaining()) throw Exception("Entry larger than the data","Folder::decodeBinary()");
    std::string bytes(std::size_t(length),'\0');
    for (auto & c : bytes) c = char(r.readUInt8());
    std::istringstream in(bytes);
    std::string tag, payload;
    unsigned int version;
    if (!readBinaryRecord(in,tag,version,payload,entryName))
      throw Exception(String("Empty record for entry ")+entryName,"Folder::decodeBinary()");
    if (in.peek()!=EOF) throw Exception(String("Unexpected data after the record of entry ")+entryName,"Folder::decodeBinary()");
    auto factory = registry().find(tag);
    if (factory==registry().end())
      throw Exception(String("Entry ")+entryName+" has type tag "+tag.c_str()+", which is not registered: call Folder::registerType<T>() for its class","Folder::decodeBinary()");
    addItem(entryName,factory->second(payload,version,entryName));
    }
  }

  void Folder::writeBinary(std::ostream & out, bool skipUnsavable) const
  {
  BinaryWriter w;
  w.writeString(_name);
  try
    {
    encodeBinary(w,skipUnsavable);
    }
  catch (FileException &)
    {
    throw;
    }
  catch (Exception & e)
    {
    throw FileException(_name,e.what(),"Folder::writeBinary()");
    }
  writeBinaryRecord(out,binaryTag(),binaryVersion(),w.buffer(),_name);
  }

  bool Folder::readBinary(std::istream & in)
  {
  return readBinaryObject(in,*this);
  }

  void Folder::saveBinary(const String & fileName, bool append, bool skipUnsavable) const
  {
  std::ofstream out(fileName.data(),std::ios::binary | (append ? std::ios::app : std::ios::trunc));
  if (!out) throw FileException(fileName,"Cannot open file for writing","Folder::saveBinary()");
  writeBinary(out,skipUnsavable);
  out.close();
  if (!out) throw FileException(fileName,"Error writing file","Folder::saveBinary()");
  }

  void Folder::loadBinary(const String & fileName, const String & folderName)
  {
  loadBinaryObject(fileName,*this,folderName);
  }

  std::vector<Folder> Folder::loadAllBinary(const String & fileName)
  {
  return loadAllBinaryObjects<Folder>(fileName);
  }

} // namespace CAP
