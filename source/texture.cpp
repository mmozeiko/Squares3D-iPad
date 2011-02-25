#include "texture.h"
#include "file.h"
#include "vmath.h"
#include "config.h"
#include "pvr.h"

Texture::Texture(const string& name, WrapType wrap)
{
    glGenTextures(1, (GLuint*)&m_handle);
    glBindTexture(GL_TEXTURE_2D, m_handle);
    
    File::Reader file("/data/textures/" + name + ".pvr");
    if (!file.is_open())
    {
        Exception("Texture '" + name + "' doesn't exist");
    }

    PVR_Texture_Header* header = (PVR_Texture_Header*)file.pointer();
    if (header->dwHeaderSize != sizeof(*header) || header->dwPVR != PVRTEX_IDENTIFIER)
    {
        Exception("Texture '" + name + "' has invalid format");
    }
    
    unsigned char* image = file.pointer() + sizeof(*header);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    if ((header->dwpfFlags & PVRTEX_PIXELTYPE) == OGL_PVRTC4)
    {
        GLenum format = header->dwAlphaBitMask ? GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG : GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;       
        
        unsigned char* ptr = image;
        int level = 0;
        int width = header->dwWidth;
        int height = header->dwHeight;
        while (level <= header->dwMipMapCount)
        {
            GLint size = (std::max<int>(width, PVRTC4_MIN_TEXWIDTH) * std::max<int>(height, PVRTC4_MIN_TEXHEIGHT) * 4 + 7) / 8;
            
            glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, ptr);
            
            level++;
            ptr += size;
            width = std::max(width >> 1, 1);
            height = std::max(height >> 1, 1);
        }
    }

    if (header->dwpfFlags & PVRTEX_MIPMAP)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
           
    switch (wrap)
    {
        case Repeat:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            break;
        case ClampToEdge:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            break;
    }
}

Texture::~Texture()
{
    glDeleteTextures(1, (GLuint*)&m_handle);
}

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, m_handle);
}
