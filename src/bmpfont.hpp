#ifndef __BMPFONT_TP_HPP___
#define __BMPFONT_TP_HPP___

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <memory>
#include <string>

namespace tp {

/**
 * renderer, x, y, font texture, character to print
 * */
int draw_char(std::shared_ptr<SDL_Renderer> r, int x, int y, std::shared_ptr<SDL_Texture> tex, char c, int w = 10, int h = 10)
{
    SDL_Rect dst_rect = {x, y, (int)w, (int)h};
    SDL_Rect src_rect = {w * (int)(c & 0x0f), h * (int)((c & 0x0f0) >> 4), (int)w, (int)h};
    double a = 0.0;
    SDL_RenderCopyEx(r.get(), tex.get(), &src_rect, &dst_rect, a, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
    return x + w;
}

void draw_text(std::shared_ptr<SDL_Renderer> r, int x, int y, std::shared_ptr<SDL_Texture> tex, std::string txt)
{
    int w = 10, h = 10;
    SDL_QueryTexture(tex.get(), NULL, NULL, &w, &h);
    w = w / 16;
    h = h / 16;
    int x0 = x;

    for (auto c : txt) {
        if (c == '\n') {
            x = x0;
            y += h;
        } else {
            x = draw_char(r, x, y, tex, c, w, h);
        }
    }
}
} // namespace tp
#endif
