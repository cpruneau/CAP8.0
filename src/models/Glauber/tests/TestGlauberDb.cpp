/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestGlauberDb.cpp
//! @brief Tests of GlauberDb: the XML file of the nuclei (DB/Glauber/NucleusDb.xml) holds the built-in table, the
//! nuclei are written and read back, and nucleus types are set from them.
#include <cmath>
#include <cstdio>
#include <string>
#include "GlauberDb.hpp"
#include "GlauberExceptions.hpp"

using namespace CAP;

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
#define CHECK(condition) check((condition), #condition, __LINE__)

bool same(const GlauberDb::Nucleus & a, const GlauberDb::Nucleus & b)
{
  auto eq = [](double x, double y) { return std::fabs(x - y) <= 1e-12*(1 + std::fabs(x)); };
  return a.name == b.name && a.A == b.A && a.Z == b.Z && a.radialType == b.radialType && eq(a.radius, b.radius)
      && eq(a.skin, b.skin) && eq(a.w, b.w) && a.neutron == b.neutron && eq(a.neutronRadius, b.neutronRadius)
      && eq(a.neutronSkin, b.neutronSkin) && eq(a.neutronW, b.neutronW) && eq(a.beta2, b.beta2) && eq(a.beta4, b.beta4)
      && eq(a.r0, b.r0) && eq(a.r1, b.r1) && eq(a.r2, b.r2) && eq(a.a, b.a) && eq(a.b, b.b)
      && eq(a.maxRadius, b.maxRadius) && a.comment == b.comment;
}

bool sameAll(const std::vector<GlauberDb::Nucleus> & a, const std::vector<GlauberDb::Nucleus> & b)
{
  if (a.size() != b.size()) return false;
  for (std::size_t k=0; k<a.size(); k++) if (!same(a[k], b[k])) { std::printf("  differ: %s\n", a[k].name.data()); return false; }
  return true;
}
} // namespace

int main()
{
  std::vector<GlauberDb::Nucleus> table = GlauberDb::builtIn();
  CHECK(table.size() == 46);

  // the file delivered holds the built-in table
  GlauberDb file;
  file.setFileName(CAP_NUCLEUS_DB);
  CHECK(sameAll(file.nuclei(), table));

  // written and read back
  GlauberDb builtIn;
  builtIn.useBuiltIn();
  GlauberDb read;
  read.loadFromXml(XmlDocument::parse(builtIn.toXml().toString()));
  CHECK(sameAll(read.nuclei(), table));

  // nucleus types
  GlauberNucleusType pb;
  file.findNucleus("Pb", pb);
  CHECK(pb.nProtons() == 82 && pb.nNeutrons() == 126);
  CHECK(pb.radialDistType() == 1 && pb.radiusProton() == 6.62 && pb.skinProton() == 0.546);
  CHECK(pb.radiusNeutron() == 6.62 && pb.skinNeutron() == 0.546);
  CHECK(std::fabs(pb.maxRadius() - 3.0*1.2*std::pow(208.0, 0.33333)) < 1e-9);
  GlauberNucleusType pbpn;
  file.findNucleus("Pbpn", pbpn);
  CHECK(pbpn.radialDistType() == 11 && pbpn.radiusNeutron() == 6.69 && pbpn.skinNeutron() == 0.56);
  GlauberNucleusType u2;
  file.findNucleus("U2", u2);
  CHECK(u2.beta2() == 0.28 && u2.beta4() == 0.093 && u2.nNucleons() == 238);
  GlauberNucleusType cu;
  file.findNucleus("Cu2rw", cu);
  CHECK(cu.r1() == -0.00298083 && cu.radialDistType() == 14);
  GlauberNucleusType he3, h3;
  file.findNucleus("He3", he3);
  file.findNucleus("H3", h3);
  CHECK(he3.nProtons() == 2 && he3.nNeutrons() == 1);
  CHECK(h3.nProtons() == 1 && h3.nNeutrons() == 2);
  bool thrown = false;
  try { file.findNucleus("Unobtainium", cu); } catch (const NucleusNotFoundGlauberException &) { thrown = true; }
  CHECK(thrown);

  // a nucleus added in a file; malformed files rejected
  GlauberDb custom;
  custom.loadFromXml(XmlDocument::parse(
    "<CAP><NUCLEUS_DB><NUCLEUS name=\"Ru\" A=\"96\" Z=\"44\" radialType=\"8\" radius=\"5.085\" skin=\"0.46\" beta2=\"0.158\""
    " maxRadius=\"12\" comment=\"isobar\"/></NUCLEUS_DB></CAP>"));
  GlauberNucleusType ru;
  custom.findNucleus("Ru", ru);
  CHECK(ru.nProtons() == 44 && ru.beta2() == 0.158 && ru.beta4() == 0 && ru.maxRadius() == 12);
  thrown = false;
  try { custom.loadFromXml(XmlDocument::parse("<CAP><NUCLEUS_DB><NUCLEUS name=\"X\" A=\"2\" Z=\"3\" radialType=\"1\"/></NUCLEUS_DB></CAP>")); }
  catch (const std::exception &) { thrown = true; }
  CHECK(thrown);
  thrown = false;
  try { custom.loadFromXml(XmlDocument::parse("<CAP><NUCLEUS_DB><NUCLEUS name=\"X\" A=\"2\" Z=\"1\" radialType=\"1\"/>"
                                              "<NUCLEUS name=\"X\" A=\"2\" Z=\"1\" radialType=\"1\"/></NUCLEUS_DB></CAP>")); }
  catch (const std::exception &) { thrown = true; }
  CHECK(thrown);

  // no file: the built-in table
  GlauberDb missing;
  missing.setFileName("/nonexistent/NucleusDb.xml");
  CHECK(sameAll(missing.nuclei(), table));

  std::printf("TestGlauberDb: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures == 0 ? 0 : 1;
}
