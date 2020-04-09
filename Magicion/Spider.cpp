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
	fSpeed += (rand() % 5 + 2) * 0.1f;

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

void Spider::Path(float PlayerX, float PlayerY)
{
	fDistanceFactor = (float)sqrt(pow((double)PlayerX - (double)x, 2) + pow((double)PlayerY - (double)y, 2));
	fDistanceFactor = log10f(fDistanceFactor);

		if (PlayerX > x)Vx += fElapsedTime * fSpeed * fDistanceFactor;
		if (PlayerX < x)Vx -= fElapsedTime * fSpeed * fDistanceFactor;
		if (PlayerY > y)Vy += fElapsedTime * fSpeed * fDistanceFactor;
		if (PlayerY < y)Vy -= fElapsedTime * fSpeed * fDistanceFactor;

		if (sqrt(pow((double)Vx, 2) + pow((double)Vy, 2)) >= 10.f)
		{
			Vx *= 0.9f;
			Vy *= 0.9f;
		}
}

void Spider::EdgeCollision()
{
	//if(x < 0)
	
}

void Spider::Move()
{
	x += Vx;
	y += Vy;
}
