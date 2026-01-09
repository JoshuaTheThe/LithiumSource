#include <engine/ui.h>

size_t LithiumCreateUXObject(SCENE *Scene, TEXTURE *Tex, VEC3 Origin, double Width, double Height)
{
        UXOBJECT *Object = calloc(1, sizeof(*Object));
        if (!Object)
        {
                printf("WARNING: Failed to create UX Object\n");
                return -1;
        }

        Object->p[0] = Origin;
        Object->p[1] = (VEC3){Origin.X, Origin.Y + Height, Origin.Z};
        Object->p[2] = (VEC3){Origin.X + Width, Origin.Y + Height, Origin.Z};
        Object->p[3] = (VEC3){Origin.X + Width, Origin.Y, Origin.Z};

        Object->Tex = Tex;
        da_append(&Scene->UXObjects, Object);
        return Scene->UXObjects.count - 1;
}

void LithiumDrawUXObject(SCENE *Scene, UXOBJECT *UX)
{
        if (!Scene)
                return;
        if (!UX)
        {
                printf("WARNING: Could not draw NULL UX Object\n");
                return;
        }

        TRI3D A = {{0}, .col = {255, 255, 255, 255}, .w = {-Scene->Player.Far - 1.0, -Scene->Player.Far - 1.0, -Scene->Player.Far - 1.0}, .Texture = UX->Tex},
              B = {{0}, .col = {255, 255, 255, 255}, .w = {-Scene->Player.Far - 1.0, -Scene->Player.Far - 1.0, -Scene->Player.Far - 1.0}, .Texture = UX->Tex};

        A.p[0] = UX->p[0];
        A.p[1] = UX->p[1];
        A.p[2] = UX->p[2];

        A.uv[0] = (UV){0.0, 0.0};
        A.uv[1] = (UV){0.0, 1.0};
        A.uv[2] = (UV){1.0, 1.0};

        B.p[0] = UX->p[0];
        B.p[1] = UX->p[2];
        B.p[2] = UX->p[3];

        B.uv[0] = (UV){0.0, 0.0};
        B.uv[1] = (UV){1.0, 1.0};
        B.uv[2] = (UV){1.0, 0.0};

        A.col.a = 255;
        B.col.a = 255;

        DrawTriWTex(Scene, A);
        DrawTriWTex(Scene, B);
}

void LithiumCleanupUXObject(UXOBJECT **UX)
{
        if (!UX || !(*UX))
        {
                printf("WARNING: Could not cleanup NULL UX Object\n");
                return;
        }

        FreeTextureData((*UX)->Tex);
        free(*UX);
}
