#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef char KEYMAP[256];

typedef struct
{
        double X;
        double Y;
        double Z;
} VEC3;

typedef struct
{
        uint32_t r, g, b;
} COLOUR;

typedef struct
{
        VEC3 p[3];
        COLOUR col;
        bool invalid;
} TRI3D;

typedef struct Mesh3DS
{
        TRI3D *tris;
        size_t tri_count;
        VEC3 origin;
        double ROTX, ROTY, ROTZ;
} Mesh3D;

typedef struct
{
        double m[4][4];
} Mat4x4;

typedef struct
{
        VEC3 Position;
        VEC3 Rotation;
        double FOV, Aspect, Near, Far;
} CAMERA;

typedef struct
{
        SDL_Window *Window;
        int WindowWidth, WindowHeight;
} WINDOW_SDL;

typedef struct
{
        SDL_Renderer *Renderer;
        int RendererWidth, RendererHeight;
} RENDERER_SDL;

typedef struct SCENE
{
        KEYMAP Keymap;
        CAMERA Camera;
        RENDERER_SDL Renderer;
        WINDOW_SDL Window;
        COLOUR CurrentColor;
        char padd[4];
        size_t new, old;
        float dt;
} SCENE;

typedef struct
{
        double m[4][4];
} MATRIX4x4;

#endif
