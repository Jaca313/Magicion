#pragma once
#include <chrono>

class Spider
{
public:
	//Position
	float x;
	float y;
	//Velocity for orbiting
	float Vx;
	float Vy;
	float fDistanceFactor;

	//Stats
	float Health;
	float fSpeed = 0.5f;



	Spider(int x = 20, int y = 20);
	~Spider();

private:
	//Timing 
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	float fElapsedTime = 0.f;
	void Timing();
public:
	//Passive Actions
	void Tick(float PlayerX, float PlayerY);
	void TakeDamage(float Damage);


	//Active
	void ReverseDirection(bool x,bool y);
private:
	//Active
	void Path(float PlayerX,float PlayerY);
	void Move();
};

