#include "player_local.h"
#include "camera.h"
#include "world.h"
#include "config.h"
#include "network.h"

LocalPlayer::LocalPlayer(const Profile* profile, Level* level)
    : Player(profile, level)
    , m_jump(false)
    , m_catch(false)

{
}

LocalPlayer::~LocalPlayer()
{
}

void LocalPlayer::setDir(const Vector& dir)
{
    m_direction = dir;
}

void LocalPlayer::jump()
{
    m_jump = true;
}

void LocalPlayer::catchBall()
{
    m_catch = true;
}

void LocalPlayer::control()
{
    Vector direction = m_direction;
    m_direction = Vector::Zero;

    direction /= 12.0f;

    Vector finalDirection = Matrix::rotateY(World::instance->m_camera->angleY()) * direction;

    Body* ball = World::instance->m_level->getBody("football");

    Vector ballPosition = ball->getPosition();
    Vector selfPosition = m_body->getPosition();

    Vector dir = ballPosition - selfPosition;
    Vector rot = m_body->getRotation();
    Vector rotation;
    rotation.y = ( rot % dir );

    bool jump = m_jump;
    bool kick = m_catch;
    rotation /= 5.0f;

    m_jump = false;
    m_catch = false;

    setDirection(finalDirection);
    setJump(jump);
    setRotation(rotation);
    setKick(kick);
}

