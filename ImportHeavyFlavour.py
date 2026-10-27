#!/usr/bin/env python3
# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""
ImportHeavyFlavour.py -- add the charm and beauty hadrons of the PDG to the CAP particle database.

Usage (from anywhere; the script locates CAP from its own position):
    python3 ImportHeavyFlavour.py                         # DB/ParticleData/ParticleDb.xml, decays from PYTHIA 8
    python3 ImportHeavyFlavour.py --database FILE.xml --output OUT.xml
    python3 ImportHeavyFlavour.py --pythia-xml PATH/ParticleData.xml --max-partonic 0.1

Particles (PDG Python API, import pdg; https://pdg.lbl.gov/api): every meson and baryon of the PDG with a Monte Carlo
code holding a charm or beauty quark, and its antiparticle, unless the database already has that code (a placeholder
entry of the database, given in REPLACE, is replaced, keeping its name). For each:
  * mass, width, charge, spin J and isospin I: PDG values (the width of a particle left without decay modes is 0
    when narrower than 1e-10 GeV, the convention of the database; a particle with decay modes keeps its width, which
    the decayer uses for its lifetime). A particle without a PDG mass is not added;
  * isospin3 and the quark content (quarks, antiquarks: d=1, u=10, s=100, c=1000, b=10000, as in the database): from
    the MC code; the charge they give is checked against the PDG charge;
  * name and title: in the style of the database (e.g. D1870plus, Dstar2010plus_bar; D^{*+}(2010)).
Decays: the channels of PYTHIA 8 (ParticleData.xml of the PYTHIA installation) that are on, whose products are all
particles of the database (or added here), 2 to 4 of them, below the mass of the parent; partonic channels (quarks,
gluons, diquarks, the 81 remnant code: decays left to PYTHIA's fragmentation) cannot be used: a hadron whose PYTHIA
decays (the channels on) are partonic for more than --max-partonic (0.1 by default) gets no decay modes (stable for
CAP's decayer). A particle without a PDG mass gets PYTHIA's (noted), and a particle with decay modes but no PDG width
PYTHIA's width (or hbar c / c tau), else a nominal 1 keV (prompt decays: NOMINAL_WIDTH). The branching ratios are written as PYTHIA gives them, not renormalized (the
decayer normalizes them). The antiparticles get the charge-conjugate channels.
Every particle added, and every hadron left without decays and why, is listed in OUT.xml.import.txt.
"""

import argparse
import glob
import math
import os
import re
import sys
import warnings
import xml.etree.ElementTree as ET

CAP_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_DB = os.path.join(CAP_DIR, "DB", "ParticleData", "ParticleDb.xml")
NARROW_WIDTH = 1.0e-10
HBARC_GEV_MM = 1.973269804e-13
#: Width given to a particle with decay modes whose width neither the PDG nor PYTHIA gives (keV-scale electromagnetic
#: or strong decays, e.g. D*0, B*): 1 keV, c tau ~ 0.2 nm, so that the decayer makes them decay promptly.
NOMINAL_WIDTH = 1.0e-6
#: Placeholder entries of the database replaced by the PDG particle of their code (keeping their names).
REPLACE = {5332}
#: Quark digit of the MC numbering scheme -> its unit in the quarks/antiquarks attributes of the database.
QUARK_UNIT = {1: 1, 2: 10, 3: 100, 4: 1000, 5: 10000}
QUARK_CHARGE = {1: -1.0/3, 2: 2.0/3, 3: -1.0/3, 4: 2.0/3, 5: -1.0/3}
GREEK = {"Lambda": "#Lambda", "Sigma": "#Sigma", "Xi": "#Xi", "Omega": "#Omega", "psi": "#psi", "chi": "#chi",
         "eta": "#eta", "Upsilon": "#Upsilon", "J/psi": "J/#psi"}


def find_pythia_xml():
    candidates = []
    for env in ("PYTHIA8DATA",):
        if os.environ.get(env):
            candidates.append(os.path.join(os.environ[env], "ParticleData.xml"))
    for env in ("CAP_PYTHIA8_PATH", "PYTHIA8"):
        if os.environ.get(env):
            candidates.append(os.path.join(os.environ[env], "share", "Pythia8", "xmldoc", "ParticleData.xml"))
    candidates += ["/usr/local/opt/pythia/share/Pythia8/xmldoc/ParticleData.xml",
                   "/opt/homebrew/opt/pythia/share/Pythia8/xmldoc/ParticleData.xml"]
    candidates += sorted(glob.glob(os.path.join(CAP_DIR, "pythia8*", "share", "Pythia8", "xmldoc", "ParticleData.xml")))
    for path in candidates:
        if os.path.exists(path):
            return path
    return None


def read_pythia(path):
    """{code: [(onMode, bRatio, [products])]} of PYTHIA's ParticleData.xml, and {code: attributes} (read with regular
    expressions: the file holds HTML entities that an XML parser rejects)."""
    text = open(path, encoding="utf-8", errors="replace").read()
    table, properties = {}, {}
    for m in re.finditer(r'<particle\b([^>]*?)/>', text):
        attrs = dict(re.findall(r'(\w+)="([^"]*)"', m.group(1)))
        properties[int(attrs["id"])] = attrs
    for m in re.finditer(r'<particle\b([^>]*?[^/])>(.*?)</particle>', text, re.S):
        attrs = dict(re.findall(r'(\w+)="([^"]*)"', m.group(1)))
        properties[int(attrs["id"])] = attrs
        channels = []
        for c in re.finditer(r'<channel\b([^>]*)/>', m.group(2)):
            a = dict(re.findall(r'(\w+)="([^"]*)"', c.group(1)))
            channels.append((int(a.get("onMode", "1")), float(a.get("bRatio", "0")),
                             [int(x) for x in a.get("products", "").split()]))
        table[int(attrs["id"])] = channels
    return table, properties


def partonic(code):
    """Whether a PYTHIA product is a parton, a diquark or a special code (left to the fragmentation)."""
    a = abs(code)
    if a <= 8 or a == 21 or 81 <= a <= 99:
        return True
    return 1000 < a < 10000 and (a // 10) % 10 == 0          # diquarks: 1103, 2101, 3303, ...


# ------------------------------------------------------------------------------------------------------------------
# Quark content, names and titles
# ------------------------------------------------------------------------------------------------------------------

def quark_content(code):
    """(quarks, antiquarks) as lists of quark digits, from the MC code (mesons: nq2 nq3; baryons: nq1 nq2 nq3)."""
    a = abs(code)
    nq1, nq2, nq3 = (a // 1000) % 10, (a // 100) % 10, (a // 10) % 10
    if nq1 == 0:                                   # meson q qbar: the up-type quark (2, 4) of nq2 is the quark
        if nq2 in (2, 4, 6):
            quarks, antiquarks = [nq2], [nq3]
        else:
            quarks, antiquarks = [nq3], [nq2]
    else:
        quarks, antiquarks = [nq1, nq2, nq3], []
    if code < 0:
        quarks, antiquarks = antiquarks, quarks
    return quarks, antiquarks


def encoded(digits):
    return sum(QUARK_UNIT[d] for d in digits)


def isospin3(quarks, antiquarks):
    nu = quarks.count(2) - antiquarks.count(2)
    nd = quarks.count(1) - antiquarks.count(1)
    return 0.5 * (nu - nd)


def charge_word(q):
    return {2: "plusplus", 1: "plus", 0: "zero", -1: "minus", -2: "minusminus"}[int(round(q))]


def split_pdg_name(name):
    """(base, bar, subscript, superscript, parenthesis, charge) of a PDG name, e.g. Dbar_s2^*(2573)- ->
    ('D', True, 's2', '*', '2573', '-')."""
    m = re.match(r"^(J/psi|[A-Za-z]+?)(bar)?(?:_([A-Za-z0-9]+))?(?:\^([*']))?(?:\(([^)]*)\))?([+\-0]*)$", name)
    if not m:
        return name, False, "", "", "", ""
    base, bar, sub, sup, paren, charge = m.groups()
    return base, bool(bar), sub or "", sup or "", paren or "", charge or ""


def cap_name(pdg_particle, mass):
    """Name in the style of the database: base, subscript, star/prime, mass in MeV, charge word (e.g. Dstar2010plus)."""
    base, _, sub, sup, _, _ = split_pdg_name(pdg_particle.name)
    stem = base.replace("/", "") + sub + {"*": "star", "'": "prime", "": ""}[sup]
    return "%s%04d%s" % (stem, int(round(1000 * mass)), charge_word(pdg_particle.charge))


def cap_title(pdg_particle):
    base, bar, sub, sup, paren, charge = split_pdg_name(pdg_particle.name)
    symbol = GREEK.get(base, base)
    if bar:
        symbol = "#bar{%s}" % symbol
    title = symbol
    if sub:
        title += "_{%s}" % sub
    up = (sup if sup else "") + charge
    if up:
        title += "^{%s}" % up.replace("'", "&apos;")
    if paren and re.match(r"^\d{4}$", paren):
        title += "(%s)" % paren
    elif paren:
        title += "(%s)" % paren
    return title


# ------------------------------------------------------------------------------------------------------------------
# PDG particles
# ------------------------------------------------------------------------------------------------------------------

def value(getter):
    try:
        v = getter()
        return float(v) if v is not None and math.isfinite(float(v)) else None
    except Exception:
        return None


def quantum(text):
    if text is None:
        return None
    text = str(text).strip()
    if not text or any(c in text for c in ",?<>"):
        return None
    try:
        if "/" in text:
            n, d = text.split("/")
            return float(n) / float(d)
        return float(text)
    except ValueError:
        return None


def heavy_hadrons(api):
    """PDG mesons and baryons with an MC code holding a c or b quark, positive codes (particles), sorted by code."""
    found = {}
    for group in api.get_particles():
        for p in group:
            try:
                code = p.mcid
                if not code or code < 0 or not (p.is_meson or p.is_baryon):
                    continue
                if any(d in (4, 5) for d in quark_content(code)[0] + quark_content(code)[1]):
                    found[code] = p
            except Exception:
                continue
    return [found[c] for c in sorted(found)]


# ------------------------------------------------------------------------------------------------------------------
# Main
# ------------------------------------------------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Add the PDG charm and beauty hadrons to the CAP particle database.")
    parser.add_argument("--database", default=DEFAULT_DB, help="CAP particle database (input)")
    parser.add_argument("--output", help="output file (default: the input file)")
    parser.add_argument("--pythia-xml", help="PYTHIA 8 ParticleData.xml (default: found from the environment)")
    parser.add_argument("--max-partonic", type=float, default=0.1,
                        help="maximum fraction of partonic PYTHIA decays for a hadron to get decays (0.1)")
    args = parser.parse_args()
    output = args.output or args.database

    warnings.filterwarnings("ignore")
    try:
        import pdg
    except ImportError:
        print("The PDG Python API is not installed (pip install pdg; see https://pdg.lbl.gov/api).", file=sys.stderr)
        return 1
    pythia_xml = args.pythia_xml or find_pythia_xml()
    if not pythia_xml:
        print("PYTHIA 8 ParticleData.xml not found: give it with --pythia-xml.", file=sys.stderr)
        return 1
    api = pdg.connect()
    pythia, pythia_properties = read_pythia(pythia_xml)
    text = open(args.database, encoding="utf-8").read()
    root = ET.fromstring(text)
    existing = {int(p.get("pdg")): p for p in root.iter("PARTICLE")}
    names = {p.get("name") for p in root.iter("PARTICLE")}
    log = ["Import of the PDG charm and beauty hadrons into %s" % output,
           "PDG: %s" % str(api).splitlines()[0], "PYTHIA decays: %s" % pythia_xml, ""]

    # ---- the particles
    entries = {}            # code -> dict of attributes (and "name"), for the particles added or replaced
    kept_names = {}         # code -> name of a replaced placeholder
    for p in heavy_hadrons(api):
        if p.mcid in existing and p.mcid not in REPLACE:
            continue
        mass = value(lambda: p.mass)
        mass_note = ""
        if mass is None and "m0" in pythia_properties.get(p.mcid, {}):
            mass = float(pythia_properties[p.mcid]["m0"])
            mass_note = "mass from PYTHIA (no PDG value)"
            log.append("  note:     %-22s %8d %s: %g GeV" % (p.name, p.mcid, mass_note, mass))
        if mass is None:
            log.append("  not added: %-22s %8d (no PDG or PYTHIA mass)" % (p.name, p.mcid))
            continue
        width = value(lambda: p.width)
        spin = quantum(getattr(p, "quantum_J", None))
        iso = quantum(getattr(p, "quantum_I", None))
        codes = [p.mcid]
        try:
            anti = api.get_particle_by_mcid(-p.mcid)
        except Exception:
            anti = None
        if anti is not None and anti.mcid == -p.mcid:
            codes.append(-p.mcid)
        for code in codes:
            q = p if code > 0 else anti
            quarks, antiquarks = quark_content(code)
            charge = value(lambda: q.charge)
            computed = sum(QUARK_CHARGE[d] for d in quarks) - sum(QUARK_CHARGE[d] for d in antiquarks)
            if charge is None or abs(charge - computed) > 0.01:
                log.append("  ! %s (%d): PDG charge %s, quark content gives %+.3g (not added)" % (q.name, code, charge, computed))
                continue
            if code in existing and code in REPLACE:
                name = existing[code].get("name")
                kept_names[code] = name
            else:
                name = cap_name(p, mass) + ("_bar" if code < 0 else "")
                if name in names:
                    name += "_%d" % abs(code)
            names.add(name)
            entries[code] = {"name": name, "title": cap_title(q), "pdg": str(code), "charge": "%g" % charge,
                             "mass": "%.7g" % mass, "massUncertainty": "0", "width": None,
                             "widthUncertainty": "0", "spin": "%g" % (spin if spin is not None else 0),
                             "isospin": "%g" % (iso if iso is not None else 0),
                             "isospin3": "%g" % isospin3(quarks, antiquarks),
                             "quarks": str(encoded(quarks)), "antiquarks": str(encoded(antiquarks)), "leptons": "0",
                             "_width": width, "_pdgname": q.name, "_spin_missing": spin is None}
    # ---- the decays
    known = {int(p.get("pdg")): (p.get("name"), float(p.get("mass"))) for p in root.iter("PARTICLE")}
    for code, e in entries.items():
        known[code] = (e["name"], float(e["mass"]))
    decays = {}
    for code in sorted(c for c in entries if c > 0):
        channels = pythia.get(code)
        reasons = ""
        if channels is None:
            reasons = "not in PYTHIA"
        else:
            total = sum(br for on, br, _ in channels if on in (1, 2))
            usable = []
            lost = {"partonic": 0.0, "off": 0.0, "product not in the database": 0.0, ">4 products": 0.0, "closed": 0.0}
            for on, br, products in channels:
                if on not in (1, 2):
                    lost["off"] += br
                elif any(partonic(x) for x in products):
                    lost["partonic"] += br
                elif any(x not in known for x in products):
                    lost["product not in the database"] += br
                elif not 2 <= len(products) <= 4:
                    lost[">4 products"] += br
                elif sum(known[x][1] for x in products) >= float(entries[code]["mass"]):
                    lost["closed"] += br
                else:
                    usable.append((br, products))
            fraction = sum(br for br, _ in usable) / total if total > 0 else 0.0
            lost_text = ", ".join("%s %.0f%%" % (k, 100 * v / total) for k, v in lost.items() if v > 0 and k != "off")
            if total <= 0 or lost["partonic"] / total > args.max_partonic or not usable:
                reasons = "partonic %.0f%% of the PYTHIA decays (usable %.0f%%; lost: %s)" % (
                    100 * lost["partonic"] / total if total > 0 else 0, 100 * fraction, lost_text)
            else:
                decays[code] = usable
                if -code in entries:
                    conj = []
                    for br, products in usable:
                        conj.append((br, [(-x if -x in known else x) for x in products]))
                    decays[-code] = conj
                log.append("  decays:   %-22s %8d %3d channels (%.0f%% of PYTHIA's%s)" %
                           (entries[code]["_pdgname"], code, len(usable), 100 * fraction,
                            "; lost: " + lost_text if lost_text else ""))
        if reasons:
            log.append("  stable:   %-22s %8d %s" % (entries[code]["_pdgname"], code, reasons))
    # widths: a narrow particle without decay modes gets 0 (convention of the database)
    for code, e in entries.items():
        width = e.pop("_width")
        if not width and code in decays:          # the decayer needs a lifetime: PYTHIA's width, else hbar c/(c tau)
            attrs = pythia_properties.get(abs(code), {})
            if float(attrs.get("mWidth", "0") or 0) > 0:
                width = float(attrs["mWidth"])
            elif float(attrs.get("tau0", "0") or 0) > 0:
                width = HBARC_GEV_MM / float(attrs["tau0"])
            if width:
                log.append("  note:     %-22s %8d width from PYTHIA (no PDG value): %.4g GeV" % (e["_pdgname"], code, width))
        if width is None:
            width = 0.0
        if code in decays and width <= 0.0:        # unmeasured (keV-scale) width: a nominal 1 keV, decays prompt
            width = NOMINAL_WIDTH
            log.append("  note:     %-22s %8d no PDG or PYTHIA width: nominal %g GeV (prompt decays)" % (e["_pdgname"], code, width))
        if code not in decays and width < NARROW_WIDTH:
            width = 0.0
        e["width"] = "%.6g" % width
    # ---- write
    order = ["name", "title", "pdg", "charge", "mass", "massUncertainty", "width", "widthUncertainty", "spin",
             "isospin", "isospin3", "quarks", "antiquarks", "leptons"]

    def element_text(code):
        e = entries[code]
        attrs = " ".join('%s="%s"' % (k, e[k].replace('"', "&quot;")) for k in order)
        modes = decays.get(code, [])
        if not modes:
            return "    <PARTICLE %s/>\n" % attrs
        lines = ["    <PARTICLE %s>\n" % attrs]
        for br, products in modes:
            lines.append('      <DECAY branchingRatio="%.7g" clebschGordan="false" products="%s"/>\n'
                         % (br, " ".join(known[x][0] for x in products)))
        lines.append("    </PARTICLE>\n")
        return "".join(lines)

    # replaced placeholders: remove their elements (single line, or until </PARTICLE>)
    for code, name in kept_names.items():
        pattern = re.compile(r'    <PARTICLE name="%s"[^\n]*?/>\n|    <PARTICLE name="%s"[^\n]*?>\n.*?    </PARTICLE>\n'
                             % (re.escape(name), re.escape(name)), re.S)
        text, n = pattern.subn("", text, count=1)
        if n != 1:
            raise ValueError("placeholder %s not found" % name)
        log.append("  replaced: %s (%d), a placeholder, by the PDG particle (name kept)" % (name, code))
    block = "".join(element_text(c) for c in sorted(entries, key=lambda c: (abs(c), c < 0)))
    end = "  </PARTICLE_DB>"
    if text.count(end) != 1:
        raise ValueError("%s: no single </PARTICLE_DB>" % args.database)
    text = text.replace(end, block + end)
    with open(output, "w", encoding="utf-8") as f:
        f.write(text)
    n_decays = sum(len(v) for v in decays.values())
    log.insert(4, "%d particles added (%d with decays, %d decay modes)" % (len(entries), len(decays), n_decays))
    log.append("")
    log.append("Particles added:")
    for code in sorted(entries, key=lambda c: (abs(c), c < 0)):
        e = entries[code]
        log.append("  %-26s %9d  %-22s m=%-10s w=%-11s J=%-4s I=%-4s q=%s/%s%s" % (
            e["name"], code, e["_pdgname"], e["mass"], e["width"], e["spin"], e["isospin"], e["quarks"],
            e["antiquarks"], "  (J unknown: 0)" if e["_spin_missing"] else ""))
    with open(output + ".import.txt", "w") as f:
        f.write("\n".join(log) + "\n")
    print("\n".join(log[:6]))
    print("Written %s; details in %s.import.txt" % (output, output))
    return 0


if __name__ == "__main__":
    sys.exit(main())
