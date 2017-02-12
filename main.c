/* 
 * File:   main.c
 * Author: quadmium
 *
 * Created on June 26, 2016, 12:29 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <time.h>
#include <math.h>
#include "main.h"
#include "Structures.h"
#include "Camera.h"
#include "include/SDL2_gfxPrimitives.h"

int screenWidth = 2048;
int screenHeight = 1152;
int inFullScreen = 0;
SDL_Rect windowTarget;
SDL_Renderer *renderer;
SDL_Window *window;
Uint32 *pixels;
double deltaTime;

static long getNanoTime();
static void handleEvents(SDL_Event *event, bool *leftMouseButtonDown, bool *rightMouseButtonDown, int *bracketDown, bool *quit,
                  int *mouseX, int *mouseY, int *mouseDX, int *mouseDY);
static void drawLine();
static void setPixel(int x, int y, int R, int G, int B, int A);

const Transform Transform_default = {0, 0, 0};
const GameObject GameObject_default = {.transform = {0, 0, 0} };

int main(int argc, char ** argv)
{
    long lastTime = getNanoTime();
    
    windowTarget.x = 0;
    windowTarget.y = 0;
    windowTarget.w = screenWidth;
    windowTarget.h = screenHeight;
    
    bool leftMouseButtonDown = false;
    bool rightMouseButtonDown = false;
    int bracketDown = 0;
    bool quit = false;
    SDL_Event event;

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init(); 
        
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear"); 

    window = SDL_CreateWindow("Heat Simulation",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowTarget.w, windowTarget.h, 0);

    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture * texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, screenWidth, screenHeight);
    
    TTF_Font *dispFont = TTF_OpenFont("default.ttf",20);
    
    pixels = malloc(sizeof(Uint32) * screenWidth * screenHeight);
    memset(pixels, 0, sizeof(Uint32) * screenWidth * screenHeight);
    
    /*for(int j=0; j<height; j++)
        for(int i=0; i<width; i++)
            pixels[(height-1-j)*width+i]=RGBA2INT(255,j*255/600,0,255);*/
    
    int mouseDX=0, mouseDY=0;
    int mouseX, mouseY;
    
    long start = getNanoTime();
    // http://aaroncox.net/tutorials/2dtutorials/sdlshapes.html
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    double points[3*8] = {
            1,  1, -1, -1,  1,  1, -1, -1,
            1, -1,  1, -1,  1, -1,  1, -1,
            1,  1,  1,  1, -1, -1, -1, -1
    };
    
    World world;
    int numObjects = 200;
    int mod = 10;
    GameObject gameObjects[numObjects];
    for(int i=0; i<numObjects; i++)
    {
        gameObjects[i] = GameObject_default;
        gameObjects[i].mesh.points = points;
        gameObjects[i].mesh.numPoints = sizeof(points) / sizeof(double) / 3;
        gameObjects[i].transform.x = 3*(i % mod);
        gameObjects[i].transform.z = 3*(i / mod);
    }
    /*gameObjects[0] = GameObject_default;
    gameObjects[0].mesh.points = points;
    gameObjects[0].mesh.numPoints = sizeof(points) / sizeof(double) / 3;
    gameObjects[1] = GameObject_default;
    gameObjects[1].mesh.points = points;
    gameObjects[1].mesh.numPoints = sizeof(points) / sizeof(double) / 3;
    gameObjects[1].transform.x = 3;*/
    world.gameObjects = gameObjects;
    world.numGameObjects = numObjects;
    
    camera_init();
    double x = 0, y = 0, z = 0, rotY = 0, rotX = 0, slowRate = 0.95, moveSpeed = 2.5, rotSpeed = 0.001, fovSpeed = 0.5, fov = 0;

    while (!quit)
    {
        long curTime = getNanoTime();
        deltaTime = (curTime - lastTime) / 1000000000.0;
        lastTime = curTime;
        
        rotY = (mouseDX) * -rotSpeed;
        rotX = (mouseDY) * -rotSpeed;
        
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        
        handleEvents(&event, &leftMouseButtonDown, &rightMouseButtonDown, &bracketDown, &quit,
                  &mouseX, &mouseY, &mouseDX, &mouseDY);
        
        //x *= slowRate;
        //y *= slowRate;
        //z *= slowRate;
        //fov *= slowRate;
        x = 0;
        y = 0;
        z = 0;
        fov = 0;
        
        if(keystate[SDL_SCANCODE_SPACE]) y=moveSpeed * deltaTime;
        if(keystate[SDL_SCANCODE_LSHIFT]) y=-moveSpeed * deltaTime;
        if(keystate[SDL_SCANCODE_D]) x=moveSpeed * deltaTime;
        if(keystate[SDL_SCANCODE_A]) x=-moveSpeed * deltaTime;
        if(keystate[SDL_SCANCODE_W]) z=moveSpeed * deltaTime;
        if(keystate[SDL_SCANCODE_S]) z=-moveSpeed * deltaTime;
        if(keystate[SDL_SCANCODE_F]) fov=fovSpeed * deltaTime;
        if(keystate[SDL_SCANCODE_R]) fov=-fovSpeed * deltaTime;
        
        camProps.rotY += rotY;
        camProps.rotX += rotX;
        camProps.fov += fov;
        
        camProps.transform.x += x * cos(camProps.rotY) - z * sin(camProps.rotY);
        camProps.transform.y += y != 0 ? y : z * sin(camProps.rotX);
        camProps.transform.z += z * cos(camProps.rotY) + x * sin(camProps.rotY);
        
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        camera_draw(&world);
        if(dispFont != NULL)
        {
            SDL_Color Black = {0, 0, 0, 255};
            char dispTxt[100];
            snprintf(dispTxt, 100, "FPS: %.2f", 1/deltaTime);
            SDL_Surface* surfaceMessage = TTF_RenderText_Blended(dispFont, dispTxt, Black);
            SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
            int x=4;
            int y=0;
            SDL_Rect Message_rect = {x, y, surfaceMessage->w, surfaceMessage->h};
            SDL_RenderCopy(renderer, Message, NULL, &Message_rect); 
            SDL_DestroyTexture(Message);
            SDL_FreeSurface(surfaceMessage);
        }
        //SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(Uint32));
        //SDL_RenderCopy(renderer, texture, NULL, &windowTarget);
        //lineRGBA(renderer, width/2,height/2,(int)(width/2 + width/2 *cos(angle)),(int)(height/2 + height/2 * sin(angle)), 0, 0, 0, 255);
        //SDL_RenderDrawLine(renderer, width/2,height/2,(int)(width/2 + width/2 *cos(angle)),(int)(height/2 + height/2 * sin(angle)));
        SDL_RenderPresent(renderer);
    }

    printf("%f\n",(getNanoTime() - start) / 1000000000.0);
    free(pixels);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

/*if(dispFont != NULL)
{
    int centerX = mouseX * size / windowTarget.w;
    int centerY = mouseY * size / windowTarget.h;
    SDL_Color White = {255, 255, 255, 255};
    char dispTxt[100];
    snprintf(dispTxt, 100, "(%d,%d) = %.2f",centerX,centerY,data[newi][centerY*size+centerX]);
    SDL_Surface* surfaceMessage = TTF_RenderText_Blended(dispFont, dispTxt, White);
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    int x=4;
    int y=0;
    SDL_Rect Message_rect = {x, y, surfaceMessage->w, surfaceMessage->h};
    SDL_RenderCopy(renderer, Message, NULL, &Message_rect); 
    SDL_DestroyTexture(Message);
    SDL_FreeSurface(surfaceMessage);
}*/

static void handleEvents(SDL_Event *event, bool *leftMouseButtonDown, bool *rightMouseButtonDown, int *bracketDown, bool *quit,
                  int *mouseX, int *mouseY, int *mouseDX, int *mouseDY)
{
    // A break is missing in the code for SDL leading to mouse set to 0 if left window
    // Documented: https://github.com/openfl/lime/commit/068919bc4c16dcda274a6315d164dd6897fbf8c3?diff=unified
    // This works around it by capturing the faulty event and skipping mouse X,Y if it last occurred
    *mouseDX = 0;
    *mouseDY = 0;
    
    while(SDL_PollEvent(event))
    {
        switch (event->type)
        {
            case SDL_MOUSEBUTTONUP:
                if (event->button.button == SDL_BUTTON_LEFT)
                    *leftMouseButtonDown = false;
                else if (event->button.button == SDL_BUTTON_RIGHT)
                    *rightMouseButtonDown = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event->button.button == SDL_BUTTON_LEFT)
                    *leftMouseButtonDown = true;
                else if (event->button.button == SDL_BUTTON_RIGHT)
                    *rightMouseButtonDown = true;
                
                SDL_SetRelativeMouseMode(SDL_TRUE);
                break;
            case SDL_MOUSEMOTION:
                *mouseX = event->motion.x;
                *mouseY = event->motion.y;
                *mouseDX = event->motion.xrel;
                *mouseDY = event->motion.yrel;
                break;
            case SDL_KEYDOWN:
                switch(event->key.keysym.sym){
                    case SDLK_LEFTBRACKET:
                        *bracketDown = -1;
                        break;
                    case SDLK_RIGHTBRACKET:
                        *bracketDown = 1;
                        break;
                    case SDLK_ESCAPE:
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                        break;
                    case SDLK_F10:
                        if(event->key.repeat == 0)
                        {
                            if(inFullScreen)
                                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                            else
                                SDL_SetWindowFullscreen(window, 0);

                            inFullScreen = !inFullScreen;
                        }
                        break;
                }
                break;
            case SDL_KEYUP:
                switch(event->key.keysym.sym){
                    case SDLK_LEFTBRACKET:
                        *bracketDown = 0;
                        break;
                    case SDLK_RIGHTBRACKET:
                        *bracketDown = 0;
                        break;
                }
                break;
            case SDL_QUIT:
                *quit = true;
                break;
        }
    }
}

/*
// http://members.chello.at/~easyfilter/bresenham.html
 * http://www.dinomage.com/tag/sdl_gpu/
static void drawLine(int x0, int y0, int x1, int y1)
{
    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
    int err = (dx>dy ? dx : -dy)/2, e2;

    for(;;)
    {
      setPixel(x0,y0, 255, 255, 255, 255);
      if (x0==x1 && y0==y1) break;
      e2 = err;
      if (e2 >-dx) { err -= dy; x0 += sx; }
      if (e2 < dy) { err += dx; y0 += sy; }
    }
}
*/

static void setPixel(int x, int y, int R, int G, int B, int A)
{
    pixels[screenWidth * y + x] = RGBA2INT(R,G,B,A);
}

static long getNanoTime() 
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

Uint32 RGBA2INT(int R, int G, int B, int A)
{
    return SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), R, G, B, A);
}