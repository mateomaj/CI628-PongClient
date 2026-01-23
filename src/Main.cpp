#include "SDL_net.h"

#include "MyGame.h"
#include "GameUI.h"

using namespace std;

const char* IP_NAME = "localhost";
const Uint16 PORT = 55555;
const int FRAME_RATE_MULTIPLIER = 2; // Multiplies tpf to account for update problems caused by refresh rate

bool is_running = true;

MyGame* game = new MyGame();

int lastReceivedTime; // Tracks when was the last time an update message was received from the server. Also used to track time for simulations.

// Cleanup note - bufferedUpdateMessage // "Stores the most recent update message so it can be handled later in onUpdate(), resets every frame and allows only the most recent update message to be used for the current frame, ignoring any extra ones sent before it. Used as a reference for deltatime only within this frame, everything else is left to simulation if not updated" // It stores update messages until they are used in onUpdate(), if multiple are received before onUpdate() is called, the newer one is used instead. It stops the game from forcing itself to process every update message when some of them become irrelevant really quickly; entities getting updated twice when only the second update is shown wastes processing time. // Player update data is also the only thing consistantly sent on every server tick, letting it get handled on the main thread saves time for other messages that might show up in that time.
char bufferedUpdateMessage[1024]; // bufferedUpdateMessage in array form // Size is the same as message_length in on_receive() for consistancy

// Method taken from https://www.geeksforgeeks.org/cpp/different-ways-to-copy-a-string-in-c-c/ to help with changing buffered message from char* to char[]
void copyString(char* t, char* s)
{
    // (return ASCII value which is True,
    // therefore will be in the loop
    // till the condition is False
    while (*t++ = *s++)
        ;
}

static int on_receive(void* socket_ptr) {
    TCPsocket socket = (TCPsocket)socket_ptr;

    const int message_length = 1024;

    char message[message_length];
    int received;

    // New - Nested strtok to deal with combined messages
    // Quickly doing multiple broadcast calls on server makes the client recieve two sets of data as one message. This fix uses a semi-colon to separate messages in case they get combined so the data can be fully processed.
    // Made with the help of: https://www.geeksforgeeks.org/cpp/strtok-strtok_r-functions-c-examples/ - Showing how to do nested strtok
    do {
        received = SDLNet_TCP_Recv(socket, message, message_length);
        message[received] = '\0';
        
        // The things we have to do without string.split()

        string cmd = "";
        
        char* outer_saveptr = NULL;
        char* inner_saveptr = NULL;
        
        char* token = strtok_s(message, ";", &outer_saveptr);

        while (token != NULL) {
            char* pch = strtok_s(token, ",", &inner_saveptr);

            // Get the command, which is the first string in the message
            cmd = string(pch);

            // Store updates for later use 
            if (cmd == "UPD") {
                lastReceivedTime = SDL_GetTicks();
                copyString(bufferedUpdateMessage, inner_saveptr);
                // The message doesn't need to be processed after getting buffered, move onto the next message
                token = strtok_s(NULL, ";", &outer_saveptr);
                continue; 
            }

            // Then get the arguments to the command
            vector<string> args;

            if (cmd == "PUPD") { // Projectile update alternate data format
                while (pch != NULL) {
                    pch = strtok_s(NULL, ":", &inner_saveptr);
                    if (pch != NULL) {
                        args.push_back(string(pch));
                    }
                }
            } else {
                while (pch != NULL) {
                    pch = strtok_s(NULL, ",", &inner_saveptr);
                    if (pch != NULL) {
                        args.push_back(string(pch));
                    }
                }
            }
            
            game->on_receive(cmd, args);
            
            if (cmd == "EXIT") {
                break;
            }

            token = strtok_s(NULL, ";", &outer_saveptr);
        }

        if (cmd == "EXIT") {
            is_running = false;
            game->forceQuit();
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
    //const int frameDelay = 1000 / 60 * FRAME_RATE_MULTIPLIER;

    int frameStart, frameTime, lastUpdateTime; // LastUpdateTime - time since the last update was called

    // "Program" loop - Cycling through: lobby > game > game over > lobby ...
    do {

        GameLobby* lobby = new GameLobby();

        // Enter the game lobby state, waiting for other players to join and start the game.
        // If the whole party readies up, the game starts. // If the player tries to exit the game, return here and let it close properly.
        game->getGameData()->setRunning(is_running = lobby->loop(renderer, game));
        delete lobby;

        lastUpdateTime = lastReceivedTime = SDL_GetTicks(); // Inital time for simulation // Just in case we would need it

        // Main game loop
        while (is_running && game->getGameData()->isRunning()) {
            frameStart = SDL_GetTicks();
            
            // input
            while (SDL_PollEvent(&event)) {
                if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat == 0) {
                    game->input(event);

                    switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        game->send("DISCONNECT");
                        SDL_Delay(100); // Add a slight delay so the server can receive the message
                        is_running = false;
                        break;
                    default:
                        break;
                    }
                }

                if (event.type == SDL_QUIT) {
                    game->send("DISCONNECT");
                    SDL_Delay(100); // Add a slight delay so the server can receive the message
                    is_running = false;
                }
            }

            // Since the background is drawn every frame and setting draw colour does nothing here, I could remove these lines
            //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer); // RenderClear should stay in case the background doesn't load

            // Run main update
            int updateTime = SDL_GetTicks();
            game->update(((updateTime - lastUpdateTime) / 1000.0) * FRAME_RATE_MULTIPLIER);
            lastUpdateTime = updateTime;
            int simTime = SDL_GetTicks();

            // Run sim update
            // Pass the buffered update message through if one was buffered this frame.
            if (bufferedUpdateMessage[0] != '\0') {
                char tempMessage[1024];
                copyString(tempMessage, bufferedUpdateMessage); // TODO - There is a very tiny chance that this could still get overwritten into a messy corrupted message if the thread and this run at the same time with UPD message[] having different contents // Until that happens, I won't think about it more than I already did.
                
                bufferedUpdateMessage[0] = '\0';
                
                // I know why updateSimulated() doesn't run as smoothly as update() - simulated time is completely different from update time and should be handled differently
                // simTime is based on the last time an update message was received or the last time the function was ran if there is no update message
                // Since simTime is meant to be relative to the update message, we can't apply the framerate multiplier onto it because that's not affected by refresh rate
                // ^ We still need to do it when simulating without an update message because that runs with the exact same context as normal update()
                game->updateSimulated(((simTime - lastReceivedTime) / 1000.0), tempMessage);
            } else {
                game->updateSimulated(((simTime - lastReceivedTime) / 1000.0) * FRAME_RATE_MULTIPLIER); // Since the server is running at ~2x speed, update sim at 2x tpf
            }
            lastReceivedTime = simTime;

            // Render the game and show the screen
            game->render(renderer);

            SDL_RenderPresent(renderer);

            // Delay until the next frame to keep a stable 60fps
            frameTime = SDL_GetTicks() - frameStart;
            if (frameDelay > frameTime) {
                SDL_Delay(frameDelay - frameTime);
            }
        }

        // Once the game loop ends, enter the game over screen.
        // Unless pressing quit, pressing any button will loop the game back to the lobby screen.
        if (is_running && !game->getGameData()->isRunning()) {
            GameEndScreen* endScreen = new GameEndScreen();
            is_running = endScreen->loop(renderer, game);
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

    game->initTextures(renderer); // Run initTextures() before starting the game loop

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
    SDL_Delay(5000); // 5sec delay because closing the game before the socket

    // Shutdown SDL_net
    SDLNet_Quit();

    // Shutdown SDL
    SDL_Quit();

    return 0;
}