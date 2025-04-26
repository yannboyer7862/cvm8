#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <SDL2/SDL_mixer.h>

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
