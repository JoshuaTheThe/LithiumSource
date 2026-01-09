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

void NPC_Physics(ENTITY *Self, SCENE *Scene)
{
        LiObj(Scene, Self->static_data[0])->Origin = Self->Origin;

        if (Self->static_data[2] == true)
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
        else
        {
                Self->Velocity.X *= 0.9;
                Self->Velocity.Z *= 0.9;
        }

        return;
}

void NPC_Interact(ENTITY *Self, SCENE *Scene)
{
        Self->static_data[2] = !Self->static_data[2]; /* Follow */
        if (Self->static_data[1] == -1)
        {
                Self->static_data[1] = LoadSound(Scene, "assets/yees.wav");
        }
        if (Self->static_data[1] > 0)
        {
                PlaySound(Scene, Self->static_data[1]);
        }
}

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
        size_t obj1 = LithiumLoadObject(Scene, "assets/scientist.obj");
        size_t obj2 = LithiumLoadObject(Scene, "assets/cube.obj"); /* Our Trigger Object */
        LiObj(Scene, obj0)->InteractionBounds.Min.X = 0;
        LiObj(Scene, obj0)->InteractionBounds.Min.Y = 0;
        LiObj(Scene, obj0)->InteractionBounds.Min.Z = 0;
        LiObj(Scene, obj0)->InteractionBounds.Max.X = 0;
        LiObj(Scene, obj0)->InteractionBounds.Max.Y = 0;
        LiObj(Scene, obj0)->InteractionBounds.Max.Z = 0;
        LiObj(Scene, obj1)->Origin.X = 0;
        LiObj(Scene, obj1)->Origin.Y = 5;
        LiObj(Scene, obj1)->Origin.Z = 5;
        LiObj(Scene, obj2)->Rotation.Z = 90;
        LiObj(Scene, obj2)->Rotation.Y = -90;
        LiObj(Scene, obj2)->Origin = LiObj(Scene, obj1)->Origin;
        LiObj(Scene, obj2)->CustomCollisionBehaviour = Trigger;
        LiObj(Scene, obj1)->PhysicsIteration = NPC_Physics;
        LiObj(Scene, obj1)->Interact = NPC_Interact;
        LiObj(Scene, obj1)->static_data[0] = obj2;
        LiObj(Scene, obj1)->static_data[1] = -1;
        LiObj(Scene, obj1)->static_data[2] = false;
        LiObj(Scene, obj2)->IsVisible = false;
        LiObj(Scene, obj2)->IsInteractable = false;
        LiObj(Scene, obj1)->IsStatic = false;
        ScaleMesh(LiObj(Scene, obj2), 2.0);

        // size_t obj1 = LithiumLoadObject(Scene, "assets/long plane.obj");
        // size_t obj2 = LithiumLoadObject(Scene, "assets/long plane.obj");
        // size_t obj3 = LithiumLoadObject(Scene, "assets/long plane.obj");
        // size_t obj4 = LithiumLoadObject(Scene, "assets/long plane.obj");
        TEXTURE *Texture = LoadTexture("assets/happy.bmp");
        // LiObj(Scene, obj0)->Origin.Z = 10.0;
        // LiObj(Scene, obj0)->Origin.Y = 1.0;
        // LiObj(Scene, obj0)->Rotation.Y = 180.0;
        // LiObj(Scene, obj1)->Origin.Z = 5.0;
        // LiObj(Scene, obj2)->Origin.Z = 5.0;
        // LiObj(Scene, obj3)->Origin.Z = 5.0;
        // LiObj(Scene, obj4)->Origin.Z = 5.0;
        // LiObj(Scene, obj1)->Origin.Y = 0.0;
        // LiObj(Scene, obj2)->Origin.Y = 2.0;
        // LiObj(Scene, obj3)->Origin.Y = 1.0;
        // LiObj(Scene, obj4)->Origin.Y = 1.0;
        // LiObj(Scene, obj3)->Rotation.Z = 90.0;
        // LiObj(Scene, obj4)->Rotation.Z = 90.0;
        // LiObj(Scene, obj3)->Origin.X = 1.0;
        // LiObj(Scene, obj4)->Origin.X = -1.0;
        Texture->repeat = true;
        Texture->scalex = 10;
        Texture->scaley = 10;
        LithiumApplyTexture(Scene, Texture, obj2);

        while (Scene->Running)
        {
                SceneTick(&Scene);
                LithiumUpdate(Scene);
                DrawScene(Scene);
        }

        LithiumEnd(Scene);
        return (0);
}
