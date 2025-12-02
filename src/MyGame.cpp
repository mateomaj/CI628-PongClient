#include "MyGame.h"

void MyGame::on_receive(std::string cmd, std::vector<std::string>& args) {
    //std::cout << cmd << std::endl;
    if (cmd == "GAME_DATA") {
        // we should have exactly 4 arguments
        /*
        if (args.size() == 4) {
            game_data.player1Y = stoi(args.at(0));
            game_data.player2Y = stoi(args.at(1));
            game_data.ballX = stoi(args.at(2));
            game_data.ballY = stoi(args.at(3));
        }
        */
    //} else if (cmd == "PLAYER_DATA") {
    } else if (cmd == "PD") { // Abbreviation
        if (args.size() == 3) {
            int id = stoi(args.at(0));
            game_data.playerMap[id]->setPosition(stoi(args.at(1)), stoi(args.at(2)));
        } else if (args.size() == 5) {
            int id = stoi(args.at(0));
            //game_data.playerMap[id]->setPosition(stoi(args.at(1)), stoi(args.at(2)));
            //game_data.playerMap[id]->setVelocity(stoi(args.at(3)), stoi(args.at(4)));
            PlayerData* player = game_data.playerMap[id];
            player->setPosition(stoi(args.at(1)), stoi(args.at(2)));
            player->setVelocity(stoi(args.at(3)), stoi(args.at(4)));
            if (player != myPlayer && player->velocityX != 0) { // Other players can face towards their velocity so we won't need to store their input handlers
                player->facingRight = player->velocityX > 0;
            }
        } else {
            std::cout << "BROKEN\n"; // A print just in case the concurrency bug does ever happen so I can catch at least one form of it // I think the crash happened? VS didn't tell me what it was so I'm not sure
        }
    } else if (cmd == "SATK") {
        //std::cout << args.at(3) << std::endl;
        if (args.size() >= 2) {
            spawnAttack(args);
        }
    } else if (cmd == "NEWPLAYER") {
        //std::cout << args.size() << std::endl;
        if (args.size() == 1) {
            game_data.playerMap[stoi(args.at(0))] = new PlayerData();
            std::cout << "NEW PLAYER ADDED\n";
        } else if (args.size() == 2) {
            //int id = stoi(args.at(0));
            //game_data.playerMap[id] = new PlayerData();
            //myPlayer = game_data.playerMap[id];
            if (stoi(args.at(1))) {
                myPlayer = (MyPlayerData*)(game_data.playerMap[stoi(args.at(0))] = new PlayerData());
                std::cout << "CLIENT ADDED TO GAME\n";
            } else {
                game_data.playerMap[stoi(args.at(0))] = new PlayerData();
                std::cout << "NEW PLAYER ADDED\n";
            }
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
        }
    } else if (cmd == "KICKPLAYER") {
        std::cout << "Kicking player ";
        if (args.size() == 1) {
            int id = stoi(args.at(0));
            std::cout << id << "\n";
            for (int i = id; i <= MAX_PLAYERS; i++) {
                //game_data.playerMap[id] = nullptr;
                //if (game_data.playerMap[id + 1] != nullptr) {
                //    game_data.playerMap[id] = game_data.playerMap[id + 1];
                //}
                //std::cout << id << ", " << i << ", " << game_data.playerMap[i] << ", " << game_data.playerMap[i + 1] << std::endl;
                game_data.playerMap[i] = game_data.playerMap[i + 1];
                if (game_data.playerMap[i + 1] == nullptr) {
                    break;
                }
            }
        } else {
            std::cout << "\n";
        }
    } else if (cmd == "CHANGE_CLASS") {
        if (args.size() == 2) {
            game_data.playerMap[stoi(args.at(0))]->setPlayerClass(PlayerClasses(stoi(args.at(1))));
        }
    } else if (cmd == "SET_READY") {
        if (args.size() == 2) {
            game_data.playerMap[stoi(args.at(0))]->isReady = stoi(args.at(1));
            //std::cout << "SET READY - " << args.at(1) << std::endl;
        }
    } else if (cmd == "START_GAME") {
        std::cout << "THE GAME SHOULD START NOW YIPEEEEEEEEEEEEEEEEEEEEEE\n";
        game_data.setReady(true);
        /*
        for (int id = 1; id <= MAX_PLAYERS; id++) {
            PlayerData* data = game_data.playerMap[id];
            if (data == nullptr) break;
            data->isReady = false; // TODO - Don't forget to do this serverside too if I really want to make the game loop without closing
        }*/
    } else if (cmd == "GAME_OVER") {
        std::cout << "THE GAME SHOULD END NOW BOOOOOOOOOOOOOOOOOOOOOOOOOO\n";
        game_data.setReady(false);
        game_data.setRunning(false);
    } else if (cmd == "EXIT") {
        std::cout << "PLAYER LOBBY IS FULL OR GAME SESSION IS ACTIVE\n";
        // Despawn everything I guess
    } else {
        std::cout << "Received: " << cmd << std::endl;
        //std::cout << "\n\n\n\n\n" << "Received: " << cmd << std::endl << "\n\n\n\n\n";
    }
}

void MyGame::send(std::string message) {
    messages.push_back(message);
}

void MyGame::input(SDL_Event& event) {
    //std::cout << SDL_GetKeyName(event.key.keysym.sym) << std::endl;
    //std::cout << "I hate you: " << (int) (char) event.key.keysym.sym << std::endl;

    // Updated key switch with a default for generic keys (a-z, 0-9, etc) because making a new case for each one is painful and inefficient
    switch (event.key.keysym.sym) {
        
        //case SDLK_UP: // Turns out these match up in SDL_GetKeyName()
            //send(event.type == SDL_KEYDOWN ? "UP_DOWN" : "UP_UP");
            //break;
        //case SDLK_DOWN:
            //send(event.type == SDL_KEYDOWN ? "DOWN_DOWN" : "DOWN_UP");
            //break;
        //case SDLK_LEFT:
            //send(event.type == SDL_KEYDOWN ? "LEFT_DOWN" : "LEFT_UP");
            //break;
        //case SDLK_RIGHT:
            //send(event.type == SDL_KEYDOWN ? "RIGHT_DOWN" : "RIGHT_UP");
            //break;
        //case SDLK_SPACE:
            //send(event.type == SDL_KEYDOWN ? "SPACE_DOWN" : "SPACE_UP");
            //break;
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
        /*case SDLK_1: // Change class debug
            if (event.type == SDL_KEYDOWN) {
                send("CHANGE_CLASS1");
                myPlayer->setPlayerClass(PlayerClasses::KNIGHT);
            }
            break;
        case SDLK_2:
            if (event.type == SDL_KEYDOWN) {
                send("CHANGE_CLASS2");
                myPlayer->setPlayerClass(PlayerClasses::RANGER);
            }
            break;
        case SDLK_3:
            if (event.type == SDL_KEYDOWN) {
                send("CHANGE_CLASS3");
                myPlayer->setPlayerClass(PlayerClasses::MAGE);
            }
            break;*/
        case SDLK_ESCAPE: // Ignore escape input as using it to close the program takes priority
            break;
        default:
            /*
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) { // VERY BASIC TURN AROUND HANDLE // TODO - IMPROVE
                case SDLK_a:
                    myPlayer->facingRight = false;
                    break;
                case SDLK_d:
                    myPlayer->facingRight = true;
                    break;
                }
            }*/
            
            switch (event.key.keysym.sym) { // VERY BASIC TURN AROUND HANDLE // TODO - IMPROVE
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
            
            //char kms[1] = { (char)event.key.keysym.sym };
            //std::cout << (char)event.key.keysym.sym << "_Down" << std::endl;
            //std::cout << kms + "_Down" << std::endl;
            //strcat(kms, "_Down");
            //std::cout << kms << std::endl; // Prints - d[redacted]d_Down // WHAT THE FUCK IS THIS, I HATE THIS CLOWN ASS LANGUAGE! I JUST WANT TO ADD A FUCKING CHARACTER TO A FUCKING STRING, WHY IS IT SO DIFFICULT??? // (I couldn't paste the whole thing in but it's "d" a billion U+2560 "d_Down")
            //std::cout << std::strcat((char*)event.key.keysym.sym, "_Down") << std::endl;

            //char kys[100] = { (char)event.key.keysym.sym };
            //char kms[999] = "_Down";
            //strcat(kys, kms); // Works, I hate this thing
            //std::cout << kys << std::endl;

            //std::cout << std::string(SDL_GetKeyName(event.key.keysym.sym)).append("_Down") << std::endl;
           
            //send(event.type == SDL_KEYDOWN ? ((char)event.key.keysym.sym) + "_DOWN" : ((char)event.key.keysym.sym)+ "_UP");

            //send(std::string(SDL_GetKeyName(event.key.keysym.sym)).append(event.type == SDL_KEYDOWN ? "_DOWN" : "_UP").c_str()); // FINALLY



            // C++ and JavaFX don't use the same names for key references. Inputs like escape, keypad #, +, etc don't match up with how they're defined in FX's KeyCode enum.
            // SDL_GetKeyName() results match up with most of the main keys that you might need, but there are still some that need to be defined manually to fully work on serverside.
            // At least this approach is better than defining every key manually, now custom input binds are more or less possible

            //https://stackoverflow.com/questions/26990270/printing-name-of-a-key-in-sdl - Helped me find the function for getting key names
            send(std::string(SDL_GetKeyName(event.key.keysym.sym)).append(event.type == SDL_KEYDOWN ? "_DOWN" : "_UP")); // FINALLY
            break;
    }

    // Old input switch
    /*
    switch (event.key.keysym.sym) {
        case SDLK_w:
            send(event.type == SDL_KEYDOWN ? "W_DOWN" : "W_UP");
            break;
        case SDLK_s:
            send(event.type == SDL_KEYDOWN ? "S_DOWN" : "S_UP"); // New - Downward movement
            break;
        case SDLK_a:
            send(event.type == SDL_KEYDOWN ? "A_DOWN" : "A_UP");
            break;
        case SDLK_d:
            send(event.type == SDL_KEYDOWN ? "D_DOWN" : "D_UP");
            break;
        case SDLK_SPACE:
            send(event.type == SDL_KEYDOWN ? "SPACE_DOWN" : "SPACE_UP");
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            send(event.type == SDL_KEYDOWN ? "SHIFT_DOWN" : "SHIFT_UP");
            break;
        case SDLK_h:
            send(event.type == SDL_KEYDOWN ? "H_DOWN" : "H_UP");
            break;
        case SDLK_UP:
            send(event.type == SDL_KEYDOWN ? "I_DOWN" : "I_UP"); // Player 2 has I,K movement in the server script
            break;
        case SDLK_DOWN:
            send(event.type == SDL_KEYDOWN ? "K_DOWN" : "K_UP");
            break;
        case SDLK_i:
            send(event.type == SDL_KEYDOWN ? "I_DOWN" : "I_UP"); // Player 2 has I,K movement in the server script
            break;
        case SDLK_k:
            send(event.type == SDL_KEYDOWN ? "K_DOWN" : "K_UP");
            break;
    }
    */
}

//void MyGame::clickInput(SDL_Event& event) {
//    std::cout << "oi\n";
//}

//void MyGame::update() {
void MyGame::update(double tpf) {
    //std::cout << tpf << std::endl;
    /*
    player1.y = game_data.player1Y;
    player2.y = game_data.player2Y; // New - Player 2 handling
    ball.x = game_data.ballX; // New - Ball handling
    ball.y = game_data.ballY;
    */

    // New - Update projectiles
    int foundProjectiles = 0;
    if (game_data.activeProjectileCount > 0) {
        for (int i = 0; i < 500; i++) {
            if (game_data.projectiles[i] != nullptr) {
                if (!game_data.projectiles[i]->justAdded) {
                    game_data.projectiles[i]->update(tpf);
                    if (game_data.projectiles[i]->markedForDespawn) {
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
                if (foundProjectiles >= game_data.activeProjectileCount) break; // stop searching if all known projectiles are seen
            }
        }
    }
}

void MyGame::updateSimulated(double tpf) {
    //std::cout << tpf << std::endl;
    for (int id = 1; id <= MAX_PLAYERS; id++) {
        PlayerData* data = game_data.playerMap[id];
        if (data == nullptr) break;
        data->update(tpf);
        //std::cout << data->entity.y << std::endl;
        //data->setSimOffsets(data->velocityX * tpf, data->velocityY * tpf);
    }
}

void MyGame::updateSimulated(double tpf, char* updateMessage) {
//void MyGame::updateSimulated(double tpf, std::string updateMessage) {
    //std::cout << updateMessage << std::endl;
    //std::cout << updateMessage << " - " << updateMessage << std::endl; // When printing these out, a random ";" is added to the message. But from what I can tell it's just a weird bug with the print, the actual message doesn't have it // Aparently it does???????????????????????????????????????????????????????????? // Aparently this is some quantum semi-colon trash
    
    // Handle the given update message as you would in onReceive(), and then update simulated as normal

    std::string cmd = "";

    char* outer_saveptr = NULL;
    char* inner_saveptr = NULL;

    //char* message;
    //message = updateMessage.assign();

    char* token = strtok_s(updateMessage, ":", &outer_saveptr);
    //char* token = strtok_s(updateMessage., ":", &outer_saveptr);

    while (token != NULL) {
        char* pch = strtok_s(token, ",", &inner_saveptr);
        //std::cout << "prebitch\n";
        // get the command, which is the first string in the message
        cmd = std::string(pch);
        //std::cout << "postbitch\n";

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

    // New - Render projectiles
    int foundProjectiles = 0;
    if (game_data.activeProjectileCount > 0) {
        for (int i = 0; i < 500; i++) {
            if (game_data.projectiles[i] != nullptr) {
                game_data.projectiles[i]->render(renderer);
                foundProjectiles++;
                if (foundProjectiles >= game_data.activeProjectileCount) break; // stop searching if all known projectiles are seen
            }
        }
    }

    for (int id = 1; id <= MAX_PLAYERS; id++) {
        PlayerData* data = game_data.playerMap[id];
        if (data == nullptr) break;
        if (data == myPlayer) continue;
        data->render(renderer);
        //if (game_data.playerMap[id] == nullptr) break;
        //if (game_data.playerMap[id] == myPlayer) continue;
        //game_data.playerMap[id]->render(renderer);
        //SDL_RenderDrawRect(renderer, &game_data.playerMap[id]->getRect());
        //SDL_RenderDrawRect(renderer, &game_data.playerMap[id]->entity);
    }
    myPlayer->render(renderer); // Rendering the client's player last so they render above the others
    /*
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &player1);
    SDL_RenderDrawRect(renderer, &player2); // New - Player 2 render
    SDL_RenderFillRect(renderer, &ball);//SDL_RenderDrawRect(renderer, &ball); // New - Render ball
    */
}

//GameData MyGame::getGameData() { // Even though it's static, I can't get the right instance back within GameUI
GameData* MyGame::getGameData() { // Even though it's static, I can't get the right instance back within GameUI
    return &game_data;
}

void MyGame::spawnAttack(std::vector<std::string>& args) {
    int id = stoi(args.at(1));
    //std::cout << args.at(0) << std::endl;
    //std::cout << LONG_MAX << std::endl;
    //std::cout << INT_MAX << std::endl; // WHY THE FUCK IS INT_MAX AND LONG_MAX THE SAME NUMBER, ISN'T THE POINT OF LONG THAT IT'S BIGGER THAN INT??????????????????????? WHAT THE HELL IS A LONG LONG???????????????????????????? // https://stackoverflow.com/questions/7456902/long-vs-int-c-c-whats-the-point - some else asked the same question
    // Apparently long is designed to be at least as long as int, where int is a ""natural" size for the processor", from what I can see long long guarantees a 64bit number
    //long timeRef = stol(args.at(0));
    long long timeRef = stoll(args.at(0));
    ProjectileData* projectilesToAdd[10]; // Max 10 so this doesn't take up too much memory
    //ProjectileData* projectilesToAdd = new ProjectileData[10]; // Max 10 so this doesn't take up too much memory
    // Spawn amount
    int projectileAmount = 0;
    switch (id) {
        case 1:
            projectileAmount = 8;
            break;
        case 2:
            projectileAmount = 7;
            break;
        case 3:
            projectileAmount = 1;
            break;
    }
    if (projectileAmount > 0 && game_data.activeProjectileCount <= 500 - projectileAmount) { // only spawn projectiles if the game can store them in the projectiles array
        //const int pAmount = projectileAmount;
        //ProjectileData* projectilesToAdd[pAmount]; // Wow I hate you
        for (int i = 0; i < projectileAmount; i++) {
            projectilesToAdd[i] = new ProjectileData(id);
        }
        int spawnX; // WHY DOES C++ NEED TO MAKE EVERYTHING SO DIFFICULT! WHY DO I NEED TO DECLARE ALL VARIABLES WITHIN A SWITCH CASE OUTSIDE OF IT??? BECAUSE THIS FREAKING LANGUAGE THINKS I MIGHT USE THEM IN THE OTHER CASES FOR SOME REASON. LET ME CODE IN PEACE!
        int spawnY;
        int gapWidth;
        int direction;
        switch (id) {
            case 1:
                //int gapWidth = 800 / projectileAmount;
                gapWidth = 800 / projectileAmount;
                //int spawnX = 0;
                spawnX = 0;
                if (args.size() >= 3) {
                    spawnX = stoi(args.at(2)) * 15;
                }
                for (int i = 0; i < projectileAmount; i++) {
                    //projectilesToAdd[i]->entity.x = spawnX + i * gapWidth;
                    //projectilesToAdd[i]->entity.y = -75;
                    projectilesToAdd[i]->setPosition(spawnX + i * gapWidth, -75);
                    projectilesToAdd[i]->velocityY = 100;
                    projectilesToAdd[i]->rotation = 90;
                }
                break;
            case 2:
                //int gapWidth = 600 / projectileAmount;
                gapWidth = 600 / projectileAmount;
                //int spawnY = 0; // I hate you I hate you I hate you
                spawnY = 0;
                //int direction = 1;
                direction = 1;
                if (args.size() >= 4) {
                    spawnY = stoi(args.at(3)) * 15;
                    direction = stoi(args.at(2));
                }
                for (int i = 0; i < projectileAmount; i++) {
                    projectilesToAdd[i]->setPosition(direction == 1 ? -75 : 875, spawnY + i * gapWidth);
                    projectilesToAdd[i]->velocityX = direction * 100;
                    projectilesToAdd[i]->rotation = direction == 1 ? 0 : 180;
                }
                break;
            case 3:
                //int direction = 1;
                direction = 1;
                if (args.size() >= 3) {
                    direction = stoi(args.at(2));
                }
                for (int i = 0; i < projectileAmount; i++) {
                    projectilesToAdd[i]->setPosition(direction == 1 ? -75 : 875, -800);
                    projectilesToAdd[i]->velocityX = direction * 100;
                    projectilesToAdd[i]->rotation = direction == 1 ? 0 : 180;
                }
                break;
        }
        //long long currentTime = getCurrentTimeMS();
        int timeOffset = getCurrentTimeMS() - timeRef;
        //int timeOffset = currentTime - timeRef;
        //std::cout << currentTime << " - " << timeRef << " = " <<  timeOffset << std::endl; // It's crazy to think about the time difference here being 0-2ms at most, I know this is a localHost connection but this also considers the time between the message is written, sent, received, and processed up to this point, all within milliseconds.
        for (int i = 0; i < projectileAmount; i++) {
            projectilesToAdd[i]->placeWithDelta(timeOffset);
        }
        // Yes there are concurrency issues here. But they only apply to the timing of when the projectiles are added vs when the next update will happen or is happening. In theory the only things that could happen here is not updating all new projectiles in the main loop because they haven't been added yet, and desyncing the projectiles from the server by an extra tick ahead by running update right as they are added in (which would probably align them better because of delays). The effect here is negligible so I'm not spending 2 days on trying to avoid it.
        // Also, this thread is the only place where this function is called. I plan to spawn attacks on server response so timers are easier to sync up, so the projectile list only gets filled on this thread, freeing up space can happen on either of them.
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
    //delete[] projectilesToAdd; // Delete the projectiles array as we don't need it
}