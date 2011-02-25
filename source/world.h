#ifndef __WORLD_H__
#define __WORLD_H__

#include <newton.h>
#include "common.h"
#include "vmath.h"
#include "system.h"
#include "state.h"
#include "video.h"

class Camera;
class Player;
class SkyBox;
class Level;
class Music;
class RefereeLocal;
class RefereeBase;
class Ball;
class Messages;
class Message;
class ScoreBoard;
class Grass;
class Profile;
class Game;
class InputMover;
class InputButton;

typedef vector<Profile*> ProfilesVector;

class World : public State, public System<World>
{
public:
    World(Profile* userProfile, int& unlockable, int current);
    ~World();

    void init();
    
    void control();
    void updateStep(float delta);
    void update(float delta);
    void prepare();
    void render() const;
    State::Type progress();

    void begin2D();
    void end2D();

    Camera*          m_camera;
    SkyBox*          m_skybox;
    Grass*           m_grass;
    NewtonWorld*     m_newtonWorld;
    Level*           m_level;
    vector<Player*>  m_localPlayers;
    Ball*            m_ball;
    RefereeBase*     m_referee;
    Messages*        m_messages;
    ScoreBoard*      m_scoreBoard;

    int              m_current;

private:
    int&           m_unlockable;

    bool           m_freeze;
    Profile*       m_userProfile;
    Message*       m_escMessage;
    Message*       m_menuMessage;
    Vector         m_lightPosition;
    
    void*          m_cameraTouch;
    int            m_cameraLastX;
    int            m_cameraLastY;

    InputMover*    m_inputMover;
    InputButton*   m_inputJump;
    InputButton*   m_inputCatch;

    void renderScene() const;

    State::Type     m_nextState;

    void setLight(const Vector& position);

    Message* createMenuMessage();
};

#endif

