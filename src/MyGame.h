#ifndef __MY_GAME_H__
#define __MY_GAME_H__

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#include "SDL.h"
#include "SDL_image.h"

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
    SDL_Texture* spriteTexture;
    double velocityX = 0;
    double velocityY = 0;
    double simXOffset = 0;
    double simYOffset = 0;
    bool facingRight = true;
    int health = 100;
    int maxHealth = 100; // Mana/Stamina/Charge could be defined in a hashMap of a new struct statData with a current value and max value // This is a lot more complicated, so the first tests for replication will just rely on health // Values like damage don't need to be sent, health of relevant entities (ones with visible health bars (all of them at the moment)) will be updated via server messages so sending damage would be pointless // It does mean that when lagging, health will decrease with a delay. Fixing that would require sending in damage, and doing clientside collision handling.
    bool isReady = false;
    //bool isMe = false; // Makes this player instance stand out as THIS client's player // aka "Is that player mine?"

    // Velocity could be included later for latency simulation / switching movement to being done clientside
    //double velocityX = 0;
    //double velocityY = 0;

    void setPlayerClass(PlayerClasses playerClass) {
        if (this->playerClass == playerClass) return; // Only run when the new player class is different so we don't unload the texture for no reason
        this->playerClass = playerClass;
        spriteTexture = nullptr;
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

    void setPosition(int x, int y) {
        entity.x = x;
        entity.y = y;
    }

    void setVelocity(double velX, double velY) {
        velocityX = velX;
        velocityY = velY;
    }

    void setSimOffsets(double simX, double simY) {
        simXOffset = simX;
        simYOffset = simY;
    }

    // Manipulate the entity rectangle and return one with the proper offsets
    SDL_Rect getRect() {
        SDL_Rect sprite = { entity.x + simXOffset - 12, entity.y + simYOffset - 12, entity.w, entity.h };
        setSimOffsets(0, 0);
        //std::cout << sprite.x << sprite.y << sprite.w << sprite.h << "\n";
        return sprite;
    }

    void checkTexture(SDL_Renderer* renderer) {
        if (spriteTexture == nullptr) {
            SDL_Surface* tempSurface;
            switch (playerClass) {
            case PlayerClasses::KNIGHT:
                tempSurface = IMG_Load("Assets/Textures/Knight.png");
                break;
            case PlayerClasses::RANGER:
                tempSurface = IMG_Load("Assets/Textures/Ranger.png");
                break;
            case PlayerClasses::MAGE:
                tempSurface = IMG_Load("Assets/Textures/Mage.png");
                break;
            default:
                tempSurface = IMG_Load("Assets/Textures/BlankPlayer.png");
                break;
            }
            spriteTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
            SDL_FreeSurface(tempSurface);
        }
    }

    void render(SDL_Renderer* renderer) {
        /*
        if (spriteTexture == nullptr) {
            SDL_Surface* tempSurface;
            switch (playerClass) {
                case PlayerClasses::KNIGHT:
                    tempSurface = IMG_Load("Assets/Textures/Knight.png");
                    break;
                case PlayerClasses::RANGER:
                    tempSurface = IMG_Load("Assets/Textures/Ranger.png");
                    break;
                case PlayerClasses::MAGE:
                    tempSurface = IMG_Load("Assets/Textures/Mage.png");
                    break;
                default:
                    tempSurface = IMG_Load("Assets/Textures/BlankPlayer.png");
                    break;
            }
            spriteTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
            SDL_FreeSurface(tempSurface);
        }*/
        checkTexture(renderer);
        if (spriteTexture != nullptr) {
            SDL_Rect srcRect = { 0, 0, 20, 20 };
            SDL_RenderCopyEx(renderer, spriteTexture, &srcRect, &getRect(), 0, nullptr, facingRight ? SDL_RendererFlip::SDL_FLIP_NONE : SDL_RendererFlip::SDL_FLIP_HORIZONTAL);
        }
    }
};

struct MyPlayerData : public PlayerData {
    bool isHoldingLeft = false;
    bool isHoldingRight = false;
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
private:
    bool ready = false; // Flags if the game is ready to start running
    bool running = true; // Flags if the game is actively running in the main state (outside of ends creen UI and game lobby) // separates the game loop running from the game itself wanting to run in main (main->is_running)
public:
    bool isReady() { return ready; }
    void setReady(bool isReady) { ready = isReady; }
    bool isRunning() { return running; }
    void setRunning(bool isRunning) { running = isRunning; }
    //https://www.geeksforgeeks.org/cpp/how-to-use-hashmap-in-cpp - hashMaps in C++
    // NOTE - Since the max number of players can be capped, playerMap can work just as well when converted to a PlayerData* array[MAX_PLAYERS], skipping the need for hashing algorythms and saving on a ton of memory use for the exact same functionality. playerMap was initially an unordered_map<> so I wouldn't have to worry about fixed size limits. npcMap however should stay as a list type because npc can come and go (at least that would be true if the game was 100% finished, as of writing, there are no minion spawns mechanics in the game, so if the final version only has a single boss per game, npcMap could be replaces with an NPCData* boss variable to save on memory and processing time) // But for now I'm leaving all the features open-ended so working with them is as simple as possible. Optimising flexible code like this is a lot easier so it can be done towards the end of the project if I have to.
    std::unordered_map<int, PlayerData*> playerMap; // List of players // The same reference is used both in gameplay and UI // Player class (and related values) and ready status are set in UI, then remain fixed when switching to gameplay
    std::unordered_map<int, NPCData*> npcMap; // 
} game_data;

class MyGame {

    private:
        //static const int MAX_PLAYERS = 4;
        //SDL_Rect player1 = { 200, 0, 20, 60 };
        //SDL_Rect player2 = { 600, 0, 20, 60 }; // New - Player 2
        //SDL_Rect ball = { 0, 0, 20, 20 }; // New - Ball
        //PlayerData* myPlayer = nullptr; // Stores a PlayerData reference as this client's player for easy access and client-specific functions // i.e simulating the client player locally, highlighting client's player in UI, etc
    public:
        static const int MAX_PLAYERS = 4;
        std::vector<std::string> messages;
        MyPlayerData* myPlayer = nullptr;

        void on_receive(std::string message, std::vector<std::string>& args);
        void send(std::string message);
        void input(SDL_Event& event);
        //void clickInput(SDL_Event& event); // New - for mouse inputs
        void update(); // Update all clientside objects at 60fps
        void updateSimulated(double tpf); // new - Simulate updates for objects that are actively updated by the server
        void render(SDL_Renderer* renderer);
        //GameData getGameData();
        GameData* getGameData(); // Return a pointer instead
        //MyGame() {
            //std::cout << "class: " << (PlayerClasses(3) == PlayerClasses::RANGER) << std::endl; // Since we can convert int to enum, we can send player class type as int/short // Since data is sent as a string, different int types don't really matter, only whole vs decimals
            //std::cout << "class: " << (PlayerClasses(3) == PlayerClasses::MAGE) << std::endl;
        //}
};

#endif