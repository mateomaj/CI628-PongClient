#ifndef __MY_GAME_H__
#define __MY_GAME_H__

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#include "SDL.h"

/*
static struct GameData {
    int player1Y = 0;
    int player2Y = 0;
    int ballX = 0;
    int ballY = 0;
} game_data;
*/

//https://www.geeksforgeeks.org/cpp/enum-classes-in-c-and-their-advantage-over-enum-datatype
enum class PlayerClasses {
    NONE, KNIGHT, RANGER, MAGE
};

enum class NPCClasses {
    NONE, FORUMAN
};

struct PlayerData {
    PlayerClasses playerClass = PlayerClasses::NONE; // Player class reference for sprites and other values
    //int playerX = 0;
    //int playerY = 0;
    SDL_Rect entity = {0, 0, 40, 40};
    int health = 0;
    int maxHealth = 0; // Mana/Stamina/Charge could be defined in a hashMap of a new struct statData with a current value and max value // This is a lot more complicated, so the first tests for replication will just rely on health // Values like damage don't need to be sent, health of relevant entities (ones with visible health bars (all of them at the moment)) will be updated via server messages so sending damage would be pointless // It does mean that when lagging, health will decrease with a delay. Fixing that would require sending in damage, and doing clientside collision handling.
    //bool isMe = false; // Makes this player instance stand out as THIS client's player // aka "Is that player mine?"

    // Velocity could be included later for latency simulation / switching movement to being done clientside
    //double velocityX = 0;
    //double velocityY = 0;

    void setPlayerClass(PlayerClasses playerClass) {
        this->playerClass = playerClass;
        switch (playerClass) {
            case PlayerClasses::KNIGHT:
                health = 250;
                maxHealth = 250;
                break;
            case PlayerClasses::RANGER:
                health = 175;
                maxHealth = 175;
                break;
            case PlayerClasses::MAGE:
                health = 150;
                maxHealth = 150;
                break;
            default:
                health = 100;
                maxHealth = 100;
                break;
        }
    }

    void setPos(int x, int y) {
        entity.x = x;
        entity.y = y;
    }

    // Manipulate the entity rectangle and return one with the proper offsets
    SDL_Rect getRect() {
        SDL_Rect sprite = { entity.x, entity.y, entity.w, entity.h };
        //std::cout << sprite.x << sprite.y << sprite.w << sprite.h << "\n";
        return sprite;
    }
};

struct NPCData {
    NPCClasses npcClass = NPCClasses::NONE;
    SDL_Rect entity = {0, 0, 196, 176};

    void setPos(int x, int y) {
        entity.x = x;
        entity.y = y;
    }
};

static struct GameData {
    bool ready = false;
    //https://www.geeksforgeeks.org/cpp/how-to-use-hashmap-in-cpp - hashMaps in C++
    std::unordered_map<int, PlayerData*> playerMap; // List of players // The same reference is used both in gameplay and UI // Player class (and related values) and ready status are set in UI, then remain fixed when switching to gameplay
    std::unordered_map<int, NPCData*> npcMap; // 
} game_data;

class MyGame {

    private:
        static const int MAX_PLAYERS = 4;
        //SDL_Rect player1 = { 200, 0, 20, 60 };
        //SDL_Rect player2 = { 600, 0, 20, 60 }; // New - Player 2
        //SDL_Rect ball = { 0, 0, 20, 20 }; // New - Ball
        PlayerData* myPlayer = nullptr; // Stores a PlayerData reference as this client's player for easy access and client-specific functions // i.e simulating the client player locally, highlighting client's player in UI, etc
    public:
        std::vector<std::string> messages;

        void on_receive(std::string message, std::vector<std::string>& args);
        void send(std::string message);
        void input(SDL_Event& event);
        void update();
        void render(SDL_Renderer* renderer);
        //MyGame() {
            //std::cout << "class: " << (PlayerClasses(3) == PlayerClasses::RANGER) << std::endl; // Since we can convert int to enum, we can send player class type as int/short // Since data is sent as a string, different int types don't really matter, only whole vs decimals
            //std::cout << "class: " << (PlayerClasses(3) == PlayerClasses::MAGE) << std::endl;
        //}
};

#endif