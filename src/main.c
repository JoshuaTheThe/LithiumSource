#include <engine/lithium.h>

int main(int Count, char **Arguments)
{
        SCENE *Scene = LithiumInit(Count, Arguments);
        Scene->SoundSys.PrimaryJumpSound     = LoadSound(Scene, "assets/jump.wav");
        Scene->SoundSys.DenySelectSound      = LoadSound(Scene, "assets/denyselect.wav");
        Scene->SoundSys.PrimaryStepSounds[0] = LoadSound(Scene, "assets/walk_0.wav");
        Scene->SoundSys.PrimaryStepSounds[1] = LoadSound(Scene, "assets/walk_1.wav");
        Scene->SoundSys.PrimaryStepSounds[2] = LoadSound(Scene, "assets/walk_2.wav");
        Scene->SoundSys.PrimaryStepSounds[3] = LoadSound(Scene, "assets/walk_3.wav");

        size_t obj0 = LithiumLoadObject(Scene, "assets/monke.obj");
        size_t obj1 = LithiumLoadObject(Scene, "assets/long plane.obj");
        size_t obj2 = LithiumLoadObject(Scene, "assets/long plane.obj");
        size_t obj3 = LithiumLoadObject(Scene, "assets/long plane.obj");
        size_t obj4 = LithiumLoadObject(Scene, "assets/long plane.obj");
        TEXTURE *Texture = LoadTexture("assets/happy.bmp");
        LiObj(Scene, obj0)->Origin.Z = 10.0;
        LiObj(Scene, obj0)->Origin.Y = 1.0;
        LiObj(Scene, obj0)->Rotation.Y = 180.0;
        LiObj(Scene, obj1)->Origin.Z = 5.0;
        LiObj(Scene, obj2)->Origin.Z = 5.0;
        LiObj(Scene, obj3)->Origin.Z = 5.0;
        LiObj(Scene, obj4)->Origin.Z = 5.0;

        LiObj(Scene, obj1)->Origin.Y = 0.0;
        LiObj(Scene, obj2)->Origin.Y = 2.0;
        LiObj(Scene, obj3)->Origin.Y = 1.0;
        LiObj(Scene, obj4)->Origin.Y = 1.0;

        LiObj(Scene, obj3)->Rotation.Z = 90.0;
        LiObj(Scene, obj4)->Rotation.Z = 90.0;

        LiObj(Scene, obj3)->Origin.X = 1.0;
        LiObj(Scene, obj4)->Origin.X = -1.0;
        
        LithiumApplyTexture(Scene, Texture, obj1);

        while (Scene)
        {
                SceneTick(&Scene);
                LithiumUpdate(Scene);
                DrawScene(Scene);
        }

        LithiumEnd(Scene);
        return (0);
}
