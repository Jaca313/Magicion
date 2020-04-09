#include "Sprite.h"

Sprite::Sprite(std::string PPM_File, unsigned short AnimationStates, int sizeX, int sizeY)
{
	this->sizeX = sizeX;
	this->sizeY = sizeY;

	this->AnimationStates = AnimationStates;
	this->AnimationFrame = 0;


	SpriteInfo = nullptr;

	LoadASCIIPPM(PPM_File);
	ParseSpriteInfo();

	PictureSizeX = this->sizeX / AnimationStates;
	PictureSizeY = this->sizeY;
}

void Sprite::LoadASCIIPPM(std::string PPM_File)
{
	std::wstring INFO;
	std::wstring COMMENT;
	std::wstring Colors;
	std::wifstream Plik(PPM_File);
	if (Plik.is_open())
	{
		std::getline(Plik, INFO);
		std::getline(Plik, COMMENT);
		float r = 0, g = 0, b = 0;
		Plik >> sizeX >> sizeY;
		Plik >> Colors;

		// Allocate memory for sprite buffer		
		SpriteInfo = new CHAR_INFO[sizeX * sizeY];
		memset(SpriteInfo, 0, sizeof(CHAR_INFO) * sizeX * sizeY);
		//

		for (int i = 0; i < sizeX * sizeY; i++)
		{
            //Read RGB data from 3 lines
			Plik >> r >> g >> b;
			//Convert it to a type that can be displayed in console
			SpriteInfo[i].Attributes = ConvertToConsoleChar(r, g, b);
		}

	}
	else std::cout << "Error reading PPMFILE %S at SPRITE \n", PPM_File;
	Plik.close();
}

WORD Sprite::ConvertToConsoleChar(float R, float G, float B)
{
	WORD ReturnColour = 0;
	/* Old Threshold Conversion now dedicated sprites
	float Threshold = 450.f;
	float T1 = 70;
	float Total = R + G + B;
	if (Total > Threshold)ReturnColour += 8;
	if ((R / Total) >= 0.32f )ReturnColour += 4;
	if ((G / Total) >= 0.32f )ReturnColour += 2;
	if ((B / Total) >= 0.32f )ReturnColour += 1;
	return ReturnColour;
	*/

	if (R == 139 || R == 169 || R == 255)ReturnColour += 4;
	if (G == 139 || G == 169 || G == 255)ReturnColour += 2;
	if (B == 139 || B == 169 || B == 255)ReturnColour += 1;
	if (R == 169 || G == 169 || B == 169)return ReturnColour;
	if (R == 128 || G == 128 || B == 128)ReturnColour = 8;
	if (R > 139 || G > 139 || B > 139)ReturnColour += 8;
	return ReturnColour;
}

void Sprite::ParseSpriteInfo()
{
	std::wofstream Plik("SpriteDebug.txt");

	for (int i = 0; i < sizeY; i++)
	{
		for (int ii = 0; ii < sizeX; ii++)
			Plik << SpriteInfo[i*sizeY+ii].Attributes << ",";
		Plik << std::endl;
	}
	Plik.close();
}



