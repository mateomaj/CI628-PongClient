#pragma once
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

// Data for how well a player did while fighting the boss
// Used for creating the game over screen
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

// Game data used all throughout the client program
struct GameData {
private:
    bool ready = false; // Flags if the game is ready to start running
    bool running = true; // Flags if the game is actively running in the main state (outside of ends creen UI and game lobby) // separates the game loop running from the game itself wanting to run in main (main->is_running)
public:
    const double GRAVITY = 8.4; // Turns out JBox2D multiplies the physics world gravity by 0.02 to find its own gravity
    bool isReady() { return ready; }
    void setReady(bool isReady) { ready = isReady; }
    bool isRunning() { return running; }
    void setRunning(bool isRunning) { running = isRunning; }
    //https://www.geeksforgeeks.org/cpp/how-to-use-hashmap-in-cpp - hashMaps in C++
    // NOTE - Since the max number of players can be capped, playerMap can work just as well when converted to a PlayerData* array[MAX_PLAYERS], skipping the need for hashing algorythms and saving on a ton of memory use for the exact same functionality. playerMap was initially an unordered_map<> so I wouldn't have to worry about fixed size limits. npcMap however should stay as a list type because npc can come and go (at least that would be true if the game was 100% finished, as of writing, there are no minion spawns mechanics in the game, so if the final version only has a single boss per game, npcMap could be replaces with an NPCData* boss variable to save on memory and processing time) // But for now I'm leaving all the features open-ended so working with them is as simple as possible. Optimising flexible code like this is a lot easier so it can be done towards the end of the project if I have to.
    std::unordered_map<int, PlayerData*> playerMap; // List of players // The same reference is used both in gameplay and UI // Player class (and related values) and ready status are set in UI, then remain fixed when switching to gameplay
    std::unordered_map<int, NPCData*> npcMap; // List of active enemies and bosses in the game // IDs are used for quick references to specific NPCs // Not used to its full extent in this version, but the support for handling multiple NPCs is there.
    ProjectileData* projectiles[500]; // List of all active projectiles // This list is synced with the server so it should not be changed on client-side unless the server says so.
    int activeProjectileCount = 0; // Number of active projectiles, used to calculate how many slots are available in the projectiles array

    std::unordered_map<int, SDL_Texture*> textures; // List of textures used within the game. Used to provide references to any existing texture without having to load it again.
    // ^ Note - Since this texture rework was added, the only sources of memory leaks that could happen are variables that weren't properly cleared.

    TTF_Font* font = nullptr;
    SDL_Color defaultFontColor = { 200, 200, 200 };
    int roundDuration;
    bool partyWon = false;
    PlayerGameData endData[4];
};

extern GameData game_data;


// New - Moved ProjectileData up above PlayerData because C++ treats forward declaration as a joke. Because problems only happen when you press the run button.

// Data used to define both player and enemy projectiles.
struct ProjectileData {
        // Found a cool problem where the projectile moved faster/slower than intended depending on which direction they were moving in. Turns out it's caused by truncation as SDL_Rect stores its values as integers // rounding down makes things move faster to the left and slower to the right as truncation removes decimals and moves the number towards 0. // Seeing the speed difference was enough to figure it out but another cool tell for it was that the projectiles changed speed when crossing x=0 on the left side of the screen.
    SDL_Rect entity = { 0, 0, 40, 40 };
    SDL_Rect sourceRect = { 0, 0, 0, 0 };
    SDL_Texture* spriteTexture = nullptr;
    double x = 0; // Storing position separately to avoid truncation issues
    double y = 0;
    double velocityX = 0;
    double velocityY = 0;
    int pivotX = 0; // Extra position offset for the projectile's sprite to account for differences in position handling between FXGL and SDL // Mainly used for the "wall" attack so it isn't placed out of bounds on spawn
    int pivotY = 0;
    double* bindX = 0; // Bind the projectile to an anchor position // Used for the knight's melee attack
    double* bindY = 0;
    bool hasGravity = false; // Is this projectile affected by gravity
    double rotation = 0;
    bool rotateToVelocity = false; // used for ranger's arrows and traps
    bool spin = false; // used to give mage fireballs a little animation
    bool justAdded = true; // Notes the entity as newly created so it gets ignored on the first update call // This is there to work with the same offset created in the server, where entities created within a tick don't move until the next one is called // Could technically be false by default, but I think keeping it true will always have the intended effect. // Yeah that fixes it a little bit
    bool markedForDespawn = false; // Sets the projectile to be despawned during an update call
    double lifetime = 30; // How long before the projectile should automatically delete itself // Default is 30 seconds to give projectiles enough time to go off screen and to match the ranger alt-fire trap duration
    double weight = 10; // Increases the effect of gravity

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

    ProjectileData() {}

    // Create a projectile with predefined stats and sprites for specific attacks
    ProjectileData(int attackID) {
        switch (attackID) {
        case 1:
        case 2:
            spriteTexture = game_data.textures[5]; // Angel sword
            sourceRect = { 0, 0, 50, 16 };
            entity = { 0, 0, 50, 16 };
            break;
        case 3:
            spriteTexture = game_data.textures[5]; // Angel sword stretched to look like a beam
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

    void update(double tpf) { // Update uses the same logic as placeWithDelta() but in double format for deltaTime, it also includes extra logic to help with unload checks

        // Update the projectile's lifetime and set it to despawn when out of time.
        lifetime -= tpf;
        if (lifetime <= 0) {
            assumeDespawn = true;
            return; // If the projectile runs out of time then just skip the rest of the function, it doesn't need to run.
        }

        // Move the projectile
        x += velocityX * tpf;
        if (hasGravity) {
            // This was meant to make the gravity simulation more accurate but it just made it desync from the server even more, so it was removed.
            //for (; tpf > 0.016; tpf -= 0.016) {
            //    y += velocityY * 0.016;
            //    velocityY += game_data.GRAVITY * 0.016;
            //}
            y += velocityY * tpf;
            velocityY += game_data.GRAVITY * tpf * weight;
        }
        else {
            y += velocityY * tpf;
        }

        // Spin the projectile
        if (spin) {
            rotation += tpf * 90;
        }

        // Automaticallly assume the projectile should be despawned when it goes out of bounds.
        double centerX = x + entity.w / 2.0;
        double centerY = y + entity.h / 2.0;
        if (centerX < -150 || centerX > 950 || centerY < -600 || centerY > 750) {
            assumeDespawn = true;
        }
    }

    // Render the projectile
    void render(SDL_Renderer* renderer) {
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
        }
    }

    // Place the projectile and move it forward by deltaTime milliseconds
    // Used to place projectiles while compensating for spawn delay
    // deltaTime is int type because most contexts for this function also use int/long
    void placeWithDelta(int deltaTime) {
        x += velocityX * (deltaTime / 1000.0);
        if (hasGravity) {
            for (; deltaTime > 16; deltaTime -= 16) { // Simulate gravity as if it happened over multiple frames if deltaTime is greater than one frame (only works at 60fps... actually all 120fps does here is add detail, though server simulations never include the extra detail as it's 60pfs ran at 2x speed, not 120fps at 1x speed. So while allowing this to support higher detail at 120fps, it technically would desync from the server instead of showing the same result at half speed)
                y += velocityY * 0.016;
                velocityY += game_data.GRAVITY * 0.016 * weight;
            }
            y += velocityY * (deltaTime / 1000.0);
            velocityY += game_data.GRAVITY * (deltaTime / 1000.0) * weight;
        } else {
            y += velocityY * (deltaTime / 1000.0);
        }
    }
};




//https://www.geeksforgeeks.org/cpp/enum-classes-in-c-and-their-advantage-over-enum-datatype - Learning about enums and enum classes
// ^ I was hoping to define some class specific functions in the enums to make the player class less cluttered, but it didn't work out

// Definitions of different player classes
enum class PlayerClasses {
    NONE, KNIGHT, RANGER, MAGE
};


// RechargableValue - A basic recreation of RechargeableDoubleComponent from FXGL with an option to let the value regenerate over time.
// Used for player class specific stats: mana, stamina, and charge
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


// Data defining a player client connected to the server.
// Contains data for both the player entity and their lobby UI
struct PlayerData {
    PlayerClasses playerClass = PlayerClasses::NONE; // Player class reference for sprites and other values
    SDL_Rect entity = { 0, 0, 40, 40 };
    SDL_Texture* spriteTexture = nullptr;
    double x = 0;
    double y = 0;
    double velocityX = 0;
    double velocityY = 0;
    bool facingRight = true;
    int health = 100;
    int maxHealth = 100;
    bool isReady = false; // Lobby UI only - Is this player ready to start the game
    int invincibilityTime = 0; // Timer showing player invincibility // Makes the player flash while it counts down.
    RechargableValue specialStat; // Special stat unique to each class
    ProjectileData* meleeAttack = nullptr; // As much as I would like the melee attack to be a projectile, I can't add it to the projectile list as it's not a projectile on server side. The attack is rendered relative to the player position and doesn't need to be updated on its own outside of deleting it after running out of time. // This work around avoids adding useless logic on server side and also makes the melee attack render on the same layer as the players. I will just work with this.
    // ^ To keep all the extra info in one spot - The melee attack is the only projectile not handled by the server through the projectile list, making it the only projectile type handled entirely by the client after being created. Unless the server attempts to spawn a melee attack while another still exists on the client, the melee attack is only despawned by timing out on the client.

    PlayerData() {
        spriteTexture = game_data.textures[0];
    }

    // Set player class and any relevant values that come with each one
    void setPlayerClass(PlayerClasses playerClass) {
        if (this->playerClass == playerClass) return; // Only run when the new player class is different so we don't unload the texture for no reason
        this->playerClass = playerClass;
        spriteTexture = nullptr;
        switch (playerClass) {
        case PlayerClasses::KNIGHT:
            spriteTexture = game_data.textures[1];
            health = 250;
            maxHealth = 250;
            specialStat = { 0, 1.5, 0 };
            break;
        case PlayerClasses::RANGER:
            spriteTexture = game_data.textures[2];
            health = 175;
            maxHealth = 175;
            specialStat = { 100, 100, 20, 0.75 };
            break;
        case PlayerClasses::MAGE:
            spriteTexture = game_data.textures[3];
            health = 150;
            maxHealth = 150;
            specialStat = { 100, 100, 15, 0.75 };
            break;
        default:
            spriteTexture = game_data.textures[0];
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

    // Update relevant timers and the melee attack if it exists
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
    // Mainly used to offset the player sprite to line up with their hitbox
    SDL_Rect getRect() {
        SDL_Rect sprite = { x - 12, fmin(540, y) - 12, entity.w, entity.h }; // ~541 is ground level so if anything, we can keep the player from falling through the floor by not letting their sprite render below floor level
        return sprite;
    }

    // Double checks if the player has a texture
    // With how textures were reworked, this function should be pointless. The only reason it still exists is because of an issue where the server sends player data back so fast that the textures don't have time to load into memory, causing problems with rendering UI sprites when opening the game. So this function has to stick around so players are assigned the right texture. // Now that I know about this issue while trying to remove this function, I added a few extra checks to catch this problem if it somehow gets past the lobby screen.
    void checkTexture() {
        if (spriteTexture == nullptr) {
            switch (playerClass) {
            case PlayerClasses::KNIGHT:
                spriteTexture = game_data.textures[1];
                break;
            case PlayerClasses::RANGER:
                spriteTexture = game_data.textures[2];
                break;
            case PlayerClasses::MAGE:
                spriteTexture = game_data.textures[3];
                break;
            default:
                spriteTexture = game_data.textures[0];
                break;
            }
        }
    }

    // Render the player sprite and their melee attack
    void render(SDL_Renderer* renderer) {
        if (spriteTexture != nullptr) {
            // While the invincibility timer ticks down, only render the player on specific values, creating a flashing effect.
            if (invincibilityTime == 0 || invincibilityTime % 333 < invincibilityTime % 666) {
                SDL_Rect srcRect = { 0, 0, 20, 20 };
                SDL_Rect dstRect = getRect();
                if (health > 0) {
                    SDL_RenderCopyEx(renderer, spriteTexture, &srcRect, &dstRect, 0, nullptr, facingRight ? SDL_RendererFlip::SDL_FLIP_NONE : SDL_RendererFlip::SDL_FLIP_HORIZONTAL);
                } else {
                    dstRect.y += 10;
                    SDL_RenderCopyEx(renderer, spriteTexture, &srcRect, &dstRect, 90, nullptr, facingRight ? SDL_RendererFlip::SDL_FLIP_NONE : SDL_RendererFlip::SDL_FLIP_HORIZONTAL); // Rotate 90 degrees if the player is dead
                }
            }
        } else {
            checkTexture(); // Might as well include this here if the player somehow switched classes and readied up before the game had the chance to load all the player sprites.
        }
        // Render the melee attack if it exists
        if (meleeAttack != nullptr && !meleeAttack->assumeDespawn) {
            meleeAttack->render(renderer);
        }
    }

    // Render player UI showing health and special stat
    void renderUI(SDL_Renderer* renderer, int positionX, int width) {
        // Render health bar
        SDL_Rect tempRect = { positionX, 580, width, 20 };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red
        SDL_RenderFillRect(renderer, &tempRect);
        SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255); // Green
        tempRect.w = (health / (double) maxHealth) * width;
        SDL_RenderFillRect(renderer, &tempRect);

        // Render special stat bar
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

// Enums for different NPC types. // Only one exists as of the final version.
enum class NPCClasses {
    NONE, FORUMAN
};

// Data for defining NPCs
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

    // Set the NPC class and stats relevant to it
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

    // Update entity position data with args // Basically the same as the "PLAYER_DATA" command but made specifically for movement in case "ENTITY_DATA" would contain something that doesn't fit the entity
    void updateMoveData(std::vector<std::string>& args) {
        switch (npcClass) {
            case NPCClasses::FORUMAN:
                if (args.size() >= 6) {
                    setPos(stoi(args.at(2)), stoi(args.at(3)));
                    setVelocity(stoi(args.at(4)) * 25, stoi(args.at(5)) * 25);
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

    // Move the NPC
    void update(double tpf) {
        switch (npcClass) {
            case NPCClasses::FORUMAN:
                // Foruman only moves while his attacks are on cooldown, making it look like he stops to summon an attack
                // Since attacks can only be summoned by the server to avoid RNG desync, Foruman will stand still until an attack is summoned
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

    // Place the NPC while compensating for delays
    void placeWithDelta(int deltaTime) {
        x += velocityX * (deltaTime / 1000.0);
        y += velocityY * (deltaTime / 1000.0);
    }

    // Render the NPC
    void render(SDL_Renderer* renderer) {
        entity.x = x;
        entity.y = y;
        if (spriteTexture != nullptr) {
            SDL_RenderCopy(renderer, spriteTexture, &sourceRect, &entity);
        }
    }

};
