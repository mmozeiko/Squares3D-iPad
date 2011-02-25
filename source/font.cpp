#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

#include "font.h"
#include "file.h"
#include "vmath.h"
#include "video.h"

typedef map<string, const Font*> FontMap;

static FontMap fonts;

const Font* Font::get(const string& name)
{
    FontMap::const_iterator iter = fonts.find(name);
    if (iter != fonts.end())
    {
        return iter->second;
    }
    return fonts.insert(make_pair(name, new Font(name))).first->second;
}

void Font::unload()
{
    for each_(FontMap, fonts, iter)
    {
        delete iter->second;
    }
    fonts.clear();
}

Font::Font(const string& filename) : m_texture(0)
{
    clog << "Loading font '" << filename << "'..." << endl;

    // loading font description
    File::Reader in("/data/font/" + filename + ".font");
    if (!in.is_open())
    {
        Exception("Font file not found");  
    }
    
    memcpy(&head, in.pointer(), sizeof(head));
    
    if (string(head.fnt+0, head.fnt+4) != "FONT")
    {
        Exception("Invalid font file");
    }
#ifdef __BIG_ENDIAN__
    head.size = ((head.size & 0xFF) << 8) + (head.size >> 8);
    head.texW = ((head.texW & 0xFF) << 8) + (head.texW >> 8);
    head.texH = ((head.texH & 0xFF) << 8) + (head.texH >> 8);
    head.height = ((head.height & 0xFF) << 8) + (head.height >> 8);
    head.count = ((head.count & 0xFF) << 8) + (head.count >> 8);
    head.maxid = ((head.maxid & 0xFF) << 8) + (head.maxid >> 8);
#endif
        
    m_count = head.maxid;
    m_height = head.height;

    vector<Char> chars(head.count);
    memcpy(&chars[0], in.pointer() + sizeof(head), sizeof(Char)*head.count);

#ifdef __BIG_ENDIAN__
    for (int i=0; i<head.count; i++)
    {
        chars[i].id = ((chars[i].id & 0xFF) << 8) + (chars[i].id >> 8);
        chars[i].x = ((chars[i].x & 0xFF) << 8) + (chars[i].x >> 8);
        chars[i].y = ((chars[i].y & 0xFF) << 8) + (chars[i].y >> 8);
    }
#endif

    // loading texture
    int width;
    int height;
    int comp;
    unsigned char* image = loadImg("/data/font/" + filename + "_00.img", width, height, comp);

    glGenTextures(1, (GLuint*)&m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    if (comp == 1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, image);
    }
    else
    {
        Exception("Invalid font texture '" + filename + "_00.tga' format");
    }

    delete [] image;
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    
    m_chars.resize(m_count);
    m_widths.resize(m_count, -1);

    int idx = 0;
    int pos = 0;
    while (idx < m_count)
    {
        while (idx != chars[pos].id)
        {
            m_widths[idx] = head.size;
            const Char& c = chars[pos];
            m_chars[idx] = c;
            m_chars[idx].id = 0;

            idx++;
        }
        
        const Char& c = chars[pos];
        m_widths[idx] = c.xadvance;
        m_chars[idx] = c;

        idx++;
        pos++;
    }
}

Font::~Font()
{
    glDeleteTextures(1, (GLuint*)&m_texture);
}

bool Font::hasChar(int ch) const
{
    return ch>=0 && ch<=static_cast<int>(m_widths.size()) && m_widths[ch]>0;
}

void Font::begin(bool shadowed, const Vector& shadow, float shadowWidth) const
{
    int viewport_width;
    int viewport_height;
    Video::instance->getViewport(viewport_width, viewport_height);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrthof(0, (float)viewport_height, 0, (float)viewport_width, -1.0f, 1.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.0f, -768.0f, 0.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, m_texture);

    m_shadow = shadow;
    m_shadowed = shadowed;
    m_shadowWidth = shadowWidth;
}

void Font::begin2() const
{
    glBindTexture(GL_TEXTURE_2D, m_texture);
}

static const signed char extra_utf8_bytes[256] = {
    /* 0xxxxxxx */
    0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0, 0, 0,

    /* 10wwwwww */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

    /* 110yyyyy */
    1, 1, 1, 1, 1, 1, 1, 1,     1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,     1, 1, 1, 1, 1, 1, 1, 1,

    /* 1110zzzz */
    2, 2, 2, 2, 2, 2, 2, 2,     2, 2, 2, 2, 2, 2, 2, 2,

    /* 11110yyy */
    3, 3, 3, 3, 3, 3, 3, 3,     -1, -1, -1, -1, -1, -1, -1, -1,
};

static const int extra_utf8_bits[4] = {
    0,
    12416,      /* (0xC0 << 6) + (0x80) */
    925824,     /* (0xE0 << 12) + (0x80 << 6) + (0x80) */
    63447168,   /* (0xF0 << 18) + (0x80 << 12) + (0x80 << 6) + 0x80 */
};

inline unsigned int nextUTF8char(const char* & x)
{
    unsigned int c = static_cast<unsigned char>(*x++);
    int extra = extra_utf8_bytes[c];
    switch (extra)
    {
        case -1: c = 0; break;
        case 3: c = (c << 6) + static_cast<unsigned char>(*x++);
        case 2: c = (c << 6) + static_cast<unsigned char>(*x++);
        case 1: c = (c << 6) + static_cast<unsigned char>(*x++);
                c -= extra_utf8_bits[extra];
    }
    return c;
}


void Font::renderPlain(const string& text) const
{
    const char* x = text.c_str();

    std::vector<float> buf;
    buf.reserve(text.size() * (2*3*(2+2)));

    float posX = 0;

    while (unsigned int c = nextUTF8char(x))
    {
        if (c < static_cast<unsigned int>(m_widths.size()) && m_widths[c] != -1)
        {
            const Char& ch = m_chars[c];
            
            float u1 = static_cast<float>(ch.x) / static_cast<float>(head.texW);
            float v1 = static_cast<float>(ch.y) / head.texH;
            float u2 = static_cast<float>(ch.x + ch.width) / static_cast<float>(head.texW);
            float v2 = static_cast<float>(ch.y + ch.height) / head.texH;

            float x1 = posX + static_cast<float>(ch.xoffset);
            float y1 = static_cast<float>(m_height - ch.yoffset);
            float x2 = posX + static_cast<float>(ch.xoffset + ch.width);
            float y2 = static_cast<float>(m_height - (ch.yoffset + ch.height));

            buf.push_back(u1);
            buf.push_back(v1);
            buf.push_back(x1);
            buf.push_back(y1);

            buf.push_back(u1);
            buf.push_back(v2);
            buf.push_back(x1);
            buf.push_back(y2);

            buf.push_back(u2);
            buf.push_back(v2);
            buf.push_back(x2);
            buf.push_back(y2);

            buf.push_back(u1);
            buf.push_back(v1);
            buf.push_back(x1);
            buf.push_back(y1);

            buf.push_back(u2);
            buf.push_back(v2);
            buf.push_back(x2);
            buf.push_back(y2);

            buf.push_back(u2);
            buf.push_back(v1);
            buf.push_back(x2);
            buf.push_back(y1);

            posX += ch.xadvance;
        }
    }

    if (buf.empty() == false)
    {
        glDisableClientState(GL_NORMAL_ARRAY);

        glTexCoordPointer(2, GL_FLOAT, 4*sizeof(float), &buf[0]);
        glVertexPointer(2, GL_FLOAT, 4*sizeof(float), &buf[2]);
        glDrawArrays(GL_TRIANGLES, 0, buf.size() / 4);

        glEnableClientState(GL_NORMAL_ARRAY);
    }
}

void Font::render(const string& text, AlignType align) const
{
    Vector shadowColor(m_shadow);
    Vector forAlpha = Video::instance->getColor();
    shadowColor.w = forAlpha.w;
    //

    size_t pos, begin = 0;
    string line;
    float linePos = 0.0f;
    int width;
    while (begin < text.size())
    {
        pos = text.find('\n', begin);
        if (pos == string::npos)
        {
            pos = text.size();
        }

        line = text.substr(begin, pos - begin);

        begin = pos + 1;

        glPushMatrix();

        switch (align)
        {
        case Align_Left:
            break;
        case Align_Center:
            width = getWidth(line);
            glTranslatef(static_cast<float>(-width/2), 0.0f, 0.0f);
            break;
        case Align_Right:
            width = getWidth(line);
            glTranslatef(static_cast<float>(-width), 0.0f, 0.0f);
            break;
        default:
            assert(false);
        };
        glTranslatef(0.0f, linePos, 0.0f);       

        if (m_shadowed)
        {
            glPushMatrix();

            Vector color = Video::instance->getColor();
            Video::instance->setColor(shadowColor);
            glTranslatef(m_shadowWidth, - m_shadowWidth, 0.0f);
            renderPlain(line);
            glPopMatrix();
            Video::instance->setColor(color);
        }

        renderPlain(line);
        glPopMatrix();
        linePos -= m_height;
    }

}

void Font::end() const
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

int Font::getWidth(const string& text) const
{
    int maxWidth = 0;
    int width = 0;
    const char* x = text.c_str();
    while (unsigned int c = nextUTF8char(x))
    {
        if (c < static_cast<unsigned int>(m_widths.size()) && m_widths[c] != -1)
        {
            if (c == '\n')
            {
                if (width > maxWidth)
                {
                    maxWidth = width;
                }
                width = 0;
            }
            width += m_widths[c];
        }
    }
    if (width > maxWidth)
    {
        maxWidth = width;
    }
    return maxWidth;
}

bool isEndline(const char& t)
{
    return t == '\n';
}

int Font::getHeight() const
{
    return m_height;
}

int Font::getHeight(const string& text) const
{
    return static_cast<int>(m_height * (1 + std::count_if(text.begin(), text.end(), isEndline)));
}
