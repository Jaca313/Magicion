#include "Game.h"


//Initializers



void GameSystem::ConstructConsole(int width, int height, int fontw, int fonth)
{
	//Ripped Code from javidx9 to create a console

	//Grab handle
	if (m_hConsole == INVALID_HANDLE_VALUE)
		return Error("Bad Handle");


	//Set some defaults
	m_nScreenWidth = width;
	m_nScreenHeight = height;


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


	//Set Console Font
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


	// Allocate memory for screen buffer(display)
	m_bufScreen = new CHAR_INFO[m_nScreenWidth * m_nScreenHeight];
	memset(m_bufScreen, 0, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);

	//Allocate memory for the second buffer(render)
	m_bufScreen_current = new CHAR_INFO[m_nScreenWidth * m_nScreenHeight];
	memset(m_bufScreen_current, 0, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);
	
	//Allocate memory for the pass buffer(shared display/render for data passthrough)
	m_bufScreen_pass = new CHAR_INFO[m_nScreenWidth * m_nScreenHeight];
	memset(m_bufScreen_pass, 0, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);


}

void GameSystem::initConsole()
{
	//Creates a Console with options from window.ini

	//Default Values
	m_nScreenWidth = 80;
	m_nScreenHeight = 30;
	int fontw = 8;
	int fonth = 8;
	m_sAppName = L"Default";


	//Grabs Setting from File
	std::wifstream ifs("Window.ini");
	if (ifs.is_open())
	{
		std::getline(ifs, m_sAppName);
		ifs >> m_nScreenWidth >> m_nScreenHeight >> fontw >> fonth;
	}
	ifs.close();


	//Creates the Console
	this->ConstructConsole(m_nScreenWidth,m_nScreenHeight, fontw, fonth);


}

void GameSystem::initSupBuffers()
{
	//Fills the render buffer with "blank" CHAR_INFO and copies it to the passthrough buffer
	ClearScreen();
	std::memcpy(m_bufScreen_pass, m_bufScreen, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);


}

void GameSystem::Timing()
{
	////Main Timing Clock
	end = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsedTime = end - start;
	start = end;
	fClockTime = elapsedTime.count();
	////Increment time in the other clocks
	fElapsedTime += fClockTime;
	fRenderTime += fClockTime;
	fBarTime += fClockTime;
	fDisplayTime += fClockTime;

	SpawnMonsterTimer += fClockTime;
	//
}

//Constructors/Destructors

GameSystem::GameSystem()
{
	m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);

	m_bEnableSound = false;

	this->initConsole();

	this->initSupBuffers();

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
		float titleAvgDisplay = 45.f / std::accumulate(AvgDisplayTime.begin(), AvgDisplayTime.end(), 0.f);
		float titleAvgRender = 90.f / std::accumulate(AvgRenderTime.begin(), AvgRenderTime.end(), 0.f);
		float titleAvgUpdate = 200.f /std::accumulate(AvgUpdateTime.begin(), AvgUpdateTime.end(), 0.f);


		wchar_t s[256];
		swprintf_s(s, 256, L"%s - Display FPS: %3.2f Render FPS: %3.2f Update FPS: %3.2f", m_sAppName.c_str(), titleAvgDisplay, titleAvgRender, titleAvgUpdate);
		SetConsoleTitle(s);

	}
}


void GameSystem::UpdateScreen()
{	   

	std::unique_lock<std::mutex> Thread_Display_Lock(DisplayGuard);
	std::memcpy(m_bufScreen_current, m_bufScreen_pass, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);
	Thread_Display_Lock.unlock();

	WriteConsoleOutput(m_hConsole, m_bufScreen_current, { (short)m_nScreenWidth, (short)m_nScreenHeight }, { 0, 0 }, &m_rectWindow);


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


		GetUserInput();

		PassiveTick();

		for (std::unique_ptr<Spider>& S : Spiders)
		{
			S->Tick(Gracz.x, Gracz.y);
			if (S->x < 0 || S->x > m_nScreenWidth)S->ReverseDirection(1, 0);
			if (S->y < 0 || S->y > m_nScreenHeight)S->ReverseDirection(0, 1);
		}
		fElapsedTime = 0.f;
	}
}

void GameSystem::render()
{
	if (fRenderTime > 1.f / 90.f)
	{
	AvgRenderTime[RenderArrayCurrent] = fRenderTime;
	RenderArrayCurrent = (++RenderArrayCurrent)%90;


	ClearScreen();


	DrawSpiders();
	DrawPlayer();
	DrawBeam();
	DrawHUD();

	std::unique_lock<std::mutex> Thread_Display_Lock2(DisplayGuard);
	std::memcpy(m_bufScreen_pass, m_bufScreen, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);
	Thread_Display_Lock2.unlock();



	fRenderTime = 0.f;
	}
}



void GameSystem::run()
{
	Tool.LOG("This is the Start of the Main Loop. Enjoy!");

	Tool.LOG("Now Playing Tension.wav");
	PlaySound(TEXT("Tension.wav"),NULL, SND_FILENAME | SND_ASYNC | SND_LOOP | SND_NOSTOP);


	while (this->Exit != 1)
	{
		std::thread t2(&GameSystem::HandleConsole,this);
		while (this->Exit != 1)
		{
			this->Timing();
			this->UpdateBar();

			this->update();
			this->render();

			//if (DisplayRefresh > 1.f / DesiredDisplayRate)
			//{
			//	DisplayRefresh = 0.f;

			//	auto i = std::async(std::launch::async, &GameSystem::UpdateScreen,this);
			//	//t1.detach();
			//}
			//this->UpdateScreen();

		}
		t2.join();
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

	for (int y = y1; y < y2; y++)
	{
		memfill(&m_bufScreen[y * m_nScreenWidth + x1], (x2 - x1) * sizeof(CHAR_INFO), &FillChar, sizeof(CHAR_INFO));
	}


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
	int DifX = x1;
	int DifY = y1;

	int x2 = x1 + Object.PictureSizeX;
	int y2 = y1 + Object.PictureSizeY;

	Clip(x1, y1);
	Clip(x2, y2);

	DifX = x1 - DifX;
	DifY = y1 - DifY;

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
			if (Object.SpriteInfo[(YR + DifY) * Object.sizeX + (XR+DifX) + Object.AnimationFrame * Object.PictureSizeX].Attributes == FG_GREY)continue;

			Draw(x,y, PIXEL_SOLID, Object.SpriteInfo[(YR+DifY)*Object.sizeX+ (XR + DifX) + Object.AnimationFrame * Object.PictureSizeX].Attributes);

		}
		XR = 0;
	}
}

void GameSystem::ClearScreen()
{
	//10 microsec

	CHAR_INFO FillsChar;
	FillsChar.Char.UnicodeChar = PIXEL_SOLID;
	FillsChar.Attributes = FG_GREY;

	memfill(&m_bufScreen[0], m_nScreenWidth * m_nScreenHeight * sizeof(CHAR_INFO), &FillsChar, sizeof(CHAR_INFO));

	//15 microsec
	//CHAR_INFO FillChar;
	//FillChar.Char.UnicodeChar = PIXEL_SOLID;
	//FillChar.Attributes = FG_GREY;
	//memcopyfill(&m_bufScreen[0], m_nScreenWidth * m_nScreenHeight * sizeof(CHAR_INFO), &FillChar, sizeof(CHAR_INFO));

	//40 microsec
	//std::memcpy(m_bufScreen, m_bufScreen_blank, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);

	//70 microsec
	//this->Fill(0, 0, m_nScreenWidth, m_nScreenHeight, 0x2588, FG_GREY);//Clear Screen
}

int GameSystem::memcopyfill(void* const pToFill, const size_t pToFillSize, const void* const pFillWith, const size_t pFillWithSize)
{
	if (pToFill == NULL || pToFillSize == 0) {
		return 0; //Nothing to do.
	}
	if (pFillWith == NULL || pFillWithSize == 0) {
		return 1; //ERROR! Something to fill and nothing to fill it with.
	}
	if (pToFillSize <= pFillWithSize) {
		memcpy(pToFill, pFillWith, pToFillSize);
		return 0; //Complete in one (possibly short) copy.
	}
	//The to buffer is bigger so we start with a full copy of pattern.
	memcpy(pToFill, pFillWith, pFillWithSize);

	//Now we keep doubling the copies by copying and copying from the target onto itself.
	char* lFillFrom = ((char*)pToFill) + pFillWithSize;
	size_t lFilledSoFar = pFillWithSize;
	char* lFillEnd = ((char*)pToFill) + pToFillSize;

	while (lFilledSoFar < (lFillEnd - lFillFrom)) {//Overflow safe.
		memcpy(lFillFrom, pToFill, lFilledSoFar);
		lFillFrom += lFilledSoFar;
		lFilledSoFar = lFilledSoFar << 1;//Doubling....
	}
	//No we fill the rest of the buffer. 
	memcpy(lFillFrom, pToFill, pToFillSize - lFilledSoFar);
	return 0;//That went well!
}

int GameSystem::memfill(CHAR_INFO* const pToFill, const size_t pToFillSize, const CHAR_INFO* const pFillWith, const size_t pFillWithSize)
{
	int max_offset = pToFillSize / pFillWithSize;

	for (CHAR_INFO* p = pToFill; p < pToFill + max_offset; p++)
	{
		*p = *pFillWith;
	}
	   	  
	return 0;
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
		/*
		if (rand() % 2 == 0)
		{
			DrawLine(static_cast<int>(Gracz.x), static_cast<int>(Gracz.y), m_mousePosX, m_mousePosY, 9608, FG_GREEN);
		}
		else
		{
			DrawLine(static_cast<int>(Gracz.x), static_cast<int>(Gracz.y), m_mousePosX, m_mousePosY, 9608, FG_YELLOW);
		}
		*/
		DrawLine(static_cast<int>(Gracz.x), static_cast<int>(Gracz.y), m_mousePosX, m_mousePosY, 9608, rand()%16);
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
	int offset = 15;
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
	if (SpawnMonsterTimer > 0.02f && Spiders.size() < 20)
	{
		SpawnMonsterTimer = 0;//Reset Spawn Timer

		int R = rand() % 100 + 0;
		if (R >= 0 && R < 100)
		{
			Spiders.push_back(std::unique_ptr<Spider>(new Spider(rand() % m_nScreenWidth, rand() % m_nScreenHeight)));//Spawn Random Spider
		}
	}
}



//
void GameSystem::GetUserInput()
{
	std::unique_lock <std::mutex> MouseGuard(MouseLock);
	m_mousePosX = buf_m_mousePosX;
	m_mousePosY = buf_m_mousePosY;
	MouseGuard.unlock();

	if (GetAsyncKeyState(0x1B))Exit = true;
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

void GameSystem::HandleConsole()
{
	while (Exit != 1)
	{
		HandleConsoleMouse();
		UpdateScreen();
	}
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
	std::unique_lock<std::mutex> MouseGuard(MouseLock);
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
				buf_m_mousePosX = inBuf[i].Event.MouseEvent.dwMousePosition.X;
				buf_m_mousePosY = inBuf[i].Event.MouseEvent.dwMousePosition.Y;
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
	MouseGuard.unlock();

}


