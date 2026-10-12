/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//!
//! Convert the ASCII particle database (particles.data and decays.data) to the XML format (one file, see
//! CAP::ParticleDbXml), then read the XML file back and check that it gives the same database: every particle
//! property and every decay mode (declared and effective branching ratios, Clebsch-Gordan flag, products).
//!
//! Usage (after sourcing SetupCAP.sh):
//!   root -l -b -q 'ConvertParticleDb.C'                                  // $CAP_DATABASE_PATH/ParticleData/ -> ParticleDb.xml there
//!   root -l -b -q 'ConvertParticleDb.C("/path/to/data/","MyDb.xml")'
//!
//! Returns 0 if the XML database matches the ASCII one.
//!
#include <iostream>
#include <fstream>
#include <map>
#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TList.h"
#include "TString.h"

int loadCapLibraries()
{
  const char * libPath = gSystem->Getenv("CAP_LIB_PATH");
  if (!libPath) { std::cout << "<ERROR> CAP_LIB_PATH is not set: source SetupCAP.sh first" << std::endl; return -1; }
  TSystemDirectory directory("lib",libPath);
  TList * files = directory.GetListOfFiles();
  if (!files) { std::cout << "<ERROR> cannot list " << libPath << std::endl; return -1; }
  int nLoaded = 0;
  for (TObject * object : *files)
    {
    TString fileName = object->GetName();
    if (!fileName.BeginsWith("lib") || !(fileName.EndsWith(".dylib") || fileName.EndsWith(".so"))) continue;
    if (gSystem->Load(TString(libPath)+"/"+fileName)>=0) nLoaded++;
    else std::cout << "<WARNING> could not load " << fileName << std::endl;
    }
  delete files;
  return nLoaded;
}

int ConvertParticleDb(const char * dataPath="", const char * xmlName="ParticleDb.xml")
{
  if (loadCapLibraries()<=0) return 1;

  TString path = dataPath;
  if (path.IsNull())
    {
    const char * db = gSystem->Getenv("CAP_DATABASE_PATH");
    if (!db) { std::cout << "<ERROR> CAP_DATABASE_PATH is not set and no data path given" << std::endl; return 1; }
    path = TString(db) + "/ParticleData/";
    }
  if (!path.EndsWith("/")) path += "/";

  try
    {
    // 1. ASCII database
    std::ifstream particles((path+"particles.data").Data());
    std::ifstream decays((path+"decays.data").Data());
    if (!particles || !decays) { std::cout << "<ERROR> cannot open " << path << "particles.data or decays.data" << std::endl; return 1; }
    CAP::ParticleDb asciiDb;
    CAP::ParticleDbParser parser;
    parser.readDb(asciiDb,particles,decays);

    // 2. write it as XML, 3. read the XML file back
    asciiDb.exportXml(CAP::String(path.Data()),CAP::String(xmlName));
    CAP::ParticleDb xmlDb;
    xmlDb.loadFromXml(CAP::String((path+xmlName).Data()));

    // 4. compare (by name: loadFromXml() sorts the particles by mass)
    int nErrors = 0;
    auto report = [&](const CAP::ParticleType * t, const char * what)
      {
      if (nErrors<20) std::cout << "<MISMATCH> " << t->name() << ": " << what << std::endl;
      nErrors++;
      };
    std::map<std::string,const CAP::ParticleType*> xmlTypes;
    for (const CAP::ParticleType * t : xmlDb.particleTypes()) xmlTypes[t->name().str()] = t;
    if (xmlDb.particleTypes().size()!=asciiDb.particleTypes().size())
      {
      std::cout << "<MISMATCH> number of particles: ascii " << asciiDb.particleTypes().size()
                << " xml " << xmlDb.particleTypes().size() << std::endl;
      nErrors++;
      }
    int nModes = 0;
    for (const CAP::ParticleType * a : asciiDb.particleTypes())
      {
      auto it = xmlTypes.find(a->name().str());
      if (it==xmlTypes.end()) { report(a,"missing from the XML database"); continue; }
      const CAP::ParticleType * x = it->second;
      if (a->title()!=x->title())                       report(a,"title");
      if (a->pdgCode()!=x->pdgCode())                   report(a,"pdg");
      if (a->charge()!=x->charge())                     report(a,"charge");
      if (a->mass()!=x->mass())                         report(a,"mass");
      if (a->massUncertainty()!=x->massUncertainty())   report(a,"massUncertainty");
      if (a->width()!=x->width())                       report(a,"width");
      if (a->widthUncertainty()!=x->widthUncertainty()) report(a,"widthUncertainty");
      if (a->spin()!=x->spin())                         report(a,"spin");
      if (a->isospin()!=x->isospin())                   report(a,"isospin");
      if (a->isospin3()!=x->isospin3())                 report(a,"isospin3");
      if (a->quarkNumbers()!=x->quarkNumbers())         report(a,"quarks");
      if (a->antiQuarkNumbers()!=x->antiQuarkNumbers()) report(a,"antiquarks");
      if (a->leptonNumbers()!=x->leptonNumbers())       report(a,"leptons");
      const auto & am = a->decayModes();
      const auto & xm = x->decayModes();
      if (am.size()!=xm.size()) { report(a,"number of decay modes"); continue; }
      for (unsigned int k=0; k<am.size(); k++)
        {
        nModes++;
        if (am[k].branchingRatio()!=xm[k].branchingRatio())                 report(a,"decay branching ratio");
        if (am[k].declaredBranchingRatio()!=xm[k].declaredBranchingRatio()) report(a,"decay declared branching ratio");
        if (am[k].clebschGordan()!=xm[k].clebschGordan())                   report(a,"decay Clebsch-Gordan flag");
        if (am[k].childrenTypes().size()!=xm[k].childrenTypes().size())     { report(a,"decay number of products"); continue; }
        for (unsigned int c=0; c<am[k].childrenTypes().size(); c++)
          if (am[k].childrenTypes()[c]->name()!=xm[k].childrenTypes()[c]->name()) report(a,"decay product");
        }
      }
    std::cout << asciiDb.particleTypes().size() << " particles, " << nModes << " decay modes written to "
              << path << xmlName << std::endl;
    if (nErrors==0) { std::cout << "XML database identical to the ASCII one." << std::endl; return 0; }
    std::cout << nErrors << " mismatch(es)" << std::endl;
    return 1;
    }
  catch (const std::exception & exception)
    {
    std::cout << "<ERROR> " << exception.what() << std::endl;
    return 1;
    }
}
