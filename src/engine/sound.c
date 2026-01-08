#include <engine/sound.h>

size_t FindEmptySound(SCENE *Scene)
{
        for (size_t i = 0; i < sizeof(Scene->SoundSys.Sounds) / sizeof(Scene->SoundSys.Sounds[0]); ++i)
        {
                if (Scene->SoundSys.Sounds[i].Valid)
                        continue;
                return i;
        }

        TODO();
}

size_t LoadSound(SCENE *Scene, const char *Path)
{
        size_t sz = strnlen(Path, 512) + strnlen(ProgramPath, 512) + 2;
        char *FullPath = calloc(1, sz);
        if (!FullPath)
                TODO();
        snprintf(FullPath, sz, "%s/%s", ProgramPath, Path);
        size_t idx = FindEmptySound(Scene);
        Scene->SoundSys.Sounds[idx].Sample = Mix_LoadWAV(FullPath);
        Scene->SoundSys.Sounds[idx].Valid = true;
        Scene->SoundSys.Sounds[idx].Playing = false;
        Scene->SoundSys.Sounds[idx].Channel = -1;

        if (!Scene->SoundSys.Sounds[idx].Sample)
        {
                TODO();
        }

        free(FullPath);
        return idx;
}

void CleanupSound(SCENE *Scene)
{
        for (size_t i = 0; i < sizeof(Scene->SoundSys.Sounds) / sizeof(Scene->SoundSys.Sounds[0]); ++i)
        {
                if (!Scene->SoundSys.Sounds[i].Valid || !Scene->SoundSys.Sounds[i].Sample)
                        continue;
                Mix_FreeChunk(Scene->SoundSys.Sounds[i].Sample);
                Scene->SoundSys.Sounds[i].Sample = NULL;
                Scene->SoundSys.Sounds[i].Valid = false;
                Scene->SoundSys.Sounds[i].Playing = false;
                Scene->SoundSys.Sounds[i].Channel = -1;
        }
        Mix_CloseAudio();
}

void PlaySound(SCENE *Scene, size_t SoundIdx)
{
        if (SoundIdx > sizeof(Scene->SoundSys.Sounds) / sizeof(Scene->SoundSys.Sounds[0]) || !Scene->SoundSys.Sounds[SoundIdx].Sample || !Scene->SoundSys.Sounds[SoundIdx].Valid)
                TODO();
        int channel = Mix_PlayChannel(-1, Scene->SoundSys.Sounds[SoundIdx].Sample, 0);
        Scene->SoundSys.Sounds[SoundIdx].Playing = true;
        Scene->SoundSys.Sounds[SoundIdx].Channel = channel;
}

void UpdateSounds(SCENE *Scene)
{
        for (size_t i = 0; i < sizeof(Scene->SoundSys.Sounds) / sizeof(Scene->SoundSys.Sounds[0]); ++i)
        {
                if (!Scene->SoundSys.Sounds[i].Valid || Scene->SoundSys.Sounds[i].Channel == -1)
                        continue;
                Scene->SoundSys.Sounds[i].Playing = Mix_Playing(Scene->SoundSys.Sounds[i].Channel) != 0;
                if (!Scene->SoundSys.Sounds[i].Playing)
                        Scene->SoundSys.Sounds[i].Channel = -1;
        }
}
