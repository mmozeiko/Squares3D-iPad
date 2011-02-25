#ifndef __FPS_H__
#define __FPS_H__

#include "common.h"
#include "vmath.h"

class Font;
class Timer;

class FPS : public NoCopy
{
public:
    FPS(const Timer& timer, const Font* font, const Vector& color = Vector(1.0f, 1.0f, 0.0f));
    void update();
    void render() const;

    void reset();
    unsigned int frames() const;
    float time() const;
    float fps() const;

private:
    float        m_time;
    float        m_nextTime;
    unsigned int m_frames;
    unsigned int m_totalFrames;
    const Timer& m_timer;
    const Font*  m_font;
    string       m_fps;
    int          m_width;
    
    Vector       m_color;
};

#endif
