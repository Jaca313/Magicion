#pragma once

//Enum File with Color and Pixel info
#include "PixelEnum.h"
//Logger
#include "Info.h"

#include "Player.h"
#include "Spider.h"
#include "Sprite.h"

#include <Windows.h>

#include <string>
#include <algorithm>
#include <stdio.h>
#include <array>
#include <numeric>

#include <thread>
#include <future>


class GameSystem
{
private:
	//Threads
	std::mutex DisplayGuard;// render/display race to m_bufscreen_pass
	std::mutex MouseLock;// Lock mouse position changing

	//Control Variables
	bool Exit = 0;

	//Input Information
	int m_mousePosX;
	int m_mousePosY;

	//buffer for reading
	int buf_m_mousePosX;
	int buf_m_mousePosY;

	//Entities
	Player Gracz;
	std::vector<std::unique_ptr<Spider>> Spiders;

	//Graphics Resources
	Sprite PlayerSprite{ "wizard2.ppm" };
	Sprite SpiderSprite{ "NinjaVampire.ppm",4 };

	////////////////////////////////////////////////////////////////////////////////
	//                                    Timing
	////////////////////////////////////////////////////////////////////////////////
private:
	//Functions
	void Timing();

	//Main Timing variables
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	float fClockTime;

	//Time Keeping ergo Timers
	float fDisplayTime = 0.f;
	float fElapsedTime = 0.f;
	float fRenderTime = 0.f;
	float fBarTime = 0.f;

	float SpawnMonsterTimer = 0;


	//Time Keeping consts
	const float DesiredDisplayRate = 30.f;

	//FrameRate Information
	int DisplayArrayCurrent = 0;
	std::array<float, 30> AvgDisplayTime;
	int RenderArrayCurrent = 0;
	std::array<float, 90> AvgRenderTime;
	int UpdateArrayCurrent = 0;
	std::array<float, 200> AvgUpdateTime;

	////////////////////////////////////////////////////////////////////////////////
	//                              Console Init 
	////////////////////////////////////////////////////////////////////////////////
public:
	//Constructors/Destructors
	GameSystem();
	~GameSystem();

private:
	//Console Init
	void initConsole();

	void ConstructConsole(int width, int height, int fontw, int fonth);
	void initSupBuffers();

	//Console Window Settings
	std::wstring m_sAppName;
	int m_nScreenWidth;
	int m_nScreenHeight;

	//Console Window Resources
	HANDLE m_hOriginalConsole;
	CONSOLE_SCREEN_BUFFER_INFO m_OriginalConsoleInfo;
	HANDLE m_hConsole;
	HANDLE m_hConsoleIn;
	SMALL_RECT m_rectWindow;

	//Console Pre-Sets
	bool m_bConsoleInFocus = true;
	bool m_bEnableSound = false;

	//Console Output Buffers
	CHAR_INFO* m_bufScreen;
	CHAR_INFO* m_bufScreen_current;
	CHAR_INFO* m_bufScreen_pass;//passthrough buffer between render and display

	//Initialize Logging Tool
	Info& Tool = Info::getInstance();

	////////////////////////////////////////////////////////////////////////////////
	//								Game Loop
	////////////////////////////////////////////////////////////////////////////////


public:
	//Entrance Point
	void run();
private:
	//Thread 1

	//Timing()
	void UpdateBar();
	void update();
	void render();


	//Thread 2
	void HandleConsole();

	void HandleConsoleMouse();
	void UpdateScreen();//Run on a Thread2



	////////////////////////////////////////////////////////////////////////////////
	//							Draw Functions
	////////////////////////////////////////////////////////////////////////////////
private:
	void Draw(int x, int y, short c, short col);
	void DrawCircle(int xc, int yc, int r, short c = 0x2588, short col = 0x000F);
	void DrawLine(int x1, int y1, int x2, int y2, short c = 0x2588, short col = 0x000F);
	void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, short c, short col);
	void DrawString(int x, int y, std::wstring c, short col = 0x000F);
	void Fill(int x1, int y1, int x2, int y2, short c = 0x2588, short col = 0x000F);
	void Clip(int& x, int& y);
	void DrawSprite(int x1, int y1, Sprite& Object);
	void ClearScreen();

	////////////////////////////////////////////////////////////////////////////////
	//								Utility
	////////////////////////////////////////////////////////////////////////////////

	//Fills the memory space with doubling the pattern
	int memcopyfill(void* const pToFill, const size_t pToFillSize, const void* const  pFillWith, const size_t pFillWithSize);

	//Fills the memory with CHAR_INFO character
	int memfill(CHAR_INFO* const pToFill, const size_t pToFillSize, const CHAR_INFO* const pFillWith, const size_t pFillWithSize);

	//Needs a rework currently useless
	void Error(std::string Message);


	////////////////////////////////////////////////////////////////////////////////
	//							     Render
	////////////////////////////////////////////////////////////////////////////////

	void DrawPlayer();
	void DrawBeam();
	void DrawHUD();
	void DrawSpiders();


	////////////////////////////////////////////////////////////////////////////////
	//								Update
	////////////////////////////////////////////////////////////////////////////////





	///////////Rework stuff below

	//Checks
	void PassiveTick();
	void DamageSpiders();
	void CheckMonsterHealth();



	//Helper Functions
	bool Is_Between(float mouse, float from, float to);
	bool HealthLow(std::unique_ptr<Spider>& Spooder);

	//Random World Functions

	void GenerateMonsters();
	//
	void GetUserInput();




};

