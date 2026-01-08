#include <engine/texture.h>

TEXTURE *LoadTexture(const char *path)
{
        size_t sz = strnlen(path, 512) + strnlen(ProgramPath, 512) + 2;
        char *FullPath = calloc(1, sz);
        if (!FullPath)
                TODO();
        snprintf(FullPath, sz, "%s/%s", ProgramPath, path);
        SDL_Surface *surface = SDL_LoadBMP(FullPath);
        if (!surface)
        {
                printf("ERROR: Failed to load texture from %s: %s\n", FullPath, SDL_GetError());
                return NULL;
        }

        SDL_Surface *converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        if (converted != surface)
        {
                SDL_FreeSurface(surface);
                surface = converted;
        }

        TEXTURE *tex = malloc(sizeof(TEXTURE));
        tex->width = surface->w;
        tex->height = surface->h;
        tex->pitch = surface->pitch;
        tex->pixels = malloc(tex->width * tex->height * sizeof(uint32_t));
        memcpy(tex->pixels, surface->pixels, tex->width * tex->height * sizeof(uint32_t));

        SDL_FreeSurface(surface);
        printf("INFO: Loaded Texture from %s at %p\n", FullPath, tex);
        free(FullPath);
        return tex;
}

void FreeTextureData(TEXTURE *tex)
{
        if (tex)
        {
                free(tex->pixels);
                free(tex);
                printf("INFO: Deleted Texture at %p\n", tex);
        }
}

COLOUR SampleTexture(TEXTURE *tex, UV uv)
{
        static int sample_count = 0;
        double u = uv.u;
        double v = uv.v;
        int texWidthMinusOne = tex->width - 1;
        int texHeightMinusOne = tex->height - 1;
        int x, y;
        size_t index, max_index;
        uint32_t pixel;
        COLOUR result;
        if (!tex)
                return (COLOUR){255, 0, 255};
        if (!tex->pixels)
                return (COLOUR){255, 255, 0};
        if (tex->width <= 0 || tex->height <= 0)
                return (COLOUR){0, 255, 255};
        if (u < 0.0)
                u = 0.0;
        if (u > 1.0)
                u = 1.0;
        if (v < 0.0)
                v = 0.0;
        if (v > 1.0)
                v = 1.0;
        if (texWidthMinusOne <= 0)
                texWidthMinusOne = 1;
        if (texHeightMinusOne <= 0)
                texHeightMinusOne = 1;
        x = (int)(u * texWidthMinusOne);
        y = (int)(v * texHeightMinusOne);
        if (x < 0)
                x = 0;
        if (x >= tex->width)
                x = tex->width - 1;
        if (y < 0)
                y = 0;
        if (y >= tex->height)
                y = tex->height - 1;
        index = (size_t)y * tex->width + x;
        max_index = (size_t)tex->width * tex->height;

        if (index >= max_index)
        {
                return (COLOUR){255, 0, 0};
        }

        pixel = tex->pixels[index];

        if (sample_count < 10)
        {
                sample_count++;
        }

        result.r = (pixel >> 0) & 0xFF;
        result.g = (pixel >> 8) & 0xFF;
        result.b = (pixel >> 16) & 0xFF;
        return result;
}