#ifndef SOUND_H
#define SOUND_H

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
#include <todo.h>
#include <engine/types.h>

size_t LoadSound(SCENE *Scene, const char *Path);
void CleanupSound(SCENE *Scene);
void PlaySound(SCENE *Scene, size_t SoundIdx);
void UpdateSounds(SCENE *Scene);

#endif
