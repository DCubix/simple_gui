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

		inline void destroyed() override {
			SDL_DestroyTexture(font);
		}

		inline void processCommand(const Command& cmd) {
			switch (cmd.type) {
				case Command::CmdDrawLine: {
					SDL_SetRenderDrawColor(ren, cmd.color[0], cmd.color[1], cmd.color[2], cmd.color[3]);
					SDL_RenderDrawLine(ren, cmd.points[0].x, cmd.points[0].y, cmd.points[1].x, cmd.points[1].y);
				} break;
				case Command::CmdDrawRect: {
					SDL_Rect rc = {
						cmd.points[0].x, cmd.points[0].y,
						cmd.points[1].x - cmd.points[0].x, cmd.points[1].y - cmd.points[0].y
					};
					SDL_SetRenderDrawColor(ren, cmd.color[0], cmd.color[1], cmd.color[2], cmd.color[3]);
					SDL_RenderDrawRect(ren, &rc);
				} break;
				case Command::CmdFillRect: {
					SDL_Rect rc = {
						cmd.points[0].x, cmd.points[0].y,
						cmd.points[1].x - cmd.points[0].x, cmd.points[1].y - cmd.points[0].y
					};
					SDL_SetRenderDrawColor(ren, cmd.color[0], cmd.color[1], cmd.color[2], cmd.color[3]);
					SDL_RenderFillRect(ren, &rc);
				} break;
				case Command::CmdDrawImage: {
					SDL_Rect dst = {
						cmd.points[0].x, cmd.points[0].y,
						cmd.points[1].x - cmd.points[0].x, cmd.points[1].y - cmd.points[0].y
					};
					SDL_Rect src = {
						cmd.src.x, cmd.src.y, cmd.src.w, cmd.src.h
					};
					SDL_Texture* img = (SDL_Texture*)cmd.image;
					SDL_SetTextureBlendMode(img, SDL_BLENDMODE_BLEND);
					SDL_SetTextureAlphaMod(img, cmd.color[3]);
					SDL_SetTextureColorMod(img, cmd.color[0], cmd.color[1], cmd.color[2]);
					SDL_RenderCopy(ren, img, &src, &dst);
				} break;
				case Command::CmdSetClip: {
					SDL_Rect rc = {
						cmd.points[0].x, cmd.points[0].y,
						cmd.points[1].x - cmd.points[0].x, cmd.points[1].y - cmd.points[0].y
					};
					SDL_RenderSetClipRect(ren, &rc);
				} break;
				case Command::CmdUnsetClip: SDL_RenderSetClipRect(ren, nullptr); break;
			}
		}

		SDL_Texture* font;
		SDL_Renderer* ren;
		SDL_Window* win;
	};
}

#endif // SIMPLE_GUI_SDL2_HPP