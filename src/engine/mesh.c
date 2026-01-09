#include <engine/mesh.h>

void DefaultInteractCallback(ENTITY *Self)
{
        printf("Hello! From %p\n", Self);
        return;
}

void DefaultPhysicsCallback(ENTITY *Self, SCENE *Scene)
{
        return;
}

ENTITY *InitMesh(SCENE *Scene, size_t triCount)
{
        ENTITY *mesh = calloc(1, sizeof(ENTITY));
        mesh->Tris = calloc(triCount, sizeof(TRI3D));
        mesh->TriCount = triCount;
        mesh->Origin = (VEC3){0.0, 0.0, 0.0};
        mesh->Scale.X = 1.0;
        mesh->Scale.Y = 1.0;
        mesh->Scale.Z = 1.0;

        mesh->Interact = DefaultInteractCallback;
        mesh->PhysicsIteration = DefaultPhysicsCallback;
        mesh->InteractSound = Scene->SoundSys.DenySelectSound;
        return mesh;
}

void DelMesh(ENTITY *Mesh)
{
        TEXTURES Textures = {0};
        for (size_t i = 0; i < Mesh->TriCount; ++i)
        {
                long Index = -1;
                da_find(&Textures, Mesh->Tris[i].Texture, Index);
                if (Index == -1)
                {
                        da_append(&Textures, Mesh->Tris[i].Texture);
                }
        }

        for (size_t i = 0; i < Textures.count; ++i)
        {
                FreeTextureData(Textures.items[i]);
        }

        free(Mesh->Tris);
        free(Mesh);
        printf("INFO: Free'd ENTITY at %p\n", Mesh);
}

bool PlayerCollides(SCENE *Scene, TRI3D *Tri, VEC3 Origin)
{
        if (!Scene || !Tri)
                return false;
        BOUNDS player_bounds = Scene->Player.Bounds;
        player_bounds.Min.X += Scene->Player.Position.X;
        player_bounds.Min.Y += Scene->Player.Position.Y;
        player_bounds.Min.Z += Scene->Player.Position.Z;
        player_bounds.Max.X += Scene->Player.Position.X;
        player_bounds.Max.Y += Scene->Player.Position.Y;
        player_bounds.Max.Z += Scene->Player.Position.Z;
        BOUNDS tri_bounds;

        tri_bounds.Min.X = Tri->p[0].X;
        tri_bounds.Min.Y = Tri->p[0].Y;
        tri_bounds.Min.Z = Tri->p[0].Z;
        tri_bounds.Max.X = Tri->p[0].X;
        tri_bounds.Max.Y = Tri->p[0].Y;
        tri_bounds.Max.Z = Tri->p[0].Z;

        for (int i = 1; i < 3; i++)
        {
                if (Tri->p[i].X < tri_bounds.Min.X)
                        tri_bounds.Min.X = Tri->p[i].X;
                if (Tri->p[i].Y < tri_bounds.Min.Y)
                        tri_bounds.Min.Y = Tri->p[i].Y;
                if (Tri->p[i].Z < tri_bounds.Min.Z)
                        tri_bounds.Min.Z = Tri->p[i].Z;
                if (Tri->p[i].X > tri_bounds.Max.X)
                        tri_bounds.Max.X = Tri->p[i].X;
                if (Tri->p[i].Y > tri_bounds.Max.Y)
                        tri_bounds.Max.Y = Tri->p[i].Y;
                if (Tri->p[i].Z > tri_bounds.Max.Z)
                        tri_bounds.Max.Z = Tri->p[i].Z;
        }

        bool x_overlap = (player_bounds.Max.X >= tri_bounds.Min.X + Origin.X) &&
                         (player_bounds.Min.X <= tri_bounds.Max.X + Origin.X);

        bool y_overlap = (player_bounds.Max.Y >= tri_bounds.Min.Y + Origin.Y) &&
                         (player_bounds.Min.Y <= tri_bounds.Max.Y + Origin.Y);

        bool z_overlap = (player_bounds.Max.Z >= tri_bounds.Min.Z + Origin.Z) &&
                         (player_bounds.Min.Z <= tri_bounds.Max.Z + Origin.Z);

        return (x_overlap && y_overlap && z_overlap);
}

bool LoadMeshFromFile(const char *fileName, ENTITY *mesh)
{
        size_t sz = strnlen(fileName, 512) + strnlen(ProgramPath, 512) + 2;
        char *FullPath = calloc(1, sz);
        if (!FullPath)
                TODO();
        snprintf(FullPath, sz, "%s/%s", ProgramPath, fileName);

        FILE *fp = fopen(FullPath, "r");
        if (fp == NULL)
        {
                free(FullPath);
                return false;
        }

        if (mesh->Tris)
                free(mesh->Tris);

        char line[256];
        size_t vertex_count = 0, face_count = 0, texcoord_count = 0;
        VEC3 *vertices = NULL;
        UV *texcoords = NULL;
        TRI3D *triangles = NULL;

        while (fgets(line, sizeof(line), fp))
        {
                if (line[0] == 'v')
                {
                        if (line[1] == ' ')
                                vertex_count++;
                        else if (line[1] == 't' && line[2] == ' ')
                                texcoord_count++;
                }
                else if (line[0] == 'f' && line[1] == ' ')
                {
                        face_count++;
                }
        }

        printf("INFO: OBJ stats: %zu vertices, %zu texture coordinates, %zu faces\n",
               vertex_count, texcoord_count, face_count);

        vertices = (VEC3 *)calloc(vertex_count, sizeof(VEC3));
        texcoords = (UV *)calloc(texcoord_count, sizeof(UV));
        triangles = (TRI3D *)calloc(face_count, sizeof(TRI3D));

        if (vertices == NULL || triangles == NULL ||
            (texcoord_count > 0 && texcoords == NULL))
        {
                free(vertices);
                free(texcoords);
                free(triangles);
                fclose(fp);
                return false;
        }

        rewind(fp);
        size_t vertex_index = 0, texcoord_index = 0, face_index = 0;

        while (fgets(line, sizeof(line), fp))
        {
                if (line[0] == 'v')
                {
                        if (line[1] == ' ')
                        {
                                VEC3 v;
                                if (sscanf(line, "v %lf %lf %lf", &v.X, &v.Y, &v.Z) == 3)
                                        vertices[vertex_index++] = v;
                        }
                        else if (line[1] == 't' && line[2] == ' ')
                        {
                                UV uv;
                                if (sscanf(line, "vt %lf %lf", &uv.u, &uv.v) == 2)
                                        texcoords[texcoord_index++] = uv;
                        }
                }
                else if (line[0] == 'f' && line[1] == ' ')
                {
                        size_t v1, v2, v3;
                        size_t t1 = 0, t2 = 0, t3 = 0;
                        size_t n1 = 0, n2 = 0, n3 = 0;

                        TRI3D *tri = &triangles[face_index];

                        tri->col.r = rand() & 255;
                        tri->col.g = rand() & 255;
                        tri->col.b = rand() & 255;
                        tri->col.a = 255;
                        tri->Texture = NULL;

                        if (sscanf(line, "f %ld/%ld/%ld %ld/%ld/%ld %ld/%ld/%ld",
                                   &v1, &t1, &n1,
                                   &v2, &t2, &n2,
                                   &v3, &t3, &n3) == 9)
                        {
                                tri->p[0] = vertices[v1 - 1];
                                tri->p[1] = vertices[v2 - 1];
                                tri->p[2] = vertices[v3 - 1];

                                if (t1 > 0 && t1 <= texcoord_count)
                                        tri->uv[0] = texcoords[t1 - 1];
                                else
                                        tri->uv[0] = (UV){0.0, 0.0};

                                if (t2 > 0 && t2 <= texcoord_count)
                                        tri->uv[1] = texcoords[t2 - 1];
                                else
                                        tri->uv[1] = (UV){0.0, 0.0};

                                if (t3 > 0 && t3 <= texcoord_count)
                                        tri->uv[2] = texcoords[t3 - 1];
                                else
                                        tri->uv[2] = (UV){0.0, 0.0};
                                // printf("INFO: Face %zu: Has texture coordinates\n", face_index);
                        }
                        else if (sscanf(line, "f %ld/%ld %ld/%ld %ld/%ld",
                                        &v1, &t1,
                                        &v2, &t2,
                                        &v3, &t3) == 6)
                        {
                                tri->p[0] = vertices[v1 - 1];
                                tri->p[1] = vertices[v2 - 1];
                                tri->p[2] = vertices[v3 - 1];

                                if (t1 > 0 && t1 <= texcoord_count)
                                        tri->uv[0] = texcoords[t1 - 1];
                                else
                                        tri->uv[0] = (UV){0.0, 0.0};

                                if (t2 > 0 && t2 <= texcoord_count)
                                        tri->uv[1] = texcoords[t2 - 1];
                                else
                                        tri->uv[1] = (UV){0.0, 0.0};

                                if (t3 > 0 && t3 <= texcoord_count)
                                        tri->uv[2] = texcoords[t3 - 1];
                                else
                                        tri->uv[2] = (UV){0.0, 0.0};

                                // printf("INFO: Face %zu: Has texture coordinates (no normals)\n", face_index);
                        }
                        else if (sscanf(line, "f %ld %ld %ld", &v1, &v2, &v3) == 3)
                        {
                                tri->p[0] = vertices[v1 - 1];
                                tri->p[1] = vertices[v2 - 1];
                                tri->p[2] = vertices[v3 - 1];

                                tri->uv[0] = (UV){0.0, 0.0};
                                tri->uv[1] = (UV){1.0, 0.0};
                                tri->uv[2] = (UV){0.0, 1.0};

                                // printf("INFO: Face %zu: No texture coordinates - using defaults\n", face_index);
                        }
                        else
                        {
                                printf("WARNING: Could not parse face line: %s", line);
                                continue;
                        }

                        // if (face_index < 5)
                        // {
                        //         printf("  UVs: (%.2f,%.2f) (%.2f,%.2f) (%.2f,%.2f)\n",
                        //                tri->uv[0].u, tri->uv[0].v,
                        //                tri->uv[1].u, tri->uv[1].v,
                        //                tri->uv[2].u, tri->uv[2].v);
                        // }

                        face_index++;
                }
        }

        mesh->Tris = triangles;
        mesh->TriCount = face_index;
        mesh->Origin = (VEC3){0.0, 0.0, 0.0};

        printf("INFO: Loaded mesh with %zu triangles\n", mesh->TriCount);

        size_t zero_uv_count = 0;
        for (size_t i = 0; i < mesh->TriCount; i++)
        {
                if (mesh->Tris[i].uv[0].u == 0.0 && mesh->Tris[i].uv[0].v == 0.0 &&
                    mesh->Tris[i].uv[1].u == 0.0 && mesh->Tris[i].uv[1].v == 0.0 &&
                    mesh->Tris[i].uv[2].u == 0.0 && mesh->Tris[i].uv[2].v == 0.0)
                {
                        zero_uv_count++;
                }
        }

        if (zero_uv_count == mesh->TriCount)
        {
                printf("WARNING: No tris have valid UV data, in mesh at %p, loaded from %s\n", mesh, FullPath);
        }

        free(vertices);
        free(texcoords);
        free(FullPath);
        fclose(fp);
        return true;
}

void ScaleMesh(ENTITY *Mesh, double s)
{
        for (size_t x = 0; x < Mesh->TriCount; ++x)
        {
                for (size_t y = 0; y < 3; ++y)
                {
                        Mesh->Tris[x].p[y].X *= s;
                        Mesh->Tris[x].p[y].Y *= s;
                        Mesh->Tris[x].p[y].Z *= s;
                }
        }
}
