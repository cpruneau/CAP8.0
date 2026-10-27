#!/usr/bin/env python3
# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""
VerifyParticleDb.py -- compare the CAP particle database with the Particle Data Group (PDG) database.

Usage (from anywhere; the script locates CAP from its own position):
    python3 VerifyParticleDb.py                        # check DB/ParticleData/ParticleDb.xml
    python3 VerifyParticleDb.py FILE.xml               # check another CAP particle database
    python3 VerifyParticleDb.py --no-decays            # properties only (faster)
    python3 VerifyParticleDb.py --csv report.csv       # also write every comparison to a CSV file
    python3 VerifyParticleDb.py --all                  # also list the values that agree
    python3 VerifyParticleDb.py --sigma 3 --rel 1e-4   # tolerances (see below)
    python3 VerifyParticleDb.py --update OUT.xml       # write a copy with the PDG values (see "Update" below)

For each particle of the CAP database (PARTICLE elements of the XML file), the PDG particle of the same Monte Carlo
code (pdg attribute) is looked up with the PDG Python API (import pdg; api = pdg.connect(); see https://pdg.lbl.gov/api)
and compared:
  * mass (GeV): a mismatch if it differs from the PDG value by more than --sigma times the PDG error AND by more than
    --rel relative (both must fail, so neither a tiny PDG error nor rounding in the CAP file is flagged on its own);
  * width (GeV): the same test. A CAP width of 0 for a particle whose PDG width is below 1e-10 GeV (weak or
    electromagnetic decays, long lifetimes) is the convention of the CAP database and reported as such, not as a
    mismatch; the PDG width of such particles is hbar/tau;
  * charge: must be equal;
  * spin J and isospin I: compared when the PDG gives a single value;
  * decay modes (DECAY elements, unless --no-decays): each CAP mode is matched to the PDG exclusive branching
    fraction with the same final-state particles, ignoring the signs of the charges (the PDG gives charge-conjugate
    modes together, e.g. K0L -> pi+- e-+ nu_e; the CAP modes of one such PDG mode are summed). The modes left (and
    those with Clebsch-Gordan weights, clebschGordan="true", whose branching ratio is that of the isospin family,
    shared among the charge states) are matched by isospin family: the PDG gives e.g. Delta(1232) -> N pi for all the
    charge states. Modes the PDG gives only as a limit or without a value, and modes it does not list, are reported
    apart. The decays of charm and beauty hadrons, taken from PYTHIA (ImportHeavyFlavour.py), are not compared (PYTHIA
    lists resonant channels apart where the PDG gives final-state totals) and never updated.
The report lists, per category, the particles and the values that need a look; the summary gives the counts.

Update (--update OUT.xml, OUT.xml may be the input file itself; every change is listed in OUT.xml.changes.txt):
  * MC codes: when the PDG charge of a code differs from the charge of the CAP particle while its name (plus, zero,
    minus, plusplus, _bar) agrees with its CAP charge, the codes of the charge states of that family (and sign) are
    reassigned among them, each charge state getting the code the PDG gives that charge; the codes in IDENTITY_FIXES
    (particles carrying the code of another state, decided from their names, masses, widths and spins) are applied
    too. Names, charges and decay tables are unchanged;
  * mass and width: the PDG value when the comparison fails (a width of 0 for a narrow particle is kept: CAP
    convention), the mass of the K0 for the K0S and K0L; spin J and isospin I: the PDG value when it is a single value;
  * branching ratios of the modes matched to a PDG value and out of tolerance: the PDG value (the modes of a CAP group
    are scaled, keeping their ratios; Clebsch-Gordan modes get the value of their family). Modes the PDG gives no
    value for are unchanged; the decayer normalizes the branching ratios of each particle.
The file is edited line by line (PARTICLE and DECAY elements), keeping its layout and order.

Requires the PDG Python API (pip install pdg); the PDG database comes with it (no network needed).
"""

import argparse
import csv
import math
import os
import sys
import warnings
import xml.etree.ElementTree as ET
from collections import defaultdict

CAP_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_DB = os.path.join(CAP_DIR, "DB", "ParticleData", "ParticleDb.xml")
#: Width below which a particle is stable for CAP's purposes (weak and electromagnetic decays): CAP sets 0 by convention.
NARROW_WIDTH = 1.0e-10


# ------------------------------------------------------------------------------------------------------------------
# CAP database
# ------------------------------------------------------------------------------------------------------------------

class CapParticle:
    """A PARTICLE element of the CAP database."""

    def __init__(self, element):
        self.name = element.get("name", "")
        self.title = element.get("title", "")
        self.pdg = int(element.get("pdg", "0"))
        self.charge = float(element.get("charge", "0"))
        self.mass = float(element.get("mass", "0"))
        self.width = float(element.get("width", "0"))
        self.spin = float(element.get("spin", "0"))
        self.isospin = float(element.get("isospin", "0"))
        self.decays = [(float(d.get("branchingRatio", "0")), d.get("clebschGordan", "false").lower() == "true",
                        d.get("products", "").split()) for d in element.findall("DECAY")]


def read_cap_db(path):
    root = ET.parse(path).getroot()
    return [CapParticle(e) for e in root.iter("PARTICLE")]


# ------------------------------------------------------------------------------------------------------------------
# PDG database
# ------------------------------------------------------------------------------------------------------------------

def value_or_none(getter):
    """Value of a PDG property, None if the PDG has none (no entry, no best value, or an undefined lifetime)."""
    try:
        v = getter()
    except Exception:            # PdgNoDataError, PdgAmbiguousValueError, ZeroDivisionError (lifetime of width 0), ...
        return None
    if v is None:
        return None
    try:
        v = float(v)
    except (TypeError, ValueError):
        return None
    return v if math.isfinite(v) else None


def quantum_number(text):
    """A PDG quantum number given as text ('1/2', '1', '0,1', '?') as a float, None if not a single value."""
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


def family(name):
    """Isospin family of a particle from its PDG name: the name without charge and 'bar' (p, n -> N; K+, K0, K(S)0,
    K(L)0 -> K; pi+, pi0 -> pi; Delta(1232)++ -> Delta(1232); e- -> e)."""
    n = name.replace("bar", "")
    while n and n[-1] in "+-0" and not n.endswith(")0") or (n.endswith(")0") and n[:-1].endswith(")")):
        n = n[:-1]
    n = n.rstrip("+-")
    if n.endswith(")0"):
        n = n[:-1]
    return {"p": "N", "n": "N", "K(S)": "K", "K(L)": "K"}.get(n, n)


class PdgLookup:
    """The PDG particles and decay products, with caches."""

    def __init__(self, api):
        self.api = api
        self.particles = {}
        self.item_codes = {}
        self.item_families = {}

    def particle(self, code):
        if code not in self.particles:
            try:
                self.particles[code] = self.api.get_particle_by_mcid(code)
            except Exception:
                self.particles[code] = None
        return self.particles[code]

    def item_code(self, item):
        """|MC code| of a decay product (charge sign ignored), None if it is not a single particle (e.g. 'neutrals')."""
        key = item.name
        if key in self.item_codes:
            return self.item_codes[key]
        code = None
        try:
            particles = list(item.particles) if item.has_particles else []
        except Exception:
            particles = []
        codes = {abs(p.mcid) for p in particles if p.mcid}
        if len(codes) == 1:
            code = codes.pop()
        else:                    # e.g. 'e-+' or 'mu+-': the particle of the first sign
            name = key.replace("+-", "+").replace("-+", "-")
            try:
                code = abs(self.api.get_particle_by_name(name).mcid)
            except Exception:
                code = None
        self.item_codes[key] = code
        return code

    def family_of_code(self, code, fallback):
        p = self.particle(code)
        return family(p.name) if p is not None else fallback

    def item_family(self, item):
        """Isospin family of a decay product (e.g. N, pi, K), None if it is not a particle or a family of particles."""
        key = item.name
        if key not in self.item_families:
            try:
                particles = list(item.particles) if item.has_particles else []
            except Exception:
                particles = []
            names = {family(p.name) for p in particles}
            if len(names) == 1:
                self.item_families[key] = names.pop()
            elif not particles and self.item_code(item) is not None:
                self.item_families[key] = family(key.replace("+-", "+").replace("-+", "-"))
            else:
                self.item_families[key] = None
        return self.item_families[key]

    def decay_modes(self, particle):
        """[(signature, families, value, is_limit, description)] of the exclusive branching fractions of a PDG particle
        (all levels: e.g. eta -> 2gamma is a mode of 'neutral modes'): signature, the sorted |MC codes| of the products
        (None if a product is not a single particle); families, the sorted isospin families of the products (None if a
        product is not a particle or a family of particles, e.g. 'neutrals')."""
        modes = []
        try:      # all the exclusive modes, nested ones included (exclusive_branching_fractions() gives the top level)
            inclusive = {bf.pdgid for bf in particle.inclusive_branching_fractions()}
            fractions = [bf for bf in particle.branching_fractions() if bf.pdgid not in inclusive]
        except Exception:
            return modes
        for bf in fractions:
            try:
                codes = []
                for product in bf.decay_products:
                    code = self.item_code(product.item)
                    if code is None:
                        codes = None
                        break
                    codes += [code] * int(product.multiplier or 1)
                families = []
                for product in bf.decay_products:
                    f = self.item_family(product.item)
                    if f is None:
                        families = None
                        break
                    families += [f] * int(product.multiplier or 1)
                value = value_or_none(lambda: bf.value)
                modes.append((tuple(sorted(codes)) if codes else None, tuple(sorted(families)) if families else None,
                              value, bool(bf.is_limit), bf.description))
            except Exception:
                continue
        return modes


# ------------------------------------------------------------------------------------------------------------------
# Comparisons
# ------------------------------------------------------------------------------------------------------------------

class Report:
    def __init__(self):
        self.rows = []                       # (category, particle, pdg, quantity, cap, pdg value, detail)
        self.counts = defaultdict(int)
        self.decay_updates = []              # (particle, "decay a b c + ...", CAP value, PDG value) out of tolerance

    def add(self, category, particle, quantity, cap_value, pdg_value, detail=""):
        self.rows.append((category, particle.name, particle.pdg, quantity, cap_value, pdg_value, detail))
        self.counts[category] += 1


def fmt(v):
    if v is None:
        return "-"
    if isinstance(v, float):
        return "%.6g" % v
    return str(v)


def compare_value(report, particle, quantity, cap_value, pdg_value, pdg_error, args):
    if pdg_value is None:
        report.add("no PDG value", particle, quantity, cap_value, None)
        return
    diff = cap_value - pdg_value
    scale = max(abs(pdg_value), 1e-300)
    sigmas = abs(diff) / pdg_error if pdg_error else float("inf") if diff else 0.0
    rel = abs(diff) / scale
    detail = "diff %+.4g (%.3g rel, %s sigma)" % (diff, rel, "%.1f" % sigmas if math.isfinite(sigmas) else "inf")
    if (pdg_error and sigmas <= args.sigma) or rel <= args.rel:
        report.add("ok", particle, quantity, cap_value, pdg_value, detail)
    else:
        report.add("MISMATCH", particle, quantity, cap_value, pdg_value, detail)


def heavy_flavour(code):
    """Whether an MC code is a hadron with a charm or beauty quark (quark digits nq1 nq2 nq3)."""
    a = abs(code)
    return a > 100 and any(d in (4, 5) for d in ((a // 1000) % 10, (a // 100) % 10, (a // 10) % 10))


def check_particle(report, lookup, particle, codes_of_names, args):
    pdg_particle = lookup.particle(particle.pdg)
    if pdg_particle is None:
        report.add("not in PDG", particle, "particle", particle.pdg, None, "no PDG particle of this MC code")
        return
    # mass
    mass = value_or_none(lambda: pdg_particle.mass)
    mass_error = value_or_none(lambda: pdg_particle.mass_error)
    if mass is None and particle.pdg in (310, 130, -310, -130):     # K0S, K0L: the PDG gives the mass of the K0
        k0 = lookup.particle(311)
        mass, mass_error = value_or_none(lambda: k0.mass), value_or_none(lambda: k0.mass_error)
    compare_value(report, particle, "mass", particle.mass, mass, mass_error, args)
    # width
    width = value_or_none(lambda: pdg_particle.width)
    width_error = value_or_none(lambda: pdg_particle.width_error)
    if width == 0.0:     # the PDG API gives 0 for a width it has no value for (unmeasured, or a stable particle)
        width = None
    if particle.width == 0.0 and width is not None and width < NARROW_WIDTH:
        report.add("width 0 by convention", particle, "width", 0.0, width, "PDG width hbar/tau, narrow")
    else:
        compare_value(report, particle, "width", particle.width, width, width_error, args)
    # charge
    charge = value_or_none(lambda: pdg_particle.charge)
    if charge is None:
        report.add("no PDG value", particle, "charge", particle.charge, None)
    elif abs(charge - particle.charge) > 0.01:
        report.add("MISMATCH", particle, "charge", particle.charge, charge)
    else:
        report.add("ok", particle, "charge", particle.charge, charge)
    # spin and isospin
    for quantity, cap_value, text in (("spin J", particle.spin, getattr(pdg_particle, "quantum_J", None)),
                                      ("isospin I", particle.isospin, getattr(pdg_particle, "quantum_I", None))):
        pdg_value = quantum_number(text)
        if pdg_value is None:
            report.add("no PDG value", particle, quantity, cap_value, text)
        elif abs(pdg_value - cap_value) > 0.01:
            report.add("MISMATCH", particle, quantity, cap_value, pdg_value)
        else:
            report.add("ok", particle, quantity, cap_value, pdg_value)
    # decay modes
    if args.no_decays or not particle.decays:
        return
    if heavy_flavour(particle.pdg):     # decay tables from PYTHIA (ImportHeavyFlavour.py): resonant channels apart,
        report.add("decay: PYTHIA table (not compared)", particle, "decays", len(particle.decays), None,
                   "charm/beauty hadron: PYTHIA lists resonant channels apart, the PDG gives final-state totals")
        return
    pdg_modes = lookup.decay_modes(pdg_particle)
    by_signature = defaultdict(list)
    for mode in pdg_modes:
        if mode[0] is not None:
            by_signature[mode[0]].append(mode)
    exact = defaultdict(lambda: [0.0, []])          # CAP modes without Clebsch-Gordan weights, by |codes|
    by_family = defaultdict(lambda: [0.0, 0.0, []])  # remaining CAP modes, by families: sum, max of the CG modes, labels
    all_family = defaultdict(lambda: [0.0, 0.0, []]) # all the CAP modes, by families (for the generic PDG modes)
    for br, clebsch, products in particle.decays:
        codes = [codes_of_names.get(p) for p in products]
        label = " ".join(products)
        if None in codes:
            report.add("decay: unknown product", particle, "decay " + label, br, None,
                       "products not in the CAP database: %s" % [p for p, c in zip(products, codes) if c is None])
            continue
        signature = tuple(sorted(abs(c) for c in codes))
        families = tuple(sorted(lookup.family_of_code(c, n) for c, n in zip(codes, products)))
        whole = all_family[families]
        if clebsch:
            whole[1] = max(whole[1], br)
        else:
            whole[0] += br
        whole[2].append(label + (" (CG)" if clebsch else ""))
        if not clebsch and signature in by_signature:
            exact[signature][0] += br
            exact[signature][1].append(label)
        else:
            entry = by_family[families]
            if clebsch:
                entry[1] = max(entry[1], br)    # Clebsch-Gordan modes share the branching ratio of their family
            else:
                entry[0] += br
            entry[2].append(label + (" (CG)" if clebsch else ""))
    used = set()

    def compare(quantity, br, matches, kind):
        values = [m for m in matches if m[2] is not None and not m[3]]
        if not values:
            limits = [m for m in matches if m[3]]
            report.add("decay: PDG limit or no value", particle, quantity, br,
                       limits[0][2] if limits else None, matches[0][4])
            return
        pdg_value = sum(m[2] for m in values)
        diff = br - pdg_value
        detail = "%s%s; diff %+.4g" % ("by family: " if kind == "family" else "", "; ".join(m[4] for m in values), diff)
        ok = abs(diff) <= max(args.br_abs, args.br_rel * pdg_value)
        report.add("ok" if ok else "MISMATCH", particle, quantity, br, pdg_value, detail)
        if not ok and br > 0:
            report.decay_updates.append((particle.name, quantity, br, pdg_value))

    for signature, (br, labels) in exact.items():
        matches = by_signature[signature]
        used.update(id(m) for m in matches)
        compare("decay " + " + ".join(labels), br, matches, "exact")
    for families, (total, cg, labels) in by_family.items():
        br = total + cg
        quantity = "decay " + " + ".join(labels)
        generic = [m for m in pdg_modes if m[1] == families and m[0] is None]
        if generic:      # e.g. f_1(1285) -> eta pi pi: all the CAP modes of the family (eta pi+ pi-, eta pi0 pi0)
            whole_total, whole_cg, whole_labels = all_family[families]
            compare("decay " + " + ".join(whole_labels), whole_total + whole_cg, generic, "family")
            continue
        matches = [m for m in pdg_modes if m[1] == families and id(m) not in used]
        if not matches:
            report.add("decay: not listed by PDG", particle, quantity, br, None, "families " + " ".join(families))
            continue
        compare(quantity, br, matches, "family")


# ------------------------------------------------------------------------------------------------------------------
# Update
# ------------------------------------------------------------------------------------------------------------------

#: Particles of the CAP database carrying the MC code of another state: name -> PDG code of the state it is (with the
#: reason). Decided from the names, titles, masses, widths and spins of the CAP entries (October 2026).
IDENTITY_FIXES = {
    "f01500zero":       (9030221,  "f_0(1500): 9000223 is the f_1(1510)"),
    "kaon1430zero":     (315,      "J=2, m=1.4256, width 0.0985: the K_2^*(1430); 10311 is the K_0^*(1430)"),
    "kaon1430zero_bar": (-315,     "J=2, m=1.4256, width 0.0985: the K_2^*(1430); -10311 is the K_0^*(1430)"),
    "kaon1430plus":     (325,      "J=2, m=1.4256, width 0.0985: the K_2^*(1430); 10321 is the K_0^*(1430)"),
    "kaon1430minus":    (-325,     "J=2, m=1.4256, width 0.0985: the K_2^*(1430); -10321 is the K_0^*(1430)"),
    "kaon1412zero":     (10311,    "J=0, m=1.412, width 0.294: the K_0^*(1430); 100313 is the K^*(1410)"),
    "kaon1412zero_bar": (-10311,   "J=0, m=1.412, width 0.294: the K_0^*(1430); -100313 is the K^*(1410)"),
    "kaon1410zero":     (100313,   "the K^*(1410)0 (9920313 is not a PDG code)"),
    "kaon1410zero_bar": (-100313,  "the K^*(1410)0 (-9920313 is not a PDG code)"),
}


def name_charge(name):
    """Charge given by the name of a CAP particle (suffix plusplus, plus, zero, minus, minusminus, and _bar), or None."""
    anti = name.endswith("_bar")
    stem = name[:-4] if anti else name
    for suffix, q in (("plusplus", 2), ("minusminus", -2), ("plus", 1), ("minus", -1), ("zero", 0)):
        if stem.endswith(suffix):
            return -q if anti else q
    return None


def code_fixes(particles, lookup, log):
    """New MC codes: {name: code}. Charge states permuted within their family (see the module documentation), then
    IDENTITY_FIXES. Raises ValueError if two particles would end up with the same code."""
    fixes = {}
    groups = defaultdict(list)
    for particle in particles:
        p = lookup.particle(particle.pdg)
        if p is None or p.charge is None:
            continue
        groups[(family(p.name), particle.pdg > 0)].append((particle, round(p.charge * 3)))
    for key, members in groups.items():
        wrong = [m for m in members if abs(m[1] - round(m[0].charge * 3)) > 0
                 and name_charge(m[0].name) is not None and abs(name_charge(m[0].name) - m[0].charge) < 0.01]
        if not wrong:
            continue
        code_of_charge = {}
        for particle, q in members:
            code_of_charge.setdefault(q, particle.pdg)
        for particle, _ in wrong:
            code = code_of_charge.get(round(particle.charge * 3))
            if code is None:
                log.append("  ! %s (%d): no state of charge %+g in its family %s; code left" %
                           (particle.name, particle.pdg, particle.charge, key[0]))
                continue
            fixes[particle.name] = code
    for name, (code, reason) in IDENTITY_FIXES.items():
        if any(p.name == name for p in particles):
            fixes[name] = code
    final = {p.name: fixes.get(p.name, p.pdg) for p in particles}
    seen = defaultdict(list)
    for name, code in final.items():
        seen[code].append(name)
    clashes = {c: n for c, n in seen.items() if len(n) > 1}
    if clashes:
        raise ValueError("codes used twice after the fixes: %s" % clashes)
    for particle in particles:
        if particle.name in fixes and fixes[particle.name] != particle.pdg:
            reason = IDENTITY_FIXES.get(particle.name, (None, "charge state: the PDG code of charge %+g" % particle.charge))[1]
            log.append("  code      %-22s %9d -> %-9d  %s" % (particle.name, particle.pdg, fixes[particle.name], reason))
    return {n: c for n, c in fixes.items() if c != next(p.pdg for p in particles if p.name == n)}


def property_fixes(particles, codes, lookup, args, log):
    """New attribute values: {name: {attribute: text}} (mass, width, spin, isospin, from the PDG particle of the code)."""
    fixes = defaultdict(dict)
    for particle in particles:
        code = codes.get(particle.name, particle.pdg)
        p = lookup.particle(code)
        if p is None:
            continue
        mass, mass_error = value_or_none(lambda: p.mass), value_or_none(lambda: p.mass_error)
        if mass is None and abs(code) in (310, 130):
            k0 = lookup.particle(311)
            mass, mass_error = value_or_none(lambda: k0.mass), value_or_none(lambda: k0.mass_error)

        def differs(cap, pdg_value, pdg_error):
            if pdg_value is None:
                return False
            diff = abs(cap - pdg_value)
            if pdg_error and diff <= args.sigma * pdg_error:
                return False
            return diff > args.rel * max(abs(pdg_value), 1e-300)

        if differs(particle.mass, mass, mass_error) and mass > 0:
            fixes[particle.name]["mass"] = "%.7g" % mass
        width, width_error = value_or_none(lambda: p.width), value_or_none(lambda: p.width_error)
        if width == 0.0:     # no PDG value (see check_particle())
            width = None
        if width is not None and not (particle.width == 0.0 and width < NARROW_WIDTH):
            if differs(particle.width, width, width_error):
                fixes[particle.name]["width"] = "%.6g" % width
        for attribute, cap_value, text in (("spin", particle.spin, getattr(p, "quantum_J", None)),
                                           ("isospin", particle.isospin, getattr(p, "quantum_I", None))):
            value = quantum_number(text)
            if value is not None and abs(value - cap_value) > 0.01:
                fixes[particle.name][attribute] = "%g" % value
        for attribute, text in fixes.get(particle.name, {}).items():
            old = {"mass": particle.mass, "width": particle.width, "spin": particle.spin, "isospin": particle.isospin}[attribute]
            log.append("  %-9s %-22s %12g -> %-12s (PDG %s)" % (attribute, particle.name, old, text, p.name))
    return fixes


def decay_fixes(particles, report, log):
    """New branching ratios: {particle: {products: text}} from the out-of-tolerance comparisons (report.decay_updates)."""
    fixes = defaultdict(dict)
    decays = {p.name: p.decays for p in particles}
    for name, quantity, cap_total, pdg_value in report.decay_updates:
        labels = [l.strip() for l in quantity[len("decay "):].split(" + ")]
        clebsch = [l[:-5] for l in labels if l.endswith(" (CG)")]
        plain = [l for l in labels if not l.endswith(" (CG)")]
        modes = {" ".join(products): br for br, _, products in decays[name]}
        plain_total = sum(modes.get(l, 0.0) for l in plain)
        cg_value = max([modes.get(l, 0.0) for l in clebsch], default=0.0)
        # the group total is plain_total + cg_value: scale it to the PDG value, keeping the ratios
        scale = pdg_value / (plain_total + cg_value) if plain_total + cg_value > 0 else 0.0
        for label in plain:
            fixes[name][label] = "%.6g" % (modes.get(label, 0.0) * scale)
        for label in clebsch:
            fixes[name][label] = "%.6g" % (cg_value * scale)
        log.append("  decay     %-22s %-60s %.4g -> %.4g" % (name, quantity[6:66], cap_total, pdg_value))
    return fixes


def write_updated(path_in, path_out, codes, properties, branchings):
    """Copy the database, changing the attributes of the PARTICLE and DECAY elements (line by line)."""
    import re
    lines = open(path_in, encoding="utf-8").read().split("\n")
    current = None
    changed = 0

    def set_attribute(line, attribute, value):
        new, n = re.subn(r'(\s%s=")[^"]*(")' % attribute, lambda m: m.group(1) + value + m.group(2), line, count=1)
        if n != 1:
            raise ValueError("attribute %s not found in: %s" % (attribute, line.strip()))
        return new

    for i, line in enumerate(lines):
        m = re.search(r'<PARTICLE\s+name="([^"]*)"', line)
        if m:
            current = m.group(1)
            new = line
            if current in codes:
                new = set_attribute(new, "pdg", str(codes[current]))
            for attribute, value in properties.get(current, {}).items():
                new = set_attribute(new, attribute, value)
            if new != line:
                lines[i] = new
                changed += 1
            continue
        m = re.search(r'<DECAY\b[^>]*\sproducts="([^"]*)"', line)
        if m and current in branchings and m.group(1) in branchings[current]:
            lines[i] = set_attribute(line, "branchingRatio", branchings[current][m.group(1)])
            changed += 1
    with open(path_out, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))
    return changed


# ------------------------------------------------------------------------------------------------------------------
# Main
# ------------------------------------------------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Compare the CAP particle database with the PDG database.")
    parser.add_argument("database", nargs="?", default=DEFAULT_DB, help="CAP particle database (XML)")
    parser.add_argument("--sigma", type=float, default=3.0, help="tolerance in PDG errors (default 3)")
    parser.add_argument("--rel", type=float, default=1e-4, help="relative tolerance (default 1e-4)")
    parser.add_argument("--br-abs", type=float, default=0.005, help="absolute tolerance on branching ratios (0.005)")
    parser.add_argument("--br-rel", type=float, default=0.05, help="relative tolerance on branching ratios (0.05)")
    parser.add_argument("--no-decays", action="store_true", help="do not compare the decay modes")
    parser.add_argument("--all", action="store_true", help="also list the values that agree")
    parser.add_argument("--csv", help="write every comparison to this CSV file")
    parser.add_argument("--update", metavar="OUT.xml", help="write a copy of the database with the PDG values")
    args = parser.parse_args()

    warnings.filterwarnings("ignore")          # SQLAlchemy warnings of the PDG API
    try:
        import pdg
    except ImportError:
        print("The PDG Python API is not installed (pip install pdg; see https://pdg.lbl.gov/api).", file=sys.stderr)
        return 1
    api = pdg.connect()
    particles = read_cap_db(args.database)
    codes_of_names = {p.name: p.pdg for p in particles}
    print("CAP database: %s (%d particles, %d decay modes)" % (args.database, len(particles),
                                                               sum(len(p.decays) for p in particles)))
    print("PDG database: %s" % str(api).splitlines()[0])
    print("Tolerances: %g sigma or %g relative (masses, widths); %g absolute or %g relative (branching ratios)\n"
          % (args.sigma, args.rel, args.br_abs, args.br_rel))

    report = Report()
    lookup = PdgLookup(api)
    log, codes = [], {}
    if args.update:          # the codes first: the comparisons (and the decays updated) are those of the right states
        codes = code_fixes(particles, lookup, log)
        for particle in particles:
            particle.pdg = codes.get(particle.name, particle.pdg)
        codes_of_names = {p.name: p.pdg for p in particles}
        print("Update: %d MC codes corrected before the comparisons\n" % len(codes))
    for k, particle in enumerate(particles):
        check_particle(report, lookup, particle, codes_of_names, args)
        if (k + 1) % 50 == 0:
            print("  ... %d/%d particles" % (k + 1, len(particles)), file=sys.stderr)

    order = ["MISMATCH", "not in PDG", "decay: unknown product", "decay: not listed by PDG",
             "decay: PDG limit or no value", "decay: PYTHIA table (not compared)", "width 0 by convention",
             "no PDG value", "ok"]
    for category in order:
        rows = [r for r in report.rows if r[0] == category]
        if not rows or (category == "ok" and not args.all):
            continue
        print("=" * 120)
        print("%s (%d)" % (category, len(rows)))
        print("=" * 120)
        print("%-22s %9s  %-34s %14s %14s  %s" % ("particle", "pdg", "quantity", "CAP", "PDG", "detail"))
        for _, name, code, quantity, cap_value, pdg_value, detail in rows:
            print("%-22s %9d  %-34s %14s %14s  %s" % (name, code, quantity[:34], fmt(cap_value), fmt(pdg_value), detail))
        print()

    print("Summary")
    for category in order:
        if report.counts[category]:
            print("  %-42s %6d" % (category, report.counts[category]))
    if args.csv:
        with open(args.csv, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["category", "particle", "pdg", "quantity", "CAP", "PDG", "detail"])
            writer.writerows(report.rows)
        print("\nAll comparisons written to %s" % args.csv)
    if args.update:
        log = ["Changes to %s (written to %s), %s" % (args.database, args.update, str(api).splitlines()[0]), ""] + log
        properties = property_fixes(particles, codes, lookup, args, log)
        branchings = decay_fixes(particles, report, log) if not args.no_decays else {}
        n = write_updated(args.database, args.update, codes, properties, branchings)
        log.insert(1, "%d MC codes, %d particles with new properties, %d particles with new branching ratios; "
                      "%d lines changed" % (len(codes), len(properties), len(branchings), n))
        with open(args.update + ".changes.txt", "w") as f:
            f.write("\n".join(log) + "\n")
        print("\nUpdated database written to %s (%s); changes listed in %s.changes.txt"
              % (args.update, log[1], args.update))
    return 0


if __name__ == "__main__":
    sys.exit(main())
