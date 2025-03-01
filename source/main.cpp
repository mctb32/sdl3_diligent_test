#include <SDL3/SDL.h>
#include <iostream>
#include "diligent_sample.h"

int main(int argc, char *argv[]) 
{
	RENDER_DEVICE_TYPE device = RENDER_DEVICE_TYPE_VULKAN;

	// Initialize the SDL video subsystem.
	if (!SDL_Init(SDL_INIT_VIDEO)) 
	{
		std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
		return 1;
	}

	// Create the window
	SDL_Window *window = SDL_CreateWindow("SdlSandbox", 800, 600, device == RENDER_DEVICE_TYPE_VULKAN ? SDL_WINDOW_VULKAN : 0);
	if (!window) 
	{
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
		SDL_Quit();
		return 1;
	}

	DiligentSample diligent(device,	SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL));
	diligent.InitPipeline();

	// Main loop
	bool running = true;
	SDL_Event event;
	while (running) 
	{
		while (SDL_PollEvent(&event)) 
		{
			if (event.type == SDL_EVENT_QUIT) 
				running = false;
			else if (event.type == SDL_EVENT_KEY_DOWN) 
			{
				if (event.key.key == SDLK_ESCAPE) 
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