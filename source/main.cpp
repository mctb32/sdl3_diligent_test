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
// 	SDL_Window *window = SDL_CreateWindow("SdlSandbox", 800, 600, device == RENDER_DEVICE_TYPE_VULKAN ? SDL_WINDOW_VULKAN : 0);
// 	if (!window) 
// 	{
// 		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
// 		SDL_Quit();
// 		return 1;
// 	}

	SDL_PropertiesID props = SDL_CreateProperties();
	if (props == 0) {
		SDL_Log("Unable to create properties: %s", SDL_GetError());
		return 1;
	}

	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "SdlSandbox");
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, false);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_ALWAYS_ON_TOP_BOOLEAN, false);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, false);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_MAXIMIZED_BOOLEAN, false);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_MINIMIZED_BOOLEAN, false);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, false);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, false);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER,800);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 600);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
	SDL_Window *window = SDL_CreateWindowWithProperties(props);

	DiligentSample diligent(device,	SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL));
	diligent.InitPipeline();

	std::cout << "Running...\n";

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
		//std::cout << "Next frame...\n";
	}

	// Clean up resources.
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}