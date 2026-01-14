#include <engine/lithium.h>

void Trigger(ENTITY *Self, SCENE *Scene, VEC3 Overlap)
{
        if (Self->static_data[0] == false)
        {
                Self->static_data[1] = LoadSound(Scene, "assets/ahfreeman.wav");
                PlaySound(Scene, Self->static_data[1]);
                printf("INFO: Triggered %p in %p with an overlap of (%f, %f, %f)\n", Self, Scene, Overlap.X, Overlap.Y, Overlap.Z);
        }
        Self->static_data[0] = true;
}

void CrowBarHit(TOOL *Self, ENTITY *Hit, SCENE *Scene, double Distance)
{
        if (Self->Entity->static_data[3] == -1)
        {
                Self->Entity->static_data[0] = LoadSound(Scene, "assets/cbar_hit1.wav");
                Self->Entity->static_data[1] = LoadSound(Scene, "assets/cbar_hit2.wav");
                Self->Entity->static_data[2] = 0;
                Self->Entity->static_data[3] = 0;
                *((float *)&Self->Entity->static_data[4]) = 0.0;
        }

        size_t soundId = Self->Entity->static_data[Self->Entity->static_data[2]];
        if (*((float *)&Self->Entity->static_data[4]) <= 0)
        {
                Self->Entity->static_data[2] ^= 1;
                Hit->Health -= 10.0;
                PlaySound(Scene, soundId);
                *((float *)&Self->Entity->static_data[4]) = 0.05;
        }

        if (Hit->Hurt)
        {
                Hit->Hurt(Hit, Scene, Self->Entity);
        }
        *((float *)&Self->Entity->static_data[4]) -= Scene->dt * 10.0;
}

void CrowBarFire(TOOL *Self) {}
void CrowBarEnd(TOOL *Self) {}

int main(int Count, char **Arguments)
{
        SCENE *Scene = LithiumInit(Count, Arguments);
        Scene->SoundSys.PrimaryJumpSound = LoadSound(Scene, "assets/jump.wav");
        Scene->SoundSys.DenySelectSound = LoadSound(Scene, "assets/denyselect.wav");
        Scene->SoundSys.PrimaryStepSounds[0] = LoadSound(Scene, "assets/walk_0.wav");
        Scene->SoundSys.PrimaryStepSounds[1] = LoadSound(Scene, "assets/walk_1.wav");
        Scene->SoundSys.PrimaryStepSounds[2] = LoadSound(Scene, "assets/walk_2.wav");
        Scene->SoundSys.PrimaryStepSounds[3] = LoadSound(Scene, "assets/walk_3.wav");

        size_t obj0 = LithiumLoadObject(Scene, "assets/map.obj");
        size_t obj1 = LithiumCreateNPC(Scene, "assets/scientist.obj");
        size_t obj2 = LithiumLoadObject(Scene, "assets/cube.obj"); /* Our Trigger Object */
        size_t obj3 = LithiumLoadObject(Scene, "assets/crowbar.obj");
        LiObj(Scene, obj1)->static_data[NPC_STATIC_TRIGGER] = obj2;
        LiObj(Scene, obj2)->IsInteractable = false;
        LiObj(Scene, obj2)->IsVisible = false;
        LiObj(Scene, obj2)->CustomCollisionBehaviour = Trigger;
        ScaleMesh(LiObj(Scene, obj2), 2.0);

        LiObj(Scene, obj0)->IsInteractable = false;
        LiObj(Scene, obj1)->Origin.X = 0;
        LiObj(Scene, obj1)->Origin.Y = 5;
        LiObj(Scene, obj1)->Origin.Z = 5;
        LiObj(Scene, obj3)->static_data[3] = -1;

        TEXTURE *Texture = LoadTexture("assets/happy.bmp");
        TEXTURE *Crowbar = LoadTexture("assets/crowbar.bmp");
        Texture->repeat = true;
        Texture->scalex = 10;
        Texture->scaley = 10;
        LithiumApplyTexture(Scene, Texture, obj2);
        LithiumApplyTexture(Scene, Crowbar, obj3);

        Scene->Player.CurrentTool = LithiumAddTool(Scene, LiObj(Scene, obj3), CrowBarHit, CrowBarFire, CrowBarEnd, 1.0);

        while (Scene->Running)
        {
                SceneTick(&Scene);
                LithiumUpdate(Scene);
                DrawScene(Scene);
        }

        LithiumEnd(Scene);
        return (0);
}
