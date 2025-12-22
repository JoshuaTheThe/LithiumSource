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

static Mat4x4 ProjectionMatrix = {{{0}}};

void SDL_SWAP(int *a, int *b)
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

Mat4x4 MakeScaleMat(float sx, float sy, float sz)
{
        Mat4x4 m = {0};
        m.m[0][0] = sx;
        m.m[1][1] = sy;
        m.m[2][2] = sz;
        m.m[3][3] = 1.0f;
        return m;
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
        mat.m[3][0] = x;
        mat.m[3][1] = y;
        mat.m[3][2] = z;
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

static Mat4x4 TransposeMat(const Mat4x4 *mat)
{
        Mat4x4 result;

        for (int i = 0; i < 4; i++)
        {
                for (int j = 0; j < 4; j++)
                {
                        result.m[i][j] = mat->m[j][i];
                }
        }

        return result;
}

static Mat4x4 InvertViewMatrix(const Mat4x4 *mat)
{
        // For a view matrix which is a rotation + translation matrix:
        // R is 3x3 rotation, T is translation
        // View = [R  T]
        //        [0  1]
        // Inverse = [R^-1  -R^-1 * T]
        //           [0      1       ]

        Mat4x4 inv;

        // Transpose the 3x3 rotation part (since R^-1 = R^T for rotation matrices)
        inv.m[0][0] = mat->m[0][0];
        inv.m[0][1] = mat->m[1][0];
        inv.m[0][2] = mat->m[2][0];
        inv.m[0][3] = 0.0;
        inv.m[1][0] = mat->m[0][1];
        inv.m[1][1] = mat->m[1][1];
        inv.m[1][2] = mat->m[2][1];
        inv.m[1][3] = 0.0;
        inv.m[2][0] = mat->m[0][2];
        inv.m[2][1] = mat->m[1][2];
        inv.m[2][2] = mat->m[2][2];
        inv.m[2][3] = 0.0;
        inv.m[3][0] = 0.0;
        inv.m[3][1] = 0.0;
        inv.m[3][2] = 0.0;
        inv.m[3][3] = 1.0;

        // Extract translation
        double tx = mat->m[3][0];
        double ty = mat->m[3][1];
        double tz = mat->m[3][2];

        // Calculate -R^T * T
        inv.m[3][0] = -(inv.m[0][0] * tx + inv.m[1][0] * ty + inv.m[2][0] * tz);
        inv.m[3][1] = -(inv.m[0][1] * tx + inv.m[1][1] * ty + inv.m[2][1] * tz);
        inv.m[3][2] = -(inv.m[0][2] * tx + inv.m[1][2] * ty + inv.m[2][2] * tz);

        return inv;
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

static void sortVertices(int *y0, int *y1, int *y2, int *x0, int *x1, int *x2)
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

static int interpolateX(int y1, int y2, int x1, int x2, int y)
{
        if (y1 == y2)
                return x1;
        return x1 + (x2 - x1) * (y - y1) / (y2 - y1);
}

static void DrawTri(SCENE *Scene, int x0, int y0, int x1, int y1, int x2, int y2, int r, int g, int b)
{
        int minY = y0, maxY = y0;
        if (y1 < minY)
                minY = y1;
        if (y2 < minY)
                minY = y2;
        if (y1 > maxY)
                maxY = y1;
        if (y2 > maxY)
                maxY = y2;

        if (minY >= Scene->Renderer.RendererHeight || maxY < 0)
                return;

        int startY = minY < 0 ? 0 : minY;
        int endY = maxY >= Scene->Renderer.RendererHeight ? Scene->Renderer.RendererHeight - 1 : maxY;

        sortVertices(&y0, &y1, &y2, &x0, &x1, &x2);
        SDL_SetRenderDrawColor(Scene->Renderer.Renderer, r, g, b, 255);

        float dxdy_left1 = (y1 - y0 != 0) ? (float)(x1 - x0) / (y1 - y0) : 0;
        float dxdy_right1 = (y2 - y0 != 0) ? (float)(x2 - x0) / (y2 - y0) : 0;

        float dxdy_left2 = (y2 - y1 != 0) ? (float)(x2 - x1) / (y2 - y1) : 0;

        float curXLeft, curXRight;

        if (y0 != y1)
        {
                curXLeft = x0;
                curXRight = x0;

                int yStart = (y0 < 0) ? 0 : y0;
                int yEnd = (y1 > endY) ? endY : y1;

                for (int y = yStart; y <= yEnd; ++y)
                {
                        int xLeft = (int)curXLeft;
                        int xRight = (int)curXRight;

                        if (xLeft > xRight)
                        {
                                SDL_SWAP(&xLeft, &xRight);
                        }

                        if (xLeft < 0)
                                xLeft = 0;
                        if (xRight >= Scene->Renderer.RendererWidth)
                                xRight = Scene->Renderer.RendererWidth - 1;

                        if (xLeft <= xRight)
                        {
                                SDL_RenderDrawLine(Scene->Renderer.Renderer, xLeft, y, xRight, y);
                        }

                        if (y < y1)
                        {
                                curXLeft += dxdy_left1;
                                curXRight += dxdy_right1;
                        }
                }
        }

        if (y1 != y2)
        {
                curXLeft = x1;

                int yStart = (y1 < 0) ? 0 : y1;
                int yEnd = (y2 > endY) ? endY : y2;

                for (int y = yStart; y <= yEnd; ++y)
                {
                        int xLeft = (int)curXLeft;
                        int xRight = (int)(x0 + (y - y0) * dxdy_right1);

                        if (xLeft > xRight)
                        {
                                SDL_SWAP(&xLeft, &xRight);
                        }

                        if (xLeft < 0)
                                xLeft = 0;
                        if (xRight >= Scene->Renderer.RendererWidth)
                                xRight = Scene->Renderer.RendererWidth - 1;

                        if (xLeft <= xRight)
                        {
                                SDL_RenderDrawLine(Scene->Renderer.Renderer, xLeft, y, xRight, y);
                        }

                        curXLeft += dxdy_left2;
                }
        }
}

static void DrawTriWire(SCENE *Scene, int x0, int y0, int x1, int y1, int x2, int y2, int r, int g, int b)
{
        SDL_SetRenderDrawColor(Scene->Renderer.Renderer, r, g, b, 255);
        SDL_RenderDrawLine(Scene->Renderer.Renderer, x0, y0, x1, y1);
        SDL_RenderDrawLine(Scene->Renderer.Renderer, x1, y1, x2, y2);
        SDL_RenderDrawLine(Scene->Renderer.Renderer, x2, y2, x0, y0);
}

static COLOUR GetCol(COLOUR col, double lum)
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

        int R = (int)(255 * mappedLum + col.r);
        int G = (int)(255 * mappedLum + col.g);
        int B = (int)(255 * mappedLum + col.b);

        return (COLOUR){R, G, B};
}

static double GetTriangleDepth(const TRI3D *tri)
{
        return (tri->p[0].Z + tri->p[1].Z + tri->p[2].Z) / 3.0;
}

static int CompareTriangles(const void *a, const void *b)
{
        const TRI3D *triA = (const TRI3D *)a;
        const TRI3D *triB = (const TRI3D *)b;
        double depthA = GetTriangleDepth(triA);
        double depthB = GetTriangleDepth(triB);

        // Sort in descending order (furthest first)
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

        Mat4x4 RotMatrixX = MakeRotationX(DEG_TO_RAD(Cube->ROTX));
        Mat4x4 RotMatrixY = MakeRotationY(DEG_TO_RAD(Cube->ROTY));
        Mat4x4 RotMatrixZ = MakeRotationZ(DEG_TO_RAD(Cube->ROTZ));

        elapsedTime0 = elapsedTime1;
        elapsedTime1 = SDL_GetTicks();

        double elapsedTime = (elapsedTime1 - elapsedTime0) / (1000.0 / TIME_SCALE);
        if (elapsedTime0 == elapsedTime1)
        {
                elapsedTime = 0.0;
        }

        (void)elapsedTime;

        Mat4x4 ObjectRotation = MulMatMat(&RotMatrixY, &RotMatrixX);
        ObjectRotation = MulMatMat(&ObjectRotation, &RotMatrixZ);
        Mat4x4 ObjectTranslation = MakeTransMat(
            Cube->origin.X,
            Cube->origin.Y,
            Cube->origin.Z);

        Mat4x4 ScaleMatrix = MakeScaleMat(
            Cube->Scale.X,
            Cube->Scale.Y,
            Cube->Scale.Z);

        Mat4x4 WorldMatrix = MulMatMat(&ScaleMatrix, &ObjectRotation);
        WorldMatrix = MulMatMat(&WorldMatrix, &ObjectTranslation);

        float cameraYaw = Scene->Camera.Rotation.Y;
        float cameraPitch = Scene->Camera.Rotation.X;

        if (cameraPitch > 89.0f)
                cameraPitch = 89.0f;
        if (cameraPitch < -89.0f)
                cameraPitch = -89.0f;

        Mat4x4 RotMatrixYaw = MakeRotationY(DEG_TO_RAD(cameraYaw));
        Mat4x4 RotMatrixPitch = MakeRotationX(DEG_TO_RAD(cameraPitch));

        Mat4x4 CameraRotation = MulMatMat(&RotMatrixYaw, &RotMatrixPitch);

        Mat4x4 CameraTranslation = MakeTransMat(
            Scene->Camera.Position.X,
            Scene->Camera.Position.Y,
            Scene->Camera.Position.Z);

        Mat4x4 InvCameraRotation = TransposeMat(&CameraRotation);

        Mat4x4 InvCameraTranslation = MakeTransMat(
            -Scene->Camera.Position.X,
            -Scene->Camera.Position.Y,
            -Scene->Camera.Position.Z);

        Mat4x4 ViewMatrix = MulMatMat(&InvCameraTranslation, &InvCameraRotation);

        Mat4x4 WorldViewMatrix = MulMatMat(&WorldMatrix, &ViewMatrix);

        TRI3D *TrisToDraw = calloc(Cube->tri_count, sizeof(TRI3D));
        if (!TrisToDraw)
        {
                fprintf(stderr, "Failed to allocate memory for triangles\n");
                return elapsedTime1;
        }

        size_t trisDrawn = 0;

        for (size_t i = 0; i < Cube->tri_count; ++i)
        {
                TRI3D tri, triTransformed, triProjected;

                tri = Cube->tris[i];

                MulMatVec(&tri.p[0], &triTransformed.p[0], &WorldViewMatrix);
                MulMatVec(&tri.p[1], &triTransformed.p[1], &WorldViewMatrix);
                MulMatVec(&tri.p[2], &triTransformed.p[2], &WorldViewMatrix);

                int verticesBehind = 0;
                for (int j = 0; j < 3; ++j)
                {
                        if (triTransformed.p[j].Z < Scene->Camera.Near)
                                verticesBehind++;
                }
                if (verticesBehind > 0)
                        continue;

                VEC3 Normal, Line1, Line2;
                Line1 = SubVec3(&triTransformed.p[1], &triTransformed.p[0]);
                Line2 = SubVec3(&triTransformed.p[2], &triTransformed.p[0]);
                Normal = CrossProdVec3(&Line1, &Line2);

                double normalLength = LenVec3(&Normal);
                if (normalLength < Scene->Camera.Near)
                        continue;

                NormaliseVec3(&Normal);

                VEC3 cameraDir = {0.0, 0.0, -1.0};
                // if (DotVec3(&Normal, &cameraDir) >= 0.0)
                //         continue;

                VEC3 LightDir = {0.0, 0.4, -1.0};
                NormaliseVec3(&LightDir);
                double dp = DotVec3(&Normal, &LightDir);
                if (dp < 0.0)
                        dp = 0.0;
                COLOUR col = GetCol(tri.col, dp);
                triProjected.col = col;

                int verticesVisible = 0;
                for (int j = 0; j < 3; ++j)
                {
                        double z = triTransformed.p[j].Z;
                        if (z < Scene->Camera.Near)
                                z = Scene->Camera.Near;

                        double aspect = (double)Scene->Renderer.RendererWidth / Scene->Renderer.RendererHeight;
                        double fovScale = 1.0 / tan(DEG_TO_RAD(Scene->Camera.FOV) * 0.5);

                        triProjected.p[j].X = triTransformed.p[j].X * fovScale / z;
                        triProjected.p[j].Y = triTransformed.p[j].Y * fovScale / z;
                        triProjected.p[j].Z = z;

                        triProjected.p[j].X = (triProjected.p[j].X + 1.0) * 0.5 * Scene->Renderer.RendererWidth;
                        triProjected.p[j].Y = (1.0 - triProjected.p[j].Y) * 0.5 * Scene->Renderer.RendererHeight;

                        verticesVisible++;
                }

                if (verticesVisible == 0)
                        continue;

                memcpy(&TrisToDraw[trisDrawn], &triProjected, sizeof(TRI3D));
                trisDrawn++;
        }

        if (trisDrawn > 1)
        {
                qsort(TrisToDraw, trisDrawn, sizeof(TRI3D), CompareTriangles);
        }

        for (size_t i = 0; i < trisDrawn; ++i)
        {
                TRI3D *triToDraw = &TrisToDraw[i];

                int valid = 1;
                // for (int j = 0; j < 3; ++j)
                //{
                //         if (triToDraw->p[j].X < 2 * -Scene->Renderer.RendererWidth || triToDraw->p[j].X > 2 * Scene->Renderer.RendererWidth ||
                //             triToDraw->p[j].Y < 2 * -Scene->Renderer.RendererHeight || triToDraw->p[j].Y > 2 * Scene->Renderer.RendererHeight)
                //         {
                //                 valid = 0;
                //                 break;
                //         }
                // }
                if (!valid)
                        continue;

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
