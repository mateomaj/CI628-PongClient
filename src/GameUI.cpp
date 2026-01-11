#include "GameUI.h"
//#include "Main.cpp"

void GameLobby::render(SDL_Renderer* renderer, MyGame* game) {
    SDL_Rect srcRect = { 0, 0, 20, 20 };
    for (int id = 1; id <= game->MAX_PLAYERS; id++) {
        SDL_Rect dstRect = { 210 + (id - 1) * 100, 260, 80, 80 };
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
        SDL_RenderFillRect(renderer, &dstRect);

        //PlayerData* data = game->getGameData().playerMap[id];
        PlayerData* data = game->getGameData()->playerMap[id]; // Cool Note - Every time we try to read a slot within the list using array index, that checked slot gets counted when calling playerMap.size()
        //PlayerData* data = game->getGameData()->playerMap.at(id); // ^ Trying playerMap.at() throws an out of bounds error instead of returning null
        //std::cout << (game->getGameData()->playerMap.find(id)) << std::endl; // ^ The find() function gives a weird variable that I can't really work with // I will just have to live with the weird size increase;
        //std::cout << "nullptr? - " << id << std::endl;
        if (data == nullptr) continue;
        //std::cout << "not nullptr\n";

        //data->checkTexture(renderer); // The context where a function is called for the first time defines which context's global variables that function is allowed to use. Because checkTexture is called here first, it makes all future calls of it reference GameUI's game_data global variable instead of the one present in MyGame.h, WHAT A FUCKING JOKE, no wonder I couldn't see anything wrong with this, because this bullshit exists
        // I don't want to swear too much in these comments because they get saved in commit history but how the hell am I not meant to crash out when every time I write two lines of code I run into the most bullshit c++ quirk I've ever seen in my life that makes me get stuck trying to fix it for two day minimum. This project should have been done at this point but it's random garbage like this that keeps ruining my sleep schedule because what do you mean a function's global scope is based on which file it was called from first... why is it like that? I can see some cool stuff being done with it but it only gets in the way.

        data->checkTexture(renderer);
        if (data->spriteTexture != nullptr) {
            SDL_RenderCopy(renderer, data->spriteTexture, &srcRect, &dstRect);
        }
        //TTF_RenderTextBlended
        if (data->isReady) {
            //std::cout << "Is ready\n";
            SDL_SetRenderDrawColor(renderer, 120, 250, 120, 255);
            SDL_RenderDrawRect(renderer, &dstRect);
        }
    }
} // TODO - Add text into UI showing player names and controls

bool GameLobby::loop(SDL_Renderer* renderer, MyGame* game) {
	SDL_Event event;

	const int frameDelay = 1000 / 60;
	int frameStart, frameTime;
    //std::cout << game->getGameData().isReady() << std::endl;
	//while (!game->getGameData().isReady()) {//while (is_running) {
	while (!game->getGameData()->isReady()) {
        if (game->shouldForceQuit()) return 0;
		//std::cout << is_running << std::endl;
		frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&event)) {
            //if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat == 0) {
            if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                switch (event.key.keysym.sym) {
                case SDLK_1: // Change class debug
                    if (event.type == SDL_KEYDOWN) {
                        game->send("CHANGE_CLASS1");
                        game->myPlayer->setPlayerClass(PlayerClasses::KNIGHT);
                    }
                    break;
                case SDLK_2:
                    if (event.type == SDL_KEYDOWN) {
                        game->send("CHANGE_CLASS2");
                        game->myPlayer->setPlayerClass(PlayerClasses::RANGER);
                    }
                    break;
                case SDLK_3:
                    if (event.type == SDL_KEYDOWN) {
                        game->send("CHANGE_CLASS3");
                        game->myPlayer->setPlayerClass(PlayerClasses::MAGE);
                    }
                    break;
                case SDLK_RETURN:
                    //std::cout << "RETURN\n";
                    if (game->myPlayer->playerClass != PlayerClasses::NONE) {
                        //std::cout << "THE ROOK\n";
                        game->send("TOGGLE_READY");
                    }
                    break;
                case SDLK_ESCAPE:
                    game->send("DISCONNECT");
                    SDL_Delay(100);
                    return false;
                    break;
                default:
                    break;
                }
            }// else if (event.type == SDL_MOUSEBUTTONDOWN) {
            //    game->clickInput(event);
            //}

            if (event.type == SDL_QUIT) {
                game->send("DISCONNECT");
                SDL_Delay(100);
                return false;
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render(renderer, game);

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            //cout << frameTime << endl;
            SDL_Delay(frameDelay - frameTime);
        }
	}
    //game->getGameData().setReady(false); // I guess setting it here doesn't work // At least this way of doing it seems to be read-only // I forgot to make getGameData return a pointer, that's why it was read only because changes to that instance didn't go anywhere
    game->getGameData()->setReady(false);
    for (int id = 1; id <= game->MAX_PLAYERS; id++) {
        //PlayerData* data = game->getGameData().playerMap[id];
        PlayerData* data = game->getGameData()->playerMap[id];
        if (data == nullptr) break;
        data->isReady = false; // TODO - Don't forget to do this serverside too if I really want to make the game loop without closing
    }
	return true;
}

void GameEndScreen::render(SDL_Renderer* renderer, MyGame* game) {
    // Simple text prompt displaying game data
    SDL_Rect textRect = { 0, 40, 0, 0 };
    std::string text = "GAME OVER";
    SDL_Surface* textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = 400 - textRect.w / 2;
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    textRect = { 0, 80, 0, 0 };
    if (game->getGameData()->partyWon) {
        text = "Your party defeated the boss in battle!";
    } else {
        text = "Your party was defeated by the boss!";
    }

    textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = 400 - textRect.w / 2;
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    textRect = { 0, 350, 0, 0 };
    SDL_Rect srcRect = { 0, 0, 20, 20 };
    for (int id = 1; id <= game->MAX_PLAYERS; id++) {
        SDL_Rect dstRect = { 210 + (id - 1) * 100, 260, 80, 80 };
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
        SDL_RenderFillRect(renderer, &dstRect);

        PlayerGameData* data = &(game->getGameData()->endData[id-1]);

        if (!data->slotActive) continue;
        if (data->health == 0) {
            dstRect.x += 2;
            dstRect.y += 10;
            if (data->sprite != nullptr) {
                SDL_RenderCopyEx(renderer, data->sprite, &srcRect, &dstRect, 90, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
            }
            
            text = "DEAD";
            textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
            textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
            textRect.x = 250 + (id - 1) * 100 - textRect.w / 2;
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
        } else {
            if (data->sprite != nullptr) {
                SDL_RenderCopy(renderer, data->sprite, &srcRect, &dstRect);
            }
            text = std::to_string(data->health) + "/" + std::to_string(data->maxHealth) + "HP";
            textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
            textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
            textRect.x = 250 + (id - 1) * 100 - textRect.w / 2;
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
        }
    }
    float healthRank = 0; // % Overall health of the party
    if (game->getGameData()->partyWon) {
        textRect = { 0, 400, 0, 0 };
        int totalMaxHealth = 0;
        int totalHealth = 0;
        for (int id = 0; id < game->MAX_PLAYERS; id++) {
            if (game->getGameData()->endData[id].slotActive) {
                totalHealth += game->getGameData()->endData[id].health;
                totalMaxHealth += game->getGameData()->endData[id].maxHealth;
            }
        }
        healthRank = (static_cast<float>(totalHealth) / static_cast<float>(totalMaxHealth)) * 100;
        text = "Party Health: " + std::to_string(totalHealth) + "/" + std::to_string(totalMaxHealth) + "HP";
        textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
        textRect.x = 400 - textRect.w / 2;
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
        textRect = { 0, 430, 0, 0 };
    } else {
        textRect = { 0, 400, 0, 0 };
    }
    int minutes = game->getGameData()->roundDuration / 60;
    int seconds = game->getGameData()->roundDuration % 60;
    std::string tMinutes;
    std::string tSeconds;
    if (minutes < 10) {
        tMinutes = "0" + std::to_string(minutes);
    } else {
        tMinutes = std::to_string(minutes);
    }
    if (seconds < 10) {
        tSeconds = "0" + std::to_string(seconds);
    }
    else {
        tSeconds = std::to_string(seconds);
    }
    if (game->getGameData()->partyWon) {
        text = "Time: " + tMinutes + ":" + tSeconds;
    }
    else {
        text = "Time Survived: " + tMinutes + ":" + tSeconds;
    }
    textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = 400 - textRect.w / 2;
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    if (game->getGameData()->partyWon) {
        textRect = { 0, 480, 0, 0 };
        text = "Health Rank: ";
        if (healthRank >= 90) {
            text += "S";
        } else if (healthRank >= 70) {
            text += "A";
        } else if (healthRank >= 50) {
            text += "B";
        } else if (healthRank >= 30) {
            text += "C";
        } else if (healthRank >= 15) {
            text += "D";
        } else {
            text += "F";
        }
        textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
        textRect.x = 400 - textRect.w / 2;
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        textRect = { 0, 510, 0, 0 };
        text = "Time Rank: ";
        if (game->getGameData()->roundDuration <= 60) {
            text += "S";
        } else if (game->getGameData()->roundDuration <= 75) {
            text += "A";
        } else if (game->getGameData()->roundDuration <= 90) {
            text += "B";
        } else if (game->getGameData()->roundDuration <= 120) {
            text += "C";
        } else if (game->getGameData()->roundDuration <= 150) {
            text += "D";
        } else {
            text += "F";
        }
        textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
        textRect.x = 400 - textRect.w / 2;
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
    textRect = { 0, 560, 0, 0 };
    text = "Press any button to continue";
    textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = 400 - textRect.w / 2;
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

bool GameEndScreen::loop(SDL_Renderer* renderer, MyGame* game) {
    SDL_Event event;

    const int frameDelay = 1000 / 60;
    int frameStart, frameTime;
    int closeDelay = SDL_GetTicks() + 1500;
    while (true) {//while (is_running) {
        if (game->shouldForceQuit()) return 0;
        //std::cout << is_running << std::endl;
        frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&event)) {
            //if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat == 0) {
            if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    game->send("DISCONNECT");
                    SDL_Delay(100);
                    return false;
                    break;
                default:
                    if (SDL_GetTicks() > closeDelay) { // new - 1.5s delay before you can close the end screen
                        return true;
                    }
                    break;
                }
            }

            if (event.type == SDL_QUIT) {
                game->send("DISCONNECT");
                SDL_Delay(100);
                return false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render(renderer, game);

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            //cout << frameTime << endl;
            SDL_Delay(frameDelay - frameTime);
        }
    }
	return true;
}

//static bool gameLobby(MyGame* game) {}

//static bool gameEndScreen(MyGame* game) {}