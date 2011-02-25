#include "openal_includes.h"

#include "audio.h"
#include "config.h"
#include "music.h"
#include "sound_buffer.h"
#include "sound.h"

template <class Audio> Audio* System<Audio>::instance = NULL;

static ALCdevice* device;
static ALCcontext* context;

static void audio_setup()
{
    clog << "Initializing audio." << endl;

    device = alcOpenDevice(NULL);
    if (device==NULL)
    {
        Exception(alcGetString(NULL, alcGetError(NULL)));
    }
    
    int attr[] = { ALC_FREQUENCY, 22050, ALC_INVALID };

    context = alcCreateContext(device, attr);
    if (context==NULL)
    {
        string error = string(alcGetString(device, alcGetError(device)));
        alcCloseDevice(device);
        Exception(error);
    }
    alcMakeContextCurrent(context);

    int major=0, minor=0;
    alcGetIntegerv(device, ALC_MAJOR_VERSION, 1, &major);
    alcGetIntegerv(device, ALC_MINOR_VERSION, 1, &minor);

    clog << " * Version    : " << major << '.' << minor << endl
         << " * Vendor     : " << alGetString(AL_VENDOR) << endl
         << " * Renderer   : " << alGetString(AL_RENDERER) << endl
         << " * Device     : " << alcGetString(device, ALC_DEVICE_SPECIFIER) << endl
         << " * Extensions : " << alcGetString(device, ALC_EXTENSIONS) << endl;
}

static void audio_finish()
{
    clog << "Closing audio." << endl;
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

Audio::Audio() : m_device(NULL), m_context(NULL) 
{
    audio_setup();
    
    m_device = device;
    m_context = context;
}

Audio::~Audio()
{
    audio_finish();

    m_device = NULL;
    m_context = NULL;
}

Music* Audio::loadMusic(const string& filename)
{
    return *m_music.insert(new Music(filename)).first;
}

void Audio::unloadMusic(Music* music)
{
    m_music.erase(music);
    delete music;
}

SoundBuffer* Audio::loadSound(const string& filename)
{
    SoundBufferMap::iterator iter = m_soundBuf.find(filename);
    if (iter != m_soundBuf.end())
    {
        return iter->second;
    }

    return m_soundBuf.insert(make_pair(filename, new SoundBuffer(filename))).first->second;
}

void Audio::unloadSound(SoundBuffer* soundBuf)
{
    for each_(SoundBufferMap, m_soundBuf, iter)
    {
        if (iter->second == soundBuf)
        {
            delete iter->second;
            m_soundBuf.erase(iter);
            break;
        }
    }
}

void Audio::update()
{
    for each_(MusicSet, m_music, iter)
    {
        (*iter)->update();
    }
}
