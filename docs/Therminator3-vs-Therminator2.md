# Therminator3 vs Therminator2: audit and model-by-model comparison

Therminator2 (T2) source: `/Users/aa7526/Developer/therminator2` (version 2.0.3, files in `build/src`, `build/include`).
Therminator3 (T3) source: `src/models/Therminator3/` (branch `std-functions`).
T2 paths below are relative to `therminator2/build/src/`. T3 paths are relative to `src/models/Therminator3/`.

## Status (24 October 2026)

Every bug of §4 (1–15) is now fixed on branch `std-functions`, one commit each (hashes in the table of §4).

**Second round, bugs 4–15:**
- Hydro2D and Hydro3D load their hypersurface from `HYPERS:IMPORT:PATH` and `HYPERS:IMPORT:FILE_NAME` (6ed1a8c). In the same commit, Hydro3D reads Λ, uses the field names `FluidUx`/`FluidUy`, and computes missing derivatives (bugs 4, 10).
- HadronGas is a static fireball (2d58091). Primordial photons are skipped (cee7e40). `gammaAtV` is removed (79ade95).
- μ_S has the physical sign, with K⁺ at S = +1 (7e70e66).
- Lengths (`rhoMax`, `TauI`, `TauC`, `Delay`) are configured in fm, and particle positions are in fm (b8c41bb). Temperatures and chemical potentials stay in GeV.
- The multiplicity fraction defaults to 1, with `MultFractionMin`, `MultFractionMax` and `FluctuationType` read from the configuration (d75c37b).
- The impact-parameter keys are read into the right settings (b80a75f).
- The hadron-gas file reads back, and `ThermalHadronGas::configure` reads `temperature`, `muBaryon`, … (3a00934).
- `finalize` deletes the model last (cd84e0d).
- The task integrates the multiplicities in `initialize` (or imports them with `MULT:IMPORT`) and generates events in `execute` (9d29536).
- The task defines all model parameters in `setDefaultConfiguration`, so the RunCAP template lists them (43c6568).
- The test gained Hydro3D and Therminator2-hypersurface checks (8830dcb).

**First round:**
- Bug 1 and bug 2 (95310c5): the sampled maximum of the integrand is kept, and the multiplicities are integrated (`Model::calculateMultiplicities`, `setHypercubeVolume`).
- Bug 3 (07d8eef): ζ is no longer overwritten in Hydro2D and Hydro3D.
- Inherited corrections:
  - Safety factor on the rejection maximum, plus an overflow count (98ad229).
  - BWA formation displaced by the delay only (00b279b).
  - Linear vT profile kept: it is Eq. (42) of the paper, arXiv:1102.0273.
  - Negative dΣ·p clipping kept: it is the standard Cooper–Frye practice, and `BackFlow` already selects the alternative.
- K± mass set to 0.493677 GeV in `DB/ParticleData/ParticleDb.xml` (d171617).
- New ctest `Therminator3Models` (6b8fb2c, `src/models/Therminator3/tests/TestTherminator3Models.cpp`).

### Check results
Therminator 2.0.3 was built from a scratch copy and run with `blastwave.ini`: IntegrateSamples = 2×10⁶, 1000 events, Randomize = 0. The scratch copy needed three build fixes (§1, item 12) and a fix to the text output (item 2).

| Primordial | T2 N̄ (integration) | T3 N̄ | T2 ⟨N⟩/event | T3 ⟨N⟩/event | T2 ⟨pT⟩ (GeV) | T3 ⟨pT⟩ (GeV) | pull |
|---|---|---|---|---|---|---|---|
| π⁺ | 417.59 | 418.45 | 418.32 ± 0.65 | 418.32 ± 0.65 | 0.44993 ± 0.00050 | 0.45031 ± 0.00050 | +0.5 |
| K⁺ | 130.50 | 130.44 | 130.23 ± 0.36 | 130.66 ± 0.36 | 0.58944 ± 0.00102 | 0.58774 ± 0.00101 | −1.2 |
| p | 41.08 | 41.20 | 41.15 ± 0.20 | 41.10 ± 0.20 | 0.75472 ± 0.00217 | 0.75221 ± 0.00214 | −0.8 |
| Λ | 17.06 | 16.96 | 17.18 ± 0.13 | 16.97 ± 0.13 | 0.82132 ± 0.00354 | 0.82428 ± 0.00361 | +0.6 |

- **Accept/reject.** No draw exceeded 1.2 × the sampled maximum.
- **Integration noise.** The integrated multiplicities fluctuate by about 1% at 2×10⁶ draws. With another seed, the p ratio was 1.0096. The test therefore allows 2.5%.
- **Hydro2D on a hypersurface built to equal BlastWave** (τ = const, ρ ∈ [0, ρmax], constant vT). The multiplicities agree with BlastWave within 0.9% (π⁺ 0.991, K⁺ 0.995, p 1.007, Λ 0.999). With the old ζ overwrite they were 0.66, 0.79, 0.91, 0.93.
- **BWA formation.** All 801 generated particles stream back onto the hypersurface. With the old displacement, 46 did not.
- **μ_S sign.** Since bug 8 was fixed, the test uses Therminator 2's μ_S = +6.9 MeV as it is, and gives the same results as before.
- **Units.** The test sets the model members directly, in GeV and GeV⁻¹ (internal units). `configure()` takes fm.

### Check results, second round
**Hydro2D on Therminator 2's own RHIC Au+Au 20–30% hypersurface** (`fomodel/lhyquid2dbi/RHICAuAu200c2030Ti455ti025Tf145.xml`), read with `readFromXmlFile`. It is compared with a Therminator 2 Lhyquid2DBI run: 2×10⁶ samples, 500 events.

| Primordial | T2 N̄ | T3 N̄ | T2 ⟨N⟩/event | T3 ⟨N⟩/event | pull | T2 ⟨pT⟩ (GeV) | T3 ⟨pT⟩ (GeV) | pull |
|---|---|---|---|---|---|---|---|---|
| π⁺ | 195.56 | 194.09 | 195.88 ± 0.63 | 194.56 ± 0.62 | −1.5 | 0.49337 ± 0.00110 | 0.49169 ± 0.00109 | −1.1 |
| K⁺ | 48.12 | 48.21 | 47.86 ± 0.31 | 48.05 ± 0.31 | +0.4 | 0.67327 ± 0.00260 | 0.67370 ± 0.00259 | +0.1 |
| p | 10.758 | 10.816 | 10.93 ± 0.15 | 10.88 ± 0.15 | −0.2 | 0.88912 ± 0.00666 | 0.88784 ± 0.00656 | −0.1 |
| Λ | 3.829 | 3.807 | 3.92 ± 0.09 | 3.88 ± 0.09 | −0.4 | 0.96253 ± 0.01196 | 0.96390 ± 0.01189 | +0.1 |

- **Accept/reject.** No draw exceeded the scaled maximum.
- **When this check runs.** It runs only when `THERMINATOR2_DIR` is set, in the environment or with `cmake -DTHERMINATOR2_DIR=...`. Otherwise it is skipped, because the hypersurface file is not in the repository.
- **Hydro3D on a 3+1 hypersurface built to equal a boost-invariant BlastWave** (|η| ≥ 6). It agrees with BlastWave over 12 units of η:
  - 2×10⁶ samples: within 1.8% (π⁺ 1.004, K⁺ 1.003, p 1.013, Λ 1.018).
  - 1.6×10⁷ samples: within 0.2% (1.002, 1.001, 1.001, 1.001), so the difference at 2×10⁶ is integration noise.
  - No Therminator 2 Lhyquid3D file exists to compare with: the one `lhyquid3d.ini` names is missing (§1, item 8).
- **Hadron gas.** The integrated gas written by `saveToAscii` reads back with `loadFromAscii`: same PDG codes, maxima and multiplicities.
- **The task in a workflow.** `projects/workflow1.xml` was run with its BasicEventGen replaced by Therminator3 (BlastWave with `blastwave.ini` values: lengths in fm, T and μ in GeV, NSamples = 2×10⁵, 200 events), through `bin/CAP` with the libraries installed in `lib/`. The job completed. ParticleSingleAnalyzer counted 3767.5 primordial particles per event; Therminator 2 has 3776.0 per event (photons excluded), a −0.2% difference.
- **Full suite.** Build and install OK. `ctest`: 32/32 tests passed. `Therminator3Models` (22 checks, plus 8 more with `THERMINATOR2_DIR` set) took 126 s with the RHIC comparison and about 50 s without it.

### Not checked
- KrakowSFO and BWA subtypes 2–5 have not been compared with Therminator 2 runs. Their integrands are the same as in Therminator 2, formula for formula.
- HadronGas (new integrand) has no reference to compare with.

### New findings (not fixed)
- `projects/test.ini` sets `RunThermalModel:HADRON_GAS:temperature`, while `ThermalModel` reads its gas from `ThermalHadronGas:…`.
- `ModelSettings::setSystemEnergy(double)` assigns a number to a `String`.
- The `Therminator3` copy constructor shares the `_model` pointer, so two copies would delete the model twice.

The rest of this document is the original audit. The fixes are marked in §4.

---

## 1. Therminator2 audit

### Structure
- Three executables built by a hand-written `Makefile`: `therm2_events` (event generator), `therm2_femto` (two-particle correlation functions), `therm2_hbtfit` (HBT radius fits). All three need ROOT (`root-config`, TTree/TFile/TRandom2/TMath). The build uses hard-coded `g++`, `-O0 -g`, and no C++ standard flag.
- Compile-time switches: `BACK_FLOW`, `RESCALE_CHANNELS`, `DISABLE_TWO_BODY_DECAYS`, `DISABLE_THREE_BODY_DECAYS`, `DEBUG`.
- Configuration: `events.ini` (main), `fomodel/*.ini` (one per model), hydro hypersurfaces in XML (`fomodel/lhyquid2dbi/*.xml`, 16 files), particle and decay tables in `share/particles.data` and `share/decays.data`.
- Internal units: GeV and GeV⁻¹. Lengths and times are read in fm and divided by ħc = 0.1973269631 GeV·fm. Temperatures and chemical potentials are read in MeV and multiplied by 0.001. Positions are multiplied back by ħc when written (`EventGenerator.cxx` SaveAsRoot, `Particle.cxx` MakeTEXTEntry).

### Freeze-out models (selected by `FreezeOutModel`, `therm2_events.cxx:141-154`)
| sModel | Name | Class |
|---|---|---|
| 0 | KrakowSFO | `Model_KrakowSFO` |
| 1 | BlastWave | `Model_BlastWave` |
| 2–6 | BWAVT, BWAVTDelay, BWAVLinear, BWAVLinearDelay, BWAVLinearFormation | `Model_BWA` |
| 10 | Lhyquid3D (3+1 hydro) | `Model_Lhyquid3D` + `Hypersurface_Lhyquid3D` |
| 11 | Lhyquid2DBI (2+1 boost-invariant hydro) | `Model_Lhyquid2DBI` + `Hypersurface_Lhyquid2D` |
| 99 | Example (template, not compiled in) | `Model_Example` |

### How an event is generated
1. **Integration** (`Integrator.cxx` Integrate): for each particle type, draw `IntegrateSamples` points uniformly in the hypercube (space–time × momentum). Each point gives the Cooper–Frye integrand `I`. The mean multiplicity is `N̄ = V_hypercube · ⟨I⟩`, and `max I` over the samples is kept. Both values are cached in `fmultiplicity_<CRC32 of the parameters>.txt`.
2. **Multiplicity** (`Event.cxx` GenerateMultiplicities): Poisson(N̄) per type. NegativeBinomial is a stub that returns 0.
3. **Primordial emission** (`Integrator.cxx` GenerateParticles): accept/reject. Draw a point, compute `I`, accept if `u·I_max < I`. Repeat until the multiplicity is reached. Primordial photons (`gam000zer`) are skipped.
4. **Decays** (`Event.cxx` DecayParticles → `ParticleDecayer.cxx`): the particle list is walked as it grows, so cascades are handled.
   - The channel is chosen from raw branching ratios. If the ratios sum to less than 1, the particle may stay undecayed.
   - The decay time is exponential with lab mean `E/(mΓ)`. The decay point is the parent position plus `(p/E)·t`.
   - Two-body decays are isotropic in the rest frame, then boosted to the lab.
   - Three-body decays use flat Dalitz sampling of (E₁, E₂) with a cos θ₁₂ ≥ |1| rejection, then a random Euler rotation and a boost.
   - The parent mass is always the pole mass: `BreitWigner()` exists but is never called.

### Common integrand (all models)
```
I = (2s+1)/(2π)³ · pT · dpT/dζ · (dΣ·p) · 1/(exp((p·u − μ)/T) ± 1)
```
- `pT = ζ/(1−ζ)` with ζ ~ U(0,1), so `dpT/dζ = 1/(1−ζ)²`.
- `φp ~ U(0,2π)`, `y ~ U(−Y/2, Y/2)`, `mT = √(m²+pT²)`.
- The momentum is `p = (mT cosh y, pT cos φp, pT sin φp, mT sinh y)`.
- The sign is +1 for fermions and −1 for bosons. The test is `spin − int(spin) < 0.01`.
- `μ = B μB + I₃ μI + S μS + C μC`, with `S = N(s̄) − N(s)` (`Parser.cxx:126`), so K⁺ has S = +1.
- The hypercube volume is `V = Δη · 2π · ρmax · Y · 1 · 2π` (BW, BWA, SFO), or `ΔX·ΔY·ΔZ · Y · 2π` (hydro).

### Broken or questionable in Therminator2
1. **Undefined behaviour.** Several `int` functions have no `return`: `ParticleDecayer::DecayParticle`, `TwoBodyDecay`, `ThreeBodyDecay`, and `Particle::operator=` (which returns `ParticleCoor&`). Modern clang on macOS can turn this into a trap or into wrong code.
2. **Dangling pointer.** `Particle::MakeTEXTEntry` returns `oss.str().data()` of a temporary (`Particle.cxx`), so the text output reads freed memory.
3. **Empty event.** `Event::DecayParticles` uses a do-while on `mParticles.begin()`, so it dereferences `end()` if the event is empty.
4. **Rejection maximum.** The accept/reject step uses a `I_max` estimated from a finite sample, with no safety factor. Points where `I > I_max` are clipped, which biases sharp peaks (hydro surfaces with large dΣ·p, high-mass states with few samples).
5. **Negative Cooper–Frye contributions are clipped.** For the hydro models, `dΣ·p < 0` is set to 0, or with `BACK_FLOW` only the negative part is kept. This is the standard approximation, but it is not a conserving one.
6. **BWA formation (model 6) displacement.** `Model_BWA.cxx`: after `Xt += delay`, it does `Xx += Xt·Px/E`. This uses the full time (hypersurface time plus delay), not just the delay, so particles are displaced as if they streamed freely from t = 0. Probably intended: `Δt·p/E` with `Δt` the delay only.
7. **BWA linear velocity profile.** The formula is `vT(ρ) = (ρ/ρmax)/(VelT + ρ/ρmax)`. The parameter VelT sits in the denominator: VelT = 0.407 gives vT(ρmax) = 0.71. This is unusual. Check it against the T2 paper before porting it as "the" profile.
8. **Lhyquid3D is broken out of the box.** `fomodel/lhyquid3d.ini` points to `lhyquid3d/*.xml`, and that directory does not exist.
9. **Channel choice.** `DecayTable::ChooseDecayChannel` (RESCALE mode) reads `mBranchingRatios[i]` before it checks the bound, so a float sum below 1 can read out of range. In the default mode, ratios that sum to less than 1 leave particles undecayed without any warning.
10. **Random streams.** There are four TRandom2 streams with fixed seeds (Integrator 41321, Event 31851, Decayer 60356, and per-event seeds 43212−2i when `Randomize = 0`). `Randomize` seeds from `TDatime`, so it has one-second resolution: parallel jobs started in the same second produce identical events.
11. The negative-binomial multiplicity option is not implemented.
12. **It does not build with a current toolchain** (found while building it for the checks above):
    - ROOT 6.40 compiles as C++17, which rejects the dynamic exception specifications in `Configurator.h` and `Hypersurface_Library.h`.
    - `therm2_events.cxx` uses `ifstream` and `ofstream` without `std::`.
    - The `-lgcc` link flag does not exist on macOS.
    - Item 2 is real: the text output contains the event headers but no particle lines.
13. **`CopyINIFile` (`therm2_events.cxx`) can empty the main settings file.** It builds the copy's name by replacing `"./"` with the event directory. When the settings file is given without `./` (`therm2_events events.ini`), source and destination are the same file, and the file is truncated to zero bytes.

---

## 2–3. Model-by-model formulas and comparison with Therminator3

### Shared machinery
| Item | T2 | T3 | Verdict |
|---|---|---|---|
| pT map, dpT | `Zet/(1−Zet)`, `1/(1−Zet)²` | same. T3 clips ζ > 0.9999999 to 1−ζ = 1e-8 (or 1e-7 in `Model.cpp:108`, `ModelBWA.cpp:32`) | Equivalent: the integrand is about 0 there |
| y, φp, η sampling | U(−R/2, R/2), U(0, 2π) | `R·(u−0.5)`, `twoPi()·u` | Identical |
| Prefactor | `(2s+1)/kTwoPi3` | `spinFactor()/twoPiCube()` (`ParticleType.hpp:436`) | Identical |
| ± statistics | Bose −1 / Fermi +1 | `statistics()` = isBoson ? −1 : +1 (`ParticleType.hpp:468`) | Identical |
| μ_S sign | `S = N(s̄) − N(s)`, K⁺ = +1 (`Parser.cxx:126`) | `strangeness() = N(s) − N(s̄)`, K⁺ = −1 (`ParticleType.hpp:331`, `ParticleType.hpp:183`) | **Different.** T2 uses the physical convention |
| Units | fm → GeV⁻¹ (÷ħc), MeV → GeV (×10⁻³) at read time. Positions × ħc on output | No conversion in `Model::configure` (`Model.cpp:32-44`). Positions never converted back | **Different** (see bug 6) |
| N̄, I_max | `Integrator::Integrate`: N̄ = V·⟨I⟩, I_max = max I | No integration. `Model::calculateMultiplicities` (`Model.cpp:85-93`) only sets `_volume = _sVolume·_pVolume`, and both are always 0. The loop is commented out (`Model.cpp:125-136`) | **Missing** |
| Accept/reject maximum | `I_max` from the integration, stored per type | `maxIntegrand = hadron.integrand()` (`Model.cpp:180`), but every `calculateIntegrand` call does `hadron.setIntegrand(integrand)`, overwriting it with the last sampled value | **Bug** |
| Multiplicity | Poisson(N̄) | `mean = multFraction · avgMult`, with `multFraction ~ U(0,1)` by default (`ModelSettings.cpp:24-26`, `Model.cpp:143-147`) | **Bug** |
| Primordial photons | skipped | not skipped (loops over all of `_thermalHadronGas`) | Minor |
| Event loop | `EventGenerator::GenerateEvents` | `Therminator3::execute` (`Therminator3.cpp:219-232`) has `generateEvent` commented out | **T3 generates nothing** |

### KrakowSFO (single freeze-out) — T2 `Model_KrakowSFO.cxx:69-109`, T3 `ModelKrakowSFO.cpp:14-46`
- Hypersurface: `τ_K` fixed. The true proper time is `τ = √(τ_K² + ρ²)`, and the flow is Hubble-like, `u = x/τ_K`.
- `p·u = [mT τ cosh(η−y) − pT ρ cos(φ−φp)]/τ_K`.
- `dΣ·p = τ_K ρ (p·u)`. Derivation: induced 3-volume element = (τ_K/τ)·ρ·τ = τ_K ρ.
- Position: `(τ cosh η, ρ cos φ, ρ sin φ, τ sinh η)`.
- **T3 matches T2 formula for formula.** The only difference is units: `TauC` and `rhoMax` must be given in GeV⁻¹, because T3 does not divide by ħc.

### BlastWave — T2 `Model_BlastWave.cxx:16-55`, T3 `ModelBlastWave.cpp:13-46`
- τ is fixed, ρ ∈ [0, ρmax], and vT is constant (no ρ dependence) and radial.
- `p·u = γ(mT cosh(η−y) − vT pT cos(φ−φp))`, with `γ = 1/√(1−vT²)`.
- `dΣ·p = τ ρ mT cosh(η−y)`.
- **T3 matches.** T3 reads τ from `TauI` (T2's key is `Tau`) and γ is precomputed in `Model.cpp:46`. Same unit caveat as KrakowSFO.

### BWA (models 2–6) — T2 `Model_BWA.cxx:21-75`, T3 `ModelBWA.cpp:16-90`
- Tilted surface `τ(ρ) = τ₀ + Aρ`.
- `dΣ·p = τ ρ (mT cosh(η−y) − A pT cos(φ−φp))`. This is correct: `dτ/dρ = A`.
- `p·u = γ(vT)(mT cosh(η−y) − vT pT cos(φ−φp))`.
- vT is constant for models 2 and 3, and `(ρ/ρmax)/(VelT + ρ/ρmax)` for models 4–6.
- Delay (models 3, 5, 6): `t += −Delay·ln u`.
- Formation (model 6): `x_i += t·p_i/E`.
- **T3 matches T2 term for term, including the questionable formation displacement (T2 item 6).** T3 maps model numbers through `_modelSubtype`, and any value other than 2–6 falls into "constant vT". The same random-number order is kept: the delay draw happens inside the integrand, as in T2.

### Lhyquid2DBI ↔ ModelHydro2D — T2 `Hypersurface_Lhyquid2D.cxx:62-97` + `Model_Lhyquid2DBI.cxx`, T3 `ModelHydro2D.cpp:16-73`
- The surface is parameterised by (ζ, φ, η): `τ = τ_i + D sin ζ`, `ρ = D cos ζ`, with `D(ζ,φ)` interpolated from the XML grid.
- `dΣ·p = D(τ_i + D sin ζ)[D cos ζ (mT sin ζ cosh(y−η) + pT cos ζ cos(φ−φp)) + ∂_ζD cos ζ (−mT cos ζ cosh(y−η) + pT sin ζ cos(φ−φp)) + ∂_φD pT sin(φ−φp)]`.
- `p·u = γ(vT)(mT cosh(y−η) − vT pT cos(φ − φp + φF))`.
- Negative `dΣ·p` is clipped (or only the negative part is kept with back flow).
- **T3 bug: ζ is overwritten before dΣ·p is computed.** `ModelHydro2D.cpp:39` reuses `zeta` for the pT random number (`zeta = gRandom->Rndm()`). Lines 50–55 then compute dΣ·p with `sin(zeta)` and `cos(zeta)` of that random number in [0,1), not of the hypersurface angle. The position (line 37) and p·u (line 49) are still correct.
- **T3 wiring: the hypersurface is never loaded.** `readFromXmlFile` (`ModelHydro2D.cpp:75`) is not called anywhere, and no configuration key names the XML file.

### Lhyquid3D ↔ ModelHydro3D — T2 `Hypersurface_Lhyquid3D.cxx:63-105`, T3 `ModelHydro3D.cpp:14-127`
- Parameterisation (ζ, φ, θ): `τ = τ_i + D sin θ sin ζ`, `ρ = D sin θ cos ζ`, `η = D cos θ/Λ`.
- `p·u = √(1+ux²+uy²) mT cosh(Y_F − y) − pT(ux cos φp + uy sin φp)`. This is correct for `u = (u⊥⁰ cosh Y_F, ux, uy, u⊥⁰ sinh Y_F)`.
- **T3 bug: same ζ overwrite as Hydro2D.** `ModelHydro3D.cpp:38` overwrites `zeta`, and the dΣ·p expression (lines 49–56) uses the wrong ζ.
- **T3 bug: Λ is never set.** `_lambda` stays 0 (`Hypersurface.cpp:19`), and `readFromXmlFile` does not read `Lambda` (T2: `Hypersurface_Lhyquid3D.cxx:146`, ÷ħc). The result is `η = D cos θ/0` → ±∞ or NaN, and `dΣ·p` is ∞ or NaN.
- **T3 bug: wrong XML field names.** `"FluiduxFluidVelocity"` and `"FluiduyFluidVelocity"` (`ModelHydro3D.cpp:109-110`) look like a search-and-replace artifact. T2 files use `FluidUx` and `FluidUy`.
- T3 requires `DistanceDZeta`, `DistanceDPhi` and `DistanceDTheta` in the file. T2 computes them by finite differences when they are missing (`Hypersurface_Lhyquid3D.cxx:170-187`).
- `readFromXmlFile` is never called, same as Hydro2D.

### HadronGas (T3 only) — `ModelHadronGas.cpp:15-47`, also `Model.cpp:95-123`
- This copies T2's `Model_Example` placeholder: `p·u = 1` and `dΣ·p = 1`. The integrand is then `∝ pT/(1−ζ)²`, which has no Boltzmann factor in pT and diverges as ζ → 1.
- A static fireball needs `p·u = mT cosh(η−y)` and `dΣ·p = τ ρ mT cosh(η−y)`. That is BlastWave with vT = 0.

---

## 4. Suspected Therminator3 bugs, ranked by impact on the generated spectra

| # | Bug | Where | Effect | Proposed fix |
|---|---|---|---|---|
| 1 | **Fixed (95310c5).** The accept/reject maximum is overwritten by every integrand evaluation | `Model.cpp:121`, `ModelBlastWave.cpp:44`, `ModelBWA.cpp:88`, `ModelKrakowSFO.cpp:45`, `ModelHydro2D.cpp:71`, `ModelHydro3D.cpp:69`, `ModelHadronGas.cpp:45`; read at `Model.cpp:180` | The "maximum" becomes the last sampled value, usually tiny, so almost every proposal is accepted. Spectra then follow the proposal density: dN/dpT ∝ 1/(1+pT)², flat in y and φ, independent of T and flow | Remove `setIntegrand` from `calculateIntegrand`. Add an integration pass (the T2 `Integrate` loop) that stores `max I` and `N̄ = V⟨I⟩` once per hadron |
| 2 | **Fixed (95310c5).** No multiplicity integration, and the volumes are never set | `Model.cpp:85-93`, `_sVolume` and `_pVolume` are always 0 | N̄ = 0 (Poisson gives 0) unless loaded from a file. The relative yields of all species are wrong or missing | Port `Integrator::Integrate`. Set the hypercube volume per model: `Δη·2π·ρmax·Y·2π`, or for the hydro models `ΔX·ΔY·ΔZ_or_Δη·Y·2π` |
| 3 | **Fixed (07d8eef).** Hydro2D and Hydro3D: ζ is reused for the pT random number before dΣ·p is computed | `ModelHydro2D.cpp:39` → `:50-55`; `ModelHydro3D.cpp:38` → `:49-56` | dΣ·p is evaluated at a random angle, not at the surface point. This scrambles the pT spectra, v₂ and the yields. Negative values are clipped, so the bias is not even symmetric | Use a separate variable (`double z = gRandom->Rndm();`) for the pT map |
| 4 | **Fixed (6ed1a8c).** Hydro3D: Λ is never read | `ModelHydro3D.cpp:35,50`, `Hypersurface.cpp:19` | η and dΣ·p are ∞ or NaN, so the model is unusable | Read `Lambda` from the XML and divide by ħc |
| 5 | **Fixed (2d58091).** HadronGas has placeholder `p·u = dΣ·p = 1` | `ModelHadronGas.cpp:41-42`, `Model.cpp:117-118` | No thermal pT shape, and the integrand diverges as pT → ∞ | Use `p·u = mT cosh(η−y)` and `dΣ·p = τρ mT cosh(η−y)` |
| 6 | **Fixed (b8c41bb).** No unit conversion | `Model.cpp:32-44` | If T2 values are given (fm, MeV), then T = 165.6 means 165.6 GeV, and τ, ρ are treated as GeV⁻¹ instead of fm, so yields are off by ħc⁻³ ≈ 130. Output positions are in GeV⁻¹, while `ParticleDecayer` adds displacements in fm (`ParticleDecayer.cpp:56`), so units are mixed | Either document GeV/GeV⁻¹ inputs, or convert at configure time as T2 does. Convert positions to fm before decays and output |
| 7 | **Fixed (d75c37b).** Multiplicity fraction defaults to U(0,1) | `ModelSettings.cpp:24-26`, used at `Model.cpp:143-147`; `setMultiplicityFraction` is never called | Mean yields are halved, with large extra event-by-event fluctuations, which affects fluctuation and correlation observables | Default to min = max = 1 (range 0), or read the values from the configuration |
| 8 | **Fixed (7e70e66).** Opposite sign convention for μ_S | `Chemistry.cpp:73`, `ThermalHadron.cpp:178`, using `ParticleType::strangeness()` = N(s) − N(s̄) | With T2's μ_S = 6.9 MeV, the K⁺/K⁻ and Λ/Λ̄ ratios move the wrong way (a small effect at these μ_S values) | Use `−strangeness()` in the chemical potential, or document that the configured μ_S is for net s-quark number |
| 9 | **Fixed (9d29536).** Event generation is not wired | `Therminator3.cpp:219-232` | No events are produced | Call `_model->generateEvent(event(), filter)` once items 1 and 2 are done |
| 10 | **Fixed (6ed1a8c).** Hydro models: the XML is never loaded, and Hydro3D uses wrong field names and requires the derivatives | `ModelHydro2D.cpp:75`, `ModelHydro3D.cpp:72-127` | Empty fields. Interpolation throws or returns garbage | Add a `HypersurfaceFile` key and call `readFromXmlFile` in `configure`. Use the names `FluidUx`/`FluidUy`. Fall back to `derivativeX/Y/Z` when the derivatives are missing |
| 11 | **Fixed (3a00934).** `ThermalHadronGas` reads and writes different file formats, and `configure` reads the wrong keys | `ThermalHadronGas.cpp:298` reads `pdg integrand avgMult`; `saveToAscii` (`ThermalHadronGas.cpp:320-334`) writes 9 lines per hadron. `configure` (`ThermalHadronGas.cpp:226-232`) reads keys `"_temperature"`, `"_muBaryon"`, …, with a leading underscore, under `HYDRO_MODEL`, while the defaults are defined as `temperature` under `HADRON_GAS` | Saved multiplicities cannot be reloaded. The missing keys are read as 0 (`Properties::valueDouble` defaults to 0), so the gas temperature is 0 | Use one format, and fix the keys |
| 12 | **Fixed (cd84e0d).** `Therminator3::finalize` uses `_model` after deleting it | `Therminator3.cpp:237` vs `:242,251` | Crash or garbage file name | Delete `_model` last |
| 13 | **Fixed (b80a75f).** Impact-parameter keys are swapped | `Model.cpp:54-57` | Wrong metadata only | Read Min into min, Max into max, and the fixed value into `setImpactParameter` |
| 14 | **Fixed (cee7e40).** Primordial photons are not excluded | `Model.cpp:150` | Extra photons, if photons are in the gas | Skip photons (`type.isPhoton()`) |
| 15 | **Fixed (79ade95).** `gammaAtV` divides by c² instead of c | `Model.hpp:66-74` | None: unused | Remove it, or divide by c |

### Inherited from T2 (worth deciding, not porting blindly)
- **BWA formation displacement** used the total time, not the delay (`ModelBWA.cpp:76-79` = T2 `Model_BWA.cxx`). **Fixed (00b279b):** the shift is now delay × p/E.
- **The linear vT profile** puts VelT in the denominator (`ModelBWA.cpp:50`). **Kept:** it is Eq. (42) of the paper (arXiv:1102.0273); a comment in the code now says so.
- **The rejection maximum is estimated from a finite sample.** **Fixed (98ad229):** a safety factor `MaxIntegrandSafety` (default 1.2) is applied, and draws above the scaled maximum are counted (`Model::nOverflows()`).
- **Negative dΣ·p is clipped** in the hydro models. **Kept:** this is standard Cooper–Frye practice, and `BackFlow` already selects the alternative.

### Outside Therminator3 but affecting the spectra
- `DB/ParticleData/ParticleDb.xml:385-386`: the K± mass was 0.497672 GeV, which is the K⁰ mass. **Fixed (d171617):** it is now 0.493677 GeV, the PDG and T2 value.
- `src/analysis/Particles/ParticleDecayer.cpp:47`: it forces the parent mass to `1.02·(m1+m2)` when it is below threshold.

### Suggested check once bugs 1–3 are fixed
Generate BlastWave with T2's `blastwave.ini` values, converted to GeV/GeV⁻¹. Compare the π⁺, K⁺ and p `dN/dy` against T2's `fmultiplicity_*.txt` (N̄ per type over Y = 4). Compare the primordial `mT` slopes against the analytic blast-wave result. T3 KrakowSFO, BlastWave and BWA already use T2's integrands, so any mismatch left points to the integration and sampling layer or to units.
