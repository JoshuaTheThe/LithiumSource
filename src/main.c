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

int main(int Count, char **Arguments)
{
        SCENE *Scene = SceneInit("Li3D", 0, 0, 800, 600);
        InitProjectionMat(Scene);
        SDL_RenderSetLogicalSize(Scene->Renderer.Renderer, 400, 300);
        Scene->Renderer.RendererHeight /= 2;
        Scene->Renderer.RendererWidth /= 2;
        Mesh3D *Mesh = InitMesh(0);
        LoadMeshFromFile("mario.obj", Mesh);
        Mesh->ROTX = 0;
        Mesh->origin = (VEC3){0, -5, 0};
        
        while (Scene)
        {
                SceneTick(&Scene);
                if (!Scene)
                        break;
                double cameraYawRad = (M_PI / 180.0) * Scene->Camera.Rotation.Y;
                
                if (Scene->Keymap['w'])
                {
                        Scene->Camera.Position.X += sin(-cameraYawRad) * Scene->dt;
                        Scene->Camera.Position.Z += cos(-cameraYawRad) * Scene->dt;
                }
                if (Scene->Keymap['s'])
                {
                        Scene->Camera.Position.X -= sin(-cameraYawRad) * Scene->dt;
                        Scene->Camera.Position.Z -= cos(-cameraYawRad) * Scene->dt;
                }
                if (Scene->Keymap['a'])
                {
                        Scene->Camera.Position.X -= sin(-cameraYawRad + DEG_TO_RAD(90)) * Scene->dt;
                        Scene->Camera.Position.Z -= cos(-cameraYawRad + DEG_TO_RAD(90)) * Scene->dt;
                }
                if (Scene->Keymap['d'])
                {
                        Scene->Camera.Position.X += sin(-cameraYawRad + DEG_TO_RAD(90)) * Scene->dt;
                        Scene->Camera.Position.Z += cos(-cameraYawRad + DEG_TO_RAD(90)) * Scene->dt;
                }
                if (Scene->Keymap['z'])
                {
                        Scene->Camera.Rotation.Y += (double)Scene->dt;
                }
                if (Scene->Keymap['x'])
                {
                        Scene->Camera.Rotation.Y -= (double)Scene->dt;
                }
                if (Scene->Keymap['q'])
                {
                        Scene->Camera.Position.Y -= (double)Scene->dt;
                }
                if (Scene->Keymap['e'])
                {
                        Scene->Camera.Position.Y += (double)Scene->dt;
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
