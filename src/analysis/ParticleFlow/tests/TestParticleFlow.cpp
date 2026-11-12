/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestParticleFlow.cpp
//! @brief Tests of the flow analysis: the correlators of FlowCorrelator (integral and differential, with weights,
//! particles both POI and reference) against sums over all tuples of distinct particles; the cumulants of
//! FlowCumulants against their explicit formulas; and a closure test of ParticleFlowHistos: events with known v_2, v_3
//! (and a hole in the acceptance corrected by weights) give back v_n{2k} for 2k = 2..8, with and without a gap, the
//! differential v'_n{2k} and v_n{SP}.
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <random>
#include <string>
#include <vector>
#include "FlowCorrelator.hpp"
#include "FlowCumulants.hpp"
#include "ParticleFlowHistos.hpp"
#include "Configuration.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "Particle.hpp"
#include "NameHelpers.hpp"

using namespace CAP;
typedef std::complex<double> Complex;

namespace
{
int nChecks = 0, nFailures = 0;

void check(bool ok, const std::string & what, int line)
{
  nChecks++;
  if (ok) return;
  nFailures++;
  std::printf("FAILED (line %d): %s\n", line, what.c_str());
}

bool close(Complex a, Complex b, double tolerance=1.0E-9)
{
  return std::abs(a-b) <= tolerance*std::max(1.0,std::abs(b));
}

//! Sum over the tuples of distinct particles (indices into phi and w) of slot k taken from lists[k].
Complex bruteForce(const std::vector<double> & phi, const std::vector<double> & w,
                   const std::vector<std::vector<int>> & lists, const std::vector<int> & h)
{
  Complex sum(0.0,0.0);
  std::vector<int> chosen;
  std::function<void(unsigned int,Complex)> visit = [&](unsigned int slot, Complex product)
    {
    if (slot==lists.size()) { sum += product; return; }
    for (int i : lists[slot])
      {
      bool used = false;
      for (int c : chosen) if (c==i) used = true;
      if (used) continue;
      chosen.push_back(i);
      visit(slot+1,product*w[i]*std::polar(1.0,h[slot]*phi[i]));
      chosen.pop_back();
      }
    };
  visit(0,Complex(1.0,0.0));
  return sum;
}
} // namespace

int main()
{
  std::mt19937_64 random(12345);
  std::uniform_real_distribution<double> uniform(0.0,1.0);

  // ----- correlators against the sums over tuples of distinct particles
  {
  const int nParticles = 9;
  std::vector<double> phi(nParticles), w(nParticles);
  for (int i=0; i<nParticles; i++) { phi[i] = 6.283185307*uniform(random); w[i] = 0.5+uniform(random); }
  std::vector<int> all;
  for (int i=0; i<nParticles; i++) all.push_back(i);
  std::vector<int> ref = {0,1,2,3,4,5,6};          // reference particles
  std::vector<int> poi = {4,5,6,7,8};              // particles of interest: 4, 5, 6 are both
  FlowVector Q(12,6), P(12,1), Qpoi(12,6);
  for (int i : ref) Q.add(phi[i],w[i]);
  for (int i : poi) P.add(phi[i],w[i]);
  for (int i : poi) for (int j : ref) if (i==j) Qpoi.add(phi[i],w[i]);
  // mixed harmonics, 4 slots, and all the subsets
  std::vector<int> h = {2,3,-1,-4};
  std::vector<Complex> n;
  FlowCorrelator::subsetSums(Q,h,n);
  bool ok = true;
  for (unsigned int s=1; s<16; s++)
    {
    std::vector<std::vector<int>> lists;
    std::vector<int> hs;
    for (unsigned int k=0; k<4; k++) if (s & (1u<<k)) { lists.push_back(ref); hs.push_back(h[k]); }
    ok = ok && close(n[s],bruteForce(phi,w,lists,hs));
    }
  check(ok, "subset sums of 4 slots with mixed harmonics = sums over distinct tuples", __LINE__);
  // 6 slots, harmonics +2 -2 +2 -2 +2 -2: <2>, <4>, <6>
  h = FlowCorrelator::alternating(2,6);
  FlowCorrelator::subsetSums(Q,h,n);
  for (unsigned int m : {2u,4u,6u})
    {
    std::vector<std::vector<int>> lists(m,ref);
    std::vector<int> hs(h.begin(),h.begin()+m);
    check(close(n[FlowCorrelator::firstSlots(m)],bruteForce(phi,w,lists,hs)),
          "numerator of <"+std::to_string(m)+">_2 = sum over distinct tuples", __LINE__);
    }
  // differential: a POI in slot 0, reference particles in the others, all distinct (POI and reference overlapping)
  for (unsigned int m : {2u,4u,6u})
    {
    unsigned int others = FlowCorrelator::firstSlots(m)^1u;
    Complex fast = FlowCorrelator::differential(P,Qpoi,2,h,others,n);
    std::vector<std::vector<int>> lists(m,ref);
    lists[0] = poi;
    std::vector<int> hs(h.begin(),h.begin()+m);
    check(close(fast,bruteForce(phi,w,lists,hs)),
          "differential numerator of <"+std::to_string(m)+"'>_2 = sum over distinct tuples", __LINE__);
    }
  // all slots +3 (sub-events), differential with harmonic 3
  h = std::vector<int>(4,3);
  FlowCorrelator::subsetSums(Q,h,n);
  {
  unsigned int others = FlowCorrelator::firstSlots(3)^1u;
  Complex fast = FlowCorrelator::differential(P,Qpoi,3,h,others,n);
  std::vector<std::vector<int>> lists = {poi,ref,ref};
  check(close(fast,bruteForce(phi,w,lists,{3,3,3})), "differential numerator, equal harmonics", __LINE__);
  }
  check(std::fabs(FlowCorrelator::blockCoefficient(1)-1.0)<1e-12 && std::fabs(FlowCorrelator::blockCoefficient(4)+6.0)<1e-12,
        "block coefficients (-1)^{b-1}(b-1)!", __LINE__);
  }

  // ----- cumulants
  {
  std::vector<double> a = {1.0,-1.0,4.0,-33.0,456.0,-9460.0};
  bool ok = true;
  for (unsigned int k=1; k<=6; k++) ok = ok && std::fabs(FlowCumulants::coefficient(k)-a[k-1])<1e-9*std::fabs(a[k-1]);
  check(ok, "A_k = 1, -1, 4, -33, 456, -9460", __LINE__);
  std::vector<double> m = {0.013, 2.1E-4, 4.4E-6, 1.1E-7};
  double m1=m[0], m2=m[1], m3=m[2], m4=m[3];
  std::vector<double> c = FlowCumulants::cumulants(m);
  check(std::fabs(c[1]-(m2-2*m1*m1))<1e-15, "c{4} = <4> - 2<2>^2", __LINE__);
  check(std::fabs(c[2]-(m3-9*m2*m1+12*m1*m1*m1))<1e-17, "c{6} = <6> - 9<4><2> + 12<2>^3", __LINE__);
  check(std::fabs(c[3]-(m4-16*m3*m1-18*m2*m2+144*m2*m1*m1-144*std::pow(m1,4)))<1e-18,
        "c{8} = <8> - 16<6><2> - 18<4>^2 + 144<4><2>^2 - 144<2>^4", __LINE__);
  std::vector<double> mp = {0.02, 3.0E-4, 6.0E-6};
  double d4 = FlowCumulants::differentialCumulant(m,mp,2), d6 = FlowCumulants::differentialCumulant(m,mp,3);
  check(std::fabs(d4-(mp[1]-2*mp[0]*m1))<1e-16, "d{4} = <4'> - 2<2'><2>", __LINE__);
  check(std::fabs(d6-(mp[2]-6*mp[1]*m1-3*m2*mp[0]+12*mp[0]*m1*m1))<1e-18,
        "d{6} = <6'> - 6<4'><2> - 3<4><2'> + 12<2'><2>^2", __LINE__);
  // a single flow magnitude: v{2k} = v, v'{2k} = v'
  const double v = 0.08, vp = 0.15;
  std::vector<double> mv, mpv;
  for (int k=1; k<=5; k++) { mv.push_back(std::pow(v,2*k)); mpv.push_back(vp*std::pow(v,2*k-1)); }
  ok = true;
  for (unsigned int k=1; k<=5; k++)
    {
    bool defined;
    double vk = FlowCumulants::flow(FlowCumulants::cumulant(mv,k),k,defined);
    double vpk = FlowCumulants::differentialFlow(FlowCumulants::differentialCumulant(mv,mpv,k),vk,k);
    ok = ok && defined && std::fabs(vk-v)<1e-9 && std::fabs(vpk-vp)<1e-8;
    }
  check(ok, "constant flow: v{2k} = v and v'{2k} = v' for 2k = 2..10", __LINE__);
  bool defined = true;
  FlowCumulants::flow(+1.0E-6,2,defined);
  check(!defined, "c{4} > 0: v{4} not defined", __LINE__);
  }

  // ----- closure: ParticleFlowHistos with v2 = 0.1, v3 = 0.05, a hole in the acceptance corrected by weights
  {
  const String task = "Flow";
  Configuration configuration;
  ParticleFlowHistos::addDefaultConfiguration(configuration,task);
  configuration.addProperty(createKey(task,"HISTOGRAM","nHarmonics"),3);
  configuration.addProperty(createKey(task,"HISTOGRAM","maxOrder"),8);
  configuration.addProperty(createKey(task,"HISTOGRAM","pt_nbins"),4);
  configuration.addProperty(createKey(task,"HISTOGRAM","pt_min"),0.0);
  configuration.addProperty(createKey(task,"HISTOGRAM","pt_max"),2.0);
  configuration.addProperty(createKey(task,"HISTOGRAM","gapSize"),0.4);
  ParticleFlowHistos histos;
  histos.configure(task,"HISTOGRAM",configuration,0);
  histos.setPoiNames({"HighPt"});
  histos.create();
  const double v2 = 0.1, v3 = 0.05;
  const int nEvents = 3000, multiplicity = 300;
  std::exponential_distribution<double> ptDistribution(2.0);
  std::vector<Particle> particles(multiplicity);
  std::vector<Particle*> ref, poi;
  std::vector<double> refW;
  std::vector<std::vector<double>> poiW(1);
  for (int e=0; e<nEvents; e++)
    {
    double psi = 6.283185307*uniform(random);
    ref.clear(); poi.clear(); refW.clear(); poiW[0].clear();
    for (int i=0; i<multiplicity; i++)
      {
      double phi;
      for (;;)
        {
        phi = 6.283185307*uniform(random);
        double f = 1.0 + 2*v2*std::cos(2*(phi-psi)) + 2*v3*std::cos(3*(phi-psi));
        if (uniform(random)*1.3<f) break;
        }
      bool hole = phi>1.0 && phi<2.0;             // efficiency 0.6 there, 1 elsewhere
      if (hole && uniform(random)>0.6) continue;
      double w = hole ? 1.0/0.6 : 1.0;
      double pt = std::min(ptDistribution(random),1.999), eta = -2.0+4.0*uniform(random);
      double pz = pt*std::sinh(eta), m = 0.14;
      MATH::VectorLorentz momentum;
      momentum.setTXYZ(std::sqrt(m*m+pt*pt+pz*pz),pt*std::cos(phi),pt*std::sin(phi),pz);
      particles[i].setMomentum(momentum);
      ref.push_back(&particles[i]); refW.push_back(w);
      if (pt>0.5) { poi.push_back(&particles[i]); poiW[0].push_back(w); }   // POI: pT > 0.5, also reference
      }
    histos.fill(100.0,ref,refW,{&poi},poiW);
    }
  histos.createDerived();
  histos.calculateDerived();
  auto value = [&](const String & name, unsigned int bin, double & error)
    {
    const MATH::Histogram * h = histos.histogram(name);
    if (!h) { std::printf("no histogram %s\n",name.c_str()); error = 1.0; return -1.0; }
    error = h->binUncertaintyAtGlobal(bin);
    return h->binValueAtGlobal(bin);
    };
  auto agrees = [&](const String & name, unsigned int bin, double expected, double slack)
    {
    double error, x = value(name,bin,error);
    bool ok = std::fabs(x-expected) < 5.0*error + slack && error>0.0;
    if (!ok || std::getenv("VERBOSE")) std::printf("  %s[%u] = %g +- %g, expected %g\n",name.c_str(),bin,x,error,expected);
    return ok;
    };
  for (unsigned int k=1; k<=4; k++)
    {
    std::string o = std::to_string(2*k);
    check(agrees(histos.integralName("V",2*k,2),0,v2,0.002), "closure: v2{"+o+"} = v2", __LINE__);
    check(agrees(histos.integralName("V",2*k,3),0,v3,0.003) || k>2, "closure: v3{"+o+"} = v3", __LINE__);
    check(agrees(histos.integralName("VGap",2*k,2),0,v2,0.002), "closure: v2{"+o+",gap} = v2", __LINE__);
    if (k<=3)
      {
      check(agrees(histos.poiName(0,"V",2*k,2,false),0,v2,0.002), "closure: v'2{"+o+"} of the POI = v2", __LINE__);
      check(agrees(histos.poiName(0,"VGap",2*k,2,false),0,v2,0.003), "closure: v'2{"+o+",gap} of the POI = v2", __LINE__);
      }
    }
  double error;
  check(std::fabs(value(histos.integralName("V",2,1),0,error))<0.01, "closure: v1{2} = 0", __LINE__);
  check(agrees(histos.poiName(0,"VSP",0,2,false),0,v2,0.003), "closure: v2{SP} of the POI = v2", __LINE__);
  bool ok = true;
  for (unsigned int b=1; b<4; b++)             // pT bins above 0.5 (bin 0: [0, 0.5), no POI)
    ok = agrees(histos.poiName(0,"V",2,2,true),b,v2,0.002) && agrees(histos.poiName(0,"VSP",0,2,true),b,v2,0.003) && ok;
  check(ok, "closure: v'2{2}(pT) and v2{SP}(pT) = v2 in each pT bin", __LINE__);
  double e0;
  value(histos.poiName(0,"V",2,2,true),0,e0);
  check(e0==0.0, "closure: an empty pT bin has no v'", __LINE__);
  }

  std::printf("TestParticleFlow: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures==0 ? 0 : 1;
}
