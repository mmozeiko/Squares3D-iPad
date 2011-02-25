#ifndef __MENU_SUBMENU_H__
#define __MENU_SUBMENU_H__

#include "common.h"
#include "vmath.h"

class Entry;
class Font;
class Menu;

typedef vector<Entry*> Entries;

class Submenu : public NoCopy
{
public:
    Entries m_entries;
    size_t  m_activeEntry;
    float   m_height;
    Vector  m_centerPos;
    std::string m_id;

    Submenu(Menu* menu, const std::string& id) :
        m_id(id), m_activeEntry(0), m_height(0), m_title(""),
        m_menu(menu) {}
    ~Submenu();

    void addEntry(Entry* entry);
    void center(const Vector& centerPos);
    void render() const;
    void control();
    void setTitle(const string& title, const Vector& position);
    void activateNextEntry(bool moveDown);

    Vector  m_upper;
    Vector  m_lower;

    string  m_title;
    Vector  m_titlePos;

private:
    Vector      m_previousMousePos;

    Menu*       m_menu;
};

#endif
