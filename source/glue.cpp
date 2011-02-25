#include "common.h"
#include "glue.h"
#include "game.h"
#include "video.h"
#include "random.h"
#include "version.h"
#include "utilities.h"

int quit_requested = 0;

static Game* game = NULL;

static std::string file_readPath;
static std::string file_writePath;

void file_set_root(const char* readPath, const char* writePath)
{
    file_readPath = readPath;
    file_writePath = writePath;
}

const char* file_get_readPath()
{
    return file_readPath.c_str();
}

const char* file_get_writePath()
{
    return file_writePath.c_str();
}

int game_begin()
{
    clog << "*** Squares 3D version: " << g_version << " ***" << endl;
    clog << "Started: " << getDateTime() << endl;
    
    Randoms::init();

    game = new Game();
    return 1;
}

void game_resize(int width, int height)
{
    if (game != NULL)
    {
        game->m_video->resize(width, height);
    }
}

void game_update(float delta)
{
    game->update(delta);
}

void game_render(float delta)
{
    game->render(delta);
}

void game_end()
{
    delete game;

    clog << "Finished: " << getDateTime() << endl;
}

