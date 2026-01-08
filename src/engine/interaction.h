#ifndef INTERACTION_H
#define INTERACTION_H

#include <engine/types.h>
#include <engine/mesh.h>

typedef struct
{
        VEC3 InitialPos, InitialDir, Pos;
} RAY3D;

Mesh3D *CastRay(SCENE *Scene, RAY3D Ray);

#endif
