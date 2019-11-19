#include <iostream>
#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include <cstdarg>
#include <sstream>

#include "SDL2/SDL.h"

#include "simple_gui.hpp"
using namespace sgui;

class SDLInput : public InputManager {
public:
	void init(std::array<int, KeyCount>& keymap) override {
		keymap[Key::KeyAlt] = SDLK_LALT;
		keymap[Key::KeyBackspace] = SDLK_BACKSPACE;
		keymap[Key::KeyCtrl] = SDLK_LCTRL;
		keymap[Key::KeyShift] = SDLK_LSHIFT;
		keymap[Key::KeyDelete] = SDLK_DELETE;
		keymap[Key::KeyEnd] = SDLK_END;
		keymap[Key::KeyEnter] = SDLK_RETURN;
		keymap[Key::KeyHome] = SDLK_HOME;
		keymap[Key::KeyLeft] = SDLK_LEFT;
		keymap[Key::KeyRight] = SDLK_RIGHT;
		keymap[Key::KeyV] = SDLK_v;
		keymap[Key::KeyX] = SDLK_x;
		keymap[Key::KeyC] = SDLK_c;
	}

	void setClipboardText(const std::string& text) override {
		SDL_SetClipboardText(text.c_str());
	}

	std::string getClipboardText() override {
		return std::string(SDL_GetClipboardText());
	}

	int time() override {
		return SDL_GetTicks();
	}

	void processEvents(void* udata) override {
		SDL_Event event = *((SDL_Event*)udata);
		switch (event.type) {
			case SDL_MOUSEMOTION:
				m_mouseX = event.motion.x;
				m_mouseY = event.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				m_mouse[event.button.button].down = true;
				m_mouse[event.button.button].pressed = true;
				break;
			case SDL_MOUSEBUTTONUP:	
				m_mouse[event.button.button].down = false;
				m_mouse[event.button.button].released = true;
				break;
			case SDL_KEYDOWN: {
				Key k = translateKey(event.key.keysym.sym);
				m_keyboard[k].down = true;
				m_keyboard[k].pressed = true;
			} break;
			case SDL_KEYUP: {
				Key k = translateKey(event.key.keysym.sym);
				m_keyboard[k].down = false;
				m_keyboard[k].released = true;
			} break;
			case SDL_TEXTINPUT:
				m_char = event.text.text[0];
				break;
		}
	}
};

class SDLRenderer : public Renderer {
public:
	SDLRenderer(SDL_Renderer* ren, SDL_Window* win)
		: ren(ren), win(win)
	{
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
	}

	void* loadFont(unsigned char* pixels, int len) override {
		SDL_RWops *fnt = SDL_RWFromMem(pixels, len);
		SDL_Surface* surf = SDL_LoadBMP_RW(fnt, 1);
		SDL_SetColorKey(surf, 1, SDL_MapRGB(surf->format, 0, 0, 0));
		font = SDL_CreateTextureFromSurface(ren, surf);
		SDL_FreeSurface(surf);
		return font;
	}

	void created() override {}

	void destroyed() override {
		SDL_DestroyTexture(font);
	}

	void line(int x1, int y1, int x2, int y2, Color color) override {
		SDL_SetRenderDrawColor(ren, color[0], color[1], color[2], color[3]);
		SDL_RenderDrawLine(ren, x1, y1, x2, y2);
	}

	void rect(Rect rect, Color color, bool fill = false) override {
		SDL_SetRenderDrawColor(ren, color[0], color[1], color[2], color[3]);

		SDL_Rect rc = { rect.x, rect.y, rect.w, rect.h };
		if (fill)	SDL_RenderFillRect(ren, &rc);
		else		SDL_RenderDrawRect(ren, &rc);
	}

	void image(void* image, Rect src, Rect dst, Color color) override {
		SDL_Texture* img = (SDL_Texture*)image;
		SDL_Rect ssrc = { src.x, src.y, src.w, src.h };
		SDL_Rect sdst = { dst.x, dst.y, dst.w, dst.h };

		SDL_SetTextureBlendMode(img, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod(img, color[3]);
		SDL_SetTextureColorMod(img, color[0], color[1], color[2]);
		SDL_RenderCopy(ren, img, &ssrc, &sdst);
	}

	void setClipRect(Rect rect) override {
		SDL_Rect rc = { rect.x, rect.y, rect.w, rect.h };
		SDL_RenderSetClipRect(ren, &rc);
	}

	void unsetClipRect() override {
		SDL_RenderSetClipRect(ren, nullptr);
	}

	Rect windowRect() override {
		Rect rc(0, 0, 0, 0);
		SDL_GetWindowSize(win, &rc.w, &rc.h);
		return rc;
	}
	
	SDL_Texture* font;
	SDL_Renderer* ren;
	SDL_Window* win;
};

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

		static Color bg(0x0);
		SDL_SetRenderDrawColor(ren, bg[0], bg[1], bg[2], 255);
		SDL_RenderClear(ren);

		gui.pushContainer(10, 10, 240, 400);
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
			gui.pushLayout(0, 0, 0, 100, Dock::DockTop, 0);
				gui.list(GEN_ID, &sel, { "Apples", "Oranges", "Grapes" });
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