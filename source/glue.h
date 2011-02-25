#pragma once

#if __cplusplus
extern "C"
{
#endif

extern int quit_requested;

void file_set_root(const char* readPath, const char* writePath);
const char* file_get_readPath();
const char* file_get_writePath();
    
int game_begin();
void game_resize(int width, int height);
void game_update(float delta);
void game_render(float delta);
void game_end();

void touch_begin(void* touch, int x, int y);
void touch_move(void* touch, int x, int y);
void touch_end(void* touch);

    
#if __cplusplus
}
#endif
