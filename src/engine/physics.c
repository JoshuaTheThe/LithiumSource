#include <engine/physics.h>

const double TERMINAL_VELOCITY = 55.0;
const double GRAVITY = 9.81;
const double FRICTION = 8.0;
const double JUMP_POWER = 0.8;

static BOUNDS GetPlayerBounds(SCENE *Scene)
{
        BOUNDS b = Scene->Player.Bounds;

        b.Min.X += Scene->Player.Position.X;
        b.Min.Y += Scene->Player.Position.Y;
        b.Min.Z += Scene->Player.Position.Z;

        b.Max.X += Scene->Player.Position.X;
        b.Max.Y += Scene->Player.Position.Y;
        b.Max.Z += Scene->Player.Position.Z;

        return b;
}

static inline BOUNDS GetTriangleBounds(const TRI3D *Tri, VEC3 Origin, VEC3 Rotation)
{
        BOUNDS b;
        VEC3 Epsilon = (VEC3){.X = 0.001, .Y = 0.001, .Z = 0.001};

        VEC3 p0 = RotatePoint(&Tri->p[0], &Rotation);
        p0 = AddVec3(&p0, &Origin);

        b.Min = b.Max = p0;

        for (int i = 1; i < 3; i++)
        {
                VEC3 p = RotatePoint(&Tri->p[i], &Rotation);
                p = AddVec3(&p, &Origin);

                b.Min.X = fmin(b.Min.X, p.X);
                b.Min.Y = fmin(b.Min.Y, p.Y);
                b.Min.Z = fmin(b.Min.Z, p.Z);

                b.Max.X = fmax(b.Max.X, p.X);
                b.Max.Y = fmax(b.Max.Y, p.Y);
                b.Max.Z = fmax(b.Max.Z, p.Z);
        }

        b.Min = SubVec3(&b.Min, &Epsilon);
        b.Max = AddVec3(&b.Max, &Epsilon);

        return b;
}

static bool AABBOverlap(BOUNDS a, BOUNDS b)
{
        return (a.Max.X > b.Min.X && a.Min.X < b.Max.X &&
                a.Max.Y > b.Min.Y && a.Min.Y < b.Max.Y &&
                a.Max.Z > b.Min.Z && a.Min.Z < b.Max.Z);
}

static bool ResolveEntityCollision(SCENE *Scene, ENTITY *A, ENTITY *B)
{
        if (!A || !B || !A->IsVisible || !B->IsVisible || !A->IsCollidable || !B->IsCollidable || (A == B))
                return false;

        BOUNDS boundsA = A->InteractionBounds;
        boundsA.Min = AddVec3(&boundsA.Min, &A->Origin);
        boundsA.Max = AddVec3(&boundsA.Max, &A->Origin);

        bool collision = false;

        for (size_t i = 0; i < B->TriCount; i++)
        {
                BOUNDS tri = GetTriangleBounds(&B->Tris[i], B->Origin, B->Rotation);

                if (!AABBOverlap(boundsA, tri))
                        continue;

                double overlapX = 0.0, overlapY = 0.0, overlapZ = 0.0;
                double left = 0.0, right = 0.0, top = 0.0, bottom = 0.0, front = 0.0, back = 0.0;

                if (boundsA.Max.X > tri.Min.X && boundsA.Min.X < tri.Max.X)
                {
                        left = tri.Max.X - boundsA.Min.X;
                        right = boundsA.Max.X - tri.Min.X;
                        overlapX = (left < right) ? left : -right;
                }

                if (boundsA.Max.Y > tri.Min.Y && boundsA.Min.Y < tri.Max.Y)
                {
                        bottom = tri.Max.Y - boundsA.Min.Y;
                        top = boundsA.Max.Y - tri.Min.Y;
                        overlapY = (bottom < top) ? bottom : -top;
                }

                if (boundsA.Max.Z > tri.Min.Z && boundsA.Min.Z < tri.Max.Z)
                {
                        front = tri.Max.Z - boundsA.Min.Z;
                        back = boundsA.Max.Z - tri.Min.Z;
                        overlapZ = (front < back) ? front : -back;
                }

                double minOverlap = fmin(fmin(fabs(overlapX), fabs(overlapY)), fabs(overlapZ));

                if (minOverlap <= 0.0)
                        continue;

                VEC3 Overlap = {.X = overlapX, .Y = overlapY, .Z = overlapZ};
                collision = true;

                if (B->CustomCollisionBehaviour)
                {
                        B->CustomCollisionBehaviour(B, Scene, Overlap);
                }
                else
                {
                        if (fabs(overlapX) == minOverlap)
                        {
                                if (!A->IsStatic)
                                        A->Origin.X += overlapX * 0.5;
                                if (!B->IsStatic)
                                        B->Origin.X -= overlapX * 0.5;

                                if (fabs(A->Velocity.X) > 0.1)
                                        A->Velocity.X = 0;
                        }
                        else if (fabs(overlapY) == minOverlap)
                        {
                                if (!A->IsStatic)
                                        A->Origin.Y += overlapY * 0.5;
                                if (!B->IsStatic)
                                        B->Origin.Y -= overlapY * 0.5;

                                if (overlapY > 0)
                                {
                                        if (A->Velocity.Y < 0)
                                        {
                                                A->Velocity.Y = 0;
                                                A->IsGrounded = true;
                                        }
                                        else
                                                A->IsGrounded = false;
                                }
                        }
                        else if (fabs(overlapZ) == minOverlap)
                        {
                                if (!A->IsStatic)
                                        A->Origin.Z += overlapZ * 0.5;
                                if (!B->IsStatic)
                                        B->Origin.Z -= overlapZ * 0.5;

                                if (fabs(A->Velocity.Z) > 0.1)
                                        A->Velocity.Z = 0;
                        }
                }
        }

        return collision;
}

static bool ResolveCollision(SCENE *Scene, ENTITY *Mesh)
{
        if (!Mesh->IsCollidable)
                return false;
        BOUNDS player = GetPlayerBounds(Scene);
        double overlapX = 0.0, overlapY = 0.0, overlapZ = 0.0;
        double left = 0.0, right = 0.0, top = 0.0, bottom = 0.0, front = 0.0, back = 0.0;
        double minOverlap = 0.0;

        bool collision = false;
        for (size_t i = 0; i < Mesh->TriCount; i++)
        {
                BOUNDS tri = GetTriangleBounds(&Mesh->Tris[i], Mesh->Origin, Mesh->Rotation);

                if (!AABBOverlap(player, tri))
                        continue;

                if (player.Max.X > tri.Min.X && player.Min.X < tri.Max.X)
                {
                        left = tri.Max.X - player.Min.X;
                        right = player.Max.X - tri.Min.X;
                        overlapX = (left < right) ? left : -right;
                }

                if (player.Max.Y > tri.Min.Y && player.Min.Y < tri.Max.Y)
                {
                        bottom = tri.Max.Y - player.Min.Y;
                        top = player.Max.Y - tri.Min.Y;
                        overlapY = (bottom < top) ? bottom : -top;
                }

                if (player.Max.Z > tri.Min.Z && player.Min.Z < tri.Max.Z)
                {
                        front = tri.Max.Z - player.Min.Z;
                        back = player.Max.Z - tri.Min.Z;
                        overlapZ = (front < back) ? front : -back;
                }

                minOverlap = fmin(fmin(fabs(overlapX), fabs(overlapY)), fabs(overlapZ));

                if (minOverlap <= 0.0)
                {
                        continue;
                }

                VEC3 Overlap = {.X = overlapX, .Y = overlapY, .Z = overlapZ};

                collision = true;
                if (Mesh->CustomCollisionBehaviour)
                        Mesh->CustomCollisionBehaviour(Mesh, Scene, Overlap);
                else
                {

                        if (fabs(overlapX) == minOverlap)
                        {
                                Scene->Player.Position.X += overlapX;
                                if (fabs(Scene->Player.Velocity.X) > 0.1)
                                        Scene->Player.Velocity.X = 0;
                        }
                        else if (fabs(overlapY) == minOverlap)
                        {
                                Scene->Player.Position.Y += overlapY;
                                if (overlapY > 0 && Scene->Player.Velocity.Y < 0)
                                {
                                        Scene->Player.Velocity.Y = 0;
                                        Scene->Player.Grounded = true;
                                }
                                else if (overlapY < 0 && Scene->Player.Velocity.Y > 0)
                                {
                                        Scene->Player.Velocity.Y = 0;
                                }
                        }
                        else if (fabs(overlapZ) == minOverlap)
                        {
                                Scene->Player.Position.Z += overlapZ;
                                if (fabs(Scene->Player.Velocity.Z) > 0.1)
                                        Scene->Player.Velocity.Z = 0;
                        }
                }
        }

        return collision;
}

void PhysicsTick(SCENE *Scene)
{
        if (!Scene)
                return;
        Scene->Player.Velocity.X -= Scene->Player.Velocity.X * FRICTION * Scene->dt;
        Scene->Player.Velocity.Z -= Scene->Player.Velocity.Z * FRICTION * Scene->dt;
        Scene->Player.Velocity.Y -= GRAVITY * Scene->dt;

        if (Scene->Player.Velocity.Y < -TERMINAL_VELOCITY)
                Scene->Player.Velocity.Y = -TERMINAL_VELOCITY;

        Scene->Player.Grounded = false;

        const double maxStep = 0.01;
        double moveX = Scene->Player.Velocity.X * Scene->dt;
        double moveZ = Scene->Player.Velocity.Z * Scene->dt;
        double moveY = Scene->Player.Velocity.Y * Scene->dt;

        int stepsX = (int)ceil(fabs(moveX) / maxStep);
        int stepsZ = (int)ceil(fabs(moveZ) / maxStep);
        int stepsY = (int)ceil(fabs(moveY) / maxStep);
        int steps = fmax(fmax(stepsX, stepsZ), stepsY);
        if (steps < 1)
                steps = 1;

        double dx = moveX / steps;
        double dz = moveZ / steps;
        double dy = moveY / steps;

        for (int i = 0; i < steps; ++i)
        {
                Scene->Player.Position.X += dx;
                for (size_t j = 0; j < Scene->count; ++j)
                        ResolveCollision(Scene, Scene->items[j]);

                Scene->Player.Position.Z += dz;
                for (size_t j = 0; j < Scene->count; ++j)
                        ResolveCollision(Scene, Scene->items[j]);

                Scene->Player.Position.Y += dy;
                for (size_t j = 0; j < Scene->count; ++j)
                        ResolveCollision(Scene, Scene->items[j]);
        }

        for (size_t i = 0; i < Scene->count; ++i)
        {
                if (Scene->items[i]->IsStatic)
                        continue;
                Scene->items[i]->Velocity.X -= Scene->items[i]->Velocity.X * FRICTION * Scene->dt;
                Scene->items[i]->Velocity.Z -= Scene->items[i]->Velocity.Z * FRICTION * Scene->dt;
                Scene->items[i]->Velocity.Y -= GRAVITY * Scene->dt;
                if (Scene->items[i]->Velocity.Y < -TERMINAL_VELOCITY)
                        Scene->items[i]->Velocity.Y = -TERMINAL_VELOCITY;

                Scene->items[i]->Origin.X += Scene->items[i]->Velocity.X * Scene->dt;
                for (size_t j = 0; j < Scene->count; ++j)
                        ResolveEntityCollision(Scene, Scene->items[i], Scene->items[j]);
                Scene->items[i]->Origin.Y += Scene->items[i]->Velocity.Y * Scene->dt;
                for (size_t j = 0; j < Scene->count; ++j)
                        ResolveEntityCollision(Scene, Scene->items[i], Scene->items[j]);
                Scene->items[i]->Origin.Z += Scene->items[i]->Velocity.Z * Scene->dt;
                for (size_t j = 0; j < Scene->count; ++j)
                        ResolveEntityCollision(Scene, Scene->items[i], Scene->items[j]);
        }
}
