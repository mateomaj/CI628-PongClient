#include "SDL_net.h"

#include "MyGame.h"
#include "GameUI.h"

using namespace std;

const char* IP_NAME = "localhost";
const Uint16 PORT = 55555;

bool is_running = true;

MyGame* game = new MyGame();

//int tpf; // time passed this frame, used in simulation

//int lastReceivedTime; // Tracks when was the last time a message was received from the server // last-now > 17 means that the server hasn't updated anything this frame so the client should simulate it
//bool confirmReceive = false; // ^^^ Alt solution - checking time can be inconsistent so this flag will be true when a message is received and reset at the end of each frame
int lastReceivedTime; // ^^^ Alt alt solution - last message received time can be used as deltatime for simulation, overriden in loop after update // 

static int on_receive(void* socket_ptr) {
    TCPsocket socket = (TCPsocket)socket_ptr;

    const int message_length = 1024;

    char message[message_length];
    int received;

    //cout << received << endl;
    // New - Nested strtok to deal with combined messages
    // Quickly doing multiple broadcast calls on server makes the client recieve two sets of data as one message. This fix uses a semi-colon to separate messages in case they get combined so the data can be fully processed.
    // Made with the help of: https://www.geeksforgeeks.org/cpp/strtok-strtok_r-functions-c-examples/
    // TODO: while(), rather than do
    do {
        received = SDLNet_TCP_Recv(socket, message, message_length);
        message[received] = '\0';
        //cout << message << endl;
        // The things we have to do without string.split() 

        //printf(message);
        //cout << endl;

        string cmd = "";

        char* outer_saveptr = NULL;
        char* inner_saveptr = NULL;

        char* token = strtok_s(message, ";", &outer_saveptr);

        while (token != NULL) {
            //char* pch = strtok(message, ",");
            char* pch = strtok_s(token, ",", &inner_saveptr);

            // get the command, which is the first string in the message
            cmd = string(pch);
            //string cmd(pch);

            // then get the arguments to the command
            vector<string> args;

            while (pch != NULL) {
                //pch = strtok(NULL, ",");
                pch = strtok_s(NULL, ",", &inner_saveptr);

                if (pch != NULL) {
                    args.push_back(string(pch));
                }
            }

            if (cmd == "UPD") { // Tells this client this message lead to a game update // It should be placed towards the end of the message so the actual time is closer to the end of the updates
                //int boogus = (lastReceivedTime - SDL_GetTicks());
                //cout << boogus << endl;
                //cout << (SDL_GetTicks() - lastReceivedTime) << endl;
                lastReceivedTime = SDL_GetTicks();
            } else {
                game->on_receive(cmd, args);
            }

            if (cmd == "EXIT") {
                break;
            }

            token = strtok_s(NULL, ";", &outer_saveptr);
        }

        //confirmReceive = true;

        if (cmd == "EXIT") {
            break;
        }

    } while (received > 0 && is_running);

    return 0;
}

static int on_send(void* socket_ptr) {
    TCPsocket socket = (TCPsocket)socket_ptr;

    while (is_running) {
        if (game->messages.size() > 0) {
            string message = "CLIENT_DATA";

            for (auto m : game->messages) {
                message += "," + m;
            }

            game->messages.clear();

            //cout << "Sending_TCP: " << message << endl;

            SDLNet_TCP_Send(socket, message.c_str(), message.length());
        }

        SDL_Delay(1);
    }

    return 0;
}

void loop(SDL_Renderer* renderer) {
    SDL_Event event;

    const int frameDelay = 1000 / 60;

    int frameStart, frameTime;

    do {

        GameLobby* lobby = new GameLobby();
        //is_running = lobby->loop(renderer, game);
        //game->getGameData().setRunning(is_running = lobby->loop(renderer, game));
        game->getGameData()->setRunning(is_running = lobby->loop(renderer, game));
        delete lobby;

        lastReceivedTime = SDL_GetTicks(); // Inital time for simulation // Just in case we would need it

        //while (is_running && game->getGameData().isRunning()) {
        while (is_running && game->getGameData()->isRunning()) {
            frameStart = SDL_GetTicks();
            //tpf = SDL_GetTicks();
            // input
            while (SDL_PollEvent(&event)) {
                if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat == 0) {
                    game->input(event);

                    switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        game->send("DISCONNECT");
                        SDL_Delay(100);
                        is_running = false;
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
                    is_running = false;
                }
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            game->update();
            int simTime = SDL_GetTicks();
            game->updateSimulated((simTime - lastReceivedTime) / 500.0); // Since the server is running at ~2x speed, update sim at 2x tpf
            lastReceivedTime = simTime;
            //game->updateSimulated(-(lastReceivedTime - (lastReceivedTime = SDL_GetTicks())) / 1000.0);
            //game->updateSimulated((SDL_GetTicks() - lastReceivedTime)/1000.0);

            //lastReceivedTime = SDL_GetTicks();

            game->render(renderer);

            //confirmReceive = false;

            SDL_RenderPresent(renderer);

            frameTime = SDL_GetTicks() - frameStart;
            if (frameDelay > frameTime) {
                //cout << frameTime << endl;
                SDL_Delay(frameDelay - frameTime);
            }

            //SDL_Delay(17);
        }

        //if (is_running && !game->getGameData().isRunning()) {
        if (is_running && !game->getGameData()->isRunning()) {
            GameEndScreen* endScreen = new GameEndScreen();
            cout << "Entering End Screen\n";
            is_running = endScreen->loop(renderer, game);
            cout << "Exiting End Screen with code: " << is_running << endl;
            delete endScreen;
        }

    } while (is_running);
}

int run_game() {
    SDL_Window* window = SDL_CreateWindow(
        "Multiplayer Pong Client",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );

    if (nullptr == window) {
        std::cout << "Failed to create window" << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (nullptr == renderer) {
        std::cout << "Failed to create renderer" << SDL_GetError() << std::endl;
        return -1;
    }

    loop(renderer);

    return 0;
}

int main(int argc, char** argv) {

    // Initialize SDL
    if (SDL_Init(0) == -1) {
        printf("SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    // Initialize SDL_net
    if (SDLNet_Init() == -1) {
        printf("SDLNet_Init: %s\n", SDLNet_GetError());
        exit(2);
    }

    IPaddress ip;

    // Resolve host (ip name + port) into an IPaddress type
    if (SDLNet_ResolveHost(&ip, IP_NAME, PORT) == -1) {
        printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        exit(3);
    }
    //cout << ip.host << endl;
    //cout << ip.port << endl;
    // Open the connection to the server
    TCPsocket socket = SDLNet_TCP_Open(&ip);

    if (!socket) {
        printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        exit(4);
    }

    SDL_CreateThread(on_receive, "ConnectionReceiveThread", (void*)socket);
    SDL_CreateThread(on_send, "ConnectionSendThread", (void*)socket);

    run_game();

    delete game;

    // Close connection to the server
    SDLNet_TCP_Close(socket);
    SDL_Delay(5000);
    // Shutdown SDL_net
    SDLNet_Quit();
    //printf("EXITING THE SYSTEMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
    // Shutdown SDL
    SDL_Quit();

    return 0;
}