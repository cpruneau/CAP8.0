# Removing ROOT from CAP (branch `remove-root`, November 2026)

ROOT is now **optional and off by default** (`-DCAP_WITH_ROOT=ON` to turn it on; "ROOT" in BuildCAP.py).
With it off, CMake does not look for ROOT, no ROOT header is on the include path, and no target links a ROOT
library. The whole tree (core, analysis, Glauber, Therminator3, Latex, Graphics, CapShell, PHSD) configures,
compiles and links that way; the C++ standard is now set to 17 by CAP itself (it used to come from ROOT's flags).

## What was replaced (no ROOT needed any more)

| Was | Now |
|---|---|
| `gRandom` (`Rndm`, `Gaus`, `Uniform`, `Poisson`, `SetSeed`), `TRandom *` | `CAP::MATH::randomGenerator()`: one shared `MersenneTwister`, seeded 4357 like TRandom3 (`Math/Random/MersenneTwister.hpp`). `log(Rndm())` uses `positive()` so it never sees 0. `ParticleDecayer` holds a `CAP::MATH::RandomGenerator *`. The `JOB:SEED` of CAP.cxx seeds it. |
| `SelectionGenerator` in Base (used gRandom) | moved to `Math/Random` (Base cannot depend on Math) |
| `TMath::BesselK1`, `TMath::BesselK(2,x)` (ThermalHadron) | `CAP::MATH::Bessel` K1, K2 (agree with `std::cyl_bessel_k` to 2e-7 for x in [0.05, 40]) |
| `TMath::Gamma(a,x)` (NNProfile) | `CAP::MATH::Distributions::regularizedGammaP(a,x)` |
| `TMath::Max` | `std::max` |
| `listDirsIn()`, `listFilesInDir()` (in RootFileHelpers, already built on FileSystem) | moved to `Base/FileListing.hpp/.cpp`; RootFileHelpers.hpp includes it, so its users are unchanged |
| `templateClassImp(CAP::ObjectCreator)` | removed (a ROOT dictionary macro) |
| Includes of ROOT headers that were not used (`TObject.h` in Timer, EventProcessor, Therminator3, Glauber; `TFile.h` in Task; `TSystem*.h` in EnvironmentVariables, StreamHelpers; `TStyle.h`, `TROOT.h`, `TH1D.h` in CAP.cxx; ...) | removed. `ThermalHadron` no longer derives from `TObject`. |

## What was taken out of the build without ROOT (built only with `CAP_WITH_ROOT`)

These need ROOT by nature. **To decide: which ones to put back, port to CAP, or drop.**

1. **Helpers library** (`core/Helpers`): `RootFileHelpers`, `RootHistogramHelpers` (~1000 uses of TH1/TH2/TH3/TProfile),
   `RootConversion`, `RootFileConversion`, `RootChainManager`, `RootFileManager`, `CanvasList`, `CanvasConfiguration`,
   `GraphConfiguration`, and `RootHistogramGroup` (moved here from Particles; nothing uses it). No core module links
   Helpers any more.
2. **`.root` files in CapGui and `bin/CapRootConvert`** (import/export of ROOT histograms): the `#ifdef` that existed
   is now `CAP_WITH_ROOT`.
3. **Epos module**: `EposEventReader` reads EPOS events from ROOT trees (`RootChainManager`). `CAP_WITH_EPOS` is now off
   by default and is skipped with a warning unless `CAP_WITH_ROOT` is on.
4. **`PythiaEventReader`** (Pythia events stored in ROOT trees): built and registered only with `CAP_WITH_ROOT`; the
   Pythia event generator itself does not need ROOT.

## ROOT drawing that was removed (nothing replaces it yet)

5. **`GlauberHistos::drawEvent()`** (event display: nuclei, nucleons, interaction points as `TEllipse`s in
   `<event>.pdf`, option `drawEvents`): now prints a warning once and draws nothing. Candidate for CAP Graphics
   (Canvas + PdfPainter; circles as Polygons).
6. **GlauberGenerator** debug plots `NNProfile.pdf`, `NNProfileS.pdf` (TCanvas) in the nucleon-profile option 1: removed.
7. **`ThermalHistos::drawMassSpectrum()`** (decay-chain diagram, `MassDecay.pdf`; TLine/TArrow): removed; it was only
   called from a commented-out line.

## Files that still contain ROOT code but are not built (to delete or keep?)

- `core/Base/TString.h`, `TString.cxx`: a copy of ROOT's own TString source.
- `core/Base/tempo/` (StreamManager, ObjectList, List, Manager: `ClassDef`, TObject).
- `core/Base/Result.hpp` (`vector<TH1*>`; does not compile).
- `core/Math/Functions/Pdf.cpp` (`ROOT::Math` beta and Landau functions; no header, not built).
- `models/Glauber/Glauber.cpp`, `Glauber.hpp` (the original TGlauber), `GlauberPlotter.*`, `NuclearDistribution.C`,
  `Plots.C`, `test.C`, `oldRootLogon.C`.
- `core/Graphics/_orig_2026-10-04/` (old Graphics with a ROOT dictionary).
- `src/Macros/` (30 ROOT macros, run in ROOT's interpreter).

## Build and tests without ROOT (Linux, gcc 11, `-DCAP_WITH_GUI=OFF`)

All targets build and link. ctest: 44 of 50 pass. The others are not caused by ROOT's removal:
`TaskRegistry` and `MathReflection` pass when linked with `-Wl,--no-as-needed` (Ubuntu's linker drops the libraries
whose task registrations are never referenced; this does not happen on macOS); `ExtremumSearch` (simulated annealing
with `std::` distributions, which differ between libstdc++ and libc++); `RunCAPFilters` (runs the installed CAP);
`RootFreeSources` and `Therminator3Models` timed out at -O0 in a small VM.
Not checked here: the macOS-only parts (MacOsCanvas.mm, CapGui) and Pythia/Herwig/Jets (their packages are not in the
test machine). None of them uses ROOT except PythiaEventReader.
