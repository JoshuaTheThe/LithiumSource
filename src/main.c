#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>
#include<stdint.h>
#include<stddef.h>
#include<engine/camera.h>
#include<engine/draw.h>
#include<engine/scene.h>

int main( int Count, char **Arguments )
{
        SCENE *Scene = SceneInit("Li3D", 0, 0, 800, 600);
        Mesh3D *Mesh = InitMesh(0);
        Scene->Camera.FOV = 60;
        LoadMeshFromFile("skull.obj", Mesh);
        Mesh->ROTX = 180;
        Mesh->origin = (VEC3){0, 0, 5};
        while(Scene)
        {
                SDL_SetRenderDrawColor(Scene->Renderer.Renderer, 0, 0, 0, 255);
                SDL_RenderClear(Scene->Renderer.Renderer);
                DrawObject(Mesh, Scene);
                SDL_RenderPresent(Scene->Renderer.Renderer);
                if (Scene->Keymap['w'])
                {
                        Scene->Camera.Rotation.X += 100.0 * (double)Scene->dt;
                }
                if (Scene->Keymap['s'])
                {
                        Scene->Camera.Rotation.X -= 100.0 * (double)Scene->dt;
                }
                SceneTick(&Scene);
        }

        SceneEnd(NULL, true);
        DelMesh(Mesh);
        (void)Count, (void)Arguments;
        return(0);
}
