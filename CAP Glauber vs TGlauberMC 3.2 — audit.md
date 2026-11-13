# CAP Glauber vs TGlauberMC 3.2 — audit

Nov 12, 2026 · @Claude Pruneau

**Status, 2026-11-12:** every item below is fixed on branch glauber-fixes (commit 242f50f), except those under "Not audited". Re-run against the TGlauber port after the fixes, Pb–Pb, p–Pb, d–Au, Au2, U, U2 and O–O (from the configuration file) agree in ⟨Npart⟩, ⟨Ncoll⟩, ε2 and ε3 within statistics. Every nucleus type in NucleusDb.xml now runs.

## Verdict

For Pb–Pb with the standard Woods–Saxon (3pF) density and a hard-sphere or gamma NN profile, CAP reproduces TGlauberMC 3.2: Npart, Ncoll, ε2 and ε3 agree within statistics from b = 0 to 14 fm. That path is what glauber.ini runs (ω = 0).

Outside it, several things are wrong. Nucleus B always gets nucleus A's name, so every asymmetric system runs as A+A. With ω = 1, the profile's effective σ\_NN is 36.8 mb instead of 67.6 mb, and ω = 1 is the code's default. The deuteron, uranium and every deformed nucleus are placed wrongly. The hard-core option cannot run.

Each finding was confirmed by running CAP's own classes, not only by reading the code (Method, at the end).

## Numerical comparison

Pb–Pb, σ\_NN = 67.6 mb, hard sphere, no minimum distance in either code (CAP cannot enforce one, bug 6). Events at a fixed b; only events with at least one collision count, as in TGlauber. CAP: 4,000 events (28,721 at b = 14 fm); TGlauber port: 2,000 (7,663 at b = 14 fm).

| b (fm) | ⟨Npart⟩ CAP | ⟨Npart⟩ TGlauber | ⟨Ncoll⟩ CAP | ⟨Ncoll⟩ TGlauber | ⟨ε2⟩ CAP | ⟨ε2⟩ TGlauber | ⟨ε3⟩ CAP | ⟨ε3⟩ TGlauber |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | 409.0 | 409.1 | 2019.6 | 2023.3 | 0.059 | 0.059 | 0.069 | 0.070 |
| 4 | 341.2 | 341.7 | 1452.7 | 1454.3 | 0.103 | 0.101 | 0.110 | 0.110 |
| 8 | 179.0 | 178.7 | 544.6 | 543.6 | 0.266 | 0.265 | 0.193 | 0.195 |
| 11 | 69.0 | 69.2 | 128.9 | 128.3 | 0.427 | 0.428 | 0.300 | 0.307 |
| 14 | 10.50 | 10.61 | 9.64 | 9.75 | 0.661 | 0.653 | 0.411 | 0.417 |

The differences are within 2 standard errors, except ε2 at b = 14 fm (0.008, about 3 standard errors). With TGlauber's 0.4 fm hard core the TGlauber values move by less than 1% (e.g. ε2 = 0.057 at b = 0, 0.264 at b = 8 fm).

p–Pb shows the proton placement (bug 7): CAP puts the proton up to 2 fm off its centre, TGlauber at the centre. The results agree in the interior, and CAP's are larger and broader at the edge.

| b (fm) | ⟨Npart⟩ CAP | ⟨Npart⟩ TGlauber | sd(Npart) CAP | sd(Npart) TGlauber |
| --- | --- | --- | --- | --- |
| 0 | 15.21 | 15.32 | 3.63 | 3.67 |
| 4 | 11.90 | 11.94 | 3.16 | 3.12 |
| 6 | 6.40 | 6.45 | 2.41 | 2.18 |
| 7 | 3.78 | 3.69 | 1.59 | 1.41 |
| 8 | 2.52 | 2.42 | 0.81 | 0.70 |

The NN profile, integrated over the plane (effective σ\_NN for 67.6 mb): 67.60 mb for ω = 0, 0.2, 0.4, 0.7 and 0.999; 36.76 mb for ω = 1 (bug 2). The Pb density CAP samples gives ⟨r⟩ = 5.2898 fm and ⟨r²⟩ = 30.441 fm², against 5.2872 fm and 30.414 fm² exactly.

## Bugs that change results

Ranked by how much they change results and how likely you are to hit them. Paths are under src/models/Glauber.

| # | Where | What is wrong | Effect | Fix |
| --- | --- | --- | --- | --- |
| 1 | GlauberGenerator.cpp:269 | Nucleus B is looked up with `nameA` | p–Pb, d–Au, Cu–Au and every other asymmetric system silently run as A+A | `findNucleus(nameB, ...)` |
| 2 | NNProfile.cpp:76 and GlauberGenerator.cpp:48 | For ω = 1 the profile is a normalized Gaussian density, p(0) = 0.27, instead of TGlauber's G exp(−G b²/R²); ω = 1 is the code's default | Effective σ\_NN 36.8 mb instead of 67.6 mb whenever ω is left at its default (glauber.ini sets 0) | Remove the ω = 1 branch: the gamma formula already holds at ω = 1 |
| 3 | NuclearDistribution.cpp:156 (types 8, 14) | Deformed density: β2, β4, radius and skin are ignored, evaluate() returns x²/2; type 14 returns a negative value | Al, Si2, Cu2, Xe2, Xe2a, Au2, U2: nucleons spread uniformly in volume out to 18–22 fm; Cu2rw, Au2rw, Xe2arw throw | Sample (r, θ) from the deformed Woods–Saxon, then rotate the nucleus randomly, as TGlauber does |
| 4 | GlauberNucleus.cpp:329–353 (types 5, 7) | Box method with acceptance fixed at 1 | U (type 5): nucleons uniform in a 44 fm cube, up to 35 fm from the centre | Accept with 1/(1+exp((r−R(θ))/a)), R(θ) = R + w cos²θ (type 5) or R(1 + β2 Y20 + β4 Y40) (type 7), then rotate |
| 5 | GlauberNucleus.cpp:288 and NuclearDistribution.cpp:128 | Deuteron: both sampling calls are commented out; the Hulthén density is a stub (t0 = 0) | d, dh: both nucleons at the centre in every event | Hulthén density; r/2 for each nucleon; the neutron opposite for type 4 |
| 6 | GlauberNucleus.cpp:390 | Hard-core option (ALLOW\_OVERLAP = false): the loop never stops after a good position, and it compares the nucleon with itself and with nucleons not placed yet | Always throws "Too Many Iterations In Overlap Test", so TGlauber's default 0.4 fm hard core cannot be used | Compare with the nucleons placed so far; stop at the first good position |
| 7 | GlauberNucleus.cpp:286 | Proton placed at r uniform in \[0, 2\] fm (the argument is a radius squared), then half-recentered | p–A geometry smeared at the edge: ⟨Npart⟩ 2.52 vs 2.42 at b = 8 fm | Put the proton at the centre, as TGlauber does after recentering |
| 8 | GlauberNucleus.cpp:441 and 428 | Centre of the nucleus divided by N+1 (the count starts at 1); recentering mode 2 moves the first nucleon by the mean instead of N × mean | Leaves 1/(N+1) of the offset (0.5% for Pb, 6% for O) | Count from 0; move by N × mean |
| 9 | GlauberEventMoments.cpp:28 | `_doN` is false and nothing sets it; the εn histograms are commented out (GlauberHistos.cpp, after line 865) | Only ε2 from the variances is filled; ε1, ε3, ε4 and ψn are never computed | Set `_doN`, restore the fills (CAP's formulas match TGlauber's; the driver checked them) |
| 10 | GlauberHistos.cpp:844 | The mean x and y are filled after the second pass, which is about the centre | The mean-position histograms are always about 0 | Keep the first pass's means |

## Missing from CAP

These TGlauber features are absent or stubbed in CAP. Selecting one either throws or, where noted, gives wrong results.

| Feature | Nuclei or option | CAP now |
| --- | --- | --- |
| Nucleon configurations read from files (type 6) | He3, H3, He4, C, O | Throws (empty density). The five \*\_plaintext.dat files are in \~/Developer/TGlauberMC-3.2 |
| Separate proton and neutron densities (types 11, 13) | Pbpn, Pbpnrw | Throws |
| Reweighted densities (types 12, 14) | Curw, Xerw, Xesrw, Aurw, Pbrw (12); Cu2rw, Au2rw, Xe2arw (14) | Throws |
| Harmonic oscillator (type 15), double Gaussian proton (type 10) | Oho, pdg | Throws |
| Gaussian proton (type 9) | pg | Uses the skin (0) instead of the radius; harmless only because a single proton ignores its density (bug 7) |
| Cross-section fluctuations (TGlauber's fPTot, with and without AAGG) | NNCOLLISION\_OPTION 2 and 3 | Throws. Option 3 as written would also apply the NN profile on top of the black disk |
| Random orientation of each nucleus | Deformed nuclei | Missing (needed with bugs 3 and 4) |
| Lattice placement and nucleon smearing | NODE\_DISTANCE, lattice types | Throws "Not implemented" |

Not audited: overlap and total area, path length, the core–corona split, two-component weights, and the total cross section from the fraction of events with a collision.

## Minor issues and different defaults

- **Hard collisions.** TGlauber counts a collision as hard when b < √(0.65 σ/(10π)), which is 1.18 fm at 67.6 mb. CAP uses a fixed HARD\_DISTANCE\_MAX, default 0.4 fm, so Nhard is much smaller.
- **Defaults.** CAP: ALLOW\_OVERLAP true, MIN\_INTER 0.2 fm, RECENTERING false. TGlauber: 0.4 fm hard core, recentering on. For Pb–Pb the effect is under 1%.
- **σ\_NN(√s), option 4 (COMPETE).** It gives 27.5 mb at 5.02 TeV and 28.1 mb at 13 TeV, so one of its parameters is wrong. Options 1, 2, 3 and 5 give 67–68 mb at 5.02 TeV and 76–78 mb at 13 TeV.
- **INTERACTION:DISTANCE\_MAX** is read but not used; execute() cuts at a fixed 4 fm. This is harmless, because every profile is negligible beyond 3 fm.
- **Density sampling.** Each bin is valued at its low edge, which shifts ⟨r⟩ by +0.0026 fm (0.05%). Valuing the bin at its centre removes it.
- **Participant-plane angle.** `_psi` = atan2(εy, εx) is 2Ψ2 in \[−π, π\]; TGlauber's Psi2 = (atan2 + π)/2. The eccentricities are the same, but compare the angles with care.
- **Smearing.** With the radial smearing on, the two passes over the participants draw different offsets, so the second pass is not about the first pass's centre.
- **He3 and H3.** CAP gives He3 Z = 2 and H3 Z = 1. TGlauber had them swapped; CAP is right.

## Method

- **TGlauber side.** ROOT is not installed here, so I ported the parts of runglauber\_v3.2.C that the comparison uses to numpy, step by step: ThrowNucleons (3pF density, continuous mode, fMinDist, fRecenter = 1), CalcEvent (hard sphere, d² = σ/(10π)) and CalcResults (εn with r^n weights, r³ for n = 1).
- **CAP side.** A small C++ program, linked to CAP's libGlauber, follows the steps of GlauberGenerator::execute(). It calls GlauberNucleus::positionNucleons(), shiftNucleus(), NNProfile::evaluate() and GlauberEventMoments in two passes, as GlauberAnalyzer does. Bug 1 is in the code that reads the configuration, which the program bypasses; it was found by reading.
- **Separate checks.** CAP's density against the exact 3pF moments; the NN profile integrated over the plane for several ω; and placement of d, U, Au2, U2 and Xe2, and of Pb with the hard core on.
- **Sources.** TGlauberMC 3.2 in \~/Developer/TGlauberMC-3.2 (Loizides, Kamin, d'Enterria, arXiv:1710.07098); CAP's src/models/Glauber on main as of today.
