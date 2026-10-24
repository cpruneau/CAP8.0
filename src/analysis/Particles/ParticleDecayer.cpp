/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <cmath>
#include <utility>
#include "ParticleDecayer.hpp"
#include "PrintHelpers.hpp"

ClassImp(CAP::ParticleDecayer);

namespace CAP
{

double ParticleDecayer::twoPi = 2.0*3.1415927;

ParticleDecayer::ParticleDecayer()
:
random(gRandom)
{
}

void  ParticleDecayer::decay2(ParticleType  & parentType,
                              VectorLorentz & parentMomentum,
                              VectorLorentz & parentPosition,
                              ParticleType  & childType1,
                              VectorLorentz & p1,
                              VectorLorentz & r1,
                              ParticleType  & childType2,
                              VectorLorentz & p2,
                              VectorLorentz & r2)
{
  double e, vx, vy, vz, mp;
  double m1, m2, minMass,tau, taup, gamma, timeToDecay;
  mp       = parentMomentum.modulus();
  m1       = childType1.mass();
  m2       = childType2.mass();
  minMass  = m1 + m2;
  if (mp < minMass) mp = 1.02*minMass;
  e        = parentMomentum.t();
  vx       = parentMomentum.x()/e;
  vy       = parentMomentum.y()/e;
  vz       = parentMomentum.z()/e;
  gamma    = parentMomentum.gamma();
  //mWidth   = parentType.getWidth();
  tau      = parentType.lifeTime();
  taup     = gamma*tau;
  timeToDecay = -3.0E23 * taup * std::log(random->Rndm()); // fm
  r1 = parentPosition;
  r2 = parentPosition;
  VectorLorentz shift;
  shift.setTXYZ(timeToDecay,vx*timeToDecay,vy*timeToDecay,vz*timeToDecay);
  r1 += shift;
  r2 += shift;
  double temp = mp*mp - m1*m1 - m2*m2;

//  cout << "           mp: " << mp << endl;
//  cout << "           m1: " << m1 << endl;
//  cout << "           m2: " << m2 << endl;
//  cout << "      minMass: " << minMass << endl;
//  cout << "           Px: " << parentMomentum.x() << endl;
//  cout << "           Py: " << parentMomentum.y() << endl;
//  cout << "           Pz: " << parentMomentum.z() << endl;
//  cout << "         beta: " << parentMomentum.Beta() << endl;
//  cout << "           vx: " << vx << endl;
//  cout << "           vy: " << vy << endl;
//  cout << "           vz: " << vz << endl;
//  cout << "        gamma: " << gamma << endl;
//  cout << "       mWidth: " << mWidth << endl;
//  cout << "          tau: " << tau << endl;
//  cout << "         taup: " << taup << endl;
//  cout << "  timeToDecay: " << timeToDecay << endl;
//  cout << "         temp: " << temp << endl;

  double p_lrf = std::sqrt(temp*temp - 4*m1*m1*m2*m2)/(2*mp);
  // randomly pick emission angle of particle 1
  double phi      = twoPi*random->Rndm();
  double cosPhi   = std::cos(phi);
  double sinPhi   = std::sin(phi);
  double cosTheta = 2.*random->Rndm() - 1.0;
  double sinTheta = std::sqrt(1.00 - cosTheta*cosTheta);
  // compute daughter particles' energy and momentum in the parent rest frame
  double e1_lrf   = std::sqrt(p_lrf*p_lrf + m1*m1);
  double p1_lrf_x = p_lrf*sinTheta*cosPhi;
  double p1_lrf_y = p_lrf*sinTheta*sinPhi;
  double p1_lrf_z = p_lrf*cosTheta;
  double e2_lrf   = std::sqrt(p_lrf*p_lrf + m2*m2);
  double p2_lrf_x = -p1_lrf_x;
  double p2_lrf_y = -p1_lrf_y;
  double p2_lrf_z = -p1_lrf_z;
  p1.setTXYZ(e1_lrf,p1_lrf_x,p1_lrf_y,p1_lrf_z);
  p2.setTXYZ(e2_lrf,p2_lrf_x,p2_lrf_y,p2_lrf_z);
  // boost in "lab frame"
  p1.boost(vx,vy,vz);
  p2.boost(vx,vy,vz);
}

void  ParticleDecayer::decay3(ParticleType & parentType,
                              VectorLorentz & parentMomentum,
                              VectorLorentz & parentPosition,
                              ParticleType  & childType1,
                              VectorLorentz & p1,
                              VectorLorentz & r1,
                              ParticleType  & childType2,
                              VectorLorentz & p2,
                              VectorLorentz & r2,
                              ParticleType  & childType3,
                              VectorLorentz & p3,
                              VectorLorentz & r3)
{
  double e, vx, vy, vz, mp;
  double m1, m2, m3, minMass,tau, taup, gamma, timeToDecay;
  double e1_lrf, e2_lrf, e3_lrf, p1_lrf, p2_lrf, cos12_lrf;
  double v2;

  mp       = parentMomentum.modulus();
  m1       = childType1.mass();
  m2       = childType2.mass();
  m3       = childType3.mass();
  minMass  = m1 + m2 + m3;
  if (mp < minMass)
    {
    //cout << "<E> ParticleDecayer::decay3(..) Parent too light: M = " << mp << ", m1 = " << m1 << ", m2 = " << m2 << ", m3 = " << m3 << endl;
    mp = 1.02*minMass;
    }
  e        = parentMomentum.t();
  vx       = parentMomentum.x()/e;
  vy       = parentMomentum.y()/e;
  vz       = parentMomentum.z()/e;
  v2       = vx*vx + vy*vy + vz*vz;
  gamma    = parentMomentum.gamma();
  //mWidth   = parentType.getWidth();
  tau      = parentType.lifeTime();
  taup     = gamma*tau;
  timeToDecay = -3.0E23 * taup * std::log(random->Rndm()); // fm
  r1 = parentPosition;
  r2 = parentPosition;
  r3 = parentPosition;
  VectorLorentz shift;
  shift.setTXYZ(timeToDecay,vx*timeToDecay,vy*timeToDecay,vz*timeToDecay);
  r1 += shift;
  r2 += shift;
  r3 += shift;
  //
  double deltaM = mp - m1 - m2 - m3;

  do {
    do {
      e1_lrf = random->Rndm()*deltaM + m1;
      e2_lrf = random->Rndm()*deltaM + m2;
    } while (e1_lrf + e2_lrf > mp);
    p1_lrf = std::sqrt(e1_lrf*e1_lrf - m1*m1);
    p2_lrf = std::sqrt(e2_lrf*e2_lrf - m2*m2);
    e3_lrf = mp - e1_lrf - e2_lrf;
    cos12_lrf = (e3_lrf*e3_lrf - p1_lrf*p1_lrf - p2_lrf*p2_lrf - m3*m3)/(2.*p1_lrf*p2_lrf);
  } while (cos12_lrf < - 1.0 || cos12_lrf > 1.0);

  // compute the momentum of decay daughters
  double tp2_lrf_x = p2_lrf*std::sqrt(1. - cos12_lrf*cos12_lrf);
  double tp2_lrf_z = p2_lrf*cos12_lrf;
  double tp3_lrf_x = - tp2_lrf_x;
  double tp3_lrf_z = - (p1_lrf + tp2_lrf_z);
  double phi       = twoPi*random->Rndm();
  double ksi       = twoPi*random->Rndm();
  double cos_theta = 2.0*random->Rndm() - 1.0;
  double sin_phi   = std::sin(phi);
  double cos_phi   = std::cos(phi);
  double sin_ksi   = std::sin(ksi);
  double cos_ksi   = std::cos(ksi);
  double sin_theta = std::sqrt(1. - cos_theta*cos_theta);

  double p1_lrf_x = - p1_lrf*sin_theta*cos_ksi;
  double p1_lrf_y = p1_lrf*sin_theta*sin_ksi;
  double p1_lrf_z = p1_lrf*cos_theta;
  e1_lrf = std::sqrt(m1*m1 + p1_lrf_x*p1_lrf_x + p1_lrf_y*p1_lrf_y + p1_lrf_z*p1_lrf_z);
  double p2_lrf_x = (tp2_lrf_x*(cos_phi*cos_theta*cos_ksi - sin_phi*sin_ksi)  - tp2_lrf_z*sin_theta*cos_ksi);
  double p2_lrf_y = (tp2_lrf_x*(-cos_phi*cos_theta*sin_ksi - sin_phi*cos_ksi) + tp2_lrf_z*sin_theta*sin_ksi);
  double p2_lrf_z = tp2_lrf_x*(cos_phi*sin_theta) + tp2_lrf_z*cos_theta;
  e2_lrf = std::sqrt(m2*m2 + p2_lrf_x*p2_lrf_x + p2_lrf_y*p2_lrf_y + p2_lrf_z*p2_lrf_z);
  double p3_lrf_x = (tp3_lrf_x*(cos_phi*cos_theta*cos_ksi - sin_phi*sin_ksi) - tp3_lrf_z*sin_theta*cos_ksi);
  double p3_lrf_y = (tp3_lrf_x*(-cos_phi*cos_theta*sin_ksi - sin_phi*cos_ksi) + tp3_lrf_z*(sin_theta*sin_ksi));
  double p3_lrf_z = tp3_lrf_x*cos_phi*sin_theta + tp3_lrf_z*cos_theta;
  e3_lrf = std::sqrt(m3*m3 + p3_lrf_x*p3_lrf_x + p3_lrf_y*p3_lrf_y + p3_lrf_z*p3_lrf_z);

  double gamma_m_1 = gamma - 1.;
  double vp1 = vx*p1_lrf_x + vy*p1_lrf_y + vz*p1_lrf_z;
  double vp2 = vx*p2_lrf_x + vy*p2_lrf_y + vz*p2_lrf_z;
  double vp3 = vx*p3_lrf_x + vy*p3_lrf_y + vz*p3_lrf_z;

  p1.setTXYZ(gamma*(e1_lrf + vp1),
             p1_lrf_x + (gamma_m_1*vp1/v2 + gamma*e1_lrf)*vx,
             p1_lrf_y + (gamma_m_1*vp1/v2 + gamma*e1_lrf)*vy,
             p1_lrf_z + (gamma_m_1*vp1/v2 + gamma*e1_lrf)*vz);
  p2.setTXYZ(gamma*(e2_lrf + vp2),
             p2_lrf_x + (gamma_m_1*vp2/v2 + gamma*e2_lrf)*vx,
             p2_lrf_y + (gamma_m_1*vp2/v2 + gamma*e2_lrf)*vy,
             p2_lrf_z + (gamma_m_1*vp2/v2 + gamma*e2_lrf)*vz);
  p3.setTXYZ(gamma*(e3_lrf + vp3),
             p3_lrf_x + (gamma_m_1*vp3/v2 + gamma*e3_lrf)*vx,
             p3_lrf_y + (gamma_m_1*vp3/v2 + gamma*e3_lrf)*vy,
             p3_lrf_z + (gamma_m_1*vp3/v2 + gamma*e3_lrf)*vz);
}


void ParticleDecayer::decay4(ParticleType & parentType,
                             VectorLorentz & parentMomentum,
                             VectorLorentz & parentPosition,
                             ParticleType  & childType1,
                             VectorLorentz & p1,
                             VectorLorentz & r1,
                             ParticleType  & childType2,
                             VectorLorentz & p2,
                             VectorLorentz & r2,
                             ParticleType  & childType3,
                             VectorLorentz & p3,
                             VectorLorentz & r3,
                             ParticleType  & childType4,
                             VectorLorentz & p4,
                             VectorLorentz & r4
                             )
{
  double mp       = parentMomentum.modulus();
  double m1       = childType1.mass();
  double m2       = childType2.mass();
  double m3       = childType3.mass();
  double m4       = childType4.mass();
  double minMass  = m1 + m2 + m3 + m4;
  if (mp < minMass) mp = 1.02*minMass;
  double e        = parentMomentum.t();
  double vx       = parentMomentum.x()/e;
  double vy       = parentMomentum.y()/e;
  double vz       = parentMomentum.z()/e;
  double gamma    = parentMomentum.gamma();
  double tau      = parentType.lifeTime();
  double taup     = gamma*tau;
  double timeToDecay = -3.0E23 * taup * std::log(random->Rndm()); // fm
  VectorLorentz shift;
  shift.setTXYZ(timeToDecay,vx*timeToDecay,vy*timeToDecay,vz*timeToDecay);
  r1 = parentPosition; r1 += shift;
  r2 = parentPosition; r2 += shift;
  r3 = parentPosition; r3 += shift;
  r4 = parentPosition; r4 += shift;   // r4 was shifted without being set to the parent position

  // Four-body phase space (GENBOD method): parent -> (123) 4, (123) -> (12) 3, (12) -> 1 2, the invariant masses
  // m12 < m123 drawn uniformly in the allowed range and accepted with the weight p*(M;123,4) p*(m123;12,3) p*(m12;1,2)
  // (product of the two-body momenta, bounded by wMax). Each two-body decay is isotropic in the rest frame of its
  // mother; the products are then boosted to the frame of the mother's mother, up to the frame of the parent.
  // (This replaces a placeholder that gave the products their mass at rest in the parent frame.)
  double kinetic = mp - minMass;
  double wMax = twoBodyMomentum(mp,m1+m2+m3,m4)*twoBodyMomentum(mp-m4,m1+m2,m3)*twoBodyMomentum(mp-m4-m3,m1,m2);
  double m12  = m1+m2;
  double m123 = m1+m2+m3;
  for (int iTry=0; iTry<10000; iTry++)
    {
    double ra = random->Rndm();
    double rb = random->Rndm();
    if (ra>rb) std::swap(ra,rb);
    m12  = m1+m2    + ra*kinetic;
    m123 = m1+m2+m3 + rb*kinetic;
    double w = twoBodyMomentum(mp,m123,m4)*twoBodyMomentum(m123,m12,m3)*twoBodyMomentum(m12,m1,m2);
    if (random->Rndm()*wMax <= w) break;
    }
  VectorLorentz p123, p12;
  twoBodyDecay(mp,  m123,m4,p123,p4);   // parent rest frame
  twoBodyDecay(m123,m12, m3,p12, p3);   // (123) rest frame
  twoBodyDecay(m12, m1,  m2,p1,  p2);   // (12) rest frame
  // (12) frame -> (123) frame
  double bx = p12.x()/p12.t(), by = p12.y()/p12.t(), bz = p12.z()/p12.t();
  p1.boost(bx,by,bz);
  p2.boost(bx,by,bz);
  // (123) frame -> parent rest frame
  bx = p123.x()/p123.t(); by = p123.y()/p123.t(); bz = p123.z()/p123.t();
  p1.boost(bx,by,bz);
  p2.boost(bx,by,bz);
  p3.boost(bx,by,bz);
  // parent rest frame -> lab frame
  p1.boost(vx,vy,vz);
  p2.boost(vx,vy,vz);
  p3.boost(vx,vy,vz);
  p4.boost(vx,vy,vz);
}

double ParticleDecayer::twoBodyMomentum(double m, double ma, double mb)
{
  double a = (m*m-(ma+mb)*(ma+mb))*(m*m-(ma-mb)*(ma-mb));
  return a>0.0 ? std::sqrt(a)/(2.0*m) : 0.0;
}

void ParticleDecayer::twoBodyDecay(double m, double ma, double mb, VectorLorentz & pa, VectorLorentz & pb)
{
  double p        = twoBodyMomentum(m,ma,mb);
  double phi      = twoPi*random->Rndm();
  double cosTheta = 2.0*random->Rndm() - 1.0;
  double sinTheta = std::sqrt(1.0 - cosTheta*cosTheta);
  double px = p*sinTheta*std::cos(phi);
  double py = p*sinTheta*std::sin(phi);
  double pz = p*cosTheta;
  pa.setTXYZ(std::sqrt(p*p+ma*ma), px, py, pz);
  pb.setTXYZ(std::sqrt(p*p+mb*mb),-px,-py,-pz);
}

void  ParticleDecayer::decay2(Particle & parent,
                              Particle & child1,
                              Particle & child2)
{
  ParticleType & parentType     = parent.type();
  VectorLorentz & parentMomentum = parent.momentum();
  VectorLorentz & parentPosition = parent.position();
  ParticleType & childType1     = child1.type();
  VectorLorentz & p1             = child1.momentum();
  VectorLorentz & r1             = child1.position();
  ParticleType & childType2     = child2.type();
  VectorLorentz & p2             = child2.momentum();
  VectorLorentz & r2             = child2.position();
  decay2(parentType,parentMomentum,parentPosition,
         childType1,p1,r1,
         childType2,p2,r2);
}

void  ParticleDecayer::decay3(Particle & parent,
                              Particle & child1,
                              Particle & child2,
                              Particle & child3)
{
  ParticleType & parentType     = parent.type();
  VectorLorentz & parentMomentum = parent.momentum();
  VectorLorentz & parentPosition = parent.position();
  ParticleType & childType1     = child1.type();
  VectorLorentz & p1             = child1.momentum();
  VectorLorentz & r1             = child1.position();
  ParticleType & childType2     = child2.type();
  VectorLorentz & p2             = child2.momentum();
  VectorLorentz & r2             = child2.position();
  ParticleType & childType3     = child3.type();
  VectorLorentz & p3             = child3.momentum();
  VectorLorentz & r3             = child3.position();
  decay3(parentType,parentMomentum,parentPosition,
         childType1,p1,r1,
         childType2,p2,r2,
         childType3,p3,r3);
}

void  ParticleDecayer::decay4(Particle & parent,
                              Particle & child1,
                              Particle & child2,
                              Particle & child3,
                              Particle & child4)
{
  ParticleType & parentType     = parent.type();
  VectorLorentz & parentMomentum = parent.momentum();
  VectorLorentz & parentPosition = parent.position();
  ParticleType & childType1     = child1.type();
  VectorLorentz & p1             = child1.momentum();
  VectorLorentz & r1             = child1.position();
  ParticleType & childType2     = child2.type();
  VectorLorentz & p2             = child2.momentum();
  VectorLorentz & r2             = child2.position();
  ParticleType & childType3     = child3.type();
  VectorLorentz & p3             = child3.momentum();
  VectorLorentz & r3             = child3.position();
  ParticleType & childType4     = child4.type();
  VectorLorentz & p4             = child4.momentum();
  VectorLorentz & r4             = child4.position();
  decay4(parentType,parentMomentum,parentPosition,
         childType1,p1,r1,
         childType2,p2,r2,
         childType3,p3,r3,
         childType4,p4,r4);
}


} // namespace CAP

