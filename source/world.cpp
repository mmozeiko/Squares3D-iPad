#include "openal_includes.h"

#include "world.h"
#include "player.h"
#include "camera.h"
#include "audio.h"
#include "player_local.h"
#include "player_ai.h"
#include "input.h"
#include "level.h"
#include "music.h"
#include "sound.h"
#include "file.h"
#include "skybox.h"
#include "properties.h"
#include "referee_local.h"
#include "referee_base.h"
#include "ball.h"
#include "messages.h"
#include "message.h"
#include "language.h"
#include "scoreboard.h"
#include "config.h"
#include "grass.h"
#include "network.h"
#include "profile.h"
#include "colors.h"
#include "game.h"
#include "fence.h"
#include "random.h"
#include "input_mover.h"
#include "input_button.h"

static const float OBJECT_BRIGHTNESS_1 = 0.5f; // shadowed
static const float OBJECT_BRIGHTNESS_2 = 0.6f; // lit
static const float GRASS_BRIGHTNESS_1 = 0.5f; // shadowed
static const float GRASS_BRIGHTNESS_2 = 0.5f; // lit

static const Vector playerPositions[4] = {Vector(- FIELD_LENGTH / 2, 1.5f, - FIELD_LENGTH / 2),
                                          Vector(- FIELD_LENGTH / 2, 1.5f,   FIELD_LENGTH / 2),
                                          Vector(  FIELD_LENGTH / 2, 1.5f,   FIELD_LENGTH / 2),
                                          Vector(  FIELD_LENGTH / 2, 1.5f, - FIELD_LENGTH / 2)
                                          };

template <class World> World* System<World>::instance = NULL;

Message* World::createMenuMessage()
{
    IntPair resolution = Video::instance->getResolution();

    return new Message(Language::instance->get(TEXT_PAUSE),
                       Vector(resolution.first/2.0f, resolution.second - 48.0f, 0),
                       Vector::One,
                       Font::Align_Center,
                       32,
                       true);
}

State::Type World::progress()
{
    return m_nextState;
}

World::World(Profile* userProfile, int& unlockable, int current) :
    m_camera(NULL),
    m_skybox(NULL),
    m_grass(NULL),
    m_newtonWorld(NULL),
    m_level(NULL),
    m_ball(NULL),
    m_referee(NULL),
    m_messages(NULL),
    m_scoreBoard(NULL),
    m_freeze(false),
    m_userProfile(userProfile),
    m_escMessage(NULL),
    m_unlockable(unlockable),
    m_current(current),
    m_cameraTouch(NULL),
    m_inputMover(NULL),
    m_inputJump(NULL),
    m_inputCatch(NULL)
{
    setInstance(this); // MUST go first

    setLight(Vector(-15.0f, 35.0f, 38.0f, 0.0f));
}

void World::init()
{
    if (m_newtonWorld != NULL)
    {
        delete m_skybox;

        delete m_messages;
        delete m_scoreBoard;

        for each_const(vector<Player*>, m_localPlayers, iter)
        {
            delete *iter;
        }
        m_localPlayers.clear();
       
        delete m_ball;
        delete m_referee;
        delete m_grass;
        delete m_level;
        delete m_camera;

        delete m_inputMover;
        //delete m_inputJump;
        //delete m_inputCatch;

        NewtonDestroyAllBodies(m_newtonWorld);
        NewtonDestroy(m_newtonWorld);

        m_skybox = NULL;

        m_messages = NULL;
        m_scoreBoard = NULL;

        m_ball = NULL;
        m_referee = NULL;
        m_level = NULL;
        m_newtonWorld = NULL;
        m_camera = NULL;
    }

    m_messages = new Messages();
    m_scoreBoard = new ScoreBoard(m_messages);

    m_messages->add2D(m_menuMessage = createMenuMessage());

    m_newtonWorld = NewtonCreate();

    // enable some Newton optimization
	NewtonSetPlatformArchitecture(m_newtonWorld, 0);
	NewtonSetSolverModel(m_newtonWorld, 0);
	NewtonSetThreadsCount(m_newtonWorld, 1);
	NewtonSetFrictionModel(m_newtonWorld, 0);
    
    m_level = new Level();

    StringSet tmp;
    m_level->load( ( m_current < 3 ? "world.xml" : "extra.xml" ), tmp);
    m_grass = new Grass(m_level);
    
    if (m_level->m_fences.empty() == false)
    {
        makeFence(m_level, m_newtonWorld);
    }

    m_skybox = new SkyBox(m_level->m_skyboxName);

    NewtonBodySetContinuousCollisionMode(m_level->getBody("football")->m_newtonBody, 1);

    m_localPlayers = Network::instance->createPlayers(m_level);

    m_ball = new Ball(m_level->getBody("football"), m_level->m_collisions["level"]);

    m_referee = new RefereeLocal(m_messages, m_scoreBoard);
    m_referee->m_field = m_level->getBody("field"); //referee now can recognize game field
    m_referee->m_ground = m_level->getBody("level");; //referee now can recognize ground outside
    
    //this is for correct registering when waiting for ball bounce in referee
    //it is handled specifically in Ball OnCollide
    m_ball->addBodyToFilter(m_referee->m_field);
    m_ball->addBodyToFilter(m_referee->m_ground);

    m_referee->registerBall(m_ball);

    m_referee->m_humanPlayer = m_localPlayers[Network::instance->getLocalIdx()];

    m_referee->registerPlayers(m_localPlayers);

    for (size_t i = 0; i < m_localPlayers.size(); i++)
    {
        m_localPlayers[i]->setPositionRotation(playerPositions[i], Vector::Zero);
        m_ball->addBodyToFilter(m_localPlayers[i]->m_body);
    }

    LocalPlayer* localPlayer = (LocalPlayer*)m_localPlayers[Network::instance->getLocalIdx()];
    m_inputMover = new InputMover(localPlayer);
    m_inputJump = new InputButton("J", localPlayer);
    m_inputCatch = new InputButton("C", localPlayer);

    m_scoreBoard->reset();

    if (!m_level->m_music.empty())
    {
        m_level->m_music[Randoms::getIntN(static_cast<unsigned int>(m_level->m_music.size()))]->play();
    }

    m_referee->m_sound->play(m_referee->m_soundGameStart);

    float angleAdjust = Network::instance->getLocalIdx() * 90.0f;
    m_camera = new Camera(Vector(0.0f, 1.0f, 12.0f), 20.0f, 0.0f + angleAdjust);
    Video::instance->setModelViewMatrix(m_camera->getModelViewMatrix());

    Input::instance->clearBuffer();
}

World::~World()
{  
    if (m_grass != NULL)
    {
        delete m_grass;
    }

    for each_const(vector<Player*>, m_localPlayers, iter)
    {
        delete *iter;
    }

    delete m_ball;
    delete m_referee;
    delete m_level;

    NewtonDestroyAllBodies(m_newtonWorld);
    NewtonDestroy(m_newtonWorld);

    delete m_scoreBoard;
    delete m_messages;
    delete m_skybox;
    delete m_camera;
}

static bool contains(const Touches& touches, void* touch)
{
    return touches.find(touch) != touches.end();
}

static void* other(const Touches& touches, void* touch)
{
    for each_const(Touches, touches, t)
    {
        if (t->first != touch)
        {
            return t->first;
        }
    }
    return NULL;
}

void World::control()
{
    m_nextState = State::Current;
    
    const Touches& touches = Input::instance->touches();

    IntPair resolution = Video::instance->getResolution();

    if (m_referee->m_over != NULL && m_menuMessage != NULL)
    {
        m_messages->remove(m_menuMessage);
        m_menuMessage = NULL;
    }

    while (true)
    {
        void* touch = Input::instance->popTouch();
        if (touch == NULL)
        {
            break;
        }
        
        Touches::const_iterator t = touches.find(touch);

        int xx = resolution.first - t->second.first;
        int yy = resolution.second - t->second.second;
        
        if (m_inputMover->touch == NULL && m_inputMover->contains(xx, yy))
        {
            m_inputMover->update(xx, yy);
            m_inputMover->touch = touch;
            continue;
        }
        
        if (m_inputCatch->touch == NULL && m_inputCatch->contains(xx, yy))
        {
            m_inputCatch->update(true);
            m_inputCatch->touch = touch;
            continue;
        }
        
        if (m_inputJump->touch == NULL && m_inputJump->contains(xx, yy))
        {
            m_inputJump->update(true);
            m_inputJump->touch = touch;
            continue;
        }
        
        if (m_menuMessage != NULL)
        {
            const Font* menuMessageFont = m_messages->m_fonts[m_menuMessage->getFontSize()];
            
            if (m_menuMessage->contains(xx, yy, menuMessageFont))
            {
                // user pressed pause
                
                m_freeze = true;
                float resY = static_cast<float>(Video::instance->getResolution().second);
                m_escMessage = new Message(
                                           Language::instance->get(TEXT_ESC_MESSAGE), 
                                           Vector(static_cast<float>(Video::instance->getResolution().first) / 2, 
                                                  resY / 2 + resY / 3, 
                                                  0.0f), 
                                           White,
                                           Font::Align_Center,
                                           32,
                                           true);
                m_messages->add2D(m_escMessage);
                m_messages->remove(m_menuMessage);
                m_menuMessage = NULL;
                
                Input::instance->waitForRelease();
                continue;                
            }
        }
        
        if (m_escMessage != NULL)
        {
            const Font* escMessageFont = m_messages->m_fonts[m_escMessage->getFontSize()];
            
            if (m_escMessage->contains(xx, yy, escMessageFont))
            {
                //user just wants to leave to menu in the middle of game - return State::MenuEasy
                //it DOESN`T change the unlockable level
                m_nextState = State::Menu;
            }
            else
            {
                m_freeze = false;
                m_messages->remove(m_escMessage);
                m_messages->add2D(m_menuMessage = createMenuMessage());
                m_escMessage = NULL;
            }
            
            Input::instance->waitForRelease();
            continue;                
        }
        
        if (m_referee->m_over != NULL)
        {
            if (m_referee->getLoserName() != m_userProfile->m_name) 
            {
                if ((m_current == m_unlockable) && (m_unlockable < 3))
                {
                    m_unlockable++;
                    Game::instance->saveUserData();
                }
                m_nextState = State::Menu;
            }
            else
            {
                if (m_referee->getLoserName() != m_userProfile->m_name)
                {
                    m_nextState = State::Menu;
                }
                else
                {
                    const Font* overMessageFont = m_messages->m_fonts[m_referee->m_over->getFontSize()];
                    
                    //here we don`t change the unlockable
                    if (m_referee->m_over->contains(xx, yy, overMessageFont))
                    {
                        //user wants to leave to menu and doesn`t want to retry - return State::MenuEasy
                        m_nextState = State::Menu;
                    }
                    else
                    {
                        //if none from above - we return State::m_current to retry (reload the same level)
                        init();
                    }
                }
            }

            Input::instance->waitForRelease();
            continue;
        }
        
        if (m_cameraTouch == NULL && t->second.first > 50 && t->second.first <= 1024 - 500)
        {
            // user starred dragging camera
            m_cameraTouch = touch;
            m_cameraLastX = t->second.first;
            m_cameraLastY = t->second.second;
        }
    }
    
    if (m_inputMover->touch != NULL)
    {
        Touches::const_iterator t = touches.find(m_inputMover->touch);
        
        if (t != touches.end())
        {
            int xx = resolution.first - t->second.first;
            int yy = resolution.second - t->second.second;

            m_inputMover->update(xx, yy);
        }
        else
        {
            m_inputMover->touch = NULL;
            m_inputMover->update();
        }
    }
    
    if (m_inputCatch->touch != NULL)
    {
        Touches::const_iterator t = touches.find(m_inputCatch->touch);
        
        if (t != touches.end())
        {
            m_inputCatch->update(true);
        }
        else
        {
            m_inputCatch->touch = NULL;
            m_inputCatch->update(false);
        }
    }
    
    if (m_inputJump->touch != NULL)
    {
        Touches::const_iterator t = touches.find(m_inputJump->touch);
        
        if (t != touches.end())
        {
            m_inputJump->update(true);
        }
        else
        {
            m_inputJump->touch = NULL;
            m_inputJump->update(false);
        }
    }

    if (m_cameraTouch != NULL)
    {
        Touches::const_iterator t = touches.find(m_cameraTouch);
        if (t == touches.end())
        {
            // user is stopped dragging
            m_cameraTouch = NULL;
            m_camera->control(0, 0);
        }
        else
        {
            // user is dragging camera
            m_camera->control(m_cameraLastX - t->second.first, t->second.second - m_cameraLastY);
            m_cameraLastX = t->second.first;
            m_cameraLastY = t->second.second;
        }
    }
    
    // player control
    for (size_t i=0; i<m_localPlayers.size(); i++)
    {
        m_localPlayers[i]->control();
    }
}

void World::updateStep(float delta)
{
    // updateStep is called more than one time in frame
    m_level->m_properties->update();
    //

    if (!m_freeze)
    {
        m_ball->triggerBegin();
        NewtonUpdate(m_newtonWorld, delta);
        m_ball->triggerEnd();

        for (size_t i=0; i<m_localPlayers.size(); i++)
        {
            if (m_localPlayers[i]->getPosition().y < -5.0f)
            {
                m_localPlayers[i]->setPositionRotation(playerPositions[i], Vector::Zero);
                NewtonBodySetFreezeState(m_localPlayers[i]->m_body->m_newtonBody, 0);
            }
        }

        if (m_ball->getPosition().y < -5.0f || 
            std::abs(m_ball->getPosition().x) > 80.0f ||
            std::abs(m_ball->getPosition().y) > 80.0f ||
            std::abs(m_ball->getPosition().z) > 80.0f)
        {
            if (m_referee->m_gameOver)
            {
                m_ball->setPosition0();
                NewtonBodySetFreezeState(m_ball->m_body->m_newtonBody, 0);
                NewtonBodySetVelocity(m_ball->m_body->m_newtonBody, Vector::Zero.v);
                NewtonBodySetOmega(m_ball->m_body->m_newtonBody, Vector::Zero.v);
            }
            else
            {
                m_referee->process(m_ball->m_body, m_level->getBody("level")); 
            }
        }

    }
}

void World::update(float delta)
{
    // update is called one time in frame

    alListenerfv(AL_POSITION, m_localPlayers[0]->getPosition().v);
    alListenerfv(AL_VELOCITY, m_localPlayers[0]->m_body->getVelocity().v);

    m_camera->update(delta);
    m_scoreBoard->update();
    m_referee->update();
    m_messages->update(delta);

    Video::instance->setModelViewMatrix(m_camera->getModelViewMatrix());
}

void World::prepare()
{
    m_camera->prepare();
    m_level->prepare();
}

void World::render() const
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    m_camera->render();

    glLightfv(GL_LIGHT0, GL_POSITION, m_lightPosition.v);
    glLightfv(GL_LIGHT0, GL_AMBIENT, (OBJECT_BRIGHTNESS_1*Vector::One).v);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, (OBJECT_BRIGHTNESS_2*Vector::One).v);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    renderScene();

    m_ball->renderShadow(m_lightPosition);
    for each_const(vector<Player*>, m_localPlayers, iter)
    {
        (*iter)->renderColor();
    }
    
    glLightfv(GL_LIGHT0, GL_DIFFUSE, (GRASS_BRIGHTNESS_1*Vector::One).v);
    glLightfv(GL_LIGHT0, GL_AMBIENT, (GRASS_BRIGHTNESS_2*Vector::One).v);

    m_grass->render();

    m_skybox->render(m_camera->getModelViewMatrix()); // IMPORTANT: must render after camera

    m_inputMover->render();
    m_inputJump->render();
    m_inputCatch->render();

    // text messages are last
    m_messages->render();
}

void World::renderScene() const
{
    m_level->render();
}

void World::setLight(const Vector& position)
{
    m_lightPosition = position;
}
