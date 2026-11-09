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
_spherocity = spherocity(_particles, true);
return _spherocity;
}

double ParticleSelection::spherocity(const std::vector<Particle*> & particles, bool weighted, unsigned int maxExact,
                                     unsigned int nSteps)
{
// the transverse vectors (unit vectors for S1)
std::vector<double> x, y;
x.reserve(particles.size());
y.reserve(particles.size());
double sum = 0.0;
for (auto particle : particles)
  {
  if (!particle) continue;
  const VectorLorentz & p = particle->momentum();
  double px = p.x(), py = p.y(), pt = std::sqrt(px*px + py*py);
  if (!(pt > 0.0)) continue;
  if (!weighted) { px /= pt; py /= pt; pt = 1.0; }
  x.push_back(px);
  y.push_back(py);
  sum += pt;
  }
if (x.empty() || !(sum > 0.0)) return 0.0;
// sum of |p x n| for the axis n = (nx, ny)
auto crossSum = [&](double nx, double ny)
  {
  double s = 0.0;
  for (std::size_t j=0; j<x.size(); j++) s += std::fabs(ny*x[j] - nx*y[j]);
  return s;
  };
double best = 1.0E300;
if (x.size() <= maxExact)
  {
  // the minimum is along one of the particles (piecewise concave sum): exact
  for (std::size_t i=0; i<x.size(); i++)
    {
    double norm = std::sqrt(x[i]*x[i] + y[i]*y[i]);
    double s = crossSum(x[i]/norm, y[i]/norm);
    if (s < best) best = s;
    }
  }
else
  {
  // directions over pi (n and -n give the same sum)
  if (nSteps < 1) nSteps = 1;
  for (unsigned int k=0; k<nSteps; k++)
    {
    double phi = M_PI*k/nSteps;
    double s = crossSum(std::cos(phi), std::sin(phi));
    if (s < best) best = s;
    }
  }
double ratio = best/sum;
return 0.25*M_PI*M_PI*ratio*ratio;
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
