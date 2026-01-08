#include <engine/lithium.h>

int compile(void);
char *ProgramPath = NULL;

SCENE *LithiumInit(int argc, char **argv)
{
        const int width = 300;
        const int height = 300;
        const int scale = 2;

        if (argc == 2 && !strncmp("--compile", argv[1], 10))
        {
                compile();
                exit(0);
                return NULL;
        }

        ProgramPath = strdup(argv[0]);
        if (!ProgramPath)
        {
                exit(0);
                return NULL;
        }
        ProgramPath = dirname(ProgramPath);

        int result = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512);
        if (result < 0)
        {
                fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
                exit(-1);
                return NULL;
        }

        Mix_AllocateChannels(16);

        SCENE *Scene = SceneInit("Li3D", 0, 0, width * scale, height * scale);
        InitProjectionMat(Scene);
        SDL_RenderSetLogicalSize(Scene->Renderer.Renderer, width, height);
        Scene->Renderer.RendererHeight /= scale;
        Scene->Renderer.RendererWidth /= scale;
        return Scene;
}

void LithiumEnd(SCENE *Scene)
{
        SceneEnd(Scene, false);
        SceneEnd(NULL, true);
        free(ProgramPath);
}

static void LithiumUpdateFlying(SCENE *Scene)
{
        if (Scene->Keymap['c'])
        {
                Scene->Player.Velocity.Y -= Scene->Player.Speed;
        }

        if (Scene->Keymap[' '])
        {
                Scene->Player.Velocity.Y += Scene->Player.Speed;
        }
}

static void LithiumUpdateGrounded(SCENE *Scene)
{
        if (Scene->Keymap[' '])
        {
                Scene->Player.Velocity.Y = GRAVITY * JUMP_POWER;
                PlaySound(Scene, Scene->SoundSys.PrimaryJumpSound);
        }

        if ((Scene->Keymap['w'] || Scene->Keymap['s'] ||
             Scene->Keymap['a'] || Scene->Keymap['d']) &&
            Scene->SoundSys.FootStepTimer <= 0.0)
        {
                size_t Idx = Scene->SoundSys.WalkCycle++;
                Scene->SoundSys.WalkCycle = (Scene->SoundSys.WalkCycle) % 4;
                PlaySound(Scene, Scene->SoundSys.PrimaryStepSounds[Idx]);
                Scene->SoundSys.FootStepTimer = Scene->SoundSys.FootStepInterval;
        }
}

static inline void LithiumPlayerUpdateHeight(PLAYER *player, double dt)
{
        double targetHeight = player->IsCrouching ? player->CrouchingHeight : player->StandingHeight;
        const double crouchSpeed = 10.0;
        double t = crouchSpeed * dt;
        t = t > 1.0 ? 1.0 : t;
        player->CurrentHeight = player->CurrentHeight * (1.0 - t) + targetHeight * t;
        player->CameraOffsetY = player->CurrentHeight - player->StandingHeight;
        player->Bounds.Max.Y = player->CurrentHeight;
}

void LithiumUpdate(SCENE *Scene)
{
        if (!Scene)
                return;

        const double yaw = DEG_TO_RAD(Scene->Player.Rotation.Y);
        const double pitch = DEG_TO_RAD(Scene->Player.Rotation.X);

        VEC3 forward = {
            sin(yaw),
            0.0,
            cos(yaw)};

        VEC3 right = {
            cos(yaw),
            0.0,
            -sin(yaw)};

        /* Physics */
        if (!Scene->Player.Flying)
                PhysicsTick(Scene);
        else
        {
                Scene->Player.Velocity.X -= Scene->Player.Velocity.X * FRICTION * Scene->dt;
                Scene->Player.Velocity.Y -= Scene->Player.Velocity.Y * FRICTION * Scene->dt;
                Scene->Player.Velocity.Z -= Scene->Player.Velocity.Z * FRICTION * Scene->dt;
                Scene->Player.Position.X += Scene->Player.Velocity.X * Scene->dt;
                Scene->Player.Position.Y += Scene->Player.Velocity.Y * Scene->dt;
                Scene->Player.Position.Z += Scene->Player.Velocity.Z * Scene->dt;
        }

        if (Scene->Keymap['q'] && (Scene->Player.Flying || Scene->Player.Grounded))
        {
                Scene->Player.Speed = Scene->Player.RunSpeed;
                Scene->Player.IsSprinting = true;
        }
        else
        {
                Scene->Player.Speed = Scene->Player.WalkSpeed;
                Scene->Player.IsSprinting = false;
        }
        if (Scene->Keymap['w'] || Scene->Keymap['s'])
        {
                double n = 1;
                if (Scene->Keymap['s'])
                        n = -1;
                Scene->Player.Velocity.X += Scene->Player.Speed * forward.X * n;
                Scene->Player.Velocity.Z += Scene->Player.Speed * forward.Z * n;
        }
        if (Scene->Keymap['a'] || Scene->Keymap['d'])
        {
                double n = 1;
                if (Scene->Keymap['a'])
                        n = -1;
                Scene->Player.Velocity.X += Scene->Player.Speed * right.X * n;
                Scene->Player.Velocity.Z += Scene->Player.Speed * right.Z * n;
        }
        if (Scene->Keymap['z'] || Scene->Keymap['x'])
        {
                double n = 1;
                if (Scene->Keymap['z'])
                        n = -1;
                Scene->Player.Rotation.Y += Scene->Player.RotSpeed * (double)Scene->dt * n;
        }
        if (Scene->Keymap['r'] || Scene->Keymap['f'])
        {
                double n = 1;
                if (Scene->Keymap['f'])
                        n = -1;
                Scene->Player.Rotation.X += Scene->Player.RotSpeed * (double)Scene->dt * n;
        }

        if (Scene->Player.Flying)
        {
                LithiumUpdateFlying(Scene);
        }
        else if (Scene->Keymap['c'])
        {
                if (!Scene->Player.IsCrouching)
                        Scene->Player.IsCrouching = true;
        }
        else if (Scene->Player.IsCrouching)
        {
                Scene->Player.IsCrouching = false;
        }
        else if (Scene->Player.Grounded)
        {
                LithiumUpdateGrounded(Scene);
        }

        if (Scene->Keymap['v'])
        {
                Scene->Player.Flying = true;
        }
        if (Scene->Keymap['n'])
        {
                Scene->Player.Flying = false;
        }

        if (Scene->JustPressed['e'])
        {
                RAY3D Ray;
                const VEC3 direction = {
                    cos(pitch) * sin(yaw),
                    sin(pitch),
                    cos(pitch) * cos(yaw)};
                Ray.InitialDir = NormaliseVec3(&direction);
                Ray.InitialPos = Scene->Player.Position;
                Mesh3D *Hit = CastRay(Scene, Ray);
                if (Hit && Hit->Interact)
                {
                        printf("INFO: Interacted with object at %p\n", Hit);
                        Hit->Interact(Hit);
                        PlaySound(Scene, Hit->InteractSound);
                }
                else
                {
                        PlaySound(Scene, Scene->SoundSys.DenySelectSound);
                }
                Scene->JustPressed['e'] = false;
        }

        LithiumPlayerUpdateHeight(&Scene->Player, Scene->dt);
        Scene->SoundSys.FootStepTimer -= Scene->dt;
        UpdateSounds(Scene);
}

static VEC3 FindMeshMin(Mesh3D *Mesh)
{
        VEC3 Min = {0};
        Min.X = Mesh->Tris[0].p[0].X * Mesh->Scale.X;
        Min.Y = Mesh->Tris[0].p[0].Y * Mesh->Scale.Y;
        Min.Z = Mesh->Tris[0].p[0].Z * Mesh->Scale.Z;
        for (size_t i = 0; i < Mesh->TriCount; ++i)
                for (size_t p = 0; p < 3; ++p)
                {
                        if (Mesh->Tris[i].p[p].X * Mesh->Scale.X < Min.X)
                                Min.X = Mesh->Tris[i].p[p].X;
                        if (Mesh->Tris[i].p[p].Y * Mesh->Scale.Y < Min.Y)
                                Min.Y = Mesh->Tris[i].p[p].Y;
                        if (Mesh->Tris[i].p[p].Z * Mesh->Scale.Z < Min.Z)
                                Min.Z = Mesh->Tris[i].p[p].Z;
                }
        return Min;
}

static VEC3 FindMeshMax(Mesh3D *Mesh)
{
        VEC3 Max = {0};
        Max.X = Mesh->Tris[0].p[0].X * Mesh->Scale.X;
        Max.Y = Mesh->Tris[0].p[0].Y * Mesh->Scale.Y;
        Max.Z = Mesh->Tris[0].p[0].Z * Mesh->Scale.Z;
        for (size_t i = 0; i < Mesh->TriCount; ++i)
                for (size_t p = 0; p < 3; ++p)
                {
                        if (Mesh->Tris[i].p[p].X * Mesh->Scale.X > Max.X)
                                Max.X = Mesh->Tris[i].p[p].X;
                        if (Mesh->Tris[i].p[p].Y * Mesh->Scale.Y > Max.Y)
                                Max.Y = Mesh->Tris[i].p[p].Y;
                        if (Mesh->Tris[i].p[p].Z * Mesh->Scale.Z > Max.Z)
                                Max.Z = Mesh->Tris[i].p[p].Z;
                }
        return Max;
}

size_t LithiumLoadObject(SCENE *Scene, char *Path)
{
        Mesh3D *Object = InitMesh(Scene, 0);
        LoadMeshFromFile(Path, Object);
        if (!Object || Object->TriCount == 0)
        {
                printf("ERROR: Could not load model from file '%s'\n", Path);
                return -1;
        }

        Object->InteractionBounds.Min = FindMeshMin(Object);
        Object->InteractionBounds.Max = FindMeshMax(Object);

        da_append(Scene, Object);
        return Scene->count - 1;
}

Mesh3D *LiObj(SCENE *Scene, size_t Index)
{
        if (Index >= Scene->count)
        {
                return NULL;
        }

        return Scene->items[Index];
}

void LithiumApplyTexture(SCENE *Scene, TEXTURE *Tex, size_t Index)
{
        if (Index >= Scene->count)
        {
                printf("ERROR: Given illegal object id\n");
                return;
        }

        Mesh3D *Object = Scene->items[Index];

        if (!Tex || !Object)
        {
                printf("ERROR: Given NULL Texture or Object, ignoring\n");
                return;
        }

        for (size_t i = 0; i < Object->TriCount; ++i)
        {
                Object->Tris[i].Texture = Tex;
        }
}
