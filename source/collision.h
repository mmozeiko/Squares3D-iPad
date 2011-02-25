#ifndef __COLLISION_H__
#define __COLLISION_H__

#include <Newton.h>
#include "common.h"
#include "vmath.h"
#include "video.h"

class Body;
class XMLnode;
class Level;

class Collision : public NoCopy
{
    friend class Body;

public:
    static Collision* create(const XMLnode& node, Level* level);
    
    virtual void render() const = 0;
    virtual void renderTri(float x, float z) const {}
    virtual float getHeight(float x, float z) const;
    virtual float getRadius() const { return 0.0f; }

    NewtonCollision*  m_newtonCollision;

    virtual ~Collision();

protected:
    Collision(const XMLnode& node);

    void create(NewtonCollision* collision);
    void create(NewtonCollision* collision, int propertyID, float mass);

private:
    Vector m_origin;
    Vector m_inertia;
    float  m_mass;
};

#endif
