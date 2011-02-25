#include "language.h"
#include "xml.h"
#include "file.h"
#include "config.h"

template <class Language> Language* System<Language>::instance = NULL;

Language::Language()
{
    clog << "Initializing language texts... ";

#define REGISTER_TEXT_TYPE(type) m_texts.insert(make_pair(STR(type), TEXT_##type))

    REGISTER_TEXT_TYPE(LOADING);

    REGISTER_TEXT_TYPE(MAIN_MENU);

    REGISTER_TEXT_TYPE(START_SINGLEPLAYER);
	REGISTER_TEXT_TYPE(RULES);
    REGISTER_TEXT_TYPE(CONTROLS);
    REGISTER_TEXT_TYPE(CREDITS);
    REGISTER_TEXT_TYPE(QUIT_GAME);

    REGISTER_TEXT_TYPE(BACK);
  
    REGISTER_TEXT_TYPE(EASY);
    REGISTER_TEXT_TYPE(NORMAL);
    REGISTER_TEXT_TYPE(HARD);
    REGISTER_TEXT_TYPE(EXTRA);

    REGISTER_TEXT_TYPE(CREDITS_SCREEN);
	REGISTER_TEXT_TYPE(RULES_SCREEN);
	REGISTER_TEXT_TYPE(CONTROLS_SCREEN);
    REGISTER_TEXT_TYPE(RLY_QUIT);

    REGISTER_TEXT_TYPE(FPS_DISPLAY);
    
    REGISTER_TEXT_TYPE(PLAYER_KICKS_OUT_BALL);
    REGISTER_TEXT_TYPE(OUT_FROM_FIELD);
    REGISTER_TEXT_TYPE(OUT_FROM_MIDDLE_LINE);
    REGISTER_TEXT_TYPE(PLAYER_TOUCHES_TWICE);
    REGISTER_TEXT_TYPE(PLAYER_UNALLOWED);

    REGISTER_TEXT_TYPE(HITS_COMBO);
    REGISTER_TEXT_TYPE(HITS);
    REGISTER_TEXT_TYPE(SCORE_MESSAGE);
    REGISTER_TEXT_TYPE(ESC_MESSAGE);
    REGISTER_TEXT_TYPE(CONTINUE);
    REGISTER_TEXT_TYPE(RESTART);

    REGISTER_TEXT_TYPE(PAUSE),
    REGISTER_TEXT_TYPE(FINISHED_GAME);
    REGISTER_TEXT_TYPE(GAME_OVER);

    REGISTER_TEXT_TYPE(TRUE);
    REGISTER_TEXT_TYPE(FALSE);
    

#undef REGISTER_TEXT_TYPE
    
    assert(m_texts.size() == static_cast<int>(TEXT_LAST_ONE));
    
    // check if language is available
    StringVector available = getAvailable();
    if (!foundIn(available, Config::instance->m_misc.language))
    {
        Config::instance->m_misc.language = "en";
        return;
    }

    // load selected language
    reload();
}

StringVector Language::getAvailable() const
{
    StringVector result;

    XMLnode xml;
    File::Reader in("/data/language/list.xml");
    if (!in.is_open())
    {
        Exception("Level file 'data/language/list.xml' not found");  
    }
    xml.load(in);

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "language")
        {
            result.push_back(node.value);
        }
        else
        {
            Exception("Invalid language list, unknown node - " + node.name);
        }
    }

    return result;
}

Formatter Language::get(TextType id)
{
    return Formatter(m_lang.find(id)->second);
}

int Language::load(const string& name)
{
    string filename = "/data/language/" + name + ".xml";

    int count = 0;

    XMLnode xml;
    File::Reader in(filename);
    if (!in.is_open())
    {
        Exception("Language file '" + filename + "' not found");  
    }
    xml.load(in);

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name != "item")
        {
            Exception("Invalid language file '" + filename + "'");
        }

        StrToTextTypeMap::const_iterator txtIter = m_texts.find(node.getAttribute("name"));
        if (txtIter == m_texts.end())
        {
            Exception("Invalid language file, unexpected '" + node.getAttribute("name") + "' item");
        }

        TextType type = txtIter->second;
        string value = (node.hasAttribute("value") ? node.getAttribute("value") : node.value);
        m_lang[type] = value;

        count++;
    }
    
    return count;
}

void Language::reload()
{
    m_lang.clear();

    // load default texts
    int count = load("en");
    if (count != static_cast<int>(m_texts.size()))
    {
        Exception("Default language file (en.xml) has invalid count of messages");  
    }

    clog << "loading " << Config::instance->m_misc.language << " language." << endl;

    int selCount = load(Config::instance->m_misc.language);
    if (selCount != count)
    {
        clog << "Warning: expected " << count << " phrases, but found only " << selCount << "!" << endl;
    }
}
