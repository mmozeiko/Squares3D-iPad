#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"
#include "vmath.h"
#include "state.h"
#include "system.h"
#include "timer.h"

const float DT = 1.0f / 30.0f;

class Config;
class Language;
class Video;
class Audio;
class Network;
class Input;
class FPS;
class Profile;
class Music;

typedef vector<Profile*> ProfilesVector;

class Game : public System<Game>, public NoCopy
{
public:
    Game();
    ~Game();
    
    void render(float delta);
    void update(float delta);
    void saveUserData();

    // Singletons
    Config*     m_config;
    Language*   m_language;
    Video*      m_video;
    Audio*      m_audio;
    Network*    m_network;
    Input*      m_input;
    //

    ProfilesVector  m_cpuProfiles[4];
    Profile*        m_userProfile;    

private:
    State*      m_state;
    Timer       m_timer;
    FPS*        m_fps;
    
    float       m_accum;
    float       m_currentTime;
    
    Music*      m_menuMusic;

    bool        m_fixedTimestep;
    int         m_unlockable;
    int         m_current;

    State* switchState(const State::Type newState);
    void loadCpuData();
    void loadUserData();
};

#endif
