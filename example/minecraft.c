/**
 * Simple Voxel Terrain Generation.
 * instead of using chunks, i just used per block.
 * this is the first time in a while for me using pthreads
 * so it may not be the best.
 *
 * Basically just an adapted version of terrain.c
 * i would reccomend only useing this while flying, as physics and lithium in general;
 * are not very well optimised.
 */

#include <engine/lithium.h>
#include <pthread.h>

typedef enum
{
        STATE_AIR,
        STATE_STONE,
        STATE_DIRT,
} BLOCK;

#define X_OFFSET ((double)(100000.0))
#define Y_OFFSET ((double)(100000.0))

#define X_SCALE (32)
#define Y_SCALE (32)

#define SCALE (0.01)

#define DIST_X (32)
#define DIST_Y (32)
#define DIST_Z (32)

pthread_mutex_t terrainLock = PTHREAD_MUTEX_INITIALIZER;
volatile int changed = 0;

typedef struct
{
        SCENE *scene;
        size_t objid;
        int running;
        ENTITY *Back;
} TerrainThreadArgs;

double noise2D(double x, double y, int seed)
{
        int n = (int)x * 49632 + (int)y * 325176 + seed;
        n = (n << 13) ^ n;
        n = (n * (n * n * 15731 + 789221) + 1376312589);

        return 1.0f - ((n & 0x7fffffff) / 1073741824.0f);
}

double lerp(double a, double b, double t)
{
        return a + t * (b - a);
}

double cerp(double a, double b, double t)
{
        double ft = t * 3.1415927f;
        double f = (1.0f - cos(ft)) * 0.5f;
        return a * (1.0f - f) + b * f;
}

double smoothNoise2D(double x, double y, int seed)
{
        int intX = (int)x;
        int intY = (int)y;
        double fracX = x - intX;
        double fracY = y - intY;

        double v1 = noise2D(intX, intY, seed);
        double v2 = noise2D(intX + 1, intY, seed);
        double v3 = noise2D(intX, intY + 1, seed);
        double v4 = noise2D(intX + 1, intY + 1, seed);

        double i1 = cerp(v1, v2, fracX);
        double i2 = cerp(v3, v4, fracX);

        return cerp(i1, i2, fracY);
}

double fBm(double x, double y, int octaves, double persistence, int seed)
{
        double total = 0.0f;
        double frequency = 1.0f;
        double amplitude = 1.0f;
        double maxValue = 0.0f;

        for (int i = 0; i < octaves; i++)
        {
                total += smoothNoise2D(x * frequency, y * frequency, seed + i) * amplitude;
                maxValue += amplitude;
                amplitude *= persistence;
                frequency *= 2.0f;
        }

        return total / maxValue;
}

double genHeight(double x, double z, int seed)
{
        double nx = x * SCALE;
        double nz = z * SCALE;

        double height = fBm(nx, nz, 4, 0.5f, seed);

        double mountainNoise = fBm(nx * 2.0f, nz * 2.0f, 2, 0.7f, seed + 1000);
        height += mountainNoise * 0.3f;

        double ridgeNoise = fBm(nx * 0.5f, nz * 0.5f, 3, 0.6f, seed + 2000);
        height = height * 0.7f + ridgeNoise * 0.3f;

        height = fmaxf(0.0f, fminf(1.0f, height));

        return height;
}

int BRand(int x, int y, int z, int seed)
{
        unsigned int hash = (unsigned int)(x * 73856093 ^ y * 19349663 ^ z * 83492791) + seed * x * z;
        return (int)hash;
}

BLOCK GetBlockAt(long x, long y, long z, int seed)
{
        int ran = BRand(x, y, z, seed);
        double terrain_height = genHeight(x + X_OFFSET, z + Y_OFFSET, seed) * 100.0;
        if (y <= terrain_height + 1 && y >= terrain_height - 1)
        {
                return STATE_DIRT;
        }
        else if (y < terrain_height)
        {
                return STATE_STONE;
        }
        else
        {
                return STATE_AIR;
        }
}

COLOUR GetBlockColour(BLOCK Block, size_t s)
{
        switch (Block)
        {
        default:
                return (COLOUR){.r = 255, .g = 0, .b = 255, .a = 255};
        case STATE_AIR:
                return (COLOUR){.r = 0, .g = 0, .b = 0, .a = 0};
        case STATE_DIRT:
                switch (s)
                {
                case 2:
                        return (COLOUR){.r = 0, .g = 64, .b = 0, .a = 255};
                default:
                        return (COLOUR){.r = 128, .g = 64, .b = 0, .a = 255};
                }
        case STATE_STONE:
                return (COLOUR){.r = 64, .g = 64, .b = 64, .a = 255};
        }

        return (COLOUR){.r = 0, .g = 0, .b = 0, .a = 0};
}

int IsFaceVisible(long x, long y, long z, int seed, int dx, int dy, int dz)
{
        BLOCK neighbor = GetBlockAt(x + dx, y + dy, z + dz, seed);
        return neighbor == STATE_AIR;
}

void GenerateBlock(ENTITY *Map, size_t I, long long x, long long y, long long z, int seed)
{
        BLOCK BlockState = GetBlockAt(x, y, z, seed);
        if (BlockState == STATE_AIR)
                return;

        COLOUR col0 = GetBlockColour(BlockState, 0);
        COLOUR col1 = GetBlockColour(BlockState, 1);
        COLOUR col2 = GetBlockColour(BlockState, 2);
        COLOUR col3 = GetBlockColour(BlockState, 3);
        COLOUR col4 = GetBlockColour(BlockState, 4);
        COLOUR col5 = GetBlockColour(BlockState, 5);

#define ADD_FACE(dx, dy, dz, x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3, c) \
        if (IsFaceVisible(x, y, z, seed, dx, dy, dz))                           \
        {                                                                       \
                Map->Tris[I + 0].col = c;                                       \
                Map->Tris[I + 0].p[0] = (VEC3){x0, y0, z0};                     \
                Map->Tris[I + 0].p[1] = (VEC3){x1, y1, z1};                     \
                Map->Tris[I + 0].p[2] = (VEC3){x2, y2, z2};                     \
                Map->Tris[I + 1].col = c;                                       \
                Map->Tris[I + 1].p[0] = (VEC3){x0, y0, z0};                     \
                Map->Tris[I + 1].p[1] = (VEC3){x2, y2, z2};                     \
                Map->Tris[I + 1].p[2] = (VEC3){x3, y3, z3};                     \
                I += 2;                                                         \
        }

        ADD_FACE(1, 0, 0,
                 x + 1, y, z, x + 1, y + 1, z, x + 1, y + 1, z + 1, x + 1, y, z + 1, col0)
        ADD_FACE(-1, 0, 0,
                 x, y, z, x, y + 1, z, x, y + 1, z + 1, x, y, z + 1, col1)
        ADD_FACE(0, 1, 0,
                 x, y + 1, z, x + 1, y + 1, z, x + 1, y + 1, z + 1, x, y + 1, z + 1, col2)
        ADD_FACE(0, -1, 0,
                 x, y, z, x + 1, y, z, x + 1, y, z + 1, x, y, z + 1, col3)
        ADD_FACE(0, 0, 1,
                 x, y, z + 1, x + 1, y, z + 1, x + 1, y + 1, z + 1, x, y + 1, z + 1, col4)
        ADD_FACE(0, 0, -1,
                 x, y, z, x + 1, y, z, x + 1, y + 1, z, x, y + 1, z, col5)
#undef ADD_FACE
}

size_t FacesNeeded(long long x, long long y, long long z, int seed)
{
        size_t faces = 0;
        faces += IsFaceVisible(x, y, z, seed, 1, 0, 0);
        faces += IsFaceVisible(x, y, z, seed, 0, 1, 0);
        faces += IsFaceVisible(x, y, z, seed, 0, 0, 1);
        faces += IsFaceVisible(x, y, z, seed, -1, 0, 0);
        faces += IsFaceVisible(x, y, z, seed, 0, -1, 0);
        faces += IsFaceVisible(x, y, z, seed, 0, 0, -1);
        return faces;
}

void GenerateVisibleData(SCENE *Scene, ENTITY *Map)
{
        if (!Map)
                return;
        /* Find total faces required */
        long long playerX = (long long)(Scene->Player.Position.X);
        long long playerY = (long long)(Scene->Player.Position.Y);
        long long playerZ = (long long)(Scene->Player.Position.Z);
        long long originX = (long long)Scene->Player.Position.X - DIST_X / 2;
        long long originY = (long long)Scene->Player.Position.Y - DIST_Y / 2;
        long long originZ = (long long)Scene->Player.Position.Z - DIST_Z / 2;

        size_t faces = 0;
        for (int z = 0; z < DIST_Z; ++z)
                for (int y = 0; y < DIST_Y; ++y)
                        for (int x = 0; x < DIST_X; ++x)
                        {
                                long long _x = originX + x;
                                long long _y = originY + y;
                                long long _z = originZ + z;
                                faces += FacesNeeded(_x, _y, _z, 12345);
                        }
        ENTITY *NMesh = InitMesh(Scene, faces * 2);
        Map->Tris = NMesh->Tris;
        Map->TriCount = NMesh->TriCount;
        NMesh->Tris = NULL;
        NMesh->TriCount = 0;
        free(NMesh);

        size_t i = 0;
        for (int z = 0; z < DIST_Z; ++z)
                for (int y = 0; y < DIST_Y; ++y)
                        for (int x = 0; x < DIST_X; ++x)
                        {
                                long long _x = originX + x;
                                long long _y = originY + y;
                                long long _z = originZ + z;
                                GenerateBlock(Map, i, _x, _y, _z, 12345);
                                i += FacesNeeded(_x, _y, _z, 12345) * 2;
                        }
}

void *TerrainThreadFunc(void *arg)
{
        TerrainThreadArgs *args = (TerrainThreadArgs *)arg;

        while (args->running)
        {
                GenerateVisibleData(args->scene, args->Back);
                changed = 1;
                usleep(1000);
        }

        return NULL;
}

int main(int Count, char **Arguments)
{
        SCENE *Scene = LithiumInit(Count, Arguments);
        Scene->SoundSys.PrimaryJumpSound = LoadSound(Scene, "assets/valve/jump.wav");
        Scene->SoundSys.DenySelectSound = LoadSound(Scene, "assets/valve/denyselect.wav");
        Scene->SoundSys.PrimaryStepSounds[0] = LoadSound(Scene, "assets/valve/step1.wav");
        Scene->SoundSys.PrimaryStepSounds[1] = LoadSound(Scene, "assets/valve/step2.wav");
        Scene->SoundSys.PrimaryStepSounds[2] = LoadSound(Scene, "assets/valve/step3.wav");
        Scene->SoundSys.PrimaryStepSounds[3] = LoadSound(Scene, "assets/valve/step4.wav");

        ENTITY *Back = InitMesh(Scene, 0);
        ENTITY *Object = InitMesh(Scene, 0);
        Object->IsStatic = true;
        Object->IsInteractable = false;
        size_t ObjectId = Scene->count;
        da_append(Scene, Object);
        Scene->Player.Position.Y = genHeight(+X_OFFSET, +Y_OFFSET, 12345) * 100.0 + 5.0;

        TerrainThreadArgs terrainArgs = {
            .scene = Scene,
            .objid = ObjectId,
            .running = 1,
            .Back = Back};
        pthread_t terrainThread;
        pthread_create(&terrainThread, NULL, TerrainThreadFunc, &terrainArgs);

        while (Scene->Running)
        {
                pthread_mutex_lock(&terrainLock);
                if (changed)
                {
                        free(Object->Tris);
                        Object->Tris = Back->Tris;
                        Object->TriCount = Back->TriCount;
                        changed = 0;
                }

                pthread_mutex_unlock(&terrainLock);
                SceneTick(&Scene);
                LithiumUpdate(Scene);
                DrawScene(Scene);
        }

        terrainArgs.running = 0;
        pthread_join(terrainThread, NULL);
        DelMesh(Back);
        LithiumEnd(Scene);
}
