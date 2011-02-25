#include "game.h"
#include "timer.h"
#include "config.h"
#include "language.h"
#include "video.h"
#include "audio.h"
#include "network.h"
#include "input.h"
#include "world.h"
#include "menu.h"
#include "intro.h"
#include "camera.h"
#include "font.h"
#include "fps.h"
#include "profile.h"
#include "vmath.h"
#include "colors.h"
#include "xml.h"
#include "random.h"
#include "music.h"
#include "glue.h"

template <class Game> Game* System<Game>::instance = NULL;

static const string USER_PROFILE_FILE = "/user.xml";

static const unsigned int M1 = 0x0BADC0DE;
static const unsigned int M2 = 0xDEADBEEF;

Game::Game() : 
    m_fixedTimestep(true),
    m_unlockable(-1),
    m_current(0),
    m_userProfile(NULL),
    m_state(NULL),
    m_menuMusic(NULL),
    m_accum(0.0f)
{
    // these and only these objects are singletons,
    // they all have public static instance attribute
    m_config = new Config();
    m_input = new Input();
    m_language = new Language();
    m_video = new Video();
    m_audio = new Audio();
    m_network = new Network();
	m_input->init();
    //

    loadUserData();
    loadCpuData();

    m_state = new Intro();
    m_fixedTimestep = false;

    m_state->init();
    
    clog << "Starting game..." << endl;
    
    m_video->init();
    
    m_timer.reset();
    m_fps = new FPS(m_timer, Font::get("Arial_32pt_bold"));
    m_currentTime = m_timer.read(); 
}

Game::~Game()
{
    clog << "Game finished... " << endl;

    delete m_fps;
    
    Font::unload();
    
    saveUserData();
 
    for (size_t i = 0; i < 4; i++)
    {
        for each_const(ProfilesVector, m_cpuProfiles[i], iter)
        {
            delete *iter;
        }
    }

    if (m_state != NULL)
    {
        delete m_state;
    }

    delete m_network;
    delete m_audio;
    delete m_video;
    delete m_language;
    delete m_input;
    delete m_config;
}

void Game::render(float delta)
{
    m_state->render();
    
    //
    //m_fps->update();
    //m_fps->render();
    //
}

void Game::update(float delta)
{
    m_audio->update();
    m_input->update();
    m_network->update();

    m_state->control();

    State::Type newState = m_state->progress();
    
    if (newState == State::Quit)
    {
        quit_requested = 1;
        return;
    }
    else if (newState != State::Current)
    {
        delete m_state;
        m_state = switchState(newState);
        m_state->init();
        m_state->update(DT);
        m_state->updateStep(DT);
        m_state->prepare();
        m_state->render();
        
        m_timer.reset();
        m_fps->reset();
        m_currentTime = m_accum = 0.0f;
    }

    float newTime = m_timer.read();
    float deltaTime = newTime - m_currentTime;
    m_currentTime = newTime;

    //clog << "deltaTime=" << deltaTime << " delta=" << delta << endl;
    
    if (m_fixedTimestep)
    {
        m_accum += deltaTime;

        m_state->update(m_accum - fmodf(m_accum, DT));

        while (m_accum >= DT)
        {
            m_state->updateStep(DT);
            m_accum -= DT;
        }
    }
    else
    {
        m_state->update(deltaTime);
        m_state->updateStep(deltaTime);
    }

    m_state->prepare();
}

State* Game::switchState(const State::Type nextState)
{
    if (nextState != State::Menu && m_menuMusic)
    {
        Audio::instance->unloadMusic(m_menuMusic);
        m_menuMusic = NULL;
    }

    m_fixedTimestep = true;
    switch (nextState)
    {
    case State::Intro:
        m_fixedTimestep = false;
        return new Intro();
    case State::Menu  :
        if (m_menuMusic == NULL)
        {
            m_menuMusic = Audio::instance->loadMusic("menu_soundtrack");
            m_menuMusic->play();
        }
        return new Menu(m_userProfile, m_unlockable, m_current);
    case State::World : return new World(m_userProfile, m_unlockable, m_current);
    default:
        assert(false);
        return NULL;
    }
}

void Game::loadUserData()
{
    clog << "Reading user information." << endl;
    XMLnode xml;
    File::Reader in(USER_PROFILE_FILE, true);
    if (in.is_open())
    {
        xml.load(in);
        for each_const(XMLnodes, xml.childs, iter)
        {
            const XMLnode& node = *iter;
            if (node.name == "profile")
            {
                if (m_userProfile != NULL)
                {
                    Exception("User profile should not be initialized twice!");
                }
                else
                {
                    m_userProfile = new Profile(node);
                }
            }
            else if (node.name == "other_data")
            {
                for each_const(XMLnodes, node.childs, iter)
                {
                    const XMLnode& node = *iter;
                    if (node.name == "magic")
                    {
                        unsigned int magic1 = node.getAttribute<unsigned int>("magic1");
                        unsigned int magic2 = node.getAttribute<unsigned int>("magic2");
                        unsigned int magic3 = node.getAttribute<unsigned int>("magic3");
                        unsigned int magic4 = node.getAttribute<unsigned int>("magic4");
                        
                        m_unlockable = ((magic1 + magic2 + M1) ^ magic4) - magic3 - M2;

                        if (m_unlockable < 0 || m_unlockable > 3)
                        {
                            m_unlockable = 0;
                        }
                        m_unlockable = 3;
                    }
                    else
                    {
                        string line = cast<string>(node.line);
                        Exception("Invalid profile file, unknown profile parameter '" + node.name + "' at line " + line);
                    }
                }
            }
            else
            {
                string line = cast<string>(node.line);
                Exception("Invalid profile file, unknown profile parameter '" + node.name + "' at line " + line);
            }
        }
        if ((m_unlockable == -1) || (m_userProfile == NULL))
        {
            Exception("Corrupted user profile file!");
        }
    }
    else
    {
        m_userProfile = new Profile();
        m_unlockable = 0;		
    }
	m_unlockable = 3;
}

void Game::saveUserData()
{
    clog << "Saving user data." << endl;

    XMLnode xml("xml");
    xml.childs.push_back(XMLnode("profile"));
    XMLnode& profile = xml.childs.back();
    profile.childs.push_back(XMLnode("name", m_userProfile->m_name));
    profile.childs.push_back(XMLnode("collision", m_userProfile->m_collisionID));
    profile.childs.push_back(XMLnode("properties"));
    profile.childs.back().setAttribute("speed", cast<string>(m_userProfile->m_speed));
    profile.childs.back().setAttribute("accuracy", cast<string>(m_userProfile->m_accuracy));
    profile.childs.back().setAttribute("jump", cast<string>(m_userProfile->m_jump));
    profile.childs.push_back(XMLnode("color"));
    profile.childs.back().setAttribute("r", cast<string>(m_userProfile->m_color.x));
    profile.childs.back().setAttribute("g", cast<string>(m_userProfile->m_color.y));
    profile.childs.back().setAttribute("b", cast<string>(m_userProfile->m_color.z));
    
    xml.childs.push_back(XMLnode("other_data"));
    XMLnode& other_data = xml.childs.back();
    other_data.childs.push_back(XMLnode("magic"));
    
    unsigned int magic1 = Randoms::getInt();
    unsigned int magic2 = Randoms::getInt();
    unsigned int magic3 = Randoms::getInt();
    unsigned int magic4 = (magic1 + magic2 + M1) ^ (magic3 + static_cast<unsigned int>(m_unlockable) + M2);
    other_data.childs.back().setAttribute("magic1", cast<string>(magic1));
    other_data.childs.back().setAttribute("magic2", cast<string>(magic2));
    other_data.childs.back().setAttribute("magic3", cast<string>(magic3));
    other_data.childs.back().setAttribute("magic4", cast<string>(magic4));

    File::Writer out(USER_PROFILE_FILE);
    if (!out.is_open())
    {
        Exception("Failed to open " + USER_PROFILE_FILE + " for writing");
    }
    xml.save(out);
    delete m_userProfile;
}

void Game::loadCpuData()
{
    XMLnode xml;
    File::Reader in("/data/level/cpu_players.xml");
    if (!in.is_open())
    {
        Exception("Level file 'data/level/cpu_players.xml' not found");  
    }
    xml.load(in);
    int checks[4] = {0,0,0,0};

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if ((node.name == "easy") || (node.name == "normal") || (node.name == "hard") || (node.name == "extra"))
        {
            size_t idx;
            if (node.name == "easy")
            {
                idx = 0;
            }
            else if (node.name == "normal")
            {
                idx = 1;
            }
            else if (node.name == "hard")
            {
                idx = 2;
            }
            else // if (node.name == "extra")
            {
                idx = 3;
            }

            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "profile")
                {
                    Profile* profile = new Profile(node);
                    m_cpuProfiles[idx].push_back(profile);
                    checks[idx]++;
                }
                else
                {
                    Exception("Invalid profile, unknown node - " + node.name);
                }
            }
        }
        else
        {
            Exception("Invalid cpu_profiles, unknown node - " + node.name);
        }
    }

    for (size_t i = 0; i < 4; i++)
    {
        if (checks[i] < 3)
        {
            Exception("Invalid cpu_profiles, there should be at least 3 profiles in each difficulty");
        }
    }
}
