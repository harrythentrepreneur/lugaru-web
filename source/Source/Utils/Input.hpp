/*
Copyright (C) 2003, 2010 - Wolfire Games
Copyright (C) 2010-2017 - Lugaru contributors (see AUTHORS file)

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _INPUT_HPP_
#define _INPUT_HPP_

#include "Game.hpp"

#include <SDL.h>
#include <string>

/**> CONSTANT DECLARATIONS <**/
#define MOUSEBUTTON_LEFT (SDL_NUM_SCANCODES + SDL_BUTTON_LEFT)
#define MOUSEBUTTON_RIGHT (SDL_NUM_SCANCODES + SDL_BUTTON_RIGHT)
#define MOUSEBUTTON_MIDDLE (SDL_NUM_SCANCODES + SDL_BUTTON_MIDDLE)
#define MOUSEBUTTON_X1 (SDL_NUM_SCANCODES + SDL_BUTTON_X1)
#define MOUSEBUTTON_X2 (SDL_NUM_SCANCODES + SDL_BUTTON_X2)

/**> GAMEPAD BINDINGS <**/
/* A binding is stored as an unsigned short so it can live next to keyboard
 * scancodes in config.txt:
 *   0                                        -> unbound
 *   1000 + button index                      -> controller button
 *   2000 + axis * 2 (+1 when negative)       -> controller axis direction */
#define GP_BIND_NONE 0
#define GP_BIND_BUTTON_BASE 1000
#define GP_BIND_AXIS_BASE 2000
#define GP_BIND_BUTTON(b) (GP_BIND_BUTTON_BASE + (b))
#define GP_BIND_AXIS(a, positive) (GP_BIND_AXIS_BASE + (a) * 2 + ((positive) ? 0 : 1))
#define GP_IS_BUTTON(c) ((c) >= GP_BIND_BUTTON_BASE && (c) < GP_BIND_AXIS_BASE)
#define GP_IS_AXIS(c) ((c) >= GP_BIND_AXIS_BASE)

/**> FUNCTION PROTOTYPES <**/
class Input
{
public:
    static void Tick();
    static bool isKeyDown(int k);
    static bool isKeyPressed(int k);
    static const char* keyToChar(unsigned short which);
    static bool MouseClicked();

    /* Gamepad */
    static void initGamepad();
    static void shutdownGamepad();
    static bool gamepadConnected();
    static void updateGamepad();
    static void refreshGamepads();
    static std::string gamepadName();
    static unsigned short pollGamepadBinding();
    static void closeGamepad(int device_index);
    static void getRightStick(float& x, float& y);
    static void getLeftStick(float& x, float& y);
    static bool isGamepadBindingDown(unsigned short binding);
    static std::string gamepadBindingName(unsigned short binding);
    static std::string gamepadAxisName(int axis);
};

#endif
