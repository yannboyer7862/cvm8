#ifndef AUDIO_HPP
#define AUDIO_HPP

#ifdef SDL2_FROM_HOMEBREW
#include <SDL_mixer.h>
#else
#include <SDL2/SDL_mixer.h>
#endif

class AudioPlayer {
    public:
        AudioPlayer() = default;
        ~AudioPlayer();

        void Init();
        void PlayBeepSound();
    private:
        Mix_Chunk* m_beepSound;
};

#endif
