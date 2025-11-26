#include "GameUI.h"
//#include "Main.cpp"

void GameLobby::render(SDL_Renderer* renderer, MyGame* game) {
    SDL_Rect srcRect = { 0, 0, 20, 20 };
    for (int id = 1; id <= game->MAX_PLAYERS; id++) {
        SDL_Rect dstRect = { 210 + (id - 1) * 100, 260, 80, 80 };
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
        SDL_RenderFillRect(renderer, &dstRect);

        //PlayerData* data = game->getGameData().playerMap[id];
        PlayerData* data = game->getGameData()->playerMap[id];
        //std::cout << "nullptr? - " << id << std::endl;
        if (data == nullptr) continue;
        //std::cout << "not nullptr\n";

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
}

bool GameEndScreen::loop(SDL_Renderer* renderer, MyGame* game) {
    SDL_Event event;

    const int frameDelay = 1000 / 60;
    int frameStart, frameTime;
    while (true) {//while (is_running) {
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
                    return true;
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