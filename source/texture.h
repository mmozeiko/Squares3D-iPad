#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

#include "common.h"

namespace File
{
    class Reader;
}

#undef None

class Texture : public NoCopy
{
public:
    enum WrapType { Repeat, ClampToEdge };

    Texture(const string& name, WrapType wrap = Repeat);
    ~Texture();

    void bind() const;

private:
    unsigned int m_handle;
};

#endif
