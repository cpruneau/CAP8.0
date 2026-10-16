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
#include "XmlScalarField.hpp"

namespace CAP
{

  namespace
  {
  // DETAIL child of an axis by name, or by position if the children are not named
  const XmlElement & axisDetail(const XmlElement & axis, const char * name, unsigned int position)
  {
  for (const XmlElement * d : axis.children())
    if (d->attribute("name","")==name) return *d;
  if (position<axis.nChildren() && !axis.child(position).hasAttribute("name")) return axis.child(position);
  throw XmlException(String("Axis ")+axis.attribute("name","?")+" has no "+name+" detail","XmlScalarField::read()");
  }
  }

  MATH::ScalarField XmlScalarField::read(const XmlElement & vector3d)
  {
  if (!vector3d.isNamed("VECTOR3D")) throw XmlException(String("VECTOR3D element expected, got <")+vector3d.name()+">",__FUNCTION__);
  String fieldName = vector3d.attribute("name","");
  std::vector<const XmlElement *> axes = vector3d.children("AXIS");
  if (axes.size()!=3) throw XmlException(String("VECTOR3D ")+fieldName+" must have 3 AXIS elements",__FUNCTION__);
  double minimum[3], maximum[3];
  long   nPoints[3];
  for (int i=0; i<3; i++)
    {
    minimum[i] = axisDetail(*axes[i],"min",0).textDouble();
    maximum[i] = axisDetail(*axes[i],"max",1).textDouble();
    nPoints[i] = axisDetail(*axes[i],"pts",2).textInt();
    if (nPoints[i]<1) throw XmlException(String("VECTOR3D ")+fieldName+": invalid number of points",__FUNCTION__);
    }
  const XmlElement * data = nullptr;
  for (const XmlElement * c : vector3d.children()) if (!c->isNamed("AXIS")) { data = c; break; }
  if (!data) throw XmlException(String("VECTOR3D ")+fieldName+" has no data element",__FUNCTION__);
  std::vector<double> values = data->textDoubles();
  if ((long)values.size()!=nPoints[0]*nPoints[1]*nPoints[2])
    throw XmlException(String("VECTOR3D ")+fieldName+": number of values does not match the axes",__FUNCTION__);
  MATH::ScalarField field;
  field.setValue(fieldName,minimum[0],maximum[0],(unsigned int)nPoints[0],minimum[1],maximum[1],(unsigned int)nPoints[1],minimum[2],maximum[2],(unsigned int)nPoints[2]);
  std::size_t n = 0;
  for (long i=0; i<nPoints[0]; i++)
    for (long j=0; j<nPoints[1]; j++)
      for (long k=0; k<nPoints[2]; k++)
        field((unsigned int)i,(unsigned int)j,(unsigned int)k) = values[n++];
  return field;
  }

  MATH::ScalarField XmlScalarField::read(const XmlElement & element, const String & name)
  {
  const XmlElement * vector3d = element.findFirst("VECTOR3D","name",name);
  if (!vector3d) throw XmlException(String("No VECTOR3D element named ")+name,__FUNCTION__);
  return read(*vector3d);
  }

} // namespace CAP
