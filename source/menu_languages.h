#ifndef __MENU_LANGUAGES_H__
#define __MENU_LANGUAGES_H__

#include "common.h"

class Texture;
class MenuLanguages
{
public:
    MenuLanguages(const std::vector<std::string>& languages);
    ~MenuLanguages();

    void onMouseMove(const float mx, const float my);
    bool onMouseClick(const float mx, const float my, size_t& idx) const;
    void render() const;

private:
    std::vector<Texture*> m_textures;
    std::vector<std::string> m_lang;
    size_t m_active;
};

#endif
