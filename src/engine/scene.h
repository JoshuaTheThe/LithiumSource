#ifndef STATE_H
#define STATE_H

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>
#include<stdint.h>
#include<stddef.h>
#include<engine/camera.h>
#include<engine/draw.h>
#include<engine/types.h>

SCENE *SceneInit( const char *Title, int X, int Y, int W, int H );
void SceneEnd( SCENE *Scene, bool Final );
void SceneTick( SCENE **Scene );

#endif
