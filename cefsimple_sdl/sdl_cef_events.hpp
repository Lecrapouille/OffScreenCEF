//
// Original version comes from:
// https://github.com/jamethy/sdl2-cef
// Created by James Burns on 7/29/18.
//

#ifndef CEF_SDL_CEF_EVENTS_H
#  define CEF_SDL_CEF_EVENTS_H

#include <SDL2/SDL.h>
#include <cef_app.h>

void handleKeyEvent(SDL_Event& event, CefRefPtr<CefBrowser> browser);

#endif // CEF_SDL_CEF_EVENTS_H
