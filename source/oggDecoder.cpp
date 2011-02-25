#include "openal_includes.h"

#include "oggDecoder.h"

static size_t ov_read_func(void* ptr, size_t size, size_t nmemb, void* datasource)
{
    OggDecoder* ogg = static_cast<OggDecoder*>(datasource);
    if (ogg->m_pos == ogg->m_reader.size())
    {
        return 0;
    }
    
    size_t toRead = std::min(size * nmemb, ogg->m_reader.size() - ogg->m_pos);
    memcpy(ptr, ogg->m_reader.pointer() + ogg->m_pos, toRead);
    ogg->m_pos += toRead;
    
    return toRead/size;
}

static int ov_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
    OggDecoder* ogg = static_cast<OggDecoder*>(datasource);
    if (whence == SEEK_SET)
    {
        ogg->m_pos = offset;
    }
    else if (whence == SEEK_CUR)
    {
        ogg->m_pos += offset;
    }
    else if (whence == SEEK_END)
    {
        ogg->m_pos = ogg->m_reader.size() + offset;
    }
    return 0;
}

static int ov_close_func(void *datasource)
{
    return 0;
}

static long ov_tell_func(void *datasource)
{
    OggDecoder* ogg = static_cast<OggDecoder*>(datasource);
    return static_cast<long>(ogg->m_pos);
}

static const ov_callbacks oggCallbacks = { ov_read_func, ov_seek_func, ov_close_func, ov_tell_func };


OggDecoder::OggDecoder(const string& filename) : m_reader(File::Reader(filename)), m_pos(0)
{
    if (!m_reader.is_open())
    {
        Exception("File '" + filename + "' not found");
    }

    int result = ov_open_callbacks(static_cast<void*>(this), &m_oggFile, NULL, 0, oggCallbacks);
    if (result != 0)
    {
        switch (result)
        {
          case OV_EREAD:      Exception("A read from media returned an error");
          case OV_ENOTVORBIS: Exception("Bitstream is not Vorbis data");
          case OV_EVERSION:   Exception("Vorbis version mismatch");
          case OV_EBADHEADER: Exception("Invalid Vorbis bitstream header");
          case OV_EFAULT:     Exception("Internal logic fault");
          default:            Exception("Unknown ogg open error");
        }
    }

    vorbis_info* info = ov_info(&m_oggFile, -1);
    if (info == NULL)
    {
        Exception("Error getting info from ogg file");
    }

    m_frequency = info->rate;
    m_channels = info->channels;

    if (m_channels == 1)
    {
        m_format = AL_FORMAT_MONO16;
    }
    else if (m_channels == 2)
    {
        m_format = AL_FORMAT_STEREO16;
    }
    else
    {
        Exception("Invalid channel count: " + cast<std::string>(m_channels));
    }
}

OggDecoder::~OggDecoder()
{
    ov_clear(&m_oggFile);
}

size_t OggDecoder::totalSize()
{
    int total = static_cast<int>(ov_pcm_total(&m_oggFile, -1));
    if (total == OV_EINVAL)
    {
        Exception("Invalid bitstream");
    }
    return 2*m_channels*static_cast<size_t>(total);
}

size_t OggDecoder::decode(char* buffer, const size_t bufferSize)
{
    int current_section;
    size_t written = 0;

    while (written < bufferSize)
    {
        int decodeSize = ov_read(&m_oggFile, buffer + written, static_cast<int>(bufferSize - written), &current_section);
        if (decodeSize <= 0)
        {
            break;
        }
        
        written += decodeSize;
    }
    return written;
}

void OggDecoder::reset()
{
    ov_raw_seek(&m_oggFile, 0);
}
