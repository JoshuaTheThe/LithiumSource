#include <engine/Scene.h>
#include <engine/draw.h>
#include <engine/camera.h>
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
        bool old_init = initialised;
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

        Scene->Camera.FOV = 90;
        Scene->Camera.Aspect = H / W;
        Scene->Camera.Position.X = 0.0;
        Scene->Camera.Position.Y = 0.0;
        Scene->Camera.Position.Z = 0.0;
        Scene->Camera.Rotation.X = 0.0;
        Scene->Camera.Rotation.Y = 0.0;
        Scene->Camera.Rotation.Z = 0.0;
        Scene->Camera.Near = 0.01;
        Scene->Camera.Far = 1000.0;
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
                        (*Scene)->Keymap[e.key.keysym.sym] = true;
                        printf("%c", (*Scene)->Keymap[e.key.keysym.sym]);
                        break;
                case SDL_KEYUP:
                        (*Scene)->Keymap[e.key.keysym.sym] = false;
                        break;
                case SDL_QUIT:
                        SceneEnd(*Scene, false);
                        (*Scene) = NULL;
                        return;
                default:
                        break;
                }
        }
}

void SceneEnd(SCENE *Scene, bool Final)
{
        if (Scene)
        {
                SDL_DestroyRenderer(Scene->Renderer.Renderer);
                SDL_DestroyWindow(Scene->Window.Window);
                free(Scene);
        }
        if (Final)
        {
                SDL_Quit();
                initialised = false;
        }
        return;
}
