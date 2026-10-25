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

#include "Utils/Input.hpp"

#include "Game.hpp"
#include <string>

using namespace Game;

bool keyDown[3000];
bool keyPressed[3000];

void Input::Tick()
{
    SDL_PumpEvents();
    int numkeys;
    const Uint8* keyState = SDL_GetKeyboardState(&numkeys);
    for (int i = 0; i < numkeys; i++) {
        keyPressed[i] = !keyDown[i] && keyState[i];
        keyDown[i] = keyState[i];
    }
    Uint8 mb = SDL_GetMouseState(NULL, NULL);
    for (int i = 1; i < 6; i++) {
        keyPressed[SDL_NUM_SCANCODES + i] = !keyDown[SDL_NUM_SCANCODES + i] && (mb & SDL_BUTTON(i));
        keyDown[SDL_NUM_SCANCODES + i] = (mb & SDL_BUTTON(i));
    }
    /* Drain the event queue so controller hotplug / device-removed events
     * are picked up without the user having to toggle gamepad support. */
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_CONTROLLERDEVICEADDED) {
            initGamepad();
        } else if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
            closeGamepad(e.cdevice.which);
        } else {
            SDL_PushEvent(&e);
        }
    }
    updateGamepad();
}

bool Input::isKeyDown(int k)
{
    if (k < 0 || k >= 3000) {
        return false;
    }
    return keyDown[k];
}

bool Input::isKeyPressed(int k)
{
    if (k < 0 || k >= 3000) {
        return false;
    }
    return keyPressed[k];
}

const char* Input::keyToChar(unsigned short i)
{
    if (i < SDL_NUM_SCANCODES) {
        return SDL_GetKeyName(SDL_GetKeyFromScancode(SDL_Scancode(i)));
    } else if (i == MOUSEBUTTON_LEFT) {
        return "mouse left button";
    } else if (i == MOUSEBUTTON_RIGHT) {
        return "mouse right button";
    } else if (i == MOUSEBUTTON_MIDDLE) {
        return "mouse middle button";
    } else if (i == MOUSEBUTTON_X1) {
        return "mouse button 4";
    } else if (i == MOUSEBUTTON_X2) {
        return "mouse button 5";
    } else {
        return "unknown";
    }
}

/* ---- Gamepad subsystem state -------------------------------------------- */
static int gamepadindices[16];
static SDL_GameController* gamepads[16] = { NULL };
static int gamepadCount = 0;
static bool gamepadInitialized = false;

bool Input::MouseClicked()
{
    if (isKeyPressed(MOUSEBUTTON_LEFT)) {
        return true;
    }
    if (gamepadInitialized && gamepadCount > 0 && gamepads[0]) {
        return SDL_GameControllerGetButton(gamepads[0], SDL_CONTROLLER_BUTTON_A);
    }
    return false;
}

/* Binding encoding (see Game.hpp):
 *   GP_BIND_NONE        = 0  (unbound)
 *   GP_BIND_BUTTON_BASE = 1000 + SDL_GameControllerButton enum value
 *   GP_BIND_AXIS_BASE   = 2000 + SDL_GameControllerAxis enum value */
void Input::initGamepad()
{
    if (gamepadInitialized) {
        return;
    }

    for (int i = 0; i < 16; ++i) {
        gamepadindices[i] = -1;
        gamepads[i] = NULL;
    }

    if (!SDL_WasInit(SDL_INIT_GAMECONTROLLER)) {
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    }

    /* Add common PS4 / DualShock 4 mapping so Bluetooth controllers work.
     * No platform: field — on Emscripten SDL reports platform "Emscripten",
     * so Linux-only mappings would never match. */
    SDL_GameControllerAddMapping("030000004c050000c405000011010000,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b11,lefttrigger:a2,rightshoulder:b5,rightstick:b12,righttrigger:a5,start:b9,x:b2,y:b3,");
    SDL_GameControllerAddMapping("030000004c050000c405000000010000,PS4 Controller,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b11,lefttrigger:a2,rightshoulder:b5,rightstick:b12,righttrigger:a5,start:b9,x:b2,y:b3,");
    SDL_GameControllerAddMapping("030000004c0500006802000011810000,PS4 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:a3,rightshoulder:b5,rightstick:b11,righttrigger:a4,start:b9,x:b0,y:b3,");

    int opened = 0;
    for (int i = 0; i < SDL_NumJoysticks() && opened < 16; ++i) {
        SDL_GameController* gc = SDL_GameControllerOpen(i);
        if (gc) {
            gamepadindices[opened] = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gc));
            gamepads[opened] = gc;
            opened++;
        }
    }

    gamepadCount = opened;
    gamepadInitialized = (opened > 0);

    if (gamepadInitialized) {
        printf("Lugaru: gamepad subsystem initialized, %d controller(s) opened.\n", opened);
    }
}

void Input::shutdownGamepad()
{
    if (!gamepadInitialized) {
        return;
    }
    for (int i = 0; i < gamepadCount; ++i) {
        if (gamepads[i]) {
            SDL_GameControllerClose(gamepads[i]);
            gamepads[i] = NULL;
        }
    }
    gamepadCount = 0;
    gamepadInitialized = false;
}

void Input::closeGamepad(int which)
{
    for (int i = 0; i < gamepadCount; ++i) {
        if (gamepads[i] && gamepadindices[i] == which) {
            SDL_GameControllerClose(gamepads[i]);
            gamepads[i] = NULL;
            gamepadindices[i] = -1;
            /* compact the array */
            for (int j = i; j + 1 < gamepadCount; ++j) {
                gamepads[j] = gamepads[j + 1];
                gamepadindices[j] = gamepadindices[j + 1];
            }
            gamepadCount--;
            break;
        }
    }
    if (gamepadCount == 0) {
        gamepadInitialized = false;
    } else {
        /* still at least one controller open */
        gamepadInitialized = true;
    }
}

void Input::refreshGamepads()
{
    if (gamepadInitialized && gamepadCount > 0 && gamepads[0]) {
        SDL_Joystick* j = SDL_GameControllerGetJoystick(gamepads[0]);
        if (j && SDL_JoystickGetAttached(j)) {
            return;
        }
    }
    if (gamepadInitialized) {
        shutdownGamepad();
    }
    initGamepad();
}

std::string Input::gamepadName()
{
    if (!gamepadInitialized || gamepadCount <= 0 || !gamepads[0]) {
        return "";
    }
    const char* n = SDL_GameControllerName(gamepads[0]);
    return n ? std::string(n) : "Controller";
}

bool Input::gamepadConnected()
{
    return gamepadInitialized;
}

bool Input::isGamepadBindingDown(unsigned short binding)
{
    if (binding == GP_BIND_NONE) {
        return false;
    }
    if (GP_IS_BUTTON(binding)) {
        SDL_GameControllerButton b = SDL_GameControllerButton((int)(binding - GP_BIND_BUTTON_BASE));
        for (int c = 0; c < gamepadCount; ++c) {
            if (!gamepads[c]) continue;
            if (SDL_GameControllerGetButton(gamepads[c], b)) {
                return true;
            }
        }
    } else if (GP_IS_AXIS(binding)) {
        int raw = (int)(binding - GP_BIND_AXIS_BASE);
        SDL_GameControllerAxis a = SDL_GameControllerAxis(raw / 2);
        bool positive = (raw % 2) == 0;
        for (int c = 0; c < gamepadCount; ++c) {
            if (!gamepads[c]) continue;
            Sint16 value = SDL_GameControllerGetAxis(gamepads[c], a);
            if (positive ? (value > 16384) : (value < -16384)) {
                return true;
            }
        }
    }
    return false;
}

/* Poll for the first gamepad button/axis currently pressed past its threshold.
 * Returns 0 when nothing is pressed, otherwise the encoded binding. */
unsigned short Input::pollGamepadBinding()
{
    if (!gamepadInitialized) {
        return 0;
    }
    for (int c = 0; c < gamepadCount; ++c) {
        SDL_GameController* gc = gamepads[c];
        if (!gc) continue;
        for (int b = 0; b < SDL_CONTROLLER_BUTTON_MAX; ++b) {
            if (SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)b)) {
                return GP_BIND_BUTTON(b);
            }
        }
        for (int a = 0; a < SDL_CONTROLLER_AXIS_MAX; ++a) {
            Sint16 v = SDL_GameControllerGetAxis(gc, (SDL_GameControllerAxis)a);
            if (v > 20000) {
                return GP_BIND_AXIS(a, true);
            }
            if (v < -20000) {
                return GP_BIND_AXIS(a, false);
            }
        }
    }
    return 0;
}

/* Right stick, dead-zoned and normalised to [-1,1]. Used for camera look. */
void Input::getLeftStick(float& x, float& y)
{
    x = 0; y = 0;
    if (!gamepadInitialized) return;
    for (int c = 0; c < gamepadCount; ++c) {
        if (!gamepads[c]) continue;
        Sint16 lx = SDL_GameControllerGetAxis(gamepads[c], SDL_CONTROLLER_AXIS_LEFTX);
        Sint16 ly = SDL_GameControllerGetAxis(gamepads[c], SDL_CONTROLLER_AXIS_LEFTY);
        if (lx > 8000 || lx < -8000) x = lx / 32768.0f;
        if (ly > 8000 || ly < -8000) y = ly / 32768.0f;
        if (x != 0 || y != 0) return;
    }
}

void Input::getRightStick(float& x, float& y)
{
    x = 0;
    y = 0;
    if (!gamepadInitialized) {
        return;
    }
    for (int c = 0; c < gamepadCount; ++c) {
        if (!gamepads[c]) continue;
        Sint16 rx = SDL_GameControllerGetAxis(gamepads[c], SDL_CONTROLLER_AXIS_RIGHTX);
        Sint16 ry = SDL_GameControllerGetAxis(gamepads[c], SDL_CONTROLLER_AXIS_RIGHTY);
        if (rx > 8000 || rx < -8000) {
            x = rx / 32768.0f;
        }
        if (ry > 8000 || ry < -8000) {
            y = ry / 32768.0f;
        }
        if (x != 0 || y != 0) {
            return;
        }
    }
}

std::string Input::gamepadBindingName(unsigned short binding)
{
    if (binding == GP_BIND_NONE) {
        return "Unbound";
    }
    if (GP_IS_BUTTON(binding)) {
        SDL_GameControllerButton b = SDL_GameControllerButton((int)(binding - GP_BIND_BUTTON_BASE));
        const char* n = SDL_GameControllerGetStringForButton(b);
        return n ? std::string(n) : "Gamepad Button";
    }
    if (GP_IS_AXIS(binding)) {
        SDL_GameControllerAxis a = SDL_GameControllerAxis((int)(binding - GP_BIND_AXIS_BASE));
        const char* n = SDL_GameControllerGetStringForAxis(a);
        return n ? std::string(n) : "Gamepad Axis";
    }
    return "Unknown";
}

std::string Input::gamepadAxisName(int axis)
{
    const char* n = SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)axis);
    return n ? std::string(n) : "Gamepad Axis";
}

void Input::updateGamepad()
{
    /* Feed each active gamepad binding into the shared keyDown/keyPressed
     * arrays so that movement / crouch / draw / throw / attack / console
     * logic keeps working unchanged. Axis bindings use SDL_CONTROLLER_AXIS
     * indices as array indices (they live below SDL_NUM_SCANCODES). */

    static Uint32 lastGamepadPoll = 0;
    Uint32 now = SDL_GetTicks();
    if (now - lastGamepadPoll > 1000) {
        lastGamepadPoll = now;
        if (!gamepadInitialized) {
            refreshGamepads();
        }
    }

    if (!gamepadInitialized) {
        return;
    }

    /* Map each gamepad action onto the keyboard key that the game logic
     * actually polls, so controller input drives movement/actions. */
    const unsigned short actionKeys[GAMEPAD_ACTION_COUNT] = {
        forwardkey, backkey, leftkey, rightkey, crouchkey,
        jumpkey, drawkey, throwkey, attackkey, consolekey
    };

    for (int a = 0; a < GAMEPAD_ACTION_COUNT; ++a) {
        unsigned short b = gamepadkeys[a];
        if (b == GP_BIND_NONE) {
            continue;
        }
        bool held = false;
        if (GP_IS_BUTTON(b)) {
            SDL_GameControllerButton btn = SDL_GameControllerButton((int)(b - GP_BIND_BUTTON_BASE));
            for (int c = 0; c < gamepadCount; ++c) {
                if (!gamepads[c]) continue;
                if (SDL_GameControllerGetButton(gamepads[c], btn)) {
                    held = true;
                    break;
                }
            }
        } else if (GP_IS_AXIS(b)) {
            int raw = (int)(b - GP_BIND_AXIS_BASE);
            SDL_GameControllerAxis ax = SDL_GameControllerAxis(raw / 2);
            bool positive = (raw % 2) == 0;
            for (int c = 0; c < gamepadCount; ++c) {
                if (!gamepads[c]) continue;
                Sint16 value = SDL_GameControllerGetAxis(gamepads[c], ax);
                if (positive ? (value > 16384) : (value < -16384)) {
                    held = true;
                    break;
                }
            }
        }

        if (held) {
            unsigned short target = actionKeys[a];
            if (target < 3000) {
                bool prev = keyDown[target];
                keyDown[target] = true;
                keyPressed[target] = !prev;
            }
        }
    }
}