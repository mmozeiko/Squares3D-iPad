#ifndef __PLAYER_LOCAL_H__
#define __PLAYER_LOCAL_H__

#include "common.h"
#include "player.h"

class LocalPlayer : public Player
{
public:
    LocalPlayer(const Profile* profile, Level* level);
    ~LocalPlayer();

    void control();

    void setDir(const Vector& dir);
    void jump();
    void catchBall();

private:
    float  m_mouseSens;
    
    bool   m_jump;
    bool   m_catch;
    Vector m_direction;
};

#endif
