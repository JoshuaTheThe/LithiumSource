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

        double fAspectRatio = (double)Scene->Renderer.RendererWidth / (double)Scene->Renderer.RendererHeight;
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
        double fAspectRatio = (double)Scene->Renderer.RendererWidth / (double)Scene->Renderer.RendererHeight;
        double fFovRad = 1.0 / tan(Scene->Camera.FOV * 0.5 / 180.0 * M_PI);
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
        sortVertices(&y0, &y1, &y2, &x0, &x1, &x2);
        SDL_Rect originalClip;
        SDL_RenderGetClipRect(Scene->Renderer.Renderer, &originalClip);
        SDL_Rect screenRect = {0, 0, Scene->Renderer.RendererWidth, Scene->Renderer.RendererHeight};
        SDL_RenderSetClipRect(Scene->Renderer.Renderer, &screenRect);
        SDL_SetRenderDrawColor(Scene->Renderer.Renderer, r, g, b, 255);
        SDL_Vertex Verts[] = {
            {.position = {x0, y0}, .color = {r, g, b, 255}, .tex_coord = {0, 0}},
            {.position = {x1, y1}, .color = {r, g, b, 255}, .tex_coord = {0, 0}},
            {.position = {x2, y2}, .color = {r, g, b, 255}, .tex_coord = {0, 0}},
        };
        SDL_RenderGeometry(Scene->Renderer.Renderer, NULL, Verts, 3, NULL, 0);
        SDL_RenderSetClipRect(Scene->Renderer.Renderer, &originalClip);
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

        int R = col.r;//(int)(256.0 * (mappedLum * ((double)col.r / 256.0)));
        int G = col.g;//(int)(256.0 * (mappedLum * ((double)col.g / 256.0)));
        int B = col.b;//(int)(256.0 * (mappedLum * ((double)col.b / 256.0)));

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

        // Sort in descending order (furthest first)
        if (triA->Depth > triB->Depth)
                return -1;
        else if (triA->Depth < triB->Depth)
                return 1;
        else
                return 0;
}

static VEC3 IntersectPlane(
    VEC3 *planePoint,
    VEC3 *planeNormal,
    VEC3 *lineStart,
    VEC3 *lineEnd)
{
        planeNormal = &(VEC3){
            planeNormal->X / LenVec3(planeNormal),
            planeNormal->Y / LenVec3(planeNormal),
            planeNormal->Z / LenVec3(planeNormal)};

        double plane_d = -DotVec3(planeNormal, planePoint);
        double ad = DotVec3(lineStart, planeNormal);
        double bd = DotVec3(lineEnd, planeNormal);
        double t = (-plane_d - ad) / (bd - ad);

        VEC3 lineStartToEnd = SubVec3(lineEnd, lineStart);
        VEC3 lineToIntersect = ScaleVec3Mul(&lineStartToEnd, t);
        return AddVec3(lineStart, &lineToIntersect);
}

static int ClipTriangleNearPlane(
    TRI3D *in,
    TRI3D *out1,
    TRI3D *out2,
    double near)
{
        VEC3 planePoint = {0, 0, near};
        VEC3 planeNormal = {0, 0, 1};

        VEC3 *inside[3];
        VEC3 *outside[3];
        int nInside = 0, nOutside = 0;

        for (int i = 0; i < 3; i++)
        {
                if (in->p[i].Z >= near)
                        inside[nInside++] = &in->p[i];
                else
                        outside[nOutside++] = &in->p[i];
        }

        if (nInside == 0)
                return 0;

        if (nInside == 3)
        {
                *out1 = *in;
                return 1;
        }

        if (nInside == 1 && nOutside == 2)
        {
                out1->p[0] = *inside[0];
                out1->p[1] = IntersectPlane(&planePoint, &planeNormal, inside[0], outside[0]);
                out1->p[2] = IntersectPlane(&planePoint, &planeNormal, inside[0], outside[1]);
                out1->col = in->col;
                return 1;
        }

        if (nInside == 2 && nOutside == 1)
        {
                out1->p[0] = *inside[0];
                out1->p[1] = *inside[1];
                out1->p[2] = IntersectPlane(&planePoint, &planeNormal, inside[0], outside[0]);

                out2->p[0] = *inside[1];
                out2->p[1] = out1->p[2];
                out2->p[2] = IntersectPlane(&planePoint, &planeNormal, inside[1], outside[0]);

                out1->col = out2->col = in->col;
                return 2;
        }

        return 0;
}

static int ProcessViewTriangle(
    const TRI3D *triView,
    TRI3D *out,
    SCENE *Scene)
{
        TRI3D clipped[2];
        int n = ClipTriangleNearPlane(
            (TRI3D *)triView,
            &clipped[0],
            &clipped[1],
            Scene->Camera.Near);

        int outCount = 0;

        for (int t = 0; t < n; t++)
        {
                TRI3D *triClip = &clipped[t];
                TRI3D triProj;

                VEC3 l1 = SubVec3(&triClip->p[1], &triClip->p[0]);
                VEC3 l2 = SubVec3(&triClip->p[2], &triClip->p[0]);
                VEC3 nrm = CrossProdVec3(&l1, &l2);

                if (LenVec3(&nrm) < 1e-6)
                        continue;

                nrm = NormaliseVec3(&nrm);

                VEC3 viewDir = {0, 0, 1};
                //if (DotVec3(&nrm, &viewDir) <= 0)
                //        continue;

                VEC3 light = {0, 0.4, 1};
                light = NormaliseVec3(&light);

                double dp = DotVec3(&nrm, &light);
                if (dp < 0)
                        dp = 0;

                triProj.col = GetCol(triView->col, dp);

                triProj.Depth =
                    (triClip->p[0].Z +
                     triClip->p[1].Z +
                     triClip->p[2].Z) /
                    3.0;

                for (int i = 0; i < 3; i++)
                {
                        MulMatVec(&triClip->p[i],
                                  &triProj.p[i],
                                  &ProjectionMatrix);

                        triProj.p[i].X =
                            (triProj.p[i].X + 1) * 0.5 *
                            Scene->Renderer.RendererWidth;

                        triProj.p[i].Y =
                            (1 - triProj.p[i].Y) * 0.5 *
                            Scene->Renderer.RendererHeight;
                }

                out[outCount++] = triProj;
        }

        return outCount;
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

        float yaw = DEG_TO_RAD(Scene->Camera.Rotation.Y);
        float pitch = DEG_TO_RAD(Scene->Camera.Rotation.X);

        if (pitch > DEG_TO_RAD(89.0f))
                pitch = DEG_TO_RAD(89.0f);
        if (pitch < DEG_TO_RAD(-89.0f))
                pitch = DEG_TO_RAD(-89.0f);

        VEC3 cameraForward = {
            sin(yaw) * cos(pitch),
            sin(pitch),
            cos(yaw) * cos(pitch)};

        cameraForward = NormaliseVec3(&cameraForward);

        VEC3 worldUp = {0.0, 1.0, 0.0};
        VEC3 cameraRight = CrossProdVec3(&worldUp, &cameraForward);
        cameraRight = NormaliseVec3(&cameraRight);
        VEC3 cameraUp = CrossProdVec3(&cameraForward, &cameraRight);

        Mat4x4 ViewMatrix = {0};

        ViewMatrix.m[0][0] = cameraRight.X;
        ViewMatrix.m[1][0] = cameraRight.Y;
        ViewMatrix.m[2][0] = cameraRight.Z;

        ViewMatrix.m[0][1] = cameraUp.X;
        ViewMatrix.m[1][1] = cameraUp.Y;
        ViewMatrix.m[2][1] = cameraUp.Z;

        ViewMatrix.m[0][2] = cameraForward.X;
        ViewMatrix.m[1][2] = cameraForward.Y;
        ViewMatrix.m[2][2] = cameraForward.Z;

        ViewMatrix.m[3][3] = 1.0;

        ViewMatrix.m[3][0] = -DotVec3(&cameraRight, &Scene->Camera.Position);
        ViewMatrix.m[3][1] = -DotVec3(&cameraUp, &Scene->Camera.Position);
        ViewMatrix.m[3][2] = -DotVec3(&cameraForward, &Scene->Camera.Position);

        Mat4x4 WorldViewMatrix = MulMatMat(&WorldMatrix, &ViewMatrix);

        TRI3D *TrisToDraw = calloc(Cube->tri_count * 4, sizeof(TRI3D));
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
                triTransformed = tri;

                MulMatVec(&tri.p[0], &triTransformed.p[0], &WorldViewMatrix);
                MulMatVec(&tri.p[1], &triTransformed.p[1], &WorldViewMatrix);
                MulMatVec(&tri.p[2], &triTransformed.p[2], &WorldViewMatrix);

                TRI3D clipped[2];
                int n = ClipTriangleNearPlane(&triTransformed,
                                              &clipped[0],
                                              &clipped[1],
                                              Scene->Camera.Near);

                TRI3D out[2];
                int count = ProcessViewTriangle(&triTransformed, out, Scene);

                for (int k = 0; k < count; k++)
                {
                        TrisToDraw[trisDrawn++] = out[k];
                }
        }

        if (trisDrawn > 1)
        {
                qsort(TrisToDraw, trisDrawn, sizeof(TRI3D), CompareTriangles);
        }

        for (size_t i = 0; i < trisDrawn; ++i)
        {
                TRI3D *triToDraw = &TrisToDraw[i];

                // int valid = 1;
                // for (int j = 0; j < 3; ++j)
                //{
                //         if (triToDraw->p[j].X < 2 * -Scene->Renderer.RendererWidth || triToDraw->p[j].X > 2 * Scene->Renderer.RendererWidth ||
                //             triToDraw->p[j].Y < 2 * -Scene->Renderer.RendererHeight || triToDraw->p[j].Y > 2 * Scene->Renderer.RendererHeight)
                //         {
                //                 valid = 0;
                //                 break;
                //         }
                // }
                // if (!valid)
                //         continue;

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
