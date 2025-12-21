#include <engine/draw.h>
#include <engine/scene.h>
#include <todo.h>

void DrawPixel( SCENE *Scene, int X, int Y )
{
        if (!Scene || !Scene->Window.Window || !Scene->Renderer.Renderer)
        {
                TODO();
        }

        SDL_SetRenderDrawColor(Scene->Renderer.Renderer, Scene->CurrentColor.r, Scene->CurrentColor.g, Scene->CurrentColor.b, 255);
        SDL_RenderDrawPoint(Scene->Renderer.Renderer, X, Y);
        return;
}
