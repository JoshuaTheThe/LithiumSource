#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <libgen.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>

#include <engine/camera.h>
#include <engine/draw.h>
#include <engine/scene.h>
#include <engine/mesh.h>
#include <engine/physics.h>
#include <engine/sound.h>

int compile(void);

char *ProgramPath = NULL;

int main(int Count, char **Arguments)
{
        if (Count == 2 && !strncmp("--compile", Arguments[1], 10))
        {
                return compile();
        }
        const int width = 500;
        const int height = 500;
        const int scale = 2;

        int result = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512);
        if (result < 0)
        {
                fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
                exit(-1);
        }

        ProgramPath = strdup(Arguments[0]);
        if (!ProgramPath)
                goto end;
        ProgramPath = dirname(ProgramPath);

        Mix_AllocateChannels(16);

        SCENE *Scene = SceneInit("Li3D", 0, 0, width * scale, height * scale);
        InitProjectionMat(Scene);
        SDL_RenderSetLogicalSize(Scene->Renderer.Renderer, width, height);
        Scene->Renderer.RendererHeight /= scale;
        Scene->Renderer.RendererWidth /= scale;
        Mesh3D *Mesh = InitMesh(0);
        LoadMeshFromFile("assets/test.obj", Mesh);

        size_t jumpidx = LoadSound(Scene, "assets/jump.wav");
        size_t walkidxs[4];
        walkidxs[0] = LoadSound(Scene, "assets/walk_0.wav");
        walkidxs[1] = LoadSound(Scene, "assets/walk_1.wav");
        walkidxs[2] = LoadSound(Scene, "assets/walk_2.wav");
        walkidxs[3] = LoadSound(Scene, "assets/walk_3.wav");

        Scene->Camera.Bounds.Max = (VEC3){.X = 1.0, .Y = 1.0, .Z = 1.0};
        Scene->Camera.Bounds.Min = (VEC3){.X = -1.0, .Y = -2.5, .Z = -1.0};
        Scene->Camera.Position.Y = 10.0;
        Scene->Camera.Velocity.Y = 0.1;
        double speed = 0.1;

        int walk_cycle = 0;

        while (Scene)
        {
                SceneTick(&Scene);
                if (!Scene)
                        break;
                double yaw = DEG_TO_RAD(Scene->Camera.Rotation.Y);

                VEC3 forward = {
                    sin(yaw),
                    0.0,
                    cos(yaw)};

                VEC3 right = {
                    cos(yaw),
                    0.0,
                    -sin(yaw)};

                /* Physics */
                PhysicsTick(Scene, Mesh);

                if (Scene->Keymap['w'])
                {
                        Scene->Camera.Velocity.X += speed * forward.X;
                        Scene->Camera.Velocity.Z += speed * forward.Z;
                }
                if (Scene->Keymap['s'])
                {
                        Scene->Camera.Velocity.X -= speed * forward.X;
                        Scene->Camera.Velocity.Z -= speed * forward.Z;
                }
                if (Scene->Keymap['a'])
                {
                        Scene->Camera.Velocity.X -= speed * right.X;
                        Scene->Camera.Velocity.Z -= speed * right.Z;
                }
                if (Scene->Keymap['d'])
                {
                        Scene->Camera.Velocity.X += speed * right.X;
                        Scene->Camera.Velocity.Z += speed * right.Z;
                }
                if (Scene->Keymap['z'])
                {
                        Scene->Camera.Rotation.Y -= 45 * (double)Scene->dt;
                }
                if (Scene->Keymap['x'])
                {
                        Scene->Camera.Rotation.Y += 45 * (double)Scene->dt;
                }
                if (Scene->Keymap['r'])
                {
                        Scene->Camera.Rotation.X += 45 * (double)Scene->dt;
                }
                if (Scene->Keymap['f'])
                {
                        Scene->Camera.Rotation.X -= 45 * (double)Scene->dt;
                }
                if (Scene->Keymap[' '] && Scene->Camera.Velocity.Y == 0.00) /* Stinky hack */
                {
                        Scene->Camera.Velocity.Y = GRAVITY * JUMP_POWER;
                        PlaySound(Scene, jumpidx);
                }

                Scene->footstep_timer -= Scene->dt;

                if ((Scene->Keymap['w'] || Scene->Keymap['s'] ||
                     Scene->Keymap['a'] || Scene->Keymap['d']) &&
                    Scene->Camera.Velocity.Y == 0.0 &&
                    Scene->footstep_timer <= 0.0)
                {
                        walk_cycle = (walk_cycle + 1) % 4;
                        PlaySound(Scene, walkidxs[walk_cycle]);
                        Scene->footstep_timer = Scene->footstep_interval;
                }

                UpdateSounds(Scene);
                SDL_SetRenderDrawColor(Scene->Renderer.Renderer, 0, 0, 0, 255);
                SDL_RenderClear(Scene->Renderer.Renderer);
                DrawObject(Mesh, Scene);
                SDL_RenderPresent(Scene->Renderer.Renderer);

                Scene->new = SDL_GetTicks();
                Scene->dt = (float)(Scene->new - Scene->old) / 1000.0;
                Scene->old = Scene->new;
        }

        SceneEnd(NULL, true);
        DelMesh(Mesh);
        free(ProgramPath);
end:
        (void)Count, (void)Arguments;
        return (0);
}
