#ifndef __MY_GAME_H__
#define __MY_GAME_H__

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "GameData.h"

class MyGame {

    private:
        bool quit = false;
    public:
        static const int MAX_PLAYERS = 4;
        int playerCount = 0; // Helps reliably track the active number of players. // Updated by on_receive() so there will probably be concurrency issues // At worst, the UI will sometimes get messed up for one frame if a player leaves mid-game
        std::vector<std::string> messages;
        MyPlayerData* myPlayer = nullptr; // A reference to existing player data that represets this specific client // Used for features relevant only to the player that you control

        void on_receive(std::string message, std::vector<std::string>& args);
        void send(std::string message);
        void input(SDL_Event& event);
        void update(double tpf); // Update all clientside objects based on local frame time
        void updateSimulated(double tpf); // Simulate updates for objects that are actively updated by the server // Edit - Simulations are done relative to the last time update data was sent / the last simulation hapenned.
        void updateSimulated(double tpf, char* updateMessage); // Simulates objects updated by the server // Reads and simulates data relative to the given update message
        void render(SDL_Renderer* renderer);
        void renderUI(SDL_Renderer* renderer);
        void forceQuit() { quit = true; }
        bool shouldForceQuit() { return quit; }
        GameData* getGameData(); // Return a pointer instead
        void spawnAttack(std::vector<std::string>& args);
        void spawnPlayerAttack(std::vector<std::string>& args);
        static long long getCurrentTimeMS() { return (_Xtime_get_ticks() / 10000); } // made it static so it can be accessed from anywhere
        void initTextures(SDL_Renderer* renderer);
        void onShutDown();
        ~MyGame();
};

#endif