#include <engine/lithium.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <immintrin.h>
#include <omp.h>

#define N (1000)

const double G = (6.6743e-4);
const double SIM_DT = 1.0;

typedef struct
{
        double x, y, z;
} Snapshot;

Snapshot snap[2][N];
_Atomic int snap_idx = 0;

typedef struct
{
        __attribute__((aligned(32))) float X[N];
        __attribute__((aligned(32))) float Y[N];
        __attribute__((aligned(32))) float Z[N];
        __attribute__((aligned(32))) float VX[N], VY[N], VZ[N];
        __attribute__((aligned(32))) float R[N], M[N];
} BodiesSoA;

void *SimulationMain(void *running)
{
        bool *pRunning = running;
        BodiesSoA *Bodies = aligned_alloc(32, sizeof(BodiesSoA));

        const double CENTRAL_MASS = 1e10;

        // Bodies->X[0] = Bodies->Y[0] = Bodies->Z[0] = 0;
        // Bodies->VX[0] = Bodies->VY[0] = Bodies->VZ[0] = 0;
        // Bodies->R[0] = 1000;
        // Bodies->M[0] = CENTRAL_MASS;
        // for (size_t i = 1; i < N; ++i)
        // {
        //         double r = 1.5e5 + ((double)rand() / RAND_MAX) * 1e6;
        //         double theta = ((double)rand() / RAND_MAX) * 2.0 * M_PI;
        //         double y_offset = (((double)rand() / RAND_MAX) - 0.5) * 5;
        //         Bodies->X[i] = r * cos(theta);
        //         Bodies->Y[i] = y_offset;
        //         Bodies->Z[i] = r * sin(theta);
        //         double speed = sqrt(G * CENTRAL_MASS / r);
        //         Bodies->VX[i] = -speed * sin(theta);
        //         Bodies->VY[i] = 0;//((double)rand() / RAND_MAX - 0.5) * 50.0;
        //         Bodies->VZ[i] = speed * cos(theta);
        //         Bodies->R[i] = 2;
        //         Bodies->M[i] = 1000.0;
        // }

        for (size_t i = 0; i < N; ++i)
        {
                Bodies->X[i] = rand() % 100;
                Bodies->Y[i] = rand() % 100;
                Bodies->Z[i] = rand() % 100;
                Bodies->VX[i] = 0;
                Bodies->VY[i] = 0;
                Bodies->VZ[i] = 0;
                Bodies->R[i] = 2;
                Bodies->M[i] = 10.0;
        }

        __attribute__((aligned(32))) double ax[N] = {0}, ay[N] = {0}, az[N] = {0};
        __attribute__((aligned(32))) double ax_new[N], ay_new[N], az_new[N];

        bool Running = atomic_load(pRunning);
        size_t timer = 1;

        while (Running)
        {
#pragma omp parallel
                {
                        __attribute__((aligned(32))) double ax_private[N] = {0}, ay_private[N] = {0}, az_private[N] = {0};

#pragma omp for schedule(static)
                        for (size_t i = 0; i < N; ++i)
                        {
                                __m256 xi = _mm256_set1_ps(Bodies->X[i]);
                                __m256 yi = _mm256_set1_ps(Bodies->Y[i]);
                                __m256 zi = _mm256_set1_ps(Bodies->Z[i]);
                                __m256 mi = _mm256_set1_ps(Bodies->M[i]);

                                for (size_t j = i + 1; j < N; j += 8)
                                {
                                        const size_t rem = N - j;
                                        const size_t vec_len = rem >= 8 ? 8 : rem;

                                        __m256 xj = _mm256_loadu_ps(&Bodies->X[j]);
                                        __m256 yj = _mm256_loadu_ps(&Bodies->Y[j]);
                                        __m256 zj = _mm256_loadu_ps(&Bodies->Z[j]);
                                        __m256 mj = _mm256_loadu_ps(&Bodies->M[j]);

                                        __m256 dx = _mm256_sub_ps(xj, xi);
                                        __m256 dy = _mm256_sub_ps(yj, yi);
                                        __m256 dz = _mm256_sub_ps(zj, zi);

                                        const double eps = 1e-3;
                                        __m256 eps2 = _mm256_set1_ps(eps * eps);

                                        __m256 d2 = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(dx, dx),
                                                                                _mm256_mul_ps(dy, dy)),
                                                                  _mm256_add_ps(_mm256_mul_ps(dz, dz), eps2));

                                        __m256 invDist = _mm256_rsqrt_ps(d2);
                                        invDist = _mm256_mul_ps(invDist,
                                                                _mm256_sub_ps(_mm256_set1_ps(1.5f),
                                                                              _mm256_mul_ps(_mm256_mul_ps(_mm256_set1_ps(0.5f), d2),
                                                                                            _mm256_mul_ps(invDist, invDist))));

                                        __m256 F = _mm256_mul_ps(_mm256_mul_ps(_mm256_set1_ps(G), mi),
                                                                 _mm256_mul_ps(mj, _mm256_mul_ps(invDist, invDist)));

                                        __m256 fx = _mm256_mul_ps(F, _mm256_mul_ps(dx, invDist));
                                        __m256 fy = _mm256_mul_ps(F, _mm256_mul_ps(dy, invDist));
                                        __m256 fz = _mm256_mul_ps(F, _mm256_mul_ps(dz, invDist));

                                        __attribute__((aligned(32))) float fx_arr[8], fy_arr[8], fz_arr[8];
                                        _mm256_store_ps(fx_arr, fx);
                                        _mm256_store_ps(fy_arr, fy);
                                        _mm256_store_ps(fz_arr, fz);

                                        for (size_t k = 0; k < vec_len; ++k)
                                        {
                                                ax_private[i] += fx_arr[k] / Bodies->M[i];
                                                ay_private[i] += fy_arr[k] / Bodies->M[i];
                                                az_private[i] += fz_arr[k] / Bodies->M[i];

                                                ax_private[j + k] -= fx_arr[k] / Bodies->M[j + k];
                                                ay_private[j + k] -= fy_arr[k] / Bodies->M[j + k];
                                                az_private[j + k] -= fz_arr[k] / Bodies->M[j + k];
                                        }
                                }
                        }

#pragma omp parallel
                        for (size_t i = 0; i < N; ++i)
                        {
                                ax[i] += ax_private[i];
                                ay[i] += ay_private[i];
                                az[i] += az_private[i];
                        }
                }

#pragma omp parallel for
                for (size_t i = 0; i < N; ++i)
                {
                        Bodies->X[i] += Bodies->VX[i] * SIM_DT + 0.5 * ax[i] * SIM_DT * SIM_DT;
                        Bodies->Y[i] += Bodies->VY[i] * SIM_DT + 0.5 * ay[i] * SIM_DT * SIM_DT;
                        Bodies->Z[i] += Bodies->VZ[i] * SIM_DT + 0.5 * az[i] * SIM_DT * SIM_DT;
                }

#pragma omp parallel
                {
                        __attribute__((aligned(32))) double ax_private[N] = {0}, ay_private[N] = {0}, az_private[N] = {0};

#pragma omp for schedule(static)
                        for (size_t i = 0; i < N; ++i)
                        {
                                __m256 xi = _mm256_set1_ps(Bodies->X[i]);
                                __m256 yi = _mm256_set1_ps(Bodies->Y[i]);
                                __m256 zi = _mm256_set1_ps(Bodies->Z[i]);
                                __m256 mi = _mm256_set1_ps(Bodies->M[i]);

                                for (size_t j = i + 1; j < N; j += 8)
                                {
                                        const size_t rem = N - j;
                                        const size_t vec_len = rem >= 8 ? 8 : rem;

                                        __m256 xj = _mm256_loadu_ps(&Bodies->X[j]);
                                        __m256 yj = _mm256_loadu_ps(&Bodies->Y[j]);
                                        __m256 zj = _mm256_loadu_ps(&Bodies->Z[j]);
                                        __m256 mj = _mm256_loadu_ps(&Bodies->M[j]);

                                        __m256 dx = _mm256_sub_ps(xj, xi);
                                        __m256 dy = _mm256_sub_ps(yj, yi);
                                        __m256 dz = _mm256_sub_ps(zj, zi);

                                        const double eps = 1e-3;
                                        __m256 eps2 = _mm256_set1_ps(eps * eps);

                                        __m256 d2 = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(dx, dx),
                                                                                _mm256_mul_ps(dy, dy)),
                                                                  _mm256_add_ps(_mm256_mul_ps(dz, dz), eps2));

                                        __m256 invDist = _mm256_rsqrt_ps(d2);
                                        invDist = _mm256_mul_ps(invDist,
                                                                _mm256_sub_ps(_mm256_set1_ps(1.5f),
                                                                              _mm256_mul_ps(_mm256_mul_ps(_mm256_set1_ps(0.5f), d2),
                                                                                            _mm256_mul_ps(invDist, invDist))));
                                        __m256 F = _mm256_mul_ps(_mm256_mul_ps(_mm256_set1_ps(G), mi),
                                                                 _mm256_mul_ps(mj, _mm256_mul_ps(invDist, invDist)));

                                        __m256 fx = _mm256_mul_ps(F, _mm256_mul_ps(dx, invDist));
                                        __m256 fy = _mm256_mul_ps(F, _mm256_mul_ps(dy, invDist));
                                        __m256 fz = _mm256_mul_ps(F, _mm256_mul_ps(dz, invDist));

                                        __attribute__((aligned(32))) float fx_arr[8], fy_arr[8], fz_arr[8];
                                        _mm256_store_ps(fx_arr, fx);
                                        _mm256_store_ps(fy_arr, fy);
                                        _mm256_store_ps(fz_arr, fz);

                                        for (size_t k = 0; k < vec_len; ++k)
                                        {

                                                ax_private[i] += fx_arr[k] / Bodies->M[i];
                                                ay_private[i] += fy_arr[k] / Bodies->M[i];
                                                az_private[i] += fz_arr[k] / Bodies->M[i];

                                                ax_private[j + k] -= fx_arr[k] / Bodies->M[j + k];
                                                ay_private[j + k] -= fy_arr[k] / Bodies->M[j + k];
                                                az_private[j + k] -= fz_arr[k] / Bodies->M[j + k];
                                        }
                                }
                        }

#pragma omp critical
                        for (size_t i = 0; i < N; ++i)
                        {
                                ax_new[i] = ax_private[i];
                                ay_new[i] = ay_private[i];
                                az_new[i] = az_private[i];
                        }
                }

#pragma omp parallel for
                for (size_t i = 0; i < N; ++i)
                {
                        Bodies->VX[i] += 0.5 * (ax[i] + ax_new[i]) * SIM_DT;
                        Bodies->VY[i] += 0.5 * (ay[i] + ay_new[i]) * SIM_DT;
                        Bodies->VZ[i] += 0.5 * (az[i] + az_new[i]) * SIM_DT;

                        ax[i] = ax_new[i];
                        ay[i] = ay_new[i];
                        az[i] = az_new[i];
                }

#pragma omp parallel for schedule(dynamic)
                for (size_t i = 0; i < N; ++i)
                {
                        __m256 xi = _mm256_set1_ps(Bodies->X[i]);
                        __m256 yi = _mm256_set1_ps(Bodies->Y[i]);
                        __m256 zi = _mm256_set1_ps(Bodies->Z[i]);
                        __m256 vxi = _mm256_set1_ps(Bodies->VX[i]);
                        __m256 vyi = _mm256_set1_ps(Bodies->VY[i]);
                        __m256 vzi = _mm256_set1_ps(Bodies->VZ[i]);
                        __m256 mi = _mm256_set1_ps(Bodies->M[i]);
                        __m256 ri = _mm256_set1_ps(Bodies->R[i]);

                        for (size_t j = i + 1; j < N; j += 8)
                        {
                                size_t rem = N - j;
                                size_t vec_len = rem >= 8 ? 8 : rem;

                                __m256 xj = _mm256_loadu_ps(&Bodies->X[j]);
                                __m256 yj = _mm256_loadu_ps(&Bodies->Y[j]);
                                __m256 zj = _mm256_loadu_ps(&Bodies->Z[j]);
                                __m256 vxj = _mm256_loadu_ps(&Bodies->VX[j]);
                                __m256 vyj = _mm256_loadu_ps(&Bodies->VY[j]);
                                __m256 vzj = _mm256_loadu_ps(&Bodies->VZ[j]);
                                __m256 mj = _mm256_loadu_ps(&Bodies->M[j]);
                                __m256 rj = _mm256_loadu_ps(&Bodies->R[j]);

                                __m256 dx = _mm256_sub_ps(xj, xi);
                                __m256 dy = _mm256_sub_ps(yj, yi);
                                __m256 dz = _mm256_sub_ps(zj, zi);

                                __m256 dist2 = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(dx, dx),
                                                                           _mm256_mul_ps(dy, dy)),
                                                             _mm256_mul_ps(dz, dz));

                                __m256 minDist = _mm256_add_ps(ri, rj);
                                __m256 minDist2 = _mm256_mul_ps(minDist, minDist);

                                __m256 mask = _mm256_cmp_ps(dist2, minDist2, _CMP_LT_OS);

                                if (_mm256_movemask_ps(mask) != 0)
                                {
                                        float dx_arr[8], dy_arr[8], dz_arr[8];
                                        float dist2_arr[8], minDist_arr[8];
                                        _mm256_store_ps(dx_arr, dx);
                                        _mm256_store_ps(dy_arr, dy);
                                        _mm256_store_ps(dz_arr, dz);
                                        _mm256_store_ps(dist2_arr, dist2);
                                        _mm256_store_ps(minDist_arr, minDist);

                                        for (size_t k = 0; k < vec_len; ++k)
                                        {
                                                if (dist2_arr[k] < minDist_arr[k] * minDist_arr[k])
                                                {
                                                        double dist = sqrt(dist2_arr[k]);
                                                        double nx = dx_arr[k] / dist;
                                                        double ny = dy_arr[k] / dist;
                                                        double nz = dz_arr[k] / dist;

                                                        double rvx = Bodies->VX[j + k] - Bodies->VX[i];
                                                        double rvy = Bodies->VY[j + k] - Bodies->VY[i];
                                                        double rvz = Bodies->VZ[j + k] - Bodies->VZ[i];

                                                        double vAlongNormal = rvx * nx + rvy * ny + rvz * nz;
                                                        if (vAlongNormal < 0)
                                                        {
                                                                double m1 = Bodies->M[i];
                                                                double m2 = Bodies->M[j + k];
                                                                double e = 0.9;
                                                                double impulse = (1 + e) * vAlongNormal / (m1 + m2);
                                                                Bodies->VX[i] += impulse * m2 * nx;
                                                                Bodies->VY[i] += impulse * m2 * ny;
                                                                Bodies->VZ[i] += impulse * m2 * nz;
                                                                Bodies->VX[j + k] -= impulse * m1 * nx;
                                                                Bodies->VY[j + k] -= impulse * m1 * ny;
                                                                Bodies->VZ[j + k] -= impulse * m1 * nz;
                                                        }

                                                        double overlap = minDist_arr[k] - dist;
                                                        Bodies->X[i] -= nx * (overlap * Bodies->M[j + k] / (Bodies->M[i] + Bodies->M[j + k]));
                                                        Bodies->Y[i] -= ny * (overlap * Bodies->M[j + k] / (Bodies->M[i] + Bodies->M[j + k]));
                                                        Bodies->Z[i] -= nz * (overlap * Bodies->M[j + k] / (Bodies->M[i] + Bodies->M[j + k]));
                                                        Bodies->X[j + k] += nx * (overlap * Bodies->M[i] / (Bodies->M[i] + Bodies->M[j + k]));
                                                        Bodies->Y[j + k] += ny * (overlap * Bodies->M[i] / (Bodies->M[i] + Bodies->M[j + k]));
                                                        Bodies->Z[j + k] += nz * (overlap * Bodies->M[i] / (Bodies->M[i] + Bodies->M[j + k]));
                                                }
                                        }
                                }
                        }
                }

                if (timer-- <= 0)
                {
                        const int w = (snap_idx + 1) % 2;
#pragma omp parallel for
                        for (size_t i = 0; i < N; ++i)
                        {
                                snap[w][i].x = Bodies->X[i];
                                snap[w][i].y = Bodies->Y[i];
                                snap[w][i].z = Bodies->Z[i];
                        }
                        atomic_store(&snap_idx, w);
                        timer = 1;
                }

                Running = atomic_load(pRunning);
        }

        free(Bodies);
}

int main(int argc, char **argv)
{
        SCENE *Scene = LithiumInit(argc, argv);
        Scene->SoundSys.PrimaryJumpSound = LoadSound(Scene, "assets/jump.wav");
        Scene->SoundSys.DenySelectSound = LoadSound(Scene, "assets/denyselect.wav");
        Scene->SoundSys.PrimaryStepSounds[0] = LoadSound(Scene, "assets/walk_0.wav");
        Scene->SoundSys.PrimaryStepSounds[1] = LoadSound(Scene, "assets/walk_1.wav");
        Scene->SoundSys.PrimaryStepSounds[2] = LoadSound(Scene, "assets/walk_2.wav");
        Scene->SoundSys.PrimaryStepSounds[3] = LoadSound(Scene, "assets/walk_3.wav");
        Scene->Player.Position.Z = 0;//-(1e6 + 2.5e5);
        Scene->Player.RunSpeed *=  10; //10000;
        Scene->Player.WalkSpeed *= 10; // 1000;

        ENTITY *K = LiObj(Scene, LithiumLoadObject(Scene, "assets/tri.obj"));
        // ENTITY *Star = LiObj(Scene, LithiumLoadObject(Scene, "assets/tri.obj"));

        for (size_t i = 0; i < K->TriCount; ++i)
        {
                K->Tris[i].col.r = 128;
                K->Tris[i].col.g = 128;
                K->Tris[i].col.b = 128;
                K->Tris[i].col.a = 255;
        }

        for (size_t i = 0; i < N; ++i)
        {
                ENTITY *E = InitMesh(Scene, 0);
                *E = *K;
                da_append(Scene, E);
        }

        pthread_t thread;
        pthread_create(&thread, NULL, SimulationMain, &Scene->Running);

        // ScaleMesh(Scene->items[1], 20000);
        ScaleMesh(Scene->items[0], 2);

        int prv = atomic_load(&snap_idx);
        while (Scene->Running)
        {
                int r = atomic_load(&snap_idx);
                if (prv != r)
                {
#pragma omp parallel for
                        for (size_t i = 0; i < N; ++i)
                        {
                                Scene->items[i + 1]->Origin.X = snap[r][i].x;
                                Scene->items[i + 1]->Origin.Y = snap[r][i].y;
                                Scene->items[i + 1]->Origin.Z = snap[r][i].z;
                        }
                }

                SceneTick(&Scene);
                LithiumUpdate(Scene);
                DrawScene(Scene);
                prv = r;
        }

        for (size_t i = 0; i < N; ++i)
        {
                ENTITY *E = Scene->items[i + 1];
                E->TriCount = 0;
                E->Tris = NULL;
        }

        pthread_join(thread, NULL);
        LithiumEnd(Scene);
}
