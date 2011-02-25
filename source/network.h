#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "common.h"
#include "timer.h"
#include "system.h"
#include "vmath.h"

class Body;
class Profile;
class Level;
class Player;
class Menu;
class RefereeBase;
class RemotePlayer;

struct ActiveBody
{
    Body*  body;
    Matrix lastPosition;
};

typedef vector<ActiveBody*> ActiveBodyVector;

class Network : public System<Network>, public NoCopy
{
public:
    Network();
    ~Network();

    void update();

    void add(Body* body);

    void setPlayerProfile(Profile* player);
    void setCpuProfiles(const vector<Profile*> profiles[], int level);
    void createRemoteProfiles();
    void setAiProfile(int idx, Profile* ai);
    Profile* getRandomAI();

    const vector<Profile*>& getCurrentProfiles() const;
    const vector<Player*>& createPlayers(Level* level);
    int getLocalIdx() const;
    void changeCpu(int idx, bool forward);
    bool isLocal(int idx) const;

    void setReferee(RefereeBase* referee);

    void updateAiProfile(int idx);
    void kickClient(int idx);

    Body* getBodyByName(const string& name) const;
    int getBodyIdx(const Body* body) const;

    string getMaxPlayerName() const;
    Vector getProfileColor(size_t idx) const;

    bool m_inMenu;

    bool m_needToStartGame;
    bool m_needToReallyStartGame;
   
    bool m_needToBeginGame;
    bool m_needToQuitGame;
    bool  m_playing;

    string getLevel() const;
    size_t getLevelCount() const;

private:
    ActiveBodyVector m_activeBodies;

    vector<Profile*> m_allProfiles[4];
    vector<Profile*> m_profiles;
    int              m_localIdx;
    bool             m_aiIdx[4];
    vector<Player*>  m_players;

    Menu*            m_menu;
    string           m_lobbySubmenu;
    string           m_joinSubmenu;

    Profile*         m_tmpProfile;
    set<Profile*>    m_garbage;

    int              m_ready_count;
    Timer            m_timer;
    Timer            m_bodyTimer;

    RefereeBase*     m_referee;
    bool             m_clientReady[4];

    StringVector     m_levelFiles;
    byte             m_curLevel;
    void             loadLevelList();
};

#endif
