#include <engine/interaction.h>

Mesh3D *CastRay(SCENE *Scene, RAY3D Ray)
{
        Mesh3D *closest_mesh = NULL;
        double closest_t = INFINITY;

        for (size_t i = 0; i < Scene->count; i++)
        {
                Mesh3D *mesh = Scene->items[i];
                if (!mesh)
                        continue;

                VEC3 local_pos = {
                    .X = Ray.InitialPos.X - mesh->Origin.X,
                    .Y = Ray.InitialPos.Y - mesh->Origin.Y,
                    .Z = Ray.InitialPos.Z - mesh->Origin.Z};

                VEC3 local_dir = Ray.InitialDir;

                if (mesh->Scale.X != 0 && mesh->Scale.Y != 0 && mesh->Scale.Z != 0)
                {
                        local_pos.X /= mesh->Scale.X;
                        local_pos.Y /= mesh->Scale.Y;
                        local_pos.Z /= mesh->Scale.Z;
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

                        if (t >= 0 && t < closest_t)
                        {
                                closest_t = t;
                                closest_mesh = mesh;
                        }
                }
        }

        return closest_mesh;
}