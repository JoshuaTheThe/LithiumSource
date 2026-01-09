#include <engine/Scene.h>
#include <engine/draw.h>
#include <engine/sound.h>
#include <engine/camera.h>
#include <engine/mesh.h>
#include <engine/ui.h>
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

SCENE *SceneInit(const char *Title, int X, int Y, int W, int H, int SCALE)
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

        Scene->Window.Window = SDL_CreateWindow(Title, X, Y, W * SCALE, H * SCALE, SDL_WINDOW_SHOWN);
        Scene->Window.WindowHeight = H;
        Scene->Window.WindowWidth = W;

        Scene->Renderer.Renderer = SDL_CreateRenderer(Scene->Window.Window, -1, SDL_RENDERER_ACCELERATED);
        Scene->Renderer.RendererWidth = W;
        Scene->Renderer.RendererHeight = H;
        SDL_RenderSetLogicalSize(Scene->Renderer.Renderer, W, H);
        Scene->Renderer.Texture = SDL_CreateTexture(Scene->Renderer.Renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, W, H);
        Scene->Player.FOV = 70;
        Scene->Player.Aspect = (double)W / (double)H;
        Scene->Player.Position.X = 0.0;
        Scene->Player.Position.Y = 0.0;
        Scene->Player.Position.Z = 0.0;
        Scene->Player.Rotation.X = 0.0;
        Scene->Player.Rotation.Y = 0.0;
        Scene->Player.Rotation.Z = 0.0;
        Scene->Player.Near = 0.1;
        Scene->Player.Far = 1000.0;

        Scene->SoundSys.DenySelectSound = -1;
        Scene->SoundSys.PrimaryJumpSound = -1;
        Scene->SoundSys.PrimaryStepSounds[0] = -1;
        Scene->SoundSys.PrimaryStepSounds[1] = -1;
        Scene->SoundSys.PrimaryStepSounds[2] = -1;
        Scene->SoundSys.PrimaryStepSounds[3] = -1;
        Scene->SoundSys.FootStepInterval = 0.3;
        Scene->SoundSys.FootStepTimer = 0.00;

        Scene->Renderer.ZBuffer = calloc(H, W * sizeof(*Scene->Renderer.ZBuffer));
        Scene->Renderer.RGBBuffer = calloc(H, W * sizeof(*Scene->Renderer.RGBBuffer));
        if (!Scene->Renderer.ZBuffer || !Scene->Renderer.RGBBuffer)
                TODO();

        Scene->Player.LightPos.X = 0.0;
        Scene->Player.LightPos.Y = 0.0;
        Scene->Player.LightPos.Z = 1.0;

        Scene->Player.Bounds.Max = (VEC3){.X = 0.254, .Y = 0.254, .Z = 0.254};
        Scene->Player.Bounds.Min = (VEC3){.X = -0.254, .Y = -1.5748, .Z = -0.254};
        Scene->Player.Velocity.Y = 0.1;
        Scene->Player.WalkSpeed = 0.635 / 2;
        Scene->Player.RunSpeed = 0.635;
        Scene->Player.Speed = Scene->Player.WalkSpeed;
        Scene->Player.RotSpeed = 90.0;
        Scene->Player.Flying = true;
        Scene->Player.IsSprinting = false;
        Scene->Player.IsCrouching = false;
        Scene->Player.StandingHeight = 0.254;
        Scene->Player.CrouchingHeight = (0.254 - 1.5748) / 3 - 0.254;
        Scene->Player.CameraOffsetY = 0.0;
        Scene->Player.CurrentHeight = Scene->Player.StandingHeight;

        Scene->Player.MaxInteraction = 2.0;
        Scene->Running = true;
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
                        (*Scene)->Running = false;
                        return;
                default:
                        break;
                }
        }

        (*Scene)->new = SDL_GetTicks();
        (*Scene)->dt = (double)((*Scene)->new - (*Scene)->old) / 1000.0;
        (*Scene)->old = (*Scene)->new;
}

void SceneClear(SCENE *Scene)
{
        for (size_t i = 0; i < Scene->count; ++i)
        {
                DelMesh(Scene->items[i]);
        }

        for (size_t i = 0; i < Scene->UXObjects.count; ++i)
        {
                LithiumCleanupUXObject(&Scene->UXObjects.items[i]);
        }

        free(Scene->items);
        Scene->items = NULL;
        Scene->capacity = 0;
        Scene->count = 0;
        Scene->UXObjects.capacity = 0;
        Scene->UXObjects.count = 0;
}

void SceneEnd(SCENE *Scene, bool Final)
{
        if (Scene)
        {
                if (Scene->capacity)
                        SceneClear(Scene);
                SDL_DestroyTexture(Scene->Renderer.Texture);
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
                tri_count += Scene->items[i]->TriCount;
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
