/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau
 * All rights reserved.
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#include "ParticleDbXml.hpp"
#include "ParticleDbParser.hpp"
#include "ParticleDecayMode.hpp"
#include <map>
#include <set>
#include <sstream>
#include <string>

namespace CAP
{

namespace
{
  const char * const kParticleAttributes[] = { "name", "title", "pdg", "charge", "mass", "massUncertainty", "width",
                                               "widthUncertainty", "spin", "isospin", "isospin3", "quarks",
                                               "antiquarks", "leptons" };
  const char * const kDecayAttributes[]    = { "branchingRatio", "clebschGordan", "products" };

  //! Name of a PARTICLE element, for error messages.
  String where(const XmlElement & element)
  {
  return String("<")+element.name()+" name=\""+element.attribute("name","?")+"\">";
  }

  //! Check that an element has only known attributes.
  template <std::size_t N>
  void checkAttributes(const XmlElement & element, const char * const (&known)[N], const String & context)
  {
  for (const auto & attribute : element.attributes())
    {
    bool ok = false;
    for (const char * k : known) if (attribute.first==k) { ok = true; break; }
    if (!ok) throw XmlException(context+": unknown attribute '"+attribute.first+"'","ParticleDbXml::read");
    }
  }

  //! Split a list of names separated by white space.
  std::vector<std::string> splitNames(const String & list)
  {
  std::vector<std::string> names;
  std::istringstream iss(list.str());
  std::string n;
  while (iss >> n) names.push_back(n);
  return names;
  }
}

bool ParticleDbXml::isXmlFileName(const String & fileName)
{
  return fileName.endsWith(".xml",true);
}

void ParticleDbXml::read(ParticleDb & particleDb, const String & fileName)
{
  XmlDocument document = XmlDocument::read(fileName);
  const XmlElement & root = document.root();
  const XmlElement * element = root.isNamed("PARTICLE_DB") ? &root : root.findFirst("PARTICLE_DB");
  if (!element) throw XmlException(String("No <PARTICLE_DB> element in ")+fileName,__FUNCTION__);
  readElement(particleDb,*element);
}

void ParticleDbXml::read(ParticleDb & particleDb, const String & path, const String & name)
{
  read(particleDb,XmlDocument::fileName(path,name));
}

void ParticleDbXml::readElement(ParticleDb & particleDb, const XmlElement & element)
{
  if (!element.isNamed("PARTICLE_DB"))
    throw XmlException(String("Expected <PARTICLE_DB>, got <")+element.name()+">",__FUNCTION__);
  if (!element.attributes().empty())
    throw XmlException(String("<PARTICLE_DB>: unknown attribute '")+element.attributes()[0].first+"'",__FUNCTION__);

  // first pass: the particles (so that decay products may be defined anywhere in the file)
  std::map<std::string,ParticleType*> byName;
  for (const ParticleType * type : particleDb.particleTypes()) byName[type->name().str()] = const_cast<ParticleType*>(type);
  std::vector<std::pair<const XmlElement*,ParticleType*>> read;
  int index = int(particleDb.particleTypes().size());
  for (const XmlElement * child : element.children())
    {
    if (!child->isNamed("PARTICLE"))
      throw XmlException(String("<PARTICLE_DB>: unexpected element <")+child->name()+">",__FUNCTION__);
    const XmlElement & e = *child;
    String context = where(e);
    checkAttributes(e,kParticleAttributes,context);
    String name = e.attribute("name");
    if (name.isNull()) throw XmlException("<PARTICLE> with an empty name",__FUNCTION__);
    if (byName.count(name.str())) throw XmlException(context+": duplicate particle name",__FUNCTION__);
    ParticleType * type = new ParticleType();
    try
      {
      type->setIndex(index);
      type->setName(name);
      type->setTitle(e.attribute("title"));
      type->setPdgCode(int(e.attributeInt("pdg")));
      type->setCharge(e.attributeDouble("charge"));
      type->setMass(e.attributeDouble("mass"));
      type->setMassUncertainty(e.attributeDouble("massUncertainty",0.0));
      type->setWidth(e.attributeDouble("width",0.0));
      type->setWidthUncertainty(e.attributeDouble("widthUncertainty",0.0));
      type->setSpin(e.attributeDouble("spin"));
      type->setIsospin(e.attributeDouble("isospin"));
      type->setIsospin3(e.attributeDouble("isospin3"));
      type->setQuarkNumbers(int(e.attributeInt("quarks")));
      type->setAntiQuarkNumbers(int(e.attributeInt("antiquarks")));
      type->setLeptonNumbers(int(e.attributeInt("leptons")));
      type->disableDecay();
      type->enable();
      }
    catch (const std::exception & exception)
      {
      delete type;
      throw XmlException(context+": "+exception.what(),"ParticleDbXml::read");
      }
    particleDb.append(type);
    byName[name.str()] = type;
    read.push_back(std::make_pair(&e,type));
    index++;
    }

  // second pass: the decay modes
  for (const auto & entry : read)
    {
    const XmlElement & e = *entry.first;
    ParticleType * parent = entry.second;
    for (const XmlElement * d : e.children())
      {
      String context = where(e) + " <" + d->name() + ">";
      if (!d->isNamed("DECAY")) throw XmlException(where(e)+": unexpected element <"+d->name()+">",__FUNCTION__);
      checkAttributes(*d,kDecayAttributes,context);
      double branchingRatio = d->attributeDouble("branchingRatio");
      bool   clebschGordan  = d->attributeBool("clebschGordan",false);
      std::vector<ParticleType*> products;
      for (const std::string & productName : splitNames(d->attribute("products")))
        {
        auto it = byName.find(productName);
        if (it==byName.end()) throw XmlException(context+": unknown product '"+productName+"'",__FUNCTION__);
        products.push_back(it->second);
        }
      if (products.size()<2 || products.size()>4)
        throw XmlException(context+": 2 to 4 products expected",__FUNCTION__);
      ParticleDbParser::addDecayMode(parent,products,branchingRatio,clebschGordan);
      }
    }

  ParticleDbParser::finishDb(particleDb);
}

XmlElement ParticleDbXml::createElement(const ParticleDb & particleDb)
{
  XmlElement element("PARTICLE_DB");
  for (const ParticleType * type : particleDb.particleTypes())
    {
    XmlElement & e = element.addChild("PARTICLE");
    e.setAttribute("name",type->name());
    e.setAttribute("title",type->title());
    e.setAttribute("pdg",type->pdgCode());
    e.setAttribute("charge",type->charge());
    e.setAttribute("mass",type->mass());
    e.setAttribute("massUncertainty",type->massUncertainty());
    e.setAttribute("width",type->width());
    e.setAttribute("widthUncertainty",type->widthUncertainty());
    e.setAttribute("spin",type->spin());
    e.setAttribute("isospin",type->isospin());
    e.setAttribute("isospin3",type->isospin3());
    e.setAttribute("quarks",type->quarkNumbers());
    e.setAttribute("antiquarks",type->antiQuarkNumbers());
    e.setAttribute("leptons",type->leptonNumbers());
    for (const ParticleDecayMode & mode : type->decayModes())
      {
      XmlElement & d = e.addChild("DECAY");
      d.setAttribute("branchingRatio",mode.declaredBranchingRatio());
      d.setAttribute("clebschGordan",mode.clebschGordan());
      String products;
      for (const ParticleType * child : mode.childrenTypes())
        {
        if (!products.isNull()) products += " ";
        products += child->name();
        }
      d.setAttribute("products",products);
      }
    }
  return element;
}

XmlDocument ParticleDbXml::createDocument(const ParticleDb & particleDb)
{
  XmlDocument document("CAP");
  document.root().setAttribute("version",1);
  document.root().setAttribute("type","particleDb");
  document.root().addChild(createElement(particleDb));
  return document;
}

void ParticleDbXml::write(const ParticleDb & particleDb, const String & fileName)
{
  createDocument(particleDb).write(fileName);
}

void ParticleDbXml::write(const ParticleDb & particleDb, const String & path, const String & name)
{
  createDocument(particleDb).write(path,name);
}

} // namespace CAP
