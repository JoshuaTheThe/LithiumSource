#include <engine/interaction.h>

static inline VEC3 RotateVectorInverse(const VEC3 *v, const VEC3 *rotation)
{
        double rx = -rotation->X * M_PI / 180.0;
        double ry = -rotation->Y * M_PI / 180.0;
        double rz = -rotation->Z * M_PI / 180.0;

        double sinX = sin(rx), cosX = cos(rx);
        double sinY = sin(ry), cosY = cos(ry);
        double sinZ = sin(rz), cosZ = cos(rz);

        double m[3][3] = {
            {cosY * cosZ, sinX * sinY * cosZ - cosX * sinZ, cosX * sinY * cosZ + sinX * sinZ},
            {cosY * sinZ, sinX * sinY * sinZ + cosX * cosZ, cosX * sinY * sinZ - sinX * cosZ},
            {-sinY, sinX * cosY, cosX * cosY}};

        return (VEC3){
            v->X * m[0][0] + v->Y * m[1][0] + v->Z * m[2][0],
            v->X * m[0][1] + v->Y * m[1][1] + v->Z * m[2][1],
            v->X * m[0][2] + v->Y * m[1][2] + v->Z * m[2][2]};
}

ENTITY *CastRay(SCENE *Scene, RAY3D Ray)
{
        ENTITY *closest_mesh = NULL;
        double closest_t = INFINITY;

        for (size_t i = 0; i < Scene->count; i++)
        {
                ENTITY *mesh = Scene->items[i];
                if (!mesh)
                        continue;

                VEC3 local_pos, local_dir;

                local_pos = SubVec3(&Ray.InitialPos, &mesh->Origin);

                if (mesh->Rotation.X != 0.0 || mesh->Rotation.Y != 0.0 || mesh->Rotation.Z != 0.0)
                {
                        local_pos = RotateVectorInverse(&local_pos, &mesh->Rotation);
                        local_dir = RotateVectorInverse(&Ray.InitialDir, &mesh->Rotation);
                }
                else
                {
                        local_dir = Ray.InitialDir;
                }

                if (mesh->Scale.X != 1.0 || mesh->Scale.Y != 1.0 || mesh->Scale.Z != 1.0)
                {
                        if (mesh->Scale.X != 0.0)
                                local_pos.X /= mesh->Scale.X;
                        if (mesh->Scale.Y != 0.0)
                                local_pos.Y /= mesh->Scale.Y;
                        if (mesh->Scale.Z != 0.0)
                                local_pos.Z /= mesh->Scale.Z;

                        if (mesh->Scale.X != 0.0)
                                local_dir.X /= mesh->Scale.X;
                        if (mesh->Scale.Y != 0.0)
                                local_dir.Y /= mesh->Scale.Y;
                        if (mesh->Scale.Z != 0.0)
                                local_dir.Z /= mesh->Scale.Z;

                        local_dir = NormaliseVec3(&local_dir);
                }

                double t_min = 0.0;
                double t_max = Scene->Player.MaxInteraction;

                for (int axis = 0; axis < 3; axis++)
                {
                        double axis_pos, axis_dir, min_bound, max_bound;

                        if (axis == 0)
                        {
                                axis_pos = local_pos.X;
                                axis_dir = local_dir.X;
                                min_bound = mesh->InteractionBounds.Min.X;
                                max_bound = mesh->InteractionBounds.Max.X;
                        }
                        else if (axis == 1)
                        {
                                axis_pos = local_pos.Y;
                                axis_dir = local_dir.Y;
                                min_bound = mesh->InteractionBounds.Min.Y;
                                max_bound = mesh->InteractionBounds.Max.Y;
                        }
                        else
                        {
                                axis_pos = local_pos.Z;
                                axis_dir = local_dir.Z;
                                min_bound = mesh->InteractionBounds.Min.Z;
                                max_bound = mesh->InteractionBounds.Max.Z;
                        }
                        if (fabs(axis_dir) < 1e-8)
                        {
                                if (axis_pos < min_bound || axis_pos > max_bound)
                                {
                                        t_min = INFINITY;
                                        t_max = -INFINITY;
                                        break;
                                }
                        }
                        else
                        {
                                double t1 = (min_bound - axis_pos) / axis_dir;
                                double t2 = (max_bound - axis_pos) / axis_dir;

                                if (t1 > t2)
                                {
                                        double temp = t1;
                                        t1 = t2;
                                        t2 = temp;
                                }

                                if (t1 > t_min)
                                        t_min = t1;
                                if (t2 < t_max)
                                        t_max = t2;

                                if (t_min > t_max)
                                {
                                        t_min = INFINITY;
                                        t_max = -INFINITY;
                                        break;
                                }

                                if (t_max < 0)
                                {
                                        t_min = INFINITY;
                                        t_max = -INFINITY;
                                        break;
                                }
                        }
                }

                if (t_min <= t_max && t_max >= 0)
                {
                        double t = (t_min < 0) ? t_max : t_min;

                        if (mesh->Scale.X != 1.0 || mesh->Scale.Y != 1.0 || mesh->Scale.Z != 1.0)
                        {
                                double avg_scale = (fabs(mesh->Scale.X) + fabs(mesh->Scale.Y) + fabs(mesh->Scale.Z)) / 3.0;
                                t *= avg_scale;
                        }

                        if (t >= 0 && t < closest_t)
                        {
                                closest_t = t;
                                closest_mesh = mesh;
                        }
                }
        }

        return closest_mesh;
}