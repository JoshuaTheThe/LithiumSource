#ifndef TEX_H
#define TEX_H

#include<engine/draw.h>
#include<engine/scene.h>
#include<todo.h>

TEXTURE *LoadTexture(const char *path);
void FreeTextureData(TEXTURE *tex);
COLOUR SampleTexture(TEXTURE *tex, UV uv);

#endif
