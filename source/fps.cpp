#include <cmath>

#include "video.h"
#include "fps.h"
#include "font.h"
#include "timer.h"
#include "config.h"
#include "language.h"

FPS::FPS(const Timer& timer, const Font* font, const Vector& color)
    : m_time(timer.read()), m_nextTime(m_time + 1.0f), m_frames(0), m_totalFrames(0),
      m_timer(timer), m_font(font), m_fps(), m_width(0), m_color(color)
{
}

void FPS::reset()
{
    m_time = m_timer.read();
    m_nextTime = m_time + 1.0f;
    m_frames = 0;
    m_totalFrames = 0;
}

void FPS::update()
{
    m_frames++;
    float curTime = m_timer.read();
    if (curTime >= m_nextTime)
    {
        float delta = curTime - m_time;
        float fps = m_frames / delta;
        fps = std::floor(fps*10.0f)/10.0f;
        m_fps = Language::instance->get(TEXT_FPS_DISPLAY)(fps);
        if (fps-std::floor(fps) == 0.0f)
        {
            m_fps += ".0";
        }
        m_width = m_font->getWidth(m_fps);
        m_time = curTime;
        m_frames = 0;
        m_nextTime = curTime + 1.0f;
    }
    m_totalFrames++;
}


void FPS::render() const
{
    const IntPair res = Video::instance->getResolution();

    m_font->begin();

    glTranslatef(
        3, //static_cast<float>(res.first) - (m_width)-3, 
        static_cast<float>(res.second) -  (3 * m_font->getHeight(m_fps) + 5),
        0.0f);
    Video::instance->setColor(m_color);
    m_font->render(m_fps);

    m_font->end();
}

unsigned int FPS::frames() const
{
    return m_totalFrames;
}

float FPS::time() const
{
    return m_timer.read();
}

float FPS::fps() const
{
    float fps = m_totalFrames/m_timer.read();
    return std::floor(fps*10.0f)/10.0f;
}
