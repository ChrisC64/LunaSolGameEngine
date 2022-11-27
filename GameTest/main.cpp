#include "LSEFramework.h"

import Platform.Win32Window;

int main()
{
	std::cout << "Hello Luna Sol Game Engine!\n";

	Ref<LS::LSWindowBase> window = LS::LSCreateWindow(800u, 700u, L"Hello App");
	window->Show();
	while (window->IsRunning())
	{
		window->PollEvent();
	}
}