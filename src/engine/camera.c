/**
 * Camera - Render 3D stuff
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

void MulMatVec(const VEC3 *const i, VEC3 *o, const Mat4x4 *const m)
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

void MulMatVecW(const VEC3 *i, VEC3 *o, double *wOut, const Mat4x4 *m)
{
        double w =
            i->X * m->m[0][3] +
            i->Y * m->m[1][3] +
            i->Z * m->m[2][3] +
            m->m[3][3];

        o->X = i->X * m->m[0][0] + i->Y * m->m[1][0] + i->Z * m->m[2][0] + m->m[3][0];
        o->Y = i->X * m->m[0][1] + i->Y * m->m[1][1] + i->Z * m->m[2][1] + m->m[3][1];
        o->Z = i->X * m->m[0][2] + i->Y * m->m[1][2] + i->Z * m->m[2][2] + m->m[3][2];

        *wOut = w;

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
        double fFovRad = 1.0 / tan(Scene->Player.FOV * 0.5f / 180.0 * M_PI);
        double far = Scene->Player.Far;
        double near = Scene->Player.Near;

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

void InitProjectionMat(SCENE *Scene)
{
        double fAspectRatio = (double)Scene->Renderer.RendererWidth / (double)Scene->Renderer.RendererHeight;
        double fFovRad = 1.0 / tan(Scene->Player.FOV * 0.5 / 180.0 * M_PI);
        double far = Scene->Player.Far;
        double near = Scene->Player.Near;

        ProjectionMatrix.m[0][0] = fAspectRatio * fFovRad;
        ProjectionMatrix.m[1][1] = fFovRad;
        ProjectionMatrix.m[2][2] = far / (far - near);
        ProjectionMatrix.m[3][2] = (-far * near) / (far - near);
        ProjectionMatrix.m[2][3] = 1.0;
        ProjectionMatrix.m[3][3] = 0.0;
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

        int R = col.r; //(int)(256.0 * (mappedLum * ((double)col.r / 256.0)));
        int G = col.g; //(int)(256.0 * (mappedLum * ((double)col.g / 256.0)));
        int B = col.b; //(int)(256.0 * (mappedLum * ((double)col.b / 256.0)));

        return (COLOUR){R, G, B};
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

static UV IntersectUV(UV uvStart, UV uvEnd, double t)
{
        return (UV){
            uvStart.u + (uvEnd.u - uvStart.u) * t,
            uvStart.v + (uvEnd.v - uvStart.v) * t};
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
        UV *uvInside[3];
        UV *uvOutside[3];
        int nInside = 0, nOutside = 0;

        for (int i = 0; i < 3; i++)
        {
                if (in->p[i].Z >= near)
                {
                        inside[nInside] = &in->p[i];
                        uvInside[nInside] = &in->uv[i];
                        nInside++;
                }
                else
                {
                        outside[nOutside] = &in->p[i];
                        uvOutside[nOutside] = &in->uv[i];
                        nOutside++;
                }
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
                out1->uv[0] = *uvInside[0];

                double t0 = (near - inside[0]->Z) / (outside[0]->Z - inside[0]->Z);
                double t1 = (near - inside[0]->Z) / (outside[1]->Z - inside[0]->Z);

                out1->p[1] = IntersectPlane(&planePoint, &planeNormal, inside[0], outside[0]);
                out1->uv[1] = IntersectUV(*uvInside[0], *uvOutside[0], t0);

                out1->p[2] = IntersectPlane(&planePoint, &planeNormal, inside[0], outside[1]);
                out1->uv[2] = IntersectUV(*uvInside[0], *uvOutside[1], t1);

                out1->col = in->col;
                return 1;
        }

        if (nInside == 2 && nOutside == 1)
        {
                out1->p[0] = *inside[0];
                out1->uv[0] = *uvInside[0];
                out1->p[1] = *inside[1];
                out1->uv[1] = *uvInside[1];

                double t0 = (near - inside[0]->Z) / (outside[0]->Z - inside[0]->Z);
                double t1 = (near - inside[1]->Z) / (outside[0]->Z - inside[1]->Z);

                out1->p[2] = IntersectPlane(&planePoint, &planeNormal, inside[0], outside[0]);
                out1->uv[2] = IntersectUV(*uvInside[0], *uvOutside[0], t0);

                out2->p[0] = *inside[1];
                out2->uv[0] = *uvInside[1];
                out2->p[1] = out1->p[2];
                out2->uv[1] = out1->uv[2];
                out2->p[2] = IntersectPlane(&planePoint, &planeNormal, inside[1], outside[0]);
                out2->uv[2] = IntersectUV(*uvInside[1], *uvOutside[0], t1);

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
            Scene->Player.Near);

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

                // VEC3 viewDir = {0, 0, 1};
                // if (DotVec3(&nrm, &viewDir) > 0)
                //         continue;

                VEC3 light = Scene->Player.LightPos;
                light = NormaliseVec3(&light);

                double dp = DotVec3(&nrm, &light);
                if (dp < 0)
                        dp = 0;

                triProj.col = GetCol(triView->col, dp);
                triProj.uv[0] = triView->uv[0];
                triProj.uv[1] = triView->uv[1];
                triProj.uv[2] = triView->uv[2];

                for (int i = 0; i < 3; i++)
                {
                        double clip_w = 0.0;
                        MulMatVecW(&triClip->p[i],
                                   &triProj.p[i],
                                   &clip_w,
                                   &ProjectionMatrix);

                        triProj.w[i] = 1.0 / clip_w;

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

static Mat4x4 MakeViewMat(VEC3 cameraForward, VEC3 cameraRight, VEC3 cameraUp, VEC3 cameraPos)
{
        Mat4x4 mat =
            {
                .m[0][0] = cameraRight.X,
                .m[1][0] = cameraRight.Y,
                .m[2][0] = cameraRight.Z,
                .m[0][1] = cameraUp.X,
                .m[1][1] = cameraUp.Y,
                .m[2][1] = cameraUp.Z,
                .m[0][2] = cameraForward.X,
                .m[1][2] = cameraForward.Y,
                .m[2][2] = cameraForward.Z,
                .m[3][3] = 1.0,
                .m[3][0] = -DotVec3(&cameraRight, &cameraPos),
                .m[3][1] = -DotVec3(&cameraUp, &cameraPos),
                .m[3][2] = -DotVec3(&cameraForward, &cameraPos),
            };
        return mat;
}

static Mat4x4 MakeWorldMat(const Mesh3D *const Mesh, VEC3 cameraForward, VEC3 cameraRight, VEC3 cameraUp, VEC3 cameraPos)
{
        Mat4x4 RotMatrixX = MakeRotationX(DEG_TO_RAD(Mesh->Rotation.X));
        Mat4x4 RotMatrixY = MakeRotationY(DEG_TO_RAD(Mesh->Rotation.Y));
        Mat4x4 RotMatrixZ = MakeRotationZ(DEG_TO_RAD(Mesh->Rotation.Z));

        Mat4x4 ObjectRotation = MulMatMat(&RotMatrixY, &RotMatrixX);
        ObjectRotation = MulMatMat(&ObjectRotation, &RotMatrixZ);
        Mat4x4 ObjectTranslation = MakeTransMat(
            Mesh->Origin.X,
            Mesh->Origin.Y,
            Mesh->Origin.Z);

        Mat4x4 ScaleMatrix = MakeScaleMat(
            Mesh->Scale.X,
            Mesh->Scale.Y,
            Mesh->Scale.Z);

        Mat4x4 WorldMatrix = MulMatMat(&ScaleMatrix, &ObjectRotation);
        WorldMatrix = MulMatMat(&WorldMatrix, &ObjectTranslation);

        Mat4x4 ViewMatrix = MakeViewMat(
            cameraForward, cameraRight, cameraUp,
            cameraPos);

        Mat4x4 WorldViewMatrix = MulMatMat(&WorldMatrix, &ViewMatrix);
        return WorldViewMatrix;
}

size_t DrawScene(SCENE *Scene)
{
        if (!Scene)
                TODO();
        SDL_SetRenderDrawColor(Scene->Renderer.Renderer, 0, 0, 0, 255);
        SDL_RenderClear(Scene->Renderer.Renderer);
        SceneClearBuffers(Scene);
        TRI3D tri, triTransformed, out[2];
        int count;
        const double yaw = DEG_TO_RAD(Scene->Player.Rotation.Y);
        const double pitch = fmax(fmin(DEG_TO_RAD(Scene->Player.Rotation.X), DEG_TO_RAD(89.0)), DEG_TO_RAD(-89.0));
        const VEC3 cameraForwardUn = {
            sin(yaw) * cos(pitch),
            sin(pitch),
            cos(yaw) * cos(pitch)};
        const VEC3 cameraForward = NormaliseVec3(&cameraForwardUn);
        const VEC3 worldUp = {0.0, 1.0, 0.0};
        const VEC3 cameraRightUn = CrossProdVec3(&worldUp, &cameraForward);
        const VEC3 cameraRight = NormaliseVec3(&cameraRightUn);
        const VEC3 cameraUp = CrossProdVec3(&cameraForward, &cameraRight);
        VEC3 position = Scene->Player.Position;
        position.Y += Scene->Player.CameraOffsetY;

        for (size_t i = 0; i < Scene->count; ++i)
        {
                const Mesh3D *Mesh = Scene->items[i];
                const Mat4x4 WorldViewMatrix = MakeWorldMat(
                    Mesh, cameraForward, cameraRight, cameraUp, position);

                for (size_t i = 0; i < Mesh->TriCount; ++i)
                {
                        triTransformed = tri = Mesh->Tris[i];
                        MulMatVec(&tri.p[0], &triTransformed.p[0], &WorldViewMatrix);
                        MulMatVec(&tri.p[1], &triTransformed.p[1], &WorldViewMatrix);
                        MulMatVec(&tri.p[2], &triTransformed.p[2], &WorldViewMatrix);
                        count = ProcessViewTriangle(&triTransformed, out, Scene);
                        for (int k = 0; k < count; k++)
                        {
                                out[k].Texture = tri.Texture;
                                DrawTriWTex(Scene, out[k]);
                        }
                }
        }

        SDL_RenderPresent(Scene->Renderer.Renderer);
        return 0;
}
