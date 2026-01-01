#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define da_append(xs, x)                                                                           \
        do                                                                                         \
        {                                                                                          \
                if ((xs)->count >= (xs)->capacity)                                                 \
                {                                                                                  \
                        if ((xs)->capacity == 0)                                                   \
                                (xs)->capacity = 256;                                              \
                        else                                                                       \
                                (xs)->capacity *= 2;                                               \
                        (xs)->items = realloc((xs)->items, (xs)->capacity * sizeof(*(xs)->items)); \
                }                                                                                  \
                                                                                                   \
                (xs)->items[(xs)->count++] = (x);                                                  \
        } while (0)

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

typedef enum
{
        AXIS_X,
        AXIS_Y,
        AXIS_Z
} AXIS;

typedef struct
{
        VEC3 p[3];
        COLOUR col;
        double Depth;
        bool invalid;
} TRI3D;

typedef struct Mesh3DS
{
        TRI3D *tris;
        size_t tri_count;
        VEC3 origin;
        VEC3 Scale;
        double ROTX, ROTY, ROTZ;
} Mesh3D;

typedef struct
{
        double m[4][4];
} Mat4x4;

typedef struct
{
        VEC3 Min, Max;
} BOUNDS;

typedef struct
{
        VEC3 Position;
        VEC3 Rotation;
        VEC3 Velocity;
        double FOV, Aspect, Near, Far;
        BOUNDS Bounds;
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

typedef struct
{
        Mix_Chunk *Sample;
        int Channel;
        bool Valid;
        bool Playing;
} SOUND;

typedef struct SCENE
{
        SOUND Sounds[512];
        KEYMAP Keymap;
        CAMERA Camera;
        RENDERER_SDL Renderer;
        WINDOW_SDL Window;
        COLOUR CurrentColor;
        char padd[4];
        Mesh3D **items;
        size_t count, capacity;
        size_t new, old;
        double dt;
        double footstep_timer;
        double footstep_interval;
        bool Grounded;
} SCENE;

typedef struct
{
        double m[4][4];
} MATRIX4x4;

static inline float Clamp(float v, float min, float max)
{
        if (v < min)
                return min;
        if (v > max)
                return max;
        return v;
}

static inline VEC3 AddVec3(const VEC3 * const a, const VEC3 * const b)
{
        return (VEC3){a->X + b->X, a->Y + b->Y, a->Z + b->Z};
}

static inline VEC3 SubVec3(const VEC3 * const a, const VEC3 * const b)
{
        return (VEC3){a->X - b->X, a->Y - b->Y, a->Z - b->Z};
}

static inline VEC3 ScaleVec3Mul(const VEC3 * const a, double s)
{
        return (VEC3){a->X * s, a->Y * s, a->Z * s};
}

static inline VEC3 ScaleVec3Div(const VEC3 * const a, double s)
{
        return (VEC3){a->X / s, a->Y / s, a->Z / s};
}

static inline double DotVec3(const VEC3 * const a, const VEC3 * const b)
{
        return a->X * b->X + a->Y * b->Y + a->Z * b->Z;
}

static inline double LenVec3(const VEC3 * const x)
{
        return sqrt(DotVec3(x, x));
}

static inline VEC3 NormaliseVec3(const VEC3 * const a)
{
        double l = LenVec3(a);
        if (l == 0.0)
        {
                return (VEC3){0};
        }
        return (VEC3){a->X / l, a->Y / l, a->Z / l};
}

static inline VEC3 CrossProdVec3(const VEC3 * const a, const VEC3 * const b)
{
        VEC3 v;
        v.X = a->Y * b->Z - a->Z * b->Y;
        v.Y = a->Z * b->X - a->X * b->Z;
        v.Z = a->X * b->Y - a->Y * b->X;
        return v;
}

extern char *ProgramPath;

#endif
