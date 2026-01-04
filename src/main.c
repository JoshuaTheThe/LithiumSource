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
#include <engine/texture.h>

int compile(void);

char *ProgramPath = NULL;

int main(int Count, char **Arguments)
{
        if (Count == 2 && !strncmp("--compile", Arguments[1], 10))
        {
                return compile();
        }
        const int width = 400;
        const int height = 400;
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
        LoadMeshFromFile("assets/monke.obj", Mesh);
        da_append(Scene, Mesh);
        Mesh->origin.Z = 10.0;
        Mesh->origin.Y = 1.0;
        Mesh->ROTY = 180.0;
        TEXTURE *Texture = NULL;//LoadTexture("happy.bmp");
        if (Texture)
        {
                for (size_t i = 0; i < Mesh->tri_count; ++i)
                {
                        Mesh->tris[i].Texture = Texture;
                }
        }

        Mesh = InitMesh(0);
        LoadMeshFromFile("assets/long plane.obj", Mesh);
        da_append(Scene, Mesh);
        Mesh->origin.Z = 5.0;
        if (Texture)
        {
                for (size_t i = 0; i < Mesh->tri_count; ++i)
                {
                        Mesh->tris[i].Texture = Texture;
                }
        }

        size_t jumpidx = LoadSound(Scene, "assets/jump.wav");
        size_t denyselect = LoadSound(Scene, "assets/denyselect.wav");
        size_t walkidxs[4];
        walkidxs[0] = LoadSound(Scene, "assets/walk_0.wav");
        walkidxs[1] = LoadSound(Scene, "assets/walk_1.wav");
        walkidxs[2] = LoadSound(Scene, "assets/walk_2.wav");
        walkidxs[3] = LoadSound(Scene, "assets/walk_3.wav");

        Scene->Camera.Bounds.Max = (VEC3){.X = 0.254, .Y = 0.254, .Z = 0.254};
        Scene->Camera.Bounds.Min = (VEC3){.X = -0.254, .Y = -1.5748, .Z = -0.254};
        Scene->Camera.Position.Y = 10.0;
        Scene->Camera.Velocity.Y = 0.1;
        const double walk_speed = 0.1;
        const double rot_speed = 90.0;
        double speed = 0.1;
        double fast_speed = 0.2;

        int walk_cycle = 0;

        bool flying = true;

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
                if (!flying)
                        PhysicsTick(Scene);
                else
                {
                        Scene->Camera.Velocity.X -= Scene->Camera.Velocity.X * FRICTION * Scene->dt;
                        Scene->Camera.Velocity.Y -= Scene->Camera.Velocity.Y * FRICTION * Scene->dt;
                        Scene->Camera.Velocity.Z -= Scene->Camera.Velocity.Z * FRICTION * Scene->dt;
                        Scene->Camera.Position.X += Scene->Camera.Velocity.X * Scene->dt;
                        Scene->Camera.Position.Y += Scene->Camera.Velocity.Y * Scene->dt;
                        Scene->Camera.Position.Z += Scene->Camera.Velocity.Z * Scene->dt;
                }

                if (Scene->Keymap['q'] && flying)
                {
                        speed = fast_speed;
                }
                else
                {
                        speed = walk_speed;
                }
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
                        Scene->Camera.Rotation.Y -= rot_speed * (double)Scene->dt;
                }
                if (Scene->Keymap['x'])
                {
                        Scene->Camera.Rotation.Y += rot_speed * (double)Scene->dt;
                }
                if (Scene->Keymap['r'])
                {
                        Scene->Camera.Rotation.X += rot_speed * (double)Scene->dt;
                }
                if (Scene->Keymap['f'])
                {
                        Scene->Camera.Rotation.X -= rot_speed * (double)Scene->dt;
                }
                if (Scene->Keymap['c'] && flying)
                {
                        Scene->Camera.Velocity.Y -= speed;
                }
                if (Scene->Keymap[' '] && flying)
                {
                        Scene->Camera.Velocity.Y += speed;
                }
                else if (Scene->Keymap[' '] && Scene->Grounded)
                {
                        Scene->Camera.Velocity.Y = GRAVITY * JUMP_POWER;
                        PlaySound(Scene, jumpidx);
                }
                else if (Scene->JustPressed['e'])
                {
                        PlaySound(Scene, denyselect);
                        Scene->JustPressed['e'] = false;
                }
                if (Scene->Keymap['v'])
                {
                        flying = true;
                }
                if (Scene->Keymap['n'])
                {
                        flying = false;
                }

                Scene->footstep_timer -= Scene->dt;

                if ((Scene->Keymap['w'] || Scene->Keymap['s'] ||
                     Scene->Keymap['a'] || Scene->Keymap['d']) &&
                    Scene->Grounded && Scene->footstep_timer <= 0.0)
                {
                        walk_cycle = (walk_cycle + 1) % 4;
                        PlaySound(Scene, walkidxs[walk_cycle]);
                        Scene->footstep_timer = Scene->footstep_interval;
                }

                UpdateSounds(Scene);
                SDL_SetRenderDrawColor(Scene->Renderer.Renderer, 0, 0, 0, 255);
                SDL_RenderClear(Scene->Renderer.Renderer);
                DrawScene(Scene);
                SDL_RenderPresent(Scene->Renderer.Renderer);

                Scene->new = SDL_GetTicks();
                Scene->dt = (double)(Scene->new - Scene->old) / 1000.0;
                Scene->old = Scene->new;
        }

        SceneEnd(NULL, true);
        // /FreeTextureData(Texture);
        free(ProgramPath);
end:
        (void)Count, (void)Arguments;
        return (0);
}
