/***********************************
Filename: Main.cpp
Author: Prof. Alex Richard
Modified By: Hiren Pandya (101198481)
Modified On: 14.04.2019
************************************/
#include "Game.h"

int main(int argc, char* args[])
{
	if (Game::Instance()->Init("SDL Space Runner", SDL_WINDOWPOS_CENTERED,
		                       SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_RESIZABLE) == false) return 1;
	while (Game::Instance()->Running())
	{
		Game::Instance()->HandleEvents();
		if (Game::Instance()->Tick())
		{
			Game::Instance()->Update();
			Game::Instance()->Render();
		}
	}
	Game::Instance()->Clean();
	return 0;
}