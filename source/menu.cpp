#include "openal_includes.h"

#include "menu.h"
#include "menu_entries.h"
#include "menu_languages.h"
#include "menu_submenu.h"

#include "game.h"
#include "world.h"
#include "font.h"
#include "video.h"
#include "texture.h"
#include "geometry.h"
#include "input.h"
#include "language.h"
#include "config.h"
#include "timer.h"
#include "profile.h"
#include "colors.h"
#include "audio.h"
#include "sound.h"
#include "sound_buffer.h"
#include "network.h"

string g_neededVersion;
static string g_submenu;

Menu::Menu(Profile* userProfile, int unlockable, int& current) :
    m_font(Font::get("Arial_32pt_bold")),
    m_fontBig(Font::get("Arial_72pt_bold")),
    m_fontSmall(Font::get("Arial_20pt_bold")),
    m_state(State::Current),
    m_sound(new Sound(true)),
    m_sound2(new Sound(true)),
    m_soundOver(NULL),
    m_soundClick(NULL),
    m_soundBackClick(NULL),
    m_soundChange(NULL),
    m_mousePrevPos(Vector(static_cast<float>(Video::instance->getResolution().first/2),
                          0.0f, 
                          static_cast<float>(Video::instance->getResolution().second/2))),
    m_languages(NULL)

{
    float resX = static_cast<float>(Video::instance->getResolution().first);
    float resY = static_cast<float>(Video::instance->getResolution().second);


    const float aspect = (1.0f - resY/resX)/2.0f;

    m_backGround.push_back(Face(UV(0.0f, aspect), Vector(), Vector::Zero));
    m_backGround.push_back(Face(UV(0.0f, 1.0f - aspect), Vector(), Vector(0.0f, resY, 0.0f)));
    m_backGround.push_back(Face(UV(1.0f, aspect), Vector(), Vector(resX, 0.0f, 0.0f)));
    m_backGround.push_back(Face(UV(1.0f, 1.0f - aspect), Vector(), Vector(resX, resY, 0.0f)));

    m_backGroundTexture = Video::instance->loadTexture("background", Texture::ClampToEdge);

    loadMenu(userProfile, unlockable, current);

    m_soundOver = Audio::instance->loadSound("menu_over");
    m_soundClick = Audio::instance->loadSound("menu_click");
    m_soundBackClick = Audio::instance->loadSound("menu_back");
    m_soundChange =Audio::instance->loadSound("menu_change");

    m_languages = new MenuLanguages(Language::instance->getAvailable());

    Network::instance->m_inMenu = true;
	Network::instance->m_needToReallyStartGame = false;

    if (g_submenu.empty() == false)
    {
        m_currentSubmenu = m_submenus[g_submenu];
        g_submenu.clear();
    }
    Input::instance->clearBuffer();
}

void Menu::loadMenu(Profile* userProfile, int unlockable, int& current)
{
    Language* language = Language::instance;

    int resX = Video::instance->getResolution().first;
    int resY = Video::instance->getResolution().second;

    Vector submenuPosition = Vector(static_cast<float>(resX) / 2,
                                    static_cast<float>(resY) / 4 * 3 / 2, 
                                    0);

    Vector titlePos = Vector(static_cast<float>(resX) / 2, 0, 0);
    float titleY = 0.0f;

    // Main Submenu

    Submenu* submenu = new Submenu(this, "main");

    submenu->setTitle(language->get(TEXT_MAIN_MENU), titlePos);

    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_START_SINGLEPLAYER), false, "startSingle"));
	submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_RULES), false, "rules"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_CONTROLS), false, "controls"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_CREDITS), false, "credits"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_QUIT_GAME), false, "quitReally"));

    m_currentSubmenu = submenu;
    submenu->center(submenuPosition);
    m_submenus[submenu->m_id] = submenu;


    // Start Game Submenu

    submenu = new Submenu(this, "startSingle");

    submenu->setTitle(language->get(TEXT_START_SINGLEPLAYER), titlePos);

    submenu->addEntry(new WorldEntry(this, language->get(TEXT_EASY), 0, current));
    submenu->addEntry(new WorldEntry(this, language->get(TEXT_NORMAL), 1, current));
    if (unlockable < 1)
    {
        submenu->m_entries.back()->disable();
    }
    submenu->addEntry(new WorldEntry(this, language->get(TEXT_HARD), 2, current));
    if (unlockable < 2)
    {
        submenu->m_entries.back()->disable();
    }
    submenu->addEntry(new WorldEntry(this, language->get(TEXT_EXTRA), 3, current));
    if (unlockable < 3)
    {
        submenu->m_entries.back()->disable();
    }
    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "main"));

    submenu->center(submenuPosition);
    m_submenus[submenu->m_id] = submenu;


    // Credits
    submenu = new Submenu(this, "credits");
    
    submenu->setTitle(language->get(TEXT_CREDITS), titlePos);

    submenu->addEntry(new LabelEntry(this, language->get(TEXT_CREDITS_SCREEN), Font::Align_Left, m_font));

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "main"));    

    // TODO: hack
    submenu->m_height += 10;

    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus[submenu->m_id] = submenu;

	// Rulez
    submenu = new Submenu(this, "rules");
    
    submenu->setTitle(language->get(TEXT_RULES), titlePos);

    submenu->addEntry(new LabelEntry(this, language->get(TEXT_RULES_SCREEN), Font::Align_Left, m_fontSmall));

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "main"));

    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus[submenu->m_id] = submenu;

	// Controls
    submenu = new Submenu(this, "controls");
    
    submenu->setTitle(language->get(TEXT_CONTROLS), titlePos);

    submenu->addEntry(new LabelEntry(this, language->get(TEXT_CONTROLS_SCREEN), Font::Align_Left, m_font));

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "main"));

    // TODO: hack
    submenu->m_height += 10;

    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus[submenu->m_id] = submenu;


    // Quit really
    submenu = new Submenu(this, "quitReally");

    submenu->setTitle(language->get(TEXT_QUIT_GAME), titlePos);

    submenu->addEntry(new LabelEntry(this, language->get(TEXT_RLY_QUIT)));
    submenu->m_entries.back()->disable();

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new QuitEntry(this, language->get(TEXT_TRUE)));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_FALSE), true, "main"));

    submenu->m_height += 20;
    submenu->center(submenuPosition);
    m_submenus[submenu->m_id] = submenu;
    

    titleY += m_font->getHeight() / 2;
    titleY = std::min(titleY, resY - static_cast<float>(m_fontBig->getHeight()));
    
    for each_(Submenus, m_submenus, iter)
    {
        if (!iter->second->m_title.empty())
        {
            iter->second->m_titlePos.y = titleY;
        }
    }
}

Menu::~Menu()
{
    Network::instance->m_inMenu = false;

    delete m_sound;
    delete m_sound2;
    Audio::instance->unloadSound(m_soundOver);
    Audio::instance->unloadSound(m_soundClick);
    Audio::instance->unloadSound(m_soundBackClick);
    Audio::instance->unloadSound(m_soundChange);

    for each_const(Submenus, m_submenus, iter)
    {
        delete iter->second;
    }

    delete m_languages;
}

State::Type Menu::progress()
{   
    if (m_state == State::Menu)
    {
        return m_state;
    }
    return (Network::instance->m_needToReallyStartGame ? State::World : State::Current);
}

void Menu::setState(State::Type state)
{
     m_state = state;
}

void Menu::setSubmenu(const string& submenuToSwitchTo)
{
    m_currentSubmenu = m_submenus.find(submenuToSwitchTo)->second;
    
    //highlight the first non disabled entry from above
    m_currentSubmenu->m_activeEntry = m_currentSubmenu->m_entries.size() - 1;
    m_currentSubmenu->activateNextEntry(true);

    Entries& entries = m_submenus[submenuToSwitchTo]->m_entries;
    for each_(Entries, entries, iter)
    {
        (*iter)->reset();
    }
}

void Menu::control()
{
    m_currentSubmenu->control();

    const Touches& touches = Input::instance->touches();
    
    if (touches.empty() == false)
    {
        float x = (float)(Video::instance->getResolution().first - touches.begin()->second.first);
        float y = (float)(Video::instance->getResolution().second - touches.begin()->second.second);

        m_languages->onMouseMove(x, y);
        
        size_t i;
        if (m_languages->onMouseClick(x, y, i))
        {
            StringVector lang = Language::instance->getAvailable();
            if (Config::instance->m_misc.language != lang[i])
            {
                g_submenu = m_currentSubmenu->m_id;
                Config::instance->m_misc.language = lang[i];
                Config::instance->save();
                Language::instance->reload();
                setState(State::Menu);
                Input::instance->waitForRelease();
            }
        }
    }
}

void Menu::render() const
{
    m_font->begin();

    Video::instance->setColor(Vector::One);

    m_backGroundTexture->bind();
    glDisableClientState(GL_NORMAL_ARRAY);
    Video::instance->renderFace(m_backGround);
    glEnableClientState(GL_NORMAL_ARRAY);

    m_languages->render();

    m_font->begin2();
    m_currentSubmenu->render();
    
    m_font->end();

    if (m_state == State::World)
    {
        m_font->begin();
        Network::instance->m_needToReallyStartGame = true;
        
        float height = m_font->getHeight();
        float width = m_font->getWidth(Language::instance->get(TEXT_LOADING));
        Vector lower = m_currentSubmenu->m_lower;
        Vector upper = m_currentSubmenu->m_lower;
        float middle = (float)(Video::instance->getResolution().first / 2);
        upper.x = lower.x = middle;
        lower -= Vector(width / 2.0f, height / 2.0f, 0.0f);
        upper += Vector(width / 2.0f, height / 2.0f, 0.0f);
        lower.y -= height * 2;
        upper.y -= height * 2;
        
        Video::instance->setColor(Vector(0.0f, 0.0f, 0.0f, 0.5f));
        Video::instance->renderRoundRect(lower, upper, height / 4.0f);
        
        glTranslatef(middle, lower.y, 0.0f);
        Video::instance->setColor(Vector::One);
        m_font->render(Language::instance->get(TEXT_LOADING), Font::Align_Center);
        m_font->end();
    }
}
