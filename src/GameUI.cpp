#include "GameUI.h"

void GameLobby::initRender(SDL_Renderer* renderer, MyGame* game) {
    SDL_Rect textRect = { 0, 40, 0, 0 };
    std::string text = "TINY WARRIORS";
    SDL_Surface* textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = 400 - textRect.w / 2;
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    textRect = { 0, 120, 0, 0 };
    text = "GAME LOBBY";
    textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = 400 - textRect.w / 2;
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    textRect = { 40, 400, 0, 0 };
    text = "CONTROLS:";
    textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    textRect = { 40, 420, 0, 0 };
    text = "ENTER - Ready Up";
    textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    textRect = { 40, 460, 0, 0 };
    text = "Class Select:";
    textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    textRect = { 40, 480, 0, 0 };
    text = "1 - Knight";
    textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    textRect = { 40, 500, 0, 0 };
    text = "2 - Ranger";
    textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    textRect = { 40, 520, 0, 0 };
    text = "3 - Mage";
    textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    textRect = { 0, 560, 0, 0 };
    text = "THE GAME BEGINS WHEN EVERYONE IS READY";
    textSurface = TTF_RenderText_Blended(game->getGameData()->font, text.c_str(), game->getGameData()->defaultFontColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = 400 - textRect.w / 2;
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void GameLobby::render(SDL_Renderer* renderer, MyGame* game) {
    SDL_Rect srcRect = { 0, 0, 20, 20 };
    for (int id = 1; id <= game->MAX_PLAYERS; id++) {
        SDL_Rect dstRect = { 210 + (id - 1) * 100, 260, 80, 80 };
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
        SDL_RenderFillRect(renderer, &dstRect);

        PlayerData* data = game->getGameData()->playerMap[id]; // Cool Note - Every time we try to read a slot within the list using array index, that checked slot gets counted when calling playerMap.size()
        //PlayerData* data = game->getGameData()->playerMap.at(id); // ^ Trying playerMap.at() throws an out of bounds error instead of returning null

        if (data == nullptr) continue;

        data->checkTexture();
        if (data->spriteTexture != nullptr) {
            SDL_RenderCopy(renderer, data->spriteTexture, &srcRect, &dstRect);
        }

        if (data->isReady) {
            SDL_SetRenderDrawColor(renderer, 120, 250, 120, 255);
            SDL_RenderDrawRect(renderer, &dstRect);
        }
    }
} // TODO - Add text into UI showing player names and controls

bool GameLobby::loop(SDL_Renderer* renderer, MyGame* game) {
	SDL_Event event;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    initRender(renderer, game);

	const int frameDelay = 1000 / 60;
	int frameStart, frameTime;

    // Stay in the lobby until the server says to switch
	while (!game->getGameData()->isReady()) {
        if (game->shouldForceQuit()) return 0;
		frameStart = SDL_GetTicks();

        // Let players choose their class with 1, 2, or 3 and ready up with the enter key
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                switch (event.key.keysym.sym) {
                case SDLK_1:
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
                    if (game->myPlayer->playerClass != PlayerClasses::NONE) {
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
            }

            if (event.type == SDL_QUIT) {
                game->send("DISCONNECT");
                SDL_Delay(100);
                return false;
            }
        }

        render(renderer, game);

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
	}

    game->getGameData()->setReady(false);
    for (int id = 1; id <= game->MAX_PLAYERS; id++) {
        PlayerData* data = game->getGameData()->playerMap[id];
        if (data == nullptr) break;
        data->isReady = false;
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
    // Display players and their state
    for (int id = 1; id <= game->MAX_PLAYERS; id++) {
        SDL_Rect dstRect = { 210 + (id - 1) * 100, 260, 80, 80 };
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
        SDL_RenderFillRect(renderer, &dstRect);

        PlayerGameData* data = &(game->getGameData()->endData[id-1]);

        if (!data->slotActive) continue;

        // Dead players are shown laying on the floor with a "DEAD" status
        // Living players are shown standing up with their health values displayed
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
    // If the party wins, health is totaled up between all players and the % of health remaining is used as a rank
    // If they all died then there's no point in showing a health rank
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

    // Converting time into a printable format
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

    // If the party wins, their time taken to win is used to calculate a rank based on how quickly they defeated the boss.
    // If the party lost, the time shows how long they survived.
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

    // Rank conversions and display
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

    // Text telling the player how to close the menu
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

    // Because nothing can ever change on this screen, technically it only needs to be rendered once
    // And that's actually a really cool optimisation so I'm doing it
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    render(renderer, game);
    SDL_RenderPresent(renderer);

    const int frameDelay = 1000 / 60;
    int frameStart, frameTime;
    int closeDelay = SDL_GetTicks() + 1500;

    // Loop the screen until the player presses a button
    while (true) {
        if (game->shouldForceQuit()) return 0;
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    game->send("DISCONNECT");
                    SDL_Delay(100);
                    return false;
                    break;
                default:
                    if (SDL_GetTicks() > closeDelay) { // 1.5s delay before you can close the end screen
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

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }
	return true;
}