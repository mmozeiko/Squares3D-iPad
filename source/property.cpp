#include <Newton.h>

#include "property.h"

Property::Property(float sF, float kF, float eC, float sC)
    : staticFriction(sF),
      kineticFriction(kF),
      elasticityCoefficient(eC),
      softnessCoefficient(sC)
{
}

void Property::apply(const NewtonMaterial* material) const
{
    NewtonMaterialSetContactSoftness(material, softnessCoefficient);
    NewtonMaterialSetContactElasticity(material, elasticityCoefficient);
    NewtonMaterialSetContactFrictionCoef(material, staticFriction, kineticFriction, 0);
    NewtonMaterialSetContactFrictionCoef(material, staticFriction, kineticFriction, 1);
}
