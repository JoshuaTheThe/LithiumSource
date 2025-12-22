/**
 * Scene->Camera.Position - Render 3D stuff
 * Courtesy of myself over a year ago for writing cool nice code, that i can reuse!
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <engine/camera.h>
#include <engine/types.h>
#include <engine/scene.h>
#include <todo.h>

static double min(double x, double y)
{
        return (x < y) ? x : y;
}

static Mat4x4 ProjectionMatrix = {{0}};

static void SDL_SWAP(int *a, int *b)
{
        int temp = *(a);
        *(a) = *(b);
        *(b) = temp;
}

VEC3 AddVec3(VEC3 *a, VEC3 *b)
{
        return (VEC3){a->X + b->X, a->Y + b->Y, a->Z + b->Z};
}

VEC3 SubVec3(VEC3 *a, VEC3 *b)
{
        return (VEC3){a->X - b->X, a->Y - b->Y, a->Z - b->Z};
}

VEC3 ScaleVec3Mul(VEC3 *a, double s)
{
        return (VEC3){a->X * s, a->Y * s, a->Z * s};
}

VEC3 ScaleVec3Div(VEC3 *a, double s)
{
        return (VEC3){a->X / s, a->Y / s, a->Z / s};
}

double DotVec3(VEC3 *a, VEC3 *b)
{
        return a->X * b->X + a->Y * b->Y + a->Z * b->Z;
}

double LenVec3(VEC3 *x)
{
        return sqrt(DotVec3(x, x));
}

VEC3 NormaliseVec3(VEC3 *a)
{
        double l = LenVec3(a);
        return (VEC3){a->X / l, a->Y / l, a->Z / l};
}

VEC3 CrossProdVec3(VEC3 *a, VEC3 *b)
{
        VEC3 v;
        v.X = a->Y * b->Z - a->Z * b->Y;
        v.Y = a->Z * b->X - a->X * b->Z;
        v.Z = a->X * b->Y - a->Y * b->X;
        return v;
}

void MulMatVec(VEC3 *i, VEC3 *o, Mat4x4 *m)
{
        o->X = i->X * m->m[0][0] + i->Y * m->m[1][0] + i->Z * m->m[2][0] + m->m[3][0];
        o->Y = i->X * m->m[0][1] + i->Y * m->m[1][1] + i->Z * m->m[2][1] + m->m[3][1];
        o->Z = i->X * m->m[0][2] + i->Y * m->m[1][2] + i->Z * m->m[2][2] + m->m[3][2];
        double w = i->X * m->m[0][3] + i->Y * m->m[1][3] + i->Z * m->m[2][3] + m->m[3][3];

        if (w != 0.0)
        {
                o->X /= w;
                o->Y /= w;
                o->Z /= w;
        }
}

Mat4x4 MatMakeIdent(void)
{
        Mat4x4 mat;
        mat.m[0][0] = 1.0;
        mat.m[1][1] = 1.0;
        mat.m[2][2] = 1.0;
        mat.m[3][3] = 1.0;
        return mat;
}

Mat4x4 MakeRotationX(double fAngleRad)
{
        Mat4x4 mat = {0};
        mat.m[0][0] = 1.0;
        mat.m[1][1] = cos(fAngleRad);
        mat.m[1][2] = sin(fAngleRad);
        mat.m[2][1] = -sin(fAngleRad);
        mat.m[2][2] = cos(fAngleRad);
        mat.m[3][3] = 1.0;
        return mat;
}

Mat4x4 MakeRotationY(double fAngleRad)
{
        Mat4x4 mat = {0};
        mat.m[0][0] = cos(fAngleRad);
        mat.m[0][2] = sin(fAngleRad);
        mat.m[2][0] = -sin(fAngleRad);
        mat.m[1][1] = 1.0;
        mat.m[2][2] = cos(fAngleRad);
        mat.m[3][3] = 1.0;
        return mat;
}

Mat4x4 MakeRotationZ(double fAngleRad)
{
        Mat4x4 mat = {0};
        mat.m[0][0] = cos(fAngleRad);
        mat.m[0][1] = sin(fAngleRad);
        mat.m[1][0] = -sin(fAngleRad);
        mat.m[1][1] = cos(fAngleRad);
        mat.m[2][2] = 1.0;
        mat.m[3][3] = 1.0;
        return mat;
}

Mat4x4 MakeTransMat(double x, double y, double z)
{
        Mat4x4 mat = {0};
        mat.m[0][0] = 1.0;
        mat.m[1][1] = 1.0;
        mat.m[2][2] = 1.0;
        mat.m[3][3] = 1.0;
        mat.m[0][3] = x;
        mat.m[1][3] = y;
        mat.m[2][3] = z;
        return mat;
}

Mat4x4 MakeProjMat(SCENE *Scene)
{
        Mat4x4 mat = {0};

        double fAspectRatio = (double)Scene->Renderer.RendererHeight / (double)Scene->Renderer.RendererWidth;
        double fFovRad = 1.0 / tan(Scene->Camera.FOV * 0.5f / 180.0 * M_PI);
        double far = Scene->Camera.Far;
        double near = Scene->Camera.Near;

        mat.m[0][0] = fAspectRatio * fFovRad;
        mat.m[1][1] = fFovRad;
        mat.m[2][2] = far / (far - near);
        mat.m[3][2] = (-far * near) / (far - near);
        mat.m[2][3] = 1.0;
        mat.m[3][3] = 0.0;
        return mat;
}

Mat4x4 MulMatMat(Mat4x4 *a, Mat4x4 *b)
{
        Mat4x4 o = {0};
        for (int c = 0; c < 4; ++c)
        {
                for (int r = 0; r < 4; ++r)
                {
                        o.m[r][c] = a->m[r][0] * b->m[0][c] + a->m[r][1] * b->m[1][c] + a->m[r][2] * b->m[2][c] + a->m[r][3] * b->m[3][c];
                }
        }
        return o;
}

Mesh3D *InitMesh(size_t triCount)
{
        Mesh3D *mesh = malloc(sizeof(Mesh3D));
        mesh->tris = malloc(sizeof(TRI3D) * triCount);
        mesh->tri_count = triCount;
        mesh->origin = (VEC3){0.0, 0.0, 0.0};
        return mesh;
}

void DelMesh(Mesh3D *mesh)
{
        free(mesh->tris);
        free(mesh);
}

bool LoadMeshFromFile(const char *fileName, Mesh3D *mesh)
{
        FILE *fp = fopen(fileName, "r");
        char line[128];
        size_t vertex_count = 0, face_count = 0;
        VEC3 *vertices = NULL;
        TRI3D *triangles = NULL;
        if (fp == NULL)
                return false;

        if (mesh->tris)
                free(mesh->tris);

        while (fgets(line, sizeof(line), fp))
        {
                if (line[0] == 'v' && line[1] == ' ')
                {
                        vertex_count++;
                }
                else if (line[0] == 'f' && line[1] == ' ')
                {
                        face_count++;
                }
        }

        vertices = (VEC3 *)calloc(vertex_count, sizeof(VEC3));
        triangles = (TRI3D *)calloc(face_count, sizeof(TRI3D));

        if (vertices == NULL || triangles == NULL)
        {
                free(vertices);
                free(triangles);
                fclose(fp);
                return false;
        }

        rewind(fp);
        size_t vertex_index = 0, face_index = 0;

        while (fgets(line, sizeof(line), fp))
        {
                if (line[0] == 'v' && line[1] == ' ')
                {
                        VEC3 v;
                        sscanf(line, "v %lf %lf %lf", &v.X, &v.Y, &v.Z);
                        vertices[vertex_index++] = v;
                }
                else if (line[0] == 'f' && line[1] == ' ')
                {
                        int v1, v2, v3;
                        sscanf(line, "f %d %d %d", &v1, &v2, &v3);
                        triangles[face_index].p[0] = vertices[v1 - 1];
                        triangles[face_index].p[1] = vertices[v2 - 1];
                        triangles[face_index].p[2] = vertices[v3 - 1];
                        face_index++;
                }
        }

        mesh->tris = triangles;
        mesh->tri_count = face_count;
        mesh->origin = (VEC3){0.0, 0.0, 0.0};

        free(vertices);
        fclose(fp);
        return true;
}

void InitProjectionMat(SCENE *Scene)
{
        double fAspectRatio = (double)Scene->Renderer.RendererHeight / (double)Scene->Renderer.RendererWidth;
        double fFovRad = 1.0 / tan(Scene->Camera.FOV * 0.5f / 180.0 * M_PI);
        double far = Scene->Camera.Far;
        double near = Scene->Camera.Near;

        ProjectionMatrix.m[0][0] = fAspectRatio * fFovRad;
        ProjectionMatrix.m[1][1] = fFovRad;
        ProjectionMatrix.m[2][2] = far / (far - near);
        ProjectionMatrix.m[3][2] = (-far * near) / (far - near);
        ProjectionMatrix.m[2][3] = 1.0;
        ProjectionMatrix.m[3][3] = 0.0;
}

void sortVertices(int *y0, int *y1, int *y2, int *x0, int *x1, int *x2)
{
        if (*y1 < *y0)
        {
                SDL_SWAP(y0, y1);
                SDL_SWAP(x0, x1);
        }
        if (*y2 < *y0)
        {
                SDL_SWAP(y0, y2);
                SDL_SWAP(x0, x2);
        }
        if (*y2 < *y1)
        {
                SDL_SWAP(y1, y2);
                SDL_SWAP(x1, x2);
        }
}

int interpolateX(int y1, int y2, int x1, int x2, int y)
{
        if (y1 == y2)
                return x1;
        return x1 + (x2 - x1) * (y - y1) / (y2 - y1);
}

void DrawTri(SCENE *Scene, int x0, int y0, int x1, int y1, int x2, int y2, int r, int g, int b)
{
        SDL_SetRenderDrawColor(Scene->Renderer.Renderer, r, g, b, 255);
        sortVertices(&y0, &y1, &y2, &x0, &x1, &x2);

        for (int y = y0; y <= y2; ++y)
        {
                int xLeft, xRight;

                if (y < y1)
                {
                        xLeft = interpolateX(y0, y2, x0, x2, y);
                        xRight = interpolateX(y0, y1, x0, x1, y);
                }
                else
                {
                        xLeft = interpolateX(y1, y2, x1, x2, y);
                        xRight = interpolateX(y0, y2, x0, x2, y);
                }

                if (xLeft > xRight)
                {
                        SDL_SWAP(&xLeft, &xRight);
                }

                for (int x = xLeft; x <= xRight; ++x)
                {
                        SDL_RenderDrawPoint(Scene->Renderer.Renderer, x, y);
                }
        }
}

void DrawTriWire(SCENE *Scene, int x0, int y0, int x1, int y1, int x2, int y2, int r, int g, int b)
{
        SDL_SetRenderDrawColor(Scene->Renderer.Renderer, r, g, b, 255);
        SDL_RenderDrawLine(Scene->Renderer.Renderer, x0, y0, x1, y1);
        SDL_RenderDrawLine(Scene->Renderer.Renderer, x1, y1, x2, y2);
        SDL_RenderDrawLine(Scene->Renderer.Renderer, x2, y2, x0, y0);
}

COLOUR GetCol(double lum)
{
        if (lum < -1)
        {
                lum = -1;
        }
        if (lum > 1)
        {
                lum = 1;
        }

        double mappedLum = (lum + 1) / 2.0;

        int R = (int)(255 * mappedLum);
        int G = (int)(255 * mappedLum);
        int B = (int)(255 * mappedLum);

        return (COLOUR){R, G, B};
}

double GetTriangleDepth(const TRI3D *tri)
{
        return (tri->p[0].Z + tri->p[1].Z + tri->p[2].Z) / 3.0;
}

int CompareTriangles(const void *a, const void *b)
{
        const TRI3D *triA = (const TRI3D *)a;
        const TRI3D *triB = (const TRI3D *)b;
        double depthA = GetTriangleDepth(triA);
        double depthB = GetTriangleDepth(triB);
        if (depthA > depthB)
                return -1;
        else if (depthA < depthB)
                return 1;
        else
                return 0;
}

size_t DrawObject(Mesh3D *Cube, SCENE *Scene)
{
        static size_t elapsedTime1 = 0;
        static size_t elapsedTime0 = 0;

        Mat4x4 RotMatrixX = MakeRotationX(DEG_TO_RAD(Cube->ROTX)),
               RotMatrixY = MakeRotationY(DEG_TO_RAD(Cube->ROTY)),
               RotMatrixZ = MakeRotationZ(DEG_TO_RAD(Cube->ROTZ));

        elapsedTime0 = elapsedTime1;
        elapsedTime1 = SDL_GetTicks();

        double elapsedTime = (elapsedTime1 - elapsedTime0) / (1000.0 / TIME_SCALE);
        if (elapsedTime0 == elapsedTime1)
        {
                elapsedTime = 0.0;
        }

        Cube->ROTY += 50.0 * (elapsedTime * 1);

        TRI3D *TrisToDraw = calloc(Cube->tri_count, sizeof(TRI3D));
        if (!TrisToDraw)
        {
                fprintf(stderr, "Failed to allocate memory for triangles\n");
                return elapsedTime1;
        }

        Mat4x4 ObjectRotation = MulMatMat(&RotMatrixZ, &RotMatrixY);
        ObjectRotation = MulMatMat(&ObjectRotation, &RotMatrixX);

        Mat4x4 CameraRotX = MakeRotationX(-DEG_TO_RAD(Scene->Camera.Rotation.X));
        Mat4x4 CameraRotY = MakeRotationY(-DEG_TO_RAD(Scene->Camera.Rotation.Y));
        Mat4x4 CameraRotZ = MakeRotationZ(-DEG_TO_RAD(Scene->Camera.Rotation.Z));

        Mat4x4 CameraRotation = MulMatMat(&CameraRotY, &CameraRotX);
        CameraRotation = MulMatMat(&CameraRotation, &CameraRotZ);

        Mat4x4 CameraTranslation = MakeTransMat(
            -Scene->Camera.Position.X,
            -Scene->Camera.Position.Y,
            -Scene->Camera.Position.Z);

        Mat4x4 ViewMatrix = MulMatMat(&CameraRotation, &CameraTranslation);

        for (size_t i = 0; i < Cube->tri_count; ++i)
        {
                TRI3D tri, triProjected, triWorld, triView;

                tri = Cube->tris[i];

                MulMatVec(&tri.p[0], &triWorld.p[0], &ObjectRotation);
                MulMatVec(&tri.p[1], &triWorld.p[1], &ObjectRotation);
                MulMatVec(&tri.p[2], &triWorld.p[2], &ObjectRotation);

                for (int j = 0; j < 3; ++j)
                {
                        triWorld.p[j].X += Cube->origin.X;
                        triWorld.p[j].Y += Cube->origin.Y;
                        triWorld.p[j].Z += Cube->origin.Z;
                }

                MulMatVec(&triWorld.p[0], &triView.p[0], &ViewMatrix);
                MulMatVec(&triWorld.p[1], &triView.p[1], &ViewMatrix);
                MulMatVec(&triWorld.p[2], &triView.p[2], &ViewMatrix);

                VEC3 Normal, Line, Line2;

                Line = SubVec3(&triView.p[1], &triView.p[0]);
                Line2 = SubVec3(&triView.p[2], &triView.p[0]);

                Normal = CrossProdVec3(&Line, &Line2);
                NormaliseVec3(&Normal);

                if (Normal.Z < 0.0)
                {
                        VEC3 LightDir = {0.0, 0.4, -1.0};
                        NormaliseVec3(&LightDir);

                        Line = SubVec3(&triWorld.p[1], &triWorld.p[0]);
                        Line2 = SubVec3(&triWorld.p[2], &triWorld.p[0]);
                        VEC3 WorldNormal = CrossProdVec3(&Line, &Line2);
                        NormaliseVec3(&WorldNormal);

                        double dp = DotVec3(&WorldNormal, &LightDir);
                        COLOUR col = GetCol(dp / 10);
                        triProjected.col = col;

                        MulMatVec(&triView.p[0], &triProjected.p[0], &ProjectionMatrix);
                        MulMatVec(&triView.p[1], &triProjected.p[1], &ProjectionMatrix);
                        MulMatVec(&triView.p[2], &triProjected.p[2], &ProjectionMatrix);

                        for (int j = 0; j < 3; ++j)
                        {
                                triProjected.p[j].X = (triProjected.p[j].X + 1.0) * 0.5 * Scene->Renderer.RendererWidth;
                                triProjected.p[j].Y = (triProjected.p[j].Y + 1.0) * 0.5 * Scene->Renderer.RendererHeight;
                        }

                        memcpy(&TrisToDraw[i], &triProjected, sizeof(TRI3D));
                }
        }

        qsort(TrisToDraw, Cube->tri_count, sizeof(TRI3D), CompareTriangles);

        for (size_t i = 0; i < Cube->tri_count; ++i)
        {
                TRI3D *triToDraw = &TrisToDraw[i];

                if (triToDraw->p[0].X == 0 && triToDraw->p[0].Y == 0 &&
                    triToDraw->p[1].X == 0 && triToDraw->p[1].Y == 0 &&
                    triToDraw->p[2].X == 0 && triToDraw->p[2].Y == 0)
                {
                        continue;
                }

                if (WIRE_FRAME)
                {
                        DrawTriWire(Scene,
                                    (int)triToDraw->p[0].X, (int)triToDraw->p[0].Y,
                                    (int)triToDraw->p[1].X, (int)triToDraw->p[1].Y,
                                    (int)triToDraw->p[2].X, (int)triToDraw->p[2].Y,
                                    triToDraw->col.r, triToDraw->col.g, triToDraw->col.b);
                }
                else
                {
                        DrawTri(Scene,
                                (int)triToDraw->p[0].X, (int)triToDraw->p[0].Y,
                                (int)triToDraw->p[1].X, (int)triToDraw->p[1].Y,
                                (int)triToDraw->p[2].X, (int)triToDraw->p[2].Y,
                                triToDraw->col.r, triToDraw->col.g, triToDraw->col.b);
                }
        }

        free(TrisToDraw);
        return elapsedTime1;
}

// Example from source of this code
// int main(int argc, char *argv[])
//{
//        InitProjectionMat();
//        Mesh3D *Cube = InitMesh(0);
//        Cube->ROTX = 180;
//
//        if (argc == 1)
//        {
//                LoadMeshFromFile("monitor.obj", Cube);
//        }
//        else
//        {
//                LoadMeshFromFile(argv[1], Cube);
//        }
//
//        Cube->origin = (VEC3){0, 0, 10};
//
//        // Initialize SDL
//        if (SDL_Init(SDL_INIT_VIDEO) < 0)
//        {
//                printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
//                return 1;
//        }
//
//        Scene->Window.Window = SDL_CreateWindow("3D Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
//                                  SCREEN_WIDTH * SCREEN_SCALE, SCREEN_HEIGHT * SCREEN_SCALE, SDL_WINDOW_SHOWN);
//        if (!Scene->Window.Window)
//        {
//                printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
//                SDL_Quit();
//                return 1;
//        }
//
//        Scene->Renderer.Renderer = SDL_CreateRenderer(Scene->Window.Window, -1, SDL_RENDERER_ACCELERATED);
//        if (!Scene->Renderer.Renderer)
//        {
//                printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
//                SDL_DestroyWindow(Scene->Window.Window);
//                SDL_Quit();
//                return 1;
//        }
//
//        SDL_RenderSetLogicalSize(Scene->Renderer.Renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
//
//        bool quit = false;
//        SDL_Event event;
//
//        while (!quit)
//        {
//                double startFrameTime = SDL_GetTicks();
//                while (SDL_PollEvent(&event) != 0)
//                {
//                        if (event.type == SDL_QUIT)
//                        {
//                                quit = true;
//                        }
//                }
//
//                HandleInput();
//
//                SDL_SetRenderDrawColor(Scene->Renderer.Renderer, 0, 0, 0, 255);
//                SDL_RenderClear(Scene->Renderer.Renderer);
//                DrawObject(Cube);
//                SDL_RenderPresent(Scene->Renderer.Renderer);
//
//                double frameTime = SDL_GetTicks() - startFrameTime;
//                if (frameTime < frameDelay)
//                {
//                        SDL_Delay(frameDelay - frameTime); // Delay to maintain the target FPS
//                }
//        }
//
//        SDL_DestroyRenderer(Scene->Renderer.Renderer);
//        SDL_DestroyWindow(Scene->Window.Window);
//        SDL_Quit();
//
//        DelMesh(Cube);
//        return 0;
//}
//
