#include "Folder.hpp"
#include "LongParameter.hpp"
#include "FileSystem.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>
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

  Folder::Folder(Folder && source)
  :
  Object(source),
  _entries(std::move(source._entries)),
  _parent(nullptr),           // a moved folder is a top folder
  _currentPath()
  {
  source._entries.clear();
  source._currentPath.clear();
  adoptSubFolders();
  }

  Folder & Folder::operator=(Folder && rhs)
  {
  if (this!=&rhs)
    {
    Object::operator=(rhs);
    _entries = std::move(rhs._entries);
    rhs._entries.clear();
    rhs._currentPath.clear();
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

  String Folder::copyEntry(const Folder & source, const String & path, const String & name)
  {
  const Entry * e = source.findPath(path);
  if (!e) throw Exception(String("No entry ")+path+" in folder "+source.name(),"Folder::copyEntry()");
  std::string leaf = path.data();
  std::size_t slash = leaf.rfind('/');
  if (slash!=std::string::npos) leaf = leaf.substr(slash+1);
  String copyName = name.isEmpty() ? String(leaf.c_str()) : name;
  e->item->load();                                  // a copy independent of the file it was read from
  std::unique_ptr<FolderItem> copy(e->item->clone());
  if (Folder * f = copy->asFolder()) { f->loadAll(); f->setName(copyName); }
  current()->addItem(copyName,copy.release());
  return copyName;
  }

  String Folder::uniqueName(const String & name) const
  {
  const Folder * target = current();
  if (!target->findEntry(name.data())) return name;
  for (int k=2; ; k++)
    {
    String candidate = name+String("_")+String(std::to_string(k).c_str());
    if (!target->findEntry(candidate.data())) return candidate;
    }
  }

  void Folder::rename(const String & path, const String & newName)
  {
  checkName(newName,"Folder::rename()");
  std::string leaf;
  Folder * parent = parentOf(path,leaf);
  if (!parent) throw Exception(String("No entry ")+path,"Folder::rename()");
  if (leaf==newName.data()) return;
  if (parent->findEntry(newName.data())) throw Exception(String("An entry named ")+newName+" already exists","Folder::rename()");
  for (auto & e : parent->_entries)
    if (e.name==leaf)
      {
      e.name = newName.data();
      if (Folder * f = e.item->asFolder()) f->setName(newName);
      return;
      }
  throw Exception(String("No entry ")+path,"Folder::rename()");
  }

  std::unique_ptr<FolderItem> Folder::take(const String & path)
  {
  std::string leaf;
  Folder * parent = parentOf(path,leaf);
  if (!parent) return nullptr;
  for (auto it=parent->_entries.begin(); it!=parent->_entries.end(); ++it)
    if (it->name==leaf)
      {
      std::unique_ptr<FolderItem> item = std::move(it->item);
      parent->_entries.erase(it);
      if (Folder * f = item->asFolder()) f->_parent = nullptr;
      return item;
      }
  return nullptr;
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

  std::map<std::string,const std::type_info *> & Folder::typeRegistry()
  {
  static std::map<std::string,const std::type_info *> types;
  return types;
  }

  const std::type_info * Folder::registeredType(const std::string & tag)
  {
  registry();   // registers Folder and LongParameter on first use
  auto found = typeRegistry().find(tag);
  return found==typeRegistry().end() ? nullptr : found->second;
  }

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

  // ---------------------------------------------------------------- lazy loading

  bool Folder::isLoaded(const String & path) const
  {
  const Entry * e = findPath(path);
  return e && e->item->isLoaded();
  }

  unsigned int Folder::nNotLoaded() const
  {
  unsigned int n = 0;
  for (const auto & e : _entries)
    {
    if (const Folder * f = e.item->asFolder()) n += f->nNotLoaded();
    else if (!e.item->isLoaded()) n++;
    }
  return n;
  }

  void Folder::loadAll()
  {
  for (auto & e : _entries)
    {
    if (Folder * f = e.item->asFolder()) f->loadAll();
    else e.item->load();
    }
  }

  FolderLazyItem::FolderLazyItem(const std::shared_ptr<const LazySource> & source, const std::string & tag,
                                 std::uint64_t offset, std::uint64_t length)
  :
  _source(source),
  _tag(tag),
  _offset(offset),
  _length(length),
  _decoded()
  { }

  FolderItem * FolderLazyItem::clone() const
  {
  if (_decoded) return _decoded->clone();
  return new FolderLazyItem(_source,_tag,_offset,_length);
  }

  const std::type_info & FolderLazyItem::type() const
  {
  if (_decoded) return _decoded->type();
  const std::type_info * type = Folder::registeredType(_tag);
  return type ? *type : decoded()->type();
  }

  std::string FolderLazyItem::typeName() const
  {
  return demangledTypeName(type());
  }

  std::string FolderLazyItem::record() const
  {
  const String & path = _source->path;
  if (!FileSystem::isFile(path) || FileSystem::size(path)!=_source->size || FileSystem::modified(path)!=_source->modified)
    throw FileException(path,"The file changed since it was read lazily: open it again","FolderLazyItem");
  std::ifstream in(path.data(),std::ios::binary);
  std::string bytes(std::size_t(_length),'\0');
  in.seekg(std::streamoff(_offset));
  if (_length>0) in.read(&bytes[0],std::streamsize(_length));
  if (!in || std::uint64_t(in.gcount())!=_length) throw FileException(path,"Cannot read a record of the file","FolderLazyItem");
  return bytes;
  }

  FolderItem * FolderLazyItem::decoded() const
  {
  if (_decoded) return _decoded.get();
  const String & where = _source->path;
  std::string bytes = record();
  BinaryReader r(bytes.data(),bytes.size());
  std::string tag;
  unsigned int version;
  const char * payload;
  std::size_t length;
  readBinaryRecord(r,tag,version,payload,length,where);
  auto factory = Folder::registry().find(tag);
  if (factory==Folder::registry().end())
    throw FileException(where,String("Type tag ")+tag.c_str()+" is not registered","FolderLazyItem");
  try
    {
    _decoded.reset(factory->second(std::string(payload,length),version,where));
    }
  catch (FileException &)
    {
    throw;
    }
  catch (Exception & e)
    {
    throw FileException(where,String("Invalid binary record: ")+e.what(),"FolderLazyItem");
    }
  return _decoded.get();
  }

  void FolderLazyItem::writeRecord(std::ostream & out, bool skipUnsavable) const
  {
  if (_decoded) { _decoded->writeRecord(out,skipUnsavable); return; }
  std::string bytes = record();   // not decoded: the record is copied as it is
  out.write(bytes.data(),std::streamsize(bytes.size()));
  if (!out) throw FileException(_source->path,"Error writing binary record","FolderLazyItem");
  }

  namespace
  {
  //! Reads a stream once, keeping the position and the checksums of the records being read (nested records each have
  //! their own); skipped bytes count in the checksums but are not kept.
  class ScanReader
  {
  public:
  ScanReader(std::istream & in, const String & where) : _in(in), _where(where), _position(0), _crcs() {}
  std::uint64_t position() const { return _position; }
  bool atEnd() { return _in.peek()==EOF; }
  void read(char * data, std::size_t n)
  {
    _in.read(data,std::streamsize(n));
    if (std::size_t(_in.gcount())!=n) fail("Truncated binary record");
    for (auto & c : _crcs) c = crc32Update(c,data,n);
    _position += n;
  }
  void skip(std::uint64_t n)
  {
    char buffer[1<<16];
    while (n>0)
      {
      std::size_t k = std::size_t(std::min<std::uint64_t>(n,sizeof(buffer)));
      read(buffer,k);
      n -= k;
      }
  }
  std::uint8_t u8() { char c; read(&c,1); return std::uint8_t(c); }
  std::uint32_t u32()
  {
    char b[4];
    read(b,4);
    std::uint32_t v = 0;
    for (int i=0; i<4; i++) v |= std::uint32_t(std::uint8_t(b[i]))<<(8*i);
    return v;
  }
  std::uint64_t varUInt()
  {
    std::uint64_t v = 0;
    for (int shift=0; shift<64; shift+=7)
      {
      std::uint8_t b = u8();
      v |= std::uint64_t(b & 0x7F)<<shift;
      if (!(b & 0x80)) return v;
      }
    fail("Malformed binary record");
  }
  String string(std::uint64_t limit)
  {
    std::uint64_t n = varUInt();
    if (n>limit) fail("Malformed binary record");
    std::string s(std::size_t(n),'\0');
    if (n>0) read(&s[0],std::size_t(n));
    return String(s);
  }
  std::string tag()
  {
    char t[4];
    read(t,4);
    if (t[0]!='C' || t[1]!='A' || t[2]!='P') fail("Not a CAP binary record (bad tag)");
    return std::string(t,4);
  }
  //! Start the checksum of a record whose payload follows.
  void beginRecord() { _crcs.push_back(0xFFFFFFFFu); }
  //! End of the payload of the innermost record: read its checksum and compare.
  void endRecord()
  {
    std::uint32_t computed = _crcs.back() ^ 0xFFFFFFFFu;
    _crcs.pop_back();
    if (u32()!=computed) fail("Corrupted binary record (checksum mismatch)");
  }
  [[noreturn]] void fail(const char * message) const { throw FileException(_where,message,"Folder::scanBinary()"); }

  private:
  std::istream & _in;
  String _where;
  std::uint64_t _position;
  std::vector<std::uint32_t> _crcs;
  };
  } // namespace

  //! Builds the tree of a lazily read folder (friend of Folder).
  struct FolderScanner
  {
  //! The entries of a folder payload, up to position end.
  static void entries(Folder & folder, ScanReader & r, const std::shared_ptr<const LazySource> & source, std::uint64_t end)
  {
    std::uint64_t n = r.varUInt();
    if (n > end - r.position()) r.fail("More entries than data");
    for (std::uint64_t i=0; i<n; i++)
      {
      String entryName = r.string(end - r.position());
      std::uint64_t length = r.varUInt();
      std::uint64_t start = r.position();
      if (length > end - start) r.fail("Entry larger than the data");
      std::string tag = r.tag();
      unsigned int version = r.u8();
      std::uint64_t payloadLength = r.varUInt();
      std::uint64_t payloadStart = r.position();
      if (payloadStart + payloadLength + 4 != start + length) r.fail("Malformed binary record");
      r.beginRecord();
      if (tag==Folder::binaryTag())
        {
        if (version==0 || version>Folder::binaryVersion()) r.fail("Binary record of an unsupported format version");
        std::unique_ptr<Folder> sub(new Folder());
        sub->setName(r.string(payloadLength));
        entries(*sub,r,source,payloadStart+payloadLength);
        if (r.position()!=payloadStart+payloadLength) r.fail("Unexpected data at the end of a folder record");
        r.endRecord();
        folder.addItem(entryName,new FolderItemOf<Folder>(sub.release()));
        }
      else
        {
        if (!Folder::registeredType(tag))
          throw FileException(source->path,String("Entry ")+entryName+" has type tag "+tag.c_str()+
                              ", which is not registered: call Folder::registerType<T>() for its class","Folder::scanBinary()");
        r.skip(payloadLength);
        r.endRecord();
        folder.addItem(entryName,new FolderLazyItem(source,tag,start,length));
        }
      }
  }
  };

  bool Folder::scanBinary(std::istream & in, const std::shared_ptr<const LazySource> & source, const String * folderName)
  {
  ScanReader r(in,source->path);
  while (!r.atEnd())
    {
    std::string tag = r.tag();
    unsigned int version = r.u8();
    std::uint64_t length = r.varUInt();
    if (length > (std::uint64_t(1)<<40)) r.fail("Implausible binary record length");
    std::uint64_t start = r.position();
    r.beginRecord();
    if (tag==binaryTag())
      {
      String name = r.string(length);
      if (!folderName || name.equalTo(*folderName))
        {
        if (version==0 || version>binaryVersion()) r.fail("Binary record of an unsupported format version");
        Folder loaded;
        try
          {
          loaded.setName(name);
          FolderScanner::entries(loaded,r,source,start+length);
          }
        catch (FileException &)
          {
          throw;
          }
        catch (Exception & e)
          {
          throw FileException(source->path,String("Invalid binary record: ")+e.what(),"Folder::scanBinary()");
          }
        if (r.position()!=start+length) r.fail("Unexpected data at the end of a folder record");
        r.endRecord();
        *this = std::move(loaded);   // the parent of this folder is kept
        return true;
        }
      }
    r.skip(start + length - r.position());
    r.endRecord();
    }
  return false;
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
    // the entry's record is read in place; only its payload is copied, for the factory
    BinaryReader entry(r.readSpan(std::size_t(length)),std::size_t(length));
    std::string tag;
    unsigned int version;
    const char * payload;
    std::size_t payloadLength;
    readBinaryRecord(entry,tag,version,payload,payloadLength,entryName);
    if (entry.remaining()!=0) throw Exception(String("Unexpected data after the record of entry ")+entryName,"Folder::decodeBinary()");
    auto factory = registry().find(tag);
    if (factory==registry().end())
      throw Exception(String("Entry ")+entryName+" has type tag "+tag.c_str()+", which is not registered: call Folder::registerType<T>() for its class","Folder::decodeBinary()");
    addItem(entryName,factory->second(std::string(payload,payloadLength),version,entryName));
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
