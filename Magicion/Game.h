#pragma once


#include "Player.h"
#include "Spider.h"
#include "Sprite.h"

#include <string>
#include <algorithm>
#include <stdio.h>
#include <array>
#include <numeric>

enum COLOUR
{
	FG_BLACK = 0x0000,
	FG_DARK_BLUE = 0x0001,
	FG_DARK_GREEN = 0x0002,
	FG_DARK_CYAN = 0x0003,
	FG_DARK_RED = 0x0004,
	FG_DARK_MAGENTA = 0x0005,
	FG_DARK_YELLOW = 0x0006,
	FG_GREY = 0x0007, // Thanks MS :-/
	FG_DARK_GREY = 0x0008,
	FG_BLUE = 0x0009,
	FG_GREEN = 0x000A,
	FG_CYAN = 0x000B,
	FG_RED = 0x000C,
	FG_MAGENTA = 0x000D,
	FG_YELLOW = 0x000E,
	FG_WHITE = 0x000F,
	BG_BLACK = 0x0000,
	BG_DARK_BLUE = 0x0010,
	BG_DARK_GREEN = 0x0020,
	BG_DARK_CYAN = 0x0030,
	BG_DARK_RED = 0x0040,
	BG_DARK_MAGENTA = 0x0050,
	BG_DARK_YELLOW = 0x0060,
	BG_GREY = 0x0070,
	BG_DARK_GREY = 0x0080,
	BG_BLUE = 0x0090,
	BG_GREEN = 0x00A0,
	BG_CYAN = 0x00B0,
	BG_RED = 0x00C0,
	BG_MAGENTA = 0x00D0,
	BG_YELLOW = 0x00E0,
	BG_WHITE = 0x00F0,
};

enum PIXEL_TYPE
{
	PIXEL_SOLID = 0x2588,
	PIXEL_THREEQUARTERS = 0x2593,
	PIXEL_HALF = 0x2592,
	PIXEL_QUARTER = 0x2591,
};

class GameSystem
{
private:

	//Console Variables
	int m_nScreenWidth;
	int m_nScreenHeight;

	std::wstring m_sAppName;
	HANDLE m_hOriginalConsole;
	CONSOLE_SCREEN_BUFFER_INFO m_OriginalConsoleInfo;
	HANDLE m_hConsole;
	HANDLE m_hConsoleIn;
	SMALL_RECT m_rectWindow;

	bool m_bConsoleInFocus = true;
	bool m_bEnableSound = false;

	//Game Loop Control
	bool Exit = 0;


	int m_mousePosX;
	int m_mousePosY;

	//Output
	CHAR_INFO* m_bufScreen;
	CHAR_INFO* m_bufScreen_current;
	CHAR_INFO* m_bufScreen_blank;

	//Entities
	Player Gracz;
	std::vector<std::unique_ptr<Spider>> Spiders;

	Sprite PlayerSprite{ "wizard2.ppm" };

	Sprite SpiderSprite{ "NinjaVampire.ppm",4 };

	//Timing variables
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	float fClockTime;

	const float DesiredDisplayRate = 30.f;

	//Time Keeping
	float DisplayRefresh = 0.f;
	float fElapsedTime = 0.f;
	float fRenderTime = 0.f;
	float fBarTime = 0.f;

	int DisplayArrayCurrent = 0;
	std::array<float, 30> AvgDisplayTime;
	int RenderArrayCurrent = 0;
	std::array<float, 45> AvgRenderTime;
	int UpdateArrayCurrent = 0;
	std::array<float, 200> AvgUpdateTime;


	float SpawnMonsterTimer = 0;

	//Initializers
	void ConstructConsole(int width, int height, int fontw, int fonth);

	void initConsole();

	void initBlankBuffer();

	void Timing();

public:

	//Constructors/Destructors
	GameSystem();
	~GameSystem();

	//Functions
private:
	void Error(std::string Message);
	void UpdateBar();
	void UpdateScreen();
	void update();
	void render();

	//Draw Functions
	void Draw(int x, int y, short c, short col);
	void DrawCircle(int xc, int yc, int r, short c = 0x2588, short col = 0x000F);
	void DrawLine(int x1, int y1, int x2, int y2, short c = 0x2588, short col = 0x000F);
	void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, short c, short col);
	void DrawString(int x, int y, std::wstring c, short col = 0x000F);
	void Fill(int x1, int y1, int x2, int y2, short c = 0x2588, short col = 0x000F);
	void Clip(int& x, int& y);
	void DrawSprite(int x1, int y1, Sprite &Object);

	//Utility
	void ClearScreen();

	//Custom Draw Entities
	void DrawPlayer();
	void DrawBeam();
	void DrawHUD();
	void DrawSpiders();

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
	void HandleConsoleMouse();
public:
	void run();



	




};

