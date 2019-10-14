/***********************************
Filename: Sprite.cpp
Author: Prof. Alex Richard
Modified By: Hiren Pandya (101198481)
Modified On: 14.04.2019
************************************/
#include <algorithm> // For min/max.
#include <cmath>	 // For cos/sin.
#include "Game.h"
#include "Sprites.h"

using namespace std;

AnimatedSprite::AnimatedSprite(SDL_Rect s, SDL_Rect d): Sprite(s, d) {}

void AnimatedSprite::Animate()
{
	m_iFrame++;
	if (m_iFrame == m_iFrameMax)
	{
		m_iFrame = 0;
		m_iSprite++;
		if (m_iSprite == m_iSpriteMax)
		{
			m_iSprite = m_iSpriteMin;
		}
	}
	m_rSrc.x = m_rSrc.w * m_iSprite;
	cout << m_iSprite << " ";
	m_rBound.h = 124;
	m_rBound.w = 64; 
}

void AnimatedSprite::SetJumping() {
	m_state = jumping;
	m_iFrame = 0;
	m_iFrameMax = 1;					// Frame speed
	m_iSprite = m_iSpriteMin = 8;		// sprite for frames
	m_iSpriteMax = 9;
	m_rSrc.y = 256;
	m_rBound.h = 124; 
	m_rBound.w = 64; // Move to contructor
}
	
void AnimatedSprite::SetRunning() {
	m_state = running;
	m_iFrame = 0;
	m_iFrameMax = 6;
	m_iSprite = m_iSpriteMin = 0;
	m_iSpriteMax = 8;
	m_rSrc.y = 256;
	m_rBound.h = 124; 
}
void AnimatedSprite::SetRolling() {
	m_state = rolling;
	m_iFrame = 0;
	m_iFrameMax = 6;
	m_iSprite = m_iSpriteMin = 0;
	m_iSpriteMax = 4;
	m_rSrc.y = 256 + 128;
	m_rBound.h = 60; 
}
void AnimatedSprite::SetDying() {
	m_state = dying;
	m_iFrame = 0;
	m_iFrameMax = 12;
	m_iSprite = m_iSpriteMin = 4;
	m_iSpriteMax = 9;
	m_iDeathCtrMax = 12 * 5; //( 12 frames * # no sprites )
	m_rSrc.y = 256 + 128;
	m_rBound.h = 60; 
}

Player::Player(SDL_Rect s, SDL_Rect d):AnimatedSprite(s,d)
{
	//m_bGrounded = false;
	m_dAccelX = m_dAccelY = m_dVelX = m_dVelY = 0.0;
	m_dGrav = GRAV;
	m_dMaxAccelX = 2.0;
	m_dMaxVelX = 6.0;
	m_dMaxVelY = m_dGrav;
	m_dDrag = 0.925;
	m_iDir = 1;
	SetRunning();
}

void Player::SetDir(int dir)
{ // Will be used to set direction of sprite. Just added it for you.
	m_iDir = dir;
}

void Player::MoveX()
{
	m_dAccelX += 0.25 * m_iDir; // Change to speed var.
}

void Player::Update()
{
	m_dAccelX = min(max(m_dAccelX, -(m_dMaxAccelX)), (m_dMaxAccelX));
	m_dVelX = (m_dVelX + m_dAccelX) * m_dDrag;
	m_dVelX = min(max(m_dVelX, -(m_dMaxVelX)), (m_dMaxVelX));
	m_rDst.x += (int)m_dVelX;
	m_rDst.x = min(max(m_rDst.x, 0), 1024-128);
	m_dVelY += m_dAccelY + m_dGrav/8;
	m_dVelY = min(max(m_dVelY, -(m_dMaxVelY * 10)), (m_dMaxVelY));
	// If you want to use non-lethal platform
	// In case of multiple jumping frames, we need another check in addition to below to confirm whether player sprite is already in jumping state or not.
	// If not, set jumping
	if (fabs(m_dVelY) > m_dGrav / 4) {
		//SetJumping();
	}
	m_rDst.y += (int)m_dVelY;
	m_rBound.x = m_rDst.x + 32;
	m_rBound.y = m_rDst.y + ((m_state == rolling)?68:4);
	Animate();
}

Obstacle::Obstacle(int x, bool b, SDL_Rect src, 
	SDL_Rect dst, bool p, bool r)
{
	m_iX = x;
	if (b) // Construct the Sprite
	{
		m_pSprite = new Sprite(src,dst);
		m_bIsPlatform = p;
		m_bRotates = r;
	}
}

Obstacle::~Obstacle()
{
	if (m_pSprite != nullptr)
	{
		delete m_pSprite;
		m_pSprite = nullptr;
	}
}

void Obstacle::Update()
{
	m_iX -= 3;
	if (m_pSprite != nullptr) {
		m_pSprite->GetDstP()->x = m_iX;		// Can be "-= 3" as well
		if (m_bRotates)
			m_dAngle += 5;
	}
}
