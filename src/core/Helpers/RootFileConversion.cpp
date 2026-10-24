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
#include <memory>
#include <set>
#include <string>
#include "TClass.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TKey.h"
#include "RootConversion.hpp"
#include "RootFileConversion.hpp"
#include "Exceptions.hpp"

namespace CAP
{

  namespace
  {
  //! Name usable in a folder: no '/', not empty.
  String entryName(const char * name)
  {
  std::string s = name ? name : "";
  for (char & c : s) if (c=='/') c = '_';
  if (s.empty()) s = "unnamed";
  return String(s.c_str());
  }

  void report(std::vector<String> * skipped, const String & path, const char * reason)
  {
  if (skipped) skipped->push_back(path + String(": ") + String(reason));
  }
  }

  int importRootDirectory(TDirectory & directory, Folder & folder, std::vector<String> * skipped)
  {
  int count = 0;
  std::set<std::string> seen;
  TIter next(directory.GetListOfKeys());
  while (TKey * key = static_cast<TKey*>(next()))
    {
    // the keys of a name are listed from the highest cycle down: the first one is the current object
    if (!seen.insert(key->GetName()).second) continue;
    String where = folder.path() + String("/") + String(key->GetName());
    TClass * type = TClass::GetClass(key->GetClassName());
    if (!type) { report(skipped,where,"unknown class"); continue; }
    if (type->InheritsFrom(TDirectory::Class()))
      {
      TDirectory * sub = directory.GetDirectory(key->GetName());
      if (!sub) { report(skipped,where,"cannot be read"); continue; }
      String name = folder.uniqueName(entryName(key->GetName()));
      count += importRootDirectory(*sub,*folder.addFolder(name),skipped);
      continue;
      }
    if (!type->InheritsFrom(TH1::Class())) { report(skipped,where,key->GetClassName()); continue; }
    std::unique_ptr<TObject> object(key->ReadObj());
    TH1 * h = dynamic_cast<TH1*>(object.get());
    if (!h) { report(skipped,where,"cannot be read"); continue; }
    h->SetDirectory(nullptr);
    try
      {
      MATH::Histogram * converted = fromRootHistogram(*h);
      String name = folder.uniqueName(entryName(key->GetName()));
      converted->setName(name);
      if (MATH::Profile * p = dynamic_cast<MATH::Profile*>(converted)) folder.add(p,name);
      else folder.add(converted,name);
      count++;
      }
    catch (const std::exception & e) { report(skipped,where,e.what()); }
    }
  return count;
  }

  int importRootFile(const String & path, Folder & folder, std::vector<String> * skipped)
  {
  std::unique_ptr<TFile> file(TFile::Open(path,"READ"));
  if (!file || file->IsZombie()) throw Exception(String("Cannot open the ROOT file ")+path,__FUNCTION__);
  int count = importRootDirectory(*file,folder,skipped);
  file->Close();
  return count;
  }

  int exportRootDirectory(const Folder & folder, TDirectory & directory, std::vector<String> * skipped)
  {
  int count = 0;
  for (const String & name : folder.names())
    {
    String where = folder.path() + String("/") + name;
    if (Folder * sub = folder.folder(name))
      {
      TDirectory * d = directory.mkdir(name);
      if (!d) { report(skipped,where,"cannot create the directory"); continue; }
      count += exportRootDirectory(*sub,*d,skipped);
      continue;
      }
    const MATH::Histogram * h = folder.get<MATH::Profile>(name);
    if (!h) h = folder.get<MATH::Histogram>(name);
    if (!h) { report(skipped,where,folder.typeName(name).c_str()); continue; }
    try
      {
      std::unique_ptr<TH1> r(toRootHistogram(*h,name));
      directory.WriteTObject(r.get(),name);
      count++;
      }
    catch (const std::exception & e) { report(skipped,where,e.what()); }
    }
  return count;
  }

  int exportRootFile(const Folder & folder, const String & path, std::vector<String> * skipped)
  {
  std::unique_ptr<TFile> file(TFile::Open(path,"RECREATE"));
  if (!file || file->IsZombie()) throw Exception(String("Cannot create the ROOT file ")+path,__FUNCTION__);
  int count = exportRootDirectory(folder,*file,skipped);
  file->Close();
  return count;
  }

} // namespace CAP
