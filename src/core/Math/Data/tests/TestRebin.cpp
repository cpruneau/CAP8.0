/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestRebin.cpp
//! @brief Test program of Histogram::rebin() and Profile::rebin() (run by ctest).
//
// TestRebin -- rebinning of histograms of rank 1, 2, 3 and 4: bins merged n by n, the last bin of an axis covering
// m mod n bins scaled by n/(m mod n), uncertainties added in quadrature, entries summed, axes and labels; profiles
// merged by their sums. Prints each failed check and exits with the number of failures (0: all passed).
//
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include "Histogram.hpp"
#include "Profile.hpp"
#include "MathExceptions.hpp"

using namespace Datum;
using namespace Datum::MATH;

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

bool near(double a, double b, double tolerance=1.0E-6) { return std::fabs(a-b) <= tolerance*(1.0+std::fabs(b)); }

#define CHECK(c) check((c), #c, __LINE__)

//! Rank 1: 10 bins in [0, 10), bin i filled i+1 times with weight 1 (value i+1, uncertainty sqrt(i+1)).
Histogram rank1()
{
  Histogram h("h","title",10,0.0,10.0);
  h.setDimensionTitle(0,"x");
  for (unsigned int i=0; i<10; i++)
    for (unsigned int k=0; k<=i; k++) h.fill(i+0.5);
  return h;
}

void testRank1()
{
  Histogram h = rank1();
  // n = 2 divides 10: 5 bins of width 2 on the same axis
  Histogram r2 = h.rebin(2);
  CHECK(r2.rank()==1);
  CHECK(r2.nBins(0)==5);
  CHECK(near(r2.Partition::minimum(0),0.0) && near(r2.Partition::maximum(0),10.0));
  CHECK(r2.name()==String("h") && r2.title()==String("title") && r2.dimensionTitle(0)==String("x"));
  for (unsigned int j=0; j<5; j++)
    {
    double v = (2*j+1) + (2*j+2);
    CHECK(near(r2.binValueAt(j),v));
    CHECK(near(r2.binUncertaintyAt(j),std::sqrt(v)));
    CHECK(near(r2.binEntriesAt(j),v));
    }
  // n = 3: 10 = 3*3 + 1, so 4 bins of width 3, [0, 12); the last bin holds bin 9 (value 10) scaled by 3/1
  Histogram r3 = h.rebin(3);
  CHECK(r3.nBins(0)==4);
  CHECK(near(r3.Partition::minimum(0),0.0) && near(r3.Partition::maximum(0),12.0,1.0E-6));
  CHECK(near(r3.binValueAt(0),1+2+3));
  CHECK(near(r3.binValueAt(2),7+8+9));
  CHECK(near(r3.binValueAt(3),3.0*10.0));
  CHECK(near(r3.binUncertaintyAt(3),3.0*std::sqrt(10.0)));
  CHECK(near(r3.binEntriesAt(3),10.0));
  // n = 4: 10 = 2*4 + 2: the last bin is bins 8 and 9 (9 + 10) scaled by 4/2
  Histogram r4 = h.rebin(4);
  CHECK(r4.nBins(0)==3);
  CHECK(near(r4.binValueAt(2),2.0*(9.0+10.0)));
  // the total of the complete bins is kept
  CHECK(near(r4.binValueAt(0)+r4.binValueAt(1),1+2+3+4+5+6+7+8));
  // n = 1: a copy; n >= m: one bin, scaled by n/m when n > m
  Histogram r1 = h.rebin(1);
  CHECK(r1.nBins(0)==10);
  for (unsigned int i=0; i<10; i++) CHECK(near(r1.binValueAt(i),h.binValueAt(i)));
  Histogram r10 = h.rebin(10);
  CHECK(r10.nBins(0)==1 && near(r10.binValueAt(0),55.0));
  Histogram r20 = h.rebin(20);
  CHECK(r20.nBins(0)==1 && near(r20.binValueAt(0),2.0*55.0));
  CHECK(near(r20.Partition::maximum(0),20.0,1.0E-6));
  // empty bins stay empty
  Histogram e("e","e",6,0.0,6.0);
  e.fill(0.5);
  Histogram re = e.rebin(2);
  CHECK(near(re.binValueAt(0),1.0) && re.binEntriesAt(1)==0 && re.binEntriesAt(2)==0);
  // errors
  bool thrown = false;
  try { h.rebin(0); } catch (const std::exception &) { thrown = true; }
  CHECK(thrown);
  thrown = false;
  try { h.rebin(std::vector<unsigned int>{2,2}); } catch (const std::exception &) { thrown = true; }
  CHECK(thrown);
  // labels kept only along an axis not rebinned
  Histogram l("l","l",4,0.0,4.0);
  l.setBinLabel(1,"b");
  CHECK(l.rebin(1).binLabel(1)==String("b"));
  CHECK(l.rebin(2).binLabels(0).empty());
  // precision kept; the polymorphic copy is a Histogram
  Histogram d("d","d",4,0.0,4.0);
  d.setPrecision(Histogram::kDouble);
  CHECK(d.rebin(2).precision()==Histogram::kDouble);
  std::unique_ptr<Histogram> p(h.rebinned(std::vector<unsigned int>{3}));
  CHECK(p && dynamic_cast<Profile*>(p.get())==nullptr && near(p->binValueAt(3),30.0));
}

void testRank2()
{
  // 5 x 4 bins, bin (i,j) value 1 + i + 10 j (one fill with that weight)
  Histogram h("h2","h2",5,0.0,5.0,4,0.0,4.0);
  for (unsigned int i=0; i<5; i++)
    for (unsigned int j=0; j<4; j++) h.fill(i+0.5,j+0.5,1.0+i+10.0*j);
  // factor 2 along both: 3 x 2 bins; along x, 5 = 2*2 + 1 (last column scaled by 2)
  Histogram r = h.rebin(2);
  CHECK(r.rank()==2 && r.nBins(0)==3 && r.nBins(1)==2);
  CHECK(near(r.Partition::maximum(0),6.0,1.0E-6) && near(r.Partition::maximum(1),4.0));
  auto v = [&](unsigned int i, unsigned int j) { return 1.0+i+10.0*j; };
  CHECK(near(r.binValueAt(0,0),v(0,0)+v(1,0)+v(0,1)+v(1,1)));
  CHECK(near(r.binValueAt(1,1),v(2,2)+v(3,2)+v(2,3)+v(3,3)));
  CHECK(near(r.binValueAt(2,0),2.0*(v(4,0)+v(4,1))));
  CHECK(near(r.binValueAt(2,1),2.0*(v(4,2)+v(4,3))));
  // uncertainty: the weights in quadrature, scaled
  CHECK(near(r.binUncertaintyAt(2,0),2.0*std::sqrt(v(4,0)*v(4,0)+v(4,1)*v(4,1))));
  // factors per axis: 3 along x (5 = 3 + 2: last scaled by 3/2), 4 along y (one bin)
  Histogram s = h.rebin(std::vector<unsigned int>{3,4});
  CHECK(s.nBins(0)==2 && s.nBins(1)==1);
  double last = 0.0, first = 0.0;
  for (unsigned int j=0; j<4; j++) { last += v(3,j)+v(4,j); first += v(0,j)+v(1,j)+v(2,j); }
  CHECK(near(s.binValueAt(0,0),first));
  CHECK(near(s.binValueAt(1,0),1.5*last));
  // both axes incomplete: the factors multiply (x: 5 = 2*2 + 1, y: 4 = 3 + 1)
  Histogram t = h.rebin(std::vector<unsigned int>{2,3});
  CHECK(t.nBins(0)==3 && t.nBins(1)==2);
  CHECK(near(t.binValueAt(2,1),2.0*3.0*v(4,3)));
  CHECK(near(t.binValueAt(0,1),3.0*(v(0,3)+v(1,3))));
}

void testRank3And4()
{
  Histogram h("h3","h3",4,0.0,4.0,3,0.0,3.0,2,0.0,2.0);
  double total = 0.0;
  for (unsigned int i=0; i<4; i++)
    for (unsigned int j=0; j<3; j++)
      for (unsigned int k=0; k<2; k++) { double w = 1.0+i+4*j+12*k; h.fill(i+0.5,j+0.5,k+0.5,w); total += w; }
  Histogram r = h.rebin(2);   // 4 -> 2, 3 -> 2 (last scaled by 2), 2 -> 1
  CHECK(r.rank()==3 && r.nBins(0)==2 && r.nBins(1)==2 && r.nBins(2)==1);
  double complete = 0.0, partial = 0.0;
  for (unsigned int i=0; i<2; i++)
    for (unsigned int k=0; k<2; k++)
      {
      for (unsigned int j=0; j<2; j++) complete += 1.0+i+4*j+12*k;
      partial += 1.0+i+4*2+12*k;
      }
  CHECK(near(r.binValueAt(0,0,0),complete));
  CHECK(near(r.binValueAt(0,1,0),2.0*partial));
  Histogram one = h.rebin(std::vector<unsigned int>{4,3,2});
  CHECK(one.nBinsTotal()==1 && near(one.binValueAt(0,0,0),total));
  // rank 4
  Partition partition;
  for (unsigned int a=0; a<4; a++) partition.addDimension(3,0.0f,3.0f,"x");
  Histogram h4(partition);
  std::vector<double> x(4);
  double sum = 0.0;
  for (unsigned int b=0; b<81; b++)
    {
    unsigned int rest = b;
    for (unsigned int a=0; a<4; a++) { x[a] = rest%3 + 0.5; rest /= 3; }
    h4.fill(x,1.0+b);
    sum += 1.0+b;
    }
  Histogram r4 = h4.rebin(3);
  CHECK(r4.rank()==4 && r4.nBinsTotal()==1 && near(r4.binValueAt(std::vector<unsigned int>{0,0,0,0}),sum));
  Histogram r42 = h4.rebin(2);   // 3 -> 2 along each axis (the last scaled by 2)
  CHECK(r42.nBinsTotal()==16);
  // the bin (1,1,1,1) holds the original bin (2,2,2,2) only, scaled by 2^4
  CHECK(near(r42.binValueAt(std::vector<unsigned int>{1,1,1,1}),16.0*(1.0+80.0)));
}

void testProfile()
{
  // 5 bins; bin i: values i and i+2 with weights 1 (mean i+1)
  Profile p("p","p",5,0.0,5.0);
  for (unsigned int i=0; i<5; i++) { p.fill(i+0.5,double(i),1.0); p.fill(i+0.5,i+2.0,1.0); }
  Profile r = p.rebin(2);   // 3 bins: (0,1), (2,3), (4)
  CHECK(r.nBins(0)==3);
  CHECK(near(r.binValueAt(0),(1.0+2.0)/2.0));   // mean of 0, 2, 1, 3
  CHECK(near(r.binValueAt(1),(3.0+4.0)/2.0));
  CHECK(near(r.binValueAt(2),5.0));            // the last bin: its mean, not scaled
  CHECK(r.nEntries(0)==4 && r.nEntries(2)==2);
  // the same as filling the rebinned binning directly
  Profile direct("d","d",3,0.0,6.0);
  for (unsigned int i=0; i<5; i++) { direct.fill(i+0.5,double(i),1.0); direct.fill(i+0.5,i+2.0,1.0); }
  for (unsigned int j=0; j<3; j++)
    {
    CHECK(near(r.binValueAt(j),direct.binValueAt(j)));
    CHECK(near(r.binUncertaintyAt(j),direct.binUncertaintyAt(j)));
    }
  // the polymorphic copy of a profile is a profile
  const Histogram & h = p;
  std::unique_ptr<Histogram> c(h.rebinned(std::vector<unsigned int>{2}));
  CHECK(dynamic_cast<Profile*>(c.get())!=nullptr);
}
}

int main()
{
  testRank1();
  testRank2();
  testRank3And4();
  testProfile();
  std::printf("TestRebin: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
