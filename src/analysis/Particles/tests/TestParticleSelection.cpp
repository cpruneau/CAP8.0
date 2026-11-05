/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestParticleSelection.cpp
//! @brief Tests of the selections shared by the tasks (ParticleSelection, Event::selection(), Event::accepted(),
//! EventFilter on a named selection): slots, contents and global quantities against direct computations, one
//! selection per filter and event, made again after a change of the particles, results of the event filters kept and
//! evaluated again when the quantities of earlier versions change.
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "Configuration.hpp"
#include "Event.hpp"
#include "EventFilter.hpp"
#include "ManagedObjects.hpp"
#include "Particle.hpp"
#include "ParticleFilter.hpp"
#include "ParticleSelection.hpp"
#include "ParticleType.hpp"

using namespace CAP;

namespace
{
int nChecks = 0, nFailures = 0;

void check(bool ok, const char * what, int line)
{
  nChecks++;
  if (ok) return;
  nFailures++;
  std::printf("FAILED (line %d): %s\n", line, what);
}
#define CHECK(x) check((x), #x, __LINE__)

bool close(double a, double b) { return std::fabs(a-b) <= 1e-12*(1.0+std::fabs(a)+std::fabs(b)); }

//! Spherocity S0 computed directly.
double spherocity(const std::vector<Particle*> & particles)
{
  double sumPt = 0.0;
  for (auto p : particles) sumPt += p->momentum().perp();
  if (particles.empty() || sumPt <= 0.0) return 0.0;
  double best = 1.0E10;
  for (int k=0; k<360; k++)
    {
    double phi = k*2.0*M_PI/360.0, sum = 0.0;
    for (auto p : particles) sum += std::fabs(std::sin(phi)*p->momentum().x() - std::cos(phi)*p->momentum().y());
    double r = sum/sumPt;
    if (r*r < best) best = r*r;
    }
  return best;
}
} // namespace

int main()
{
  ParticleSelection::resetSlots();
  ParticleType plus, minus;
  plus.setName("plus");   plus.setCharge(1.0);  plus.setPdgCode(211);
  minus.setName("minus"); minus.setCharge(-1.0); minus.setPdgCode(-211);
  std::vector<Particle> store(40);
  Event event;
  for (unsigned int k=0; k<store.size(); k++)
    {
    double pt = 0.1 + 0.1*k, phi = 0.37*k, pz = 0.2*k - 3.0;
    double px = pt*std::cos(phi), py = pt*std::sin(phi);
    double e = std::sqrt(px*px + py*py + pz*pz + 0.14*0.14);
    store[k].set(k%3 ? &plus : &minus, px, py, pz, e, 0.0, 0.0, 0.0, 0.0);
    event.addParticle(&store[k]);
    }

  ParticleFilter low, high;
  low.setName("Low");   low.addCondition(ParticleFilter::kPt, 0.5, 2.0);
  high.setName("High"); high.addCondition(ParticleFilter::kPt, 1.0, 5.0);
  unsigned int sLow = ParticleSelection::slot(&low, 3), sHigh = ParticleSelection::slot(&high, 1);
  CHECK(sLow == 0 && sHigh == 1);
  CHECK(ParticleSelection::slot(&low) == sLow);                // a filter shared by several tasks: one slot
  CHECK(ParticleSelection::nSlots() == 2 && ParticleSelection::ptOrder() == 3);

  // contents and quantities
  ParticleSelection & a = event.selection(sLow);
  std::vector<Particle*> expected;
  double energy = 0.0, charge = 0.0, ptSum = 0.0;
  std::vector<double> moments(4, 0.0);
  for (auto p : event.particles())
    if (low.accept(*p))
      {
      expected.push_back(p);
      energy += p->momentum().t();
      charge += p->type().charge();
      double pt = p->momentum().perp();
      ptSum += pt;
      for (int k=0; k<4; k++) moments[k] += std::pow(pt, k);
      }
  CHECK(a.particles() == expected);
  CHECK(a.multiplicity() == double(expected.size()) && !a.empty());
  CHECK(close(a.energy(), energy) && close(a.charge(), charge) && close(a.ptSum(), ptSum));
  CHECK(a.ptMoments().size() == 4);
  bool momentsOk = true;
  for (int k=0; k<4; k++) momentsOk = momentsOk && close(a.ptMoments()[k], moments[k]);
  CHECK(momentsOk);
  CHECK(close(a.spherocity(), spherocity(expected)));
  CHECK(a.filter() == &low && a.slot() == sLow);

  // one selection per filter and event: the same object, not made again
  Particle * first = expected.front();
  VectorLorentz saved = first->momentum();
  first->momentum().setTXYZ(saved.t(), 100.0, saved.y(), saved.z());   // changed in place, not announced
  CHECK(&event.selection(sLow) == &a && event.selection(sLow).particles() == expected);
  event.invalidateSelections();                                // announced: made again
  CHECK(event.selection(sLow).multiplicity() == double(expected.size()) - 1.0);
  first->momentum() = saved;
  event.invalidateSelections();
  CHECK(event.selection(sLow).particles() == expected);
  unsigned long epoch = event.epoch();
  Particle extra;
  extra.set(&plus, 1.0, 0.0, 0.0, 1.01, 0.0, 0.0, 0.0, 0.0);
  event.addParticle(&extra);                                    // adding particles: made again
  CHECK(event.epoch() != epoch && event.selection(sLow).multiplicity() == double(expected.size()) + 1.0);

  // a slot added after the first selections: the earlier ones stay in place
  ParticleFilter all;
  all.setName("All");
  ParticleSelection * before = &event.selection(sHigh);
  unsigned int sAll = ParticleSelection::slot(&all);
  CHECK(&event.selection(sHigh) == before);
  CHECK(event.selection(sAll).multiplicity() == double(event.nParticles()));

  // event filters on named selections, kept per event
  ManagedObjects<ParticleFilter>::getObjectStore().push_back(&low);
  ManagedObjects<ParticleFilter>::getObjectStore().push_back(&high);
  Configuration configuration;
  configuration.addProperty("T:EVENT_FILTER_0:CONDITION:N", 2);
  configuration.addProperty("T:EVENT_FILTER_0:CONDITION_0:TYPE", String("DOUBLE_RANGE"));
  configuration.addProperty("T:EVENT_FILTER_0:CONDITION_0:SUBTYPE", String("MULT"));
  configuration.addProperty("T:EVENT_FILTER_0:CONDITION_0:SELECTION", String("Low"));
  configuration.addProperty("T:EVENT_FILTER_0:CONDITION_0:MINIMUM", 5.0);
  configuration.addProperty("T:EVENT_FILTER_0:CONDITION_0:MAXIMUM", 100.0);
  configuration.addProperty("T:EVENT_FILTER_0:CONDITION_1:TYPE", String("DOUBLE_RANGE"));
  configuration.addProperty("T:EVENT_FILTER_0:CONDITION_1:SUBTYPE", String("PTSUM"));
  configuration.addProperty("T:EVENT_FILTER_0:CONDITION_1:SELECTION", String("High"));
  configuration.addProperty("T:EVENT_FILTER_0:CONDITION_1:MINIMUM", 0.0);
  configuration.addProperty("T:EVENT_FILTER_0:CONDITION_1:MAXIMUM", 1000.0);
  EventFilter named;
  named.setName("Named");
  named.configure("T", "EVENT_FILTER", configuration, 0);
  CHECK(named.selectionName(0) == String("Low") && named.selectionName(1) == String("High"));
  unsigned int fNamed = ParticleSelection::slot(&named);
  CHECK(event.accepted(named, fNamed));
  configuration.addProperty("T:EVENT_FILTER_1:CONDITION:N", 1);
  configuration.addProperty("T:EVENT_FILTER_1:CONDITION_0:TYPE", String("DOUBLE_RANGE"));
  configuration.addProperty("T:EVENT_FILTER_1:CONDITION_0:SUBTYPE", String("CHARGE"));
  configuration.addProperty("T:EVENT_FILTER_1:CONDITION_0:SELECTION", String("High"));
  configuration.addProperty("T:EVENT_FILTER_1:CONDITION_0:MINIMUM", 1000.0);
  configuration.addProperty("T:EVENT_FILTER_1:CONDITION_0:MAXIMUM", 2000.0);
  EventFilter none;
  none.configure("T", "EVENT_FILTER", configuration, 1);
  CHECK(!event.accepted(none, ParticleSelection::slot(&none)));
  // a condition without SELECTION, an unknown particle filter
  configuration.addProperty("T:EVENT_FILTER_2:CONDITION:N", 1);
  configuration.addProperty("T:EVENT_FILTER_2:CONDITION_0:TYPE", String("DOUBLE_RANGE"));
  configuration.addProperty("T:EVENT_FILTER_2:CONDITION_0:SUBTYPE", String("MULT"));
  configuration.addProperty("T:EVENT_FILTER_2:CONDITION_0:MINIMUM", 0.0);
  configuration.addProperty("T:EVENT_FILTER_2:CONDITION_0:MAXIMUM", 1.0);
  bool thrown = false;
  try { EventFilter f; f.configure("T", "EVENT_FILTER", configuration, 2); } catch (const Exception &) { thrown = true; }
  CHECK(thrown);
  configuration.addProperty("T:EVENT_FILTER_2:CONDITION_0:SELECTION", String("Nope"));
  thrown = false;
  try { EventFilter f; f.configure("T", "EVENT_FILTER", configuration, 2); f.accept(event); }
  catch (const Exception &) { thrown = true; }
  CHECK(thrown);

  // event filters of earlier versions (MULT_k): evaluated again when the quantities change
  configuration.addProperty("T:EVENT_FILTER_3:CONDITION:N", 1);
  configuration.addProperty("T:EVENT_FILTER_3:CONDITION_0:TYPE", String("DOUBLE_RANGE"));
  configuration.addProperty("T:EVENT_FILTER_3:CONDITION_0:SUBTYPE", String("MULT_2"));
  configuration.addProperty("T:EVENT_FILTER_3:CONDITION_0:MINIMUM", 10.0);
  configuration.addProperty("T:EVENT_FILTER_3:CONDITION_0:MAXIMUM", 20.0);
  EventFilter legacy;
  legacy.configure("T", "EVENT_FILTER", configuration, 3);
  unsigned int fLegacy = ParticleSelection::slot(&legacy);
  event.setMult(2, 5.0);
  CHECK(!event.accepted(legacy, fLegacy));
  event.setMult(2, 15.0);                                       // set by GlobalAnalyzer later in the event
  CHECK(event.accepted(legacy, fLegacy));
  event.setMult(5, 1.0);                                        // k = 0..5 exist
  CHECK(event.mult(5) == 1.0);

  // a new event: everything made again
  event.reset();
  CHECK(event.selection(sLow).empty() && event.selection(sLow).multiplicity() == 0.0);
  CHECK(!event.accepted(named, fNamed));

  ManagedObjects<ParticleFilter>::getObjectStore().clear();
  std::printf("TestParticleSelection: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures == 0 ? 0 : 1;
}
