#include "color.hpp"
#include "math.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include "world.hpp"
#include <SDL2/SDL.h>
#include <iostream>

// Definitions
// #define DEBUG_FRAME_TIME
#define MOUSE_MOVE_LIGHT
// #define MOUSE_MOVE_SPHERE

// SDL Variables
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

// Frame time variables
uint64_t lastFrameTime = 0;
double deltaTime = 0;

// Viewport variables
int width = 400, height = 225;

// Misc
World world;

void setPixel(int x, int y, float r, float g, float b) {
	SDL_SetRenderDrawColor(renderer, (uint8_t)(r * 255.0), (uint8_t)(g * 255.0), (uint8_t)(b * 255.0), 255);
	SDL_RenderDrawPoint(renderer, x, y);
}

inline void setPixel(int x, int y, Color color) {
	setPixel(x, y, color.red, color.green, color.blue);
}

inline void onFrame() {
	float aspectRatio = float(width) / float(height);
	SDL_GL_GetDrawableSize(window, &width, &height);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			// Calculate the UV coordinates [0.0 to 1.0]
			float u = (float(x) / float(width)) * 2.0f - 1.0f;
			float v = (float(y) / float(height)) * 2.0f - 1.0f;

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

void onMouseMove() {
	// Get the current mouse status
	int x = 0, y = 0;
	SDL_GetMouseState(&x, &y);
	// std::cout << "Mouse: " << x << ", " << y << std::endl;

	// Fetch the window size, this is needed in case of a Retina/HighDPI display
	SDL_GetWindowSize(window, &width, &height);

	// Localize to screen UV coordinates [0.0 to 1.0]
	float nX = x / float(width), nY = y / float(height);

#ifdef MOUSE_MOVE_LIGHT
	world.light.x = (1.0f - nX) * 4.0f - 2.0f;
	world.light.y = (1.0f - nY) * 4.0f - 2.0f;
	world.spheres.at(0).color = Color(1.0f, 0.0f, 0.0f);
	// std::cout << "Light: " << world.light.x << ", " << world.light.y << std::endl;
#endif

#ifdef MOUSE_MOVE_SPHERE
	Sphere sphere = world.spheres.at(0);
	sphere.position.x = nX * 2.0f - 1.0f;
	sphere.position.y = nY * 2.0f - 1.0f;
	// std::cout << "Sphere: " << sphere.position.x << ", " << sphere.position.y << std::endl;
#endif
}

int main() {
	SDL_Event event;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "SDL could not initialize! " << SDL_GetError() << std::endl;
		return -1;
	}

	// Create window
	window = SDL_CreateWindow(
		"Raytracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_ALLOW_HIGHDPI
	);
	if (window == nullptr) {
		std::cout << "SDL could not create a Window! " << SDL_GetError() << std::endl;
		return -1;
	}

	// Size the window
	// SDL_SetWindowSize(window, width, height);
	// SDL_GetWindowSize(window, &width, &height);
	SDL_GL_GetDrawableSize(window, &width, &height);

	// Create the renderer
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == nullptr) {
		std::cout << "SDL could not create a Renderer! " << SDL_GetError() << std::endl;
		return -1;
	}

	// Main loop
	while (true) {
		// Wait
		SDL_Delay(33);
		SDL_PollEvent(&event);

		// Handle window close event
		if (event.type == SDL_QUIT) break;

		// Handle mouse events
		if (event.type == SDL_MOUSEMOTION) onMouseMove();

		// Clears the screen
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		// Calculate the delta time
		uint64_t currentFrameTime = SDL_GetPerformanceCounter();
		deltaTime = ((currentFrameTime - lastFrameTime) * 1000 / (double)SDL_GetPerformanceFrequency());
		lastFrameTime = currentFrameTime;

		// Paints the back buffer
#ifdef DEBUG_FRAME_TIME
		long start = SDL_GetTicks64();
		onFrame();
		std::cout << (SDL_GetTicks64() - start) << "ms" << std::endl;
#else
		onFrame();
#endif

		// Display back buffer
		SDL_RenderPresent(renderer);
	}

	// Free resources
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	// Quit
	SDL_Quit();
	return 0;
}