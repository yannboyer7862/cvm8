#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>
#ifdef SDL2_FROM_HOMEBREW
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include "Consts.hpp"
#include "VM.hpp"

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        std::cerr << "[ERROR] No ROM provided !\n";
        std::cerr << "[INFO] Usage : ./cvm8 my_game.ch8/my_game.rom\n";
        return EXIT_FAILURE;
    }

    VM chip8VM;
    std::string romPath = std::string(argv[1]);

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "[ERROR] Unable to initialize SDL2 !\n";
        return EXIT_FAILURE;
    }

    try {
        chip8VM.InitVMAudio();
        chip8VM.LoadROMFromFile(romPath);
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow("CVM8 - by Yann BOYER", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == nullptr) {
        std::cerr << "[ERROR] Unable to initialize the window !\n";
        return EXIT_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    if (renderer == nullptr) {
        std::cerr << "[ERROR] Unable to initialize the renderer !\n";
        return EXIT_FAILURE;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderPresent(renderer);

    bool isRunning = true;

    uint8_t divCycles = 0;

    while (isRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    break;
                case SDL_KEYUP:
                    break;
                case SDL_QUIT:
                    isRunning = false;
                    std::cout << "[INFO] Exiting...\n";
                    break;
            }
        }

        chip8VM.ExecuteSingleVirtualCPUInstruction();
        divCycles++;

        // Draw
        for (uint8_t y = 0; y < CHIP8_SCREEN_HEIGHT; y++) {
            for (uint8_t x = 0; x < CHIP8_SCREEN_WIDTH; x++) {
                SDL_Rect pixel = { x * PIXEL_SCALE_FACTOR, y * PIXEL_SCALE_FACTOR, PIXEL_SCALE_FACTOR, PIXEL_SCALE_FACTOR };
                if (chip8VM.IsPixelOn_FromRT(x, y))
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                else
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                SDL_RenderFillRect(renderer, &pixel);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_RenderClear(renderer); // Avoid slowdowns.

        if (divCycles == TIMER_CLOCK_DIVISION) {
            divCycles = 0;
            chip8VM.UpdateVirtualCPUTimers();
        }

        std::this_thread::sleep_for(std::chrono::microseconds(CPU_CLOCK_DELAY));
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
