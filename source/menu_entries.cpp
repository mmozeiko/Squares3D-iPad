#include "menu_entries.h"
#include "menu_submenu.h"
#include "menu.h"

#include "language.h"
#include "input.h"
#include "video.h"
#include "colors.h"
#include "geometry.h"
#include "sound.h"
#include "network.h"
#include "profile.h"
#include "game.h"
#include "config.h"
#include "version.h"

/*** ENTRY ***/

Entry::Entry(Menu* menu, const string& stringIn, Font::AlignType align, const Font* font) : 
    m_string(stringIn), m_enabled(true), 
    m_menu(menu), m_forBounds(false), m_font(font), m_align(align)
{
    if (m_font == NULL)
    {
        m_font = menu->m_font;
    }
}

bool Entry::isMouseOver(const Vector& mousePos) const
{
    return isPointInRectangle(mousePos, m_lowerLeft, m_upperRight);
}

void Entry::calculateBounds(const Vector& position)
{
    m_forBounds = true;
    m_lowerLeft = Vector(position.x - m_font->getWidth(getString())/2.0f, 0, position.y);
    m_upperRight = Vector(position.x + m_font->getWidth(getString())/2.0f, 0, position.y + m_font->getHeight(getString()));
    m_forBounds = false;
}

void Entry::setXBound(float minX, float maxX)
{
    m_lowerLeft.x = minX;
    m_upperRight.x = maxX;
}

void Entry::render() const
{
    m_font->render(getString(), m_align);
}

int Entry::getMaxLeftWidth() const
{
    return m_font->getWidth(m_string)/2;
}

int Entry::getMaxRightWidth() const
{
    return m_font->getWidth(m_string)/2;
}

/*** WORLDENTRY ***/

void WorldEntry::click() 
{ 
    Network::instance->setPlayerProfile(Game::instance->m_userProfile);
    Network::instance->setCpuProfiles(Game::instance->m_cpuProfiles, m_switchTo);

    m_current = m_switchTo;
    m_menu->setState(State::World);
}

/*** QUITENTRY ***/

void QuitEntry::click() 
{ 
    m_menu->setState(State::Quit);
	exit(0);
}

void SubmenuEntry::click() 
{ 
    m_menu->setSubmenu(m_submenuToSwitchTo); 
    if (m_back)
    {
        m_menu->m_sound2->play(m_menu->m_soundBackClick);
    }
    else
    {
        m_menu->m_sound2->play(m_menu->m_soundClick);
    }
}

string LabelEntry::getString() const
{
    Video::instance->setColor(Vector::One);
    return m_string;
}
