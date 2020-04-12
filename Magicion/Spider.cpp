#include "Spider.h"

Spider::Spider(int x, int y)
{
	//Initializing Variables

	//Velocity
	this->Vx = 0.f;
	this->Vy = 0.f;
	this->fDistanceFactor = 0.f;

	//Stats
	Health = 300.f;
	fSpeed += (rand() % 5 + 2) * 0.01f;

	///Necessary 

	//Position
	this->x = (float)x;
	this->y = (float)y;

}

Spider::~Spider()
{

}

void Spider::Timing()
{
	end = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsedTime = end - start;
	start = end;
	fElapsedTime = elapsedTime.count();

}

void Spider::Tick(float PlayerX, float PlayerY)
{
	this->Timing();

	this->Path(PlayerX,PlayerY);
	this->Move();

}

void Spider::TakeDamage(float Damage)
{
	Health -= Damage;
}

void Spider::ReverseDirection(bool dx, bool dy)
{
	if (dx == 1)Vx = -Vx * 1.00f;
	if (dy == 1)Vy = -Vy * 1.00f;
	this->Move();
}

void Spider::Path(float PlayerX, float PlayerY)
{
	double dx = ((double)PlayerX - (double)x) * ((double)PlayerX - (double)x);
	double dy = ((double)PlayerY - (double)y) * ((double)PlayerY - (double)y);

	fDistanceFactor = (float)sqrt(dx + dy);
	fDistanceFactor = fDistanceFactor / (sqrt(600 * 600 * 2)*15.f);

		if (PlayerX > x)Vx += fElapsedTime * (fSpeed + fDistanceFactor);
		if (PlayerX < x)Vx -= fElapsedTime * (fSpeed + fDistanceFactor);
		if (PlayerY > y)Vy += fElapsedTime * (fSpeed + fDistanceFactor);
		if (PlayerY < y)Vy -= fElapsedTime * (fSpeed + fDistanceFactor);

		if (sqrt(Vx*Vx + Vy*Vy) >= 0.5f)
		{
			Vx *= 0.8f;
			Vy *= 0.8f;
		}
}

void Spider::Move()
{
	x += Vx;
	y += Vy;
}
