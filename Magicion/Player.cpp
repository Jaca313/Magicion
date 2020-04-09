#include "Player.h"

Player::Player()
{
	x = 200;
	y = 60;
}

Player::~Player()
{
}
void Player::Move(float xV, float yV)
{
	this->x += xV * PlayerSpeed;
	this->y += yV * PlayerSpeed;
}

void Player::Regeneration(float felapsedtime)
{
	if(static_cast<int>(Health) < 100)Health += felapsedtime * 100 * HealthRegen;
	if(static_cast<int>(Mana) < 100)Mana += felapsedtime * 100 * ManaRegen;
}

void Player::Beam(int Casting, float felapsedtime)
{
	if (Casting == 1 && Mana > 0)CastingBeam = Casting;
	else CastingBeam = 0;

	if (CastingBeam)
	{
		if (Mana > 0)Mana -= felapsedtime * CostBeam;
	}
}

void Player::FireCircle()
{


}

