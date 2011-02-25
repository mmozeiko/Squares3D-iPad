#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

#include "common.h"
#include "system.h"
#include "formatter.h"

// remember to update in Language constructor
enum TextType
{
    TEXT_LOADING,
    
    TEXT_MAIN_MENU,
    TEXT_START_SINGLEPLAYER,
	TEXT_RULES,
    TEXT_CONTROLS,
    TEXT_CREDITS,
    TEXT_QUIT_GAME,
    TEXT_RLY_QUIT,

    TEXT_BACK,
  
    TEXT_EASY,
    TEXT_NORMAL,
    TEXT_HARD,
    TEXT_EXTRA,

    TEXT_CREDITS_SCREEN,
	TEXT_RULES_SCREEN,
	TEXT_CONTROLS_SCREEN,

    TEXT_FPS_DISPLAY,
    
    TEXT_PLAYER_KICKS_OUT_BALL,
    TEXT_OUT_FROM_FIELD,
    TEXT_OUT_FROM_MIDDLE_LINE,
    TEXT_PLAYER_TOUCHES_TWICE,
    TEXT_PLAYER_UNALLOWED,

    TEXT_HITS_COMBO,
    TEXT_HITS,

    TEXT_SCORE_MESSAGE,
    TEXT_ESC_MESSAGE,
    TEXT_CONTINUE,
    TEXT_RESTART,

    TEXT_PAUSE,
    TEXT_FINISHED_GAME,
    TEXT_GAME_OVER,

    TEXT_TRUE,
    TEXT_FALSE,

    TEXT_LAST_ONE, // THIS MUST BE TEH LAST ONE
};

typedef map<TextType, string> TextTypeMap;
typedef map<string, TextType> StrToTextTypeMap;

class Language : public System<Language>, public NoCopy
{
public:
    Language();
    Formatter get(TextType id);
    StringVector getAvailable() const;

    void reload();

private:
    int load(const string& name);
    TextTypeMap      m_lang;
    StrToTextTypeMap m_texts;
    
};

#endif
