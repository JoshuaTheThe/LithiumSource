#ifndef UI_H
#define UI_H

#include <engine/types.h>
#include <engine/draw.h>
#include <engine/texture.h>

size_t LithiumCreateUXObject(SCENE *Scene, TEXTURE *Tex, VEC3 Origin, double Width, double Height);
void LithiumDrawUXObject(SCENE *Scene, UXOBJECT *UX);
void LithiumCleanupUXObject(UXOBJECT **UX);

#endif
