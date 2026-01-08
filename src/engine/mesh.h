#ifndef MESH_H
#define MESH_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <engine/camera.h>
#include <engine/types.h>
#include <engine/texture.h>
#include <todo.h>

typedef struct
{
        TEXTURE **items;
        size_t count, capacity;
} TEXTURES;

bool LoadMeshFromFile(const char *fileName, Mesh3D *mesh);
void DelMesh(Mesh3D *mesh);
Mesh3D *InitMesh(SCENE *Scene, size_t triCount);
bool PlayerCollides(SCENE *Scene, TRI3D *Tri, VEC3 Origin);
void ScaleMesh(Mesh3D *Mesh, double s);

#endif
