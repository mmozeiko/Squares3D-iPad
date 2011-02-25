#pragma once

#include "common.h"
#include "input.h"
#include "vmath.h"

class LocalPlayer;

class InputMover : NoCopy
{
public:
    InputMover(LocalPlayer* player);
    ~InputMover();
    
    void* touch;
    
    bool contains(int x, int y) const;
    void update();
    void update(int x, int y);
    
    void render();
   
private:
    bool m_active;
    Vector m_dir;
    LocalPlayer* m_player;
};

