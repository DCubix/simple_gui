#ifndef SIMPLE_GUI_SDL2_HPP
#define SIMPLE_GUI_SDL2_HPP

#if __has_include("SDL2/SDL.h")
#include "SDL2/SDL.h"
#else
#include "SDL.h"
#endif

#include "simple_gui.hpp"

namespace sgui {
	class SDLInput : public InputManager {
	public:
		inline void init(std::array<int, KeyCount>& keymap) override {
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

		inline void setClipboardText(const std::string& text) override {
			SDL_SetClipboardText(text.c_str());
		}

		inline std::string getClipboardText() override {
			return std::string(SDL_GetClipboardText());
		}

		inline int time() override {
			return SDL_GetTicks();
		}

		inline void processEvents(void* udata) override {
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
		inline SDLRenderer(SDL_Renderer* ren, SDL_Window* win)
			: ren(ren), win(win)
		{
			SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
		}

		inline void* loadFont(const std::vector<byte>& pixels, int width, int height) override {
			Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
			SDL_Surface* surf = SDL_CreateRGBSurfaceFrom((void*) pixels.data(), width, height, 32, width * 4, rmask, gmask, bmask, amask);
			SDL_SetColorKey(surf, 1, SDL_MapRGB(surf->format, 0, 0, 0));
			font = SDL_CreateTextureFromSurface(ren, surf);
			SDL_FreeSurface(surf);
			return font;
		}

		inline void created() override {}

		inline void destroyed() override {
			SDL_DestroyTexture(font);
		}

		inline void line(int x1, int y1, int x2, int y2, Color color) override {
			SDL_SetRenderDrawColor(ren, color[0], color[1], color[2], color[3]);
			SDL_RenderDrawLine(ren, x1, y1, x2, y2);
		}

		inline void rect(Rect rect, Color color, bool fill = false) override {
			SDL_SetRenderDrawColor(ren, color[0], color[1], color[2], color[3]);

			SDL_Rect rc = { rect.x, rect.y, rect.w, rect.h };
			if (fill)	SDL_RenderFillRect(ren, &rc);
			else		SDL_RenderDrawRect(ren, &rc);
		}

		inline void image(void* image, Rect src, Rect dst, Color color) override {
			SDL_Texture* img = (SDL_Texture*)image;
			SDL_Rect ssrc = { src.x, src.y, src.w, src.h };
			SDL_Rect sdst = { dst.x, dst.y, dst.w, dst.h };

			SDL_SetTextureBlendMode(img, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(img, color[3]);
			SDL_SetTextureColorMod(img, color[0], color[1], color[2]);
			SDL_RenderCopy(ren, img, &ssrc, &sdst);
		}

		inline void setClipRect(Rect rect) override {
			SDL_Rect rc = { rect.x, rect.y, rect.w, rect.h };
			SDL_RenderSetClipRect(ren, &rc);
		}

		inline void unsetClipRect() override {
			SDL_RenderSetClipRect(ren, nullptr);
		}

		SDL_Texture* font;
		SDL_Renderer* ren;
		SDL_Window* win;
	};
}

#endif // SIMPLE_GUI_SDL2_HPP