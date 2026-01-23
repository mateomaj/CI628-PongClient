#include "MyGame.h"

void MyGame::on_receive(std::string cmd, std::vector<std::string>& args) {
    //std::cout << cmd << std::endl;
    if (cmd == "PD") { // Abbreviation - "PLAYER_DATA" // PD,<PlayerID>,<PosX>,<PosY>,<VelX>,<VelY>;
        if (args.size() == 3) {
            int id = stoi(args.at(0));
            if (game_data.playerMap[id] != nullptr) { // New - PlayerData nullptr check // After adding the update message buffer, players can be kicked while their update data stays in the buffer // The behaviour of the crash doesn't match the problem but this check will fix this no matter what the problem is
                game_data.playerMap[id]->setPosition(stoi(args.at(1)), stoi(args.at(2)));
            }
        } else if (args.size() == 5) {
            int id = stoi(args.at(0));
            if (game_data.playerMap[id] != nullptr) {
                PlayerData* player = game_data.playerMap[id];
                player->setPosition(stoi(args.at(1)), stoi(args.at(2)));
                player->setVelocity(stoi(args.at(3)), stoi(args.at(4)));
                if (player != myPlayer && player->velocityX != 0) { // Other players can face towards their velocity so we won't need to store their input handlers
                    player->facingRight = player->velocityX > 0;
                }
            }
        } else {
            std::cout << "BROKEN\n"; // A print just in case the concurrency bug does ever happen so I can catch at least one form of it // I think the crash happened? VS didn't tell me what it was so I'm not sure
        }
    } else if (cmd == "PUPD") { // "PROJECTILE_UPDATE" - Go through the projectile list and edit/delete existing entries
        if (args.size() > 2) {
            if (game_data.activeProjectileCount == 0) return; // Don't bother if no projectiles exist
            int deltaTime = getCurrentTimeMS() - stoll(args.at(0));
            int projectileID = stoi(args.at(1)) - 1;
            int foundProjectiles = 0;
            char* saveptr = NULL;
            ProjectileData* data = nullptr;

            // Read through remaining args as projectile data
            for (int i = 2; i < args.size(); i++) {
                // Search through the projectile list until a non-nullptr entry is found
                // To avoid adding IDs to the message, the args are in the same order as existing projectiles in the list
                // ^ Assuming that every message sent has to be received by the client in the right order through TCP, the list should not desync between the client and server
                for (projectileID++; projectileID < 500; projectileID++) {
                    if ((data = game_data.projectiles[projectileID]) != nullptr) {
                        foundProjectiles++;
                        break;
                    }
                }
                if (data == nullptr) break; // Null check in case the server sends too many arguments and the last projectile slot is empty // VS kept yelling at me about the chance of this being null, now I know why.

                char* copy = strdup(args.at(i).c_str());
                char* pch = strtok_s(copy, ",", &saveptr);
                std::vector<std::string> innerArgs;
                // Check the arg for internal argumens
                while (pch != NULL) {
                    innerArgs.push_back(std::string(pch));
                    pch = strtok_s(NULL, ",", &saveptr);
                }
                free(copy);
                
                data->assumeDespawn = false; // If mentioned in the list, turn off assume despawn
                
                // Handle the projectile based on given data
                if (innerArgs.size() == 0) { // No args - delete projectile
                    game_data.projectiles[projectileID] = nullptr;
                    delete data;
                    foundProjectiles--;
                    game_data.activeProjectileCount--;
                //} else if (innerArgs.size() == 1) { // 1 arg - skip projectile update and use local data /OR/ special state character // 0 & 1 args use one char to get // Skip is useful for ranger sticky traps which stay in place once stuck
                    // Since nothing runs here this whole check can be commented out. Not deleting it because the above explanation is important
                } else if (innerArgs.size() == 2) { // 2 args - update position, maintain velocity
                    data->setPosition(stof(innerArgs.at(0)), stof(innerArgs.at(1)));
                    data->placeWithDelta(deltaTime);
                } else if (innerArgs.size() == 3) { // 3 args - update position & stick projectile
                    data->setPosition(stof(innerArgs.at(1)), stof(innerArgs.at(2)));
                    data->hasGravity = false;
                    data->setVelocity(0, 0);
                } else if (innerArgs.size() == 4) { // 4 args - update position and velocity
                    data->setPosition(stof(innerArgs.at(0)), stof(innerArgs.at(1)));
                    data->setVelocity(stof(innerArgs.at(2)), stof(innerArgs.at(3)));
                    data->placeWithDelta(deltaTime);
                }

                // If all known projectiles were updated, end early
                // This check is only important in case the server gives too many arguments
                // Both this and the above null check make each other kind of useless, but this makes the function finish a bit quicker.
                // ^ They are still both necessary in case foundProjectiles starts showing the wrong number 
                if (foundProjectiles == game_data.activeProjectileCount) break;
            }
        }
    } else if (cmd == "EMOVE") { // "ENTITY_MOVE" - EMOVE,<EntityID>,<MoveData...>
        if (args.size() >= 2) {
            NPCData* data = game_data.npcMap[stoi(args.at(1))];
            if (data == nullptr) return;
            data->updateMoveData(args);
        }
    }
    else if (cmd == "UPH") { // UPH(,<id>,<newValue>,[<iFramesLeft>,]:)...,<timeRef>; // "UPDATE_PLAYER_HEALTH" // [] - Optional // ... - multiple can be added // An infinitely more complicated version of "PH" because adding a <timeRef> for each player is redundant as they should be the same
        // ^ This version saves on characters compared to "PH" ONLY when two or more players take damage on the same frame
        if (args.size() > 2) {
            int size = args.size() - 1;
            int deltaTime = getCurrentTimeMS() - stoll(args.at(size));
            int index = 0;
            // Go through the arguments // Each player has either 3-4 different arguments (including ":" as a separator)
            while (index < size) {
                PlayerData* player = game_data.playerMap[stoi(args.at(index))];
                if (player == nullptr) { // No player found - skip ahead
                    index += 2; // Skip to 3rd argument
                    if (args.at(index) == ":") { // Skip ahead 1 or 2 args depending on if an IFrames value was added
                        index++;
                    } else {
                        index += 2;
                    }
                } else {
                    index++; // Increment to health
                    player->health = SDL_min(SDL_max(stoi(args.at(index)), 0), player->maxHealth);
                    index++; // Increment to IFrames
                    if (args.at(index) == ":") {
                        index++; // No IFrames - increment to the next player's ID
                    } else {
                        player->invincibilityTime = SDL_max(stoi(args.at(index)) - deltaTime, 0);
                        index += 2; // Increment to the next player's ID // +2 because ":" is always the next argument here
                    }
                    // If the health update kills the player, turn off invincibility and prepare them for their dead state
                    if (player->health == 0) {
                        player->invincibilityTime = 0;
                        player->specialStat.setValue(0);
                    }
                }
            }
        }
        // vvv Leaving it here to contrast "UPH"
    //} else if (cmd == "PH") { // "PH" - "PLAYER_HEALTH" - Implies the player's health changed to a different value
    //    if (args.size() >= 2) {
    //        PlayerData* player = game_data.playerMap[stoi(args.at(0))];
    //        if (player == nullptr) return;
    //        player->health = SDL_min(SDL_max(stoi(args.at(1)), 0), player->maxHealth);
    //        if (args.size() == 4) {
    //            // TODO - Add a way of applying i-frames
    //            player->invincibilityTime = SDL_max(stoi(args.at(2)), 0) - (getCurrentTimeMS() - stoll(args.at(3)));
    //        }
    //    }
    } else if (cmd == "PS") { // "PS" - "PLAYER_SPECIAL" // PS,<playerID>,<newValue>(,<timeRef>); // Updating class-specific stats like mana and stamina, with an option to start the stat's regen delay
        if (args.size() >= 2) {
            PlayerData* player = game_data.playerMap[stoi(args.at(0))];
            if (player == nullptr) return;
            player->specialStat.setValue(stod(args.at(1)));
            if (args.size() == 3) {
                player->specialStat.chargeDelayTimer = player->specialStat.chargeDelay - (getCurrentTimeMS() - stoll(args.at(2))) / 1000.0;
            }
        }
    } else if (cmd == "PA") { // "PLAYER_ATTACK" - Spawn an attack specific to the player class with given parameters
        if (args.size() >= 2) {
            spawnPlayerAttack(args);
        }
    } else if (cmd == "SATK") { // "SPAWN_ATTACK" - Spawn boss attack pattern with given parameters
        //std::cout << args.at(3) << std::endl;
        if (args.size() >= 2) {
            spawnAttack(args);
        }
    }
    else if (cmd == "KC") { // "KNIGHT_CHARGE" - Notifies that a knight started charging // KC,<playerID>,<timeRef>;
        if (args.size() == 2) {
            PlayerData* player = game_data.playerMap[stoi(args.at(0))];
            if (player == nullptr) return;
            player->specialStat.chargeSpeed = 1;
            player->specialStat.update(SDL_max(getCurrentTimeMS() - stoll(args.at(1)) - 16, 0) / 1000.0);
            if (player->meleeAttack != nullptr) {
                player->meleeAttack->assumeDespawn = true; // New - KC now marks the knight's melee attack for despawn if it somehow hasn't despawned on its own yet.
            }
        }
    } else if (cmd == "KR") { // "KNIGHT_RELEASE" - Notifies that a knight stopped charging // KR,<playerID> // Not charging != heavy attack BUT heavy attack == not charging // local player can technically work without this but it's needed for other players
        if (args.size() == 1) {
            PlayerData* player = game_data.playerMap[stoi(args.at(0))];
            if (player == nullptr) return;
            player->specialStat.chargeSpeed = 0;
            player->specialStat.setValue(0);
        }
    } else if (cmd == "EH") { // "ENTITY_HEALTH" - EH,<ID>,<Value>;
        if (args.size() == 2) {
            NPCData* npc = game_data.npcMap[stoi(args.at(0))];
            if (npc == nullptr) return;
            npc->health = stoi(args.at(1));
        }
    } else if (cmd == "SE") { // "SPAWN_ENTITY" - SE,<ID>,<Type>(,<SpawnX>,<SpawnY>); // Create a new entity
        if (args.size() >= 2) {
            int id = stoi(args.at(0));
            game_data.npcMap[id] = new NPCData();
            game_data.npcMap[id]->setNPCClass(NPCClasses(stoi(args.at(1))));
            if (args.size() >= 4) {
                game_data.npcMap[id]->setPos(stoi(args.at(2)), stoi(args.at(3)));
            }
        }
    } else if (cmd == "NEWPLAYER") { // NEWPLAYER,<ID>(,<IsMyPlayer>(,<PlayerClass>,<IsReady>)); // Adds new player data to the client's player list // The client can't know its own player's ID until the server tells them through this.
        // ^ Players are only allowed to join while in the lobby state, so the only relevant data is player class and ready status if this client joins a lobby with other players already present.
        if (args.size() == 1) {
            game_data.playerMap[stoi(args.at(0))] = new PlayerData();
            std::cout << "NEW PLAYER ADDED\n";
            playerCount++;
        } else if (args.size() == 2) {
            if (stoi(args.at(1))) {
                myPlayer = (MyPlayerData*)(game_data.playerMap[stoi(args.at(0))] = new PlayerData());
                std::cout << "CLIENT ADDED TO GAME\n";
            } else {
                game_data.playerMap[stoi(args.at(0))] = new PlayerData();
                std::cout << "NEW PLAYER ADDED\n";
            }
            playerCount++;
        } else if (args.size() == 4) {
            if (stoi(args.at(1))) {
                myPlayer = (MyPlayerData*)(game_data.playerMap[stoi(args.at(0))] = new PlayerData());
                myPlayer->setPlayerClass(PlayerClasses(stoi(args.at(2))));
                myPlayer->isReady = stoi(args.at(3));
                std::cout << "CLIENT ADDED TO GAME\n";
            } else {
                game_data.playerMap[stoi(args.at(0))] = new PlayerData();
                game_data.playerMap[stoi(args.at(0))]->setPlayerClass(PlayerClasses(stoi(args.at(2))));
                game_data.playerMap[stoi(args.at(0))]->isReady = stoi(args.at(3));
                std::cout << "NEW PLAYER ADDED\n";
            }
            playerCount++;
        }
    } else if (cmd == "KICKPLAYER") { // KICKPLAYER,<ID>; // Removes the player from the player list and re-orders the list if needed
        if (args.size() == 1) {
            int id = stoi(args.at(0));
            std::cout << "Kicking player " << id << "\n";
            if (game_data.playerMap[id] != nullptr) {
                playerCount--;
                delete game_data.playerMap[id]; // Delete the kicked player's reference to avoid memory leaks
            }
            for (int i = id; i <= MAX_PLAYERS; i++) {
                game_data.playerMap[i] = game_data.playerMap[i + 1];
                if (game_data.playerMap[i + 1] == nullptr) {
                    break;
                }
            }
        }
    } else if (cmd == "CHANGE_CLASS") { // CHANGE_CLASS,<ID>,<ClassID>;
        if (args.size() == 2) {
            game_data.playerMap[stoi(args.at(0))]->setPlayerClass(PlayerClasses(stoi(args.at(1))));
        }
    } else if (cmd == "SET_READY") { // SET_READY,<ID>,<IsReady>;
        if (args.size() == 2) {
            game_data.playerMap[stoi(args.at(0))]->isReady = stoi(args.at(1));
        }
    } else if (cmd == "START_GAME") {
        std::cout << "THE GAME SHOULD START NOW!\n";
        game_data.setReady(true);
    } else if (cmd == "GAME_OVER") { // GAME_OVER(,<DidPartyWin>,<RoundDuration>);
        std::cout << "THE GAME SHOULD END NOW!\n";
        // Delete any projectiles left in the list
        for (int i = 0; i < 500; i++) {
            ProjectileData* data = game_data.projectiles[i];
            if (data != nullptr) {
                game_data.projectiles[i] = nullptr;
                delete data;
            }
        }
        // Game over data setup
        for (int id = 0; id < MAX_PLAYERS; id++) {
            game_data.endData[id].slotActive = false;
        }
        // TODO - It would mean reworking the player list system and there isn't enough time but it would be cool if players that left half way through were also counted
        // Go through all players and store their final stats as game over data
        for (int id = 1; id <= MAX_PLAYERS; id++) {
            PlayerData* data = game_data.playerMap[id];
            if (data == nullptr) break;
            game_data.endData[id - 1].slotActive = true;
            game_data.endData[id - 1].health = data->health;
            game_data.endData[id - 1].maxHealth = data->maxHealth;
            game_data.endData[id - 1].sprite = data->spriteTexture;
            data->facingRight = true; // Players face to the right by default when spawning in. This sets them up for the next round.
            PlayerClasses playerClass = data->playerClass;
            data->setPlayerClass(PlayerClasses::NONE);
            data->setPlayerClass(playerClass);
        }
        if (args.size() == 2) {
            game_data.partyWon = stoi(args.at(0)) != 0;
            game_data.roundDuration = stoi(args.at(1));
        } else { // Just in case no data is given, assume it's a loss.
            game_data.partyWon = false;
            game_data.roundDuration = 0;
        }
        // Change these variables at the end so the main thread goes into the game over screen after all the data is generated.
        game_data.setReady(false);
        game_data.setRunning(false);
    } else if (cmd == "EXIT") {
        std::cout << "PLAYER LOBBY IS FULL OR GAME SESSION IS ACTIVE\n";
    }
}

void MyGame::send(std::string message) {
    messages.push_back(message);
}

void MyGame::input(SDL_Event& event) {
    // Updated key switch with a default for generic keys (a-z, 0-9, etc) because making a new case for each one is painful and inefficient
    switch (event.key.keysym.sym) {
        case SDLK_LSHIFT: // Like explained serverside, JavaFX doesn't separate modifier keys for left and right, so here those keys will be sent as the same key
        case SDLK_RSHIFT: // Note - The key names are case sensitive
            send(event.type == SDL_KEYDOWN ? "Shift_DOWN" : "Shift_UP");
            break;
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            send(event.type == SDL_KEYDOWN ? "Ctrl_DOWN" : "Ctrl_UP");
            break;
        case SDLK_LALT:
        case SDLK_RALT:
            send(event.type == SDL_KEYDOWN ? "Alt_DOWN" : "Alt_UP");
            break;
        case SDLK_ESCAPE: // Ignore escape input as using it to close the program takes priority
            break;
        case SDLK_h:
            // If the client's player is a knight, letting go of attack will reset charge to 0
            // ^ We don't need to reset the charge specifically through a server command as it can be easily handled on clientside 
            if (myPlayer->playerClass == PlayerClasses::KNIGHT && event.type == SDL_KEYUP) {
                myPlayer->specialStat.chargeSpeed = 0;
                myPlayer->specialStat.setValue(0);
            }
            // Continue to the default case because we still need to handle the attack input
        default:
            if (myPlayer->health > 0) { // If client player is dead, disable turning around
                switch (event.key.keysym.sym) { // VERY BASIC TURN AROUND HANDLE
                case SDLK_a:
                    if (event.type == SDL_KEYDOWN) {
                        myPlayer->facingRight = false;
                        myPlayer->isHoldingLeft = true;
                    } else {
                        myPlayer->isHoldingLeft = false;
                        myPlayer->facingRight = myPlayer->isHoldingRight;
                    }
                    break;
                case SDLK_d:
                    if (event.type == SDL_KEYDOWN) {
                        myPlayer->facingRight = true;
                        myPlayer->isHoldingRight = true;
                    } else {
                        myPlayer->isHoldingRight = false;
                        myPlayer->facingRight = !myPlayer->isHoldingLeft;
                    }
                    break;
                }
            }

            // C++ and JavaFX don't use the same names for key references. Inputs like escape, keypad #, +, etc don't match up with how they're defined in FX's KeyCode enum.
            // SDL_GetKeyName() results match up with most of the main keys that you might need, but there are still some that need to be defined manually to fully work on serverside.
            // At least this approach is better than defining every key manually, now custom input binds are more or less possible

            //https://stackoverflow.com/questions/26990270/printing-name-of-a-key-in-sdl - Helped me find the function for getting key names
            send(std::string(SDL_GetKeyName(event.key.keysym.sym)).append(event.type == SDL_KEYDOWN ? "_DOWN" : "_UP"));
            break;
    }
}

// Update entities in client game time
void MyGame::update(double tpf) {
    // Update NPCs
    for (int id = 1; id <= game_data.npcMap.size(); id++) {
        NPCData* data = game_data.npcMap[id];
        if (data == nullptr) continue; // ??? - might be better than break in this case? // This version goes up to the list's size so it doesn't matter here but we still need some kind of null check just in case.
        data->update(tpf);
    } // Note - Since game_data.npcMap.size() is not reliable, we need a different method for tracking the npc count

    // Update projectiles
    int foundProjectiles = 0;
    if (game_data.activeProjectileCount > 0) {
        for (int i = 0; i < 500; i++) {
            if (game_data.projectiles[i] != nullptr) {
                if (!game_data.projectiles[i]->justAdded) { // Don't update projectiles added this frame to compensate for the delay between spawning and updating projectiles on the server
                    if (!game_data.projectiles[i]->assumeDespawn) game_data.projectiles[i]->update(tpf); // If the projectile thinks it should despawn, don't update it
                    if (game_data.projectiles[i]->markedForDespawn) { // Projectiles marked for despawn are deleted - Not really used after the projectile list rework as the server handles removing projectiles now.
                        delete game_data.projectiles[i];
                        //free(game_data.projectiles[i]); https://www.quora.com/Why-does-C-use-free-instead-of-delete-to-deallocate-memory-allocated-by-new - Looks like delete is the better keyword to use when it comes to deleting 'new' instances
                        game_data.projectiles[i] = nullptr;
                        game_data.activeProjectileCount--;
                    } else {
                        foundProjectiles++;
                    }
                } else {
                    game_data.projectiles[i]->justAdded = false;
                    foundProjectiles++;
                }
                if (foundProjectiles >= game_data.activeProjectileCount) break; // Stop searching if all known projectiles are seen
            }
        }
    }

    // Update players - As player movement is handled on the server, this update only focuses on things like mana regeneration that don't interact with the server often.
    for (int id = 1; id <= MAX_PLAYERS; id++) {
        PlayerData* data = game_data.playerMap[id];
        if (data == nullptr) break;
        data->update(tpf);
    }
}

// Update entities on simulated time, tracked based on the last time they were updated by the server.
// Since the new version of this function was added, this version is a failsafe for when the client doesn't receive an update for over a frame;
void MyGame::updateSimulated(double tpf) {
    for (int id = 1; id <= MAX_PLAYERS; id++) {
        PlayerData* data = game_data.playerMap[id];
        if (data == nullptr) break;
        data->simUpdate(tpf);
    }
}

// Update entities on simulated time, tracked based on the last time they were updated by the server. With a parameter for the update data message.
void MyGame::updateSimulated(double tpf, char* updateMessage) {

    // Handle the given update message as you would in onReceive(), and then update simulated as normal

    std::string cmd = "";

    char* outer_saveptr = NULL;
    char* inner_saveptr = NULL;

    char* token = strtok_s(updateMessage, ":", &outer_saveptr);

    while (token != NULL) {
        char* pch = strtok_s(token, ",", &inner_saveptr);
        // get the command, which is the first string in the message
        cmd = std::string(pch);

        // then get the arguments to the command
        std::vector<std::string> args;

        while (pch != NULL) {
            pch = strtok_s(NULL, ",", &inner_saveptr);

            if (pch != NULL) {
                args.push_back(std::string(pch));
            }
        }

        on_receive(cmd, args);

        token = strtok_s(NULL, ":", &outer_saveptr);
    }
    updateSimulated(tpf);
}

void MyGame::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Render the background
    SDL_Rect background = { 0, 0, 800, 600 };
    SDL_RenderCopy(renderer, game_data.textures[11], &background, &background);

    // Render entities // Entities are rendered in the background so they don't cover up players and projectiles
    for (int id = 1; id <= game_data.npcMap.size(); id++) {
        NPCData* data = game_data.npcMap[id];
        if (data == nullptr) continue; // ??? - might be better than break in this case? // This version goes up to the list's size so it doesn't matter here but we still need some kind of null check just in case.
        data->render(renderer);
    }

    // Render projectiles // Projectiles are rendered behind the player so big projectiles don't cover them up
    int foundProjectiles = 0;
    if (game_data.activeProjectileCount > 0) {
        for (int i = 0; i < 500; i++) {
            if (game_data.projectiles[i] != nullptr) {
                if (!game_data.projectiles[i]->assumeDespawn) game_data.projectiles[i]->render(renderer);
                foundProjectiles++;
                if (foundProjectiles >= game_data.activeProjectileCount) break; // stop searching if all known projectiles are seen
            }
        }
    }

    // Render players
    for (int id = 1; id <= MAX_PLAYERS; id++) {
        PlayerData* data = game_data.playerMap[id];
        if (data == nullptr) break;
        if (data == myPlayer) continue;
        data->render(renderer);
    }
    myPlayer->render(renderer); // Rendering the client's player last so they render above the others
    
    renderUI(renderer);
}

// Render the UI used to show names and health/special values of players and NPCs
void MyGame::renderUI(SDL_Renderer* renderer) {
    // Until I rework how bosses are stored to allow multiple bosses to exist at once, I will just add add support for one boss UI
    if (game_data.npcMap[1] != nullptr) {
        NPCData* npc = game_data.npcMap[1];
        
        // Render boss name
        SDL_Rect textRect = { 0, 0, 0, 0 };
        SDL_Surface* textSurface = TTF_RenderText_Blended(game_data.font, npc->name, game_data.defaultFontColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
        textRect.x = 400 - textRect.w / 2.0;
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
        
        // Render boss health bar
        textRect = { 0, textRect.h, 800, 20 };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red
        SDL_RenderFillRect(renderer, &textRect);
        SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255); // Green
        textRect.w = (npc->health / (double)npc->maxHealth) * 800;
        SDL_RenderFillRect(renderer, &textRect);
    }

    // Render player UI
    // This implementation is meant to imitate the layout seen on the server // It's not 1:1 but close enough for a max of 4 players
    if (playerCount > 0) {
        // A bunch of math to poorly copy JavaFX's layout
        int sectionWidth = 800 / playerCount - 5 * (1 / playerCount * (playerCount - 1));
        int maxWidth = SDL_min(180, sectionWidth);
        int startX = 400 - (sectionWidth / 2) * playerCount;
        
        for (int id = 1; id <= MAX_PLAYERS; id++) {
            PlayerData* data = game_data.playerMap[id];
            if (data == nullptr) break;
            
            // Render player name "P<ID>"
            int UIX = startX + sectionWidth * (id - 1) + sectionWidth/2 - maxWidth/2;
            SDL_Rect textRect = { UIX, 580, 0, 0 };
            std::string playerName = "P" + std::to_string(id);
            SDL_Surface* textSurface = TTF_RenderText_Blended(game_data.font, playerName.c_str(), game_data.defaultFontColor);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
            
            // Leave the rest of the rendering to the player for health and special stat bars
            data->renderUI(renderer, UIX + textRect.w + 5, maxWidth - textRect.w - 5);
        }
    }
}

GameData* MyGame::getGameData() { // Even though it's static, I can't get the right instance back within GameUI // Cleanup note - This was before the GameData.h rework where game_data became truly global even though using this function does everything we need
    return &game_data;
}

// Spawn boss attack pattern with given args // Called from the "SATK" command
void MyGame::spawnAttack(std::vector<std::string>& args) {
    int entityID = stoi(args.at(1));
    int id = stoi(args.at(2)); // Attack ID

    //std::cout << LONG_MAX << std::endl;
    //std::cout << INT_MAX << std::endl; // WHY THE F*** IS INT_MAX AND LONG_MAX THE SAME NUMBER, ISN'T THE POINT OF LONG THAT IT'S BIGGER THAN INT??????????????????????? WHAT THE HELL IS A LONG LONG???????????????????????????? // https://stackoverflow.com/questions/7456902/long-vs-int-c-c-whats-the-point - some else asked the same question
    // Apparently long is designed to be at least as long as int, where int is a ""natural" size for the processor", from what I can see long long guarantees a 64bit number
    
    // ^v^v Cleanup note - Edit: swearing. Keeping this here for the funny discovery and because it keeps the source link relevant. // Also keeping a few other discoveries like this below because the complaints are valid.
    
    long long timeRef = stoll(args.at(0));
    ProjectileData* projectilesToAdd[10]; // Max 10 so this doesn't take up too much memory
    
    // Set the spawn amount based on attack pattern
    int projectileAmount = 0;
    switch (id) {
        case 1:
            projectileAmount = 8;
            game_data.npcMap[entityID]->attackCooldown = 2.5; // Attack cooldown is also updated here so we don't add extra delays from another switch-case
            break;
        case 2:
            projectileAmount = 7;
            game_data.npcMap[entityID]->attackCooldown = 3;
            break;
        case 3:
            projectileAmount = 1;
            game_data.npcMap[entityID]->attackCooldown = 5;
            break;
    }
    if (projectileAmount > 0 && game_data.activeProjectileCount <= 500 - projectileAmount) { // Only spawn projectiles if the game can store them in the projectiles array
        //const int pAmount = projectileAmount;
        //ProjectileData* projectilesToAdd[pAmount]; // Wow I hate you
        
        // Create new projectiles and store them in the temporary array
        for (int i = 0; i < projectileAmount; i++) {
            projectilesToAdd[i] = new ProjectileData(id);
        }

        int spawnX; // WHY DOES C++ NEED TO MAKE EVERYTHING SO DIFFICULT! WHY DO I NEED TO DECLARE ALL VARIABLES WITHIN A SWITCH CASE OUTSIDE OF IT??? BECAUSE THIS FREAKING LANGUAGE THINKS I MIGHT USE THEM IN THE OTHER CASES FOR SOME REASON. LET ME CODE IN PEACE!
        int spawnY;
        int gapWidth;
        int direction;

        // Change the projectiles to create the attack pattern using args data
        switch (id) {
            case 1:
                gapWidth = 800 / projectileAmount;
                spawnX = 0;
                if (args.size() >= 4) {
                    spawnX = stoi(args.at(3)) * 15;
                }
                for (int i = 0; i < projectileAmount; i++) {
                    projectilesToAdd[i]->setPosition(spawnX + i * gapWidth, -75);
                    projectilesToAdd[i]->velocityY = 100;
                    projectilesToAdd[i]->rotation = 90;
                }
                break;
            case 2:
                gapWidth = 600 / projectileAmount;
                spawnY = 0;
                direction = 1;
                if (args.size() >= 5) {
                    spawnY = stoi(args.at(4)) * 15;
                    direction = stoi(args.at(3));
                }
                for (int i = 0; i < projectileAmount; i++) {
                    projectilesToAdd[i]->setPosition(direction == 1 ? -75 : 875, spawnY + i * gapWidth);
                    projectilesToAdd[i]->velocityX = direction * 100;
                    projectilesToAdd[i]->rotation = direction == 1 ? 0 : 180;
                }
                break;
            case 3:
                direction = 1;
                if (args.size() >= 4) {
                    direction = stoi(args.at(3));
                }
                for (int i = 0; i < projectileAmount; i++) {
                    projectilesToAdd[i]->pivotY = -1200;
                    projectilesToAdd[i]->setPosition(direction == 1 ? -75 : 875, 300);
                    projectilesToAdd[i]->velocityX = direction * 100;
                    projectilesToAdd[i]->rotation = direction == 1 ? 0 : 180;
                    projectilesToAdd[i]->lifetime = 6;
                }
                break;
        }

        int timeOffset = getCurrentTimeMS() - timeRef;

        //std::cout << currentTime << " - " << timeRef << " = " <<  timeOffset << std::endl; // It's crazy to think about the time difference here being 0-2ms at most, I know this is a localHost connection but this also considers the time between when the message is written, sent, received, and processed up to this point, all within milliseconds.

        // Set the attack cooldown using deltatime since the message was sent
        game_data.npcMap[entityID]->attackCooldown -= timeOffset / 1000.0;
        for (int i = 0; i < projectileAmount; i++) {
            projectilesToAdd[i]->placeWithDelta(timeOffset);
            projectilesToAdd[i]->lifetime -= timeOffset / 1000.0;
        }

        // Yes there are concurrency issues here. But they only apply to the timing of when the projectiles are added vs when the next update will happen or is happening. In theory the only things that could happen here is not updating all new projectiles in the main loop because they haven't been added yet, and desyncing the projectiles from the server by an extra tick ahead by running update right as they are added in (which would probably align them better because of delays). The effect here is negligible so I'm not spending 2 days on trying to avoid it.
        // Also, this thread is the only place where this function is called. I plan to spawn attacks on server response so timers are easier to sync up, so the projectile list only gets filled on this thread, freeing up space can happen on either of them.

        // Add projectiles to the main projectile list
        for (int i = 0; i < 500; i++) {
            if (game_data.projectiles[i] == nullptr) {
                projectileAmount--;
                game_data.projectiles[i] = projectilesToAdd[projectileAmount];
                game_data.activeProjectileCount++;
            }
            if (projectileAmount == 0) {
                break; // Break once all projectiles are added
            }
        }
    }
}

// Spawn player attack from given args // Called from the "PA" command
// The code follows the same idea as spawnAttack() but it's player class specific
void MyGame::spawnPlayerAttack(std::vector<std::string>& args) {
    int playerID = stoi(args.at(0));
    PlayerData* player = game_data.playerMap[playerID];
    if (player == nullptr || game_data.activeProjectileCount >= 500) return;
    long long timeRef = getCurrentTimeMS() - stoll(args.at(1));
    ProjectileData* playerProjectile = nullptr;
    
    // Check player class and args count to spawn the correct attack
    if (player->playerClass == PlayerClasses::KNIGHT && args.size() >= 4) {
        player->specialStat.setValue(0);
        int angle = stoi(args.at(3));
        if (stoi(args.at(2)) == 0) { // Check if this is a normal or charged attack to pick the right sprite for it
            playerProjectile = new ProjectileData(4);
        } else {
            playerProjectile = new ProjectileData(5);
        }
        playerProjectile->rotation = angle;
        playerProjectile->bindX = &player->x;
        playerProjectile->bindY = &player->y;
        
        // Place the sword attack in the right place relative to the player.
        // Entity width and height are used so the placement should be the same for either sprite.
        // The sprite position doesn't exactly match what it was on the server, but it's close enough.
        if (angle == 90) { // down
            playerProjectile->x = (20 - playerProjectile->entity.w) / 2.0;
            playerProjectile->y = playerProjectile->entity.w/2.0 - playerProjectile->entity.h/2.0 + 25;
        } else if (angle == 180) { // left
            playerProjectile->x = -(playerProjectile->entity.w + 5);
            playerProjectile->y = (20 - playerProjectile->entity.h) / 2.0;
        } else if (angle == -90) { // up
            playerProjectile->x = (20 - playerProjectile->entity.w) / 2.0;
            playerProjectile->y = -playerProjectile->entity.w / 2.0 - playerProjectile->entity.h / 2.0 - 5;
        } else { // right
            playerProjectile->x = 25;
            playerProjectile->y = (20 - playerProjectile->entity.h) / 2.0;
        }
    } else if (player->playerClass == PlayerClasses::RANGER && args.size() >= 5) {
        int directionX = stoi(args.at(3));
        int directionY = stoi(args.at(4));
        double h = hypot(directionX, directionY);
        if (stoi(args.at(2)) == 0) {
            playerProjectile = new ProjectileData(6);
            playerProjectile->setVelocity((directionX / h) * 250, (directionY / h) * 250);
        } else {
            playerProjectile = new ProjectileData(7);
            playerProjectile->setVelocity((directionX / h) * 200, (directionY / h) * 200);
        }
        playerProjectile->setPosition(player->x, player->y); // Placeholder - Spawn at player position. The projectiles will self-correct on the next PUPD call
    } else if (player->playerClass == PlayerClasses::MAGE && args.size() >= 5) {
        player->specialStat.setValue(stod(args.at(4)));
        player->specialStat.chargeDelayTimer = player->specialStat.chargeDelay - timeRef / 1000.0;
        int directionX = stoi(args.at(2));
        int directionY = stoi(args.at(3));
        playerProjectile = new ProjectileData(8);
        double h = hypot(directionX, directionY);
        playerProjectile->setVelocity((directionX / h) * 250, (directionY / h) * 250);
        playerProjectile->setPosition(player->x, player->y); // Placeholder - Spawn at player position. The projectiles will self-correct on the next PUPD call
    }
    if (playerProjectile == nullptr) return;

    // Adjust projectile lifetime by deltatime
    playerProjectile->lifetime -= timeRef / 1000.0;

    // If the player is a knight, give them the projectile reference. // Otherwise, add the projectile to the projectile list.
    if (player->playerClass == PlayerClasses::KNIGHT) {
        if (player->meleeAttack != nullptr) {
            delete player->meleeAttack; // This shouldn't happen at all but it solves a memory leak.
            player->meleeAttack = nullptr; // If this ever calls it might cause a concurrency error where the projectile is deleted twice. This line hopefully lowers the chance of that
        }
        player->meleeAttack = playerProjectile;
    } else {
        playerProjectile->placeWithDelta(timeRef);
        for (int i = 0; i < 500; i++) {
            if (game_data.projectiles[i] == nullptr) {
                game_data.projectiles[i] = playerProjectile;
                game_data.activeProjectileCount++;
                break;
            }
        }
    }
}

// Create texture references for all relevant sprites within the asset folder and store them under a sprite ID
// For the sake of knowing exactly what ID belongs to which texture, these references will be defined manually
void MyGame::initTextures(SDL_Renderer* renderer) {
    SDL_Surface* tempSurface = IMG_Load("Assets/Textures/BlankPlayer.png");
    game_data.textures[0] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    tempSurface = IMG_Load("Assets/Textures/Knight.png");
    game_data.textures[1] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    tempSurface = IMG_Load("Assets/Textures/Ranger.png");
    game_data.textures[2] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    tempSurface = IMG_Load("Assets/Textures/Mage.png");
    game_data.textures[3] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    tempSurface = IMG_Load("Assets/Textures/Foruman.png");
    game_data.textures[4] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    tempSurface = IMG_Load("Assets/Textures/angel sword.png");
    game_data.textures[5] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    tempSurface = IMG_Load("Assets/Textures/sword.png");
    game_data.textures[6] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    tempSurface = IMG_Load("Assets/Textures/super sword.png");
    game_data.textures[7] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    tempSurface = IMG_Load("Assets/Textures/arrow.png");
    game_data.textures[8] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    tempSurface = IMG_Load("Assets/Textures/spike trap.png");
    game_data.textures[9] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    tempSurface = IMG_Load("Assets/Textures/fireball.png");
    game_data.textures[10] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    tempSurface = IMG_Load("Assets/Textures/stage v3.png");
    game_data.textures[11] = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    TTF_Init();
    game_data.font = TTF_OpenFont("Assets/Fonts/pong.ttf", 20);

    // Basic init for player game over data
    // For the sake of not creating an entire function just for this for loop, it's added here as this is an init function
    for (int id = 0; id < MAX_PLAYERS; id++) {
        game_data.endData->id = id + 1;
    }
}

// Unload all assets when shutting down the game
void MyGame::onShutDown() {
    for (int id = 0; id < game_data.textures.size(); id++) {
        if (game_data.textures[id] != nullptr) {
            SDL_DestroyTexture(game_data.textures[id]);
        }
    }
    TTF_CloseFont(game_data.font);
    TTF_Quit();
}
 
MyGame::~MyGame() {
    onShutDown();
}