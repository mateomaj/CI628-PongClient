#include "MyGame.h"

void MyGame::on_receive(std::string cmd, std::vector<std::string>& args) {
    //std::cout << cmd << std::endl;
    if (cmd == "GAME_DATA") {
        // we should have exactly 4 arguments
        if (args.size() == 4) {
            game_data.player1Y = stoi(args.at(0));
            game_data.player2Y = stoi(args.at(1));
            game_data.ballX = stoi(args.at(2));
            game_data.ballY = stoi(args.at(3));
        }
    } else {
        std::cout << "Received: " << cmd << std::endl;
    }
}

void MyGame::send(std::string message) {
    messages.push_back(message);
}

void MyGame::input(SDL_Event& event) {
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
}

void MyGame::update() {
    player1.y = game_data.player1Y;
    player2.y = game_data.player2Y; // New - Player 2 handling
    ball.x = game_data.ballX; // New - Ball handling
    ball.y = game_data.ballY;
}

void MyGame::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &player1);
    SDL_RenderDrawRect(renderer, &player2); // New - Player 2 render
    SDL_RenderFillRect(renderer, &ball);//SDL_RenderDrawRect(renderer, &ball); // New - Render ball
}