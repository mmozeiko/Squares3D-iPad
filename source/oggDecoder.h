#ifndef __OGG_DECODER_H__
#define __OGG_DECODER_H__

#include <ivorbisfile.h>
#include "common.h"
#include "file.h"

class OggDecoder : public NoCopy
{
public:
    OggDecoder(const string& filename);
    ~OggDecoder();

    void reset();

    size_t m_pos;
    File::Reader m_reader;

protected:
    unsigned int m_frequency;
    unsigned int m_channels;
    unsigned int m_format;

    size_t totalSize();
    size_t decode(char* buffer, const size_t bufferSize);

private:
    OggVorbis_File m_oggFile;
};

#endif
