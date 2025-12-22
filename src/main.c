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
        LoadMeshFromFile("mon guy.obj", Mesh);
        Mesh->ROTX = 180;
        Mesh->origin = (VEC3){0, 5, 25};
        while (Scene)
        {
                SceneTick(&Scene);
                if (!Scene)
                        break;
                if (Scene->Keymap['w'])
                {
                        Scene->Camera.Position.X -= 100 * cos((Scene->Camera.Rotation.Y + 90) * (M_PI / 180.0)) * Scene->dt;
                        Scene->Camera.Position.Z += 100 * sin((Scene->Camera.Rotation.Y + 90) * (M_PI / 180.0)) * Scene->dt;
                }
                if (Scene->Keymap['s'])
                {
                        Scene->Camera.Position.X += 100 * cos((Scene->Camera.Rotation.Y + 90) * (M_PI / 180.0)) * Scene->dt;
                        Scene->Camera.Position.Z -= 100 * sin((Scene->Camera.Rotation.Y + 90) * (M_PI / 180.0)) * Scene->dt;
                }
                if (Scene->Keymap['a'])
                {
                        Scene->Camera.Rotation.Y += 100.0 * (double)Scene->dt;
                }
                if (Scene->Keymap['d'])
                {
                        Scene->Camera.Rotation.Y -= 100.0 * (double)Scene->dt;
                }
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
        (void)Count, (void)Arguments;
        return (0);
}
