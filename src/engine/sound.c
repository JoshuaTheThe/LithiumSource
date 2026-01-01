#include <engine/sound.h>

size_t FindEmptySound(SCENE *Scene)
{
        for (size_t i = 0; i < sizeof(Scene->Sounds) / sizeof(Scene->Sounds[0]); ++i)
        {
                if (Scene->Sounds[i].Valid)
                        continue;
                return i;
        }

        TODO();
}

size_t LoadSound(SCENE *Scene, const char *Path)
{
        size_t idx = FindEmptySound(Scene);
        Scene->Sounds[idx].Sample = Mix_LoadWAV(Path);
        Scene->Sounds[idx].Valid = true;
        Scene->Sounds[idx].Playing = false;
        Scene->Sounds[idx].Channel = -1;

        if (!Scene->Sounds[idx].Sample)
        {
                TODO();
        }

        return idx;
}

void CleanupSound(SCENE *Scene)
{
        for (size_t i = 0; i < sizeof(Scene->Sounds) / sizeof(Scene->Sounds[0]); ++i)
        {
                if (!Scene->Sounds[i].Valid || !Scene->Sounds[i].Sample)
                        continue;
                Mix_FreeChunk(Scene->Sounds[i].Sample);
                Scene->Sounds[i].Sample = NULL;
                Scene->Sounds[i].Valid = false;
                Scene->Sounds[i].Playing = false;
                Scene->Sounds[i].Channel = -1;
        }
        Mix_CloseAudio();
}

void PlaySound(SCENE *Scene, size_t SoundIdx)
{
        if (SoundIdx > sizeof(Scene->Sounds) / sizeof(Scene->Sounds[0]) || !Scene->Sounds[SoundIdx].Sample || !Scene->Sounds[SoundIdx].Valid)
                TODO();
        int channel = Mix_PlayChannel(-1, Scene->Sounds[SoundIdx].Sample, 0);
        Scene->Sounds[SoundIdx].Playing = true;
        Scene->Sounds[SoundIdx].Channel = channel;
}

void UpdateSounds(SCENE *Scene)
{
        for (size_t i = 0; i < sizeof(Scene->Sounds) / sizeof(Scene->Sounds[0]); ++i)
        {
                if (!Scene->Sounds[i].Valid || Scene->Sounds[i].Channel == -1)
                        continue;
                Scene->Sounds[i].Playing = Mix_Playing(Scene->Sounds[i].Channel) != 0;
                if (!Scene->Sounds[i].Playing)
                        Scene->Sounds[i].Channel = -1;
        }
}
