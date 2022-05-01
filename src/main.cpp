#include <stdio.h>
#include <string>
#include <iostream>

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <emscripten/websocket.h>
#else
// #include "httplib.h"
#include <SDL.h>
#include <SDL_image.h>
#endif

#undef main // Needed for windows.
#if __EMSCRIPTEN__

EM_JS(int, canvas_get_width, (), {
	return canvas.width;
});

EM_JS(int, canvas_get_height, (), {
	return canvas.height;
});

EM_JS(void, call_alert, (), {
	console.log('It bounced');
});

EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData)
{
	puts("onopen");

	EMSCRIPTEN_RESULT result;
	result = emscripten_websocket_send_utf8_text(websocketEvent->socket, "hoge");
	if (result)
	{
		printf("Failed to emscripten_websocket_send_utf8_text(): %d\n", result);
	}
	return EM_TRUE;
}
EM_BOOL onerror(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData)
{
	puts("onerror");

	return EM_TRUE;
}
EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData)
{
	puts("onclose");

	return EM_TRUE;
}
EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData)
{
	puts("onmessage");
	if (websocketEvent->isText)
	{
		// For only ascii chars.
		printf("message: %s\n", websocketEvent->data);
	}

	EMSCRIPTEN_RESULT result;
	result = emscripten_websocket_close(websocketEvent->socket, 1000, "no reason");
	if (result)
	{
		printf("Failed to emscripten_websocket_close(): %d\n", result);
	}
	return EM_TRUE;
}

#endif

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Event *event = NULL;
SDL_Texture *circle = NULL;
SDL_Rect textureRect;
SDL_Rect positionRect;

bool quit = false;
bool falling = true;

int SCREEN_WIDTH = 640;	 // 640;
int SCREEN_HEIGHT = 480; // 480;

double dt = 10;						 // The interval between updating the physics. IE update physics every 100th of a second
double currentTime = SDL_GetTicks(); // in miliseconds
double accumulator = 0.0;			 // This will hold the accumulation of physics steps (any time left over if the graphics renders faster than the physics simulates)

double velocity = 1;

void sendMessage();

SDL_Texture *loadTexture(const std::string &file, SDL_Renderer *ren)
{
	SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
	if (texture == nullptr)
	{
		std::cout << "Could not load texture" << std::endl;
		std::cout << IMG_GetError() << std::endl;
	}
	return texture;
}

void init()
{

	// Start up SDL and make sure it went ok
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		throw("SDL failed to initialise");
	}

	window = SDL_CreateWindow("SDL2 Example!!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (window == nullptr)
	{
		SDL_Quit();
		throw("Failed to create window");
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!SDL_RenderSetLogicalSize(renderer, 640, 480))
	{
		std::cout << SDL_GetError() << std::endl;
	}

	if (renderer == nullptr)
	{
		window = NULL;
		SDL_Quit();
		throw("Failed to create renderer");
	}

	event = new SDL_Event();

	circle = loadTexture("resources/example_texture.png", renderer);

	// This rectangle represents where from the circles.png we should grab the texture.
	// X, Y, W, H means that we grab 100 x 100 pixels from the top left of the target image (when we use SDL_RenderCopy)
	textureRect = {0, 0, 100, 100};

	// This represents where on the screen we will put the circle texture and it's size,
	// this will initialise it at the top left and the image will be squished to 15 x 15
	positionRect = {(640 / 2) - 7, // X position - this is overcomplicated but it just puts the circle in the center of the screen.
					0,			   // Y position - sets the circle at the top of the screen
					15,			   // Sets the height of the circle
					15};		   // Sets the weidth of the circle
}

void input()
{
	if (event->type == SDL_QUIT)
	{
		quit = true;
	}

	if (event->window.event == SDL_WINDOWEVENT_RESIZED)
	{
		SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
		std::cout << "The window was resized: " << SCREEN_WIDTH << std::endl;
	}

	if (event->type == SDL_KEYDOWN)
	{
		switch (event->key.keysym.sym)
		{
		case SDLK_KP_A:

			break;
		case SDLK_KP_D:

			break;
		default:
			break;
		}
	}
}

void update(double dt)
{

	if (positionRect.y <= 0)
	{
		falling = true;
	//	sendMessage();
	};

	if (positionRect.y >= (480 - positionRect.h))
	{
		falling = false;
	//	sendMessage();
	};

	if (falling)
	{
		positionRect.y += velocity * dt;
	}
	else
	{

		positionRect.y -= velocity * dt;
	}
}

void render()
{
	// Sets a background color for the scene
	SDL_SetRenderDrawColor(renderer, 91, 10, 145, 255);

	// clears previous frame.
	SDL_RenderClear(renderer);

	// Set up the circle on the next render frame.
	SDL_RenderCopy(renderer, circle, &textureRect, &positionRect);

	// Renders current frame.
	SDL_RenderPresent(renderer);
}

void mainLoop()
{
	double newTime = SDL_GetTicks();		  // in miliseconds
	double frameTime = newTime - currentTime; // Essentially stores how long the previous frame ran for in miliseconds

	// limits frame time to 100th of a second
	if (frameTime > 250)
	{
		std::cout << "UPPER BOUND HIT, LAG ENCOUNTERED" << std::endl;
		frameTime = 250; // Upper bound on the time between processing this loop. If physics simulation is slower than render calculation then the game could halt.
	}

	// #if __EMSCRIPTEN__
	// int canvasWidth = canvas_get_width();
	// std::cout << "The canvas width was: " << canvasWidth << std::endl;
	// #endif

	currentTime = newTime;

	accumulator += frameTime;

	while (accumulator >= dt)
	{
		update(dt); // consumes dt
		accumulator -= dt;
	};

	render(); // Produces dt (takes time to calculate)

	// Event Polling
	while (SDL_PollEvent(event))
	{
		input();
	}
}

// #if !__EMSCRIPTEN__
// void sendMessage()
// {
// 	httplib::Client cli("localhost", 8000);

// 	if (auto res = cli.Get("/api/v1/test1"))
// 	{
// 		if (res->status == 200)
// 		{
// 			std::cout << "Response was 200" << std::endl;
// 			std::cout << res->body << std::endl;
// 		}
// 		else
// 		{
// 			std::cout << "Something else happened" << std::endl;
// 		}
// 	}
// 	else
// 	{
// 		auto err = res.error();
// 		std::cout << "Server response is: " << err << std::endl;
// 	}
// };

// #endif

#if __EMSCRIPTEN__

void sendMessage()
{
	call_alert();
};

void setupWebSocket()
{
	if (!emscripten_websocket_is_supported())
	{
		std::cout << "WebSockets are not supported by this browser." << std::endl;
		return;
	}
	EmscriptenWebSocketCreateAttributes ws_attrs = {
		"wss://echo.websocket.org",
		NULL,
		EM_TRUE};

	EMSCRIPTEN_WEBSOCKET_T ws = emscripten_websocket_new(&ws_attrs);
	emscripten_websocket_set_onopen_callback(ws, NULL, onopen);
	emscripten_websocket_set_onerror_callback(ws, NULL, onerror);
	emscripten_websocket_set_onclose_callback(ws, NULL, onclose);
	emscripten_websocket_set_onmessage_callback(ws, NULL, onmessage);
};
#endif

int main(int, char **)
{
	init();

	// sendMessage();
// When creating a native app (.exe on windows or sh on OSX/Linux this will directly call mainLoop. when running in browser emscripten deals with calls to the main method)
#if __EMSCRIPTEN__
	emscripten_set_main_loop(mainLoop, -1, 1);
	setupWebSocket();
#else
	while (quit != true)
	{
		mainLoop();
	}
#endif

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	renderer = NULL;
	window = NULL;
	IMG_Quit();
	SDL_Quit();

	return 0;
}