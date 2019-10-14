/***********************************
Filename: FSM.h
Author: Prof. Alex Richard
Modified By: Hiren Pandya (101198481)
Modified On: 14.04.2019
************************************/
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "Button.h"
#include "Sprites.h"
#include "SDL_ttf.h"
using namespace std;

class State // This is the abstract base class for all specific states.
{
protected:
	TTF_Font* m_Font;
	SDL_Texture* m_pFontText;
	SDL_Rect m_rFontRect;

public:
	virtual void Update() = 0; // A 'pure virtual' method.
	virtual void Render();     
	void RenderFont(bool, const char*, int, int);
	virtual void Enter() = 0;
	virtual void Exit() = 0;
	virtual void Resume() = 0;
};

class PauseState : public State
{
private:
	vector<Button*> m_vButtons;
	enum btn { resume, exit };

public:
	PauseState() {}
	void Update(); // Method prototype.
	void Render();
	void Enter();
	void Exit();
	void Resume() {}
};

class GameState : public State
{
private: 
	Player* m_pPlayer;
	Obstacle* m_pGround;
	vector<Obstacle*> m_vObstacles;
	int m_iTime, m_iLastTime, m_iTimeCtr, m_iObsCtr, m_iObsMax;
	int m_iHighScore = 0;
	string m_sScore;
	string m_sTime;
	SDL_Texture* m_pBGTexture;	// Background texture
	SDL_Texture* m_pPTexture;	// Player texture
	SDL_Texture* m_pOTexture;	// Obstacle texture

	Background m_Backgrounds[2] = { Background({0,0,1024,768},{0,0,1024,768},1),
									Background({0,0,1024,768},{1024,0,1024,768},1) };
	Background m_Midgrounds[5] = { Background({1024,0,256,512},{0,0,256,512},3),
								   Background({1024,0,256,512},{256,0,256,512},3),
								   Background({1024,0,256,512},{512,0,256,512},3),
								   Background({1024,0,256,512},{768,0,256,512},3),
								   Background({1024,0,256,512},{1024,0,256,512},3) };
	Background m_Foregrounds[3] = { Background({1024,512,512,256},{0,512,512,256},4),
									Background({1024,512,512,256},{512,512,512,256},4),
									Background({1024,512,512,256},{1024,512,512,256},4) };

public:
	GameState() : m_iTime(0), m_iLastTime(-1),	m_iTimeCtr(0),
				  m_iObsCtr(0), m_iObsMax(2) {}
	void Update();
	void Render();
	void Enter(); 
	void Exit();
	void Resume() { cout << "Resuming Game..." << endl; }
};

class TitleState : public State
{
private:
	vector<Button*> m_vButtons;
	enum btn { play, exit };

public:
	TitleState() {}
	void Update();
	void Render();
	void Enter();
	void Exit();
	void Resume() {}
};

class LoseState : public State
{
private:
	vector<Button*> m_vButtons;
	enum btn { exit };
	string m_sTime;

public:
	LoseState(int t) { m_sTime = to_string(t); }
	void Update();
	void Render();
	void Enter();
	void Exit();
	void Resume() {}
};

class StateMachine
{
private:
	vector<State*> m_vStates;

public:
	~StateMachine();
	void Update();
	void Render();
	void PushState(State* pState);
	void ChangeState(State* pState);
	void PopState();
	void Clean();
	vector<State*>& GetStates() { return m_vStates; }
};