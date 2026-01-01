#ifndef PHYSICS_H
#define PHYSICS_H

#include <engine/camera.h>
#include <engine/types.h>
#include <engine/scene.h>

extern const double TERMINAL_VELOCITY;
extern const double GRAVITY;
extern const double FRICTION;
extern const double PHYSICS_WAIT;
extern const double JUMP_POWER;

void PhysicsTick(SCENE *Scene, Mesh3D *Ground);

#endif
