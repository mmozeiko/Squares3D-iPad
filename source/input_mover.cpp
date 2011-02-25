#include "input_mover.h"
#include "video.h"
#include "font.h"
#include "player_local.h"

static const int RADIUS = 100;
static const int RADIUS2 = 50;

InputMover::InputMover(LocalPlayer* player) : m_active(false), m_player(player), touch(NULL)
{
}

InputMover::~InputMover()
{
}

bool InputMover::contains(int x, int y) const
{
    int centerX = RADIUS + 10;
    int centerY = RADIUS + 100;

    if (m_active)
    {
        return true;
    }
    else
    {
        return sqr(abs(x - centerX)) + sqr(abs(y - centerY)) <= sqr(RADIUS);
    }
}

void InputMover::update()
{
    m_active = false;
    m_dir = Vector::Zero;
    m_player->setDirection(Vector::Zero);
}

void InputMover::update(int x, int y)
{
    int centerX = RADIUS + 10;
    int centerY = RADIUS + 100;

    m_dir = Vector(float(x - centerX), 0, float(y - centerY));

    float len2 = m_dir.magnitude2();
    /*if (len2 > sqr(RADIUS - RADIUS2))
    {
        m_dir *= (RADIUS - RADIUS2);
    }
    else */if (len2 < sqr(5))
    {
        m_dir = Vector::Zero;
    }

    m_player->setDir(m_dir);

    m_active = true;
}
    
void InputMover::render()
{
    int centerX = RADIUS + 10;
    int centerY = RADIUS + 100;

    const Font* font = Font::get("Arial_48pt_bold");

    font->begin();

    Video::instance->setColor(Vector(0,0,0,0.4f));
    
    Vector pos((float)centerX, (float)centerY, 0);
    Video::instance->renderRoundRect(pos, pos, (float)RADIUS);

    pos.x += m_dir.x;
    pos.y += m_dir.z;

    if (m_active)
    {
        Video::instance->setColor(Vector(0.5f,0.5f,0.5f,0.4f));
    }
    Video::instance->renderRoundRect(pos, pos, (float)RADIUS2);

    font->begin2();
    Video::instance->setColor(Vector::One);
    glTranslatef(pos.x, pos.y - font->getHeight() / 2.0f, 0);
    font->render("M", Font::Align_Center);
    font->end();
}
