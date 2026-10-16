/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__XmlScalarField
#define CAP__XmlScalarField
#include "XmlElement.hpp"
#include "ScalarField.hpp"

namespace CAP
{

//!
//! @brief Reading of rank 3 scalar fields (MATH::ScalarField) stored in XML, as in the hydrodynamic models of
//! Therminator.
//!
//! A field is stored as a VECTOR3D element:
//! @code
//!   <VECTOR3D name="Distance">
//!     <AXIS name="X"> <DETAIL name="min">-15</DETAIL> <DETAIL name="max">15</DETAIL> <DETAIL name="pts">61</DETAIL> </AXIS>
//!     <AXIS name="Y"> ... </AXIS>
//!     <AXIS name="Z"> ... </AXIS>
//!     <DATA> v(0,0,0) v(0,0,1) ... </DATA>
//!   </VECTOR3D>
//! @endcode
//! The minimum, maximum and number of points of each axis are the DETAIL children named min, max and pts (or, without
//! names, the first three children of the AXIS, in that order). The values follow in the first child that is not an
//! AXIS, separated by white space, with the last index varying fastest: v(i,j,k) for i, then j, then k.
//!
class XmlScalarField
{
public:

  //!
  //! @brief Field stored in a VECTOR3D element.
  //! @param vector3d VECTOR3D element @return field named after the element's name attribute
  //! @throws XmlException if the element is not a VECTOR3D, an axis is incomplete, or the number of values does not
  //! match the axes
  //!
  static MATH::ScalarField read(const XmlElement & vector3d);

  //!
  //! @brief Field stored in the VECTOR3D element of a document with the given name attribute.
  //! @param element element searched (and its sub-tree) @param name value of the name attribute of the VECTOR3D element
  //! @return field @throws XmlException if there is no such element, or see read(vector3d)
  //!
  static MATH::ScalarField read(const XmlElement & element, const String & name);
};

} // namespace CAP

#endif
