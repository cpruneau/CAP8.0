/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetSettings.hpp"
#include <cstdio>
#include <string>
#include "Exceptions.hpp"
#include "NameHelpers.hpp"

namespace CAP
{

JetSettings::JetSettings()
:
ManagedObject(),
algorithm(AntiKt),
radius(0.4),
recombination(EScheme),
ptFloor(1.0)
{
  appendClassName("JetSettings");
}

void JetSettings::configure(const String & ownerName, const String & typeName, const Configuration & configuration,
                            unsigned int index)
{
  auto key = [&](const char * name) { return createKey(ownerName,typeName,index,name); };
  algorithm     = algorithmOf(configuration.valueString(key("ALGORITHM"),true,String("ANTIKT")));
  recombination = recombinationOf(configuration.valueString(key("RECOMBINATION"),true,String("E")));
  radius        = configuration.valueDouble(key("R"),true,0.4);
  ptFloor       = configuration.valueDouble(key("PT_FLOOR"),true,1.0);
  if (!(radius>0.0))
    throw ConfigurationException(key("R"),String("the radius of jet settings ")+name()+" must be positive",__FUNCTION__);
  if (!(ptFloor>=0.0))
    throw ConfigurationException(key("PT_FLOOR"),String("the pT floor of jet settings ")+name()+" must be positive or 0",
                                 __FUNCTION__);
}

String JetSettings::description() const
{
  char text[128];
  std::snprintf(text,sizeof(text),"%s R = %g, %s scheme, pT > %g GeV/c",
                algorithm==AntiKt ? "anti-kt" : algorithm==Kt ? "kt" : "Cambridge-Aachen",radius,
                recombination==EScheme ? "E" : "pT",ptFloor);
  return String(text);
}

JetSettings::Algorithm JetSettings::algorithmOf(const String & name)
{
  String n = name;
  n.toUpper();
  if (n=="ANTIKT" || n=="ANTI_KT" || n=="ANTI-KT") return AntiKt;
  if (n=="KT")                                      return Kt;
  if (n=="CA" || n=="CAMBRIDGE" || n=="CAMBRIDGEAACHEN" || n=="CAMBRIDGE_AACHEN") return CambridgeAachen;
  throw Exception(String("Unknown jet algorithm: ")+name+" (ANTIKT, KT or CA)","JetSettings::algorithmOf()");
}

JetSettings::Recombination JetSettings::recombinationOf(const String & name)
{
  String n = name;
  n.toUpper();
  if (n=="E" || n=="E_SCHEME")    return EScheme;
  if (n=="PT" || n=="PT_SCHEME")  return PtScheme;
  throw Exception(String("Unknown recombination scheme: ")+name+" (E or PT)","JetSettings::recombinationOf()");
}

String JetSettings::algorithmName(Algorithm algorithm)
{
  return String(algorithm==AntiKt ? "ANTIKT" : algorithm==Kt ? "KT" : "CA");
}

String JetSettings::recombinationName(Recombination recombination)
{
  return String(recombination==EScheme ? "E" : "PT");
}

String JetSettings::radiusLabel(double radius)
{
  char text[32];
  std::snprintf(text,sizeof(text),"%g",radius);
  std::string s(text);
  for (char & c : s) if (c=='.') c = 'p';
  return String((std::string("R")+s).c_str());
}

} // namespace CAP
