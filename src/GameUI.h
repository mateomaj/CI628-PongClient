#include "SDL.h"
#include "SDL_ttf.h"
#include "MyGame.h"

class GameLobby {
	private:
		void initRender(SDL_Renderer* renderer, MyGame* game);
		void render(SDL_Renderer* renderer, MyGame* game);
	public:
		bool loop(SDL_Renderer* renderer, MyGame* game);
};

class GameEndScreen {
	private:
		void render(SDL_Renderer* renderer, MyGame* game);
	public:
		bool loop(SDL_Renderer* renderer, MyGame* game);
};