#ifndef SIMPLE_GUI_GL3_HPP
#define SIMPLE_GUI_GL3_HPP

#include "glad.h"

#include "simple_gui.hpp"

#ifndef __gl_h_
#error Please include some OpenGL3 headers
#else

#include <iostream>

#define SGUI_GL3_MAX_VERTICES 100000
namespace sgui {
	struct Texture { GLuint id{ 0 }; int w, h; };
	struct Vert { float x, y, u, v, r, g, b, a; };
	struct Batch { int offset{ 0 }, length{ 0 }; GLenum prim{ 0 }; Texture tex{}; Rect scissor{ 0, 0, 0, 0 }; };
	struct Glyph { std::vector<Vert> vertices; Texture tex{}; Rect scissor{ 0, 0, 0, 0 }; GLuint prim{ 0 }; };

	class GL3Renderer : public Renderer {
	public:
		inline void* loadFont(const std::vector<byte>& pixels, int width, int height) override {
			m_font.w = width;
			m_font.h = height;
			glGenTextures(1, &m_font.id);
			glBindTexture(GL_TEXTURE_2D, m_font.id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
			glBindTexture(GL_TEXTURE_2D, 0);
			return (void*) &m_font;
		}

		inline void created() override {
			glGenBuffers(1, &m_vbo);
			glGenVertexArrays(1, &m_vao);

			glBindVertexArray(m_vao);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
			glBufferData(GL_ARRAY_BUFFER, SGUI_GL3_MAX_VERTICES * sizeof(Vert), nullptr, GL_DYNAMIC_DRAW);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Vert), (void*) 0);
			glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vert), (void*) 8);
			glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(Vert), (void*) 16);

			glBindVertexArray(0);

			const char* vert = R"(
#version 330 core
layout (location = 0) in vec2 vPos;
layout (location = 1) in vec2 vTex;
layout (location = 2) in vec4 vCol;

uniform mat4 uProj;

out vec2 oTex;
out vec4 oCol;

void main() {
	gl_Position = uProj * vec4(vPos, 0.0, 1.0);
	oTex = vTex;
	oCol = vCol;
}
			)";

			const char* frag = R"(
#version 330 core
out vec4 fragColor;

uniform bool uTexOn;
uniform sampler2D uTex;

in vec2 oTex;
in vec4 oCol;

void main() {
	vec4 col = oCol;
	if (uTexOn) col *= texture(uTex, oTex);
	fragColor = col;
}
			)";

			GLuint vs = createShader(vert, GL_VERTEX_SHADER);
			GLuint fs = createShader(frag, GL_FRAGMENT_SHADER);
			
			m_shader = glCreateProgram();
			glAttachShader(m_shader, vs);
			glAttachShader(m_shader, fs);
			glLinkProgram(m_shader);

			char log[1024];
			glGetProgramInfoLog(m_shader, 1024, nullptr, log);
			std::cerr << log << std::endl;

			m_uTex   = glGetUniformLocation(m_shader, "uTex");
			m_uProj  = glGetUniformLocation(m_shader, "uProj");
			m_uTexOn = glGetUniformLocation(m_shader, "uTexOn");
		}

		inline void destroyed() override {
			glDeleteBuffers(1, &m_vbo);
			glDeleteVertexArrays(1, &m_vao);
			glDeleteTextures(1, &m_font.id);
			glDeleteProgram(m_shader);
		}

		void updateBuffer() {
			std::vector<Vert> verts;
			verts.reserve(SGUI_GL3_MAX_VERTICES);
			
			Glyph first = m_glyphs[0];
			Batch b{};
			b.offset = 0;
			b.length = first.vertices.size();
			b.prim = first.prim;
			b.tex = first.tex;
			b.scissor = first.scissor;
			m_batches.push_back(b);

			verts.insert(verts.end(), first.vertices.begin(), first.vertices.end());

			int off = 0;
			for (size_t i = 1; i < m_glyphs.size(); i++) {
				Glyph curr = m_glyphs[i];
				Glyph prev = m_glyphs[i - 1];
				if (curr.tex.id != prev.tex.id || curr.prim != prev.prim) {
					off += m_batches.back().length;
					Batch b{};
					b.offset = off;
					b.length = curr.vertices.size();
					b.prim = curr.prim;
					b.tex = curr.tex;
					b.scissor = curr.scissor;
					m_batches.push_back(b);
				} else {
					m_batches.back().length += curr.vertices.size();
				}
				verts.insert(verts.end(), curr.vertices.begin(), curr.vertices.end());
			}

			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(Vert), verts.data());

			m_glyphs.clear();
		}

		virtual void end(int width, int height) {
			if (m_glyphs.empty()) return;

			updateBuffer();

			int vp[4];
			glGetIntegerv(GL_VIEWPORT, vp);
			glViewport(0, 0, width, height);

			GLint prog; glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
			glUseProgram(m_shader);

			float m[16]; ortho(0, width, height, 0, -1, 1, m);
			glUniformMatrix4fv(m_uProj, 1, true, m);
			glUniform1i(m_uTex, 0);

			bool hasDepth = glIsEnabled(GL_DEPTH_TEST);
			bool hasScissor = glIsEnabled(GL_SCISSOR_TEST);
			bool hasCull = glIsEnabled(GL_CULL_FACE);
			bool hasBlend = glIsEnabled(GL_BLEND);
			GLint bsrc, bdest;
			glGetIntegerv(GL_BLEND_SRC, &bsrc);
			glGetIntegerv(GL_BLEND_DST, &bdest);

			if (!hasScissor) glEnable(GL_SCISSOR_TEST);
			if (hasDepth) glDisable(GL_DEPTH_TEST);
			if (hasCull) glDisable(GL_CULL_FACE);
			if (!hasBlend) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			glBindVertexArray(m_vao);
			for (auto b : m_batches) {
				if (b.tex.id != 0) {
					glBindTexture(GL_TEXTURE_2D, b.tex.id);
					glActiveTexture(GL_TEXTURE0);
					glUniform1i(m_uTexOn, 1);
				} else {
					glUniform1i(m_uTexOn, 0);
				}

				glScissor(b.scissor.x, height - b.scissor.h - b.scissor.y, b.scissor.w, b.scissor.h);
				if (b.length > 0) glDrawArrays(b.prim, b.offset, b.length);

				if (b.tex.id != 0) {
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}

			glBindVertexArray(0);
			glUseProgram(prog);

			if (!hasScissor) glDisable(GL_SCISSOR_TEST);
			if (hasDepth) glEnable(GL_DEPTH_TEST);
			if (hasCull) glEnable(GL_CULL_FACE);
			if (!hasBlend) glDisable(GL_BLEND);
			else glBlendFunc(bsrc, bdest);

			m_batches.clear();
			glViewport(vp[0], vp[1], vp[2], vp[3]);
		}

		inline void processCommand(const Command& cmd) {
			Glyph g;
			g.tex.id = 0;
			g.prim = 0xFF;

			switch (cmd.type) {
				case Command::CmdDrawLine: {
					g.prim = GL_LINES;
					g.vertices.push_back(Vert{ cmd.points[0].x, cmd.points[0].y, 0, 0, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[1].x, cmd.points[1].y, 0, 0, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
				} break;
				case Command::CmdDrawRect: {
					g.prim = GL_LINE_LOOP;
					g.vertices.push_back(Vert{ cmd.points[0].x, cmd.points[0].y, 0, 0, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[1].x, cmd.points[0].y, 1, 0, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[1].x, cmd.points[1].y, 1, 1, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[0].x, cmd.points[1].y, 0, 1, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
				} break;
				case Command::CmdFillRect: {
					g.prim = GL_TRIANGLES;
					g.vertices.push_back(Vert{ cmd.points[0].x, cmd.points[0].y, 0, 0, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[1].x, cmd.points[0].y, 1, 0, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[1].x, cmd.points[1].y, 1, 1, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[1].x, cmd.points[1].y, 1, 1, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[0].x, cmd.points[1].y, 0, 1, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[0].x, cmd.points[0].y, 0, 0, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
				} break;
				case Command::CmdDrawImage: {
					Texture* img = (Texture*) cmd.image;
					float u1 = float(cmd.src.x) / img->w;
					float v1 = float(cmd.src.y) / img->h;
					float u2 = float(cmd.src.x + cmd.src.w) / img->w;
					float v2 = float(cmd.src.y + cmd.src.h) / img->h;

					g.prim = GL_TRIANGLES;
					g.tex = *img;
					g.vertices.push_back(Vert{ cmd.points[0].x, cmd.points[0].y, u1, v1, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[1].x, cmd.points[0].y, u2, v1, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[1].x, cmd.points[1].y, u2, v2, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[1].x, cmd.points[1].y, u2, v2, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[0].x, cmd.points[1].y, u1, v2, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
					g.vertices.push_back(Vert{ cmd.points[0].x, cmd.points[0].y, u1, v1, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a });
				} break;
				case Command::CmdSetClip: {
					int x = cmd.points[0].x,
						y = cmd.points[0].y,
						w = cmd.points[1].x - cmd.points[0].x,
						h = cmd.points[1].y - cmd.points[0].y;
					m_nextClip = Rect(x, y, w, h);
				} break;
				case Command::CmdUnsetClip: {
					int vp[4];
					glGetIntegerv(GL_VIEWPORT, vp);
					m_nextClip = Rect(vp[0], vp[1], vp[2], vp[3]);
				} break;
			}
			g.scissor = m_nextClip;
			m_glyphs.push_back(g);
		}

	private:
		Texture m_font;
		GLuint m_vbo, m_vao, m_shader, m_uProj, m_uTex, m_uTexOn;

		Rect m_nextClip{ 0, 0, 0, 0 };

		std::vector<Batch> m_batches;
		std::vector<Glyph> m_glyphs;

		inline GLuint createShader(const char* src, GLenum type) {
			GLuint s = glCreateShader(type);
			glShaderSource(s, 1, &src, nullptr);
			glCompileShader(s);
			char log[1024];
			glGetShaderInfoLog(s, 1024, nullptr, log);
			std::cerr << log << std::endl;
			return s;
		}

		inline void ortho(float left, float right, float bottom, float top, float near, float far, float* mat) {
			const float w = right - left;
			const float h = top - bottom;
			const float d = far - near;
			float m[] = {
				2.0f / w, 0.0f, 0.0f, -((right + left) / w),
				0.0f, 2.0f / h, 0.0f, -((top + bottom) / h),
				0.0f, 0.0f, -2.0f / d, -((far + near) / d),
				0.0f, 0.0f, 0.0f, 1.0f
			};
			std::copy(m, m + 16, mat);
		}
	};
}
#endif

#endif // SIMPLE_GUI_GL3_HPP