#include "input_button.h"
#include "video.h"
#include "font.h"
#include "player_local.h"

static const int RADIUS = 50;

InputButton::InputButton(const std::string& text, LocalPlayer* player)
    : m_active(false)
    , m_delta(text == "C" ? RADIUS : 2*RADIUS + 10 + RADIUS)
    , m_text(text)
    , m_player(player)
    , touch(NULL)
{
}

InputButton::~InputButton()
{
}

bool InputButton::contains(int x, int y) const
{
    IntPair res = Video::instance->getResolution();
    int centerX = res.first - (RADIUS + 10);
    int centerY = 100 + m_delta;

    if (m_active)
    {
        return true;
    }
    else
    {
        return sqr(abs(x - centerX)) + sqr(abs(y - centerY)) <= sqr(RADIUS);
    }
}

void InputButton::update(bool active)
{
    m_active = active;
    if (active)
    {
        if (m_text[0] == 'J')
        {
            m_player->jump();
        }
        else
        {
            m_player->catchBall();
        }
    }
}
    
void InputButton::render()
{
    IntPair res = Video::instance->getResolution();
    int centerX = res.first - (RADIUS + 10);
    int centerY = 100 + m_delta;

    const Font* font = Font::get("Arial_48pt_bold");

    font->begin();

    Video::instance->setColor(m_active ? Vector(0.5f,0.5f,0.5f,0.4f) : Vector(0,0,0,0.4f));
    Vector pos((float)centerX, (float)centerY, 0);
    Video::instance->renderRoundRect(pos, pos, (float)RADIUS);

    font->begin2();
    Video::instance->setColor(Vector::One);
    glTranslatef((float)centerX, centerY - font->getHeight() / 2.0f, 0);
    font->render(m_text, Font::Align_Center);
    font->end();
}
