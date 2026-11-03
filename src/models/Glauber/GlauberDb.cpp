#include <cmath>
#include <cstdio>
#include "GlauberDb.hpp"
#include "Exceptions.hpp"
#include "FileSystem.hpp"
#include "MathBasicFunctions.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"

using CAP::MATH::power;

namespace CAP
{

  namespace
  {
  //! Number written with up to 10 significant digits
  String number(double v)
  {
  char buffer[32];
  std::snprintf(buffer, sizeof(buffer), "%.10g", v);
  return String(buffer);
  }
  } // namespace

  GlauberDb::GlauberDb()
  :
  ManagedObject(),
  _fileName("Glauber/NucleusDb.xml"),
  _loaded(false),
  _source(),
  _nuclei()
  {
  appendClassName("GlauberDb");
  setName("GlauberDb");
  }

  void GlauberDb::configure(const String & taskName, const String & objectType, const Configuration & configuration,
                            unsigned int index)
  {
  setFileName(configuration.valueString(createKey(taskName,objectType,index,"FILE"), true, "Glauber/NucleusDb.xml"));
  }

  //!
  //! The nuclei delivered with CAP (DB/Glauber/NucleusDb.xml): name, A, Z, radialType, radius, skin, w, neutron
  //! parameters (flag, radius, skin, w), beta2, beta4, r0, r1, r2, comment.
  //! Uranium (U): Heinz and Kuhlman, nucl-th/0411054, with radius = 6.8 x 0.91 and w = 6.8 x 0.26.
  //!
  std::vector<GlauberDb::Nucleus> GlauberDb::builtIn()
  {
  struct Row { const char * name; int A, Z, type; double radius, skin, w; bool neutron; double nRadius, nSkin, nW, beta2, beta4, r0, r1, r2; const char * comment; };
  static const Row rows[] = {
    {"p", 1, 1, 0, 0.234, 0, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"pg", 1, 1, 9, 0.514, 0, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"pdg", 1, 1, 10, 1, 0, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, "from arXiv:1101.5953"},
    {"dpf", 2, 1, 1, 0.01, 0.5882, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, "deuteron 2pf (tuned to useHulthen)"},
    {"dh", 2, 1, 3, 0.2283, 1.1765, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, "deuteron useHulthen free"},
    {"d", 2, 1, 4, 0.2283, 1.1765, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, "deuteron useHulthen constrained"},
    {"He3", 3, 2, 6, 0.00, 0.0000, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, "read configurations from file"},
    {"H3", 3, 1, 6, 0.00, 0.0000, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, "read configurations from file"},
    {"He4", 4, 2, 6, 0.00, 0.0000, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, "read configurations from file"},
    {"C", 12, 6, 6, 2.608, 0.513, -0.051, false, 0, 0, 0, 0, 0, 0, 0, 0, "read configurations from file"},
    {"O", 16, 8, 6, 2.608, 0.513, -0.051, false, 0, 0, 0, 0, 0, 0, 0, 0, "read configurations from file"},
    {"Opar", 16, 8, 1, 2.608, 0.513, -0.051, false, 0, 0, 0, 0, 0, 0, 0, 0, "WS parameterization"},
    {"Oho", 16, 8, 15, 1.833, 1.544, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, "Harmonic oscillator parameterization"},
    {"Al", 27, 13, 8, 3.34, 0.580, 0.0, false, 0, 0, 0, -0.448, 0.239, 0, 0, 0, ""},
    {"Si", 28, 14, 1, 3.34, 0.580, -0.233, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"Si2", 28, 14, 8, 3.34, 0.580, 0, false, 0, 0, 0, -0.478, 0.250, 0, 0, 0, ""},
    {"S", 32, 16, 2, 2.54, 2.191, 0.16, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"Ar", 40, 18, 1, 3.53, 0.542, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"Ca", 40, 20, 1, 3.766, 0.586, -0.161, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"Ni", 58, 28, 1, 4.309, 0.517, -0.1308, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"Cu", 63, 29, 1, 4.20, 0.596, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"Curw", 63, 29, 12, 4.20, 0.596, 0, false, 0, 0, 0, 0, 0, 1.00898, -0.000790403, -0.000389897, ""},
    {"Cu2", 63, 29, 8, 4.20, 0.596, 0, false, 0, 0, 0, 0.162, -0.006, 0, 0, 0, ""},
    {"Cu2rw", 63, 29, 14, 4.20, 0.596, 0, false, 0, 0, 0, 0.162, -0.006, 1.01269, -0.00298083, -9.97222e-05, ""},
    {"CuHN", 63, 29, 1, 4.28, 0.5, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, "from arXiv:0904.4080v1"},
    {"Xe", 129, 54, 1, 5.36, 0.59, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, "adapted from arXiv:1703.04278"},
    {"Xes", 129, 54, 1, 5.42, 0.57, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, "scale from Sb (Antimony, A=122, r=5.32) by 1.019 = (129/122)**0.333"},
    {"Xe2", 129, 54, 8, 5.36, 0.59, 0, false, 0, 0, 0, 0.161, -0.003, 0, 0, 0, "adapted from arXiv:1703.04278 and Z. Physik (1974) 270: 113"},
    {"Xe2a", 129, 54, 8, 5.36, 0.59, 0, false, 0, 0, 0, 0.18, 0, 0, 0, 0, "ALICE parameters (see public note from 2018 at https://cds.cern.ch/collection/ALICE%20Public%20Notes?ln=en)"},
    {"Xerw", 129, 54, 12, 5.36, 0.59, 0, false, 0, 0, 0, 0, 0, 1.00911, -0.000722999, -0.0002663, ""},
    {"Xesrw", 129, 54, 12, 5.42, 0.57, 0, false, 0, 0, 0, 0, 0, 1.0096, -0.000874123, -0.000256708, ""},
    {"Xe2arw", 129, 54, 14, 5.36, 0.59, 0, false, 0, 0, 0, 0.18, 0, 1.01246, -0.0024851, -5.72464e-05, ""},
    {"W", 186, 74, 1, 6.58, 0.480, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"Au", 197, 79, 1, 6.38, 0.535, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"Aurw", 197, 79, 12, 6.38, 0.535, 0, false, 0, 0, 0, 0, 0, 1.00899, -0.000590908, -0.000210598, ""},
    {"Au2", 197, 79, 8, 6.38, 0.535, 0, false, 0, 0, 0, -0.131, -0.031, 0, 0, 0, ""},
    {"Au2rw", 197, 79, 14, 6.38, 0.535, 0, false, 0, 0, 0, -0.131, -0.031, 1.01261, -0.00225517, -3.71513e-05, ""},
    {"AuHN", 197, 79, 1, 6.42, 0.44, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, "from arXiv:0904.4080v1"},
    {"Pb", 208, 82, 1, 6.62, 0.546, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"Pbrw", 208, 82, 12, 6.62, 0.546, 0, false, 0, 0, 0, 0, 0, 1.00863, -0.00044808, -0.000205872, "only Pb 207 was tested but should be the same for 208"},
    {"Pb*", 208, 82, 1, 6.624, 0.549, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"PbHN", 208, 82, 1, 6.65, 0.460, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"Pbpn", 208, 82, 11, 6.68, 0.447, 0, true, 6.69, 0.56, 0, 0, 0, 0, 0, 0, ""},
    {"Pbpnrw", 208, 82, 13, 6.68, 0.447, 0, true, 6.69, 0.56, 0, 0, 0, 0, 0, 0, ""},
    {"U", 238, 92, 5, 6.188, 0.54, 1.77, false, 0, 0, 0, 0, 0, 0, 0, 0, ""},
    {"U2", 238, 92, 8, 6.67, 0.44, 0, false, 0, 0, 0, 0.280, 0.093, 0, 0, 0, ""},
  };
  std::vector<Nucleus> nuclei;
  for (const Row & r : rows)
    {
    Nucleus n;
    n.name = r.name; n.A = r.A; n.Z = r.Z; n.radialType = r.type;
    n.radius = r.radius; n.skin = r.skin; n.w = r.w;
    n.neutron = r.neutron; n.neutronRadius = r.nRadius; n.neutronSkin = r.nSkin; n.neutronW = r.nW;
    n.beta2 = r.beta2; n.beta4 = r.beta4; n.r0 = r.r0; n.r1 = r.r1; n.r2 = r.r2;
    n.comment = r.comment;
    nuclei.push_back(n);
    }
  return nuclei;
  }

  void GlauberDb::useBuiltIn()
  {
  _nuclei = builtIn();
  _source = "built-in";
  _loaded = true;
  }

  void GlauberDb::loadFromXml(const String & fileName)
  {
  loadFromXml(XmlDocument::read(fileName));
  _source = fileName;
  }

  void GlauberDb::loadFromXml(const XmlDocument & document)
  {
  const XmlElement & root = document.root();
  const XmlElement * db = root.isNamed("NUCLEUS_DB") ? &root : root.child("NUCLEUS_DB");
  if (!db) throw Exception("no NUCLEUS_DB element", __FUNCTION__);
  std::vector<Nucleus> nuclei;
  for (const XmlElement * e : db->children("NUCLEUS"))
    {
    Nucleus n;
    n.name = e->attribute("name");
    if (n.name.isEmpty()) throw Exception("NUCLEUS without a name", __FUNCTION__);
    for (const Nucleus & other : nuclei)
      if (other.name == n.name) throw Exception(String("two nuclei named ") + n.name, __FUNCTION__);
    n.A          = int(e->attributeInt("A"));
    n.Z          = int(e->attributeInt("Z"));
    if (n.A < 1 || n.Z < 0 || n.Z > n.A) throw Exception(String("nucleus ") + n.name + String(": bad A or Z"), __FUNCTION__);
    n.radialType = int(e->attributeInt("radialType"));
    n.radius     = e->attributeDouble("radius", 0.0);
    n.skin       = e->attributeDouble("skin", 0.0);
    n.w          = e->attributeDouble("w", 0.0);
    n.neutron    = e->hasAttribute("neutronRadius") || e->hasAttribute("neutronSkin") || e->hasAttribute("neutronW");
    n.neutronRadius = e->attributeDouble("neutronRadius", n.neutron ? 0.0 : n.radius);
    n.neutronSkin   = e->attributeDouble("neutronSkin",   n.neutron ? 0.0 : n.skin);
    n.neutronW      = e->attributeDouble("neutronW",      n.neutron ? 0.0 : n.w);
    if (!n.neutron) n.neutronRadius = n.neutronSkin = n.neutronW = 0.0;
    n.beta2      = e->attributeDouble("beta2", 0.0);
    n.beta4      = e->attributeDouble("beta4", 0.0);
    n.r0         = e->attributeDouble("r0", 0.0);
    n.r1         = e->attributeDouble("r1", 0.0);
    n.r2         = e->attributeDouble("r2", 0.0);
    n.a          = e->attributeDouble("a", 0.0);
    n.b          = e->attributeDouble("b", 0.0);
    n.maxRadius  = e->attributeDouble("maxRadius", 0.0);
    n.comment    = e->attribute("comment", "");
    nuclei.push_back(n);
    }
  _nuclei = nuclei;
  _loaded = true;
  }

  XmlDocument GlauberDb::toXml() const
  {
  XmlDocument document("CAP");
  XmlElement & root = document.root();
  root.setAttribute("version", 1);
  root.setAttribute("type", "glauberNucleusDb");
  XmlElement & db = root.addChild("NUCLEUS_DB");
  for (const Nucleus & n : _nuclei)
    {
    XmlElement & e = db.addChild("NUCLEUS");
    e.setAttribute("name", n.name);
    e.setAttribute("A", n.A);
    e.setAttribute("Z", n.Z);
    e.setAttribute("radialType", n.radialType);
    e.setAttribute("radius", number(n.radius));
    e.setAttribute("skin", number(n.skin));
    e.setAttribute("w", number(n.w));
    if (n.neutron)
      {
      e.setAttribute("neutronRadius", number(n.neutronRadius));
      e.setAttribute("neutronSkin", number(n.neutronSkin));
      e.setAttribute("neutronW", number(n.neutronW));
      }
    const std::pair<const char *, double> optional[] = { {"beta2", n.beta2}, {"beta4", n.beta4}, {"r0", n.r0}, {"r1", n.r1},
                                                         {"r2", n.r2}, {"a", n.a}, {"b", n.b}, {"maxRadius", n.maxRadius} };
    for (const auto & o : optional) if (o.second != 0.0) e.setAttribute(o.first, number(o.second));
    if (!n.comment.isEmpty()) e.setAttribute("comment", n.comment);
    }
  return document;
  }

  void GlauberDb::ensureLoaded()
  {
  if (_loaded) return;
  if (_fileName.isEmpty() || _fileName == "NONE") { useBuiltIn(); return; }
  String path = FileSystem::configuredPath(_fileName, "CAP_DATABASE_PATH");
  if (!path.isEmpty() && !path.endsWith(".xml") && !FileSystem::isFile(path)) path += ".xml";
  if (path.isEmpty() || !FileSystem::isFile(path))
    {
    if (reportWarning(__FUNCTION__))
      std::cout << "Nucleus database " << _fileName << " not found (is CAP_DATABASE_PATH set?): built-in table used" << std::endl;
    useBuiltIn();
    return;
    }
  loadFromXml(path);
  if (reportInfo(__FUNCTION__)) std::cout << _nuclei.size() << " nuclei read from " << path << std::endl;
  }

  const std::vector<GlauberDb::Nucleus> & GlauberDb::nuclei()
  {
  ensureLoaded();
  return _nuclei;
  }

  const GlauberDb::Nucleus * GlauberDb::find(const String & name)
  {
  ensureLoaded();
  for (const Nucleus & n : _nuclei) if (n.name == name) return &n;
  return nullptr;
  }

  void GlauberDb::findNucleus(const String & name, GlauberNucleusType & nucleusType)
  {
  const Nucleus * n = find(name);
  if (!n) throw NucleusNotFoundGlauberException(name, String("Nucleus name not found in DB ") + _source, __FUNCTION__);
  nucleusType.setName(name);
  nucleusType.setNProtons(n->Z);
  nucleusType.setNNeutrons(n->A - n->Z);
  nucleusType.setMaxRadius(n->maxRadius > 0 ? n->maxRadius : 3.0*1.2*power(n->A,0.33333));
  nucleusType.setRadiusProton(n->radius);
  nucleusType.setSkinProton(n->skin);
  nucleusType.setWProton(n->w);
  nucleusType.setBeta2(n->beta2);
  nucleusType.setBeta4(n->beta4);
  nucleusType.setA(n->a);
  nucleusType.setB(n->b);
  nucleusType.setR0(n->r0);
  nucleusType.setR1(n->r1);
  nucleusType.setR2(n->r2);
  nucleusType.setRadialDistType(n->radialType);
  nucleusType.setRadiusNeutron(n->neutron ? n->neutronRadius : n->radius);
  nucleusType.setSkinNeutron(  n->neutron ? n->neutronSkin   : n->skin);
  nucleusType.setWNeutron(     n->neutron ? n->neutronW      : n->w);
  }

}  // namespace CAP
