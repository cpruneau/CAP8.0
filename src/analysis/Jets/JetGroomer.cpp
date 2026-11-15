/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetGroomer.hpp"
#include <algorithm>
#include <cmath>
#include "fastjet/ClusterSequence.hh"
#include "fastjet/tools/Pruner.hh"
#include "Exceptions.hpp"
#include "NameHelpers.hpp"
#include "Particle.hpp"

namespace CAP
{

namespace
{

fastjet::JetAlgorithm fastjetAlgorithm(JetSettings::Algorithm algorithm)
{
  return algorithm==JetSettings::Kt ? fastjet::kt_algorithm
       : algorithm==JetSettings::AntiKt ? fastjet::antikt_algorithm : fastjet::cambridge_algorithm;
}

//! The constituents of a jet as pseudojets (user index: position in jet.constituents).
std::vector<fastjet::PseudoJet> inputOf(const Jet & jet)
{
  std::vector<fastjet::PseudoJet> input;
  input.reserve(jet.constituents.size());
  for (unsigned int i=0; i<jet.constituents.size(); i++)
    {
    const MATH::VectorLorentz & p = jet.constituents[i]->momentum();
    fastjet::PseudoJet pj(p.x(),p.y(),p.z(),p.t());
    pj.set_user_index(int(i));
    input.push_back(pj);
    }
  return input;
}

//! Copy a pseudojet (its four-momentum and constituents) into a groomed jet.
void setFrom(GroomedJet & groomed, const fastjet::PseudoJet & pj, const Jet & jet)
{
  groomed.momentum.setTXYZ(pj.e(),pj.px(),pj.py(),pj.pz());
  groomed.constituents.clear();
  for (const fastjet::PseudoJet & c : pj.constituents())
    {
    int index = c.user_index();
    if (index>=0 && index<int(jet.constituents.size())) groomed.constituents.push_back(jet.constituents[index]);
    }
}

//! The subjets of a splitting, the harder (pT) first. @return false if the pseudojet has no parents
bool split(const fastjet::PseudoJet & j, fastjet::PseudoJet & harder, fastjet::PseudoJet & softer)
{
  if (!j.has_parents(harder,softer)) return false;
  if (harder.perp()<softer.perp()) std::swap(harder,softer);
  return true;
}

double massOf(const fastjet::PseudoJet & j)
{
  double m2 = j.m2();
  return m2>0.0 ? std::sqrt(m2) : 0.0;
}

double jetRadius(const Jet & jet)
{
  return jet.settings!=nullptr ? jet.settings->radius : 0.4;
}

} // namespace

double GroomedJet::mass() const
{
  double m2 = momentum.t()*momentum.t() - momentum.x()*momentum.x() - momentum.y()*momentum.y()
              - momentum.z()*momentum.z();
  return m2>0.0 ? std::sqrt(m2) : 0.0;
}

JetGroomer::JetGroomer()
:
ManagedObject()
{
  appendClassName("JetGroomer");
}

JetGroomer::Method JetGroomer::methodOf(const String & name)
{
  String n = name;
  n.toUpper();
  if (n=="SOFT_DROP" || n=="SOFTDROP" || n=="SD" || n=="MMDT") return SoftDrop;
  if (n=="TRIMMING" || n=="TRIM")                               return Trimming;
  if (n=="FILTERING" || n=="FILTER")                            return Filtering;
  if (n=="PRUNING" || n=="PRUNE")                               return Pruning;
  if (n=="DYNAMICAL" || n=="DYNAMICAL_GROOMING" || n=="DYG")   return Dynamical;
  throw Exception(String("Unknown grooming method: ")+name+" (SOFT_DROP, TRIMMING, FILTERING, PRUNING or DYNAMICAL)",
                  "JetGroomer::methodOf()");
}

String JetGroomer::methodName(Method method)
{
  switch (method)
    {
    case SoftDrop:  return String("SOFT_DROP");
    case Trimming:  return String("TRIMMING");
    case Filtering: return String("FILTERING");
    case Pruning:   return String("PRUNING");
    default:        return String("DYNAMICAL");
    }
}

void JetGroomer::configure(const String & ownerName, const String & typeName, const Configuration & configuration,
                           unsigned int index)
{
  auto key = [&](const char * name) { return createKey(ownerName,typeName,index,name); };
  auto d = [&](const char * name, double value) { return configuration.valueDouble(key(name),true,value); };
  method     = methodOf(configuration.valueString(key("METHOD"),true,String("SOFT_DROP")));
  recluster  = JetSettings::algorithmOf(configuration.valueString(key("RECLUSTER"),true,String("CA")));
  zCut       = d("Z_CUT",0.1);
  beta       = d("BETA",0.0);
  r0         = d("R0",0.0);
  mu         = d("MU",1.0);
  String m   = configuration.valueString(key("MODE"),true,String("GROOM"));
  m.toUpper();
  if (m=="GROOM" || m=="GROOMING") mode = Grooming;
  else if (m=="TAG" || m=="TAGGING") mode = Tagging;
  else throw ConfigurationException(key("MODE"),String("MODE ")+m+" (GROOM or TAG expected)",__FUNCTION__);
  rSub       = d("R_SUB",0.2);
  fCut       = d("F_CUT",0.03);
  rFilt      = d("R_FILT",0.3);
  nKeep      = configuration.valueInt(key("N_KEEP"),true,3);
  rCutFactor = d("R_CUT_FACTOR",0.5);
  a          = d("A",1.0);
  auto positive = [&](const char * name, double value)
    {
    if (!(value>0.0))
      throw ConfigurationException(key(name),String(name)+" of jet groomer "+this->name()+" must be positive",__FUNCTION__);
    };
  switch (method)
    {
    case SoftDrop:
      if (!(zCut>=0.0)) positive("Z_CUT",zCut);
      if (!(r0>=0.0)) positive("R0",r0);
      positive("MU",mu);
      break;
    case Trimming:  positive("R_SUB",rSub);  if (!(fCut>=0.0)) positive("F_CUT",fCut);  break;
    case Filtering: positive("R_FILT",rFilt);  positive("N_KEEP",nKeep);  break;
    case Pruning:   if (!(zCut>=0.0)) positive("Z_CUT",zCut);  positive("R_CUT_FACTOR",rCutFactor);  break;
    case Dynamical: if (!(a>=0.0)) positive("A",a);  break;
    }
}

GroomedJet JetGroomer::groom(const Jet & jet) const
{
  GroomedJet groomed;
  groomed.jet = &jet;
  std::vector<fastjet::PseudoJet> input = inputOf(jet);
  if (input.empty()) return groomed;                                  // nothing to groom: Failed
  const fastjet::JetAlgorithm algorithm = fastjetAlgorithm(recluster);
  switch (method)
    {
    case SoftDrop:
    case Dynamical:
      {
      // the binary tree of the constituents: one jet (radius large enough to hold them all)
      fastjet::ClusterSequence sequence(input,fastjet::JetDefinition(algorithm,fastjet::JetDefinition::max_allowable_R));
      std::vector<fastjet::PseudoJet> top = sequence.exclusive_jets(1);
      fastjet::PseudoJet j = top.front(), j1, j2;
      const double R = jetRadius(jet);
      if (method==SoftDrop)
        {
        const double R0 = r0>0.0 ? r0 : R;
        bool found = false;
        int n = 0;
        while (split(j,j1,j2))
          {
          const double pt12 = j1.perp()+j2.perp();
          const double z = pt12>0.0 ? j2.perp()/pt12 : 0.0;
          const double dR = j1.delta_R(j2);
          bool pass = z > zCut*std::pow(dR/R0,beta);
          if (pass && mu<1.0) pass = std::max(massOf(j1),massOf(j2)) < mu*massOf(j);
          if (pass)
            {
            n++;
            if (!found)
              {
              found = true;
              setFrom(groomed,j,jet);
              groomed.zg = z;  groomed.rg = dR;  groomed.ktg = j2.perp()*dR;
              }
            }
          j = j1;                                                       // the softer subjet is dropped
          }
        if (found)
          { groomed.status = GroomedJet::Groomed;  groomed.nSD = n; }
        else if (mode==Grooming)
          { groomed.status = GroomedJet::NoSplitting;  groomed.nSD = 0;  setFrom(groomed,j,jet); }
        else
          { groomed.status = GroomedJet::Failed;  groomed.nSD = 0; }
        }
      else
        {
        double best = -1.0;
        fastjet::PseudoJet parent;
        while (split(j,j1,j2))
          {
          const double pt12 = j1.perp()+j2.perp();
          const double z = pt12>0.0 ? j2.perp()/pt12 : 0.0;
          const double dR = j1.delta_R(j2);
          const double kappa = z*(1.0-z)*j.perp()*std::pow(dR/R,a);
          if (kappa>best)
            {
            best = kappa;
            parent = j;
            groomed.zg = z;  groomed.rg = dR;  groomed.ktg = j2.perp()*dR;
            }
          j = j1;
          }
        if (best>=0.0)
          { groomed.status = GroomedJet::Groomed;  setFrom(groomed,parent,jet); }
        else
          { groomed.status = GroomedJet::NoSplitting;  setFrom(groomed,top.front(),jet); }
        }
      break;
      }
    case Trimming:
    case Filtering:
      {
      const double radius = method==Trimming ? rSub : rFilt;
      fastjet::ClusterSequence sequence(input,fastjet::JetDefinition(algorithm,radius));
      std::vector<fastjet::PseudoJet> subjets = fastjet::sorted_by_pt(sequence.inclusive_jets());
      std::vector<fastjet::PseudoJet> kept;
      for (unsigned int k=0; k<subjets.size(); k++)
        {
        if (method==Trimming ? subjets[k].perp()>=fCut*jet.pt() : int(k)<nKeep) kept.push_back(subjets[k]);
        }
      if (kept.empty()) return groomed;                                 // Failed
      setFrom(groomed,fastjet::join(kept),jet);
      groomed.status = GroomedJet::Groomed;
      break;
      }
    case Pruning:
      {
      fastjet::ClusterSequence sequence(input,fastjet::JetDefinition(algorithm,fastjet::JetDefinition::max_allowable_R));
      fastjet::PseudoJet whole = sequence.exclusive_jets(1).front();
      fastjet::Pruner pruner(fastjet::JetDefinition(algorithm,fastjet::JetDefinition::max_allowable_R),zCut,rCutFactor);
      fastjet::PseudoJet pruned = pruner(whole);
      if (pruned.constituents().empty()) return groomed;                // Failed
      setFrom(groomed,pruned,jet);
      groomed.status = GroomedJet::Groomed;
      break;
      }
    }
  return groomed;
}

} // namespace CAP
