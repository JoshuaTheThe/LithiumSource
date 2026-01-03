#include <engine/physics.h>

const double TERMINAL_VELOCITY = 100.0;
const double GRAVITY = 9.81;
const double FRICTION = 8.0;
const double PHYSICS_WAIT = 1.0;
const double JUMP_POWER = 0.8;

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
	VEC3 Epsilon = (VEC3){.X = 0.015, .Y = 0.015, .Z = 0.015};
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

static bool ResolveCollision(SCENE *Scene, Mesh3D *Mesh)
{
	BOUNDS player = GetPlayerBounds(Scene);
	double overlapX = 0.0, overlapY = 0.0, overlapZ = 0.0,
	       left, right, top, bottom, front, back,
	       minOverlap;
	bool collision = false;
	for (size_t i = 0; i < Mesh->tri_count; i++)
	{
		BOUNDS tri = GetTriangleBounds(&Mesh->tris[i], Mesh->origin);

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

		collision = true;
		if (fabs(overlapX) == minOverlap)
		{
			Scene->Camera.Position.X -= overlapX;
			if (fabs(Scene->Camera.Velocity.X) > 0.1)
				Scene->Camera.Velocity.X = 0;
		}
		else if (fabs(overlapY) == minOverlap)
		{
			Scene->Camera.Position.Y += overlapY;
			// Only stop vertical velocity if hitting from above or below
			if (overlapY > 0 && Scene->Camera.Velocity.Y < 0) // Landing on ground
			{
				Scene->Camera.Velocity.Y = 0;
				Scene->Grounded = true;
			}
			else if (overlapY < 0 && Scene->Camera.Velocity.Y > 0) // Hitting ceiling
			{
				Scene->Camera.Velocity.Y = 0;
			}
		}
		else if (fabs(overlapZ) == minOverlap)
		{
			Scene->Camera.Position.Z -= overlapZ;
			if (fabs(Scene->Camera.Velocity.Z) > 0.1)
				Scene->Camera.Velocity.Z = 0;
		}
	}

	return collision;
}

void PhysicsTick(SCENE *Scene)
{
	// Apply gravity and friction
	Scene->Camera.Velocity.X -= Scene->Camera.Velocity.X * FRICTION * Scene->dt * PHYSICS_WAIT;
	Scene->Camera.Velocity.Z -= Scene->Camera.Velocity.Z * FRICTION * Scene->dt * PHYSICS_WAIT;
	Scene->Camera.Velocity.Y -= GRAVITY * Scene->dt * PHYSICS_WAIT;

	if (Scene->Camera.Velocity.Y < -TERMINAL_VELOCITY)
		Scene->Camera.Velocity.Y = -TERMINAL_VELOCITY;

	Scene->Grounded = false;

	// Sub-stepping for smooth collision
	const double maxStep = 0.01;
	double moveX = Scene->Camera.Velocity.X * Scene->dt * PHYSICS_WAIT;
	double moveZ = Scene->Camera.Velocity.Z * Scene->dt * PHYSICS_WAIT;
	double moveY = Scene->Camera.Velocity.Y * Scene->dt * PHYSICS_WAIT;

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
		Scene->Camera.Position.X += dx;
		for (size_t j = 0; j < Scene->count; ++j)
			ResolveCollision(Scene, Scene->items[j]);

		Scene->Camera.Position.Z += dz;
		for (size_t j = 0; j < Scene->count; ++j)
			ResolveCollision(Scene, Scene->items[j]);

		Scene->Camera.Position.Y += dy;
		for (size_t j = 0; j < Scene->count; ++j)
			ResolveCollision(Scene, Scene->items[j]);
	}
}