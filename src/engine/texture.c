#include <engine/texture.h>

TEXTURE *LoadTexture(const char *path)
{
        SDL_Surface *surface = SDL_LoadBMP(path);
        if (!surface)
        {
                printf("Failed to load texture: %s\n", SDL_GetError());
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
        printf("Loaded Texture\n");
        return tex;
}

void FreeTextureData(TEXTURE *tex)
{
        if (tex)
        {
                free(tex->pixels);
                free(tex);
        }
}

COLOUR SampleTexture(TEXTURE *tex, UV uv)
{
        if (!tex)
        {
                return (COLOUR){255, 0, 255};
        }

        if (!tex->pixels)
        {
                return (COLOUR){255, 255, 0};
        }

        if (tex->width <= 0 || tex->height <= 0)
        {
                return (COLOUR){0, 255, 255};
        }

        double u = uv.u;
        double v = uv.v;

        if (u < 0.0)
                u = 0.0;
        if (u > 1.0)
                u = 1.0;
        if (v < 0.0)
                v = 0.0;
        if (v > 1.0)
                v = 1.0;

        int texWidthMinusOne = tex->width - 1;
        int texHeightMinusOne = tex->height - 1;

        if (texWidthMinusOne <= 0)
                texWidthMinusOne = 1;
        if (texHeightMinusOne <= 0)
                texHeightMinusOne = 1;

        int x = (int)(u * texWidthMinusOne);
        int y = (int)(v * texHeightMinusOne);

        if (x < 0)
                x = 0;
        if (x >= tex->width)
                x = tex->width - 1;
        if (y < 0)
                y = 0;
        if (y >= tex->height)
                y = tex->height - 1;

        size_t index = (size_t)y * tex->width + x;
        size_t max_index = (size_t)tex->width * tex->height;

        if (index >= max_index)
        {
                return (COLOUR){255, 0, 0};
        }

        uint32_t pixel = tex->pixels[index];

        static int sample_count = 0;
        if (sample_count < 10)
        {
                sample_count++;
        }

        // Extract RGB - FIXED byte order!
        // SDL_PIXELFORMAT_RGBA32 on little-endian (x86) is actually ABGR in memory
        // Let's check what we actually have by examining the masks

        COLOUR result;

        // Option 1: Most likely for SDL on x86 - ABGR format
        // Byte order in memory: [B][G][R][A] (little-endian)
        result.r = (pixel >> 0) & 0xFF; // Third byte = Red in ABGR
        result.g = (pixel >> 8) & 0xFF;  // Second byte = Green
        result.b = (pixel >> 16) & 0xFF;         // First byte = Blue

        // Option 2: If that looks wrong, try RGBA format
        //result.r = (pixel >> 24) & 0xFF;  // First byte = Red in RGBA
        //result.g = (pixel >> 16) & 0xFF;  // Second byte = Green
        //result.b = (pixel >> 8) & 0xFF;   // Third byte = Blue

        return result;
}