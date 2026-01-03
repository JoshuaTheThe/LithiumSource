#ifndef DRAW_H
#define DRAW_H

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>
#include<stdint.h>
#include<stddef.h>
#include<SDL2/SDL.h>
#include<engine/types.h>
#include <engine/scene.h>
#include <engine/texture.h>

void PutPixel(SCENE *Scene, double X, double Y, double Z, COLOUR Col);
void DrawLine(SCENE *Scene, const VEC3 A, const VEC3 B, COLOUR Col);
void DrawTri(SCENE *Scene, TRI3D Tri);
void DrawTriWTex(SCENE *Scene, TRI3D Tri);
void PutPixelWTex(SCENE *Scene, double X, double Y, double Z, UV Uv, TEXTURE *Texture, double lum);

#endif
