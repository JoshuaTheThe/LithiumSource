#ifndef TOOL_H
#define TOOL_H

#include <engine/types.h>
#include <engine/mesh.h>
#include <engine/interaction.h>

size_t LithiumAddTool(SCENE *Scene, ENTITY *BindedEntity, void (*Hit)(struct TOOL *Self, ENTITY *Entity, SCENE* Scene, double Dist), void (*Fire)(struct TOOL *Self), void (*EndFire)(struct TOOL *Self), double Range);
void LithiumClearInventory(SCENE *Scene);
void LithiumUpdateTools(SCENE *Scene);
void LithiumFire(SCENE *Scene);
void LithiumEndFire(SCENE *Scene);

#endif
