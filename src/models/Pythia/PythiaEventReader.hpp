/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__PythiaEventReader
#define CAP__PythiaEventReader
#include "EventProcessor.hpp"
#include "RootChainManager.hpp"
#include "EventProcessor.hpp"
#include "Event.hpp"
#include "EventFilter.hpp"
#include "EventEfficiency.hpp"
#include "EventSmearer.hpp"
#include "Particle.hpp"
#include "ParticleFilter.hpp"
#include "ParticleEfficiency.hpp"
#include "ParticleSmearer.hpp"
#include "ClassDef.hpp"

namespace CAP
{
//!
//! @brief Task reading PYTHIA 8 events from ROOT trees (TClonesArray of TParticle) and copying their particles into
//! the current CAP event.
//!
//! The input is set by \<name\>:DataImportPath (default $CAP_DATA_IMPORT_PATH), \<name\>:DataImportFile (file name,
//! wildcards allowed) and \<name\>:TreeName; initialize() maps the branches of the tree onto the members of the reader.
//! Which particles are kept is set by SaveFinalOnly, SavePhotons, SaveNeutrinos, SaveQuarks, SaveGaugeBosons.
//!
class PythiaEventReader
:
public EventProcessor,
public RootChainManager
{
public:

  //! @brief Default constructor.
  PythiaEventReader();
  //! @brief Copy constructor. @param task object to copy
  PythiaEventReader(const PythiaEventReader & task);
  //! @brief Copy assignment. @param task object to copy @return this object
  PythiaEventReader & operator=(const PythiaEventReader & task);
  //! @brief Destructor.
  virtual ~PythiaEventReader() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Initialize the task.
  virtual void initialize();
  //! @brief Execute the task (process the current event).
  virtual void execute();
  
protected:

  //! Whether only final-state particles are saved.
  bool saveFinalOnly;
  //! Whether photons are saved.
  bool savePhotons;
  //! Whether neutrinos are saved.
  bool saveNeutrinos;
  //! Whether quarks are saved.
  bool saveQuarks;
  //! Whether gauge bosons are saved.
  bool saveGaugeBosons;
  
  //! Maximum number of particles per event.
  static const int nMaxTracks = 5000;

  //! Number of particles of the current event.
  Int_t           nParticles;
  //! @name Tree leaves (one entry per particle, TParticle data members)
  //! @{
  UInt_t          tracks_fUniqueID[nMaxTracks];   //[tracks_]
  UInt_t          tracks_fBits[nMaxTracks];   //[tracks_]
  Short_t         tracks_fLineColor[nMaxTracks];   //[tracks_]
  Short_t         tracks_fLineStyle[nMaxTracks];   //[tracks_]
  Short_t         tracks_fLineWidth[nMaxTracks];   //[tracks_]
  Int_t           tracks_fPdgCode[nMaxTracks];   //[tracks_]
  Int_t           tracks_fStatusCode[nMaxTracks];   //[tracks_]
  Int_t           tracks_fMother[nMaxTracks][2];   //[tracks_]
  Int_t           tracks_fDaughter[nMaxTracks][2];   //[tracks_]
  Float_t         tracks_fWeight[nMaxTracks];   //[tracks_]
  Double_t        tracks_fCalcMass[nMaxTracks];   //[tracks_]
  Double_t        tracks_fPx[nMaxTracks];   //[tracks_]
  Double_t        tracks_fPy[nMaxTracks];   //[tracks_]
  Double_t        tracks_fPz[nMaxTracks];   //[tracks_]
  Double_t        tracks_fE[nMaxTracks];   //[tracks_]
  Double_t        tracks_fVx[nMaxTracks];   //[tracks_]
  Double_t        tracks_fVy[nMaxTracks];   //[tracks_]
  Double_t        tracks_fVz[nMaxTracks];   //[tracks_]
  Double_t        tracks_fVt[nMaxTracks];   //[tracks_]
  Double_t        tracks_fPolarTheta[nMaxTracks];   //[tracks_]
  Double_t        tracks_fPolarPhi[nMaxTracks];   //[tracks_]
  //! @}

  //! @name Indices of the incoming, outgoing, intermediate and kicked partons of the hard process
  //! @{
  Int_t           HP_Inc1;
  Int_t           HP_Inc2;
  Int_t           HP_Inc3;
  Int_t           HP_Inc4;
  Int_t           HP_Out1;
  Int_t           HP_Out2;
  Int_t           HP_Out3;
  Int_t           HP_Out4;
  Int_t           HP_Inter1;
  Int_t           HP_Inter2;
  Int_t           HP_Inter3;
  Int_t           HP_Inter4;
  Int_t           HP_Kick1;
  Int_t           HP_Kick2;
  Int_t           HP_Kick3;
  Int_t           HP_Kick4;

  // List of branches
  //! @}

  //! @name Tree branches
  //! @{
  TBranch        *b_tracks_;   //!
  TBranch        *b_tracks_fUniqueID;   //!
  TBranch        *b_tracks_fBits;   //!
  TBranch        *b_tracks_fLineColor;   //!
  TBranch        *b_tracks_fLineStyle;   //!
  TBranch        *b_tracks_fLineWidth;   //!
  TBranch        *b_tracks_fPdgCode;   //!
  TBranch        *b_tracks_fStatusCode;   //!
  TBranch        *b_tracks_fMother;   //!
  TBranch        *b_tracks_fDaughter;   //!
  TBranch        *b_tracks_fWeight;   //!
  TBranch        *b_tracks_fCalcMass;   //!
  TBranch        *b_tracks_fPx;   //!
  TBranch        *b_tracks_fPy;   //!
  TBranch        *b_tracks_fPz;   //!
  TBranch        *b_tracks_fE;   //!
  TBranch        *b_tracks_fVx;   //!
  TBranch        *b_tracks_fVy;   //!
  TBranch        *b_tracks_fVz;   //!
  TBranch        *b_tracks_fVt;   //!
  TBranch        *b_tracks_fPolarTheta;   //!
  TBranch        *b_tracks_fPolarPhi;   //!
  TBranch        *b_HP_Inc1;   //!
  TBranch        *b_HP_Inc2;   //!
  TBranch        *b_HP_Inc3;   //!
  TBranch        *b_HP_Inc4;   //!
  TBranch        *b_HP_Out1;   //!
  TBranch        *b_HP_Out2;   //!
  TBranch        *b_HP_Out3;   //!
  TBranch        *b_HP_Out4;   //!
  TBranch        *b_HP_Inter1;   //!
  TBranch        *b_HP_Inter2;   //!
  TBranch        *b_HP_Inter3;   //!
  TBranch        *b_HP_Inter4;   //!
  TBranch        *b_HP_Kick1;   //!
  TBranch        *b_HP_Kick2;   //!
  TBranch        *b_HP_Kick3;   //!
  TBranch        *b_HP_Kick4;   //!


  //! @}
  //! Input file(s) (DataImportPath/DataImportFile).
  String          _inputFiles;

  ClassDef(PythiaEventReader,0)
};

}

#endif /* CAP__PythiaEventReader */
