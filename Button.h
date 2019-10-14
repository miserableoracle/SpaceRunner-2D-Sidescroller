/***********************************
Filename: Button.h
Author: Prof. Alex Richard
Modified By: Hiren Pandya (101198481)
Modified On: 14.04.2019
************************************/
#pragma once
#include "SDL.h"

class Button
{
private:
	enum state { MOUSE_UP, MOUSE_OVER, MOUSE_DOWN };
	bool m_bReleased;
	int m_iFrame;
	SDL_Rect m_rSrc;
	SDL_Rect m_rDst;
	SDL_Surface* m_pSurf;
	SDL_Texture* m_pText;

public:
	Button(const char*, SDL_Rect, SDL_Rect);
	~Button();
	void Update();
	void Render();
	bool Clicked() { return m_iFrame == 2; }
};