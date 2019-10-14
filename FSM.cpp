/***********************************
Filename: FSM.cpp
Author: Prof. Alex Richard
Modified By: Hiren Pandya (101198481)
Modified On: 14.04.2019
************************************/
#include <string>
#include "FSM.h"
#include "Game.h"
#include "tinyxml2.h"
#define ROWS 6
#define COLS 8
#define SIZE 128
using namespace tinyxml2;

void State::Render()
{
	SDL_RenderPresent(Game::Instance()->GetRenderer());
}

void State::RenderFont(bool r, const char* c, int x, int y)
{
	if (r)
	{
		SDL_Color textColor = { 255, 255, 255, 0 }; // White text.
		SDL_Surface* fontSurf = TTF_RenderText_Solid(m_Font, c, textColor);
		SDL_DestroyTexture(m_pFontText); // Need to de-allocate previous font texture.
		m_pFontText = SDL_CreateTextureFromSurface(Game::Instance()->GetRenderer(), fontSurf);
		m_rFontRect = { x, y, fontSurf->w, fontSurf->h };
		SDL_FreeSurface(fontSurf);
	}
	SDL_RenderCopy(Game::Instance()->GetRenderer(), m_pFontText, 0, &m_rFontRect);
}

// Begin TitleState
void TitleState::Enter()
{
	cout << "Entering Title..." << endl;
	Game::Instance()->SetLeftMouse(false);
	m_vButtons.push_back(new Button("Img/play.png", { 0,0,400,100 }, { 312,200,400,100 }));
	m_vButtons.push_back(new Button("Img/exit.png", { 0,0,400,100 }, { 312,400,400,100 }));
}

void TitleState::Update()
{
	// Update buttons. Allows for mouseovers.
	for (int i = 0; i < (int)m_vButtons.size(); i++)
		m_vButtons[i]->Update();
	// Parse buttons. Make sure buttons are in an if..else structure.
	if (m_vButtons[btn::play]->Clicked())
		Game::Instance()->GetFSM()->ChangeState(new GameState());
	else if (m_vButtons[btn::exit]->Clicked())
	{
		SDL_Delay(500); // Just pause to let the button sound play.
		Game::Instance()->QuitGame();
	}
}

void TitleState::Render()
{
	cout << "Rendering Title..." << endl;
	SDL_SetRenderDrawColor(Game::Instance()->GetRenderer(), 255, 128, 0, 255);
	SDL_RenderClear(Game::Instance()->GetRenderer());
	for (int i = 0; i < (int)m_vButtons.size(); i++)
		m_vButtons[i]->Render();
	State::Render();
}

void TitleState::Exit()
{
	cout << "Exiting Title..." << endl;
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		delete m_vButtons[i];
		m_vButtons[i] = nullptr;
	}
	m_vButtons.clear();
	m_vButtons.shrink_to_fit();
}
// End TitleState

// Begin GameState
void GameState::Enter()
{
	XMLDocument xmlDoc;
	cout << "Entering Game..." << endl;
	Game::Instance()->GetAM()->LoadMusic("Aud/game.mp3");
	Game::Instance()->GetAM()->PlayMusic();
	m_Font = TTF_OpenFont("Img/LTYPE.TTF", 20);
	SDL_Surface* pBGSurface = IMG_Load("Img/Backgrounds.png");
	m_pBGTexture = SDL_CreateTextureFromSurface(Game::Instance()->GetRenderer(),
		pBGSurface);
	SDL_FreeSurface(pBGSurface);
	// Create player texture
	SDL_Surface* pPSurface = IMG_Load("Img/Player.png");
	m_pPTexture = SDL_CreateTextureFromSurface(Game::Instance()->GetRenderer(), pPSurface);
	SDL_FreeSurface(pPSurface);

	// Obstacle Texture creation.
	SDL_Surface* pOSurface = IMG_Load("Img/Obstacles.png");
	m_pOTexture = SDL_CreateTextureFromSurface(Game::Instance()->GetRenderer(), pOSurface);
	SDL_FreeSurface(pOSurface);

	m_pPlayer = new Player({ 0,256,128,128 },
		{ 1024 / 2 - 64,300,128,128 });
	m_pGround = new Obstacle(0, true, {0,0,0,0},
		{0,512,1024,256}, true);

	// Generate obstacles
	for (int col = 0; col <= COLS; col++) // Looping 9 times
		m_vObstacles.push_back(new Obstacle(col*SIZE));

	if (xmlDoc.LoadFile("Scores.xml") == XML_SUCCESS) {
		XMLNode* pRoot = xmlDoc.FirstChild();
		XMLElement* pElement = pRoot->FirstChildElement("HighScore");
		pElement->QueryIntAttribute("score", &m_iHighScore);
		cout << "Highscore loaded" << endl;
	}
}

void GameState::Update()
{
	if (Game::Instance()->KeyDown(SDL_SCANCODE_P) == 1)
	{
		Game::Instance()->GetFSM()->PushState(new PauseState());
		return;
	}
	// Parse player states
	if (m_pPlayer->m_state == dying)
	{
		m_pPlayer->Animate();
		m_pPlayer->m_iDeathCtr++;
		if (m_pPlayer->m_iDeathCtr == m_pPlayer->m_iDeathCtrMax)
		{
			// SDL_Delay(1000);
			Game::Instance()->GetFSM()->ChangeState(new LoseState(m_iTime));
			return;
		}
		return;
	}

	// If player is either running or rolling
	if (m_pPlayer->m_state == running)
	{
		if (Game::Instance()->KeyDown(SDL_SCANCODE_S))
			m_pPlayer->SetRolling();

		else if (Game::Instance()->KeyDown(SDL_SCANCODE_SPACE) && Game::Instance()->GetSpaceOk())
		{
			Game::Instance()->SetSpaceOk(false);
			m_pPlayer->SetAccelY(-GRAV * 2.25); // Sets the jump force.
			m_pPlayer->SetJumping();
			Game::Instance()->GetAM()->PlaySound(1);
		}
	}
	else if (m_pPlayer->m_state == rolling) {
		if (!Game::Instance()->KeyDown(SDL_SCANCODE_S))
			m_pPlayer->SetRunning();
	}

	// Move player
	if (Game::Instance()->KeyDown(SDL_SCANCODE_A))
	{
		m_pPlayer->SetDir(-1);
		m_pPlayer->MoveX();
	}
	else if (Game::Instance()->KeyDown(SDL_SCANCODE_D))
	{
		m_pPlayer->SetDir(1);
		m_pPlayer->MoveX();
	}
	else 
		m_pPlayer->SetAccelX(0.0);
	
	m_pPlayer->Update();
	m_pPlayer->SetAccelY(0.0); // After jump, reset vertical acceleration.
	if (SDL_HasIntersection(m_pPlayer->GetDstP(), m_pGround->GetSprite()->GetDstP()))
	{
		if ((m_pPlayer->GetDstP()->y + m_pPlayer->GetDstP()->h) - m_pPlayer->GetVelY() <= m_pGround->GetSprite()->GetDstP()->y)
		{ // Collision from top.
			if (m_pPlayer->m_state == jumping) {
				// As soon as the player hits the ground, the player state will be changed to running
				m_pPlayer->SetRunning();
			}
			m_pPlayer->SetVelY(0.0); // Stop the player from moving vertically. We aren't modifying gravity.
			m_pPlayer->SetY(m_pGround->GetSprite()->GetDstP()->y - m_pPlayer->GetDstP()->h - 1);
		}
		else if (m_pPlayer->GetDstP()->y - m_pPlayer->GetVelY() >= m_pGround->GetSprite()->GetDstP()->y + m_pGround->GetSprite()->GetDstP()->h)
		{ // Collision from bottom.
			m_pPlayer->SetVelY(0.0); // Stop the player from moving vertically. We aren't modifying gravity.
			m_pPlayer->SetY(m_pGround->GetSprite()->GetDstP()->y + m_pGround->GetSprite()->GetDstP()->h + 1);
		}
		else if ((m_pPlayer->GetDstP()->x + m_pPlayer->GetDstP()->w) - m_pPlayer->GetVelX() <= m_pGround->GetSprite()->GetDstP()->x)
		{ // Collision from left.
			m_pPlayer->SetVelX(0.0); // Stop the player from moving horizontally.
			m_pPlayer->SetX(m_pGround->GetSprite()->GetDstP()->x - m_pPlayer->GetDstP()->w - 1);
		}
		else if (m_pPlayer->GetDstP()->x - m_pPlayer->GetVelX() >= m_pGround->GetSprite()->GetDstP()->x + m_pGround->GetSprite()->GetDstP()->w)
		{ // Collision from right.
			m_pPlayer->SetVelX(0.0); // Stop the player from moving horizontally.
			m_pPlayer->SetX(m_pGround->GetSprite()->GetDstP()->x + m_pGround->GetSprite()->GetDstP()->w + 1);
		}
	}
	// Check if player is on the platform

	for (int col = 0; col < COLS; col++)
	{
		if (m_vObstacles[col]->GetSprite() != nullptr) {
			if (SDL_HasIntersection(m_pPlayer->GetBound(), m_vObstacles[col]->GetSprite()->GetDstP()) && m_vObstacles[col]->IsPlatform())
			{
				if ((m_pPlayer->GetDstP()->y + m_pPlayer->GetDstP()->h) - m_pPlayer->GetVelY() <= m_vObstacles[col]->GetSprite()->GetDstP()->y)
				{ // Collision from top.
					if (m_pPlayer->m_state == jumping) {
						// As soon as the player hits the ground, the player state will be changed to running
						m_pPlayer->SetRunning();
					}
					m_pPlayer->SetVelY(0.0); // Stop the player from moving vertically.
					m_pPlayer->SetY(m_vObstacles[col]->GetSprite()->GetDstP()->y - m_pPlayer->GetDstP()->h - 1);
				}
				else if (m_pPlayer->GetDstP()->y - m_pPlayer->GetVelY() >= m_vObstacles[col]->GetSprite()->GetDstP()->y + m_vObstacles[col]->GetSprite()->GetDstP()->h)
				{ // Collision from bottom.
					m_pPlayer->SetVelY(0.0); // Stop the player from moving vertically. We aren't modifying gravity.
					m_pPlayer->SetY(m_vObstacles[col]->GetSprite()->GetDstP()->y + m_vObstacles[col]->GetSprite()->GetDstP()->h + 1);
				}
				else if ((m_pPlayer->GetDstP()->x + m_pPlayer->GetDstP()->w) - m_pPlayer->GetVelX() <= m_vObstacles[col]->GetSprite()->GetDstP()->x)
				{ // Collision from left.
					m_pPlayer->SetVelX(0.0); // Stop the player from moving horizontally.
					m_pPlayer->SetX(m_vObstacles[col]->GetSprite()->GetDstP()->x - m_pPlayer->GetDstP()->w - 1);
				}
				else if (m_pPlayer->GetDstP()->x - m_pPlayer->GetVelX() >= m_vObstacles[col]->GetSprite()->GetDstP()->x + m_vObstacles[col]->GetSprite()->GetDstP()->w)
				{ // Collision from right.
					m_pPlayer->SetVelX(0.0); // Stop the player from moving horizontally.
					m_pPlayer->SetX(m_vObstacles[col]->GetSprite()->GetDstP()->x + m_vObstacles[col]->GetSprite()->GetDstP()->w + 1);
				}
				break;
			}
		}
	}


	// Checking collision with obstacles
	for (int col = 0; col <= COLS ; col++)
	{
		if (m_vObstacles[col]->GetSprite() != nullptr) {
			if (SDL_HasIntersection(m_pPlayer->GetBound(),
				m_vObstacles[col]->GetSprite()->GetDstP()) && !m_vObstacles[col]->IsPlatform()) {

				m_pPlayer->SetDying();
				Game::Instance()->GetAM()->PlaySound(2);
				break;
			}
		}
	}
	m_iTimeCtr++;
	// Scroll the backgrounds.
	for (int i = 0; i < 2; i++)
		m_Backgrounds[i].Update();
	for (int i = 0; i < 5; i++)
		m_Midgrounds[i].Update();
	for (int i = 0; i < 3; i++)
		m_Foregrounds[i].Update();
	// The next bit shifts the background images back.
	if (m_Backgrounds[0].GetDstP()->x <= -(m_Backgrounds[0].GetDstP()->w))
	{
		for (int i = 0; i < 2; i++)
			m_Backgrounds[i].GetDstP()->x += m_Backgrounds[i].GetDstP()->w;
	}
	if (m_Midgrounds[0].GetDstP()->x <= -(m_Midgrounds[0].GetDstP()->w))
	{
		for (int i = 0; i < 5; i++)
			m_Midgrounds[i].GetDstP()->x += m_Midgrounds[i].GetDstP()->w;
	}
	if (m_Foregrounds[0].GetDstP()->x <= -(m_Foregrounds[0].GetDstP()->w))
	{
		for (int i = 0; i < 3; i++)
			m_Foregrounds[i].GetDstP()->x += m_Foregrounds[i].GetDstP()->w;
	}
	// Wrap the obstacles
	if (m_vObstacles.front()->GetX() <= -128) {
		// Pop the obstacle
		delete m_vObstacles.front();		// De-allocate the front obstacle which went out of the screen view
		m_vObstacles.erase(m_vObstacles.begin());	// Remove pointer
		// Push new obstacle
		if (m_iObsCtr == 0) {
			int roll = 1 + rand() % 4;
			if (roll == 1) { // Spike wall
				m_vObstacles.push_back(new Obstacle(8 * SIZE, true,
					{0,0,128,448}, { 8 * SIZE,0,128,448} 
					));
			}
			else if (roll == 2) { // Platform, not hazard
				m_vObstacles.push_back(new Obstacle(8 * SIZE, true,
					{ 128,0,128,42 }, { 8*SIZE,340,128,42 }, true
				));
			}
			else if (roll == 3) { // Spikes
				m_vObstacles.push_back(new Obstacle(8 * SIZE, true,
					{ 128,64,128,64 }, { 8 * SIZE,448,128,64 }
				));
			}
			else { // Spinning blade
				m_vObstacles.push_back(new Obstacle(8 * SIZE, true,
					{ 128,128,128,128 }, { 8 * SIZE,448,128,128 }, 0, 1
				));
			}
		}
		else {
			m_vObstacles.push_back(new Obstacle(8*SIZE));
		}
		if (m_iObsCtr == m_iObsMax)
			m_iObsCtr = 0;
		else
			m_iObsCtr++;
	}
	// Scroll the obstacles
	for (int col = 0; col <= COLS; col++)
		m_vObstacles[col]->Update();
}

void GameState::Render()
{
	cout << "Rendering Game..." << endl;
	SDL_SetRenderDrawColor(Game::Instance()->GetRenderer(), 255, 0, 255, 255);
	SDL_RenderClear(Game::Instance()->GetRenderer());
	// Render the backgrounds.
	for (int i = 0; i < 2; i++)
		SDL_RenderCopy(Game::Instance()->GetRenderer(), m_pBGTexture, m_Backgrounds[i].GetSrcP(), m_Backgrounds[i].GetDstP());
	for (int i = 0; i < 5; i++)
		SDL_RenderCopy(Game::Instance()->GetRenderer(), m_pBGTexture, m_Midgrounds[i].GetSrcP(), m_Midgrounds[i].GetDstP());

	// Render the obstacles
	for (int col = 0; col <= COLS; col++)
	{
		if(m_vObstacles[col]->GetSprite() != nullptr) 
			SDL_RenderCopyEx(Game::Instance()->GetRenderer(), m_pOTexture, 
				m_vObstacles[col]->GetSprite()->GetSrcP(), 
				m_vObstacles[col]->GetSprite()->GetDstP(),
				m_vObstacles[col]->GetAngle(), NULL, SDL_FLIP_NONE);
	}
	for (int i = 0; i < 3; i++)
		SDL_RenderCopy(Game::Instance()->GetRenderer(), m_pBGTexture, m_Foregrounds[i].GetSrcP(), m_Foregrounds[i].GetDstP());
	// Render the player.
	
	SDL_RenderCopy(Game::Instance()->GetRenderer(), m_pPTexture, m_pPlayer->GetSrcP(), m_pPlayer->GetDstP());
	// Now render the timer.
	m_iTime =  m_iTimeCtr / FPS;
	if (m_iHighScore < m_iTime)
		m_iHighScore = m_iTime;
	m_sTime = "TIME: " + to_string(m_iTime) + " | HIGHSCORE: " + to_string(m_iHighScore);
	RenderFont((m_iLastTime < m_iTime?1:0), m_sTime.c_str(), 32, 50);
	m_iLastTime = m_iTime;
	
	State::Render();
}

void GameState::Exit()
{
	// Create XML file to store highscore
	XMLDocument xmlDoc;
	XMLNode* pRoot = xmlDoc.NewElement("Root");
	xmlDoc.InsertEndChild(pRoot);
	XMLElement* pElement = xmlDoc.NewElement("HighScore");
	pElement->SetAttribute("score", m_iHighScore);
	pRoot->InsertEndChild(pElement);
	xmlDoc.SaveFile("Scores.xml");

	cout << "Exiting Game..." << endl;
	Game::Instance()->GetAM()->ClearMusic(); // De-allocate the music track.
	TTF_CloseFont(m_Font);
	SDL_DestroyTexture(m_pFontText);
	SDL_DestroyTexture(m_pBGTexture);
	delete m_pPlayer;
	delete m_pGround;
}
// End GameState

// Begin PauseState
void PauseState::Enter()
{
	cout << "Entering Pause..." << endl;
	m_vButtons.push_back(new Button("Img/resume.png", { 0,0,200,80 }, { 412,200,200,80 }));
	m_vButtons.push_back(new Button("Img/exit.png", { 0,0,400,100 }, { 412,400,200,80 }));
	Game::Instance()->GetAM()->ToggleMusic(); // Pause the music.
}

void PauseState::Update()
{
	// Update buttons. Allows for mouseovers.
	for (int i = 0; i < (int)m_vButtons.size(); i++)
		m_vButtons[i]->Update();
	// Parse buttons. Make sure buttons are in an if..else structure.
	if (m_vButtons[btn::resume]->Clicked())
		Game::Instance()->GetFSM()->PopState();
	else if (m_vButtons[btn::exit]->Clicked())
	{
		Game::Instance()->GetFSM()->Clean(); // Clear all states, including GameState on bottom.
		Game::Instance()->GetFSM()->ChangeState(new TitleState()); // Then change to a new TitleState.
	}
}

void PauseState::Render()
{
	cout << "Rendering Pause..." << endl;
	Game::Instance()->GetFSM()->GetStates().front()->Render();
	SDL_SetRenderDrawBlendMode(Game::Instance()->GetRenderer(), SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(Game::Instance()->GetRenderer(), 64, 64, 128, 128);
	SDL_Rect rect = { 256, 128, 512, 512 };
	SDL_RenderFillRect(Game::Instance()->GetRenderer(), &rect);
	for (int i = 0; i < (int)m_vButtons.size(); i++)
		m_vButtons[i]->Render();
	State::Render();
}

void PauseState::Exit()
{
	cout << "Exiting Pause..." << endl;
	Game::Instance()->GetAM()->ToggleMusic(); // Resume the music.
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		delete m_vButtons[i];
		m_vButtons[i] = nullptr;
	}
	m_vButtons.clear();
	m_vButtons.shrink_to_fit();
}
// End PauseState

// Begin LoseState
void LoseState::Enter()
{
	cout << "Entering Lose..." << endl;
	m_vButtons.push_back(new Button("Img/exit.png", { 0,0,400,100 }, { 412,400,200,80 }));
	Game::Instance()->GetAM()->LoadMusic("Aud/lose.mp3");
	Game::Instance()->GetAM()->PlayMusic();
	m_Font = TTF_OpenFont("Img/LTYPE.TTF", 40);
	// Do the font stuff once, so Render() only needs to invoke SDL_RenderCopy().
	m_sTime = "YOU CRASHED! FINAL TIME: " + m_sTime;
	SDL_Color textColor = { 255, 255, 255, 0 }; // White text.
	SDL_Surface* fontSurf = TTF_RenderText_Solid(m_Font, m_sTime.c_str(), textColor);
	m_pFontText = SDL_CreateTextureFromSurface(Game::Instance()->GetRenderer(), fontSurf);
	m_rFontRect = { 200, 250, fontSurf->w, fontSurf->h };
	SDL_FreeSurface(fontSurf);
}

void LoseState::Update()
{
	// Update buttons. Allows for mouseovers.
	for (int i = 0; i < (int)m_vButtons.size(); i++)
		m_vButtons[i]->Update();
	// Parse buttons. Make sure buttons are in an if..else structure.
	if (m_vButtons[btn::exit]->Clicked())
	{
		Game::Instance()->GetFSM()->Clean(); // Clear all states, including GameState on bottom.
		Game::Instance()->GetFSM()->ChangeState(new TitleState()); // Then change to a new TitleState.
	}
}

void LoseState::Render()
{
	cout << "Rendering Lose..." << endl;
	SDL_SetRenderDrawColor(Game::Instance()->GetRenderer(), 255, 0, 0, 255);
	SDL_RenderClear(Game::Instance()->GetRenderer());
	for (int i = 0; i < (int)m_vButtons.size(); i++)
		m_vButtons[i]->Render();
	SDL_RenderCopy(Game::Instance()->GetRenderer(), m_pFontText, 0, &m_rFontRect);
	State::Render();
}

void LoseState::Exit()
{
	cout << "Exiting Lose..." << endl;
	Game::Instance()->GetAM()->ClearMusic(); // De-allocate the music track.
	TTF_CloseFont(m_Font);
	SDL_DestroyTexture(m_pFontText);
	for (int i = 0; i < (int)m_vButtons.size(); i++)
	{
		delete m_vButtons[i];
		m_vButtons[i] = nullptr;
	}
	m_vButtons.clear();
	m_vButtons.shrink_to_fit();
}
// End PauseState

// Begin StateMachine
void StateMachine::Update()
{
	if (!m_vStates.empty()) // empty() and back() are methods of the vector type.
		m_vStates.back()->Update();
}

void StateMachine::Render()
{
	if (!m_vStates.empty())
		m_vStates.back()->Render();
}

void StateMachine::PushState(State* pState)
{
	m_vStates.push_back(pState); // push_back() is a method of the vector type.
	m_vStates.back()->Enter();
}

void StateMachine::ChangeState(State* pState)
{
	if (!m_vStates.empty())
	{
		m_vStates.back()->Exit();
		delete m_vStates.back(); // De-allocating the state in the heap.
		m_vStates.back() = nullptr; // Nullifying pointer to the de-allocated state.
		m_vStates.pop_back(); // Removes the now-null pointer from the vector.
	}
	pState->Enter();
	m_vStates.push_back(pState);
}

void StateMachine::PopState()
{
	if (!m_vStates.empty())
	{
		m_vStates.back()->Exit();
		delete m_vStates.back();
		m_vStates.back() = nullptr;
		m_vStates.pop_back();
	}
	m_vStates.back()->Resume();
}

void StateMachine::Clean()
{
	while (!m_vStates.empty()) // Ensures that ALL states left in the vector are cleaned up.
	{						   
		m_vStates.back()->Exit();
		delete m_vStates.back();
		m_vStates.back() = nullptr;
		m_vStates.pop_back();
	}
}

StateMachine::~StateMachine()
{
	cout << "Destroying FSM..." << endl;
	Clean();
}
// End StateMachine