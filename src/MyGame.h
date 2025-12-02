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
    double x = 0;
    double y = 0;
    double velocityX = 0;
    double velocityY = 0;
    //double simXOffset = 0;
    //double simYOffset = 0;
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
        this->x = x;
        this->y = y;
    }

    void setPosition(double x, double y) {
        entity.x = x;
        entity.y = y;
        this->x = x;
        this->y = y;
    }

    void setVelocity(double velX, double velY) {
        velocityX = velX;
        velocityY = velY;
    }

    //void setSimOffsets(double simX, double simY) {
    //    simXOffset = simX;
    //    simYOffset = simY;
    //}

    void update(double tpf) {
        x += velocityX * tpf;
        //if (hasGravity) {
            for (; tpf > 0.016; tpf -= 0.016) {
                y += velocityY * 0.016;
                velocityY += -420 * 0.016;
            }
            y += velocityY * tpf;
            velocityY += -420 * tpf;
        //}
        //else {
        //    y += velocityY * tpf;
        //}
    }

    // Manipulate the entity rectangle and return one with the proper offsets
    SDL_Rect getRect() {
        //SDL_Rect sprite = { entity.x + simXOffset - 12, entity.y + simYOffset - 12, entity.w, entity.h };
        SDL_Rect sprite = { x - 12, fmin(540, y) - 12, entity.w, entity.h };
        //setSimOffsets(0, 0);
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

struct ProjectileData {
//class ProjectileData {
//public:
    // Found a cool problem where the projectile moved faster/slower than intended depending on which direction they were moving in. Turns out it's caused by truncation as SDL_Rect stores its values as integers // rounding down makes things move faster to the left and slower to the right as truncation removes decimals and moves the number towards 0. // Seeing the speed difference was enough to figure it out but another cool tell for it was that the projectiles changed speed when crossing x=0 on the left side of the screen.
    SDL_Rect entity = { 0, 0, 40, 40 };
    SDL_Rect sourceRect = { 0, 0, 0, 0 };
    SDL_Texture* spriteTexture = nullptr;
    char* spriteRef = "";
    SDL_Color* flatColor = nullptr;
    double x = 0; // Storing position separately to avoid truncation issues
    double y = 0;
    double velocityX = 0;
    double velocityY = 0;
    bool hasGravity = false;
    double rotation = 0; 
    bool markedForDespawn = false;

    void setPosition(int x, int y) {
        entity.x = x;
        entity.y = y;
        this->x = x;
        this->y = y;
    }

    void setPosition(double x, double y) {
        entity.x = x;
        entity.y = y;
        this->x = x;
        this->y = y;
    }

    void setVelocity(double velX, double velY) {
        velocityX = velX;
        velocityY = velY;
    }

    void checkTexture(SDL_Renderer* renderer) {
        if (spriteTexture == nullptr && spriteRef != nullptr) {
            SDL_Surface* tempSurface = IMG_Load(spriteRef);
            spriteTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
            SDL_FreeSurface(tempSurface);
        }
    }

    ProjectileData() {}

    ProjectileData(int attackID) {
        switch (attackID) {
            case 1:
            case 2:
                spriteRef = "Assets/Textures/angel sword.png";
                sourceRect = { 0, 0, 50, 16 };
                entity = { 0, 0, 50, 16 };
                break;
            case 3:
                spriteRef = "Assets/Textures/angel sword.png";
                sourceRect = { 0, 0, 50, 16 };
                entity = { 0, 0, 50, 2400 };
                break;
        }
    }

    void update(double tpf) { // Update uses the same logic as placeWithDelta() but in double format for deltaTime, it also includes extra logic to help with unload checks
        /*
        entity.x += velocityX * tpf;
        if (hasGravity) {
            for (; tpf > 0.016; tpf -= 0.016) {
                entity.y += velocityY * 0.016;
                //velocityY += game_data.GRAVITY * 0.016;
                velocityY += -420 * 0.016;
            }
            entity.y += velocityY * tpf;
            //velocityY += game_data.GRAVITY * tpf;
            velocityY += -420 * tpf;
        }
        else {
            entity.y += velocityY * tpf;
        }
        */
        x += velocityX * tpf;
        if (hasGravity) {
            for (; tpf > 0.016; tpf -= 0.016) {
                y += velocityY * 0.016;
                //velocityY += game_data.GRAVITY * 0.016;
                velocityY += -420 * 0.016;
            }
            y += velocityY * tpf;
            //velocityY += game_data.GRAVITY * tpf;
            velocityY += -420 * tpf;
        }
        else {
            y += velocityY * tpf;
        }
        double centerX = x + entity.w/2.0;
        double centerY = y + entity.h/2.0;
        if (centerX < -150 || centerX > 950 || centerY < -150 || centerY > 750) markedForDespawn = true;
    }

    void render(SDL_Renderer* renderer) {
        checkTexture(renderer);
        entity.x = x;
        entity.y = y;
        if (spriteTexture != nullptr) {
            SDL_RenderCopyEx(renderer, spriteTexture, &sourceRect, &entity, rotation, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
        } else if (flatColor != nullptr) {
            SDL_SetRenderDrawColor(renderer, flatColor->r, flatColor->g, flatColor->b, flatColor->a); // Can I not rotate non-sprite rectangles???
            SDL_RenderFillRect(renderer, &entity);
        }
    }

    void placeWithDelta(int deltaTime) {
        /*
        entity.x += velocityX * (deltaTime / 1000.0);
        if (hasGravity) {
            for (; deltaTime > 16; deltaTime -= 16) { // Simulate gravity as if it happened over multiple frames if deltaTime is greater than one frame (only works at 60fps... actually all 120fps does here is add detail, though server simulations never include the extra detail as it's 60pfs ran at 2x speed, not 120fps at 1x speed. So while allowing this to support higher detail at 120fps, it technically would desync from the server instead of showing the same result at half speed)
                entity.y += velocityY * 0.016;
                //velocityY += game_data.GRAVITY * 0.016;
                velocityY += -420 * 0.016;
            }
            entity.y += velocityY * (deltaTime / 1000.0);
            //velocityY += game_data.GRAVITY * (deltaTime / 1000.0);
            velocityY += -420 * (deltaTime / 1000.0);
        } else {
            entity.y += velocityY * (deltaTime / 1000.0);
        }
        */
        x += velocityX * (deltaTime / 1000.0);
        if (hasGravity) {
            for (; deltaTime > 16; deltaTime -= 16) { // Simulate gravity as if it happened over multiple frames if deltaTime is greater than one frame (only works at 60fps... actually all 120fps does here is add detail, though server simulations never include the extra detail as it's 60pfs ran at 2x speed, not 120fps at 1x speed. So while allowing this to support higher detail at 120fps, it technically would desync from the server instead of showing the same result at half speed)
                y += velocityY * 0.016;
                //velocityY += game_data.GRAVITY * 0.016;
                velocityY += -420 * 0.016;
            }
            y += velocityY * (deltaTime / 1000.0);
            //velocityY += game_data.GRAVITY * (deltaTime / 1000.0);
            velocityY += -420 * (deltaTime / 1000.0);
        }
        else {
            y += velocityY * (deltaTime / 1000.0);
        }
    }
};

struct PlayerAttackData : public ProjectileData {
//class PlayerAttackData : public ProjectileData {
    bool boundToPlayer = false;
    int boundPlayerID = 0;
    int bindDirectionX = 1;
    int bindDirectionY = 0;

    PlayerAttackData(int attackID) {
        switch (attackID) {
        case 1:
        case 2:
            spriteRef = "Assets/Textures/angel sword.png";
            sourceRect = { 0, 0, 50, 16 };
            break;
        case 3:
            spriteRef = "Assets/Textures/angel sword.png";
            sourceRect = { 0, 0, 50, 2400 };
            break;
        }
    }
};

static struct GameData {
private:
    bool ready = false; // Flags if the game is ready to start running
    bool running = true; // Flags if the game is actively running in the main state (outside of ends creen UI and game lobby) // separates the game loop running from the game itself wanting to run in main (main->is_running)
public:
    static const int GRAVITY = -420;
    bool isReady() { return ready; }
    void setReady(bool isReady) { ready = isReady; }
    bool isRunning() { return running; }
    void setRunning(bool isRunning) { running = isRunning; }
    //https://www.geeksforgeeks.org/cpp/how-to-use-hashmap-in-cpp - hashMaps in C++
    // NOTE - Since the max number of players can be capped, playerMap can work just as well when converted to a PlayerData* array[MAX_PLAYERS], skipping the need for hashing algorythms and saving on a ton of memory use for the exact same functionality. playerMap was initially an unordered_map<> so I wouldn't have to worry about fixed size limits. npcMap however should stay as a list type because npc can come and go (at least that would be true if the game was 100% finished, as of writing, there are no minion spawns mechanics in the game, so if the final version only has a single boss per game, npcMap could be replaces with an NPCData* boss variable to save on memory and processing time) // But for now I'm leaving all the features open-ended so working with them is as simple as possible. Optimising flexible code like this is a lot easier so it can be done towards the end of the project if I have to.
    std::unordered_map<int, PlayerData*> playerMap; // List of players // The same reference is used both in gameplay and UI // Player class (and related values) and ready status are set in UI, then remain fixed when switching to gameplay
    std::unordered_map<int, NPCData*> npcMap; // List of active enemies and bosses in the game // IDs are used for quick references to specific NPCs
    //std::vector<ProjectileData*> projectiles; // List of all active projectiles // Technically doesn't need IDs but also does when it comes to despawning projectiles after a collision if collisions aren't handled client-side. // for now, no IDs are used just to make this work
    //std::list<ProjectileData*> projectiles;
    ProjectileData* projectiles[500]; // List and vector require an allocator to work with classes
    int activeProjectileCount = 0; // Number of active projectiles, used to calculate how many slots are available in the projectiles array
    
} game_data;

class MyGame {

    private:
        //static const int MAX_PLAYERS = 4;
        //SDL_Rect player1 = { 200, 0, 20, 60 };
        //SDL_Rect player2 = { 600, 0, 20, 60 }; // New - Player 2
        //SDL_Rect ball = { 0, 0, 20, 20 }; // New - Ball
        //PlayerData* myPlayer = nullptr; // Stores a PlayerData reference as this client's player for easy access and client-specific functions // i.e simulating the client player locally, highlighting client's player in UI, etc
        bool quit = false;
    public:
        static const int MAX_PLAYERS = 4;
        std::vector<std::string> messages;
        MyPlayerData* myPlayer = nullptr;

        void on_receive(std::string message, std::vector<std::string>& args);
        void send(std::string message);
        void input(SDL_Event& event);
        //void clickInput(SDL_Event& event); // New - for mouse inputs
        //void update(); // Update all clientside objects at 60fps
        void update(double tpf); // Update all clientside objects based on local frame time
        void updateSimulated(double tpf); // new - Simulate updates for objects that are actively updated by the server // Edit - Simulations are done relative to the last time update data was sent / the last simulation hapenned.
        void updateSimulated(double tpf, char* updateMessage); // Simulates objects updated by the server // Reads and simulates data relative to the given update message
        //void updateSimulated(double tpf, std::string updateMessage); // Simulates objects updated by the server // Reads and simulates data relative to the given update message
        void render(SDL_Renderer* renderer);
        void forceQuit() { quit = true; }
        bool shouldForceQuit() { return quit; }
        //GameData getGameData();
        GameData* getGameData(); // Return a pointer instead
        void spawnAttack(std::vector<std::string>& args);
        //long getCurrentTimeMS() { return (long) (_Xtime_get_ticks() / 10000); }
        long long getCurrentTimeMS() { return (_Xtime_get_ticks() / 10000); }
        //MyGame() {
            //std::cout << "class: " << (PlayerClasses(3) == PlayerClasses::RANGER) << std::endl; // Since we can convert int to enum, we can send player class type as int/short // Since data is sent as a string, different int types don't really matter, only whole vs decimals
            //std::cout << "class: " << (PlayerClasses(3) == PlayerClasses::MAGE) << std::endl;
        //}
};

#endif