#include "menu_submenu.h"
#include "menu_entries.h"
#include "menu_entries.h"
#include "menu.h"

#include "video.h"
#include "input.h"
#include "font.h"
#include "colors.h"
#include "sound.h"

void Submenu::control()
{
    const Touches& touches = Input::instance->touches();
    if (touches.empty() == false)
    {
        int x = touches.begin()->second.first;
        int y = touches.begin()->second.second;

        int videoHeight = Video::instance->getResolution().second;
        Vector mousePos = Vector(static_cast<float>(x), 0.0f, static_cast<float>(videoHeight - y));

        if (m_menu->m_mousePrevPos != mousePos)
        {
            //adjust active entry depending on mouse position
            for (size_t i = 0; i < m_entries.size(); i++)
            {
                Entry* currentEntry = m_entries[i];
                if ( (m_previousMousePos != mousePos)
                    && currentEntry->isMouseOver(mousePos)
                    && (currentEntry->isEnabled()))
                {
                    if (m_activeEntry != i)
                    {
                        m_menu->m_sound->play(m_menu->m_soundOver);
                    }
                    m_activeEntry = i;
                    Input::instance->waitForRelease();
                    break;
                }
            }
            m_menu->m_mousePrevPos = mousePos;
        }

        Entry* currentEntry = m_entries[m_activeEntry];
        
        if (currentEntry->isMouseOver(mousePos))
        {
            currentEntry->click();
            Input::instance->waitForRelease();
        }

        m_previousMousePos = mousePos;
    }
}

Submenu::~Submenu()
{
    for each_const(Entries, m_entries, iter)
    {
        delete *iter;
    }
}

void Submenu::addEntry(Entry* entry)
{
    m_entries.push_back(entry);
    m_height += entry->m_font->getHeight(entry->m_string) + 20;
}

void Submenu::center(const Vector& centerPos)
{
    m_centerPos = centerPos;
    Vector upperPos = centerPos;

    upperPos.y += m_height / 2;

    m_upper.y = upperPos.y + m_entries.front()->m_font->getHeight();

    int maxX = 0;
        
    for (size_t i = 0; i < m_entries.size(); i++)
    {      
        Entry* entry = m_entries[i];
        entry->calculateBounds(upperPos);
        upperPos.y -= m_entries[i]->m_font->getHeight(m_entries[i]->m_string) + 20;

        int l = entry->getMaxLeftWidth();
        int r = entry->getMaxRightWidth();
        if (l > maxX)
        {
            maxX = l;
        }
        if (r > maxX)
        {
            maxX = r;
        }
    }

    for (size_t i=0; i<m_entries.size(); i++)
    {
        m_entries[i]->setXBound(centerPos.x - maxX, centerPos.x + maxX);
    }

    m_upper.x = centerPos.x + maxX; //std::max(maxR, maxL);
    m_lower.x = centerPos.x - maxX; //std::max(maxR, maxL);
    m_lower.y = m_upper.y  - m_height + m_entries.back()->m_font->getHeight(m_entries.back()->m_string);
}

void Submenu::setTitle(const string& title, const Vector& position)
{
    m_title = title;
    m_titlePos = position;
}

void Submenu::activateNextEntry(bool moveDown)
{
    //should be used just for key input (up/down used)
    if (!m_entries.empty())
    {
        if (moveDown && (m_activeEntry == (m_entries.size() - 1)))
        {
            m_activeEntry = 0;
        }
        else if (!moveDown && (m_activeEntry == 0))
        {
            m_activeEntry = m_entries.size() - 1;
        }
        else
        {
            moveDown ? m_activeEntry++ : m_activeEntry--;
        }
        
        if (!m_entries[m_activeEntry]->isEnabled())
        {
            activateNextEntry(moveDown);
        }

    }
}

void Submenu::render() const
{
    Video::instance->setColor(Vector(0.0f, 0.0f, 0.0f, 0.5f));
    Video::instance->renderRoundRect(m_lower, m_upper, static_cast<float>(m_menu->m_font->getHeight()/2));

    for (size_t i = 0; i < m_entries.size(); i++)
    {
        Entry* entry = m_entries[i];

        glPushMatrix();

        if (entry->m_align == Font::Align_Center)
        {
    		glTranslatef(m_centerPos.x, entry->m_lowerLeft.z, m_centerPos.z);
        }
        else
        {
            int w = entry->getMaxLeftWidth();
    		glTranslatef(m_centerPos.x - w, entry->m_lowerLeft.z, m_centerPos.z);
        }

        Video::instance->setColor(White);
        
        /*
        if (m_activeEntry == i)
        {
            Video::instance->setColor(Yellow);              // active entry
        }
        else
        {
         */
            if (entry->isEnabled())
            {
                Video::instance->setColor(White);              // normal entry
            }
            else
            {
                Video::instance->setColor(Grey); // disabled entry
            }
        /*
        }
        */
        entry->m_font->begin2();
        entry->render();
        glPopMatrix();

    }

    if (!m_title.empty())
    {
        // TODO: move somewhere else
        m_menu->m_fontBig->begin();
        glPushMatrix();
        glTranslatef(m_titlePos.x, m_titlePos.y, 0);
        Video::instance->setColor(darker(Pink, 1.5f));
        m_menu->m_fontBig->render(m_title, Font::Align_Center);
        glPopMatrix();
        m_menu->m_fontBig->end();
    }
}
