/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani, Chun Shen
 * All rights reserved.
 *
 * This code is adapted from Chun Shen's code
 * for the purpose of computing hadronic resonance
 * correlations..
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticleDecayMode
#define CAP__ParticleDecayMode
#include <iostream>
#include <iomanip>
#include <vector>
#include "Aliases.hpp"
#include "ParticleType.hpp"
#include "ParticleDb.hpp"
#include "ClassDef.hpp"

using namespace std;

namespace CAP
{

  class ParticleType;
  class ParticleDb;

  //!
  //! @brief Decay mode of a particle type: branching ratio and products (2 to 4), identified by their PDG codes and
  //! types.
  //!
  //! The database keeps the branching ratio as declared in its files and whether it is completed by a Clebsch-Gordan
  //! coefficient (see ParticleDbXml::addDecayMode()); branchingRatio() is the ratio used to generate decays.
  //!
  class ParticleDecayMode
  {
  private:

  double _branchingRatio;          //!< branching ratio used to generate decays
  double _declaredBranchingRatio;  //!< branching ratio as given in the database (negative if not set: same as _branchingRatio)
  bool   _clebschGordan;           //!< whether _branchingRatio is _declaredBranchingRatio completed by a Clebsch-Gordan coefficient
  //! PDG codes of the products.
  vector<int> _childrenPdgCodes;
  //! Types of the products (owned by the database).
  vector<ParticleType*> _childrenTypes;

  public:
  //! @brief Default constructor: branching ratio 1, no products.
  ParticleDecayMode();
  //! @brief Copy constructor. @param source mode to copy
  ParticleDecayMode(const ParticleDecayMode & source);
  //! @brief Destructor.
  virtual ~ParticleDecayMode();
  //! @brief Copy assignment. @param source mode to copy @return this mode
  ParticleDecayMode & operator=(const ParticleDecayMode & source);
  //! @brief Find the product types from their PDG codes in a database.
  //! @param collection database @throws InvalidIndexParticleTypeException for an unknown code
  void resolveTypes(ParticleDb & collection);

  //! @brief Set the branching ratio used to generate decays. @param value ratio
  void   setBranchingRatio(double value) { _branchingRatio = value; }
  //! @brief Branching ratio used to generate decays. @return ratio
  double branchingRatio() const { return _branchingRatio; }
  //! @brief Set the branching ratio as given in the database (before any Clebsch-Gordan completion). @param value ratio
  void   setDeclaredBranchingRatio(double value) { _declaredBranchingRatio = value; }
  //! @brief Branching ratio as given in the database; branchingRatio() if it was not set. @return ratio
  double declaredBranchingRatio() const { return _declaredBranchingRatio>=0.0 ? _declaredBranchingRatio : _branchingRatio; }
  //! @brief Set whether branchingRatio() is the declared ratio completed by a Clebsch-Gordan coefficient. @param value flag
  void   setClebschGordan(bool value) { _clebschGordan = value; }
  //! @brief Whether branchingRatio() is the declared ratio completed by a Clebsch-Gordan coefficient. @return flag
  bool   clebschGordan() const { return _clebschGordan; }
  //! @brief Number of products. @return number of products
  int    nChildren() const { return _childrenPdgCodes.size(); }
  //! @brief Add a product by PDG code (its type is found by resolveTypes()). @param pdgCode PDG code
  void   addChild(int pdgCode) {  _childrenPdgCodes.push_back(pdgCode); }
  //! @brief Add a product. @param type type of the product
  void   addChild(ParticleType* type);
  //! @brief PDG codes of the products. @return codes
  inline const vector<int> & childrenPdgCodes() const {  return _childrenPdgCodes; }
  //! @brief PDG codes of the products. @return codes
  inline vector<int> & childrenPdgCodes() {  return _childrenPdgCodes; }
  //! @brief Types of the products. @return types
  const vector<ParticleType*> & childrenTypes() const {   return _childrenTypes; }
  //! @brief Types of the products. @return types
  vector<ParticleType*> & childrenTypes() {   return _childrenTypes; }
  //! @brief Type of a product. @param index index @return type @throws ParticleDecayException if index>=nChildren()
  const ParticleType & childType(unsigned int index) const;
  //! @brief Type of a product. @param index index @return type @throws ParticleDecayException if index>=nChildren()
  ParticleType & childType(unsigned int index);
  //! @brief PDG code of a product. @param index index @return code, or -1 if index>=nChildren()
  int childPdgCode(unsigned int index) const ;
  //! @brief PDG code of a product. @param index index @return code, or -1 if index>=nChildren()
  int childPdgCode(unsigned int index);
  //! @brief Print the branching ratio and the products.
  void print();

  ClassDef(ParticleDecayMode,0)
  };

} // namespace CAP

#endif  // CAP__ParticleDecayMode

