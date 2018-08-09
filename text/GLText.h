#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/gl.h>
#include <GL/glu.h>

void preRenderText(int font_id, TTF_Font* font, char* text, char fgR, char fgG, char fgB,
		int *w, int *h);
class GLText
{
	public:
	bool borderOn;
		void setSize(int s);
		GLText(int s);
		GLText(int s, bool b);
		void setText(char* characters, char fgR, char fgG, char fgB);
		void setPosition(int x, int y);
		void render(int r, int g, int b); //colore di bordo
		bool hit(int x, int y);
		~GLText();
		int _w, _h,_x,_y;
	private:
		TTF_Font  *_font;
		int  _id;
};
