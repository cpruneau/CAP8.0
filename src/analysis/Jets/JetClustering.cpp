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
#include <cmath>
#include <map>
#include <tuple>
#include "fastjet/ClusterSequence.hh"
#include "Event.hpp"
#include "Particle.hpp"

namespace CAP
{

namespace
{
struct CacheEntry
{
  unsigned long epoch = 0;
  unsigned long eventIndex = 0;
  std::vector<Jet> jets;
};

typedef std::tuple<const Event*, const ParticleFilter*, JetSettings> CacheKey;

struct KeyLess
{
  bool operator()(const CacheKey & a, const CacheKey & b) const
  {
  if (std::get<0>(a)!=std::get<0>(b)) return std::get<0>(a)<std::get<0>(b);
  if (std::get<1>(a)!=std::get<1>(b)) return std::get<1>(a)<std::get<1>(b);
  return std::get<2>(a)<std::get<2>(b);
  }
};

std::map<CacheKey,CacheEntry,KeyLess> & cache()
{
  static std::map<CacheKey,CacheEntry,KeyLess> theCache;
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
  std::vector<fastjet::PseudoJet> found = fastjet::sorted_by_pt(sequence.inclusive_jets(settings.ptMin));
  const double etaMax = settings.jetEtaMax();
  for (const fastjet::PseudoJet & pseudoJet : found)
    {
    if (pseudoJet.perp()<settings.ptMin) continue;
    if (std::fabs(pseudoJet.pseudorapidity())>=etaMax) continue;
    Jet jet;
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
  CacheEntry & entry = cache()[CacheKey(&event,filter,settings)];
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

} // namespace CAP
