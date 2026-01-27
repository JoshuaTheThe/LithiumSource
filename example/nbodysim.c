#include <engine/lithium.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <immintrin.h>
#include <omp.h>

#define N (10000)

const double G = (6.6743e-1);
const double SIM_DT = 10.0;

typedef struct
{
        double x, y, z;
} Snapshot;

Snapshot snap[2][N];
_Atomic int snap_idx = 0;

typedef struct
{
        __attribute__((aligned(32))) double X[N];
        __attribute__((aligned(32))) double Y[N];
        __attribute__((aligned(32))) double Z[N];
        __attribute__((aligned(32))) double VX[N], VY[N], VZ[N];
        double R[N], M[N];
} BodiesSoA;

void *SimulationMain(void *running)
{
        bool *pRunning = running;
        BodiesSoA Bodies;

        const double CENTRAL_MASS = 1e10;

        Bodies.X[0] = Bodies.Y[0] = Bodies.Z[0] = 0;
        Bodies.VX[0] = Bodies.VY[0] = Bodies.VZ[0] = 0;
        Bodies.R[0] = 1000;
        Bodies.M[0] = CENTRAL_MASS;

        for (size_t i = 1; i < N; ++i)
        {
                double r = 1.5e5 + ((double)rand() / RAND_MAX) * 1e6;
                double theta = ((double)rand() / RAND_MAX) * 2.0 * M_PI;
                double y_offset = (((double)rand() / RAND_MAX) - 0.5) * 5000;

                Bodies.X[i] = r * cos(theta);
                Bodies.Y[i] = y_offset;
                Bodies.Z[i] = r * sin(theta);

                double speed = sqrt(G * CENTRAL_MASS / r);

                Bodies.VX[i] = -speed * sin(theta);
                Bodies.VY[i] = ((double)rand() / RAND_MAX - 0.5) * 50.0;
                Bodies.VZ[i] = speed * cos(theta);

                Bodies.R[i] = 0.9;
                Bodies.M[i] = 100.0;
        }

        double ax[N] = {0}, ay[N] = {0}, az[N] = {0};
        double ax_new[N], ay_new[N], az_new[N];

        bool Running = atomic_load(pRunning);
        size_t timer = 1;

        while (Running)
        {
#pragma omp parallel
                {
                        double ax_private[N] = {0}, ay_private[N] = {0}, az_private[N] = {0};

#pragma omp for schedule(dynamic)
                        for (size_t i = 0; i < N; ++i)
                        {
                                __m256d xi = _mm256_set1_pd(Bodies.X[i]);
                                __m256d yi = _mm256_set1_pd(Bodies.Y[i]);
                                __m256d zi = _mm256_set1_pd(Bodies.Z[i]);
                                __m256d mi = _mm256_set1_pd(Bodies.M[i]);

                                for (size_t j = i + 1; j < N; j += 4)
                                {
                                        const size_t rem = N - j;
                                        const size_t vec_len = rem >= 4 ? 4 : rem;

                                        __m256d xj = _mm256_loadu_pd(&Bodies.X[j]);
                                        __m256d yj = _mm256_loadu_pd(&Bodies.Y[j]);
                                        __m256d zj = _mm256_loadu_pd(&Bodies.Z[j]);
                                        __m256d mj = _mm256_loadu_pd(&Bodies.M[j]);

                                        __m256d dx = _mm256_sub_pd(xj, xi);
                                        __m256d dy = _mm256_sub_pd(yj, yi);
                                        __m256d dz = _mm256_sub_pd(zj, zi);

                                        const double eps = 1e-3;
                                        __m256d eps2 = _mm256_set1_pd(eps * eps);

                                        __m256d d2 = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(dx, dx),
                                                                                 _mm256_mul_pd(dy, dy)),
                                                                   _mm256_add_pd(_mm256_mul_pd(dz, dz), eps2));

                                        __m256d invDist = _mm256_div_pd(_mm256_set1_pd(1.0), _mm256_sqrt_pd(d2));
                                        __m256d F = _mm256_mul_pd(_mm256_mul_pd(_mm256_set1_pd(G), mi),
                                                                  _mm256_mul_pd(mj, _mm256_mul_pd(invDist, invDist)));

                                        __m256d fx = _mm256_mul_pd(F, _mm256_mul_pd(dx, invDist));
                                        __m256d fy = _mm256_mul_pd(F, _mm256_mul_pd(dy, invDist));
                                        __m256d fz = _mm256_mul_pd(F, _mm256_mul_pd(dz, invDist));

                                        double fx_arr[4], fy_arr[4], fz_arr[4];
                                        _mm256_storeu_pd(fx_arr, fx);
                                        _mm256_storeu_pd(fy_arr, fy);
                                        _mm256_storeu_pd(fz_arr, fz);

                                        for (size_t k = 0; k < vec_len; ++k)
                                        {
                                                ax_private[i] += fx_arr[k] / Bodies.M[i];
                                                ay_private[i] += fy_arr[k] / Bodies.M[i];
                                                az_private[i] += fz_arr[k] / Bodies.M[i];

                                                ax_private[j + k] -= fx_arr[k] / Bodies.M[j + k];
                                                ay_private[j + k] -= fy_arr[k] / Bodies.M[j + k];
                                                az_private[j + k] -= fz_arr[k] / Bodies.M[j + k];
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
                        Bodies.X[i] += Bodies.VX[i] * SIM_DT + 0.5 * ax[i] * SIM_DT * SIM_DT;
                        Bodies.Y[i] += Bodies.VY[i] * SIM_DT + 0.5 * ay[i] * SIM_DT * SIM_DT;
                        Bodies.Z[i] += Bodies.VZ[i] * SIM_DT + 0.5 * az[i] * SIM_DT * SIM_DT;
                }

#pragma omp parallel
                {
                        double ax_private[N] = {0}, ay_private[N] = {0}, az_private[N] = {0};

#pragma omp for schedule(dynamic)
                        for (size_t i = 0; i < N; ++i)
                        {
                                __m256d xi = _mm256_set1_pd(Bodies.X[i]);
                                __m256d yi = _mm256_set1_pd(Bodies.Y[i]);
                                __m256d zi = _mm256_set1_pd(Bodies.Z[i]);
                                __m256d mi = _mm256_set1_pd(Bodies.M[i]);

                                for (size_t j = i + 1; j < N; j += 4)
                                {
                                        size_t rem = N - j;
                                        size_t vec_len = rem >= 4 ? 4 : rem;

                                        __m256d xj = _mm256_loadu_pd(&Bodies.X[j]);
                                        __m256d yj = _mm256_loadu_pd(&Bodies.Y[j]);
                                        __m256d zj = _mm256_loadu_pd(&Bodies.Z[j]);
                                        __m256d mj = _mm256_loadu_pd(&Bodies.M[j]);

                                        __m256d dx = _mm256_sub_pd(xj, xi);
                                        __m256d dy = _mm256_sub_pd(yj, yi);
                                        __m256d dz = _mm256_sub_pd(zj, zi);

                                        const double eps = 1e-3;
                                        __m256d eps2 = _mm256_set1_pd(eps * eps);

                                        __m256d d2 = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(dx, dx),
                                                                                 _mm256_mul_pd(dy, dy)),
                                                                   _mm256_add_pd(_mm256_mul_pd(dz, dz), eps2));

                                        __m256d invDist = _mm256_div_pd(_mm256_set1_pd(1.0), _mm256_sqrt_pd(d2));
                                        __m256d F = _mm256_mul_pd(_mm256_mul_pd(_mm256_set1_pd(G), mi),
                                                                  _mm256_mul_pd(mj, _mm256_mul_pd(invDist, invDist)));

                                        __m256d fx = _mm256_mul_pd(F, _mm256_mul_pd(dx, invDist));
                                        __m256d fy = _mm256_mul_pd(F, _mm256_mul_pd(dy, invDist));
                                        __m256d fz = _mm256_mul_pd(F, _mm256_mul_pd(dz, invDist));

                                        double fx_arr[4], fy_arr[4], fz_arr[4];
                                        _mm256_storeu_pd(fx_arr, fx);
                                        _mm256_storeu_pd(fy_arr, fy);
                                        _mm256_storeu_pd(fz_arr, fz);

                                        for (size_t k = 0; k < vec_len; ++k)
                                        {
                                                ax_private[i] += fx_arr[k] / Bodies.M[i];
                                                ay_private[i] += fy_arr[k] / Bodies.M[i];
                                                az_private[i] += fz_arr[k] / Bodies.M[i];

                                                ax_private[j + k] -= fx_arr[k] / Bodies.M[j + k];
                                                ay_private[j + k] -= fy_arr[k] / Bodies.M[j + k];
                                                az_private[j + k] -= fz_arr[k] / Bodies.M[j + k];
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
                        Bodies.VX[i] += 0.5 * (ax[i] + ax_new[i]) * SIM_DT;
                        Bodies.VY[i] += 0.5 * (ay[i] + ay_new[i]) * SIM_DT;
                        Bodies.VZ[i] += 0.5 * (az[i] + az_new[i]) * SIM_DT;

                        ax[i] = ax_new[i];
                        ay[i] = ay_new[i];
                        az[i] = az_new[i];
                }

#pragma omp parallel for schedule(dynamic)
                for (size_t i = 0; i < N; ++i)
                {
                        __m256d xi = _mm256_set1_pd(Bodies.X[i]);
                        __m256d yi = _mm256_set1_pd(Bodies.Y[i]);
                        __m256d zi = _mm256_set1_pd(Bodies.Z[i]);
                        __m256d vxi = _mm256_set1_pd(Bodies.VX[i]);
                        __m256d vyi = _mm256_set1_pd(Bodies.VY[i]);
                        __m256d vzi = _mm256_set1_pd(Bodies.VZ[i]);
                        __m256d mi = _mm256_set1_pd(Bodies.M[i]);
                        __m256d ri = _mm256_set1_pd(Bodies.R[i]);

                        for (size_t j = i + 1; j < N; j += 4)
                        {
                                size_t rem = N - j;
                                size_t vec_len = rem >= 4 ? 4 : rem;

                                __m256d xj = _mm256_loadu_pd(&Bodies.X[j]);
                                __m256d yj = _mm256_loadu_pd(&Bodies.Y[j]);
                                __m256d zj = _mm256_loadu_pd(&Bodies.Z[j]);
                                __m256d vxj = _mm256_loadu_pd(&Bodies.VX[j]);
                                __m256d vyj = _mm256_loadu_pd(&Bodies.VY[j]);
                                __m256d vzj = _mm256_loadu_pd(&Bodies.VZ[j]);
                                __m256d mj = _mm256_loadu_pd(&Bodies.M[j]);
                                __m256d rj = _mm256_loadu_pd(&Bodies.R[j]);

                                __m256d dx = _mm256_sub_pd(xj, xi);
                                __m256d dy = _mm256_sub_pd(yj, yi);
                                __m256d dz = _mm256_sub_pd(zj, zi);

                                __m256d dist2 = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(dx, dx),
                                                                            _mm256_mul_pd(dy, dy)),
                                                              _mm256_mul_pd(dz, dz));

                                __m256d minDist = _mm256_add_pd(ri, rj);
                                __m256d minDist2 = _mm256_mul_pd(minDist, minDist);

                                __m256d mask = _mm256_cmp_pd(dist2, minDist2, _CMP_LT_OS);

                                if (_mm256_movemask_pd(mask) != 0)
                                {
                                        double dx_arr[4], dy_arr[4], dz_arr[4];
                                        double dist2_arr[4], minDist_arr[4];
                                        _mm256_storeu_pd(dx_arr, dx);
                                        _mm256_storeu_pd(dy_arr, dy);
                                        _mm256_storeu_pd(dz_arr, dz);
                                        _mm256_storeu_pd(dist2_arr, dist2);
                                        _mm256_storeu_pd(minDist_arr, minDist);

                                        for (size_t k = 0; k < vec_len; ++k)
                                        {
                                                if (dist2_arr[k] < minDist_arr[k] * minDist_arr[k])
                                                {
                                                        double dist = sqrt(dist2_arr[k]);
                                                        double nx = dx_arr[k] / dist;
                                                        double ny = dy_arr[k] / dist;
                                                        double nz = dz_arr[k] / dist;

                                                        double rvx = Bodies.VX[j + k] - Bodies.VX[i];
                                                        double rvy = Bodies.VY[j + k] - Bodies.VY[i];
                                                        double rvz = Bodies.VZ[j + k] - Bodies.VZ[i];

                                                        double vAlongNormal = rvx * nx + rvy * ny + rvz * nz;
                                                        if (vAlongNormal < 0)
                                                        {
                                                                double m1 = Bodies.M[i];
                                                                double m2 = Bodies.M[j + k];
                                                                double e = 0.9;
                                                                double impulse = (1 + e) * vAlongNormal / (m1 + m2);
                                                                Bodies.VX[i] += impulse * m2 * nx;
                                                                Bodies.VY[i] += impulse * m2 * ny;
                                                                Bodies.VZ[i] += impulse * m2 * nz;
                                                                Bodies.VX[j + k] -= impulse * m1 * nx;
                                                                Bodies.VY[j + k] -= impulse * m1 * ny;
                                                                Bodies.VZ[j + k] -= impulse * m1 * nz;
                                                        }

                                                        double overlap = minDist_arr[k] - dist;
                                                        Bodies.X[i] -= nx * (overlap * Bodies.M[j + k] / (Bodies.M[i] + Bodies.M[j + k]));
                                                        Bodies.Y[i] -= ny * (overlap * Bodies.M[j + k] / (Bodies.M[i] + Bodies.M[j + k]));
                                                        Bodies.Z[i] -= nz * (overlap * Bodies.M[j + k] / (Bodies.M[i] + Bodies.M[j + k]));
                                                        Bodies.X[j + k] += nx * (overlap * Bodies.M[i] / (Bodies.M[i] + Bodies.M[j + k]));
                                                        Bodies.Y[j + k] += ny * (overlap * Bodies.M[i] / (Bodies.M[i] + Bodies.M[j + k]));
                                                        Bodies.Z[j + k] += nz * (overlap * Bodies.M[i] / (Bodies.M[i] + Bodies.M[j + k]));
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
                                snap[w][i].x = Bodies.X[i];
                                snap[w][i].y = Bodies.Y[i];
                                snap[w][i].z = Bodies.Z[i];
                        }
                        atomic_store(&snap_idx, w);
                        timer = 1;
                }

                Running = atomic_load(pRunning);
        }
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
        Scene->Player.Position.Z = -(1e6 + 2.5e5);
        Scene->Player.RunSpeed *= 10000;
        Scene->Player.WalkSpeed *= 1000;

        ENTITY *K = LiObj(Scene, LithiumLoadObject(Scene, "assets/tri.obj"));
        ENTITY *Star = LiObj(Scene, LithiumLoadObject(Scene, "assets/tri.obj"));

        for (size_t i = 0; i < K->TriCount; ++i)
        {
                K->Tris[i].col.r = 128;
                K->Tris[i].col.g = 128;
                K->Tris[i].col.b = 128;
                K->Tris[i].col.a = 255;
        }

        for (size_t i = 0; i < N - 1; ++i)
        {
                ENTITY *E = InitMesh(Scene, 0);
                *E = *K;
                da_append(Scene, E);
        }

        pthread_t thread;
        pthread_create(&thread, NULL, SimulationMain, &Scene->Running);

        ScaleMesh(Scene->items[1], 1000);

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
