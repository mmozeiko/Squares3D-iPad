#include "body.h"
#include "collision.h"
#include "game.h"
#include "xml.h"
#include "world.h"
#include "level.h"
#include "properties.h"
#include "geometry.h"

Body::Body(const string& id, const Level* level, const CollisionSet& collisions):
    m_id(id),
    m_newtonBody(NULL),
    m_matrix(),
    m_collisions(collisions),
    m_soundable(false),
    m_important(false),
    m_totalMass(0.0f),
    m_totalInertia(),
    m_collideable(NULL),
    m_velocity(),
    m_kickForce(),
    m_level(level)
{
    createNewtonBody(Vector::Zero, Vector::Zero);
}    

Body::Body(const XMLnode& node, const Level* level):
    m_id(""),
    m_newtonBody(NULL),
    m_matrix(),
    m_collisions(),
    m_soundable(false),
    m_important(false),
    m_totalMass(0.0f),
    m_totalInertia(),
    m_collideable(NULL),
    m_velocity(),
    m_kickForce(),
    m_level(level)
{
    m_id = node.getAttribute("id");
    m_soundable = node.getAttribute<int>("soundable", 0) == 1;
    m_important = node.getAttribute<int>("important", 0) == 1;
    
    Vector position(0.0f, 0.0f, 0.0f);
    Vector rotation(0.0f, 0.0f, 0.0f);

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "position")
        {
            position = node.getAttributesInVector("xyz");
        }
        else if (node.name == "rotation")
        {
            rotation = node.getAttributesInVector("xyz") * DEG_IN_RAD;
        }
        else if (node.name == "collision")
        { 
            m_collisions.insert(World::instance->m_level->getCollision(node.value));
        }
        else
        {
            Exception("Invalid body, unknown node - " + node.name);
        }
    }

    if (m_collisions.size() == 0)
    {
        Exception("No collisions were found for body '" + m_id + "'");
    }

    createNewtonBody(position, rotation);
}

void Body::setKickForce(const Vector& force)
{
    m_kickForce = force;
}

void Body::setMatrix(const Matrix& matrix)
{
    NewtonBodySetMatrix(m_newtonBody, matrix.m);
}

void Body::setTransform(const Vector& position, const Vector& rotation)
{
    NewtonSetEulerAngle(rotation.v, m_matrix.m);
    m_matrix = Matrix::translate(position) * m_matrix;
            
    NewtonBodySetMatrix(m_newtonBody, m_matrix.m);
}

void Body::createNewtonBody(const Vector&          position,
                            const Vector&          rotation)
{
    Vector totalOrigin;

    NewtonCollision* newtonCollision = NULL;
    if (m_collisions.size() == 1)
    {
        const Collision* collision = *m_collisions.begin();

        m_totalMass = collision->m_mass;
        m_totalInertia = collision->m_inertia;
        totalOrigin = collision->m_origin;

        newtonCollision = collision->m_newtonCollision;
    }
    else if (m_collisions.size() > 1)
    {
        vector<NewtonCollision*> newtonCollisions(m_collisions.size());
        int cnt = 0;
        for each_const(CollisionSet, m_collisions, iter)
        {
            const Collision* collision = *iter;
            newtonCollisions[cnt++] = collision->m_newtonCollision;
            m_totalMass += collision->m_mass;
            m_totalInertia += collision->m_inertia;
            totalOrigin += collision->m_origin;
        }
        totalOrigin /= m_totalMass;

        newtonCollision = NewtonCreateCompoundCollision(
                                                World::instance->m_newtonWorld,
                                                cnt,
                                                &newtonCollisions[0],
                                                0);

        for each_const(vector<NewtonCollision*>, newtonCollisions, collision)
        {
            //NewtonReleaseCollision(World::instance->m_newtonWorld, *collision);
        }
    }
    
    NewtonSetEulerAngle(rotation.v, m_matrix.m);
    m_matrix = Matrix::translate(position) * m_matrix;

    m_newtonBody = NewtonCreateBody(World::instance->m_newtonWorld, newtonCollision, m_matrix.m);
    NewtonBodySetUserData(m_newtonBody, static_cast<void*>(this));

    if (m_id != "level" && m_id != "wall")
    {
        NewtonBodySetMassMatrix(m_newtonBody, m_totalMass, m_totalInertia.x, m_totalInertia.y, m_totalInertia.z);
        NewtonBodySetCentreOfMass(m_newtonBody, totalOrigin.v);
        NewtonBodySetAutoSleep(m_newtonBody, 0);
        NewtonBodySetForceAndTorqueCallback(m_newtonBody, onSetForceAndTorque);
    }

    // danger - ball depends on newtonCollision variable
    //NewtonReleaseCollision(World::instance->m_newtonWorld, newtonCollision);
}

Body::~Body()
{
    //TODO: destroy m_body

    /*
    for each_const(CollisionSet, m_collisions, iter)
    {
        NewtonReleaseCollision(World::instance->m_newtonWorld, (*iter)->m_newtonCollision);
    }
    */
}

void Body::prepare()
{
    NewtonBodyGetMatrix(m_newtonBody, m_matrix.m);
    NewtonBodyGetVelocity(m_newtonBody, m_velocity.v);
}

Vector Body::getPosition() const
{
    return m_matrix.row(3);
}

Vector Body::getRotation() const
{
    return m_matrix.row(0);
}

Vector Body::getVelocity() const
{
    return m_velocity;
}

void Body::onSetForceAndTorque(float timestep)
{
    if (NewtonBodyGetFreezeState(m_newtonBody) != 1)
    {
        Vector force = m_level->m_gravity * m_totalMass;
        NewtonBodySetForce(m_newtonBody, force.v);
    }
    
    if (m_kickForce != Vector::Zero)
    {
        NewtonBodySetVelocity(m_newtonBody, Vector::Zero.v);
        NewtonBodyAddForce(m_newtonBody, m_kickForce.v);
        m_kickForce = Vector::Zero;
    }

    if (m_collideable != NULL)
    {
        m_collideable->onSetForceAndTorque(timestep);
    }
}

void Body::onSetForceAndTorque(const NewtonBody* body, dFloat timestep, int threadIndex)
{
    Body* self = static_cast<Body*>(NewtonBodyGetUserData(body));
    self->onSetForceAndTorque(timestep);
}

void Body::render() const
{
    Video::instance->begin(m_matrix);

    for each_const(CollisionSet, m_collisions, iter)
    {
        (*iter)->render();
    }

    Video::instance->end();
}

void Body::setCollideable(Collideable* collideable)
{
    m_collideable = collideable;
}

bool Body::isMovable()
{
    return (m_totalMass != 0);
}

void Body::onCollide(const Body* other, const Vector& position, float speed)
{
    if (m_collideable != NULL)
    {
        m_collideable->onCollide(other, position, speed);
    }
}

void Body::onCollideHull(const Body* other)
{
    if (m_collideable != NULL)
    {
        m_collideable->onCollideHull(other);
    }
}

float Body::getMass() const
{
    return m_totalMass;
}

const Vector& Body::getInertia() const
{
    return m_totalInertia;
}
