/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FileSystem.cpp
//! @brief Implementation of the FileSystem functions.
#include "FileSystem.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace CAP
{
namespace FileSystem
{

  namespace
  {
  std::string str(const String & s) { return std::string(s.data()); }

  std::string lower(std::string s)
  {
  std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){ return char(std::tolower(c)); });
  return s;
  }

  //! Extension with its leading '.', or "" for an empty extension.
  std::string dotted(const String & extension)
  {
  std::string e = str(extension);
  if (!e.empty() && e[0]!='.') e.insert(0,1,'.');
  return e;
  }

  //! The path without its final '/' characters (but "/" stays "/").
  std::string withoutTrailingSlash(std::string p)
  {
  while (p.size()>1 && p.back()=='/') p.pop_back();
  return p;
  }

  bool userMay(const String & path, int mode)
  {
  return path.length()>0 && ::access(path.data(),mode)==0;
  }

  [[noreturn]] void fail(const String & path, const std::string & what, const std::error_code & ec, const char * source)
  {
  std::string message = what;
  if (ec) message += ": " + ec.message();
  throw FileException(path,String(message),source);
  }

  EntryType typeOf(const fs::file_status & status)
  {
  switch (status.type())
    {
    case fs::file_type::regular:   return EntryType::kFile;
    case fs::file_type::directory: return EntryType::kDirectory;
    case fs::file_type::symlink:   return EntryType::kSymlink;
    case fs::file_type::not_found:
    case fs::file_type::none:      return EntryType::kNone;
    default:                       return EntryType::kOther;
    }
  }

  //! Information about an entry known to exist; directory entries give their status without another system call.
  EntryInfo makeInfo(const String & path, const String & name)
  {
  EntryInfo info;
  info.path = path;
  info.name = name;
  std::error_code ec;
  fs::file_status linkStatus = fs::symlink_status(str(path),ec);
  info.isLink = !ec && fs::is_symlink(linkStatus);
  fs::file_status status = fs::status(str(path),ec);
  info.type = ec ? (info.isLink ? EntryType::kSymlink : EntryType::kNone) : typeOf(status);
  struct stat buffer;
  if (::stat(path.data(),&buffer)==0)
    {
    info.modified = buffer.st_mtime;
    if (S_ISREG(buffer.st_mode)) info.size = std::uintmax_t(buffer.st_size);
    }
  info.readable   = userMay(path,R_OK);
  info.writable   = userMay(path,W_OK);
  info.executable = userMay(path,X_OK);
  info.hidden     = name.length()>0 && name[0]=='.';
  return info;
  }

  bool matchesAny(const String & name, const std::vector<String> & patterns)
  {
  for (const auto & pattern : patterns) if (matches(name,pattern)) return true;
  return false;
  }

  bool hasAnyExtension(const String & name, const std::vector<String> & extensions)
  {
  for (const auto & extension : extensions) if (hasExtension(name,extension)) return true;
  return false;
  }

  bool lessByName(const EntryInfo & a, const EntryInfo & b)
  {
  std::string x = lower(str(a.name)), y = lower(str(b.name));
  return x!=y ? x<y : str(a.name)<str(b.name);
  }

  //! Entries of one directory (unsorted); hidden ones left out unless asked. Unreadable directories give nothing.
  std::vector<EntryInfo> entriesOf(const String & directory, bool hidden)
  {
  std::vector<EntryInfo> entries;
  std::error_code ec;
  fs::directory_iterator it(str(directory),fs::directory_options::skip_permission_denied,ec), end;
  for (; !ec && it!=end; it.increment(ec))
    {
    String name(it->path().filename().string());
    if (!hidden && name.length()>0 && name[0]=='.') continue;
    entries.push_back(makeInfo(join(directory,name),name));
    }
  return entries;
  }

  void listInto(const String & directory, const ListOptions & options, int level, std::vector<EntryInfo> & result)
  {
  std::vector<EntryInfo> entries = entriesOf(directory,options.hidden);
  if (options.order==ListOptions::kDirectoriesFirst)
    std::sort(entries.begin(),entries.end(),[](const EntryInfo & a, const EntryInfo & b)
      { return a.isDirectory()!=b.isDirectory() ? a.isDirectory() : lessByName(a,b); });
  else
    std::sort(entries.begin(),entries.end(),lessByName);
  for (const auto & entry : entries)
    {
    if (matchesAny(entry.name,options.exclude)) continue;
    if (entry.isDirectory())
      {
      if (options.directories) result.push_back(entry);
      bool descend = options.depth<0 || level<options.depth;
      if (descend && (!entry.isLink || options.followLinks)) listInto(entry.path,options,level+1,result);
      }
    else if (options.files)
      {
      if (!options.extensions.empty() && !hasAnyExtension(entry.name,options.extensions)) continue;
      if (!options.include.empty() && !matchesAny(entry.name,options.include)) continue;
      result.push_back(entry);
      }
    }
  }

  //! Glob matching of one name against one pattern component, from positions n and p.
  bool matchFrom(const std::string & name, std::size_t n, const std::string & pattern, std::size_t p)
  {
  while (p<pattern.size())
    {
    char c = pattern[p];
    if (c=='*')
      {
      while (p<pattern.size() && pattern[p]=='*') p++;
      if (p==pattern.size()) return true;
      for (std::size_t k=n; k<=name.size(); k++) if (matchFrom(name,k,pattern,p)) return true;
      return false;
      }
    if (n>=name.size()) return false;
    if (c=='?') { n++; p++; continue; }
    if (c=='[')
      {
      std::size_t close = pattern.find(']',p+2);   // a ']' right after '[' (or "[!") is a member of the set
      if (close==std::string::npos) { if (name[n]!='[') return false; n++; p++; continue; }
      std::size_t q = p+1;
      bool negate = pattern[q]=='!' || pattern[q]=='^';
      if (negate) q++;
      bool found = false;
      for (; q<close; q++)
        {
        if (q+2<close && pattern[q+1]=='-') { if (name[n]>=pattern[q] && name[n]<=pattern[q+2]) found = true; q += 2; }
        else if (pattern[q]==name[n]) found = true;
        }
      if (found==negate) return false;
      n++; p = close+1;
      continue;
      }
    if (c!=name[n]) return false;
    n++; p++;
    }
  return n==name.size();
  }

  bool hasWildcard(const std::string & component)
  {
  return component.find_first_of("*?[")!=std::string::npos;
  }

  //! The directory itself and all its sub-directories (not hidden ones, not through links).
  void allDirectories(const String & directory, std::vector<String> & result)
  {
  result.push_back(directory);
  for (const auto & entry : entriesOf(directory.isNull() ? String(".") : directory,false))
    if (entry.isDirectory() && !entry.isLink)
      allDirectories(directory.isNull() ? entry.name : entry.path,result);
  }
  } // namespace

  // ---------------------------------------------------------------- information

  bool exists(const String & path)
  {
  std::error_code ec;
  return path.length()>0 && fs::exists(fs::symlink_status(str(path),ec));
  }

  bool isFile(const String & path)
  {
  std::error_code ec;
  return path.length()>0 && fs::is_regular_file(str(path),ec);
  }

  bool isDirectory(const String & path)
  {
  std::error_code ec;
  return path.length()>0 && fs::is_directory(str(path),ec);
  }

  bool isLink(const String & path)
  {
  std::error_code ec;
  return path.length()>0 && fs::is_symlink(str(path),ec);
  }

  bool isReadable(const String & path)   { return userMay(path,R_OK); }
  bool isExecutable(const String & path) { return userMay(path,X_OK); }

  bool isWritable(const String & path)
  {
  if (exists(path)) return userMay(path,W_OK);
  String directory = parent(path);
  return isDirectory(directory.isNull() ? String(".") : directory) && userMay(directory.isNull() ? String(".") : directory,W_OK);
  }

  std::uintmax_t size(const String & path)
  {
  if (!isFile(path)) throw FileException(path,"Not a regular file","FileSystem::size()");
  std::error_code ec;
  std::uintmax_t bytes = fs::file_size(str(path),ec);
  if (ec) fail(path,"Cannot get the size",ec,"FileSystem::size()");
  return bytes;
  }

  std::time_t modified(const String & path)
  {
  struct stat buffer;
  if (path.length()==0 || ::stat(path.data(),&buffer)!=0) throw FileException(path,"No such file or directory","FileSystem::modified()");
  return buffer.st_mtime;
  }

  EntryInfo info(const String & path)
  {
  if (!exists(path)) throw FileException(path,"No such file or directory","FileSystem::info()");
  return makeInfo(path,name(path));
  }

  // ---------------------------------------------------------------- paths

  String join(const String & directory, const String & name)
  {
  if (name.isNull()) return directory;
  if (directory.isNull() || name[0]=='/') return name;
  std::string d = str(directory);
  if (d.back()!='/') d += '/';
  return String(d + str(name));
  }

  String join(const String & directory, const String & name, const String & extension)
  {
  String file = name;
  if (!extension.isNull() && !hasExtension(name,extension)) file.append(String(dotted(extension)));
  return join(directory,file);
  }

  String normalize(const String & path)
  {
  std::error_code ec;
  fs::path p = fs::absolute(path.isNull() ? fs::path(".") : fs::path(str(path)),ec);
  fs::path canonical = fs::weakly_canonical(p,ec);
  if (!ec) p = canonical;
  return String(withoutTrailingSlash(p.lexically_normal().string()));
  }

  String parent(const String & path)
  {
  std::string p = withoutTrailingSlash(str(path));
  std::size_t slash = p.rfind('/');
  if (slash==std::string::npos) return String("");
  if (slash==0) return String("/");
  return String(withoutTrailingSlash(p.substr(0,slash)));
  }

  String name(const String & path)
  {
  std::string p = withoutTrailingSlash(str(path));
  if (p=="/") return String("/");
  std::size_t slash = p.rfind('/');
  return String(slash==std::string::npos ? p : p.substr(slash+1));
  }

  String stem(const String & path)
  {
  std::string n = str(name(path));
  std::string e = str(extension(path));
  return String(n.substr(0,n.size()-e.size()));
  }

  String extension(const String & path)
  {
  std::string n = str(name(path));
  std::size_t dot = n.rfind('.');
  if (dot==std::string::npos || dot==0 || n=="..") return String("");
  return String(n.substr(dot));
  }

  bool hasExtension(const String & path, const String & extension)
  {
  std::string e = dotted(extension);
  return !e.empty() && lower(str(FileSystem::extension(path)))==lower(e);
  }

  String withExtension(const String & path, const String & extension)
  {
  std::string p = withoutTrailingSlash(str(path));
  std::string current = str(FileSystem::extension(String(p)));
  return String(p.substr(0,p.size()-current.size()) + dotted(extension));
  }

  String expand(const String & path)
  {
  std::string in = str(path), out;
  std::size_t k = 0;
  if (!in.empty() && in[0]=='~' && (in.size()==1 || in[1]=='/')) { out = str(home()); k = 1; }
  while (k<in.size())
    {
    if (in[k]!='$') { out += in[k++]; continue; }
    std::size_t start = k+1, end;
    std::string variable;
    if (start<in.size() && in[start]=='{')
      {
      end = in.find('}',start);
      if (end==std::string::npos) { out += in.substr(k); break; }
      variable = in.substr(start+1,end-start-1);
      end++;
      }
    else
      {
      end = start;
      while (end<in.size() && (std::isalnum((unsigned char)in[end]) || in[end]=='_')) end++;
      variable = in.substr(start,end-start);
      }
    const char * value = variable.empty() ? nullptr : std::getenv(variable.c_str());
    out += value ? std::string(value) : in.substr(k,end-k);   // undefined: left as written
    k = end;
    }
  return String(out);
  }

  String home()
  {
  const char * value = std::getenv("HOME");
  return String(value ? value : "");
  }

  String current()
  {
  std::error_code ec;
  fs::path p = fs::current_path(ec);
  return String(ec ? std::string(".") : p.string());
  }

  String resolve(const String & path, const String & variable)
  {
  String p = expand(path);
  if (p.length()>0 && p[0]=='/') return p;
  if (p.equalTo(".") || p.beginsWith("./")) return join(current(),String(p.length()>1 ? p.subString(2) : String("")));
  const char * value = std::getenv(variable.data());
  if (!value || !*value)
    throw FileException(path,String("Relative path and the environment variable ")+variable+" is not defined","FileSystem::resolve()");
  return join(String(value),p);
  }

  String configuredPath(const String & path, const String & variable)
  {
  const char * value = std::getenv(variable.data());
  String directory(value ? value : "");
  if (path.isNull() || path.equalTo("DEFAULT")) return directory;
  String p = expand(path);
  if (p.length()>0 && p[0]=='/') return p;
  if (p.equalTo(".") || p.beginsWith("./")) return join(current(),String(p.length()>1 ? p.subString(2) : String("")));
  return directory.isNull() ? p : join(directory,p);
  }

  String firstExisting(const std::vector<String> & candidates)
  {
  for (const auto & candidate : candidates) if (exists(candidate)) return candidate;
  return String("");
  }

  // ---------------------------------------------------------------- listing and searching

  std::vector<EntryInfo> list(const String & directory, const ListOptions & options)
  {
  if (!isDirectory(directory)) throw FileException(directory,"Not a directory","FileSystem::list()");
  if (!isReadable(directory) || !isExecutable(directory)) throw FileException(directory,"Directory cannot be read","FileSystem::list()");
  std::vector<EntryInfo> result;
  listInto(directory,options,1,result);
  if (options.order==ListOptions::kModified)
    std::stable_sort(result.begin(),result.end(),[](const EntryInfo & a, const EntryInfo & b){ return a.modified>b.modified; });
  else if (options.order==ListOptions::kSize)
    std::stable_sort(result.begin(),result.end(),[](const EntryInfo & a, const EntryInfo & b){ return a.size>b.size; });
  return result;
  }

  bool matches(const String & name, const String & pattern)
  {
  return matchFrom(str(name),0,str(pattern),0);
  }

  std::vector<String> glob(const String & pattern)
  {
  std::string p = str(expand(pattern));
  std::vector<std::string> components;
  std::size_t k = 0;
  while (k<p.size())
    {
    std::size_t slash = p.find('/',k);
    if (slash==std::string::npos) slash = p.size();
    if (slash>k) components.push_back(p.substr(k,slash-k));
    k = slash+1;
    }
  std::vector<String> bases{ String(!p.empty() && p[0]=='/' ? "/" : "") };
  for (std::size_t c=0; c<components.size(); c++)
    {
    const std::string & component = components[c];
    bool last = (c+1==components.size());
    std::vector<String> next;
    for (const auto & base : bases)
      {
      String where = base.isNull() ? String(".") : base;
      if (component=="**")
        {
        if (isDirectory(where)) allDirectories(base,next);
        }
      else if (hasWildcard(component))
        {
        bool hidden = component[0]=='.';
        for (const auto & entry : entriesOf(where,hidden))
          {
          if (!matches(entry.name,String(component))) continue;
          if (!last && !entry.isDirectory()) continue;
          next.push_back(join(base,entry.name));
          }
        }
      else
        {
        String candidate = join(base,String(component));
        if (last ? exists(candidate) : isDirectory(candidate)) next.push_back(candidate);
        }
      }
    bases.swap(next);
    }
  if (components.empty()) bases.clear();
  // "**" may reach the same path more than once; "" (current directory, from a final "**") is not a path
  std::vector<String> result;
  for (const auto & b : bases) if (!b.isNull()) result.push_back(b);
  std::sort(result.begin(),result.end(),[](const String & a, const String & b){ return str(a)<str(b); });
  result.erase(std::unique(result.begin(),result.end(),[](const String & a, const String & b){ return a.equalTo(b); }),result.end());
  return result;
  }

  // ---------------------------------------------------------------- changes

  void createDirectory(const String & path, bool parents)
  {
  if (isDirectory(path)) return;
  if (exists(path)) throw FileException(path,"Exists and is not a directory","FileSystem::createDirectory()");
  std::error_code ec;
  if (parents) fs::create_directories(str(path),ec);
  else         fs::create_directory(str(path),ec);
  if (ec || !isDirectory(path)) fail(path,"Cannot create the directory",ec,"FileSystem::createDirectory()");
  }

  void remove(const String & path)
  {
  if (!exists(path)) throw FileException(path,"No such file or directory","FileSystem::remove()");
  std::error_code ec;
  fs::remove(str(path),ec);
  if (ec) fail(path,"Cannot remove",ec,"FileSystem::remove()");
  }

  std::uintmax_t removeTree(const String & path)
  {
  if (!exists(path)) throw FileException(path,"No such file or directory","FileSystem::removeTree()");
  String target = normalize(path);
  if (target.equalTo("/") || target.equalTo(normalize(home())))
    throw FileException(path,"Refusing to remove the root or home directory","FileSystem::removeTree()");
  std::error_code ec;
  std::uintmax_t removed = fs::remove_all(str(path),ec);
  if (ec) fail(path,"Cannot remove",ec,"FileSystem::removeTree()");
  return removed;
  }

  void rename(const String & from, const String & to)
  {
  if (!exists(from)) throw FileException(from,"No such file or directory","FileSystem::rename()");
  std::error_code ec;
  fs::rename(str(from),str(to),ec);
  if (ec) fail(from,std::string("Cannot rename to ")+str(to),ec,"FileSystem::rename()");
  }

  void copy(const String & from, const String & to, bool overwrite)
  {
  if (!exists(from)) throw FileException(from,"No such file or directory","FileSystem::copy()");
  if (!overwrite && exists(to)) throw FileException(to,"Already exists","FileSystem::copy()");
  fs::copy_options options = fs::copy_options::recursive;
  if (overwrite) options |= fs::copy_options::overwrite_existing;
  std::error_code ec;
  fs::copy(str(from),str(to),options,ec);
  if (ec) fail(from,std::string("Cannot copy to ")+str(to),ec,"FileSystem::copy()");
  }

} // namespace FileSystem
} // namespace CAP
