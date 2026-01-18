#include <engine/lithium.h>

#define X_OFFSET ((double)(100000.0))
#define Y_OFFSET ((double)(100000.0))

#define X_SCALE (32)
#define Y_SCALE (32)

#define SCALE (0.01)

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

int main(int Count, char **Arguments)
{
        SCENE *Scene = LithiumInit(Count, Arguments);
        Scene->SoundSys.PrimaryJumpSound = LoadSound(Scene, "assets/jump.wav");
        Scene->SoundSys.DenySelectSound = LoadSound(Scene, "assets/denyselect.wav");
        Scene->SoundSys.PrimaryStepSounds[0] = LoadSound(Scene, "assets/walk_0.wav");
        Scene->SoundSys.PrimaryStepSounds[1] = LoadSound(Scene, "assets/walk_1.wav");
        Scene->SoundSys.PrimaryStepSounds[2] = LoadSound(Scene, "assets/walk_2.wav");
        Scene->SoundSys.PrimaryStepSounds[3] = LoadSound(Scene, "assets/walk_3.wav");

        ENT Map = LithiumLoadObject(Scene, "assets/plane sub25.obj"); /* 25 subdivisions */
        ENT Water = LithiumLoadObject(Scene, "assets/plane.obj");
        ScaleMesh(LiObj(Scene, Map), 50.0);
        ScaleMesh(LiObj(Scene, Water), 50.0);
        ENTITY *EMap = LiObj(Scene, Map);
        ENTITY *EWat = LiObj(Scene, Water);

        ENTITY *Sci = LiObj(Scene, LithiumCreateNPC(Scene, "assets/scientist.obj"));

        EMap->IsStatic = true;
        EWat->IsStatic = true;
        EMap->IsInteractable = true;

        const int seed = 12345;
        const double heightMultiplier = 100.0;

        COLOUR Ground = {.a = 255, .r = 32, .g = 255, .b = 32};
        double lowest_point = 0.0, highest_point = 0.0;
        for (size_t i = 0; i < EMap->TriCount; ++i)
        {
                TRI3D *Tri = &EMap->Tris[i];
                Tri->col = Ground;
                Tri->p[0].Y = genHeight(Tri->p[0].X + X_OFFSET, Tri->p[0].Z + Y_OFFSET, seed) * heightMultiplier;
                Tri->p[1].Y = genHeight(Tri->p[1].X + X_OFFSET, Tri->p[1].Z + Y_OFFSET, seed) * heightMultiplier;
                Tri->p[2].Y = genHeight(Tri->p[2].X + X_OFFSET, Tri->p[2].Z + Y_OFFSET, seed) * heightMultiplier;
                for (size_t j = 0; j < 3; ++j)
                        if (Tri->p[j].Y < lowest_point)
                                lowest_point = Tri->p[j].Y;
                        else if (Tri->p[j].Y > highest_point)
                                highest_point = Tri->p[j].Y;
        }

        double height = highest_point - lowest_point;

        for (size_t i = 0; i < EWat->TriCount; ++i)
        {
                TRI3D *Tri = &EWat->Tris[i];
                Tri->p[0].Y = -highest_point + height / 2;
                Tri->p[1].Y = -highest_point + height / 2;
                Tri->p[2].Y = -highest_point + height / 2;
                Tri->col.r = 0;
                Tri->col.g = 0;
                Tri->col.b = 127;
                Tri->col.a = 127;
        }

        for (size_t i = 0; i < EMap->TriCount; ++i)
        {
                TRI3D *Tri = &EMap->Tris[i];

                double avgY =
                    (Tri->p[0].Y + Tri->p[1].Y + Tri->p[2].Y) / 3.0;

                double normalised =
                    (avgY - lowest_point) / height;
                double r, g, b, or, og, ob;
                or = ((double)Tri->col.r) / 255.0;
                og = ((double)Tri->col.g) / 255.0;
                ob = ((double)Tri->col.b) / 255.0;

                Tri->p[0].Y -= highest_point;
                Tri->p[1].Y -= highest_point;
                Tri->p[2].Y -= highest_point;

                if (normalised < 0.0)
                        normalised = 0.0;
                if (normalised > 1.0)
                        normalised = 1.0;
                if (normalised < 0.5)
                {
                        or = 128.0 / 255.0;
                        og = 128.0 / 255.0;
                        ob = 0.0;
                        normalised = (avgY - lowest_point) / (height / 2);
                        if (normalised < 0.0)
                                normalised = 0.0;
                        if (normalised > 1.0)
                                normalised = 1.0;
                }
                else if (normalised < 0.8)
                {
                        /* Do Nothing */
                }
                else if (normalised < 0.9)
                {
                        or = 128.0 / 255.0;
                        og = 128.0 / 255.0;
                        ob = 128.0 / 255.0;
                        normalised = exp(normalised);
                        if (normalised < 0.0)
                                normalised = 0.0;
                        if (normalised > 0.6)
                                normalised = 0.6;
                }
                else
                {
                        or = 200.0 / 255.0;
                        og = 200.0 / 255.0;
                        ob = 200.0 / 255.0;
                        normalised = exp(normalised);
                        if (normalised < 0.0)
                                normalised = 0.0;
                        if (normalised > 1.0)
                                normalised = 1.0;
                }

                r = (normalised * or) * 255.0;
                g = (normalised * og) * 255.0;
                b = (normalised * ob) * 255.0;
                Tri->col.r = r;
                Tri->col.g = g;
                Tri->col.b = b;
        }

        EMap->InteractionBounds.Min = FindMeshMin(EMap);
        EMap->InteractionBounds.Max = FindMeshMax(EMap);

        while (Scene->Running)
        {
                SceneTick(&Scene);
                LithiumUpdate(Scene);
                DrawScene(Scene);
        }

        LithiumEnd(Scene);
}
