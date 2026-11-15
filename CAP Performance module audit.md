# CAP Performance module audit

Nov 5, 2026 · @Claude Pruneau

## RunCAP.py changes

RunCAP.py now offers only the tasks of the last build, and no longer offers the base class Task.

- **Tasks from the last build only:** `bin/CAP --xml-templates` now writes into a staging folder, and `DB/Templates` is then replaced with exactly that set. Templates of classes no longer built are deleted, and the output pane lists which ones were removed. If `bin/CAP` writes nothing, the previous templates are kept.
- **Task:** the base class is no longer in the list of available tasks. Its template is still loaded, because the stage wrappers (Workflow, DerivedHistos, BalanceFct) use it.
- **Tested:** the folder update was tested against a throwaway folder, and RunCAP's Python tests pass, except the one that runs the macOS `bin/CAP`, which can't execute in a Linux shell.

## Performance module: verdict

None of the module's intended chain works end to end today. The chain is: simulate a measurement (smear generator particles), histogram reco − gen, then run closure tests.

The module (`src/analysis/Performance`) has about 2,100 lines in 13 classes. Six tasks are registered: PerformanceAnalyzer, MeasurementSimulator, ClosureCalculator, ClosureIterator, EventPlaneRandomizerTask and EventVertexRandomizerTask. The module has no tests.

## Blocking: the chain can't run

1. **`ParticleSmearer::smear()` (in Particles) never links a reco particle to its generator particle.** `setTruth()` is never called anywhere in CAP, so PerformanceAnalyzer throws "no truth particle" on the first particle.
   1. It also leaves out the particle type, so reco-level filters on charge or species can't work.
   2. It passes the smeared pseudorapidity as the rapidity to `setMPhiPtY`, which is wrong for massive particles.
2. **MeasurementSimulator smears a particle once for every filter combination it passes.** With more than one event filter, or overlapping particle filters, the reco event contains duplicate particles. It also:
   1. copies no event-level information (vertex, centrality, event number) to the reco event;
   2. applies no efficiency losses;
   3. doesn't check that both events exist.
3. **ClosureIterator does nothing.** It builds a ClosureCalculator per file but never configures or runs it (a `// BUG` comment marks the spot). It also:
   1. repeats its whole loop once per subtask;
   2. strips a `.root` extension from `.cap` files;
   3. passes full paths where file names are expected.
4. **ClosureCalculator's default paths don't work.** It passes `DEFAULT` literally instead of resolving it with `FileSystem::configuredPath()`, so default settings look for `DEFAULT/<file>.cap`.

## Bugs

Four more bugs, numbered on from the blocking issues.

| # | Where | Problem | Fix |
| --- | --- | --- | --- |
| 5 | ParticlePerformanceHistos::loadFrom() | Imports `n1_dpPt`; the histogram is created as `n1_dpt` | Use `n1_dpt` |
| 6 | ParticlePerformanceHistos::fill() | Δφ not wrapped into \[−π, π\]: tracks near φ = ±π give entries near ±2π | Wrap Δφ before filling |
| 7 | RunClosureAnalysis | Passes the generator file as the reco file; writes keys ClosureCalculator never reads; nothing calls it | Delete the class |
| 8 | EventVertexRandomizerTask | z spread defaults to 5 µm (beam spread is about 5 cm); ns converted with the µm→fm factor 1e9 (ns→fm/c is about 3e14); leftover key `EventsUseStream0` | Correct the defaults and the time conversion; drop the key |

## Gaps in the physics

- **Bias only, no width.** The profiles give the average reco − gen (the bias), not the resolution. ParticlePerformanceDerivedHistos, which would compute resolutions, isn't compiled.
- **No efficiency measurement.** Nothing computes reco/gen efficiency; CalibrationProducer was meant to, but isn't compiled. Efficiency maps are only used as correction weights in ParticleSingleAnalyzer, so a closure test can't check efficiency corrections.
- **Dead keys in RunCAP.** The `phi_*` and `rapidity_*` binning keys are offered but never used.
- **No tests** for any class in the module.
- **ROOT's random generator.** Randomness comes from ROOT's `gRandom`, not CAP's own Mersenne Twister. That matters for reproducible seeds per sub-sample, and for dropping ROOT later.

## Housekeeping

Several files in the folder are not compiled; each needs a revive-or-remove decision.

| File | Status | Suggested action |
| --- | --- | --- |
| EfficiencyHistos | Obsolete duplicate of ParticlePerformanceHistos (same class name) | Remove |
| ParticlePerformanceDerivedHistos | Resolutions and biases from the performance histograms | Revive (resolution widths) |
| CalibrationProducer | Efficiency from reco/gen single-particle ratios | Revive (efficiency measurement) |
| EfficiencyFunction | Parameterized efficiency vs pT and η | Decide |
| ResolutionFunction | Parameterized bias and rms vs pT and η | Decide |
| `,,` folder | Two filter creators written against an old template API | Remove |

RunCAP currently offers PerformanceAnalyzer, ClosureCalculator and ClosureIterator, but none of them works yet.

## Suggested order of work

- [x] Fix the smearer and MeasurementSimulator (items 1–2), so the chain produces histograms
- [x] Add resolution widths and an efficiency measurement
- [x] Repair the closure tasks (items 3–4), so closure tests run end to end
- [x] Add a small test running gen → smear → performance → closure on a toy event sample
- [ ]

## Progress

As of Nov 6, 2026, items 1–7 and the four work items are done. The new Performance test passes all 34 checks (built against ROOT 6.40.04 on Linux, not yet on the Mac). Nothing is committed.

| Item | Status | What changed |
| --- | --- | --- |
| 1 Smearer | Done | Reco particle linked to its generator particle (`setTruth`); type, position, pid and mass kept; η smeared as a pseudorapidity |
| 2 MeasurementSimulator | Done | Each generator particle measured once; particle efficiency applies losses; event number, index and projectiles copied; clear error without `EVENT_1`. Efficiencies and smearers: none, one, or one per particle filter |
| 3 ClosureIterator | Done | Runs ClosureCalculator on each `_Gen` file; file patterns now appear in RunCAP |
| 4 ClosureCalculator | Done | Resolves `DEFAULT` paths; compares whole folder trees; new `GEN_FOLDER` / `REC_FOLDER` keys compare two analyzers inside one output file |
| 5 `loadFrom` name | Done | Imports `n1_dpt` |
| 6 Δφ wrap | Done | Δφ in \[−π, π) |
| 7 RunClosureAnalysis | Done | Deleted |
| 8 Vertex randomizer defaults | Open | — |
| Resolution widths | Done | New PerformanceDerivedCalculator: rms of reco − gen vs pT and η |
| Efficiency measurement | Done | Same calculator: reco/gen vs pT and η, binomial uncertainties |
| Test | Done | `src/analysis/Performance/tests/TestPerformance.cpp` (ctest `Performance`): smearing, biases, resolutions, efficiencies, Δφ, file round trip, closure |

**To check on the Mac:**

- [ ] Build CAP and run `ctest`
- [ ] In `RunAnaPerformance.xml`, give PerformanceAnalyzer `EVENT_1` = Pythia (the generator event); without it the efficiency histograms stay empty
- [ ] Run the workflow with its Derived Histos stage: the first end-to-end test of MeasurementSimulator

**Still open:** bug 8; housekeeping (ParticlePerformanceDerivedHistos is superseded by the new calculator and can be removed); random numbers from ROOT's `gRandom`.
