/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#include "ParticleDbXml.hpp"
#include "ParticleDecayMode.hpp"
#include "MathBasicFunctions.hpp"
#include <cmath>
#include <map>
#include <set>
#include <sstream>
#include <string>

namespace CAP
{

namespace
{
  const double factorials[7] = {1.0, 1.0, 2.0, 6.0, 24.0, 120.0, 720.0 };

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
      addDecayMode(parent,products,branchingRatio,clebschGordan);
      }
    }

  particleDb.mapAntiParticleIndices();
  particleDb.setupDecayGenerator();
}

void ParticleDbXml::addDecayMode(ParticleType * parentType,
                                 const std::vector<ParticleType*> & children,
                                 double declaredBranchingRatio,
                                 bool clebschGordanFlag)
{
  if (children.size()<2 || children.size()>4)
    {
    String message = "Decay mode of "; message += parentType->name(); message += " with "; message += int(children.size());
    message += " products (2 to 4 expected)";
    throw Exception(message,__FUNCTION__);
    }
  ParticleType * childType1 = children[0];
  ParticleType * childType2 = children[1];
  double ratio = declaredBranchingRatio;
  if (clebschGordanFlag)
    {
    // complete branching ratio by Clebsch-Gordan coefficient
    double J  = parentType->isospin();
    double M  = parentType->isospin3();
    double j1 = childType1->isospin();
    double m1 = childType1->isospin3();
    double j2 = childType2->isospin();
    double m2 = childType2->isospin3();
    double CB = clebschGordan(J, M, j1, m1, j2, m2);
    ratio = CB*CB * declaredBranchingRatio;
    // Multiply the Clebsh by two?
    // The same spin, mass, strangeness (charm?)
    // and different I3?
    if (  (CAP::MATH::absolute(childType1->spin()  - childType2->spin()) < 0.01)
        &&  (CAP::MATH::absolute(childType1->mass() - childType2->mass()) < 0.01)
        &&  (CAP::MATH::absolute(childType1->isospin3()-childType2->isospin3())   > 0.01)
        &&  (childType1->strangeness()-childType2->strangeness() == 0)
        &&  (childType1->charm()- childType2->charm() == 0)        )
      {
      ratio *= 2.0;
      }
    }
  ParticleDecayMode decayMode;
  decayMode.setBranchingRatio(ratio);
  decayMode.setDeclaredBranchingRatio(declaredBranchingRatio);
  decayMode.setClebschGordan(clebschGordanFlag);
  for (ParticleType * child : children) decayMode.addChild(child);  // was: the 4th product added the 3rd one again
  parentType->addDecayMode(decayMode);
}

double ParticleDbXml::clebschGordan(double aJot,  double aEm,
                                    double aJot1, double aEm1,
                                    double aJot2, double aEm2)
{
  int mint, maxt;
  double cgc = 0.0;
  int tIter;
  double coef;

  maxt = lrint(aJot1 + aJot2 - aJot);
  mint = 0;
  if (lrint(aJot1 - aEm1)  < maxt) maxt = lrint(aJot1 - aEm1);
  if (lrint(aJot2 + aEm2)  < maxt) maxt = lrint(aJot2 + aEm2);
  if (lrint(-(aJot-aJot2+aEm1))  > mint) mint = lrint(-(aJot-aJot2+aEm1));
  if (lrint(-(aJot-aJot1-aEm2))  > mint) mint = lrint(-(aJot-aJot1-aEm2));

  //  // PRINT_DEBUG_3("\t\t\tmint " << mint << " j1 " <<  aJot1 << " m1 " << aEm1);
  //  // PRINT_DEBUG_3("\t\t\tampxt " << maxt << " j2 " <<  aJot2 << " m2 " << aEm2);

  for (tIter = mint; tIter<=maxt; tIter ++)
    {
    coef = CAP::MATH::power(-1, tIter);
    // PRINT_DEBUG_3("\t\t\tcoef1 " << coef);
    coef *= std::sqrt( (2*aJot+1)
                 * factorials[lrint(aJot1+aEm1)]
                 * factorials[lrint(aJot1-aEm1)]
                 * factorials[lrint(aJot2+aEm2)]
                 * factorials[lrint(aJot2-aEm2)]
                 * factorials[lrint(aJot+aEm)]
                 * factorials[lrint(aJot-aEm)]
                 );
    // PRINT_DEBUG_3("\t\t\tcoef2 " << coef);
    coef /= factorials[tIter]
    * factorials[lrint(aJot1+aJot2-aJot-tIter)]
    * factorials[lrint(aJot1-aEm1-tIter)]
    * factorials[lrint(aJot2+aEm2-tIter)]
    * factorials[lrint(aJot-aJot2+aEm1+tIter)]
    * factorials[lrint(aJot-aJot1-aEm2+tIter)];
    // PRINT_DEBUG_3("\t\t\tcoef3 " << coef);
    cgc += coef;
    }
  cgc *= deltaJ(aJot1, aJot2, aJot);

  return cgc;
}

double ParticleDbXml::deltaJ(double aJot1, double aJot2, double aJot)
{
  double res = std::sqrt( 1.0
                    * factorials[lrint( aJot1 + aJot2 - aJot)]
                    * factorials[lrint( aJot1 - aJot2 + aJot)]
                    * factorials[lrint(-aJot1 + aJot2 + aJot)]
                    / factorials[lrint( aJot1 + aJot2 + aJot + 1)]
                    );
  return res;
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
