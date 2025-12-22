#ifndef CAMERA_H
#define CAMERA_H

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>
#include<stdint.h>
#include<stddef.h>
#include<SDL2/SDL.h>
#include<engine/types.h>

#define WIRE_FRAME 0
#define RAD_TO_DEG(r) ((180 / M_PI) * r)
#define DEG_TO_RAD(r) ((M_PI / 180) * r)
#define frameDelay 33.3333333
#define camSpeedIncrease 1
#define TIME_SCALE 1.0

size_t DrawObject(Mesh3D *Cube, SCENE *Scene);
void InitProjectionMat(SCENE *Scene);

#endif
