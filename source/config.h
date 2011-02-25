#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "common.h"
#include "system.h"

struct MiscConfig
{
    string language;
};

class Config : public System<Config>, public NoCopy
{
public:
    Config();
    ~Config();
    
    void save();

    MiscConfig  m_misc;

    static const MiscConfig defaultMisc;

private:
    static const string CONFIG_FILE;
};

#endif
