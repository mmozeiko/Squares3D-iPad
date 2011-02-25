#include "menu_languages.h"
#include "common.h"
#include "texture.h"
#include "video.h"
#include "config.h"

static const size_t INVALID_LANGUAGE = 1000000;
static const float DISTANCE_BETWEEN = 10.0f;
static const float FLAG_SIZE = 64.0f;

MenuLanguages::MenuLanguages(const std::vector<std::string>& languages)
    : m_active(INVALID_LANGUAGE)
{
    m_lang = languages;
    for each_const(std::vector<std::string>, languages, lang)
    {
        m_textures.push_back(new Texture("flags/" + *lang, Texture::ClampToEdge));
    }
}

MenuLanguages::~MenuLanguages()
{
    for each_(std::vector<Texture*>, m_textures, tex)
    {
        delete *tex;
    }
}

void MenuLanguages::render() const
{
    IntPair resolution = Video::instance->getResolution();
    float width = (float)resolution.first;
    float height = (float)resolution.second;

    float x = width - (DISTANCE_BETWEEN + (FLAG_SIZE + DISTANCE_BETWEEN)*m_textures.size());
    float y = height - DISTANCE_BETWEEN * 2;
    
    float w = FLAG_SIZE;
    float h = FLAG_SIZE*3/4;

    Vector lower(x-DISTANCE_BETWEEN, y-h-DISTANCE_BETWEEN, 0.0f);
    Vector upper(width-DISTANCE_BETWEEN, y+DISTANCE_BETWEEN, 0.0f);

    Video::instance->setColor(Vector(0.0f, 0.0f, 0.0f, 0.5f));
    Video::instance->renderRoundRect(lower, upper, 0.0f);
    Video::instance->setColor(Vector::One);

    glDisable(GL_BLEND);
    glDisableClientState(GL_NORMAL_ARRAY);

    for (size_t i=0; i<m_textures.size(); i++)
    {
        bool selected = m_active == i || Config::instance->m_misc.language == m_lang[i];
        if (selected)
        {
            x -= DISTANCE_BETWEEN / 2;
            y += DISTANCE_BETWEEN / 2;
            w += DISTANCE_BETWEEN;
            h += DISTANCE_BETWEEN;
        }

        m_textures[i]->bind();

        float buf[] = { 
            0, 1.0f/3, x, y-h,
            0, 1,      x, y,
            1, 1,      x+w, y,
            0, 1.0f/3, x, y-h,
            1, 1,      x+w, y,
            1, 1.0f/3, x+w, y-h
        };

        glTexCoordPointer(2, GL_FLOAT, (2+2)*sizeof(float), buf + 0);
        glVertexPointer(2, GL_FLOAT, (2+2)*sizeof(float), buf + 2);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        if (selected)
        {
            x += DISTANCE_BETWEEN / 2;
            y -= DISTANCE_BETWEEN / 2;
            w -= DISTANCE_BETWEEN;
            h -= DISTANCE_BETWEEN;
        }

        x += FLAG_SIZE + DISTANCE_BETWEEN;
    }

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnable(GL_BLEND);
}

bool MenuLanguages::onMouseClick(const float mx, const float my, size_t& idx) const
{
    IntPair resolution = Video::instance->getResolution();
    float width = (float)resolution.first;
    float height = (float)resolution.second;

    float x = width - DISTANCE_BETWEEN - (FLAG_SIZE + DISTANCE_BETWEEN)*m_textures.size();
    float y = height - DISTANCE_BETWEEN * 2;

    for (size_t i=0; i<m_textures.size(); i++)
    {
        if (mx >= x && mx < x+FLAG_SIZE && my >= y-FLAG_SIZE*3/4 && my <= y)
        {
            idx = i;
            return true;
        }

        x += FLAG_SIZE + DISTANCE_BETWEEN;
    }
    return false;
}

void MenuLanguages::onMouseMove(const float mx, const float my)
{
    m_active = INVALID_LANGUAGE;

    IntPair resolution = Video::instance->getResolution();
    float width = (float)resolution.first;
    float height = (float)resolution.second;

    float x = width - DISTANCE_BETWEEN - (FLAG_SIZE + DISTANCE_BETWEEN)*m_textures.size();
    float y = height - DISTANCE_BETWEEN * 2;

    for (size_t i=0; i<m_textures.size(); i++)
    {
        if (mx >= x && mx < x+FLAG_SIZE && my >= y-FLAG_SIZE*3/4 && my <= y)
        {
            m_active = i;
            break;
        }

        x += FLAG_SIZE + DISTANCE_BETWEEN;
    }
}
