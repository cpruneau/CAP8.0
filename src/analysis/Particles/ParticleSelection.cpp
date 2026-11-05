/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#include <cmath>
#include "ParticleSelection.hpp"
#include "Particle.hpp"
#include "ParticleFilter.hpp"
#include "EventFilter.hpp"

namespace CAP
{

namespace
{
//! Slots of the particle filters and of the event filters, and the highest order of the pT moments asked for.
struct Slots
{
  std::vector<const ParticleFilter *> particleFilters;
  std::vector<const EventFilter *> eventFilters;
  unsigned int ptOrder = 0;
};

Slots & slots()
{
  static Slots s;
  return s;
}
} // namespace

ParticleSelection::ParticleSelection(const ParticleFilter * filter, unsigned int slot)
:
_filter(filter),
_slot(slot),
_particles(),
_moments(),
_energy(0.0),
_charge(0.0),
_strangeness(0.0),
_baryon(0.0),
_ptSum(0.0),
_spherocity(0.0),
_hasSpherocity(false)
{
_particles.reserve(1000);
}

void ParticleSelection::clear()
{
_particles.clear();
for (auto & m : _moments) m = 0.0;
_energy = _charge = _strangeness = _baryon = _ptSum = 0.0;
_spherocity = 0.0;
_hasSpherocity = false;
}

void ParticleSelection::fill(const std::vector<Particle*> & particles, unsigned int ptOrder)
{
_moments.assign(ptOrder+1, 0.0);
_particles.clear();
_energy = _charge = _strangeness = _baryon = _ptSum = 0.0;
_hasSpherocity = false;
for (auto particle : particles)
  {
  if (_filter && !_filter->accept(*particle)) continue;
  _particles.push_back(particle);
  const VectorLorentz & momentum = particle->momentum();
  const ParticleType & type = particle->type();
  double pt = momentum.perp();
  double ptFac = 1.0;
  for (unsigned int k=0; k<=ptOrder; k++)
    {
    _moments[k] += ptFac;
    ptFac *= pt;
    }
  _energy      += momentum.t();
  _charge      += type.charge();
  _strangeness += type.strangeness();
  _baryon      += type.baryonNumber();
  _ptSum       += pt;
  }
}

double ParticleSelection::spherocity() const
{
if (_hasSpherocity) return _spherocity;
_hasSpherocity = true;
_spherocity = 0.0;
if (_particles.empty() || !(_ptSum > 0.0)) return _spherocity;
const int nSteps = 360;
const double step = 2.0*M_PI/nSteps;
double s0 = 1.0E10;
for (int k=0; k<nSteps; k++)
  {
  double nx = std::cos(k*step), ny = std::sin(k*step);
  double sum = 0.0;
  for (auto particle : _particles)
    {
    const VectorLorentz & momentum = particle->momentum();
    sum += std::fabs(ny*momentum.x() - nx*momentum.y());
    }
  double ratio = sum/_ptSum;
  if (ratio*ratio < s0) s0 = ratio*ratio;
  }
_spherocity = s0;
return _spherocity;
}

unsigned int ParticleSelection::slot(const ParticleFilter * filter, unsigned int ptOrder)
{
Slots & s = slots();
if (ptOrder > s.ptOrder) s.ptOrder = ptOrder;
for (unsigned int k=0; k<s.particleFilters.size(); k++)
  if (s.particleFilters[k] == filter) return k;
s.particleFilters.push_back(filter);
return (unsigned int)(s.particleFilters.size()-1);
}

unsigned int ParticleSelection::slot(const EventFilter * filter)
{
Slots & s = slots();
for (unsigned int k=0; k<s.eventFilters.size(); k++)
  if (s.eventFilters[k] == filter) return k;
s.eventFilters.push_back(filter);
return (unsigned int)(s.eventFilters.size()-1);
}

unsigned int ParticleSelection::nSlots()      { return (unsigned int) slots().particleFilters.size(); }
unsigned int ParticleSelection::nEventSlots() { return (unsigned int) slots().eventFilters.size(); }
unsigned int ParticleSelection::ptOrder()     { return slots().ptOrder; }

const ParticleFilter * ParticleSelection::filterAt(unsigned int slot)
{
return slot < slots().particleFilters.size() ? slots().particleFilters[slot] : nullptr;
}

void ParticleSelection::resetSlots()
{
slots() = Slots();
}

} // namespace CAP
