#include "config.h"
#include "file.h"
#include "xml.h"
#include "version.h"

template <class Config> Config* System<Config>::instance = NULL;

const string Config::CONFIG_FILE = "/config.xml";

const MiscConfig Config::defaultMisc = { "en" };

Config::Config() : m_misc(defaultMisc)
{
    clog << "Reading configuration." << endl;

    XMLnode xml;
    File::Reader in(CONFIG_FILE, true);
    if (in.is_open())
    {
        xml.load(in);
    }

    string version = xml.getAttribute("version", string());
    
    if (version != g_version)
    {
        // config file version mismatch
        //return;
    }

    // TODO: maybe rewrite with map<string, variable&>

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;

        if (node.name == "misc")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "language")
                {
                    m_misc.language = node.value;
                }
                else
                {
                    string line = cast<string>(node.line);
                    Exception("Invalid configuration file, unknown misc parameter '" + node.name + "' at line " + line);
                }
            }
        }
        else
        {
            string line = cast<string>(node.line);
            Exception("Invalid configuration file, unknown section '" + node.name + "' at line " + line);
        }
    }
}

Config::~Config()
{
    save();
}

void Config::save()
{
    clog << "Saving configuration." << endl;

    XMLnode xml("config");
    xml.setAttribute("version", g_version);

    xml.childs.push_back(XMLnode("misc"));
    xml.childs.back().childs.push_back(XMLnode("language", m_misc.language));

    File::Writer out(CONFIG_FILE);
    if (!out.is_open())
    {
        Exception("Failed to open " + CONFIG_FILE + " for writing");
    }
    xml.save(out);
}
