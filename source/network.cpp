#include "network.h"
#include "random.h"
#include "body.h"
#include "profile.h"
#include "player_ai.h"
#include "player_local.h"
#include "network.h"
#include "menu.h"
#include "game.h"
#include "version.h"
#include "referee_base.h"
#include "world.h"
#include "properties.h"
#include "config.h"
#include "xml.h"

template <class Network> Network* System<Network>::instance = NULL;

Network::Network() :
    m_inMenu(false),
    m_menu(NULL),
    m_tmpProfile(NULL),
    m_ready_count(0),
    m_needToStartGame(false),
    m_needToReallyStartGame(false),
    m_needToBeginGame(false),
    m_needToQuitGame(false),
    m_localIdx(0)
{
    clog << "Initializing network." << endl;

    loadLevelList();

    m_profiles.resize(4);

    m_tmpProfile = new Profile();
    m_tmpProfile->m_name = "???";

    for (int i=0; i<4; i++) m_clientReady[i] = false;
}

Network::~Network()
{
    clog << "Closing network." << endl;

    delete m_tmpProfile;
}

void Network::setReferee(RefereeBase* referee)
{
    m_referee = referee;
}

void Network::update()
{
}

void Network::add(Body* body)
{
    ActiveBody* ac = new ActiveBody();
    ac->body = body;
    ac->lastPosition = body->m_matrix;
    m_activeBodies.push_back(ac);
}

const vector<Profile*>& Network::getCurrentProfiles() const
{
    return m_profiles;
}

void Network::setPlayerProfile(Profile* player)
{
    m_profiles[0] = player;
    m_localIdx = 0;
    m_aiIdx[0] = false;
}

void Network::setCpuProfiles(const vector<Profile*> profiles[], int level)
{
    vector<Profile*> temp;
    if (level == -1)
    {
        temp.assign(profiles[0].begin(), profiles[0].end());
        temp.insert(temp.end(), profiles[1].begin(), profiles[1].end());
        temp.insert(temp.end(), profiles[2].begin(), profiles[2].end());
        temp.insert(temp.end(), profiles[3].begin(), profiles[3].end());
        
        m_allProfiles[0] = profiles[0];
        m_allProfiles[1] = profiles[1];
        m_allProfiles[2] = profiles[2];
        m_allProfiles[3] = profiles[3];
    }
    else
    {
        temp.assign(profiles[level].begin(), profiles[level].end());
    }

    std::random_shuffle(temp.begin(), temp.end());
    for (int i = 0; i < 3; i++)
    {
        m_profiles[1+i] = temp[i];
        m_aiIdx[1+i] = true;
    }
}

Profile* Network::getRandomAI()
{
    bool found = true;
    int i, k;
    while (found)
    {
        i = Randoms::getIntN(4);
        k = Randoms::getIntN(static_cast<int>(m_allProfiles[i].size()));
        found = foundIn(m_profiles, m_allProfiles[i][k]);
    }
    return m_allProfiles[i][k];
}

void Network::createRemoteProfiles()
{
    for (int i = 0; i < 4; i++)
    {
        m_profiles[i] = m_tmpProfile;
        m_aiIdx[i] = false;
    }
}

const vector<Player*>& Network::createPlayers(Level* level)
{
    m_players.resize(4);
    for (int i=0; i<4; i++)
    {
        if (i == m_localIdx)
        {
            m_players[i] = new LocalPlayer(m_profiles[i], level);
        }
        else if (m_aiIdx[i])
        {
            m_players[i] = new AiPlayer(m_profiles[i], level);
        }
    }
    
    return m_players;
}

int Network::getLocalIdx() const
{
    return m_localIdx;
}

void Network::changeCpu(int idx, bool forward)
{
    int found = -1;
    int i;
    for (i=0; i<4; i++)
    {
        for (size_t k=0; k<m_allProfiles[i].size(); k++)
        {
            if (m_allProfiles[i][k] == m_profiles[idx])
            {
                found = static_cast<int>(k);
                break;
            }
        }
        if (found != -1)
        {
            break;
        }
    }

    assert(found != -1);

    // TODO: ugly ugly code
    bool ok = false;
    while (ok == false)
    {
        found += (forward ? +1 : -1);

        if (found >= static_cast<int>(m_allProfiles[i].size()))
        {
            i++;
            found = 0;
            if (i >= 4)
            {
                i = 0;
            }
        }
        else if (found < 0)
        {
            i--;
            if (i < 0)
            {
                i = 3;
            }
            found = static_cast<int>(m_allProfiles[i].size()-1);
        }

        ok = true;
        for (int k=0; k<4; k++)
        {
            if (m_profiles[k] == m_allProfiles[i][found] ||
                m_profiles[k]->m_color == m_allProfiles[i][found]->m_color)
            {
                ok = false;
                break;
            }
        }
        
    }

    m_profiles[idx] = m_allProfiles[i][found];
}

bool Network::isLocal(int idx) const
{
    return idx == m_localIdx || m_aiIdx[idx];
}

int Network::getBodyIdx(const Body* body) const
{
    for (size_t i = 0; i < m_activeBodies.size(); i++)
    {
        if (m_activeBodies[i]->body == body)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

Body* Network::getBodyByName(const string& name) const
{
    for each_const(ActiveBodyVector, m_activeBodies, iter)
    {
        if ((*iter)->body->m_id == name)
        {
            return (*iter)->body;
        }
    }
    return NULL;
}

void Network::setAiProfile(int idx, Profile* ai)
{
    m_profiles[idx] = ai;
}

string Network::getMaxPlayerName() const
{
    string m;
    for (int i=0; i<4; i++)
    {
        if (m_profiles[i]->m_name.size() > m.size())
        {
            m = m_profiles[i]->m_name;
        }
    }
    return m;
}

Vector Network::getProfileColor(size_t idx) const
{
    assert(idx >=0 && idx < 4);
    return m_profiles[idx]->m_color;
}

string Network::getLevel() const
{
    return m_levelFiles[m_curLevel];
}

void Network::loadLevelList()
{
    XMLnode xml;
    File::Reader in("/data/level/level_list.xml");
    if (!in.is_open())
    {
        Exception("Level file 'data/level/level_list.xml' not found");  
    }
    xml.load(in);

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "level")
        {
            m_levelFiles.push_back(node.getAttribute("file"));
        }
        else
        {
            Exception("Invalid cpu_profiles, unknown node - " + node.name);
        }
    }

    m_curLevel = 0;
}

size_t Network::getLevelCount() const
{
    return m_levelFiles.size();
}
