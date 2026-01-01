#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <engine/camera.h>
#include <engine/draw.h>
#include <engine/scene.h>
#include <engine/mesh.h>
#include <engine/physics.h>

int main(int Count, char **Arguments)
{
        const int width = 800;
        const int height = 600;
        const int scale = 1;
        SCENE *Scene = SceneInit("Li3D", 0, 0, width * scale, height * scale);
        InitProjectionMat(Scene);
        SDL_RenderSetLogicalSize(Scene->Renderer.Renderer, width, height);
        Scene->Renderer.RendererHeight /= scale;
        Scene->Renderer.RendererWidth /= scale;
        Mesh3D *Mesh = InitMesh(0);
        LoadMeshFromFile("test.obj", Mesh);

        Scene->Camera.Bounds.Max = (VEC3){.X = 1.0, .Y = 1.0, .Z = 1.0};
        Scene->Camera.Bounds.Min = (VEC3){.X = -1.0, .Y = -2.5, .Z = -1.0};
        Scene->Camera.Position.Y = 10.0;
        Scene->Camera.Velocity.Y = 0.1;
        double speed = 0.005;

        while (Scene)
        {
                SceneTick(&Scene);
                if (!Scene)
                        break;
                double cameraYawRad = (M_PI / 180.0) * Scene->Camera.Rotation.Y;
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
                        Scene->Camera.Rotation.Y -= 5 * (double)Scene->dt;
                }
                if (Scene->Keymap['x'])
                {
                        Scene->Camera.Rotation.Y += 5 * (double)Scene->dt;
                }
                if (Scene->Keymap['r'])
                {
                        Scene->Camera.Rotation.X += 5 * (double)Scene->dt;
                }
                if (Scene->Keymap['f'])
                {
                        Scene->Camera.Rotation.X -= 5 * (double)Scene->dt;
                }
                if (Scene->Keymap[' '] && Scene->Camera.Velocity.Y == 0.00) /* Stinky hack */
                {
                        Scene->Camera.Velocity.Y = GRAVITY * JUMP_POWER;
                }

                SDL_SetRenderDrawColor(Scene->Renderer.Renderer, 0, 0, 0, 255);
                SDL_RenderClear(Scene->Renderer.Renderer);
                DrawObject(Mesh, Scene);
                SDL_RenderPresent(Scene->Renderer.Renderer);

                Scene->new = SDL_GetTicks();
                Scene->dt = (float)(Scene->new - Scene->old) / 100.0;
                Scene->old = Scene->new;
        }

        SceneEnd(NULL, true);
        DelMesh(Mesh);
        (void)Count, (void)Arguments;
        return (0);
}
