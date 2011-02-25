#pragma once

#include "common.h"
#include "input.h"

class LocalPlayer;

class InputButton: NoCopy
{
public:
    InputButton(const std::string& text, LocalPlayer* player);
    ~InputButton();

    void* touch;
    
    bool contains(int x, int y) const;
    void update(bool active);
    
    void render();
   
private:
    LocalPlayer* m_player;
    bool m_active;
    int m_delta;
    std::string m_text;
};

