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

const double TERMINAL_VELOCITY = 100.0;

int main(int Count, char **Arguments)
{
        const int width = 1024;
        const int height = 768;
        const int scale = 1;
        SCENE *Scene = SceneInit("Li3D", 0, 0, width*scale, height*scale);
        InitProjectionMat(Scene);
        SDL_RenderSetLogicalSize(Scene->Renderer.Renderer, width, height);
        Scene->Renderer.RendererHeight /= scale;
        Scene->Renderer.RendererWidth /= scale;
        Mesh3D *Mesh = InitMesh(0);
        LoadMeshFromFile("plane.obj", Mesh);
        Mesh->origin = (VEC3){0, 0, 0};
        Mesh->Scale.X = 100.0;
        Mesh->Scale.Y = 100.0;
        Mesh->Scale.Z = 100.0;

        Scene->Camera.Bounds.Max = (VEC3){.X= 0.5, .Y= 2, .Z= 0.5};
        Scene->Camera.Bounds.Min = (VEC3){.X=-0.5, .Y=-2, .Z=-0.5};

        Scene->Camera.Position.Y = 10.0;
        
        bool collided = false;

        while (Scene)
        {
                SceneTick(&Scene);
                if (!Scene)
                        break;
                double cameraYawRad = (M_PI / 180.0) * Scene->Camera.Rotation.Y;
                
                if (Scene->Keymap['w'])
                {
                        Scene->Camera.Velocity.X += sin(-cameraYawRad);
                        Scene->Camera.Velocity.Z += cos(-cameraYawRad);
                }
                if (Scene->Keymap['s'])
                {
                        Scene->Camera.Velocity.X -= sin(-cameraYawRad);
                        Scene->Camera.Velocity.Z -= cos(-cameraYawRad);
                }
                if (Scene->Keymap['a'])
                {
                        Scene->Camera.Velocity.X -= sin(-cameraYawRad + DEG_TO_RAD(90));
                        Scene->Camera.Velocity.Z -= cos(-cameraYawRad + DEG_TO_RAD(90));
                }
                if (Scene->Keymap['d'])
                {
                        Scene->Camera.Velocity.X += sin(-cameraYawRad + DEG_TO_RAD(90));
                        Scene->Camera.Velocity.Z += cos(-cameraYawRad + DEG_TO_RAD(90));
                }
                if (Scene->Keymap['z'])
                {
                        Scene->Camera.Rotation.Y += 5*(double)Scene->dt;
                }
                if (Scene->Keymap['x'])
                {
                        Scene->Camera.Rotation.Y -= 5*(double)Scene->dt;
                }
                if (Scene->Keymap['r'])
                {
                        Scene->Camera.Rotation.X -= 5*(double)Scene->dt;
                }
                if (Scene->Keymap['f'])
                {
                        Scene->Camera.Rotation.X += 5*(double)Scene->dt;
                }
                if (Scene->Keymap['q'])
                {
                        Scene->Camera.Velocity.Y -= 1.0;
                }
                if (Scene->Keymap['e'])
                {
                        Scene->Camera.Velocity.Y += 1.0;
                }

                /* Physics */
                Scene->Camera.Velocity.X *= 0.9;
                Scene->Camera.Velocity.Y *= 0.9;
                Scene->Camera.Velocity.Z *= 0.9;
//
                //if (fabs(Scene->Camera.Velocity.Y) > TERMINAL_VELOCITY)
                //{
                //        if (Scene->Camera.Velocity.Y < 0)
                //                Scene->Camera.Velocity.Y = -TERMINAL_VELOCITY;
                //        else
                //                Scene->Camera.Velocity.Y = TERMINAL_VELOCITY;
                //}
//
                //collided = false;
                Scene->Camera.Position.X += Scene->Camera.Velocity.X * Scene->dt;
                Scene->Camera.Position.Y += Scene->Camera.Velocity.Y * Scene->dt;
                Scene->Camera.Position.Z += Scene->Camera.Velocity.Z * Scene->dt;
//
                //while (PlayerCollides(Scene, Mesh->tris))
                //{
                //        Scene->Camera.Position.Y += 0.01;
                //        Scene->Camera.Velocity.Y = 0;
                //        collided = true;
                //}

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
