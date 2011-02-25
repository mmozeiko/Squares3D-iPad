#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "common.h"
#include "vmath.h"
#include "body.h"
#include "level.h"
#include "timer.h"

class RefereeBase;
class Collision;

static const float FIELD_LENGTH = 3.0f;

class Player : public Collideable
{
public:
    Player(const Profile* profile, Level* level);
    virtual ~Player();

    virtual void halt() {}
    virtual void release() {}
    
    void setDirection(const Vector& direction);
    void setRotation(const Vector& rotation);
    void setJump(bool needJump);
    void setKick(bool needKick);

    void setPositionRotation(const Vector& position, const Vector& rotation);

    virtual void control() = 0;

    Vector getPosition() const;
    Vector getFieldCenter() const;

    void renderColor() const;

    //todo: maybe private
    void onCollide(const Body* other, const Vector& position, float speed);
    void onSetForceAndTorque(float timestep);
    
    RefereeBase*      m_referee;
    Body*             m_body;
    const Profile*    m_profile;

    //player must recognize his field
    Vector            m_lowerLeft;
    Vector            m_upperRight;

    float             m_radius;

protected:
    NewtonJoint* m_upVector;

    bool         m_isOnGround; // TODO: rename, current name is incorrect
    bool         m_jump;
    bool         m_kick;

    Vector       m_direction;
    Vector       m_rotation;
    Timer        m_timer;

    float        m_speedCoefficient;
    float        m_accuracyCoefficient;
    float        m_jumpCoefficient;
    float        m_rotateSpeedCoefficient;


    const Collision* m_levelCollision;
    Body*            m_ballBody;
};

#endif
