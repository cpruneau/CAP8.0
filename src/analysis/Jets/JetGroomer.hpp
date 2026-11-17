/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetGroomer.hpp
//! @brief JetGroomer: grooming of a jet (Soft Drop / modified mass drop, trimming, filtering, pruning, dynamical
//! grooming); GroomedJet: the result; JetGroomerCreator: the task creating the jet groomers of a job (JET_GROOMER).
#ifndef DATUM__JetGroomer
#define DATUM__JetGroomer
#include <vector>
#include "ManagedObject.hpp"
#include "ObjectCreator.hpp"
#include "Configuration.hpp"
#include "String.hpp"
#include "VectorLorentz.hpp"
#include "Jet.hpp"

namespace Datum
{

class Particle;

//!
//! @brief A jet after grooming: its four-momentum and constituents, and the splitting the groomer selected (Soft Drop and
//! dynamical grooming only).
//!
//! status: Groomed (the groomer kept a splitting or subjets), NoSplitting (Soft Drop in grooming mode or dynamical
//! grooming on a jet with no splitting satisfying the condition: the groomed jet is its hardest constituent, or the jet
//! itself if it has one constituent; z_g, R_g and k_T,g are undefined) or Failed (Soft Drop in tagging mode without a
//! splitting satisfying the condition; trimming without a subjet above the cut; pruning removing everything: no groomed
//! jet).
//!
//! The splitting variables, for the two subjets j1, j2 of the selected splitting (pT1 >= pT2), distances in rapidity
//! and azimuth: zg = pT2/(pT1 + pT2), rg = Delta R_12, ktg = pT2 Delta R_12; nSD (Soft Drop only): the number of
//! splittings of the primary branch (the harder subjet at each step, to the end of the declustering) satisfying the
//! Soft Drop condition. They are -1 where undefined (the other methods, or no splitting).
//!
struct GroomedJet
{
  //! @brief Outcome of the grooming.
  enum Status { Groomed, NoSplitting, Failed };

  Status status = Failed;                //!< outcome
  const Jet * jet = nullptr;             //!< the jet before grooming
  MATH::VectorLorentz momentum;          //!< four-momentum of the groomed jet
  std::vector<Particle*> constituents;   //!< its constituents
  double zg = -1.0;                      //!< momentum fraction of the selected splitting
  double rg = -1.0;                      //!< angle (Delta R) of the selected splitting
  double ktg = -1.0;                     //!< transverse momentum of the selected splitting
  int nSD = -1;                          //!< splittings of the primary branch satisfying the Soft Drop condition

  //! @brief Whether there is a groomed jet (not Failed). @return true if so
  bool kept() const { return status!=Failed; }
  //! @brief Whether a splitting was selected (z_g, R_g, k_T,g defined). @return true if so
  bool hasSplitting() const { return zg>=0.0; }
  //! @brief Transverse momentum of the groomed jet. @return pT
  double pt() const { return momentum.perp(); }
  //! @brief Mass of the groomed jet (0 if space-like by rounding). @return mass
  double mass() const;
};

//!
//! @brief A jet groomer: a method and its parameters, applied to the constituents of a jet (FastJet core; pruning with
//! fastjet::Pruner of the FastJet tools).
//!
//! Methods (METHOD) and their parameters (keys \<owner\>:JET_GROOMER_\<k\>:\<name\>):
//!  - SOFT_DROP (Larkoski, Marzani, Soyez, Thaler, arXiv:1402.2657): the constituents are reclustered (RECLUSTER, C/A
//!    by default) into a binary tree; going down the harder branch from the last clustering, the first splitting with
//!    min(pT1, pT2)/(pT1 + pT2) > Z_CUT (Delta R_12/R0)^BETA (and, if MU < 1, the mass drop max(m1, m2) < MU m) is kept:
//!    the groomed jet is its parent. The softer subjet of the other splittings is dropped. R0: the radius of the jet
//!    settings if 0. MODE GROOM (a jet without such splitting is kept as its hardest constituent; IRC safe for BETA > 0)
//!    or TAG (it fails; IRC safe for BETA <= 0). BETA = 0 is the modified mass-drop tagger (Dasgupta, Fregoso,
//!    Marzani, Salam, arXiv:1307.0007; MU = 1: no mass drop).
//!  - TRIMMING (Krohn, Thaler, Wang, arXiv:0912.1342): the constituents are reclustered into subjets of radius R_SUB
//!    (RECLUSTER; the paper uses kt); the subjets with pT < F_CUT pT_jet are dropped, the others form the groomed jet.
//!  - FILTERING (Butterworth, Davison, Rubin, Salam, arXiv:0802.2470): the constituents are reclustered into subjets of
//!    radius R_FILT (RECLUSTER, C/A); the N_KEEP hardest form the groomed jet.
//!  - PRUNING (Ellis, Vermilion, Walsh, arXiv:0903.5081, 0912.0033): the constituents are reclustered (RECLUSTER, C/A);
//!    at each recombination i, j -> p with min(pTi, pTj)/pTp < Z_CUT and Delta R_ij > D_cut = 2 R_CUT_FACTOR m/pT (m,
//!    pT of the jet; R_CUT_FACTOR 0.5 gives D_cut = m/pT of the paper), the softer one is dropped (fastjet::Pruner).
//!  - DYNAMICAL (Mehtar-Tani, Soto-Ontoso, Tywoniuk, arXiv:1911.00375): the constituents are reclustered (RECLUSTER,
//!    C/A); along
//!    the primary branch (the harder subjet at each step), the splitting with the largest
//!    kappa = z (1 - z) pT_parent (Delta R/R)^A (z = pT2/(pT1 + pT2), R: the radius of the jet settings) is kept: the
//!    groomed jet is its parent (the earlier, wider-angle emissions are dropped). A = 0.1: zDrop, 1: kTDrop, 2: TimeDrop.
//!
//! RECLUSTER: CA (default), KT or ANTIKT. A groomer selects nothing: the jets groomed are those of a jet filter.
//!
class JetGroomer : public ManagedObject
{
public:

  //! @brief Grooming methods.
  enum Method { SoftDrop, Trimming, Filtering, Pruning, Dynamical };
  //! @brief Soft Drop: what becomes of a jet without a splitting satisfying the condition.
  enum Mode { Grooming, Tagging };

  //! @brief Default groomer: Soft Drop, Z_CUT 0.1, BETA 0 (modified mass drop), grooming mode.
  JetGroomer();
  //! @brief Copy constructor. @param source groomer
  JetGroomer(const JetGroomer & source) = default;
  //! @brief Copy assignment. @param rhs groomer @return this groomer
  JetGroomer & operator=(const JetGroomer & rhs) = default;
  //! @brief Destructor.
  virtual ~JetGroomer() {}

  //! @brief Read the method and its parameters (\<owner\>:\<type\>_\<index\>:METHOD, ...). @param ownerName owner
  //! @param typeName JET_GROOMER @param configuration configuration @param index index
  //! @throws ConfigurationException if a value is invalid
  virtual void configure(const String & ownerName, const String & typeName, const Configuration & configuration,
                         unsigned int index) override;

  //! @brief Groom a jet. @param jet jet (its constituents and settings) @return the groomed jet
  GroomedJet groom(const Jet & jet) const;

  //! @brief Method from its name (SOFT_DROP, TRIMMING, FILTERING, PRUNING, DYNAMICAL). @param name name
  //! @return method @throws Exception if unknown
  static Method methodOf(const String & name);
  //! @brief Name of a method. @param method method @return name
  static String methodName(Method method);

  Method method = SoftDrop;                                    //!< method
  JetSettings::Algorithm recluster = JetSettings::CambridgeAachen; //!< reclustering algorithm
  double zCut = 0.1;          //!< Soft Drop, pruning: z cut
  double beta = 0.0;          //!< Soft Drop: angular exponent
  double r0 = 0.0;            //!< Soft Drop: R0 (0: the radius of the jet settings)
  double mu = 1.0;            //!< Soft Drop: mass drop (1: none)
  Mode mode = Grooming;       //!< Soft Drop: grooming or tagging mode
  double rSub = 0.2;          //!< trimming: radius of the subjets
  double fCut = 0.03;         //!< trimming: pT fraction of the jet below which a subjet is dropped
  double rFilt = 0.3;         //!< filtering: radius of the subjets
  int nKeep = 3;              //!< filtering: number of subjets kept
  double rCutFactor = 0.5;    //!< pruning: D_cut = 2 rCutFactor m/pT
  double a = 1.0;             //!< dynamical grooming: exponent of the angle
};

//!
//! @brief Task creating the jet groomers of a job (JET_GROOMER), see ObjectCreator; a subtask of FilterCreator.
//!
class JetGroomerCreator : public ObjectCreator<JetGroomer>
{
public:
  //! @brief Constructor.
  JetGroomerCreator() : ObjectCreator<JetGroomer>("JET_GROOMER")
  { appendClassName("JetGroomerCreator"); setName("JetGroomerCreator"); }
  //! @brief Copy. @return new task
  Task * clone() override { return new JetGroomerCreator(*this); }
};

} // namespace Datum

#endif /* DATUM__JetGroomer */
