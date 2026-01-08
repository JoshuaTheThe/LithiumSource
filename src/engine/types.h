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

#define __VER__ "0.1.0" // LETS FUCKING GO

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

#define da_find(xs, x, res)                                    \
        do                                                     \
        {                                                      \
                res = -1;                                      \
                for (size_t _n_ = 0; _n_ < (xs)->count; ++_n_) \
                        if ((xs)->items[_n_] == x)             \
                                res = _n_;                     \
        } while (0)

#define da_free(xs)                               \
        do                                        \
        {                                         \
                free((xs)->items);                \
                (xs)->items = NULL;               \
                (xs)->count = (xs)->capacity = 0; \
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

typedef struct TEXTURE
{
        uint32_t *pixels;
        int width, height;
        int pitch;
} TEXTURE;

typedef struct
{
        double u, v;
} UV;

typedef struct
{
        VEC3 p[3];
        COLOUR col;
        UV uv[3];
        double w[3];
        TEXTURE *Texture;
        bool invalid;
} TRI3D;

typedef struct
{
        VEC3 Min, Max;
} BOUNDS;

typedef struct Mesh3DS
{
        BOUNDS InteractionBounds;
        VEC3 Origin, Scale, Rotation;
        TRI3D *Tris;
        size_t TriCount;
        size_t InteractSound;
        void (*Interact)(struct Mesh3DS *Self);
} Mesh3D;

typedef struct
{
        double m[4][4];
} Mat4x4;

typedef struct
{
        VEC3 Position;
        VEC3 Rotation;
        VEC3 Velocity;
        VEC3 LightPos;
        double FOV, Aspect, Near, Far;
        BOUNDS Bounds;
        
        bool Flying;
        bool Grounded;
        bool IsCrouching;
        bool IsSprinting;

        double StandingHeight;
        double CrouchingHeight;
        double CurrentHeight;
        double Speed, RunSpeed, WalkSpeed, RotSpeed;
        double MaxInteraction;
        double CameraOffsetY;
} PLAYER;

typedef struct
{
        SDL_Window *Window;
        int WindowWidth, WindowHeight;
} WINDOW_SDL;

typedef struct
{
        SDL_Renderer *Renderer;
        double *ZBuffer;
        COLOUR *RGBBuffer;
        int RendererWidth, RendererHeight;
} RENDERER_SDL;

typedef struct
{
        Mix_Chunk *Sample;
        int Channel;
        bool Valid;
        bool Playing;
} SOUND;

typedef struct
{
        SOUND Sounds[512];
        double FootStepTimer;
        double FootStepInterval;
        size_t WalkCycle;
        size_t PrimaryJumpSound, PrimaryStepSounds[4], DenySelectSound;
} SOUND_SYSTEM;

typedef struct SCENE
{
        SOUND_SYSTEM SoundSys;
        KEYMAP Keymap, JustPressed;
        PLAYER Player;
        RENDERER_SDL Renderer;
        WINDOW_SDL Window;
        Mesh3D **items;
        size_t count, capacity;
        size_t new, old;
        double dt;
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

static inline VEC3 AddVec3(const VEC3 *const a, const VEC3 *const b)
{
        return (VEC3){a->X + b->X, a->Y + b->Y, a->Z + b->Z};
}

static inline VEC3 SubVec3(const VEC3 *const a, const VEC3 *const b)
{
        return (VEC3){a->X - b->X, a->Y - b->Y, a->Z - b->Z};
}

static inline VEC3 ScaleVec3Mul(const VEC3 *const a, double s)
{
        return (VEC3){a->X * s, a->Y * s, a->Z * s};
}

static inline VEC3 ScaleVec3Div(const VEC3 *const a, double s)
{
        return (VEC3){a->X / s, a->Y / s, a->Z / s};
}

static inline double DotVec3(const VEC3 *const a, const VEC3 *const b)
{
        return a->X * b->X + a->Y * b->Y + a->Z * b->Z;
}

static inline double LenVec3(const VEC3 *const x)
{
        return sqrt(DotVec3(x, x));
}

static inline VEC3 NormaliseVec3(const VEC3 *const a)
{
        double l = LenVec3(a);
        if (l == 0.0)
        {
                return (VEC3){0};
        }
        return (VEC3){a->X / l, a->Y / l, a->Z / l};
}

static inline VEC3 CrossProdVec3(const VEC3 *const a, const VEC3 *const b)
{
        VEC3 v;
        v.X = a->Y * b->Z - a->Z * b->Y;
        v.Y = a->Z * b->X - a->X * b->Z;
        v.Z = a->X * b->Y - a->Y * b->X;
        return v;
}

extern char *ProgramPath;

#endif
