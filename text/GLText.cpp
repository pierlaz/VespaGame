#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <vector>
#include <string>
#include "GLText.h"


using namespace std;

void border(int a, int b, int c, int d) {
    int borderSize = 2;
    glBegin(GL_QUADS);
    glVertex2i(a - borderSize, b - borderSize);
    glVertex2i(a + c + borderSize, b - borderSize);
    glVertex2i(a + c + borderSize, b + d + borderSize);
    glVertex2i(a - borderSize, b + d + borderSize);
    glEnd();
}

void preRenderText(int font_id, TTF_Font *font, char *text, char fgR, char fgG, char fgB,
                   int *w, int *h) {
    SDL_Color tmpfontcolor = {(unsigned char)fgR, (unsigned char)fgG, (unsigned char)fgB, 0};
    SDL_Surface *initial;
    SDL_Surface *intermediary;
    SDL_Rect location;
    /* Use SDL_TTF to render our text */
    initial = NULL;
    initial = TTF_RenderText_Blended(font, text, tmpfontcolor);

    /* Convert the rendered text to a known format */
    *w = initial->w;
    *h = initial->h;
    // cerr << text << ": " << *w << " " << *h << endl;
    intermediary = SDL_CreateRGBSurface(3, *w, *h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    SDL_BlitSurface(initial, 0, intermediary, 0);

    /* Tell GL about our new texture */
    glBindTexture(GL_TEXTURE_2D, font_id);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, *w, *h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, intermediary->pixels);
    glEnable(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SDL_FreeSurface(initial);
    SDL_FreeSurface(intermediary);
}

void GLText::setSize(int s) {

}

GLText::GLText(int s) {
    if (TTF_Init() < 0) {
        fprintf(stderr, "Couldn't initialize TTF: %s\n", SDL_GetError());
        SDL_Quit();
    }
    borderOn = true;
    _font = TTF_OpenFont("FreeSans.ttf", s);
    _id = 99;
    _x = 0;
    _y = 0;
    _w = 0;
    _h = 0;
}

GLText::GLText(int s, bool b) {
    if (TTF_Init() < 0) {
        fprintf(stderr, "Couldn't initialize TTF: %s\n", SDL_GetError());
        SDL_Quit();
    }
    borderOn = false;
    _font = TTF_OpenFont("FreeSans.ttf", s);
    _id = 99;
    _x = 0;
    _y = 0;
    _w = 0;
    _h = 0;
}

void GLText::setText(char *characters, char fgR, char fgG, char fgB) {
    preRenderText(_id, _font, characters, fgR, fgG, fgB, &_w, &_h);
}

void GLText::setPosition(int x, int y) {
    _x = x;
    _y = y;
}

void GLText::render(int r, int g, int b) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_BLEND);
    /* prepare to render our texture */
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _id);
    // glColor3f(1.0, 0.0,0.0);

    /* Draw a quad at location */
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(_x, _y);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(_x + _w, _y);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(_x + _w, _y + _h);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(_x, _y + _h);
    glEnd();

    /* Clean up */
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glColor3ub(r, g, b);
    if (borderOn) border(_x, _y, _w, _h);
}

bool GLText::hit(int x, int y) {
    return x >= _x && x <= _x + _w && y >= _y && y <= _y + _h;
}

GLText::~GLText() {}
