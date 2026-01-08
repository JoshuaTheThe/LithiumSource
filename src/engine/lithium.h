#ifndef LITHIUM_H
#define LITHIUM_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <libgen.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>

#include <engine/types.h>
#include <engine/camera.h>
#include <engine/draw.h>
#include <engine/scene.h>
#include <engine/mesh.h>
#include <engine/physics.h>
#include <engine/sound.h>
#include <engine/texture.h>
#include <engine/lithium.h>
#include <engine/interaction.h>

SCENE *LithiumInit(int argc, char **argv);
void LithiumEnd(SCENE *Scene);
void LithiumUpdate(SCENE *ActiveScene);
size_t LithiumLoadObject(SCENE *Scene, char *Path);
Mesh3D *LiObj(SCENE *Scene, size_t Index);
void LithiumApplyTexture(SCENE *Scene, TEXTURE *Tex, size_t Index);

#endif
