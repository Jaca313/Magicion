#include "Info.h"


char* Info::GetTime()
{
	auto fTime = std::chrono::system_clock::now();
	std::time_t Time = std::chrono::system_clock::to_time_t(fTime);
	return std::ctime(&Time);
}

void Info::LOG(std::string message)
{
	INFO << "///////////////////////////////////////////////////" <<std::endl;
	INFO << GetTime();// << std::endl;
	INFO << message << std::endl;
}
