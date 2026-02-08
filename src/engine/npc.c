#include <engine/npc.h>

void NPC_Hit(ENTITY *Self, SCENE *Scene, ENTITY *Cause)
{
        if (!Scene->SoundSys.Sounds[Self->static_data[NPC_STATIC_SCREAM_ID]].Playing)
                PlaySound(Scene, Self->static_data[NPC_STATIC_SCREAM_ID]);
        Self->static_data[NPC_STATIC_STATE] = NPC_STATE_RUNNING;
        *(double **)&Self->static_data[NPC_STATIC_ORG_X] = &Cause->Origin.X;
        *(double **)&Self->static_data[NPC_STATIC_ORG_Y] = &Cause->Origin.Y;
        *(double **)&Self->static_data[NPC_STATIC_ORG_Z] = &Cause->Origin.Z;

        if (Self->Health <= 0.0)
        {
                PlaySound(Scene, Self->static_data[NPC_STATIC_DEATH_ID]);
                Self->static_data[NPC_STATIC_STATE] = NPC_STATE_DEAD;
                Self->IsVisible = false;
                Self->IsCollidable = false;
                Self->IsInteractable = false;
        }
}

void NPC_Follow(ENTITY *Self, SCENE *Scene)
{
        VEC3 player_pos = Scene->Player.Position;

        VEC3 direction = SubVec3(&player_pos, &Self->Origin);
        double distance = LenVec3(&direction);

        if (distance > 2.0 && distance < 30.0)
        {
                if (distance > 0.001)
                {
                        direction = NormaliseVec3(&direction);

                        double speed = 2.0;
                        Self->Velocity.X = direction.X * speed;
                        Self->Velocity.Z = direction.Z * speed;

                        double angle_to_player = atan2(direction.X, direction.Z) * 180.0 / M_PI;
                        Self->Rotation.Y = -angle_to_player;
                }
        }
        else if (distance <= 2.0)
        {
                Self->Velocity.X = 0;
                Self->Velocity.Z = 0;
        }
        else
        {
                Self->Velocity.X = 0;
                Self->Velocity.Z = 0;
        }
}

void NPC_Run(ENTITY *Self, SCENE *Scene, VEC3 Origin)
{
        VEC3 direction = SubVec3(&Origin, &Self->Origin);
        double distance = LenVec3(&direction);

        if (distance > 0.001)
        {
                direction = NormaliseVec3(&direction);

                double speed = 4.0;
                Self->Velocity.X = -direction.X * speed;
                Self->Velocity.Z = -direction.Z * speed;

                double angle_away_from_origin = atan2(-direction.X, -direction.Z) * 180.0 / M_PI;
                Self->Rotation.Y = -angle_away_from_origin;
        }
}

void NPC_Physics(ENTITY *Self, SCENE *Scene)
{
        if (Self->static_data[NPC_STATIC_TRIGGER] != -1)
                LiObj(Scene, Self->static_data[NPC_STATIC_TRIGGER])->Origin = Self->Origin;

        if (Self->static_data[NPC_STATIC_STATE] == NPC_STATE_FOLLOWING)
        {
                NPC_Follow(Self, Scene);
        }
        else if (Self->static_data[NPC_STATIC_STATE] == NPC_STATE_IDLE)
        {
                Self->Velocity.X *= 0.9;
                Self->Velocity.Z *= 0.9;
        }
        else if (Self->static_data[NPC_STATIC_STATE] == NPC_STATE_RUNNING)
        {
                VEC3 Origin;
                Origin.X = **(double **)&Self->static_data[NPC_STATIC_ORG_X];
                Origin.Y = **(double **)&Self->static_data[NPC_STATIC_ORG_Y];
                Origin.Z = **(double **)&Self->static_data[NPC_STATIC_ORG_Z];
                NPC_Run(Self, Scene, Origin);
        }

        *((float *)&Self->static_data[NPC_STATIC_OLD_HEALTH]) = Self->Health; /* Old Health */
        return;
}

void NPC_Interact(ENTITY *Self, SCENE *Scene)
{
        if (Self->static_data[NPC_STATIC_STATE] == NPC_STATE_FOLLOWING)
                Self->static_data[NPC_STATIC_STATE] = NPC_STATE_IDLE;
        else
                Self->static_data[NPC_STATIC_STATE] = NPC_STATE_FOLLOWING;
        if (Self->static_data[NPC_STATIC_FOLLOW_AGREE_ID] > 0)
        {
                PlaySound(Scene, Self->static_data[NPC_STATIC_FOLLOW_AGREE_ID]);
        }
}

size_t LithiumCreateNPC(SCENE *Scene, char *Path)
{
        size_t obj = LithiumLoadObject(Scene, Path);
        LiObj(Scene, obj)->PhysicsIteration = NPC_Physics;
        LiObj(Scene, obj)->Hurt = NPC_Hit;
        LiObj(Scene, obj)->Interact = NPC_Interact;
        LiObj(Scene, obj)->static_data[NPC_STATIC_SCREAM_ID] = LoadSound(Scene, "assets/valve/sci_pain3.wav");
        LiObj(Scene, obj)->static_data[NPC_STATIC_FOLLOW_AGREE_ID] = LoadSound(Scene, "assets/valve/yees.wav");
        LiObj(Scene, obj)->static_data[NPC_STATIC_DEATH_ID] = LoadSound(Scene, "assets/valve/whatyoudoing.wav");
        LiObj(Scene, obj)->static_data[NPC_STATIC_OLD_HEALTH] = LiObj(Scene, obj)->Health;
        LiObj(Scene, obj)->static_data[NPC_STATIC_STATE] = NPC_STATE_IDLE;
        LiObj(Scene, obj)->static_data[NPC_STATIC_TRIGGER] = -1;
        LiObj(Scene, obj)->IsStatic = false;
        return obj;
}
