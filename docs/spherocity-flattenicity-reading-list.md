# Spherocity & Flattenicity: Reading List

*Compiled 13 Nov 2026 for the Spherocity project and the CAP paper.*

Priority markers: ★ = core reading (read first) · ◆ = useful context · ⚠ = details to check before citing

---

## 1. Foundations: event-shape definitions and theory

- ★ **A. Banfi, G. P. Salam, G. Zanderighi**, "Phenomenology of event shapes at hadron colliders," *JHEP* 06 (2010) 038, arXiv:1001.4082.
  Standard reference for the hadron-collider definitions of transverse event shapes, including transverse spherocity, with matched NLL+NLO predictions. Experiments cite it for the definition of S₀.
- ◆ **A. Banfi, G. P. Salam, G. Zanderighi**, "Resummed event shapes at hadron–hadron colliders," *JHEP* 08 (2004) 062, arXiv:hep-ph/0407287.
  Introduces the family of hadron-collider event shapes and jet rates.
- ◆ **G. Zanderighi**, "Motivations and ideas behind hadron–hadron event shapes," arXiv:hep-ph/0605332.
  Short conceptual overview.
- ◆ **M. Dasgupta, G. P. Salam**, "Event shapes in e⁺e⁻ annihilation and deep inelastic scattering," *J. Phys. G* 30 (2004) R143, arXiv:hep-ph/0312283.
  Background on e⁺e⁻ and DIS event shapes (sphericity, thrust) and IRC safety.

**Definition to carry into the CAP summary:**
S₀ = (π²/4) · min over n̂ of ( Σᵢ |p⃗_T,i × n̂| / Σᵢ p_T,i )².
S₀ → 0 means jet-like (back-to-back); S₀ → 1 means isotropic. ALICE's later "unweighted" variant S₀^{pT=1} sets every p_T to 1, so each track counts equally.

---

## 2. Transverse spherocity: phenomenology and MC studies

- ★ **E. Cuautle, R. Jimenez, I. Maldonado, A. Ortiz, G. Paić, E. Pérez**, "Disentangling the soft and hard components of the pp collisions using the sphero(i)city approach," arXiv:1404.2372 (2014).
  Proposes using sphericity and spherocity to split pp events into soft (many-MPI) and hard samples, using PYTHIA 8.
- ★ **A. Ortiz, G. Paić, E. Cuautle**, "Mid-rapidity charged hadron transverse spherocity in pp collisions simulated with Pythia," *Nucl. Phys. A* 941 (2015) 78, arXiv:1503.03129.
  The standard phenomenology reference for S₀ as an event classifier at mid-rapidity.
- ◆ **A. Ortiz, P. Christiansen, E. Cuautle, I. Maldonado, G. Paić**, "Color reconnection and flowlike patterns in pp collisions," *Phys. Rev. Lett.* 111 (2013) 042001.
  Motivation: color reconnection produces flow-like effects, which is why topology selection matters.
- ◆ **G. Bencédi, A. Ortiz, H. Bello**, *J. Phys. G* 44 (2017) 065001.
  Jets and color reconnection producing flow-like patterns, studied with event shapes.
- ★ **A. Ortiz, L. Valencia Palomo, V. M. Minjares Neriz**, "Study of transverse-spherocity biased pp collisions at the LHC energies using the PYTHIA 8 event generator," *Universe* 10 (2024) 30, arXiv:2310.20406.
  How spherocity and multiplicity selections jointly bias pp samples, and how well PYTHIA reproduces this. Important for interpreting the ALICE ⟨p_T⟩ vs S₀ data.
- ◆ **A. Ayala, E. Cuautle, I. Dominguez, A. Ortiz, G. Paić**, "Fine structure in the azimuthal transverse momentum correlations at √s_NN = 200 GeV using the event shape analysis," arXiv:0902.0074.
  An early application at RHIC energy: transverse sphericity and recoil in PYTHIA pp at 200 GeV. This is MC only, not a measurement.
- ◆ **J. E. Muñoz Méndez, A. Ortiz**, "Investigating the most active pp collisions (top 0.1%) using the tools developed by experiments at the LHC," arXiv:2506.06588 (2025).
  PYTHIA comparison of the classifiers N_ch, S₀, S_T, R_T, V0M and flattenicity. Concludes flattenicity has the smallest biases.
- ◆ **G. Bencédi, A. Ortiz**, "Disentangling the hard gluon bremsstrahlung effects from the relative transverse activity classifier in pp collisions," arXiv:2105.04838.
  A companion classifier (R_T); useful for the "estimator choices" discussion.

---

## 3. Flattenicity: definition and phenomenology

- ★ **A. Ortiz, G. Paić**, "A look into the 'hedgehog' events in pp collisions using a new event shape — flattenicity," arXiv:2204.13733 (2022).
  The original proposal. ρ is the relative standard deviation of ⟨p_T⟩ (or N_ch) across η–φ cells. It was conceived for ALICE 3 (|η| < 4).
- ★ **A. Ortiz, A. Khuntia, O. Vázquez-Rueda, S. Tripathy, G. Bencédi, S. Prasad, F. Fan**, "Unveiling the effects of multiple soft partonic interactions in pp collisions at √s = 13.6 TeV using a new event classifier," *Phys. Rev. D* 107 (2023) 076012, arXiv:2211.06093.
  Adapts flattenicity to the forward-multiplicity (V0) acceptance of existing ALICE detectors and tests its sensitivity to MPI and hardness in PYTHIA 8.
- ◆ **A. Ortiz, G. Bencédi, F. Fan**, "Flattenicity as 'centrality' estimator in p–Pb collisions simulated with PYTHIA 8.312 Angantyr," arXiv:2407.07724; published as *J. Phys. G* 51 (2024) 125003 ⚠ *(confirm the journal reference)*.
  Extends flattenicity to p–A as a less-biased "centrality" estimator.
- ◆ **A. Ortiz, D. Sahu**, "Finite V₂Δ puzzle in low-multiplicity pp collisions from ultralong-range azimuthal correlations in the string-shoving model," *Phys. Rev. D* (2026), arXiv:2512.09195.
  Argues that flattenicity handles non-flow better than N_ch.
- ◆ **L. Gyulai, G. Bíró, R. Vértesi, G. G. Barnaföldi**, "Effect of event classification on the Tsallis-thermometer," *Phys. Rev. D* 113 (2026) 016021, arXiv:2601.04106.
  Fits ALICE spectra classified by multiplicity, flattenicity and spherocity. Gives the ALICE flattenicity definition (1440 η–φ cells) alongside S₀^{pT=1}.
- ◆ Heavy-flavour angular correlations vs flattenicity (PYTHIA 8), arXiv:2306.05910.
  Flattenicity used to isolate final-state-radiation events.

**Definition to carry into the CAP summary:**
ρ = √( Σᵢ (N_ch,i − ⟨N_ch⟩)² / N_cell² ) / ⟨N_ch⟩, summed over N_cell segments.
In ALICE Run 2 the segments are the 64 V0 channels (2.8 < η < 5.1 and −3.7 < η < −1.7). Events are classified in percentiles of 1 − ρ: 1 − ρ → 1 means "flat", high-MPI and high-multiplicity; 1 − ρ → 0 means jetty or low activity.

---

## 4. Measurements at the LHC

### ALICE: spherocity
- ★ **ALICE**, "Charged-particle production as a function of multiplicity and transverse spherocity in pp collisions at √s = 5.02 and 13 TeV," *Eur. Phys. J. C* 79 (2019) 857, arXiv:1905.07208.
  First ALICE ⟨p_T⟩ vs multiplicity in spherocity classes. Jet-like (low-S₀) events have higher ⟨p_T⟩ than INEL>0 and isotropic events have lower.
- ★ **ALICE**, "Light-flavor particle production in high-multiplicity pp collisions at √s = 13 TeV as a function of transverse spherocity," arXiv:2310.10236, *JHEP* (2024) ⚠ *(confirm volume and page)*.
  π, K, p, φ, K*⁰, K⁰_S, Λ, Ξ in narrow S₀^{pT=1} classes with two multiplicity estimators. Strangeness is slightly enhanced in isotropic events and strongly suppressed in jetty ones.
- ◆ **ALICE**, "System size and energy dependence of the mean transverse momentum fluctuations at the LHC," *Eur. Phys. J. C* 85 (2025) 776, arXiv:2411.09334.
  pp ⟨p_T⟩ fluctuations in S₀ classes: jetty events show larger correlation strength.
- ◆ R. Rath (ALICE), proceedings: *EPJ Web Conf.* 291 (2024) 02008.
  Summary of the light-flavour vs spherocity results.

### ALICE: flattenicity
- ★ **ALICE**, "Particle production as a function of charged-particle flattenicity in pp collisions at √s = 13 TeV," *Phys. Rev. D* 111 (2025) 012010, arXiv:2407.20037.
  The first flattenicity measurement: π, K, p and h± spectra up to 20 GeV/c in minimum-bias and 0–1% V0M events. Low-flattenicity events show a pronounced intermediate-p_T peak relative to minimum bias, and the ratio approaches unity at high p_T. The behaviour differs from V0M selection.
- ◆ G. Bencédi (ALICE), "Multi-parton interactions in pp collisions using charged-particle flattenicity with ALICE," arXiv:2312.11140 (proceedings).
- ◆ **Run 3, preliminary (QM2025):** K⁰_S and Λ vs flattenicity in pp at 13.6 TeV (S. Prasad, R. Sahoo for ALICE). Run 3 flattenicity uses forward FIT acceptance; the abstract quotes 2.2 < η < 5.0 and −3.4 < η < −2.3. Strangeness vs spherocity, jets and flattenicity in Run 2+3 (N. Jacazio for ALICE). ⚠ *These are conference results; check for papers before citing.*

### Related ALICE event-shape measurements
- ★ **ALICE**, "Transverse sphericity of primary charged particles in minimum bias proton–proton collisions at √s = 0.9, 2.76 and 7 TeV," *Eur. Phys. J. C* 72 (2012) 2124, arXiv:1205.3963.
  The precursor measurement: S_T grows with multiplicity, opposite to the generators of the time.
- ◆ **ALICE**, "Study of charged-particle production at high p_T using event topology in pp, p–Pb and Pb–Pb collisions at √s_NN = 5.02 TeV," *Phys. Lett. B* 843 (2023) 137649.
  An R_T / topology-based companion.

### ATLAS
- ★ **ATLAS**, "Measurement of charged-particle event shape variables in inclusive √s = 7 TeV proton–proton interactions with the ATLAS detector," *Phys. Rev. D* 88 (2013) 032004.
  Transverse thrust, thrust minor and transverse sphericity in minimum bias (≥ 6 charged particles) vs leading p_T, N_ch and Σp_T. The MC models deviate significantly from data.
- ◆ **ATLAS**, "Measurement of event shapes at large momentum transfer with the ATLAS detector in pp collisions at √s = 7 TeV," *Eur. Phys. J. C* 72 (2012) 2211.
  Jet-based event shapes (y₂₃, thrust, sphericity, S_⊥, aplanarity).

### CMS
- ⚠ No dedicated CMS spherocity or flattenicity measurement turned up in this search. CMS has hadronic event-shape (transverse thrust) measurements in multijet events; add the exact reference if you want CMS represented.

---

## 5. Heavy-ion collisions and RHIC energies

**Status at RHIC:** this search found no published STAR, PHENIX or sPHENIX measurement of transverse spherocity or flattenicity. STAR's "event-shape engineering" in Au+Au at 200 GeV is based on the flow vector, which is a different technique. RHIC-energy work on S₀ is currently MC/phenomenology only. This is a gap worth stating explicitly in the CAP paper.

- ★ **N. Mallick, R. Sahoo, S. Tripathy, A. Ortiz**, "Study of transverse spherocity and azimuthal anisotropy in Pb–Pb collisions at √s_NN = 5.02 TeV using A Multi-Phase Transport model," *J. Phys. G* 48 (2021) 045104, arXiv:2008.13616.
  The first use of S₀ in heavy-ion collisions (AMPT). Low-S₀ events dominate v₂.
- ◆ **N. Mallick et al.**, "First implementation of transverse spherocity analysis for heavy-ion collisions at the LHC energies," arXiv:2001.06849.
  S₀ distributions in pp, p–Pb, Xe–Xe and Pb–Pb at fixed N_ch. Larger systems are more isotropic.
- ◆ **S. Prasad, N. Mallick, D. Behera, R. Sahoo, S. Tripathy**, "Event topology and global observables in heavy-ion collisions at the LHC," arXiv:2112.03892.
- ◆ Spherocity and reduced flow vector as joint event-shape classifiers in AMPT (eccentricity–v₂ correlations); see the SCOAP3 record "Probing initial geometrical anisotropy and final azimuthal anisotropy… through event-shape engineering."
- ★ **S. Choudhury, E. Nandy**, "Intrinsic coupling between transverse spherocity and elliptic flow in heavy-ion collisions," *Phys. Rev. D*, doi:10.1103/3kvq-sqgq.
  Important caveat: in A–A collisions S₀ is intrinsically anticorrelated with v₂. It should be read as a probe of collective anisotropy, not of jettiness. Read this alongside Mallick et al.
- ◆ Chiral magnetic effect search with spherocity classification in AMPT, arXiv:2604.05654 (2026).
- ◆ SQM2026 poster: spherocity in O–O, Ne–Ne, p–O and p–Ne at 200 GeV and 5.36 TeV with AMPT. This is the closest RHIC-energy item, and it is a conference poster, not a paper.

---

## 6. Methods and corrections (where to look)

| Topic | Where it's treated |
|---|---|
| S₀ definition, IRC safety, transverse-plane restriction | Banfi–Salam–Zanderighi 2010; Ortiz–Paić–Cuautle 2015 |
| Track selection (|η|<0.8, p_T>0.15 GeV/c, minimum N_tracks), S₀ resolution | ALICE EPJC 79 (2019) 857; ALICE arXiv:2310.10236 |
| p_T-weighted vs unweighted S₀ (S₀^{pT=1}) | ALICE arXiv:2310.10236 |
| Unfolding / detector-response correction of S₀ classes; class migration | ALICE EPJC 79 (2019) 857 and arXiv:2310.10236, analysis sections ⚠ *(confirm method per paper)* |
| Multiplicity-estimator choice (mid-rapidity CL1 vs forward V0M) and autocorrelation bias | ALICE arXiv:2310.10236; Ortiz et al. *Universe* 2024 |
| Flattenicity cell segmentation and stability | Ortiz et al. PRD 107 (2023) 076012 |
| V0 channel-by-channel calibration, percentile classes, (1−ρ) | ALICE PRD 111 (2025) 012010 |
| Run 3 detector change (V0 → FIT) | ALICE Run 3 preliminaries; Gyulai et al. 2026 |
| Biases of each estimator compared | Muñoz Méndez & Ortiz 2025; Ortiz et al. *Universe* 2024 |

---

## 7. Reviews and suggested reading order

**Reviews**
- ★ **A. Ortiz**, "Experimental results on event shapes at hadron colliders," *Adv. Ser. Direct. High Energy Phys.* 29 (2018) 343, arXiv:1705.02056.
- ★ "Event Topology Classifiers at the Large Hadron Collider," arXiv:2506.03782 (2025). Reviews S₀, R_T and flattenicity across all LHC experiments, with Run 3/4 outlook. ⚠ *Author list not captured; it appears to be by Tripathy, Sahoo et al.*
- ◆ J. F. Grosse-Oetringhaus, U. A. Wiedemann, "A decade of collectivity in small systems," arXiv:2407.07484. Gives the physics context for why the classifiers matter.

**Suggested order (about 10 papers to get up to speed)**
1. Ortiz review (arXiv:1705.02056)
2. Banfi–Salam–Zanderighi 2010
3. Cuautle et al. 2014, then Ortiz–Paić–Cuautle 2015
4. ALICE EPJC 79 (2019) 857
5. ALICE arXiv:2310.10236 (spherocity + strangeness)
6. Ortiz & Paić 2022, then Ortiz et al. PRD 107 (2023)
7. ALICE PRD 111 (2025) 012010 (flattenicity)
8. Mallick et al. J. Phys. G 48 (2021), then Choudhury & Nandy (A–A caveat)
9. Event Topology Classifiers review (arXiv:2506.03782)
