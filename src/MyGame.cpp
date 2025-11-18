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
    } else if (cmd == "PLAYER_DATA") {
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
        }
    } else if (cmd == "NEWPLAYER") {
        if (args.size() == 1) {
            game_data.playerMap[stoi(args.at(0))] = new PlayerData();
            std::cout << "NEW PLAYER ADDED\n";
        } else if (args.size() == 2 && stoi(args.at(1))) {
            //int id = stoi(args.at(0));
            //game_data.playerMap[id] = new PlayerData();
            //myPlayer = game_data.playerMap[id];
            myPlayer = (MyPlayerData*) (game_data.playerMap[stoi(args.at(0))] = new PlayerData()); // works?
            std::cout << "CLIENT ADDED TO GAME\n";
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
    } else {
        std::cout << "Received: " << cmd << std::endl;
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
        case SDLK_1: // Change class debug
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
            break;
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

void MyGame::update() {
    /*
    player1.y = game_data.player1Y;
    player2.y = game_data.player2Y; // New - Player 2 handling
    ball.x = game_data.ballX; // New - Ball handling
    ball.y = game_data.ballY;
    */
}

void MyGame::updateSimulated(double tpf) {
    //std::cout << tpf << std::endl;
    for (int id = 1; id <= MAX_PLAYERS; id++) {
        PlayerData* data = game_data.playerMap[id];
        if (data == nullptr) break;
        data->setSimOffsets(data->velocityX * tpf, data->velocityY * tpf);
    }
}

void MyGame::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int id = 1; id <= MAX_PLAYERS; id++) {
        if (game_data.playerMap[id] == nullptr) break;
        game_data.playerMap[id]->render(renderer);
        //SDL_RenderDrawRect(renderer, &game_data.playerMap[id]->getRect());
        //SDL_RenderDrawRect(renderer, &game_data.playerMap[id]->entity);
    }
    /*
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &player1);
    SDL_RenderDrawRect(renderer, &player2); // New - Player 2 render
    SDL_RenderFillRect(renderer, &ball);//SDL_RenderDrawRect(renderer, &ball); // New - Render ball
    */
}