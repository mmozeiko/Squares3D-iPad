#include "timer.h"
#include <mach/mach.h>
#include <mach/mach_time.h>

Timer::Timer(bool start) :
    m_running(start ? 1 : 0),
    m_elapsed(0.0f),
    m_resumed(0.0f)
{
    reset(start);
}

void Timer::pause()
{
    m_running--;
    if (m_running != 0)
    {
        return;
    }

    m_elapsed += mach_absolute_time() - m_resumed;
}

void Timer::resume()
{
    m_running++;
    if (m_running != 1)
    {
        return;
    }

    m_resumed = mach_absolute_time();
}

void Timer::reset(bool start)
{
    m_running = (start ? 1 : 0);
    if (start)
    {
        m_resumed = mach_absolute_time();
    }
    m_elapsed = 0;
}

float Timer::read() const
{
    uint64_t res;
    if (m_running <= 0)
    {
        res = m_elapsed;
    }
    res = mach_absolute_time() - m_resumed + m_elapsed;

    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    
    res *= info.numer;
    res /= info.denom;
    res /= 1000000;
    
    return (float)res / 1000.0f;
}
