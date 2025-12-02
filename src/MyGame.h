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
        if (spriteTexture != nullptr) SDL_DestroyTexture(spriteTexture); // SDL_DestroyTexture() can be used to unload textures from entities. It's good enough for now, but I could also handle textures by storing each texture somewhere in memory and have entities point to them. That would remove the need to constantly load and unload images for each new entity.
        //std::cout << "changeSprite\n";
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
                velocityY += 420 * 0.016;
            }
            y += velocityY * tpf;
            velocityY += 420 * tpf;
        //}
        //else {
        //    y += velocityY * tpf;
        //}
    }

    // Manipulate the entity rectangle and return one with the proper offsets
    SDL_Rect getRect() {
        //SDL_Rect sprite = { entity.x + simXOffset - 12, entity.y + simYOffset - 12, entity.w, entity.h };
        SDL_Rect sprite = { x - 12, fmin(540, y) - 12, entity.w, entity.h }; // ~541 is ground level so if anything, we can keep the player from falling through the floor by not letting their sprite render below floor level
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

    ~PlayerData() {
        if (spriteTexture != nullptr) SDL_DestroyTexture(spriteTexture); // Temporary, destroy the projectile's texture so it doesn't say behind in memory
        //delete& entity;
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
    //SDL_Color* flatColor = nullptr;
    double x = 0; // Storing position separately to avoid truncation issues
    double y = 0;
    double velocityX = 0;
    double velocityY = 0;
    bool hasGravity = false;
    double rotation = 0; 
    bool justAdded = true; // Notes the entity as newly created so it gets ignored on the first update call // This is there to work with the same offset created in the server, where entities created within a tick don't move until the next one is called // Could technically be false by default, but I think keeping it true will always have the intended effect. // Yeah that fixes it a little bit
    bool markedForDespawn = false; // Sets the projectile to be despawned during an update call

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

    // I don't have a good place to put this but since it relates to memory I'll write it here...
    // As of the current setup where projectiles can spawn & despawn where the projectiles and textures are deleted when they aren't needed anymore, I ran a test to check memory usage over time to catch any possible leaks. // I didn't watch the memory value the entire time, just left the game running in the background for a while
    // First ~30 minutes had no signs of any leaks, memory capped at 172mb, with 173mb being possible if enoguh projectiles spawned at once with good rng // Memory stayed around 171mb on average, often going down to 170mb, but rarely going to 172mb // The Process Memory debug display showed values up to 189mb (the upper cap being important as it changes relative to the highest recorded value)
    // Around ~50 minutes the first sign of 173mb was spotted in the upper bound (moving from 189 to 190) // Not a guaranteed sign of a memory leak as it could be achieved with enough luck // Values still averaged at 171mb but 172mb became slightly more common // Either being a good streak of luck or a tiny memory leak
    // Around ~80 minutes the cap increased to 191mb, implying a 174mb value was hit at some point // (this needs to be verified) Either I remembered the upper bound wrong or an impossibly low chance was hit where enough projectiles spawned to briefly reach 174mb. // Pausing the server to allow all projectiles to naturally despawn showed the memory level at 170mb, this isn't good as it's supposed to be 169mb by default, implying a possible memory leak. // 172mb became a lot more common on average, being as common as 171mb if not more. Around this point, 173mb was also spotted on the graph at a few points past this point. That is a fairly strong sign of a memory leak. // A source image for the flatline 170mb exists
    // Around ~140 minutes something weird happened, 173mb was spottet multiple times, but also a lot of 170mb. Pausing the server again to see where memory rests at left it at 169mb, the normal value. // Either the thing causing the memory leak reset, or something else happened that I don't know about. // A source image for the 169mb exists
    // Past 140 and around 150-160-170 minutes where this is being written, the values fluctuate between 170mb and 172mb, 171mb became a brief transition between the two. // Pausing the server for a third time dropped the value down to 170mb again // source image is included // After unpausing, the values now fluctuate almost evenly between 170-172mb, though sometimes 172mb appears a lot more often. // While 170mb still appears with active projectiles, pausing the server at 170mb doesn't bring the number down.
    // Concluding at ~180 minutes, the value remains flatlined at 170mb with no projectiles on screen. I will leave it running in case anything changes while I write this out. // There is a small chance of a memory leak unless it's a part of the program that changes in size over time, more on that later.
    //
    // The conclusion from this is that there most likely isn't a significant memory leak in the projectile system. At worst it would have to be a really tiny one.
    // My main reason for this is that this behaviour mimicks what I found from another system in the client, the update message buffer.
    // Aside from possible leaks (as of writing) in the player and npc systems, the projectile and update message buffer system are the only ones with a chance of a memory leak while not actively interacting with the game. The projectile system was just added, and similar behaviour was seen without it.
    // The addition of the update message buffer was the first source of a memory leak that I found, mostly because of concurency issues. It used to leak memory constantly but later I added a system for that memory to be cleared whenever it was possible without breaking anything. In the version as of writing this, with the update message buffer active, it took ~30-40 minutes to increase the memory from 169mb to 170mb, after another 20-30 minutes, it went back down to 169mb. As the memory went back down eventually, I decided to leave it as is since it's not going up constantly, though why this up and down happens I have no idea. If this weird fluctuation is the same reason why the test happens like this here, then there most likely is no memory leak in the projectile system, and all objects are removed properly.
    // Final update, 200 minutes, 170mb flatline. If there is a memory leak, it only went up by ~1mb in 3 hours and 20 minutes, with the flip to 170mb happening early on. If that's all there is then there's nothing to worry about. With the chance of the value dropping back down, it's hard to say if anything is leaking at all.
    //
    // Restarting the game to see how it operates at the start:
    //  - At a fixed 168mb, the debug profiler shows the cap as 185mb, showing that it tries to give an extra 17mb worth of room when displaying memory values, which also proves that there might have been a 174mb value hit at some point during testing
    //  - The game runs at a default 169mb in the game state. // A few projectiles can exist before moving from 169mb to 170mb // This contrasts the late stages of the test where having a few projectiles hit minimum 170mb, with 169mb being unreachable with no projectiles present.
    //  - The fluctuations in memory are actually very similar at the beginning of the game as at the end of the test, though there might be a slight shift in favour of higher values later on which aligns with the update message buffer leak.
    //
    // In the end, this test didn't show anything worrying.
    //
    // Looking back on the update message buffer, it's all char[] arrays so there is no memory to leak from there as all the memory used is already defined. There's still (maybe) a possible concurrency issue that didn't happen once in 200 minutes, but there should be no leaks from there. That means that the leak that goes back and forth comes from somewhere else.
    // ^ I can't find anything obvious that could cause a leak in the game loop while running with no input from the player

    ~ProjectileData() {
        //std::cout << "hello\n";
        if (spriteTexture != nullptr) SDL_DestroyTexture(spriteTexture); // Temporary, destroy the projectile's texture so it doesn't say behind in memory
        //delete& entity;
        //delete& sourceRect;
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
                velocityY += 420 * 0.016;
            }
            y += velocityY * tpf;
            //velocityY += game_data.GRAVITY * tpf;
            velocityY += 420 * tpf;
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
        }// else if (flatColor != nullptr) {
        //    SDL_SetRenderDrawColor(renderer, flatColor->r, flatColor->g, flatColor->b, flatColor->a); // Can I not rotate non-sprite rectangles???
        //    SDL_RenderFillRect(renderer, &entity);
        //}
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
                velocityY += 420 * 0.016;
            }
            y += velocityY * (deltaTime / 1000.0);
            //velocityY += game_data.GRAVITY * (deltaTime / 1000.0);
            velocityY += 420 * (deltaTime / 1000.0);
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
    static const int GRAVITY = 420;
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