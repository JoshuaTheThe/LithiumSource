#ifndef NPC_H
#define NPC_H

#include <engine/lithium.h>

enum
{
        NPC_STATIC_SCREAM_ID,
        NPC_STATIC_FOLLOW_AGREE_ID,
        NPC_STATIC_DEATH_ID,
        NPC_STATIC_OLD_HEALTH,
        NPC_STATIC_TRIGGER,
        NPC_STATIC_STATE,
        NPC_STATIC_ORG_X,
        NPC_STATIC_ORG__X,
        NPC_STATIC_ORG_Y,
        NPC_STATIC_ORG__Y,
        NPC_STATIC_ORG_Z,
        NPC_STATIC_ORG__Z,
};

enum
{
        NPC_STATE_IDLE,
        NPC_STATE_RUNNING,
        NPC_STATE_FOLLOWING,
        NPC_STATE_DEAD,
};

void NPC_Interact(ENTITY *Self, SCENE *Scene);
void NPC_Physics(ENTITY *Self, SCENE *Scene);
void NPC_Hit(ENTITY *Self, SCENE *Scene, ENTITY *Cause);
size_t LithiumCreateNPC(SCENE *Scene, char *Path);

#endif
