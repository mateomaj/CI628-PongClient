#pragma once
//#include "MyGame.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

// Credit where credit is due:
// The idea to define GameData in a separate file was also given by copilot while researching into the global variable scope issue
//https://learn.microsoft.com/en-us/cpp/error-messages/tool-errors/linker-tools-error-lnk2005 - This error came up while trying to figure out what was going on, the error page along with copilot's response pushed me in the right direction on tackling this issue
//https://stackoverflow.com/questions/3746484/why-am-i-getting-this-redefinition-of-class-error - Helped fighting the 'redefinition of class' issue. I knew about '#pragma once' as it showed up before but I didn't realise what it meant until seeing this forum post
//https://stackoverflow.com/questions/10422034/when-to-use-extern-in-c - Helped with understanding what the 'extern' keyword meant
//
// Like mentioned in my git commit for this change, this fix is technically pointless as game_data could be a variable within the MyGame class instead of keeping it within global scope. Since using global variables isn't recommended, I will probably revert this back into MyGame.h at some point.

struct PlayerGameData {
    bool slotActive = false;
    int id = 0;
    int health = 0;
    int maxHealth = 0;
    SDL_Texture* sprite = nullptr;
};

struct PlayerData;
struct MyPlayerData;
struct NPCData;
struct ProjectileData;
struct PlayerAttackData;

//static struct GameData {
struct GameData {
private:
    bool ready = false; // Flags if the game is ready to start running
    bool running = true; // Flags if the game is actively running in the main state (outside of ends creen UI and game lobby) // separates the game loop running from the game itself wanting to run in main (main->is_running)
public:
    //static const int GRAVITY = 420; 
    const double GRAVITY = 8.4; // Turns out JBox2D multiplies the physics world gravity by 0.02 to find its own gravity
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
    std::unordered_map<int, SDL_Texture*> textures; // List of textures used within the game. Used to provide references to any existing texture without having to load it again.
    TTF_Font* font = nullptr;
    SDL_Color defaultFontColor = { 200, 200, 200 };
    int roundDuration;
    bool partyWon = false;
    PlayerGameData endData[4];
//} game_data;
};
//static GameData game_data;

extern GameData game_data;




// New - Moved ProjectileData up above PlayerData because C++ treats forward declaration as a joke. Because problems only happen when you press the run button.
struct ProjectileData {
    //class ProjectileData {
    //public:
        // Found a cool problem where the projectile moved faster/slower than intended depending on which direction they were moving in. Turns out it's caused by truncation as SDL_Rect stores its values as integers // rounding down makes things move faster to the left and slower to the right as truncation removes decimals and moves the number towards 0. // Seeing the speed difference was enough to figure it out but another cool tell for it was that the projectiles changed speed when crossing x=0 on the left side of the screen.
    SDL_Rect entity = { 0, 0, 40, 40 };
    SDL_Rect sourceRect = { 0, 0, 0, 0 };
    SDL_Texture* spriteTexture = nullptr;
    //char* spriteRef = "";
    //SDL_Color* flatColor = nullptr;
    double x = 0; // Storing position separately to avoid truncation issues
    double y = 0;
    double velocityX = 0;
    double velocityY = 0;
    int pivotX = 0;
    int pivotY = 0;
    double* bindX = 0;
    double* bindY = 0;
    bool hasGravity = false;
    double rotation = 0;
    bool rotateToVelocity = false; // used for ranger's arrows and traps
    bool spin = false; // used to give mage fireballs a little animation
    bool justAdded = true; // Notes the entity as newly created so it gets ignored on the first update call // This is there to work with the same offset created in the server, where entities created within a tick don't move until the next one is called // Could technically be false by default, but I think keeping it true will always have the intended effect. // Yeah that fixes it a little bit
    bool markedForDespawn = false; // Sets the projectile to be despawned during an update call
    double lifetime = 30; // How long before the projectile should automatically delete itself // Default is 30 seconds to give projectiles enough time to go off screen and to match the ranger alt-fire trap duration
    double weight = 10;

    // Synchronising projectiles - switching projectiles to being referenced by ID from the server means that the client can't create and destroy projectiles unless told to by the server
    // Assuming that all data is given in the correct order and is never skipped, projectiles could be added into an array without any problems caused by latency if the server controlls their creation and destruction.

    bool assumeDespawn = false; // Puts the projectile into a despawned state without marking it for deletion. // markedForDespawn might become redundant unless projectiles are deleted on update()

    void setPosition(int x, int y) {
        entity.x = x + pivotX;
        entity.y = y + pivotY;
        this->x = x + pivotX;
        this->y = y + pivotY;
    }

    void setPosition(double x, double y) {
        entity.x = x + pivotX;
        entity.y = y + pivotY;
        this->x = x + pivotX;
        this->y = y + pivotY;
    }

    void setVelocity(double velX, double velY) {
        velocityX = velX;
        velocityY = velY;
        if (rotateToVelocity) {
            rotation = SDL_atan2(velocityX, velocityY) * 180 / 3.14 - 90;
        }
    }

    //void checkTexture(SDL_Renderer* renderer) {
        //if (spriteTexture == nullptr && spriteRef != nullptr) {
            //SDL_Surface* tempSurface = IMG_Load(spriteRef);
            //spriteTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
            //SDL_FreeSurface(tempSurface);
        //}
    //}

    ProjectileData() {}

    ProjectileData(int attackID) {
        switch (attackID) {
        case 1:
        case 2:
            //spriteRef = "Assets/Textures/angel sword.png";
            spriteTexture = game_data.textures[5];
            sourceRect = { 0, 0, 50, 16 };
            entity = { 0, 0, 50, 16 };
            break;
        case 3:
            //spriteRef = "Assets/Textures/angel sword.png";
            spriteTexture = game_data.textures[5];
            sourceRect = { 0, 0, 50, 16 };
            entity = { 0, 0, 50, 2400 };
            break;
        case 4:
            spriteTexture = game_data.textures[6]; // Sword
            sourceRect = { 0, 0, 50, 30 };
            entity = { 0, 0, 50, 30 };
            lifetime = 0.5;
            break;
        case 5:
            spriteTexture = game_data.textures[7]; // Charged sword
            sourceRect = { 0, 0, 70, 40 };
            entity = { 0, 0, 70, 40 };
            lifetime = 0.5;
            break;
        case 6:
            spriteTexture = game_data.textures[8]; // Arrow
            sourceRect = { 0, 0, 20, 10 };
            entity = { 0, 0, 20, 10 };
            hasGravity = true;
            rotateToVelocity = true;
            break;
        case 7:
            spriteTexture = game_data.textures[9]; // Spike trap
            sourceRect = { 0, 0, 15, 15 };
            entity = { 0, 0, 15, 15 };
            hasGravity = true;
            rotateToVelocity = true;
            weight = 20;
            break;
        case 8:
            spriteTexture = game_data.textures[10]; // Fireball
            sourceRect = { 0, 0, 30, 30 };
            entity = { 0, 0, 30, 30 };
            spin = true;
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
        //if (spriteTexture != nullptr) SDL_DestroyTexture(spriteTexture); // Temporary, destroy the projectile's texture so it doesn't say behind in memory
        //delete& entity;
        //delete& sourceRect;
    }

    void update(double tpf) { // Update uses the same logic as placeWithDelta() but in double format for deltaTime, it also includes extra logic to help with unload checks

        lifetime -= tpf;
        if (lifetime <= 0) {
            assumeDespawn = true;//markedForDespawn = true;
            return; // If the projectile runs out of time then just skip the rest of the function, it doesn't need to run.
        }

        //entity.x += velocityX * tpf;
        //if (hasGravity) {
        //    for (; tpf > 0.016; tpf -= 0.016) {
        //        entity.y += velocityY * 0.016;
        //        //velocityY += game_data.GRAVITY * 0.016;
        //        velocityY += -420 * 0.016;
        //    }
        //    entity.y += velocityY * tpf;
        //    //velocityY += game_data.GRAVITY * tpf;
        //    velocityY += -420 * tpf;
        //}
        //else {
        //    entity.y += velocityY * tpf;
        //}

        x += velocityX * tpf;
        if (hasGravity) {
            //for (; tpf > 0.016; tpf -= 0.016) {
            //    y += velocityY * 0.016;
            //    velocityY += game_data.GRAVITY * 0.016;
            //    //velocityY += 420 * 0.016;
            //}
            y += velocityY * tpf;
            velocityY += game_data.GRAVITY * tpf * weight;
            //velocityY += 420 * tpf;
        }
        else {
            y += velocityY * tpf;
        }

        if (spin) {
            rotation += tpf * 90;
        }
        //else if (rotateToVelocity) {
            //rotation = SDL_atan2(velocityX, velocityY) * 180 / 3.14 - 90;
        //    rotation = SDL_atan2(velocityY, velocityX) * 180 / 3.14;
        //}

        double centerX = x + entity.w / 2.0;
        double centerY = y + entity.h / 2.0;
        if (centerX < -150 || centerX > 950 || centerY < -150 || centerY > 750) {
            //markedForDespawn = true;
            assumeDespawn = true;
        }
    }

    void render(SDL_Renderer* renderer) {
        //checkTexture(renderer);
        if (bindX != nullptr) {
            entity.x = x + *bindX;
        } else {
            entity.x = x;
        }
        if (bindY != nullptr) {
            entity.y = y + *bindY;
        }  else {
            entity.y = y;
        }
        if (rotateToVelocity) {
            rotation = SDL_atan2(velocityY, velocityX) * 180 / 3.14;
        }
        if (spriteTexture != nullptr) {
            SDL_RenderCopyEx(renderer, spriteTexture, &sourceRect, &entity, rotation, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
        }// else if (flatColor != nullptr) {
        //    SDL_SetRenderDrawColor(renderer, flatColor->r, flatColor->g, flatColor->b, flatColor->a); // Can I not rotate non-sprite rectangles???
        //    SDL_RenderFillRect(renderer, &entity);
        //}
    }

    void placeWithDelta(int deltaTime) {

        //entity.x += velocityX * (deltaTime / 1000.0);
        //if (hasGravity) {
        //    for (; deltaTime > 16; deltaTime -= 16) { // Simulate gravity as if it happened over multiple frames if deltaTime is greater than one frame (only works at 60fps... actually all 120fps does here is add detail, though server simulations never include the extra detail as it's 60pfs ran at 2x speed, not 120fps at 1x speed. So while allowing this to support higher detail at 120fps, it technically would desync from the server instead of showing the same result at half speed)
        //        entity.y += velocityY * 0.016;
        //        //velocityY += game_data.GRAVITY * 0.016;
        //        velocityY += -420 * 0.016;
        //    }
        //    entity.y += velocityY * (deltaTime / 1000.0);
        //    //velocityY += game_data.GRAVITY * (deltaTime / 1000.0);
        //    velocityY += -420 * (deltaTime / 1000.0);
        //} else {
        //    entity.y += velocityY * (deltaTime / 1000.0);
        //}

        x += velocityX * (deltaTime / 1000.0);
        if (hasGravity) {
            for (; deltaTime > 16; deltaTime -= 16) { // Simulate gravity as if it happened over multiple frames if deltaTime is greater than one frame (only works at 60fps... actually all 120fps does here is add detail, though server simulations never include the extra detail as it's 60pfs ran at 2x speed, not 120fps at 1x speed. So while allowing this to support higher detail at 120fps, it technically would desync from the server instead of showing the same result at half speed)
                y += velocityY * 0.016;
                velocityY += game_data.GRAVITY * 0.016 * weight;
                //velocityY += 420 * 0.016;
            }
            y += velocityY * (deltaTime / 1000.0);
            velocityY += game_data.GRAVITY * (deltaTime / 1000.0) * weight;
            //velocityY += 420 * (deltaTime / 1000.0);
        }
        else {
            y += velocityY * (deltaTime / 1000.0);
        }
    }
};








//https://www.geeksforgeeks.org/cpp/enum-classes-in-c-and-their-advantage-over-enum-datatype
enum class PlayerClasses {
    NONE, KNIGHT, RANGER, MAGE
};

//enum class NPCClasses {
//    NONE, FORUMAN
//};

struct RechargableValue {
    double value = 0;
    double maxValue = 0;
    int chargeSpeed = 1;
    double chargeDelay = 0;
    double chargeDelayTimer = 0;
    void update(double tpf) {
        if (chargeDelayTimer > 0) {
            chargeDelayTimer -= tpf;
        } else {
            value = SDL_min(value + tpf * chargeSpeed, maxValue);
        }
    }
    void setValue(double amount) {
        value = SDL_max(SDL_min(amount, maxValue), 0);
    }
    void damage(double amount) {
        value = SDL_max(value - amount, 0);
    }
    void restore(double amount) {
        value = SDL_min(value + amount, maxValue);
    }
};

struct PlayerData {
    PlayerClasses playerClass = PlayerClasses::NONE; // Player class reference for sprites and other values
    //int playerX = 0;
    //int playerY = 0;
    SDL_Rect entity = { 0, 0, 40, 40 };
    SDL_Texture* spriteTexture = nullptr;
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
    int invincibilityTime = 0;
    //bool isMe = false; // Makes this player instance stand out as THIS client's player // aka "Is that player mine?"
    RechargableValue specialStat;
    ProjectileData* meleeAttack = nullptr; // As much as I would like the melee attack to be a projectile, I can't add it to the projectile list as it's not a projectile on server side. The attack is rendered relative to the player position and doesn't need to be updated on its own outside of deleting it after running out of time. // This work around avoids adding useless logic on server side and also makes the melee attack render on the same layer as the players. I will just work with this.

    // Velocity could be included later for latency simulation / switching movement to being done clientside
    //double velocityX = 0;
    //double velocityY = 0;

    //PlayerData() {
        //GameData data = game_data;
        //std::cout << "FUCKING KILL ME\n";
    //}

    void setPlayerClass(PlayerClasses playerClass) {
        if (this->playerClass == playerClass) return; // Only run when the new player class is different so we don't unload the texture for no reason
        this->playerClass = playerClass;
        //if (spriteTexture != nullptr) SDL_DestroyTexture(spriteTexture); // SDL_DestroyTexture() can be used to unload textures from entities. It's good enough for now, but I could also handle textures by storing each texture somewhere in memory and have entities point to them. That would remove the need to constantly load and unload images for each new entity.
        //std::cout << "changeSprite\n";
        spriteTexture = nullptr;
        switch (playerClass) {
        case PlayerClasses::KNIGHT:
            //spriteTexture = game_data.textures[1];
            health = 250;
            maxHealth = 250;
            specialStat = { 0, 1.5, 0 }; // Might need to use a server update to manage this one // "KC" - "KNIGHT_CHARGE" // "KR"- "KNIGHT_RELEASE" // KC/R,<TimeRef>,<PlayerID>; // TODO
            break;
        case PlayerClasses::RANGER:
            //spriteTexture = game_data.textures[2];
            health = 175;
            maxHealth = 175;
            specialStat = { 100, 100, 20, 0.75 };
            break;
        case PlayerClasses::MAGE:
            //spriteTexture = game_data.textures[3];
            health = 150;
            maxHealth = 150;
            specialStat = { 100, 100, 15, 0.75 };
            break;
        default:
            //spriteTexture = game_data.textures[0];
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
        if (health > 0) { // Don't update special stats while dead
            specialStat.update(tpf);
        }
        if (invincibilityTime > 0) {
            invincibilityTime = SDL_max(invincibilityTime - tpf * 1000, 0); // Limit it to 0 at lowest
        }
        if (meleeAttack != nullptr) {
            if (health > 0) {
                meleeAttack->update(tpf);
            } else {
                meleeAttack->assumeDespawn = true; // Despawn the melee attack if the knight dies
            }
            if (meleeAttack->assumeDespawn) { // Outside of "KNIGHT_CHARGE", the server doesn't send hints about melee attacks despawning. This means the attack can be deleted when it times out on the client, "KC" can force it to despawn if it hasn't already but it should have already timed out by the time the message gets there.
                delete meleeAttack;
                meleeAttack = nullptr;
            }
        }
        //x += velocityX * tpf;
        //if (hasGravity) {
        //for (; tpf > 0.016; tpf -= 0.016) {
        //    y += velocityY * 0.016;
            //velocityY += 420 * 0.016;
        //    velocityY += game_data.GRAVITY * 0.016;
        //}
        //y += velocityY * tpf;
        //velocityY += 420 * tpf;
        //velocityY += game_data.GRAVITY * tpf; // Right........ Why the fuck is update() the only function that gets the proper reference to game_data??? // Both update() and render() are called from MyGame.cpp, so why does one get special treatment while the other doesn't
        //std::cout << game_data.GRAVITY << std::endl;
    //}
    //else {
    //    y += velocityY * tpf;
    //}
    }

    void simUpdate(double tpf) { // Physics updates are now in a separate update function to separate update time and simulated time
        x += velocityX * tpf;
        for (; tpf > 0.016; tpf -= 0.016) {
            y += velocityY * 0.016;
            velocityY += game_data.GRAVITY * 0.016;
        }
        y += velocityY * tpf;
        velocityY += game_data.GRAVITY * tpf;
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
            //SDL_Surface* tempSurface;
            switch (playerClass) {
            case PlayerClasses::KNIGHT:
                //tempSurface = IMG_Load("Assets/Textures/Knight.png");
                spriteTexture = game_data.textures[1];
                //std::cout << game_data.textures[1] << std::endl; // jmdcnioavsjapfdnihjfdjONICVHFALJMODACANFISHDMIcnafihd WHAT THE FUCK // Two structs that are defined in the same way, within the same header file, using the same variable reference -> one can access the variable and the other can't... WHYYYYYYYYYYYYYYYY
                // ProjectileData has no problems accessing the texture data when using game_data, but PlayerData can't for some reason.
                //std::cout << &game_data << std::endl;
                break;
            case PlayerClasses::RANGER:
                //tempSurface = IMG_Load("Assets/Textures/Ranger.png");
                spriteTexture = game_data.textures[2];
                break;
            case PlayerClasses::MAGE:
                //tempSurface = IMG_Load("Assets/Textures/Mage.png");
                spriteTexture = game_data.textures[3];
                break;
            default:
                //tempSurface = IMG_Load("Assets/Textures/BlankPlayer.png");
                spriteTexture = game_data.textures[0];
                break;
            }
            //spriteTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
            //SDL_FreeSurface(tempSurface);
        }
    }

    void render(SDL_Renderer* renderer) {
        //if (spriteTexture == nullptr) {
        //    SDL_Surface* tempSurface;
        //    switch (playerClass) {
        //        case PlayerClasses::KNIGHT:
        //            tempSurface = IMG_Load("Assets/Textures/Knight.png");
        //            break;
        //        case PlayerClasses::RANGER:
        //            tempSurface = IMG_Load("Assets/Textures/Ranger.png");
        //            break;
        //        case PlayerClasses::MAGE:
        //            tempSurface = IMG_Load("Assets/Textures/Mage.png");
        //            break;
        //        default:
        //            tempSurface = IMG_Load("Assets/Textures/BlankPlayer.png");
        //            break;
        //    }
        //    spriteTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
        //    SDL_FreeSurface(tempSurface);
        //}
        checkTexture(renderer);
        if (spriteTexture != nullptr) {
            if (invincibilityTime == 0 || invincibilityTime % 333 < invincibilityTime % 666) {
                SDL_Rect srcRect = { 0, 0, 20, 20 };
                SDL_Rect dstRect = getRect();
                //SDL_RenderCopyEx(renderer, spriteTexture, &srcRect, &getRect(), 0, nullptr, facingRight ? SDL_RendererFlip::SDL_FLIP_NONE : SDL_RendererFlip::SDL_FLIP_HORIZONTAL);
                //SDL_RenderCopyEx(renderer, spriteTexture, &srcRect, &getRect(), health > 0 ? 0 : 90, nullptr, facingRight ? SDL_RendererFlip::SDL_FLIP_NONE : SDL_RendererFlip::SDL_FLIP_HORIZONTAL); // Rotate 90 degrees if the player is dead
                if (health > 0) {
                    SDL_RenderCopyEx(renderer, spriteTexture, &srcRect, &dstRect, 0, nullptr, facingRight ? SDL_RendererFlip::SDL_FLIP_NONE : SDL_RendererFlip::SDL_FLIP_HORIZONTAL);
                } else {
                    dstRect.y += 10;
                    SDL_RenderCopyEx(renderer, spriteTexture, &srcRect, &dstRect, 90, nullptr, facingRight ? SDL_RendererFlip::SDL_FLIP_NONE : SDL_RendererFlip::SDL_FLIP_HORIZONTAL);
                }
            }
        }
        if (meleeAttack != nullptr && !meleeAttack->assumeDespawn) {
            meleeAttack->render(renderer);
        }
    }

    //void renderUI(SDL_Renderer* renderer, int positionX) {
    void renderUI(SDL_Renderer* renderer, int positionX, int width) {
        //std::cout << "Hello - " << positionX << std::endl;
        //SDL_Rect tempRect = { positionX, 580, 100, 20 };
        SDL_Rect tempRect = { positionX, 580, width, 20 };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red
        SDL_RenderFillRect(renderer, &tempRect);
        SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255); // Green
        //tempRect.w = (health / maxHealth) * 100;
        tempRect.w = (health / (double) maxHealth) * width;
        SDL_RenderFillRect(renderer, &tempRect);
        tempRect.w = (specialStat.value / specialStat.maxValue) * width;
        tempRect.y = 595;
        switch (playerClass) {
            case PlayerClasses::KNIGHT:
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
                SDL_RenderFillRect(renderer, &tempRect);
                break;
            case PlayerClasses::RANGER:
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow
                SDL_RenderFillRect(renderer, &tempRect);
                break;
            case PlayerClasses::MAGE:
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // Cyan
                SDL_RenderFillRect(renderer, &tempRect);
                break;
            default:
                break;
        }
    }

    ~PlayerData() {
        //if (spriteTexture != nullptr) SDL_DestroyTexture(spriteTexture); // Temporary, destroy the projectile's texture so it doesn't say behind in memory
        //delete& entity;
        if (meleeAttack != nullptr) {
            delete meleeAttack; // Delete the melee attack if it happens to be active
        }
    }
};

// PlayerData with extra values that are only relevant to the local player
struct MyPlayerData : public PlayerData {
    bool isHoldingLeft = false;
    bool isHoldingRight = false;
};

enum class NPCClasses {
    NONE, FORUMAN
};

struct NPCData {
    NPCClasses npcClass = NPCClasses::NONE;
    SDL_Rect entity = { 0, 0, 20, 20 };
    SDL_Rect sourceRect = { 0, 0, 0, 0 };
    SDL_Texture* spriteTexture = nullptr;
    char* name = "";
    double x = 0;
    double y = 0;
    double velocityX = 0;
    double velocityY = 0;
    int health;
    int maxHealth;
    double attackCooldown = 0;

    void setPos(int x, int y) {
        entity.x = x;
        entity.y = y;
        this->x = x;
        this->y = y;
    }

    void setPos(double x, double y) {
        entity.x = x;
        entity.y = y;
        this->x = x;
        this->y = y;
    }

    void setVelocity(double velX, double velY) {
        velocityX = velX;
        velocityY = velY;
    }

    void setNPCClass(NPCClasses NPCClass) {
        if (this->npcClass == NPCClass) return;
        this->npcClass = NPCClass;
        spriteTexture = nullptr;
        switch (NPCClass) {
            case NPCClasses::FORUMAN:
                entity = { 0, 0, 196, 176 };
                sourceRect = { 0, 0, 49, 44 };
                spriteTexture = game_data.textures[4];
                health = 4000;
                maxHealth = 4000;
                name = "Foruman - The Mortal Angel";
                break;
            default:
                entity = { 0, 0, 20, 20 };
                sourceRect = { 0, 0, 20, 20 };
                spriteTexture = game_data.textures[0];
                health = 100;
                maxHealth = 100;
                break;
        }
    }

    // Update entity position data with args // Basically the same as the "PLAYER_DATA" command but made specifically for movement in case something "ENTITY_DATA" would contain something that doesn't fit the entity
    void updateMoveData(std::vector<std::string>& args) {
        switch (npcClass) {
            case NPCClasses::FORUMAN:
                if (args.size() >= 6) {
                    //long long timeRef = stoll(args.at(0));
                    setPos(stoi(args.at(2)), stoi(args.at(3)));
                    setVelocity(stoi(args.at(4)) * 25, stoi(args.at(5)) * 25);
                    //double tpf = ((timeRef - (_Xtime_get_ticks() / 10000)) / 1000.0);
                    double tpf = ((stoll(args.at(0)) - (_Xtime_get_ticks() / 10000)) / 1000.0);
                    if (tpf > 0.016) { // Ignore the first frame because the position data already has the first frame accounted for
                        tpf -= 0.016;
                        x += velocityX * tpf;
                        y += velocityY * tpf;
                    }
                }
                break;
            default:
                // It's hard to predict what the args might actually want without an npc class reference so I will leave this blank
                break;
        }

    }

    void update(double tpf) {
        switch (npcClass) {
            case NPCClasses::FORUMAN:
                // Foruman only moves while his attacks are on cooldown, making it look like he stops to summon an attack
                // Since attacks can only be summoned by the server to avoid RNG desync, Foruman will stand still until an attack is summoned
                //std::cout << attackCooldown << std::endl;
                if (attackCooldown > 0) {
                    attackCooldown -= tpf;
                    if (attackCooldown >= 0.5) {
                        x += velocityX * tpf;
                        y += velocityY * tpf;
                    }
                }
                break;
            default:
                x += velocityX * tpf;
                y += velocityY * tpf;
                break;
        }
    }

    void placeWithDelta(int deltaTime) {
        x += velocityX * (deltaTime / 1000.0);
        y += velocityY * (deltaTime / 1000.0);
    }

    void render(SDL_Renderer* renderer) {
        entity.x = x;
        entity.y = y;
        if (spriteTexture != nullptr) {
            SDL_RenderCopy(renderer, spriteTexture, &sourceRect, &entity);
        }
    }

};

/*
struct ProjectileData {
    //class ProjectileData {
    //public:
        // Found a cool problem where the projectile moved faster/slower than intended depending on which direction they were moving in. Turns out it's caused by truncation as SDL_Rect stores its values as integers // rounding down makes things move faster to the left and slower to the right as truncation removes decimals and moves the number towards 0. // Seeing the speed difference was enough to figure it out but another cool tell for it was that the projectiles changed speed when crossing x=0 on the left side of the screen.
    SDL_Rect entity = { 0, 0, 40, 40 };
    SDL_Rect sourceRect = { 0, 0, 0, 0 };
    SDL_Texture* spriteTexture = nullptr;
    //char* spriteRef = "";
    //SDL_Color* flatColor = nullptr;
    double x = 0; // Storing position separately to avoid truncation issues
    double y = 0;
    double velocityX = 0;
    double velocityY = 0;
    int pivotX = 0;
    int pivotY = 0;
    int* bindX = 0;
    int* bindY = 0;
    bool hasGravity = false;
    double rotation = 0;
    bool rotateToVelocity = false; // used for ranger's arrows and traps
    bool spin = false; // used to give mage fireballs a little animation
    bool justAdded = true; // Notes the entity as newly created so it gets ignored on the first update call // This is there to work with the same offset created in the server, where entities created within a tick don't move until the next one is called // Could technically be false by default, but I think keeping it true will always have the intended effect. // Yeah that fixes it a little bit
    bool markedForDespawn = false; // Sets the projectile to be despawned during an update call
    double lifetime = 30; // How long before the projectile should automatically delete itself // Default is 30 seconds to give projectiles enough time to go off screen and to match the ranger alt-fire trap duration
    
    // Synchronising projectiles - switching projectiles to being referenced by ID from the server means that the client can't create and destroy projectiles unless told to by the server
    // Assuming that all data is given in the correct order and is never skipped, projectiles could be added into an array without any problems caused by latency if the server controlls their creation and destruction.

    bool assumeDespawn = false; // Puts the projectile into a despawned state without marking it for deletion. // markedForDespawn might become redundant unless projectiles are deleted on update()

    void setPosition(int x, int y) {
        entity.x = x + pivotX;
        entity.y = y + pivotY;
        this->x = x + pivotX;
        this->y = y + pivotY;
    }

    void setPosition(double x, double y) {
        entity.x = x + pivotX;
        entity.y = y + pivotY;
        this->x = x + pivotX;
        this->y = y + pivotY;
    }

    void setVelocity(double velX, double velY) {
        velocityX = velX;
        velocityY = velY;
        if (rotateToVelocity) {
            rotation = SDL_atan2(velocityX, velocityY) * 180 / 3.14 - 90;
        }
    }

    //void checkTexture(SDL_Renderer* renderer) {
        //if (spriteTexture == nullptr && spriteRef != nullptr) {
            //SDL_Surface* tempSurface = IMG_Load(spriteRef);
            //spriteTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
            //SDL_FreeSurface(tempSurface);
        //}
    //}

    ProjectileData() {}

    ProjectileData(int attackID) {
        switch (attackID) {
        case 1:
        case 2:
            //spriteRef = "Assets/Textures/angel sword.png";
            spriteTexture = game_data.textures[5];
            sourceRect = { 0, 0, 50, 16 };
            entity = { 0, 0, 50, 16 };
            break;
        case 3:
            //spriteRef = "Assets/Textures/angel sword.png";
            spriteTexture = game_data.textures[5];
            sourceRect = { 0, 0, 50, 16 };
            entity = { 0, 0, 50, 2400 };
            break;
        case 4:
            spriteTexture = game_data.textures[6]; // Sword
            sourceRect = { 0, 0, 50, 30 };
            entity = { 0, 0, 50, 30 };
            break;
        case 5:
            spriteTexture = game_data.textures[7]; // Charged sword
            sourceRect = { 0, 0, 50, 30 };
            entity = { 0, 0, 70, 40 };
            break;
        case 6:
            spriteTexture = game_data.textures[8]; // Arrow
            sourceRect = { 0, 0, 20, 10 };
            entity = { 0, 0, 20, 10 };
            hasGravity = true;
            rotateToVelocity = true;
            break;
        case 7:
            spriteTexture = game_data.textures[9]; // Spike trap
            sourceRect = { 0, 0, 15, 15 };
            entity = { 0, 0, 15, 15 };
            hasGravity = true;
            rotateToVelocity = true;
            break;
        case 8:
            spriteTexture = game_data.textures[10]; // Fireball
            sourceRect = { 0, 0, 30, 30 };
            entity = { 0, 0, 30, 30 };
            spin = true;
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
        //if (spriteTexture != nullptr) SDL_DestroyTexture(spriteTexture); // Temporary, destroy the projectile's texture so it doesn't say behind in memory
        //delete& entity;
        //delete& sourceRect;
    }

    void update(double tpf) { // Update uses the same logic as placeWithDelta() but in double format for deltaTime, it also includes extra logic to help with unload checks

        lifetime -= tpf;
        if (lifetime <= 0) {
            assumeDespawn = true;//markedForDespawn = true;
            return; // If the projectile runs out of time then just skip the rest of the function, it doesn't need to run.
        }

        //entity.x += velocityX * tpf;
        //if (hasGravity) {
        //    for (; tpf > 0.016; tpf -= 0.016) {
        //        entity.y += velocityY * 0.016;
        //        //velocityY += game_data.GRAVITY * 0.016;
        //        velocityY += -420 * 0.016;
        //    }
        //    entity.y += velocityY * tpf;
        //    //velocityY += game_data.GRAVITY * tpf;
        //    velocityY += -420 * tpf;
        //}
        //else {
        //    entity.y += velocityY * tpf;
        //}

        x += velocityX * tpf;
        if (hasGravity) {
            for (; tpf > 0.016; tpf -= 0.016) {
                y += velocityY * 0.016;
                velocityY += game_data.GRAVITY * 0.016;
                //velocityY += 420 * 0.016;
            }
            y += velocityY * tpf;
            velocityY += game_data.GRAVITY * tpf;
            //velocityY += 420 * tpf;
        }
        else {
            y += velocityY * tpf;
        }

        if (spin) {
            rotation += tpf * 90;
        } else if (rotateToVelocity) {
            rotation = SDL_atan2(velocityX, velocityY) * 180 / 3.14 - 90;
        }

        double centerX = x + entity.w / 2.0;
        double centerY = y + entity.h / 2.0;
        if (centerX < -150 || centerX > 950 || centerY < -150 || centerY > 750) {
            //markedForDespawn = true;
            assumeDespawn = true;
        }
    }

    void render(SDL_Renderer* renderer) {
        //checkTexture(renderer);
        entity.x = x + *bindX;
        entity.y = y + *bindY;
        if (spriteTexture != nullptr) {
            SDL_RenderCopyEx(renderer, spriteTexture, &sourceRect, &entity, rotation, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
        }// else if (flatColor != nullptr) {
        //    SDL_SetRenderDrawColor(renderer, flatColor->r, flatColor->g, flatColor->b, flatColor->a); // Can I not rotate non-sprite rectangles???
        //    SDL_RenderFillRect(renderer, &entity);
        //}
    }

    void placeWithDelta(int deltaTime) {

        //entity.x += velocityX * (deltaTime / 1000.0);
        //if (hasGravity) {
        //    for (; deltaTime > 16; deltaTime -= 16) { // Simulate gravity as if it happened over multiple frames if deltaTime is greater than one frame (only works at 60fps... actually all 120fps does here is add detail, though server simulations never include the extra detail as it's 60pfs ran at 2x speed, not 120fps at 1x speed. So while allowing this to support higher detail at 120fps, it technically would desync from the server instead of showing the same result at half speed)
        //        entity.y += velocityY * 0.016;
        //        //velocityY += game_data.GRAVITY * 0.016;
        //        velocityY += -420 * 0.016;
        //    }
        //    entity.y += velocityY * (deltaTime / 1000.0);
        //    //velocityY += game_data.GRAVITY * (deltaTime / 1000.0);
        //    velocityY += -420 * (deltaTime / 1000.0);
        //} else {
        //    entity.y += velocityY * (deltaTime / 1000.0);
        //}

        x += velocityX * (deltaTime / 1000.0);
        if (hasGravity) {
            for (; deltaTime > 16; deltaTime -= 16) { // Simulate gravity as if it happened over multiple frames if deltaTime is greater than one frame (only works at 60fps... actually all 120fps does here is add detail, though server simulations never include the extra detail as it's 60pfs ran at 2x speed, not 120fps at 1x speed. So while allowing this to support higher detail at 120fps, it technically would desync from the server instead of showing the same result at half speed)
                y += velocityY * 0.016;
                velocityY += game_data.GRAVITY * 0.016;
                //velocityY += 420 * 0.016;
            }
            y += velocityY * (deltaTime / 1000.0);
            velocityY += game_data.GRAVITY * (deltaTime / 1000.0);
            //velocityY += 420 * (deltaTime / 1000.0);
        }
        else {
            y += velocityY * (deltaTime / 1000.0);
        }
    }
};
*/
/*
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
            //spriteRef = "Assets/Textures/angel sword.png";
            spriteTexture = game_data.textures[5];
            sourceRect = { 0, 0, 50, 16 };
            break;
        case 3:
            //spriteRef = "Assets/Textures/angel sword.png";
            spriteTexture = game_data.textures[5];
            sourceRect = { 0, 0, 50, 2400 };
            break;
        }
    }
};
*/

//
// Actions:
// 1.	Render boss UI \,
// 2.	Replicate special stats \,
// 3.	Player attacks on client \,
// 4.	Lose state
// 5.	Background animation/image
// 6.	Interpolated animations
// 7.	Synchronise projectiles with the client \,
// 8.	Add arena
// 9.	Sounds, Sprites (stretch goal)
// 10.	Complete report
// 11.	Talk about technical stuff
// a.	Client disconnects
// b.	Latency fixes
// c.	Networking
// d.	Compression/encription
