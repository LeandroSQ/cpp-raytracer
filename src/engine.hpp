#pragma once
#include "color.hpp"
#include "math.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include "world.hpp"
#include <SDL2/SDL.h>
#include <iostream>

#include "../bindings/imgui_impl_sdl.h"
#include "../bindings/imgui_impl_sdlrenderer.h"
#include <imgui.h>
// #include "../bindings/imgui_impl_opengl2.h"

struct Size {
	int width = 0;
	int height = 0;

	Size() : width(0), height(0) { }
	Size(int width, int height) : width(width), height(height) { }
};

#define ENABLE_HIGHDPI

#define PIN std::cout << __FILE__ << ":" << __LINE__ << "    " << __func__ << std::endl;
#define SDL_HandleError(msg)                                                                                           \
 std::cout << msg << " " << SDL_GetError() << std::endl;                                                               \
 return -1;

class Engine {
  public:
	// Viewport
	Size viewport = Size(800, 450);

	// Flags
	bool isGammaCorrectionEnabled = true;
	bool isMouseMovingLight = false;
	bool isMouseMovingCamera = false;
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
	double intervalBetweenFrames = 1000.0 / 60.0;
	uint16_t fps = 0;
	uint16_t frameCounter = 0;
	uint64_t lastFrameTime = 0L;
	double lastFrameDuration = 0;
	double deltaTime = 0.0;

	// Viewport
	Size virtualViewport = viewport;
	float aspectRatio = float(viewport.width) / float(viewport.height);

  public:
	Engine() { }

	~Engine() {
		// Free ImGui resources
		ImGui_ImplSDL2_Shutdown();
		ImGui_ImplSDLRenderer_Shutdown();
		ImGui::DestroyContext();

		// Free SDL resources
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

		// Create ImGui context
		if (createImGuiContext() < 0) return -1;

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
				lastFrameDuration =
					(SDL_GetPerformanceCounter() - currentFrameTime) * 1000 / (double)SDL_GetPerformanceFrequency();
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

		SDL_RendererInfo rendererInfo;
		SDL_GetRendererInfo(renderer, &rendererInfo);
		std::cout << "Renderer: " << rendererInfo.name << std::endl;

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

	int createImGuiContext() {
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		embraceTheDarkness();

		// ImGui::GetIO().DisplayFramebufferScale = ImVec2(virtualViewport.width / viewport.width,
		// virtualViewport.height / viewport.height); ImGui::GetIO().DisplayFramebufferScale = ImVec2(4f, 4f);

		// std::cout << .x << ", " << ImGui::GetIO().DisplayFramebufferScale.y << std::endl;

		// Setup Platform/Renderer bindings
		if (!ImGui_ImplSDL2_InitForSDLRenderer(window, renderer)) return -1;
		if (!ImGui_ImplSDLRenderer_Init(renderer)) return -1;

		return 0;
	}

	void embraceTheDarkness() {
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
		colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
		colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding = ImVec2(8.00f, 8.00f);
		style.FramePadding = ImVec2(5.00f, 2.00f);
		style.CellPadding = ImVec2(6.00f, 6.00f);
		style.ItemSpacing = ImVec2(6.00f, 6.00f);
		style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
		style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
		style.IndentSpacing = 25;
		style.ScrollbarSize = 15;
		style.GrabMinSize = 10;
		style.WindowBorderSize = 1;
		style.ChildBorderSize = 1;
		style.PopupBorderSize = 1;
		style.FrameBorderSize = 1;
		style.TabBorderSize = 1;
		style.WindowRounding = 7;
		style.ChildRounding = 4;
		style.FrameRounding = 3;
		style.PopupRounding = 4;
		style.ScrollbarRounding = 9;
		style.GrabRounding = 3;
		style.LogSliderDeadzone = 4;
		style.TabRounding = 4;
	}

	void handleEvents() {
		// Fetch the next event in queue
		SDL_Event event;
		SDL_PollEvent(&event);

		// Transpose to ImGui
		ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type) {
			/* Window closed */
			case SDL_QUIT:
				isRunning = false;
				break;

			/* Mouse event */
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
				onMouseEvent(event);
				break;
		}
	}

	void onMouseEvent(SDL_Event event) {
		// Get the current mouse status
		int x = event.motion.x, y = event.motion.y;
		if (x <= 0 || x >= virtualViewport.width || y <= 0 || y >= virtualViewport.height) return;
		// SDL_GetMouseState(&x, &y);
		// int x = ImGui::GetMousePos().x;
		// int y = ImGui::GetMousePos().y;

		// Localize to screen UV coordinates [0.0 to 1.0]
		float nX = x / float(viewport.width), nY = y / float(viewport.height);

		if (isMouseMovingLight) {
			world.light.x = (1.0f - nX) * 4.0f - 2.0f;
			world.light.y = (1.0f - nY) * 4.0f - 2.0f;
			// std::cout << "Light: " << world.light.x << ", " << world.light.y << std::endl;
		}

		if (isMouseMovingCamera) {
			world.camera.origin.x = (nX)*2.0f - 1.0f;
			world.camera.origin.y = (nY)*2.0f - 1.0f;
			// world.camera.origin.z = 2.0f * ((nX + nY) / 2);
		} else {
			world.camera.origin.x = 0;
			world.camera.origin.y = 0;
		}
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

		onRenderGUI();

		// Display the back buffer into the window
		SDL_RenderPresent(renderer);
	}

	void onRenderGUI() {
		ImGui_ImplSDL2_NewFrame(window);
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui::NewFrame();

		onRenderOverlay();
		// ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	}

	void onRenderOverlay() {
		const float padding = 0.0f;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const ImGuiIO& io = ImGui::GetIO();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
										ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
										ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

		// Define the overlay position
		ImVec2 overlayPosition(viewport->WorkPos.x + padding, viewport->WorkPos.y + padding);
		ImGui::SetNextWindowPos(overlayPosition, ImGuiCond_Always, ImVec2(0.0f, 0.0f));

		// Define the transparent background
		ImGui::SetNextWindowBgAlpha(0.35f);

		if (ImGui::Begin("Example: Simple overlay", NULL, window_flags)) {
			ImGui::Text("Ray tracer");
			ImGui::Separator();
			ImGui::Text("FPS: %d - (%.2f ms)", fps, lastFrameDuration);
			ImGui::Text(
				"Camera: {%.2f, %.2f, %.2f}", world.camera.origin.x, world.camera.origin.y, world.camera.origin.z
			);
			ImGui::Separator();
			ImGui::Checkbox("Gamma correction", &isGammaCorrectionEnabled);
			ImGui::Checkbox("Mouse move light", &isMouseMovingLight);
			ImGui::Checkbox("Mouse move camera", &isMouseMovingCamera);
			ImGui::Separator();

			if (ImGui::IsMousePosValid()) ImGui::Text("Mouse Position: (%.1f, %.1f)", io.MousePos.x, io.MousePos.y);

			ImGui::End();
		}
	}

	void onRender() {
		for (int y = 0; y < virtualViewport.height; y++) {
			for (int x = 0; x < virtualViewport.width; x++) {
				// Calculate the UV coordinates [0.0 to 1.0]
				float u = (float(x) / float(virtualViewport.width)) * 2.0f - 1.0f;
				float v = (float(y) / float(virtualViewport.height)) * 2.0f - 1.0f;

				// Maintain the aspect ratio
				u *= aspectRatio;

				// Create the
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
							float gamma = 2.2f;
							color = Color::pow(color, 1.0f / gamma);
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
		uint8_t* pointer = (uint8_t*)pixels;

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