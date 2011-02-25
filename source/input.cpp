#include "input.h"
#include "video.h"
#include "glue.h"

template <class Input> Input* System<Input>::instance = NULL;

Touches Input::m_touches;
Touches Input::m_empty;
vector<void*> Input::m_buffer;

void touch_begin(void* touch, int x, int y)
{
    if (Input::instance != NULL);
    {
        Input::instance->m_touches.insert(make_pair(touch, make_pair(y, x)));
        Input::instance->m_buffer.push_back(touch);
    }
}

void touch_move(void* touch, int x, int y)
{
    if (Input::instance != NULL);
    {
        Input::instance->m_touches[touch] = make_pair(y, x);
    }
}

void touch_end(void* touch)
{
    if (Input::instance != NULL);
    {
        Input::instance->m_touches.erase(touch);
    }
}

Input::Input() : m_waitForRelease(false)
{
    clog << "Initializing input... " << endl;
}

Input::~Input()
{
    clog << "Closing input." << endl;
}

void Input::init()
{
    m_touches.clear();
}

void Input::waitForRelease()
{
    m_waitForRelease = true;
}

void Input::update()
{
    if (m_waitForRelease && m_touches.empty())
    {
        m_waitForRelease = false;
    }
}

void Input::clearBuffer()
{
    m_buffer.clear();
}

void* Input::popTouch()
{
    if (m_buffer.empty())
    {
        return NULL;
    }
    
    void* ret = m_buffer[0];
    m_buffer.erase(m_buffer.begin());
    return ret;
}

const Touches& Input::touches() const
{
    return m_waitForRelease ? m_empty : m_touches;
    
}
