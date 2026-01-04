#include <engine/Scene.h>
#include <engine/draw.h>
#include <engine/sound.h>
#include <engine/camera.h>
#include <engine/mesh.h>
#include <todo.h>

static bool initialised = false;

static void Init(void)
{
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
                fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
                exit(EXIT_FAILURE);
                TODO();
        }

        initialised = true;
}

SCENE *SceneInit(const char *Title, int X, int Y, int W, int H)
{
        SCENE *Scene = calloc(1, sizeof(*Scene));
        if (!Title || W <= 0 || H <= 0 || X < 0 || Y < 0 || !Scene)
        {
                if (Scene)
                        free(Scene);
                TODO();
        }
        else if (!initialised)
        {
                Init();
        }

        Scene->Window.Window = SDL_CreateWindow(Title, X, Y, W, H, SDL_WINDOW_SHOWN);
        Scene->Window.WindowHeight = H;
        Scene->Window.WindowWidth = W;

        Scene->Renderer.Renderer = SDL_CreateRenderer(Scene->Window.Window, -1, SDL_RENDERER_SOFTWARE);
        Scene->Renderer.RendererWidth = W;
        Scene->Renderer.RendererHeight = H;

        Scene->CurrentColor.r = 255;
        Scene->CurrentColor.g = 255;
        Scene->CurrentColor.b = 255;

        Scene->Camera.FOV = 70;
        Scene->Camera.Aspect = (double)W / (double)H;
        Scene->Camera.Position.X = 0.0;
        Scene->Camera.Position.Y = 0.0;
        Scene->Camera.Position.Z = 0.0;
        Scene->Camera.Rotation.X = 0.0;
        Scene->Camera.Rotation.Y = 0.0;
        Scene->Camera.Rotation.Z = 0.0;
        Scene->Camera.Near = 1.0;
        Scene->Camera.Far = 1000.0;

        Scene->footstep_interval = 0.3;
        Scene->footstep_timer = 0.00;
        Scene->Renderer.ZBuffer = calloc(H, W * sizeof(*Scene->Renderer.ZBuffer));
        Scene->Renderer.RGBBuffer = calloc(H, W * sizeof(*Scene->Renderer.RGBBuffer));
        if (!Scene->Renderer.ZBuffer || !Scene->Renderer.RGBBuffer)
                TODO();

        Scene->LightPos.X = 0.0;
        Scene->LightPos.Y = 0.0;
        Scene->LightPos.Z = 1.0;
        return Scene;
}

void SceneTick(SCENE **Scene)
{
        SDL_Event e;
        if (!Scene || !(*Scene))
                return;
        while (SDL_PollEvent(&e))
        {
                switch (e.type)
                {
                case SDL_MOUSEMOTION:
                        break;
                case SDL_KEYDOWN:
                        if (e.key.keysym.sym < 255)
                        {
                                if (!(*Scene)->Keymap[e.key.keysym.sym])
                                        (*Scene)->JustPressed[e.key.keysym.sym] = true;
                                else
                                        (*Scene)->JustPressed[e.key.keysym.sym] = false;
                                (*Scene)->Keymap[e.key.keysym.sym] = true;

                        }
                        break;
                case SDL_KEYUP:
                        if (e.key.keysym.sym < 255)
                        {
                                (*Scene)->JustPressed[e.key.keysym.sym] = false;
                                (*Scene)->Keymap[e.key.keysym.sym] = false;
                        }
                        break;
                case SDL_QUIT:
                        CleanupSound(*Scene);
                        SceneEnd(*Scene, false);
                        (*Scene) = NULL;
                        return;
                default:
                        break;
                }
        }
}

void SceneClear(SCENE *Scene)
{
        for (size_t i = 0; i < Scene->count; ++i)
        {
                DelMesh(Scene->items[i]);
        }

        free(Scene->items);
        Scene->items = NULL;
        Scene->capacity = 0;
        Scene->count = 0;
}

void SceneEnd(SCENE *Scene, bool Final)
{
        if (Scene)
        {
                if (Scene->capacity)
                        SceneClear(Scene);
                SDL_DestroyRenderer(Scene->Renderer.Renderer);
                SDL_DestroyWindow(Scene->Window.Window);
                free(Scene->Renderer.ZBuffer);
                free(Scene->Renderer.RGBBuffer);
                memset(Scene, 0, sizeof(*Scene));
                free(Scene);
        }
        if (Final)
        {
                SDL_Quit();
                initialised = false;
        }
        return;
}

size_t SceneTriCount(SCENE *Scene)
{
        if (!Scene)
                return 0;
        size_t tri_count = 0;
        for (size_t i = 0; i < Scene->count; ++i)
        {
                tri_count += Scene->items[i]->tri_count;
        }

        return tri_count;
}

void SceneClearBuffers(SCENE *Scene)
{
        if (!Scene)
                TODO();
        memset(Scene->Renderer.RGBBuffer, 0, Scene->Renderer.RendererWidth * Scene->Renderer.RendererHeight * sizeof(*Scene->Renderer.RGBBuffer));
	for (size_t i = 0; i < (size_t)Scene->Renderer.RendererWidth * (size_t)Scene->Renderer.RendererHeight; ++i)
	{
		Scene->Renderer.ZBuffer[i] = -INFINITY;
	}
}
