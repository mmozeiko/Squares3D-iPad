#ifndef __PROPERTY_H__
#define __PROPERTY_H__

#include <Newton.h>
#include "common.h"

class Properties;

class Property
{
    friend class Properties;
public:
    Property(float sF, float kF, float eC, float sC);

    void apply(const NewtonMaterial* material) const;

private:
    float staticFriction;
    float kineticFriction;
    float elasticityCoefficient;
    float softnessCoefficient;
};

#endif
