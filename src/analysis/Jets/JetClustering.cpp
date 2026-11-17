/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetClustering.hpp"
#include <algorithm>
#include <map>
#include <tuple>
#include "fastjet/ClusterSequence.hh"
#include "Event.hpp"
#include "Particle.hpp"

namespace Datum
{

namespace
{
struct CacheEntry
{
  unsigned long epoch = 0;
  unsigned long eventIndex = 0;
  std::vector<Jet> jets;
};

// the jets of an event, particle filter and jet settings (the object: the jets point to it); its definition is in the
// key too, so that settings changed between two calls are never served the jets of their former definition
typedef std::tuple<const Event*, const ParticleFilter*, const JetSettings*, int, double, int, double> CacheKey;

CacheKey keyOf(const Event & event, const ParticleFilter * filter, const JetSettings & settings)
{
  return CacheKey(&event,filter,&settings,int(settings.algorithm),settings.radius,int(settings.recombination),
                  settings.ptFloor);
}

std::map<CacheKey,CacheEntry> & cache()
{
  static std::map<CacheKey,CacheEntry> theCache;
  return theCache;
}

unsigned long & clusteringCount()
{
  static unsigned long count = 0;
  return count;
}
} // namespace

std::vector<Jet> JetClustering::cluster(const std::vector<Particle*> & particles, const JetSettings & settings)
{
  clusteringCount()++;
  std::vector<fastjet::PseudoJet> input;
  input.reserve(particles.size());
  for (unsigned int i=0; i<particles.size(); i++)
    {
    const MATH::VectorLorentz & p = particles[i]->momentum();
    fastjet::PseudoJet pseudoJet(p.x(),p.y(),p.z(),p.t());
    pseudoJet.set_user_index(int(i));
    input.push_back(pseudoJet);
    }
  fastjet::JetAlgorithm algorithm = settings.algorithm==JetSettings::Kt ? fastjet::kt_algorithm
                                  : settings.algorithm==JetSettings::CambridgeAachen ? fastjet::cambridge_algorithm
                                  : fastjet::antikt_algorithm;
  fastjet::RecombinationScheme scheme = settings.recombination==JetSettings::PtScheme ? fastjet::pt_scheme
                                                                                      : fastjet::E_scheme;
  fastjet::JetDefinition definition(algorithm,settings.radius,scheme);
  std::vector<Jet> jets;
  if (input.empty()) return jets;
  fastjet::ClusterSequence sequence(input,definition);
  // all the jets above the pT floor: no other selection here (that of the jets is the jet filters')
  std::vector<fastjet::PseudoJet> found = fastjet::sorted_by_pt(sequence.inclusive_jets(settings.ptFloor));
  for (const fastjet::PseudoJet & pseudoJet : found)
    {
    if (pseudoJet.perp()<settings.ptFloor) continue;
    Jet jet;
    jet.settings = &settings;
    jet.momentum.setTXYZ(pseudoJet.e(),pseudoJet.px(),pseudoJet.py(),pseudoJet.pz());
    for (const fastjet::PseudoJet & constituent : pseudoJet.constituents())
      {
      int index = constituent.user_index();
      if (index>=0 && index<int(particles.size())) jet.constituents.push_back(particles[index]);
      }
    jets.push_back(jet);
    }
  std::stable_sort(jets.begin(),jets.end(),[](const Jet & a, const Jet & b) { return a.pt()>b.pt(); });
  for (unsigned int k=0; k<jets.size(); k++) jets[k].rank = k+1;
  return jets;
}

const std::vector<Jet> & JetClustering::jets(const Event & event, const ParticleFilter * filter,
                                             const std::vector<Particle*> & particles, const JetSettings & settings)
{
  CacheEntry & entry = cache()[keyOf(event,filter,settings)];
  bool fresh = entry.epoch==event.epoch() && entry.eventIndex==event.eventIndex() && entry.epoch!=0;
  if (!fresh)
    {
    entry.jets = cluster(particles,settings);
    entry.epoch = event.epoch();
    entry.eventIndex = event.eventIndex();
    }
  return entry.jets;
}

void JetClustering::clearCache()
{
  cache().clear();
}

unsigned long JetClustering::nClusterings()
{
  return clusteringCount();
}

} // namespace Datum
