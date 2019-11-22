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

		gui.pushContainer(10, 25, 240, 420);
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