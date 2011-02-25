#ifndef __MENU_ENTRIES_H__
#define __MENU_ENTRIES_H__

#include "common.h"
#include "vmath.h"
#include "timer.h"
#include "font.h"

class Menu;
class Submenu;
class Entry;

typedef vector<Entry*> Entries;

class Entry : public NoCopy
{
public: 
    string      m_string;
    Vector      m_lowerLeft;
    Vector      m_upperRight;

    Entry(Menu* menu, const string& stringIn, Font::AlignType align = Font::Align_Center, const Font* font = NULL);
    virtual ~Entry() {}

    virtual void click() = 0;
    virtual string getString() const                     { return m_string;   }
    virtual string getValueID() const                    { return "";         }
    virtual string getValue() const                      { return "";         }
    virtual int getCurrentValueIdx() const               { return -1;         }
    virtual void reset()                                 {}
    virtual int  getHeight()                             { return 1;          }

    bool isEnabled() const                               { return m_enabled;  }
    void disable()                                       { m_enabled = false; }
    virtual void enable()                                { m_enabled = true;  }

    bool isMouseOver(const Vector& mousePos) const;
    virtual void calculateBounds(const Vector& position);
    void setXBound(float minX, float maxX);
    virtual void render() const;
    virtual int getMaxLeftWidth() const;
    virtual int getMaxRightWidth() const;

    const Font* m_font;

    Font::AlignType m_align;
protected:
    bool m_enabled;
    Menu* m_menu;
    bool m_forBounds;
};

class SubmenuEntry : public Entry
{
public: 
    SubmenuEntry(Menu* menu, const string& stringIn, bool back, const string&  submenuToSwitchTo, const Font* font = NULL) :
      Entry(menu, stringIn, Font::Align_Center, font), m_back(back), m_submenuToSwitchTo(submenuToSwitchTo) {}

    virtual void click(); 

protected:
    bool   m_back;
    string m_submenuToSwitchTo;
};

class WorldEntry : public Entry
{
public: 
    WorldEntry(Menu* menu, const string& stringIn, int switchTo, int& current) :
        Entry(menu, stringIn),
        m_switchTo(switchTo),
        m_current(current)
   {}
    
    void click();

private:
    int   m_switchTo;
    int&  m_current;
};

class QuitEntry : public Entry
{
public: 
    QuitEntry(Menu* menu, const string& stringIn) : Entry(menu, stringIn) {}
    
    void click();
};

class SpacerEntry : public Entry
{
public: 
    SpacerEntry(Menu* menu) : Entry(menu, "") { disable(); }
    void click()               {}
    string getString() const   { return ""; }
    bool isEnabled() const     { return false; }
    void enable()              { }
};

class LabelEntry : public Entry
{
public: 
    LabelEntry(Menu* menu, const string& label, Font::AlignType align = Font::Align_Center, const Font* font = NULL) :
      Entry(menu, label, align, font) { m_enabled = false; }

    string getString() const;
    void click() {}
};

#endif
