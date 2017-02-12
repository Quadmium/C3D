#include <SDL.h>
#include <stdbool.h>

extern int screenWidth, screenHeight;
extern SDL_Renderer *renderer;
extern double deltaTime;

Uint32 RGBA2INT(int R, int G, int B, int A);