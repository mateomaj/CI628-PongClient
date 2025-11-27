#include "SDL_net.h"

#include "MyGame.h"
#include "GameUI.h"

using namespace std;

const char* IP_NAME = "localhost";
const Uint16 PORT = 55555;
const int FRAME_RATE_MULTIPLIER = 2;

bool is_running = true;

MyGame* game = new MyGame();

//int tpf; // time passed this frame, used in simulation

//int lastReceivedTime; // Tracks when was the last time a message was received from the server // last-now > 17 means that the server hasn't updated anything this frame so the client should simulate it
//bool confirmReceive = false; // ^^^ Alt solution - checking time can be inconsistent so this flag will be true when a message is received and reset at the end of each frame
int lastReceivedTime; // ^^^ Alt alt solution - last message received time can be used as deltatime for simulation, overriden in loop after update // 
//char* bufferedUpdateMessage = nullptr; // Stores the most recent update message so it can be handled later in onUpdate(), resets every frame and allows only the most recent update message to be used for the current frame, ignoring any extra ones sent before it. Used as a reference for deltatime only within this frame, everything else is left to simulation if not updated
//bool settingBufferedUpdateMessage = false; // flag to stop quantum entanglement concurency issues
//bool copiedUpdateMessage = false;
//string bufferedUpdateMessage = "";
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

    //cout << received << endl;
    // New - Nested strtok to deal with combined messages
    // Quickly doing multiple broadcast calls on server makes the client recieve two sets of data as one message. This fix uses a semi-colon to separate messages in case they get combined so the data can be fully processed.
    // Made with the help of: https://www.geeksforgeeks.org/cpp/strtok-strtok_r-functions-c-examples/
    // TODO: while(), rather than do
    do {
        //cout << "8 - " << bufferedUpdateMessage << endl;
        received = SDLNet_TCP_Recv(socket, message, message_length);
        //cout << "9 - " << bufferedUpdateMessage << endl;
        //cout << "A - " << bufferedUpdateMessage << endl;
        message[received] = '\0';
        //cout << "B - " << bufferedUpdateMessage << endl;
        //cout << message << endl;
        // The things we have to do without string.split() 

        //printf(message);
        //cout << endl;

        string cmd = "";
        //cout << "< - " << bufferedUpdateMessage << endl;
        char* outer_saveptr = NULL;
        char* inner_saveptr = NULL;
        //cout << "> - " << bufferedUpdateMessage << endl;
        char* token = strtok_s(message, ";", &outer_saveptr);
        //cout << "| - " << bufferedUpdateMessage << endl; // The message is broken between SDLNet_TCP_Recv() and the first call of strtok_s()... why??? Other than inner_saveptr being NULL, nothing interacts with it, and it goes back to normal without anything interacting with it // both SDLNet and strtok_s must somehow change what the thing points to by getting to its memory address or something. // It just doesn't make any sense, how does this thing display the entire message apart from the first command??? // Nothing in this entire code can show the message with that syntax, outer pointer tracks progress of the entire message and doesn't care about commas, and inner pointer cares about the commas but can only read within semi-colons. The bugged-quantum-whatever printout doesn't have the command AND contains the rest of the message including semi-colons. // We know changing inner_saveptr does nothing to change bufferedUpdateMessage because I already tested it, completely changing the value of it does nothing // the only thing that would make sense to me is if strtok_s() changes the end point for all its pointers in memory to the end of the message, regardless of where they start from. OR bufferedUpdateMessage points to a segment of message that is then uncapped when the message resets, but that's some absolute bullshit if it works like that because it's supposed to point to its own unique string, not some random segment
        // Here's what I know from the latest test // 1 - bufferedUpdateMessage receives the correct string between "UPD," and ";" // 2 - Past SDLNet_TCP_Recv(), the buffered message is overwritten to the contents of the newly received message. If the new message is shorter than the last, remnants of the previous message are shown up to the next ";" after the end of the new message // 3 - Past message[received] = '\0'; the excess of the message is removed up to... // 4 - Past strtok_s(~, ~, &outer_saveptr) the message returns to "normal" (more tests needed) // Update: 4 - Past strtok_s(~, ~, &outer_saveptr) the message points up to the first ";"
        // Too many tests were done but I now fully know what is going on here. Everything here effectively points to a segment of message[], with both a start and end point, resetting the contents of the message also effects the value of anything that points to a string (char*) that came from the message, the start and end points are unchanged until the message changes or '\0' is added to the message // That last bit is badly explained. // Point is, bufferedUpdateMessage resets to display the entire message past its start point once the message changes. // The only way to fix it is to make the contents of bufferedUpdateMessage a new char[] or char* independant of the contents of message[]
        // Every time I try to add a SIMPLE thing, I run into the most obscure and random issue that forces me to spend days of testing to understand it and fix it for good. // I get this is meant to be a learning experience, and I am learning thanks to all of this, but this is torture.
        
        //cout << message << endl;

        while (token != NULL) {
            //char* pch = strtok(message, ",");
            char* pch = strtok_s(token, ",", &inner_saveptr);
            //cout << token << endl;
            //cout << inner_saveptr << endl;

            // get the command, which is the first string in the message
            cmd = string(pch);
            //string cmd(pch);

            // New store updates for later use 
            if (cmd == "UPD") {
                //if (settingBufferedUpdateMessage) {
                //    cout << "fuck off";
                //}
                //while (settingBufferedUpdateMessage) { cout << "fuck off\n"; };
                //cout << "# - " << bufferedUpdateMessage << endl;
                //bufferedUpdateMessage = inner_saveptr;
                //free(bufferedUpdateMessage);
                //bufferedUpdateMessage = strdup(inner_saveptr); // https://stackoverflow.com/questions/481673/make-a-copy-of-a-char // FREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEDOM // Ok it works but also causes a memory leak // I have to run free(bufferedUpdateMessage); somewhere as the duplicated string is not cleared automatically
                //if (!copiedUpdateMessage) {
                
                //if (bufferedUpdateMessage != nullptr) {
                    //free(bufferedUpdateMessage);
                    //bufferedUpdateMessage = nullptr; // The memory leak should be fixed though the program did crash once. Adding this line made it slightly less likey to happen but the concurrency issue still exists
                //}
                //bufferedUpdateMessage = strdup(inner_saveptr);
                lastReceivedTime = SDL_GetTicks();
                copyString(bufferedUpdateMessage, inner_saveptr);
                //free(bufferedUpdateMessage);
                //cout << "1 - " << bufferedUpdateMessage << endl;
                //inner_saveptr = "fuck";
                //cout << "2 - " << bufferedUpdateMessage << endl;
                //bufferedUpdateMessage = string(inner_saveptr);
                //lastReceivedTime = SDL_GetTicks();
                //cout << "New Message: " << bufferedUpdateMessage << "\n";
                token = strtok_s(NULL, ";", &outer_saveptr);
                continue;
            }

            // then get the arguments to the command
            vector<string> args;

            while (pch != NULL) {
                //pch = strtok(NULL, ",");
                pch = strtok_s(NULL, ",", &inner_saveptr);
                //cout << "3 - " << bufferedUpdateMessage << endl;
                if (pch != NULL) {
                    args.push_back(string(pch));
                }
            }
            //cout << "4 - " << bufferedUpdateMessage << endl;
            //if (cmd == "UPD") { // Tells this client this message lead to a game update // It should be placed towards the end of the message so the actual time is closer to the end of the updates
                //int boogus = (lastReceivedTime - SDL_GetTicks());
                //cout << boogus << endl;
                //cout << (SDL_GetTicks() - lastReceivedTime) << endl;
            //    lastReceivedTime = SDL_GetTicks();
            //} else {
                game->on_receive(cmd, args);
            //}
                //cout << "5 - " << bufferedUpdateMessage << endl;
            if (cmd == "EXIT") {
                break;
            }

            token = strtok_s(NULL, ";", &outer_saveptr);
            //cout << "6 - " << bufferedUpdateMessage << endl;
        }

        //cout << "7 - " << bufferedUpdateMessage << endl;
        //confirmReceive = true;

        if (cmd == "EXIT") {
            is_running = false;
            game->forceQuit();
            //SDL_Event* quit = (SDL_Event*) new SDL_QuitEvent();
            //SDL_PushEvent(quit);
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

    int frameStart, frameTime, lastUpdateTime; // LastUpdateTime - time since the last update was called

    do {

        GameLobby* lobby = new GameLobby();
        //is_running = lobby->loop(renderer, game);
        //game->getGameData().setRunning(is_running = lobby->loop(renderer, game));
        game->getGameData()->setRunning(is_running = lobby->loop(renderer, game));
        delete lobby;

        //lastReceivedTime = SDL_GetTicks(); // Inital time for simulation // Just in case we would need it
        lastUpdateTime = lastReceivedTime = SDL_GetTicks(); // Inital time for simulation // Just in case we would need it

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

            int updateTime = SDL_GetTicks();
            game->update(((updateTime - lastUpdateTime) / 1000.0) * FRAME_RATE_MULTIPLIER);
            lastUpdateTime = updateTime;
            int simTime = SDL_GetTicks();
            if (bufferedUpdateMessage[0] != '\0') {
            //if (bufferedUpdateMessage != nullptr) {
            //if (bufferedUpdateMessage != "") {
                //settingBufferedUpdateMessage = true;
                
                //char* tempMessage = bufferedUpdateMessage;
                char tempMessage[1024];
                copyString(tempMessage, bufferedUpdateMessage);
                //bufferedUpdateMessage = nullptr;
                bufferedUpdateMessage[0] = '\0';
                //cout << "SET MESSAGE: " << tempMessage << endl;
                //bufferedUpdateMessage[0] = '\0';
                //cout << "GET MESSAGE: " << tempMessage << endl;
                //bufferedUpdateMessage = nullptr;
                //settingBufferedUpdateMessage = false;
                //cout << "SET MESSAGE: " << tempMessage << endl;
                //cout << "SET MESSAGE: " << bufferedUpdateMessage << endl; // The value of bufferedUpdateMessage itself is changing after the initial printout
                game->updateSimulated(((simTime - lastReceivedTime) / 1000.0) * FRAME_RATE_MULTIPLIER, tempMessage);
                //free(tempMessage);
                //bufferedUpdateMessage[0] = '\0';
            } else {
                //cout << "not sim\n";
                game->updateSimulated(((simTime - lastReceivedTime) / 1000.0) * FRAME_RATE_MULTIPLIER); // Since the server is running at ~2x speed, update sim at 2x tpf
            }
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