#include <engine/physics.h>

const double TERMINAL_VELOCITY = 100.0;
const double GRAVITY = 0.0981;
const double FRICTION = 0.8;
const double PHYSICS_WAIT = 1.0;
const double JUMP_POWER = 5.0;

static BOUNDS GetPlayerBounds(SCENE *Scene)
{
        BOUNDS b = Scene->Camera.Bounds;

        b.Min.X += Scene->Camera.Position.X;
        b.Min.Y += Scene->Camera.Position.Y;
        b.Min.Z += Scene->Camera.Position.Z;

        b.Max.X += Scene->Camera.Position.X;
        b.Max.Y += Scene->Camera.Position.Y;
        b.Max.Z += Scene->Camera.Position.Z;

        return b;
}

static BOUNDS GetTriangleBounds(const TRI3D *Tri, VEC3 Origin)
{
        BOUNDS b;
        VEC3 Epsilon = (VEC3){.X = 0.1, .Y = 0.1, .Z = 0.1};
        b.Min = b.Max = Tri->p[0];

        for (int i = 1; i < 3; i++)
        {
                b.Min.X = fmin(b.Min.X, Tri->p[i].X);
                b.Min.Y = fmin(b.Min.Y, Tri->p[i].Y);
                b.Min.Z = fmin(b.Min.Z, Tri->p[i].Z);

                b.Max.X = fmax(b.Max.X, Tri->p[i].X);
                b.Max.Y = fmax(b.Max.Y, Tri->p[i].Y);
                b.Max.Z = fmax(b.Max.Z, Tri->p[i].Z);
        }

        b.Min = AddVec3(&b.Min, &Origin);
        b.Max = AddVec3(&b.Max, &Origin);
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

static void ResolveAxis(
    SCENE *Scene,
    Mesh3D *Mesh,
    AXIS axis)
{
        BOUNDS player = GetPlayerBounds(Scene);

        for (size_t i = 0; i < Mesh->tri_count; i++)
        {
                BOUNDS tri = GetTriangleBounds(&Mesh->tris[i], Mesh->origin);

                if (!AABBOverlap(player, tri))
                        continue;

                if (axis == AXIS_X)
                {
                        if (Scene->Camera.Velocity.X > 0)
                                Scene->Camera.Position.X -= (player.Max.X - tri.Min.X);
                        else
                                Scene->Camera.Position.X += (tri.Max.X - player.Min.X);

                        Scene->Camera.Velocity.X = 0;
                }

                else if (axis == AXIS_Z)
                {
                        if (Scene->Camera.Velocity.Z > 0)
                                Scene->Camera.Position.Z -= (player.Max.Z - tri.Min.Z);
                        else
                                Scene->Camera.Position.Z += (tri.Max.Z - player.Min.Z);

                        Scene->Camera.Velocity.Z = 0;
                }

                else if (axis == AXIS_Y)
                {
                        if (Scene->Camera.Velocity.Y > 0)
                                Scene->Camera.Position.Y -= (player.Max.Y - tri.Min.Y);
                        else
                                Scene->Camera.Position.Y += (tri.Max.Y - player.Min.Y);

                        Scene->Camera.Velocity.Y = 0;
                }

                player = GetPlayerBounds(Scene);
        }
}

static bool PointInTriangleXZ(VEC3 p, VEC3 a, VEC3 b, VEC3 c)
{
        // Project points to XZ plane
        double px = p.X, pz = p.Z;
        double ax = a.X, az = a.Z;
        double bx = b.X, bz = b.Z;
        double cx = c.X, cz = c.Z;

        double area = 0.5 * (-bz * cx + az * (-bx + cx) + ax * (bz - cz) + bx * cz);
        double s = 1 / (2 * area) * (az * cx - ax * cz + (cz - az) * px + (ax - cx) * pz);
        double t = 1 / (2 * area) * (ax * bz - az * bx + (az - bz) * px + (bx - ax) * pz);

        return s >= 0 && t >= 0 && (s + t) <= 1;
}

static bool ResolveGround(SCENE *Scene, Mesh3D *Mesh)
{
        bool grounded = false;
        BOUNDS player = GetPlayerBounds(Scene);
        double feetY = player.Min.Y;

        for (size_t i = 0; i < Mesh->tri_count; i++)
        {
                TRI3D *t = &Mesh->tris[i];
                VEC3 p0 = AddVec3(&t->p[0], &Mesh->origin);
                VEC3 p1 = AddVec3(&t->p[1], &Mesh->origin);
                VEC3 p2 = AddVec3(&t->p[2], &Mesh->origin);

                VEC3 l1 = SubVec3(&p1, &p0);
                VEC3 l2 = SubVec3(&p2, &p0);
                VEC3 n = CrossProdVec3(&l1, &l2);

                double max_step = Scene->Camera.Velocity.Y * Scene->dt;
                if (LenVec3(&n) < 1e-6 || n.Y > -max_step)
                        continue;

                n = NormaliseVec3(&n);
                double d = -(n.X * p0.X + n.Y * p0.Y + n.Z * p0.Z);
                double dist = n.X * Scene->Camera.Position.X + n.Y * feetY + n.Z * Scene->Camera.Position.Z + d;

                if (dist < 0.0 && dist > -0.3)
                {
                        if (PointInTriangleXZ((VEC3){Scene->Camera.Position.X, 0, Scene->Camera.Position.Z}, p0, p1, p2))
                        {
                                grounded = true;
                        }
                }
        }

        return grounded;
}

void PhysicsTick(SCENE *Scene, Mesh3D *Ground)
{
        Scene->Camera.Velocity.X -= Scene->Camera.Velocity.X * FRICTION * Scene->dt * PHYSICS_WAIT;
        Scene->Camera.Velocity.Z -= Scene->Camera.Velocity.Z * FRICTION * Scene->dt * PHYSICS_WAIT;
        Scene->Camera.Velocity.Y -= GRAVITY * Scene->dt * PHYSICS_WAIT;
        Scene->Camera.Position.X += Scene->Camera.Velocity.X * Scene->dt * PHYSICS_WAIT;
        ResolveAxis(Scene, Ground, AXIS_X);
        Scene->Camera.Position.Z += Scene->Camera.Velocity.Z * Scene->dt * PHYSICS_WAIT;
        ResolveAxis(Scene, Ground, AXIS_Z);
        Scene->Camera.Position.Y += Scene->Camera.Velocity.Y * Scene->dt * PHYSICS_WAIT;
        Scene->Grounded = ResolveGround(Scene, Ground);
        ResolveAxis(Scene, Ground, AXIS_Y);
}
