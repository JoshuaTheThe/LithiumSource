#include <engine/tool.h>

size_t LithiumAddTool(SCENE *Scene, ENTITY *BindedEntity, void (*Hit)(struct TOOL *Self, ENTITY *Entity, SCENE *Scene, double Dist), void (*Fire)(struct TOOL *Self), void (*EndFire)(struct TOOL *Self), double Range)
{
        TOOL Tool;
        Tool.Entity = BindedEntity;
        Tool.Hit = Hit;
        Tool.Fire = Fire;
        Tool.EndFire = EndFire;
        Tool.Range = Range;
        if (BindedEntity)
        {
                BindedEntity->IsCollidable = false;
                BindedEntity->IsGrounded = false;
                BindedEntity->IsStatic = true;
                BindedEntity->IsVisible = false;
                BindedEntity->IsInteractable = false;
        }
        da_append(&Scene->Inventory, Tool);
        return Scene->Inventory.count - 1;
}

void LithiumClearInventory(SCENE *Scene)
{
        if (Scene && Scene->Inventory.items)
        {
                free(Scene->Inventory.items);
                Scene->Inventory.capacity = 0;
                Scene->Inventory.count = 0;
                Scene->Inventory.items = NULL;
                Scene->Player.CurrentTool = -1;
        }
}

void LithiumUpdateTools(SCENE *Scene)
{
        if (!Scene)
                return;
        for (size_t i = 0; i < Scene->Inventory.count; ++i)
        {
                RAY3D Ray, Ray2;
                const double yaw0 = DEG_TO_RAD(Scene->Player.Rotation.Y);
                const double pitch0 = DEG_TO_RAD(Scene->Player.Rotation.X);
                const VEC3 forward = {
                    cos(pitch0) * sin(yaw0),
                    sin(pitch0),
                    cos(pitch0) * cos(yaw0)};
                const double yaw1 = DEG_TO_RAD(Scene->Player.Rotation.Y + 90);
                const double pitch1 = DEG_TO_RAD(Scene->Player.Rotation.X);
                const VEC3 right = {
                    cos(pitch1) * sin(yaw1),
                    sin(pitch1),
                    cos(pitch1) * cos(yaw1)};
                Ray2.InitialDir = NormaliseVec3(&right);
                Ray2.InitialPos = Scene->Player.Position;
                CastRay(Scene, &Ray2, NULL, 0.1);
                Ray.InitialDir = NormaliseVec3(&forward);
                Ray.InitialPos = Ray2.Pos;
                CastRay(Scene, &Ray, NULL, 0.2);

                Scene->Inventory.items[i].Entity->Origin = Ray.Pos;
                Scene->Inventory.items[i].Entity->Rotation.Y = -Scene->Player.Rotation.Y;
                if (i == Scene->Player.CurrentTool)
                        Scene->Inventory.items[i].Entity->IsVisible = true;
                else
                        Scene->Inventory.items[i].Entity->IsVisible = false;
        }
}

void LithiumFire(SCENE *Scene)
{
        size_t ToolID = Scene->Player.CurrentTool;
        if (ToolID == (size_t)-1 || ToolID >= Scene->Inventory.count)
                return;
        double Dist;
        TOOL *Tool = &Scene->Inventory.items[ToolID];
        RAY3D Ray;
        const double yaw = DEG_TO_RAD(Scene->Player.Rotation.Y);
        const double pitch = DEG_TO_RAD(Scene->Player.Rotation.X);
        const VEC3 direction = {
            cos(pitch) * sin(yaw),
            sin(pitch),
            cos(pitch) * cos(yaw)};
        Ray.InitialDir = NormaliseVec3(&direction);
        Ray.InitialPos = Scene->Player.Position;
        ENTITY *Hit = CastRay(Scene, &Ray, &Dist, Tool->Range);
        if (Tool->Fire)
                Tool->Fire(Tool);
        if (Hit && Tool->Hit)
                Tool->Hit(Tool, Hit, Scene, Dist);
}

void LithiumEndFire(SCENE *Scene)
{
        size_t ToolID = Scene->Player.CurrentTool;
        if (ToolID == (size_t)-1 || ToolID >= Scene->Inventory.count)
                return;
        TOOL *Tool = &Scene->Inventory.items[ToolID];
        Tool->EndFire(Tool);
}
