#include <engine/mesh.h>

Mesh3D *InitMesh(size_t triCount)
{
        Mesh3D *mesh = malloc(sizeof(Mesh3D));
        mesh->tris = malloc(sizeof(TRI3D) * triCount);
        mesh->tri_count = triCount;
        mesh->origin = (VEC3){0.0, 0.0, 0.0};
        mesh->Scale.X = 1.0;
        mesh->Scale.Y = 1.0;
        mesh->Scale.Z = 1.0;
        return mesh;
}

void DelMesh(Mesh3D *mesh)
{
        free(mesh->tris);
        free(mesh);
}

bool LoadMeshFromFile(const char *fileName, Mesh3D *mesh)
{
        FILE *fp = fopen(fileName, "r");
        if (fp == NULL)
                return false;

        if (mesh->tris)
                free(mesh->tris);
        char line[128];
        size_t vertex_count = 0, face_count = 0;
        VEC3 *vertices = NULL;
        TRI3D *triangles = NULL;

        while (fgets(line, sizeof(line), fp))
        {
                if (line[0] == 'v' && line[1] == ' ')
                {
                        vertex_count++;
                }
                else if (line[0] == 'f' && line[1] == ' ')
                {
                        face_count++;
                }
        }

        vertices = (VEC3 *)malloc(vertex_count * sizeof(VEC3));
        triangles = (TRI3D *)malloc(face_count * sizeof(TRI3D));

        if (vertices == NULL || triangles == NULL)
        {
                free(vertices);
                free(triangles);
                fclose(fp);
                return false;
        }

        rewind(fp);
        size_t vertex_index = 0, face_index = 0;

        while (fgets(line, sizeof(line), fp))
        {
                if (line[0] == 'v' && line[1] == ' ')
                {
                        VEC3 v;
                        sscanf(line, "v %lf %lf %lf", &v.X, &v.Y, &v.Z);
                        vertices[vertex_index++] = v;
                }
                else if (line[0] == 'f' && line[1] == ' ')
                {
                        int v1, v2, v3;
                        sscanf(line, "f %d %d %d", &v1, &v2, &v3);
                        triangles[face_index].p[0] = vertices[v1 - 1];
                        triangles[face_index].p[1] = vertices[v2 - 1];
                        triangles[face_index].p[2] = vertices[v3 - 1];
                        triangles[face_index].col.r = rand() & 255;
                        triangles[face_index].col.g = rand() & 255;
                        triangles[face_index].col.b = rand() & 255;
                        face_index++;
                }
        }

        // Populate the mesh structure
        mesh->tris = triangles;
        mesh->tri_count = face_count;
        mesh->origin = (VEC3){0.0, 0.0, 0.0}; // Set origin to zero

        // Cleanup
        free(vertices);
        fclose(fp);

        return true;
}
