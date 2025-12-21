#ifndef CAMERA_H
#define CAMERA_H

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>
#include<stdint.h>
#include<stddef.h>
#include<SDL2/SDL.h>
#include<engine/types.h>

#define WIRE_FRAME 0
#define RAD_TO_DEG(r) ((180 / M_PI) * r)
#define DEG_TO_RAD(r) ((M_PI / 180) * r)
#define frameDelay 33.3333333
#define camSpeedIncrease 1
#define TIME_SCALE 1.0

size_t DrawObject(Mesh3D *Cube, SCENE *Scene);
int CompareTriangles(const void *a, const void *b);
double GetTriangleDepth(const TRI3D *tri);
COLOUR GetCol(double lum);
void DrawTriWire(SCENE *Scene, int x0, int y0, int x1, int y1, int x2, int y2, int r, int g, int b);
void DrawTri(SCENE *Scene, int x0, int y0, int x1, int y1, int x2, int y2, int r, int g, int b);
int interpolateX(int y1, int y2, int x1, int x2, int y);
void sortVertices(int *y0, int *y1, int *y2, int *x0, int *x1, int *x2);
void InitProjectionMat(SCENE *Scene);
bool LoadMeshFromFile(const char *fileName, Mesh3D *mesh);
void DelMesh(Mesh3D *mesh);
Mesh3D *InitMesh(size_t triCount);
Mat4x4 MulMatMat(Mat4x4 *a, Mat4x4 *b);
Mat4x4 MakeProjMat(SCENE *Scene);
Mat4x4 MakeTransMat(double x, double y, double z);
Mat4x4 MakeRotationZ(double fAngleRad);
Mat4x4 MakeRotationY(double fAngleRad);
Mat4x4 MakeRotationX(double fAngleRad);
Mat4x4 MatMakeIdent();
void MulMatVec(VEC3 *i, VEC3 *o, Mat4x4 *m);
VEC3 CrossProdVec3(VEC3 *a, VEC3 *b);
VEC3 NormaliseVec3(VEC3 *a);
double LenVec3(VEC3 *x);
double DotVec3(VEC3 *a, VEC3 *b);
VEC3 ScaleVec3Div(VEC3 *a, double s);
VEC3 ScaleVec3Mul(VEC3 *a, double s);
VEC3 SubVec3(VEC3 *a, VEC3 *b);
VEC3 AddVec3(VEC3 *a, VEC3 *b);

#endif
