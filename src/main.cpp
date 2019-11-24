#include <iostream>
#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include <cstdarg>
#include <sstream>

#include "SDL2/SDL.h"

#define lerp(a, b, t) ((1.0f - t) * a + b * t)

#include "simple_gui_sdl2.hpp"
#include "simple_gui_gl3.hpp"
using namespace sgui;

int main(int argc, char** argv) {
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window *win = SDL_CreateWindow("Simple GUI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (win == nullptr) return 1;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GLContext ctx = SDL_GL_CreateContext(win);
	SDL_GL_MakeCurrent(win, ctx);

	gladLoadGL();
	// SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	// if (ren == nullptr) return 1;

	Gui gui(new SDLInput(), new GL3Renderer());

	SDL_Event evt;
	bool running = true;
	while (running) {
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_QUIT) running = false;
			else gui.input()->processEvents(&evt);
		}

		int w, h;
		SDL_GetWindowSize(win, &w, &h);

		gui.prepare();

		static Color bg(0x0);
		static Color abg(0x0);

		abg.r = lerp(abg.r, bg.r, 0.1f);
		abg.g = lerp(abg.g, bg.g, 0.1f);
		abg.b = lerp(abg.b, bg.b, 0.1f);

		glClearColor(abg.r, abg.g, abg.b, abg.a);
		glClear(GL_COLOR_BUFFER_BIT);
		// SDL_SetRenderDrawColor(ren, abg[0], abg[1], abg[2], 255);
		// SDL_RenderClear(ren);

		gui.pushContainer(0, 0, w, 22);
			static int fileSel = -1;
			static int editSel = -1;
			gui.pushLayout(0, 0, gui.textWidth("File") + 16, 0, Dock::DockLeft, 0);
				if (gui.menu(GEN_ID, "File", &fileSel, { "New", "-", "Open", "Save", "-", "Exit" })) {

				}
			gui.popLayout();

			gui.pushLayout(0, 0, gui.textWidth("Edit") + 16, 0, Dock::DockLeft, 0);
				if (gui.menu(GEN_ID, "Edit", &editSel, { "Undo", "Redo", "-", "Random Background" })) {
					if (editSel == 2) {
						bg.r = float(std::rand()) / RAND_MAX;
						bg.g = float(std::rand()) / RAND_MAX;
						bg.b = float(std::rand()) / RAND_MAX;
					}
				}
			gui.popLayout();
		gui.popContainer();

		gui.pushScrollContainer(GEN_ID, 32, 32, 400, 400,  640, 640);
			gui.pushLayout(0, 0, 120, 20, Dock::DockTop);
				gui.button(GEN_ID + 3, "Testing");
			gui.popLayout();
		gui.popScrollContainer();

		gui.finish(w, h);

		SDL_GL_SwapWindow(win);
		// SDL_RenderPresent(ren);
		SDL_Delay(1000 / 60);
	}

	SDL_GL_DeleteContext(ctx);
	SDL_DestroyWindow(win);
	// SDL_DestroyRenderer(ren);
	SDL_Quit();
	return 0;
}