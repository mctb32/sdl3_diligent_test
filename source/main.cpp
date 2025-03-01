#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <iostream>
#include "diligent_sample.h"

int main(int argc, char *argv[]) 
{
	RENDER_DEVICE_TYPE device = RENDER_DEVICE_TYPE_VULKAN;

	// Initialize the SDL video subsystem.
	if (SDL_Init(SDL_INIT_VIDEO) < 0) 
	{
		std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
		return 1;
	}

	// Create the window
	SDL_Window *window = SDL_CreateWindow("SdlSandbox", 0,0,800, 600, device == RENDER_DEVICE_TYPE_VULKAN ? SDL_WINDOW_VULKAN : 0);
	if (!window) 
	{
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
		SDL_Quit();
		return 1;
	}

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);

	DiligentSample diligent(device, info.info.win.window);
	diligent.InitPipeline();

	// Main loop
	bool running = true;
	SDL_Event event;
	while (running) 
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				running = false;
			else if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
					running = false;
			}
		}

		diligent.Render();
		SDL_Delay(10);
	}

	// Clean up resources.
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}