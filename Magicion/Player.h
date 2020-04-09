#pragma once

#include <windows.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <string>
#include <sstream>
#include <fstream>
#include <conio.h>
#include <vector>
#include <algorithm>
#include <memory>



class Player
{
private:

public:
	///Balance Modifiers
	const float HealthRegen = 1.f / 50;
	const float ManaRegen = 1.f / 10;
	const float PlayerSpeed = 100;

	const float CostBeam = 20;
	const float DamageBeam = 1000;
	///
	//Constructors/Destructors
	Player();
	~Player();
	//Position
	float x;
	float y;
	//Movement
	void Move(float xV, float yV);

	//Spells
	float Health = 100;
	float Mana = 100;

	void Regeneration(float felapsedtime);

	bool CastingBeam = 0;
	void Beam(int Casting, float felapsedtime = 0);
	void FireCircle();
};

