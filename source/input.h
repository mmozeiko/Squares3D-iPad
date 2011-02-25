#ifndef __INPUT_H__
#define __INPUT_H__

#include "common.h"
#include "system.h"

typedef std::map<void*, IntPair> Touches;

class Input : public System<Input>, public NoCopy
{
public:
    Input();
    ~Input();

    void init();
    void update();

    void waitForRelease();
    
    void clearBuffer();
    void* popTouch();

    const Touches& touches() const;

    static Touches m_touches;
    static vector<void*> m_buffer;

private:   
    static Touches m_empty;

    bool    m_waitForRelease;
};

#endif
