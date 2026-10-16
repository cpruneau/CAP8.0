/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau, 05/16/2024
 *
 * *********************************************************************/
#include "ParticleDbParser.hpp"
#include "PrintHelpers.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include "ParticleDecayMode.hpp"
#include "MathBasicFunctions.hpp"

using CAP::MATH::absolute;
using CAP::MATH::power;

ClassImp(CAP::ParticleDbParser);


namespace CAP
{


const double factorials[7] = {1.0, 1.0, 2.0, 6.0, 24.0, 120.0, 720.0 };



ParticleDbParser::ParticleDbParser()
:
PropertiesParser()
{ }

namespace
{
  //! Whether a line holds data (not empty, not a comment); leading white space is ignored.
  bool isDataLine(const std::string & line)
  {
  std::size_t first = line.find_first_not_of(" \t\r");
  return first!=std::string::npos && line[first]!='#';
  }

  String lineError(const char * fileName, int lineNumber, const std::string & line)
  {
  String message = fileName;
  message += " line ";
  message += lineNumber;
  message += ": cannot parse '";
  message += line.c_str();
  message += "'";
  return message;
  }
}

//!
//! Read particles.data and decays.data (see the class description).
//!
void  ParticleDbParser::readDb(ParticleDb  & particleDb,
                               std::ifstream & inputFile,
                               std::ifstream & inputFileDecays,
                               bool verbose)
{
  std::string line;
  int lineNumber = 0;
  int index = 0;

  //
  // read particles
  //
  while (std::getline(inputFile,line))
    {
    lineNumber++;
    if (!isDataLine(line)) continue;
    std::istringstream iss(line);
    std::string name, title;
    int    pdgCode, quarks, antiquarks, leptons;
    double charge, mass, massUncertainty, width, widthUncertainty, spin, isospin, isospin3;
    if (!(iss >> name >> title >> pdgCode >> charge >> mass >> massUncertainty >> width >> widthUncertainty
              >> spin >> isospin >> isospin3 >> quarks >> antiquarks >> leptons))
      throw Exception(lineError("particles.data",lineNumber,line),__FUNCTION__);
    ParticleType * particleType = new ParticleType();
    particleType->setIndex(index);
    particleType->setName(name.c_str());
    particleType->setTitle(title.c_str());
    particleType->setCharge(charge);
    particleType->setMass(mass);
    particleType->setMassUncertainty(massUncertainty);
    particleType->setWidth(width);
    particleType->setWidthUncertainty(widthUncertainty);
    particleType->setSpin(spin);
    particleType->setIsospin(isospin);
    particleType->setIsospin3(isospin3);
    particleType->setQuarkNumbers(quarks);
    particleType->setAntiQuarkNumbers(antiquarks);
    particleType->setLeptonNumbers(leptons);
    particleType->setPdgCode(pdgCode);
    particleType->disableDecay();
    particleType->enable();
    particleDb.append(particleType);
    index++;
    }

  //
  // read decays
  //
  lineNumber = 0;
  while (std::getline(inputFileDecays,line))
    {
    lineNumber++;
    if (!isDataLine(line)) continue;
    std::istringstream iss(line);
    std::string parentName, childName[4];
    double branchingRatio;
    int    clebschGordanFlag; // complete branching ratio by Clebsch-Gordan coefficient: 0-no 1-yes
    if (!(iss >> parentName >> childName[0] >> childName[1] >> childName[2] >> childName[3] >> branchingRatio >> clebschGordanFlag))
      throw Exception(lineError("decays.data",lineNumber,line),__FUNCTION__);
    ParticleType * parentType = particleDb.particleType(parentName.c_str());  // throws if unknown
    std::vector<ParticleType*> children;
    for (const std::string & childN : childName)
      {
      if (childN!="none") children.push_back(particleDb.particleType(childN.c_str()));
      }
    addDecayMode(parentType,children,branchingRatio,clebschGordanFlag!=0,verbose);
    }

  if (verbose)
    {
    printCR();
    printValue("Total index of particles read",particleDb.size());
    }
  inputFileDecays.close();
  finishDb(particleDb);
}

void ParticleDbParser::addDecayMode(ParticleType * parentType,
                                    const std::vector<ParticleType*> & children,
                                    double declaredBranchingRatio,
                                    bool clebschGordanFlag,
                                    bool verbose)
{
  if (children.size()<2 || children.size()>4)
    {
    String message = "Decay mode of "; message += parentType->name(); message += " with "; message += int(children.size());
    message += " products (2 to 4 expected)";
    throw Exception(message,__FUNCTION__);
    }
  if (verbose)
    {
    double massDiff = -parentType->mass();
    for (const ParticleType * child : children) massDiff += child->mass();
    if (massDiff>0)
      {
      printCR();
      printValue("Mass exception: parentName",parentType->name());
      for (const ParticleType * child : children) printValue("Mass exception: childName",child->name());
      printValue("Mass exception: massDiff",massDiff);
      }
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
    if (  (absolute(childType1->spin()  - childType2->spin()) < 0.01)
        &&  (absolute(childType1->mass() - childType2->mass()) < 0.01)
        &&  (absolute(childType1->isospin3()-childType2->isospin3())   > 0.01)
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

void ParticleDbParser::finishDb(ParticleDb & particleDb)
{
  particleDb.mapAntiParticleIndices();
  particleDb.setupDecayGenerator();
}

double ParticleDbParser::clebschGordan(double aJot,  double aEm,
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
    coef = power(-1, tIter);
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

double ParticleDbParser::deltaJ(double aJot1, double aJot2, double aJot)
{
  double res = std::sqrt( 1.0
                    * factorials[lrint( aJot1 + aJot2 - aJot)]
                    * factorials[lrint( aJot1 - aJot2 + aJot)]
                    * factorials[lrint(-aJot1 + aJot2 + aJot)]
                    / factorials[lrint( aJot1 + aJot2 + aJot + 1)]
                    );
  return res;
}





}
