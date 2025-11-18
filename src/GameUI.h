#include "SDL.h"
#include "SDL_ttf.h"
#include "MyGame.h"

class GameLobby {
	private:
		void render(SDL_Renderer* renderer, MyGame* game);
	public:
		bool loop(SDL_Renderer* renderer, MyGame* game);
		//GameLobby();
};

class GameEndScreen {
	private:
		void render(SDL_Renderer* renderer, MyGame* game);
	public:
		bool loop(SDL_Renderer* renderer, MyGame* game);
		//GameEndScreen();
};

//static bool gameLobby(MyGame* game);
//static bool gameEndScreen(MyGame* game);