#include <iostream>
#include "Time.h"

int main()
{
	std::cout << "Welcome to the Light Vox Engine!" << std::endl;
	auto time = Time::GetInstance();
	time->Init();
	while (true)
	{
		time->UpdateTimer();
		std::cout << time->getTotalTimeFloat() << std::endl;
	}
	time->ReleaseInstance();
	return 0;
}