#include <iostream>
#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include <cstdarg>
#include <sstream>

#include "SDL2/SDL.h"

#include "simple_gui_sdl2.hpp"
using namespace sgui;

int main(int argc, char** argv) {
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window *win = SDL_CreateWindow("Simple GUI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
	if (win == nullptr) return 1;

	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (ren == nullptr) return 1;

	Gui gui(new SDLInput(), new SDLRenderer(ren, win));

	SDL_Event evt;
	bool running = true;
	while (running) {
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_QUIT) running = false;
			else gui.input()->processEvents(&evt);
		}

		int w, h;
		SDL_GetWindowSize(win, &w, &h);

		gui.prepare(w, h);

		static Color bg(0x0);
		SDL_SetRenderDrawColor(ren, bg[0], bg[1], bg[2], 255);
		SDL_RenderClear(ren);

		gui.pushContainer(10, 10, 240, 470);
			gui.pushContainer(0, 0, 0, 64, Dock::DockTop);
				gui.text(0, 0, "Hello World! This is a simple test.", Overflow::OverfowWrap);
			gui.popContainer();

			gui.pushLayout(0, 0, 0, 22, Dock::DockTop, 0);
				gui.slider(GEN_ID, &bg.r, 0.0f, 1.0f, "R: %.2f");
			gui.popLayout();
			gui.pushLayout(0, 0, 0, 22, Dock::DockTop, 0);
				gui.slider(GEN_ID, &bg.g, 0.0f, 1.0f, "G: %.2f");
			gui.popLayout();
			gui.pushLayout(0, 0, 0, 22, Dock::DockTop, 0);
				gui.slider(GEN_ID, &bg.b, 0.0f, 1.0f, "B: %.2f");
			gui.popLayout();

			static std::string user = "";
			static std::string pass = "";
			gui.pushLayout(0, 0, 0, 20, Dock::DockTop, 0);
				gui.pushLayout(0, 0, 64, 0, Dock::DockLeft, 0);
					gui.text(0, 0, "User");
				gui.popLayout();
				gui.pushLayout(0, 0, 0, 0, Dock::DockFill, 0);
					gui.edit(GEN_ID, user);
				gui.popLayout();
			gui.popLayout();

			gui.pushLayout(0, 0, 0, 20, Dock::DockTop, 0);
				gui.pushLayout(0, 0, 64, 0, Dock::DockLeft, 0);
					gui.text(0, 0, "Password");
				gui.popLayout();
				gui.pushLayout(0, 0, 0, 0, Dock::DockFill, 0);
					gui.edit(GEN_ID, pass, true);
				gui.popLayout();
			gui.popLayout();

			static bool tggl = false;
			gui.pushLayout(0, 0, 0, 22, Dock::DockTop, 0);
				gui.toggle(GEN_ID, "Toggle", &tggl);
			gui.popLayout();

			static int sel = 1;
			// gui.pushLayout(0, 0, 0, 60, Dock::DockTop, 0);
			// 	gui.list(GEN_ID, &sel, { "Apples", "Oranges", "Grapes", "Really freakin' long text that gets hidden" });
			// gui.popLayout();

			gui.pushLayout(0, 0, 0, 22, Dock::DockTop, 0);
				gui.dropdown(GEN_ID, &sel, { "Apples", "Oranges", "Grapes", "Really freakin' long text that gets hidden" });
			gui.popLayout();

			gui.pushLayout(0, 0, 0, 22, Dock::DockTop, 0);
				gui.button(GEN_ID, "Button");
			gui.popLayout();
		gui.popContainer();

		gui.finish();

		SDL_RenderPresent(ren);
		SDL_Delay(1000 / 60);
	}

	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(ren);
	SDL_Quit();
	return 0;
}