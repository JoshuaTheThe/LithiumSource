#include <engine/draw.h>
#include <todo.h>

#define SWAP(a, b)               \
        {                        \
                typeof(a) t = a; \
                a = b;           \
                b = t;           \
        }

static double interpolateX(double y1, double y2, double x1, double x2, double y)
{
        if (y1 == y2)
                return x1;
        return x1 + (x2 - x1) * (y - y1) / (y2 - y1);
}

double interpolateDouble(int a, int b, double va, double vb, int x)
{
        if (a == b)
                return va;
        return va + (vb - va) * (x - a) / (b - a);
}

void PutPixel(SCENE *Scene, double X, double Y, double Z, COLOUR Col)
{
        if (!Scene || !Scene->Renderer.ZBuffer)
                TODO();
        if (X < 0.0 || X >= (double)Scene->Renderer.RendererWidth || Y < 0.0 || Y >= (double)Scene->Renderer.RendererHeight)
                return;
        size_t index = (int)Y * Scene->Renderer.RendererWidth + (int)X;
        if (Scene->Renderer.ZBuffer[index] < Z)
        {
                Scene->Renderer.ZBuffer[index] = Z;
                float alpha = (float)Col.a / 255.0f;
                if (alpha != 255.0)
                {
                        float inv_alpha = 1.0f - alpha;

                        COLOUR background = Scene->Renderer.RGBBuffer[index];
                        COLOUR blended;
                        blended.r = (Uint8)(Col.r * alpha + background.r * inv_alpha);
                        blended.g = (Uint8)(Col.g * alpha + background.g * inv_alpha);
                        blended.b = (Uint8)(Col.b * alpha + background.b * inv_alpha);
                        blended.a = 255;

                        Scene->Renderer.RGBBuffer[index] = blended;
                }
                else
                {
                        Scene->Renderer.RGBBuffer[index] = Col;
                }
                SDL_SetRenderDrawColor(Scene->Renderer.Renderer, Scene->Renderer.RGBBuffer[index].r, Scene->Renderer.RGBBuffer[index].g, Scene->Renderer.RGBBuffer[index].b, 255);
                SDL_RenderDrawPoint(Scene->Renderer.Renderer, (int)X, (int)Y);
        }
}

void DrawLine(SCENE *Scene, const VEC3 A, const VEC3 B, COLOUR Col)
{
        int x0 = (int)A.X;
        int y0 = (int)A.Y;
        double z0 = A.Z;

        int x1 = (int)B.X;
        int y1 = (int)B.Y;
        double z1 = B.Z;

        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;

        double total_steps = sqrt(dx * dx + dy * dy);
        double z_step = (total_steps > 0) ? (z1 - z0) / total_steps : 0;
        double current_z = z0;

        while (true)
        {
                PutPixel(Scene, x0, y0, current_z, Col);

                if (x0 == x1 && y0 == y1)
                        break;

                int e2 = err * 2;
                if (e2 > -dy)
                {
                        err -= dy;
                        x0 += sx;
                }
                if (e2 < dx)
                {
                        err += dx;
                        y0 += sy;
                }

                current_z += z_step;
        }
}

static double interpolatePerspective(double a, double b,
                                     double va, double vb,
                                     double wa, double wb,
                                     double x)
{
        if (a == b)
                return va;

        double t = (x - a) / (b - a);
        double invZ = (1.0 / wa) * (1.0 - t) + (1.0 / wb) * t;
        double uOverZ = va * (1.0 - t) + vb * t;
        return uOverZ / invZ;
}

void DrawTri(SCENE *Scene, TRI3D Tri)
{
        double temp_z, z_start, z_end, z;
        int temp_x, x_start, x_end, y0, y1, y2;

        if (Tri.p[1].Y < Tri.p[0].Y)
        {
                VEC3 temp = Tri.p[0];
                Tri.p[0] = Tri.p[1];
                Tri.p[1] = temp;
        }
        if (Tri.p[2].Y < Tri.p[0].Y)
        {
                VEC3 temp = Tri.p[0];
                Tri.p[0] = Tri.p[2];
                Tri.p[2] = temp;
        }
        if (Tri.p[2].Y < Tri.p[1].Y)
        {
                VEC3 temp = Tri.p[1];
                Tri.p[1] = Tri.p[2];
                Tri.p[2] = temp;
        }

        y0 = (int)Tri.p[0].Y;
        y1 = (int)Tri.p[1].Y;
        y2 = (int)Tri.p[2].Y;

        for (int y = y0; y <= y2; y++)
        {
                if (y < y1)
                {
                        x_start = interpolateX(y0, y1, (int)Tri.p[0].X, (int)Tri.p[1].X, y);
                        x_end = interpolateX(y0, y2, (int)Tri.p[0].X, (int)Tri.p[2].X, y);
                        z_start = interpolateDouble(y0, y1, Tri.p[0].Z, Tri.p[1].Z, y);
                        z_end = interpolateDouble(y0, y2, Tri.p[0].Z, Tri.p[2].Z, y);
                }
                else
                {
                        x_start = interpolateX(y1, y2, (int)Tri.p[1].X, (int)Tri.p[2].X, y);
                        x_end = interpolateX(y0, y2, (int)Tri.p[0].X, (int)Tri.p[2].X, y);
                        z_start = interpolateDouble(y1, y2, Tri.p[1].Z, Tri.p[2].Z, y);
                        z_end = interpolateDouble(y0, y2, Tri.p[0].Z, Tri.p[2].Z, y);
                }

                if (x_start > x_end)
                {
                        temp_x = x_start;
                        temp_z = z_start;
                        x_start = x_end;
                        x_end = temp_x;
                        z_start = z_end;
                        z_end = temp_z;
                }

                for (int x = x_start; x <= x_end; x++)
                {
                        z = interpolatePerspective(x_start, x_end, z_start, z_end, 1.0, 1.0, x);
                        PutPixel(Scene, x, y, z, Tri.col);
                }
        }
}

void DrawTriWTex(SCENE *Scene, TRI3D Tri)
{
        // Sort vertices by Y (v0.Y <= v1.Y <= v2.Y)
        if (Tri.p[1].Y < Tri.p[0].Y)
        {
                SWAP(Tri.p[0], Tri.p[1]);
                SWAP(Tri.uv[0], Tri.uv[1]);
                SWAP(Tri.w[0], Tri.w[1]);
        }
        if (Tri.p[2].Y < Tri.p[0].Y)
        {
                SWAP(Tri.p[0], Tri.p[2]);
                SWAP(Tri.uv[0], Tri.uv[2]);
                SWAP(Tri.w[0], Tri.w[2]);
        }
        if (Tri.p[2].Y < Tri.p[1].Y)
        {
                SWAP(Tri.p[1], Tri.p[2]);
                SWAP(Tri.uv[1], Tri.uv[2]);
                SWAP(Tri.w[1], Tri.w[2]);
        }

        double y0 = Tri.p[0].Y;
        double y1 = Tri.p[1].Y;
        double y2 = Tri.p[2].Y;

        // Precompute u/w and v/w for perspective-correct interpolation
        double u0w = Tri.uv[0].u * Tri.w[0], v0w = Tri.uv[0].v * Tri.w[0];
        double u1w = Tri.uv[1].u * Tri.w[1], v1w = Tri.uv[1].v * Tri.w[1];
        double u2w = Tri.uv[2].u * Tri.w[2], v2w = Tri.uv[2].v * Tri.w[2];

        for (double y = y0; y <= y2; y += 1.0)
        {
                if (y < 0 || y >= Scene->Renderer.RendererHeight)
                        continue;

                // Determine which segment (top or bottom)
                double t1 = (y1 != y0 && y < y1) ? (double)(y - y0) / (y1 - y0) : (y2 != y1 && y >= y1) ? (double)(y - y1) / (y2 - y1)
                                                                                                        : 0;
                double t2 = (double)(y - y0) / (y2 - y0);

                double x_start, x_end;
                double w_start, w_end;
                double u_start, u_end, v_start, v_end;

                if (y < y1)
                {
                        x_start = Tri.p[0].X + (Tri.p[1].X - Tri.p[0].X) * t1;
                        x_end = Tri.p[0].X + (Tri.p[2].X - Tri.p[0].X) * t2;

                        w_start = Tri.w[0] + (Tri.w[1] - Tri.w[0]) * t1;
                        w_end = Tri.w[0] + (Tri.w[2] - Tri.w[0]) * t2;

                        u_start = u0w + (u1w - u0w) * t1;
                        u_end = u0w + (u2w - u0w) * t2;

                        v_start = v0w + (v1w - v0w) * t1;
                        v_end = v0w + (v2w - v0w) * t2;
                }
                else
                {
                        x_start = Tri.p[1].X + (Tri.p[2].X - Tri.p[1].X) * t1;
                        x_end = Tri.p[0].X + (Tri.p[2].X - Tri.p[0].X) * t2;

                        w_start = Tri.w[1] + (Tri.w[2] - Tri.w[1]) * t1;
                        w_end = Tri.w[0] + (Tri.w[2] - Tri.w[0]) * t2;

                        u_start = u1w + (u2w - u1w) * t1;
                        u_end = u0w + (u2w - u0w) * t2;

                        v_start = v1w + (v2w - v1w) * t1;
                        v_end = v0w + (v2w - v0w) * t2;
                }

                // Swap if necessary to ensure x_start < x_end
                if (x_start > x_end)
                {
                        SWAP(x_start, x_end);
                        SWAP(w_start, w_end);
                        SWAP(u_start, u_end);
                        SWAP(v_start, v_end);
                }

                double xs = x_start;
                double xe = x_end;

                for (double x = xs; x <= xe; x += 1.0)
                {
                        if (x < 0 || x >= Scene->Renderer.RendererWidth)
                                continue;

                        double t = (xe != xs) ? (double)(x - xs) / (xe - xs) : 0;

                        double w = w_start + (w_end - w_start) * t;

                        double u = (u_start + (u_end - u_start) * t) / w;
                        double v = (v_start + (v_end - v_start) * t) / w;

                        UV uv = {u, v};
                        double z = 1.0 / w;

                        if (!Tri.Texture)
                        {
                                PutPixel(Scene, x, y, -z, Tri.col);
                                continue;
                        }
                        COLOUR col = SampleTexture(Tri.Texture, uv);
                        PutPixel(Scene, x, y, -z, col);
                }
        }
}

void PutPixelWTex(SCENE *Scene, double X, double Y, double Z,
                  UV Uv, TEXTURE *TextureData, double lum)
{
        if (!Scene || !TextureData)
                return;

        COLOUR texCol = SampleTexture(TextureData, Uv);

        texCol.r = (int)(texCol.r * lum);
        texCol.g = (int)(texCol.g * lum);
        texCol.b = (int)(texCol.b * lum);

        if (texCol.r > 255)
                texCol.r = 255;
        if (texCol.g > 255)
                texCol.g = 255;
        if (texCol.b > 255)
                texCol.b = 255;

        PutPixel(Scene, X, Y, Z, texCol);
}