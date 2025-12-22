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
        LoadMeshFromFile("c0a0.obj", Mesh);
        Mesh->ROTX = -90;
        Mesh->origin = (VEC3){0, 0, 0};
        Mesh->Scale.X = 1.0;
        Mesh->Scale.Y = 1.0;
        Mesh->Scale.Z = 1.0;
        
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
                        Scene->Camera.Rotation.Y += 5*(double)Scene->dt;
                }
                if (Scene->Keymap['x'])
                {
                        Scene->Camera.Rotation.Y -= 5*(double)Scene->dt;
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
