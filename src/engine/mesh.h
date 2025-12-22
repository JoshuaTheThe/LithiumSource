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
#include <todo.h>

bool LoadMeshFromFile(const char *fileName, Mesh3D *mesh);
void DelMesh(Mesh3D *mesh);
Mesh3D *InitMesh(size_t triCount);
bool PlayerCollides(SCENE *Scene, TRI3D *Tri);

#endif
