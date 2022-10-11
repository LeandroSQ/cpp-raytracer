#pragma once
#include "color.hpp"
#include "math.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include "world.hpp"
#include <SDL2/SDL.h>
#include <iostream>

struct Size {
	int width = 0;
	int height = 0;

	Size() : width(0), height(0) { }
	Size(int width, int height) : width(width), height(height) { }
};

#define ENABLE_HIGHDPI
#define MOUSE_MOVE_LIGHT
// #define MOUSE_MOVE_SPHERE
#define DEBUG_FRAME_TIME

#define PIN std::cout << __FILE__ << ":" << __LINE__ << "    " << __func__ << std::endl;
#define SDL_HandleError(msg) std::cout << msg << " " << SDL_GetError() << std::endl; return -1;

class Engine {
  public:
	// Viewport
	Size viewport = Size(800, 450);

	bool isGammaCorrectionEnabled = true;

	bool isRunning = true;

	World world;

  private:
	// SDL
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* frameBuffer = nullptr;
	void* pixels = nullptr;
    int pitch = 0;

	// Fps variables
	double intervalBetweenFrames = 33.33;
	uint16_t fps = 0;
	uint16_t frameCounter = 0;
	uint64_t lastFrameTime = 0L;
	double deltaTime = 0.0;

	// Viewport
	Size virtualViewport = viewport;
	float aspectRatio = float(viewport.width) / float(viewport.height);

  public:
	Engine() { }

	~Engine() {
		// Free resources
		SDL_DestroyTexture(frameBuffer);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		// Quit
		SDL_Quit();
	}

	int init() {
		// Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0) {
			SDL_HandleError("SDL could not initialize!");
		}

		// Create the SDL window
		if (createWindow() < 0) return -1;

		// Update window sizings
		SDL_GetWindowSize(window, &viewport.width, &viewport.height);
		SDL_GL_GetDrawableSize(window, &virtualViewport.width, &virtualViewport.height);
		aspectRatio = float(viewport.width) / float(viewport.height);

        // Create frame buffer
        if (createFrameBuffer() < 0) return -1;

        return 0;
	}

	void loop() {
		// Variables
		double frameTimer = 0.0;
		double fpsTimer = 0.0;

		// Initialize the last frame time, so delta time won't start at zero
		lastFrameTime = SDL_GetPerformanceCounter();

		// Main loop
		while (isRunning) {
			// Delay
			SDL_Delay(0);

			// Handle incoming events
			handleEvents();

			// Calculate the delta time
			uint64_t currentFrameTime = SDL_GetPerformanceCounter();
			deltaTime = ((currentFrameTime - lastFrameTime) * 1000 / (double)SDL_GetPerformanceFrequency());

			// Estimate frame rate
			fpsTimer += deltaTime;
			if (fpsTimer >= 1000.0) {
				fpsTimer -= 1000.0;
				fps = frameCounter;
				frameCounter = 0;
			}

			// Handle internal timer
			frameTimer += deltaTime;
			if (frameTimer >= intervalBetweenFrames) {
				frameTimer -= intervalBetweenFrames;

				onFrame();

                // Print out the time elapsed to compose this frame
#ifdef DEBUG_FRAME_TIME
			std::cout.precision(2);
			std::cout << std::fixed
					  << (SDL_GetPerformanceCounter() - currentFrameTime) * 1000 / (double)SDL_GetPerformanceFrequency()
					  << "ms" << std::endl;
#endif
			}

			lastFrameTime = SDL_GetPerformanceCounter();
		}
	}

  private:
	int createWindow() {
		// Define the window flags
		int flags = SDL_WINDOW_SHOWN;
#ifdef ENABLE_HIGHDPI
		flags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif

		// Create window
		window = SDL_CreateWindow(
			"Raytracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, viewport.width, viewport.height, flags
		);

		if (window == nullptr) {
			SDL_HandleError("SDL could not create Window!");
		}

		// Create the renderer
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (renderer == nullptr) {
			SDL_HandleError("SDL could not create Renderer!");
		}

        return 0;
	}

	int createFrameBuffer() {
		// Create the texture
		frameBuffer = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			virtualViewport.width,
			virtualViewport.height
		);

		if (frameBuffer == nullptr) {
			SDL_HandleError("SDL could not create Texture!");
		}

        return 0;
    }

	void handleEvents() {
		// Fetch the next event in queue
		SDL_Event event;
		SDL_PollEvent(&event);

		switch (event.type) {
			/* Window closed */
			case SDL_QUIT:
				isRunning = false;
				break;

			/* Mouse event */
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
				onMouseEvent();
				break;
		}
	}

	void onMouseEvent() {
		// Get the current mouse status
		int x = 0, y = 0;
		uint32_t buttonState = SDL_GetMouseState(&x, &y);
		// std::cout << "Mouse: " << x << ", " << y << std::endl;

		// If the mouse is clicked, toggle gamma correction
		if ((buttonState & SDL_BUTTON_LMASK) != 0) {
			isGammaCorrectionEnabled = !isGammaCorrectionEnabled;
			std::cout << "Gamma correction: " << (isGammaCorrectionEnabled ? "ON" : "OFF") << std::endl;
		}

		// Localize to screen UV coordinates [0.0 to 1.0]
		float nX = x / float(viewport.width), nY = y / float(viewport.height);

#ifdef MOUSE_MOVE_LIGHT
		world.light.x = (1.0f - nX) * 4.0f - 2.0f;
		world.light.y = (1.0f - nY) * 4.0f - 2.0f;
		// std::cout << "Light: " << world.light.x << ", " << world.light.y << std::endl;
#endif

#ifdef MOUSE_MOVE_SPHERE
		Sphere sphere = world.spheres.at(0);
		sphere.position.x = nX * 2.0f - 1.0f;
		sphere.position.y = nY * 2.0f - 1.0f;
		// std::cout << "Sphere: " << sphere.position.x << ", " << sphere.position.y << std::endl;
#endif
	}

	void onFrame() {
		// Lock the texture and acquire the pixel data
		SDL_LockTexture(frameBuffer, nullptr, &pixels, &pitch);

		// Increment rendered frames count
		onRender();
		frameCounter++;

		SDL_UnlockTexture(frameBuffer);

		// Clear the back buffer
		SDL_RenderClear(renderer);
		// Copy the texture into the back buffer
		SDL_RenderCopy(renderer, frameBuffer, nullptr, nullptr);

		// Display the back buffer into the window
		SDL_RenderPresent(renderer);
	}

	void onRender() {
		for (int y = 0; y < virtualViewport.height; y++) {
			for (int x = 0; x < virtualViewport.width; x++) {
				// Calculate the UV coordinates [0.0 to 1.0]
				float u = (float(x) / float(virtualViewport.width)) * 2.0f - 1.0f;
				float v = (float(y) / float(virtualViewport.height)) * 2.0f - 1.0f;

				// Maintain the aspect ratio
				u *= aspectRatio;

				Ray ray(world.camera.origin, Vector3(u, v, -1.0f));

				bool hasHitAnObject = false;
				for (Sphere sphere : world.spheres) {
					Vector3 center = ray.origin - sphere.position;

					float a = Vector3::dot(ray.direction, ray.direction);
					float b = 2.0f * Vector3::dot(center, ray.direction);
					float c = Vector3::dot(center, ray.origin) - sphere.radius * sphere.radius;
					float discriminant = b * b - 4 * a * c;

					if (discriminant >= 0) {
						float t = (-b - sqrtf(discriminant)) / (2.0f * a);

						// Calculate the hit position and hit surface normal
						Vector3 hitPosition = ray.origin + ray.direction * t;
						Vector3 normal = Vector3::normalize(hitPosition /*  - sphere.position */);

						// Calculate basic normal shading
						float light = max(Vector3::dot(normal, -world.light), 0.0f);
						Color color = sphere.color * light;

						// Applies gamma correction
						if (isGammaCorrectionEnabled) {
							color = Color::pow(color, 1.0f / 2.2f);
						}

						// Set the pixel
						setPixel(x, y, color);

						// Notify that one object was hit
						hasHitAnObject = true;

						// Already hit one sphere, breaks the for-loop
						break;
					}
				}

				// If none object was hit, paint a sky gradient
				if (!hasHitAnObject) {
					float gradient = v * 1.3;
					Color color = Color::mix(Color(0.5f, 0.7f, 1.0f), Color(1.0, 1.0, 1.0), gradient);
					setPixel(x, y, color);
				}
			}
		}
	}

	void setPixel(int x, int y, Color color) {
		constexpr int channels = 4;

		// Convert 2d positions to 1d offset
		uint32_t offset = (y * virtualViewport.width + x) * (sizeof(uint8_t) * channels);
		// uint32_t offset = (y * pitch) + (x * sizeof(uint8_t) * channels);
        uint8_t* pointer = (uint8_t*) pixels;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		// ARGB
		pointer[offset + 0] = 255;
		pointer[offset + 1] = (uint8_t)(color.red * 255.0);
		pointer[offset + 2] = (uint8_t)(color.green * 255.0);
		pointer[offset + 3] = (uint8_t)(color.blue * 255.0);
#else
		// BGRA
		pointer[offset + 0] = (uint8_t)(color.blue * 255.0);
		pointer[offset + 1] = (uint8_t)(color.green * 255.0);
		pointer[offset + 2] = (uint8_t)(color.red * 255.0);
		pointer[offset + 3] = 255;
#endif
	}
};