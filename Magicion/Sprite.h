#pragma once
#include <windows.h>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iostream>
#include <string>

class Sprite
{
public:
	int sizeX;
	int sizeY;
	int PictureSizeX;
	int PictureSizeY;

	Sprite(std::string PPM_File,unsigned short AnimationStates = 1, int sizeX = 1, int sizeY = 1);

	//Information about Sprite
	CHAR_INFO* SpriteInfo;
	unsigned short AnimationStates;
	unsigned int AnimationFrame;
	int AnimationSpeed = 2;
	float AnimationCounter = 0;

	void LoadASCIIPPM(std::string PPM_File);
	WORD ConvertToConsoleChar(float R, float G, float B);
	void ParseSpriteInfo();
};

