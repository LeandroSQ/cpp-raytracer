#include "color.hpp"
#include "math.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include "world.hpp"
#include <SDL2/SDL.h>
#include <iostream>

// Definitions
#define ENABLE_HIGHDPI
#define DEBUG_FRAME_TIME
#define MOUSE_MOVE_LIGHT
// #define MOUSE_MOVE_SPHERE

#define SDL_HandleError(msg)                                                                                           \
 std::cout << msg << " " << SDL_GetError() << std::endl;                                                               \
 return -1;
#define PIN std::cout << __FILE__ << ":" << __LINE__ << "  -  " << __func__ << std::endl;

// SDL Variables
SDL_Window* window = nullptr;
SDL_Surface* surface = nullptr;
SDL_Texture* texture = nullptr;

void* pixels = nullptr;
int pitch = 0;

// Frame time variables
uint64_t lastFrameTime = 0;
double deltaTime = 0;

// Viewport variables
int width = 400, height = 225;

// Misc
World world;

void setPixel(int x, int y, float r, float g, float b) {
	constexpr int channels = 4;

	// Convert 2d positions to 1d ofsset
    PIN;
	uint32_t offset = (y * pitch) + (x * sizeof(uint8_t) * channels);
    PIN;
	// Get a pointer to the first element in the array
    PIN;
	uint8_t* pointer = (uint8_t*)pixels;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	// ARGB
	pointer[ offset + 0 ] = 255;
	pointer[ offset + 1 ] = (uint8_t)(r * 255.0);
	pointer[ offset + 2 ] = (uint8_t)(g * 255.0);
	pointer[ offset + 3 ] = (uint8_t)(b * 255.0);
#else
	// BGRA
    PIN;
	pointer[ offset + 0 ] = (uint8_t)(b * 255.0);
    PIN;
	pointer[ offset + 1 ] = (uint8_t)(g * 255.0);
    PIN;
	pointer[ offset + 2 ] = (uint8_t)(r * 255.0);
    PIN;
	pointer[ offset + 3 ] = 255;
#endif
}

inline void setPixel(int x, int y, Color color) {
	setPixel(x, y, color.red, color.green, color.blue);
}

inline void onFrame() {
	// SDL_GL_GetDrawableSize(window, &width, &height);
    // SDL_GetWindowSize(window, &width, &height);
	width = surface->w;
	height = surface->h;

	float aspectRatio = float(width) / float(height);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
            PIN;
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
		SDL_HandleError("SDL could not initialize!");
	}

	// Create window
	int flags = SDL_WINDOW_SHOWN;

#ifdef ENABLE_HIGHDPI
	flags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif

	window = SDL_CreateWindow("Raytracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
	if (window == nullptr) {
		SDL_HandleError("SDL could not create a Window!");
	}

	// Create the renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr) {
		SDL_HandleError("SDL could not create a Renderer!");
	}

	// Size the window
	SDL_GL_GetDrawableSize(window, &width, &height);
	// Create the surface
	surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	if (surface == nullptr) {
		SDL_HandleError("SDL could not create a Surface!");
	}

	// Create the texture
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (texture == nullptr) {
		SDL_HandleError("SDL could not create a Texture!");
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

		// Calculate the delta time
		uint64_t currentFrameTime = SDL_GetPerformanceCounter();
		deltaTime = ((currentFrameTime - lastFrameTime) * 1000 / (double)SDL_GetPerformanceFrequency());
		lastFrameTime = currentFrameTime;

		// Paints the back buffer
		long start = SDL_GetTicks64();

		if (SDL_MUSTLOCK(surface)) SDL_LockTexture(texture, nullptr, &pixels, &pitch);
        onFrame();
		if (SDL_MUSTLOCK(surface)) SDL_UnlockTexture(texture);

		// Display back buffer
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);

		std::cout << (SDL_GetTicks64() - start) << "ms" << std::endl;
	}

	std::cout << "Screen: " << surface->w << ", " << surface->h << std::endl;

	// Free resources
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	// Quit
	SDL_Quit();
	return 0;
}