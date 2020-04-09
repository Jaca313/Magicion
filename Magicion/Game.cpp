#include "Game.h"


//Initializers



void GameSystem::ConstructConsole(int width, int height, int fontw, int fonth)
{
	//Ripped Code from javidx9 to create a console

	if (m_hConsole == INVALID_HANDLE_VALUE)
		return Error("Bad Handle");

	m_nScreenWidth = width;
	m_nScreenHeight = height;

	// Update 13/09/2017 - It seems that the console behaves differently on some systems
	// and I'm unsure why this is. It could be to do with windows default settings, or
	// screen resolutions, or system languages. Unfortunately, MSDN does not offer much
	// by way of useful information, and so the resulting sequence is the reult of experiment
	// that seems to work in multiple cases.
	//
	// The problem seems to be that the SetConsoleXXX functions are somewhat circular and 
	// fail depending on the state of the current console properties, i.e. you can't set
	// the buffer size until you set the screen size, but you can't change the screen size
	// until the buffer size is correct. This coupled with a precise ordering of calls
	// makes this procedure seem a little mystical :-P. Thanks to wowLinh for helping - Jx9

	// Change console visual size to a minimum so ScreenBuffer can shrink
	// below the actual visual size
	m_rectWindow = { 0, 0, 1, 1 };
	SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow);

	// Set the size of the screen buffer
	COORD coord = { (short)m_nScreenWidth, (short)m_nScreenHeight };
	if (!SetConsoleScreenBufferSize(m_hConsole, coord))
		Error("SetConsoleScreenBufferSize");

	// Assign screen buffer to the console
	if (!SetConsoleActiveScreenBuffer(m_hConsole))
		return Error("SetConsoleActiveScreenBuffer");

	// Set the font size now that the screen buffer has been assigned to the console
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = (short)fontw;
	cfi.dwFontSize.Y = (short)fonth;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;

	/*	DWORD version = GetVersion();
	DWORD major = (DWORD)(LOBYTE(LOWORD(version)));
	DWORD minor = (DWORD)(HIBYTE(LOWORD(version)));*/

	//if ((major > 6) || ((major == 6) && (minor >= 2) && (minor < 4)))		
	//	wcscpy_s(cfi.FaceName, L"Raster"); // Windows 8 :(
	//else
	//	wcscpy_s(cfi.FaceName, L"Lucida Console"); // Everything else :P

	//wcscpy_s(cfi.FaceName, L"Liberation Mono");
	wcscpy_s(cfi.FaceName, L"Consolas");
	if (!SetCurrentConsoleFontEx(m_hConsole, false, &cfi))
		return Error("SetCurrentConsoleFontEx");

	// Get screen buffer info and check the maximum allowed window size. Return
	// error if exceeded, so user knows their dimensions/fontsize are too large
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(m_hConsole, &csbi))
		return Error("GetConsoleScreenBufferInfo");
	if (m_nScreenHeight > csbi.dwMaximumWindowSize.Y)
		return Error("Screen Height / Font Height Too Big");
	if (m_nScreenWidth > csbi.dwMaximumWindowSize.X)
		return Error("Screen Width / Font Width Too Big");

	// Set Physical Console Window Size
	m_rectWindow = { 0, 0, (short)m_nScreenWidth - 1, (short)m_nScreenHeight - 1 };
	if (!SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow))
		return Error("SetConsoleWindowInfo");

	// Set flags to allow mouse input		
	if (!SetConsoleMode(m_hConsoleIn, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT))
		return Error("SetConsoleMode");

	// Allocate memory for screen buffer
	m_bufScreen = new CHAR_INFO[m_nScreenWidth * m_nScreenHeight];
	memset(m_bufScreen, 0, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);

	//Allocate memory for the second buffer
	m_bufScreen_current = new CHAR_INFO[m_nScreenWidth * m_nScreenHeight];
	memset(m_bufScreen_current, 0, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);

	//Allocate memory for the blank buffer
	m_bufScreen_blank = new CHAR_INFO[m_nScreenWidth * m_nScreenHeight];
	memset(m_bufScreen_blank, 0, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);


	//SetConsoleCtrlHandler((PHANDLER_ROUTINE)CloseHandler, TRUE);

}

void GameSystem::initConsole()
{
	//Creates a Console with options from window.ini

	std::wifstream ifs("Window.ini");

	//Default Values
	m_nScreenWidth = 80;
	m_nScreenHeight = 30;
	int fontw = 8;
	int fonth = 8;
	m_sAppName = L"Default";

	if (ifs.is_open())
	{
		std::getline(ifs, m_sAppName);
		ifs >> m_nScreenWidth >> m_nScreenHeight >> fontw >> fonth;
	}

	ifs.close();

	this->ConstructConsole(m_nScreenWidth,m_nScreenHeight, fontw, fonth);
}

void GameSystem::initBlankBuffer()
{
	int x1 = 0;
	int x2 = m_nScreenWidth;

	int y1 = 0;
	int y2 = m_nScreenHeight;

	Fill(x1, y1, x2, y2);

	std::memcpy(m_bufScreen_blank, m_bufScreen, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);

}

void GameSystem::Timing()
{
	end = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsedTime = end - start;
	start = end;


	fClockTime = elapsedTime.count();

	fElapsedTime += fClockTime;
	fRenderTime += fClockTime;
	fBarTime += fClockTime;
	DisplayRefresh += fClockTime;
	//fElapsedTime = elapsedTime.count();

	SpawnMonsterTimer += fClockTime;


}

//Constructors/Destructors

GameSystem::GameSystem()
{
	m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);

	m_bEnableSound = false;

	this->initConsole();

	this->initBlankBuffer();

	AvgUpdateTime.fill(0);
	AvgRenderTime.fill(0);



}

GameSystem::~GameSystem()
{
	SetConsoleActiveScreenBuffer(m_hOriginalConsole);
	delete[] m_bufScreen;
	delete[] m_bufScreen_current;
}

//Functions

void GameSystem::Error(std::string Message)
{
	std::cout << Message << "\n";
}

void GameSystem::UpdateBar()
{
	if (fBarTime > 1.0f)
	{
		fBarTime = 0.f;
		//(1.f)/(Sum/x) == x/Sum
		float titleAvgDisplay = 30.f / std::accumulate(AvgDisplayTime.begin(), AvgDisplayTime.end(), 0.f);
		float titleAvgRender = 45.f / std::accumulate(AvgRenderTime.begin(), AvgRenderTime.end(), 0.f);
		float titleAvgUpdate = 200.f /std::accumulate(AvgUpdateTime.begin(), AvgUpdateTime.end(), 0.f);


		wchar_t s[256];
		swprintf_s(s, 256, L"%s - Display FPS: %3.2f Render FPS: %3.2f Update FPS: %3.2f", m_sAppName.c_str(), titleAvgDisplay, titleAvgRender, titleAvgUpdate);
		SetConsoleTitle(s);

	}
}


void GameSystem::UpdateScreen()
{	   
	if (DisplayRefresh > 1.f/DesiredDisplayRate)
	{
		AvgDisplayTime[DisplayArrayCurrent] = DisplayRefresh;
		DisplayArrayCurrent = (++DisplayArrayCurrent) % 30;


		DisplayRefresh = 0.00f;

		m_rectWindow.Top = 0;
		m_rectWindow.Left = 0;
		m_rectWindow.Right = (short)m_nScreenWidth;
		m_rectWindow.Bottom = (short)m_nScreenHeight;

		/*
		for (int x = 0; x < m_nScreenWidth; x++)
		{
			for (int y = 0; y < m_nScreenHeight; y++)
			{
				m_bufScreen_current[y * m_nScreenWidth + x] = m_bufScreen[y * m_nScreenWidth + x];
			}
		}
		*/

		std::memcpy(m_bufScreen_current, m_bufScreen, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);



		WriteConsoleOutput(m_hConsole, m_bufScreen_current, { (short)m_nScreenWidth, (short)m_nScreenHeight }, { 0, 0 }, &m_rectWindow);
	}
	/*Selective Overwriting of different pixels disabled as off 08.04.20
	else
	{
		for (int x = 0; x < m_nScreenWidth; x++)
		{
			m_rectWindow.Left = x;
			m_rectWindow.Right = x;
			for (int y = 0; y < m_nScreenHeight; y++)
			{
				m_rectWindow.Top = y;
				m_rectWindow.Bottom = y;
				//if (m_bufScreen[y*m_nScreenWidth+x] == m_bufScreen_current[y * m_nScreenWidth + x])
				if (m_bufScreen[y * m_nScreenWidth + x].Attributes != m_bufScreen_current[y * m_nScreenWidth + x].Attributes)
				{
					m_bufScreen_current[y * m_nScreenWidth + x].Attributes = m_bufScreen[y * m_nScreenWidth + x].Attributes;
					m_bufScreen_current[y * m_nScreenWidth + x].Char = m_bufScreen[y * m_nScreenWidth + x].Char;
					WriteConsoleOutput(m_hConsole, m_bufScreen_current, { (short)m_nScreenWidth, (short)m_nScreenHeight }, { (short)x, (short)y }, &m_rectWindow);
				}
			}
		}
	}

	*/

	/*
	for (int x = 0; x < m_nScreenWidth; x++)
	{
		bool Change = 0;
		m_rectWindow.Left = x;
		m_rectWindow.Right = x;
		m_rectWindow.Top = 0;
		m_rectWindow.Bottom = m_nScreenHeight;
		for (int y = 0; y < m_nScreenHeight; y++)
		{
			if (m_bufScreen[y * m_nScreenWidth + x].Attributes != m_bufScreen_current[y * m_nScreenWidth + x].Attributes)
			{
				Change = 1;
				m_bufScreen_current[y * m_nScreenWidth + x].Attributes = m_bufScreen[y * m_nScreenWidth + x].Attributes;
				m_bufScreen_current[y * m_nScreenWidth + x].Char = m_bufScreen[y * m_nScreenWidth + x].Char;
			}
		}
		if(Change)WriteConsoleOutput(m_hConsole, m_bufScreen_current, { (short)m_nScreenWidth, (short)m_nScreenHeight }, { (short)x, (short)0 }, &m_rectWindow);
	}
	*/
	//WriteConsoleOutput(m_hConsole, m_bufScreen, { (short)m_nScreenWidth, (short)m_nScreenHeight }, { 0, 0 }, &m_rectWindow); 
	
}


void GameSystem::update()
{
	if (fElapsedTime > 1.f / 200.f)
	{
		AvgUpdateTime[UpdateArrayCurrent] = fElapsedTime;
		UpdateArrayCurrent = (++UpdateArrayCurrent)%200;

		HandleConsoleMouse();
		GetUserInput();

		PassiveTick();

		for (std::unique_ptr<Spider>& S : Spiders)
		{
			S->Tick(Gracz.x, Gracz.y);
		}
		fElapsedTime = 0.f;
	}
}

void GameSystem::render()
{
	if (fRenderTime > 1.f / 45.f)
	{
	AvgRenderTime[RenderArrayCurrent] = fRenderTime;
	RenderArrayCurrent = (++RenderArrayCurrent)%45;



	//this->Fill(0, 0, m_nScreenWidth, m_nScreenHeight, 0x2588, FG_GREY);//Clear Screen
	ClearScreen();


	DrawSpiders();
	DrawPlayer();
	DrawBeam();
	DrawHUD();

	fRenderTime = 0.f;
	}
}



void GameSystem::run()
{
	while (this->Exit != 1)
	{
		this->Timing();	
		this->UpdateBar();

		this->update();
		this->render();
		this->UpdateScreen();

	}
}

//Draw Functions

void GameSystem::Draw(int x, int y, short c = 0x2588, short col = 0x000F)
{
	if (x >= 0 && x < m_nScreenWidth && y >= 0 && y < m_nScreenHeight)
	{
		m_bufScreen[y * m_nScreenWidth + x].Char.UnicodeChar = c;
		m_bufScreen[y * m_nScreenWidth + x].Attributes = col;
	}
}

void GameSystem::DrawCircle(int xc, int yc, int r, short c, short col)
{
	int x = 0;
	int y = r;
	int p = 3 - 2 * r;
	if (!r) return;

	while (y >= x) // only formulate 1/8 of circle
	{
		Draw(xc - x, yc - y, c, col);//upper left left
		Draw(xc - y, yc - x, c, col);//upper upper left
		Draw(xc + y, yc - x, c, col);//upper upper right
		Draw(xc + x, yc - y, c, col);//upper right right
		Draw(xc - x, yc + y, c, col);//lower left left
		Draw(xc - y, yc + x, c, col);//lower lower left
		Draw(xc + y, yc + x, c, col);//lower lower right
		Draw(xc + x, yc + y, c, col);//lower right right
		if (p < 0) p += 4 * x++ + 6;
		else p += 4 * (x++ - y--) + 10;
	}
}

void GameSystem::DrawLine(int x1, int y1, int x2, int y2, short c, short col)
{
	int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
	dx = x2 - x1; dy = y2 - y1;
	dx1 = abs(dx); dy1 = abs(dy);
	px = 2 * dy1 - dx1;	py = 2 * dx1 - dy1;
	if (dy1 <= dx1)
	{
		if (dx >= 0)
		{
			x = x1; y = y1; xe = x2;
		}
		else
		{
			x = x2; y = y2; xe = x1;
		}

		Draw(x, y, c, col);

		for (i = 0; x < xe; i++)
		{
			x = x + 1;
			if (px < 0)
				px = px + 2 * dy1;
			else
			{
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) y = y + 1; else y = y - 1;
				px = px + 2 * (dy1 - dx1);
			}
			Draw(x, y, c, col);
		}
	}
	else
	{
		if (dy >= 0)
		{
			x = x1; y = y1; ye = y2;
		}
		else
		{
			x = x2; y = y2; ye = y1;
		}

		Draw(x, y, c, col);

		for (i = 0; y < ye; i++)
		{
			y = y + 1;
			if (py <= 0)
				py = py + 2 * dx1;
			else
			{
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) x = x + 1; else x = x - 1;
				py = py + 2 * (dx1 - dy1);
			}
			Draw(x, y, c, col);
		}
	}
}

void GameSystem::DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, short c = 0x2588, short col = 0x000F)
{
	DrawLine(x1, y1, x2, y2, c, col);
	DrawLine(x2, y2, x3, y3, c, col);
	DrawLine(x3, y3, x1, y1, c, col);
}

void GameSystem::DrawString(int x, int y, std::wstring c, short col)
{
		for (size_t i = 0; i < c.size(); i++)
		{
			m_bufScreen[y * m_nScreenWidth + x + i].Char.UnicodeChar = c[i];
			m_bufScreen[y * m_nScreenWidth + x + i].Attributes = col;
		}

}

void GameSystem::Fill(int x1, int y1, int x2, int y2, short c, short col)
{
	Clip(x1, y1);
	Clip(x2, y2);

	CHAR_INFO FillChar;
	FillChar.Char.UnicodeChar = c;
	FillChar.Attributes = col;

	//std::memset(&m_bufScreen[y1*m_nScreenWidth + x1],FillChar,sizeof(CHAR_INFO)*(x2-x1)*(y2-y1))//wont work as it is on a per byte basis

	//std::fill_n(m_bufScreen[y1 * m_nScreenWidth + x1], sizeof(CHAR_INFO) * (x2 - x1) * (y2 - y1), FillChar);
	//std::fill(m_bufScreen[y1 * m_nScreenWidth + x1], m_bufScreen[y2 * m_nScreenWidth + x2],FillChar);


	
	for (int x = x1; x < x2; x++)
		for (int y = y1; y < y2; y++)
			Draw(x, y, c, col);
	
}

void GameSystem::Clip(int& x, int& y)
{
	if (x < 0) x = 0;
	if (x >= m_nScreenWidth) x = m_nScreenWidth;
	if (y < 0) y = 0;
	if (y >= m_nScreenHeight) y = m_nScreenHeight;
}

void GameSystem::DrawSprite(int x1, int y1, Sprite& Object)
{
	int x2 = x1 + Object.PictureSizeX;
	int y2 = y1 + Object.PictureSizeY;
	Clip(x1, y1);
	Clip(x2, y2);

	int XR = 0;
	int YR = 0;

	//Animation Stuff
	Object.AnimationCounter += fRenderTime;
	if ((int)Object.AnimationCounter >= Object.AnimationSpeed)
	{
		Object.AnimationCounter = 0;
		Object.AnimationFrame++;
		Object.AnimationFrame = Object.AnimationFrame % Object.AnimationStates;

	}


	for (int y = y1; y < y2; y++,YR++)
	{
		for (int x = x1; x < x2; x++,XR++)
		{
			//Skip Background Colour
			if (Object.SpriteInfo[YR * Object.sizeX + XR + Object.AnimationFrame * Object.PictureSizeX].Attributes == FG_GREY)continue;

			Draw(x,y, PIXEL_SOLID, Object.SpriteInfo[YR*Object.sizeX+XR  + Object.AnimationFrame * Object.PictureSizeX].Attributes);

		}
		XR = 0;
	}
}

void GameSystem::ClearScreen()
{
	std::memcpy(m_bufScreen, m_bufScreen_blank, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);
}


//Custom Draws
void GameSystem::DrawPlayer()
{
	//Draw PlayerSprite Finally 08.04.20
	DrawSprite((int)Gracz.x - (PlayerSprite.sizeX / 2), (int)Gracz.y - (PlayerSprite.sizeY / 2), PlayerSprite);

}

void GameSystem::DrawBeam()
{
	if (Gracz.CastingBeam == 1)//Draw Spell Beam
	{
		if (rand() % 2 == 0)
		{
			DrawLine(static_cast<int>(Gracz.x), static_cast<int>(Gracz.y), m_mousePosX, m_mousePosY, 9608, FG_GREEN);
		}
		else
		{
			DrawLine(static_cast<int>(Gracz.x), static_cast<int>(Gracz.y), m_mousePosX, m_mousePosY, 9608, FG_YELLOW);
		}
	}
}

void GameSystem::DrawHUD()
{
	this->Fill(0, 0, 100, 6, 0x2588, BG_GREY);//Draw Background
	this->Fill(0, 0, (int)Gracz.Health, 3, 0x2588, FG_RED);//Draw Health
	this->Fill(0, 3, (int)Gracz.Mana, 6, 0x2588, FG_BLUE);//Draw Mana
}

void GameSystem::DrawSpiders()
{
	for (std::unique_ptr<Spider>& S : Spiders)
	{
		DrawSprite((int)S->x - 10, (int)S->y - 10, SpiderSprite);
	}
}


//Checks

void GameSystem::PassiveTick()
{
	Gracz.Regeneration(fElapsedTime);
	DamageSpiders();
	CheckMonsterHealth();

	GenerateMonsters();

}

void GameSystem::DamageSpiders()
{
	int offset = 9;
	for (std::unique_ptr<Spider>& S : Spiders)
	{
		if (Is_Between((float)m_mousePosX, S->x - offset, S->x + offset) && Is_Between((float)m_mousePosY, S->y - offset, S->y + offset) && Gracz.CastingBeam == 1)S->TakeDamage(fElapsedTime * Gracz.DamageBeam);
	}
}

void GameSystem::CheckMonsterHealth()
{

	Spiders.erase(std::remove_if(Spiders.begin(), Spiders.end(), [](std::unique_ptr<Spider>& S) {return(S->Health < 0.f); }), Spiders.end());
    
	//[](std::unique_ptr<Spider>& S) {return(S->Health < 0.f);}
}



//Helper Functions

bool GameSystem::Is_Between(float mouse, float from, float to)
{
	if (mouse >= from && mouse <= to)return true;
	else return false;
}

bool GameSystem::HealthLow(std::unique_ptr<Spider>& Spooder)
{
	return(Spooder->Health < 0.f);
}

//Random World Functions

void GameSystem::GenerateMonsters()
{
	if (SpawnMonsterTimer > 2)
	{
		SpawnMonsterTimer = 0;//Reset Spawn Timer

		int R = rand() % 100 + 0;
		if (R >= 0 && R < 100)Spiders.push_back(std::unique_ptr<Spider>(new Spider( rand() % m_nScreenWidth, rand() % m_nScreenHeight )));//Spawn Random Spider

	}
}



//
void GameSystem::GetUserInput()
{
	/*
	if (_kbhit())
	{
		int k = _getch();
		if (k == 0)k = _getch();
		switch (k)
		{
		case 72 :
			Gracz.Move(0, -1 * fElapsedTime);
			break;
		case 80 :
			Gracz.Move(0, 1 * fElapsedTime);
			break;
		case 75:
			Gracz.Move(-1 * fElapsedTime,0);
			break;
		case 77:
			Gracz.Move(1 * fElapsedTime,0);
			break;

		default:
			break;
		}

	}
	*/
	if (GetAsyncKeyState(0x57))Gracz.Move(0, -1 * fElapsedTime);
	if (GetAsyncKeyState(0x53))Gracz.Move(0, 1 * fElapsedTime);
	if (GetAsyncKeyState(0x41))Gracz.Move(-1 * fElapsedTime, 0);
	if (GetAsyncKeyState(0x44))Gracz.Move(1 * fElapsedTime, 0);
	if (GetAsyncKeyState(VK_RBUTTON))
	{
		Gracz.Beam(1,fElapsedTime);
	}
	else Gracz.Beam(0);
}

void GameSystem::HandleConsoleMouse()
{
	// Handle Mouse Input - Check for window events
	INPUT_RECORD inBuf[32];
	DWORD events = 0;
	GetNumberOfConsoleInputEvents(m_hConsoleIn, &events);
	if (events > 0)
		ReadConsoleInput(m_hConsoleIn, inBuf, events, &events);

	// Handle events - we only care about mouse clicks and movement
	// for now
	for (DWORD i = 0; i < events; i++)
	{
		switch (inBuf[i].EventType)
		{
		case FOCUS_EVENT:
		{
			m_bConsoleInFocus = inBuf[i].Event.FocusEvent.bSetFocus;
		}
		break;

		case MOUSE_EVENT:
		{
			switch (inBuf[i].Event.MouseEvent.dwEventFlags)
			{
			case MOUSE_MOVED:
			{
				m_mousePosX = inBuf[i].Event.MouseEvent.dwMousePosition.X;
				m_mousePosY = inBuf[i].Event.MouseEvent.dwMousePosition.Y;
			}
			break;

			case 0:
			{
				//for (int m = 0; m < 5; m++)
					//m_mouseNewState[m] = (inBuf[i].Event.MouseEvent.dwButtonState & (1 << m)) > 0;

			}
			break;

			default:
				break;
			}
		}
		break;

		default:
			break;
			// We don't care just at the moment
		}
	}
}


