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

bool PlayerCollides(SCENE *Scene, TRI3D *Tri, VEC3 Origin)
{
	if (!Scene || !Tri)
		return false;
	BOUNDS player_bounds = Scene->Camera.Bounds;
	player_bounds.Min.X += Scene->Camera.Position.X;
	player_bounds.Min.Y += Scene->Camera.Position.Y;
	player_bounds.Min.Z += Scene->Camera.Position.Z;
	player_bounds.Max.X += Scene->Camera.Position.X;
	player_bounds.Max.Y += Scene->Camera.Position.Y;
	player_bounds.Max.Z += Scene->Camera.Position.Z;
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

bool LoadMeshFromFile(const char *fileName, Mesh3D *mesh)
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
	free(FullPath);

	if (mesh->tris)
		free(mesh->tris);

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

	printf("OBJ stats: %zu vertices, %zu texture coordinates, %zu faces\n",
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
			int v1, v2, v3;
			int t1 = 0, t2 = 0, t3 = 0;
			int n1 = 0, n2 = 0, n3 = 0;

			TRI3D *tri = &triangles[face_index];

			// Reset to defaults
			tri->col.r = rand() & 255;
			tri->col.g = rand() & 255;
			tri->col.b = rand() & 255;
			tri->Texture = NULL;

			if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
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

				printf("Face %zu: Has texture coordinates\n", face_index);
			}
			else if (sscanf(line, "f %d/%d %d/%d %d/%d",
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

				printf("Face %zu: Has texture coordinates (no normals)\n", face_index);
			}
			else if (sscanf(line, "f %d %d %d", &v1, &v2, &v3) == 3)
			{
				tri->p[0] = vertices[v1 - 1];
				tri->p[1] = vertices[v2 - 1];
				tri->p[2] = vertices[v3 - 1];

				tri->uv[0] = (UV){0.0, 0.0};
				tri->uv[1] = (UV){1.0, 0.0};
				tri->uv[2] = (UV){0.0, 1.0};

				printf("Face %zu: No texture coordinates - using defaults\n", face_index);
			}
			else
			{
				printf("WARNING: Could not parse face line: %s", line);
				continue;
			}

			if (face_index < 5)
			{
				printf("  UVs: (%.2f,%.2f) (%.2f,%.2f) (%.2f,%.2f)\n",
				       tri->uv[0].u, tri->uv[0].v,
				       tri->uv[1].u, tri->uv[1].v,
				       tri->uv[2].u, tri->uv[2].v);
			}

			face_index++;
		}
	}

	mesh->tris = triangles;
	mesh->tri_count = face_index;
	mesh->origin = (VEC3){0.0, 0.0, 0.0};

	printf("Loaded mesh with %zu triangles\n", mesh->tri_count);

	size_t zero_uv_count = 0;
	for (size_t i = 0; i < mesh->tri_count; i++)
	{
		if (mesh->tris[i].uv[0].u == 0.0 && mesh->tris[i].uv[0].v == 0.0 &&
		    mesh->tris[i].uv[1].u == 0.0 && mesh->tris[i].uv[1].v == 0.0 &&
		    mesh->tris[i].uv[2].u == 0.0 && mesh->tris[i].uv[2].v == 0.0)
		{
			zero_uv_count++;
		}
	}

	if (zero_uv_count == mesh->tri_count)
	{
		printf("WARNING: All triangles have zero UV coordinates!\n");
		printf("The OBJ file probably doesn't have texture coordinates.\n");
		printf("You need to either:\n");
		printf("1. Use an OBJ file with 'vt' lines\n");
		printf("2. Generate UV coordinates manually\n");
	}

	free(vertices);
	free(texcoords);
	fclose(fp);
	return true;
}

void ScaleMesh(Mesh3D *Mesh, double s)
{
	for (size_t x = 0; x < Mesh->tri_count; ++x)
	{
		for (size_t y = 0; y < 3; ++y)
		{
			Mesh->tris[x].p[y].X *= s;
			Mesh->tris[x].p[y].Y *= s;
			Mesh->tris[x].p[y].Z *= s;
		}
	}
}
