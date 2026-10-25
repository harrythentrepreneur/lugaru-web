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

#include "User/Settings.hpp"

#include "Game.hpp"
#include "Utils/Folders.hpp"
#include "Utils/Input.hpp"

using namespace Game;

void DefaultSettings()
{
    detail = 2;
    ismotionblur = 1;
    usermousesensitivity = 3;
    gamepadsensitivity = 2;
    newscreenwidth = kContextWidth = 1024;
    newscreenheight = kContextHeight = 768;
    fullscreen = 0;
    floatjump = 0;
    autoslomo = 1;
    decalstoggle = true;
    invertmouse = 0;
    bloodtoggle = 0;
    foliage = 1;
    musictoggle = 1;
    trilinear = 1;
    gamespeed = 1;
    damageeffects = 0;
    texttoggle = 1;
    alwaysblur = 0;
    showpoints = 0;
    showdamagebar = 0;
    immediate = 0;
    velocityblur = 0;
    volume = 0.8f;
    ambientsound = 1;
    devtools = 0;

    crouchkey = SDL_SCANCODE_LSHIFT;
    jumpkey = SDL_SCANCODE_SPACE;
    leftkey = SDL_SCANCODE_A;
    forwardkey = SDL_SCANCODE_W;
    backkey = SDL_SCANCODE_S;
    rightkey = SDL_SCANCODE_D;
    drawkey = SDL_SCANCODE_E;
    throwkey = SDL_SCANCODE_Q;
    attackkey = MOUSEBUTTON_LEFT;
    consolekey = SDL_SCANCODE_GRAVE;

    gamepad = false;
    gamepadkeys[GP_FORWARD] = GP_BIND_AXIS(SDL_CONTROLLER_AXIS_LEFTY, false);
    gamepadkeys[GP_BACK] = GP_BIND_AXIS(SDL_CONTROLLER_AXIS_LEFTY, true);
    gamepadkeys[GP_LEFT] = GP_BIND_AXIS(SDL_CONTROLLER_AXIS_LEFTX, false);
    gamepadkeys[GP_RIGHT] = GP_BIND_AXIS(SDL_CONTROLLER_AXIS_LEFTX, true);
    gamepadkeys[GP_CROUCH] = GP_BIND_BUTTON(SDL_CONTROLLER_BUTTON_B);
    gamepadkeys[GP_JUMP] = GP_BIND_BUTTON(SDL_CONTROLLER_BUTTON_A);
    gamepadkeys[GP_DRAW] = GP_BIND_BUTTON(SDL_CONTROLLER_BUTTON_Y);
    gamepadkeys[GP_THROW] = GP_BIND_BUTTON(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    gamepadkeys[GP_ATTACK] = GP_BIND_BUTTON(SDL_CONTROLLER_BUTTON_X);
    gamepadkeys[GP_CONSOLE] = GP_BIND_BUTTON(SDL_CONTROLLER_BUTTON_BACK);

    newdetail = detail;
}

void SaveSettings()
{
    if (newdetail < 0) {
        newdetail = 0;
    }
    if (newdetail > 2) {
        newdetail = 2;
    }
    if (newscreenwidth < minscreenwidth || newscreenwidth > maxscreenwidth) {
        newscreenwidth = screenwidth;
    }
    if (newscreenheight < minscreenheight || newscreenheight > maxscreenheight) {
        newscreenheight = screenheight;
    }
    errno = 0;
    ofstream opstream(Folders::getConfigFilePath());
    if (opstream.fail()) {
        perror(("Couldn't save config file " + Folders::getConfigFilePath()).c_str());
        return;
    }
    opstream << "Screenwidth:\n";
    opstream << newscreenwidth;
    opstream << "\nScreenheight:\n";
    opstream << newscreenheight;
    opstream << "\nFullscreen:\n";
    opstream << fullscreen;
    opstream << "\nMouse sensitivity:\n";
    opstream << usermousesensitivity;
    opstream << "\nGamepad sensitivity:\n";
    opstream << gamepadsensitivity;
    opstream << "\nBlur(0,1):\n";
    opstream << ismotionblur;
    opstream << "\nOverall Detail(0,1,2) higher=better:\n";
    opstream << newdetail;
    opstream << "\nFloating jump:\n";
    opstream << floatjump;
    opstream << "\nMouse jump:\n";
    opstream << mousejump;
    opstream << "\nAmbient sound:\n";
    opstream << ambientsound;
    opstream << "\nBlood (0,1,2):\n";
    opstream << bloodtoggle;
    opstream << "\nAuto slomo:\n";
    opstream << autoslomo;
    opstream << "\nFoliage:\n";
    opstream << foliage;
    opstream << "\nMusic:\n";
    opstream << musictoggle;
    opstream << "\nTrilinear:\n";
    opstream << trilinear;
    opstream << "\nDecals(shadows,blood puddles,etc):\n";
    opstream << decalstoggle;
    opstream << "\nInvert mouse:\n";
    opstream << invertmouse;
    opstream << "\nGamespeed:\n";
    if (oldgamespeed == 0) {
        oldgamespeed = 1;
    }
    opstream << oldgamespeed;
    opstream << "\nDamage effects(blackout, doublevision):\n";
    opstream << damageeffects;
    opstream << "\nText:\n";
    opstream << texttoggle;
    opstream << "\nShow Points:\n";
    opstream << showpoints;
    opstream << "\nAlways Blur:\n";
    opstream << alwaysblur;
    opstream << "\nImmediate mode (turn on on G5):\n";
    opstream << immediate;
    opstream << "\nVelocity blur:\n";
    opstream << velocityblur;
    opstream << "\nVolume:\n";
    opstream << volume;
    opstream << "\nForward key:\n";
    opstream << forwardkey;
    opstream << "\nBack key:\n";
    opstream << backkey;
    opstream << "\nLeft key:\n";
    opstream << leftkey;
    opstream << "\nRight key:\n";
    opstream << rightkey;
    opstream << "\nJump key:\n";
    opstream << jumpkey;
    opstream << "\nCrouch key:\n";
    opstream << crouchkey;
    opstream << "\nDraw key:\n";
    opstream << drawkey;
    opstream << "\nThrow key:\n";
    opstream << throwkey;
    opstream << "\nAttack key:\n";
    opstream << attackkey;
    opstream << "\nConsole key:\n";
    opstream << consolekey;
    opstream << "\nGamepad:\n";
    opstream << gamepad;
    static const char* gamepadSettingNames[GAMEPAD_ACTION_COUNT] = {
        "Gamepad Forwards", "Gamepad Back", "Gamepad Left", "Gamepad Right",
        "Gamepad Crouch", "Gamepad Jump", "Gamepad Draw", "Gamepad Throw",
        "Gamepad Attack", "Gamepad Console"
    };
    for (int i = 0; i < GAMEPAD_ACTION_COUNT; i++) {
        opstream << "\n" << gamepadSettingNames[i] << ":\n";
        opstream << gamepadkeys[i];
    }
    opstream << "\nDamage bar:\n";
    opstream << showdamagebar;
    opstream << "\nStereoMode:\n";
    opstream << stereomode;
    opstream << "\nStereoSeparation:\n";
    opstream << stereoseparation;
    opstream << "\nStereoReverse:\n";
    opstream << stereoreverse;
    opstream << "\n";
    opstream.close();

#ifdef __EMSCRIPTEN__
    std::ostringstream confStr;
    confStr << opstream.rdbuf();
    opstream.close();
    Folders::saveStringToStorage("lugaru_config", confStr.str());
#else
    Folders::syncPersistentStorage();
#endif
}

bool LoadSettings()
{
    std::string data = Folders::loadStringFromStorage("lugaru_config");
    if (data.empty()) {
        perror("Couldn't read config (localStorage empty) — using defaults.");
        DefaultSettings();
        SaveSettings();
        return true;
    }
    /* Reuse the original parser by piping the string through a temp FILE* */
    FILE* tmp = tmpfile();
    if (!tmp) {
        perror("Couldn't create temp config");
        DefaultSettings();
        return false;
    }
    fputs(data.c_str(), tmp);
    rewind(tmp);
    int ch;
    while ((ch = fgetc(tmp)) != EOF && ch != '\0') {
        fputc(ch, tmp);
    }
    rewind(tmp);
    char setting[256];
    char string[256];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
    printf("Loading config\n");
    while (!feof(tmp)) {
        if (fgets(setting, sizeof(setting), tmp) == NULL) break;
        if (strlen(setting) == 0 || setting[0] == ' ' || setting[0] == '\t') continue;
        if (fscanf(tmp, "%255s", setting) == 0) break;
        if (strncmp(setting, "Screenwidth", 11)) { fscanf(tmp, "%d", &kContextWidth); }
        else if (strncmp(setting, "Screenheight", 12)) { fscanf(tmp, "%d", &kContextHeight); }
        else if (strncmp(setting, "Fullscreen", 10)) { fscanf(tmp, "%d", &fullscreen); }
        else if (strncmp(setting, "Mouse sensitivity", 17)) { fscanf(tmp, "%d", &usermousesensitivity); }
        else if (strncmp(setting, "Gamepad sensitivity", 20)) { fscanf(tmp, "%f", &gamepadsensitivity); }
        else if (strncmp(setting, "Blur", 4)) { fscanf(tmp, "%d", &ismotionblur); }
        else if (strncmp(setting, "Overall Detail", 14)) { fscanf(tmp, "%d", &detail); }
        else if (strncmp(setting, "Floating jump", 13)) { fscanf(tmp, "%d", &floatjump); }
        else if (strncmp(setting, "Mouse jump", 10)) { fscanf(tmp, "%d", &mousejump); }
        else if (strncmp(setting, "Ambient sound", 13)) { fscanf(tmp, "%d", &ambientsound); }
        else if (strncmp(setting, "Blood", 5)) { fscanf(tmp, "%d", &bloodtoggle); }
        else if (strncmp(setting, "Auto slomo", 10)) { fscanf(tmp, "%d", &autoslomo); }
        else if (strncmp(setting, "Foliage", 7)) { fscanf(tmp, "%d", &foliage); }
        else if (strncmp(setting, "Music", 5)) { fscanf(tmp, "%d", &musictoggle); }
        else if (strncmp(setting, "Trilinear", 9)) { fscanf(tmp, "%d", &trilinear); }
        else if (strncmp(setting, "Decals", 6)) { fscanf(tmp, "%d", &decalstoggle); }
        else if (strncmp(setting, "Invert mouse", 12)) { fscanf(tmp, "%d", &invertmouse); }
        else if (strncmp(setting, "Gamespeed", 9)) { fscanf(tmp, "%d", &gamespeed); oldgamespeed = gamespeed; if (oldgamespeed == 0) { gamespeed = 1; oldgamespeed = 1; } }
        else if (strncmp(setting, "Damage effects", 14)) { fscanf(tmp, "%d", &damageeffects); }
        else if (strncmp(setting, "Text", 4)) { fscanf(tmp, "%d", &texttoggle); }
        else if (strncmp(setting, "Devtools", 8)) { fscanf(tmp, "%d", &devtools); }
        else if (strncmp(setting, "Show Points", 11)) { fscanf(tmp, "%d", &showpoints); }
        else if (strncmp(setting, "Always Blur", 11)) { fscanf(tmp, "%d", &alwaysblur); }
        else if (strncmp(setting, "Immediate mode ", 15)) { fscanf(tmp, "%d", &immediate); }
        else if (strncmp(setting, "Velocity blur", 13)) { fscanf(tmp, "%d", &velocityblur); }
        else if (strncmp(setting, "Volume", 6)) { fscanf(tmp, "%f", &volume); }
        else if (strncmp(setting, "Forward key", 11)) { fscanf(tmp, "%d", &forwardkey); }
        else if (strncmp(setting, "Back key", 8)) { fscanf(tmp, "%d", &backkey); }
        else if (strncmp(setting, "Left key", 8)) { fscanf(tmp, "%d", &leftkey); }
        else if (strncmp(setting, "Right key", 9)) { fscanf(tmp, "%d", &rightkey); }
        else if (strncmp(setting, "Jump key", 8)) { fscanf(tmp, "%d", &jumpkey); }
        else if (strncmp(setting, "Crouch key", 10)) { fscanf(tmp, "%d", &crouchkey); }
        else if (strncmp(setting, "Draw key", 8)) { fscanf(tmp, "%d", &drawkey); }
        else if (strncmp(setting, "Throw key", 9)) { fscanf(tmp, "%d", &throwkey); }
        else if (strncmp(setting, "Attack key", 10)) { fscanf(tmp, "%d", &attackkey); }
        else if (strncmp(setting, "Console key", 11)) { fscanf(tmp, "%d", &consolekey); }
        else if (strncmp(setting, "Gamepad Forwards", 16)) { fscanf(tmp, "%d", &gamepadkeys[GP_FORWARD]); }
        else if (strncmp(setting, "Gamepad Back", 12)) { fscanf(tmp, "%d", &gamepadkeys[GP_BACK]); }
        else if (strncmp(setting, "Gamepad Left", 12)) { fscanf(tmp, "%d", &gamepadkeys[GP_LEFT]); }
        else if (strncmp(setting, "Gamepad Right", 13)) { fscanf(tmp, "%d", &gamepadkeys[GP_RIGHT]); }
        else if (strncmp(setting, "Gamepad Crouch", 14)) { fscanf(tmp, "%d", &gamepadkeys[GP_CROUCH]); }
        else if (strncmp(setting, "Gamepad Jump", 12)) { fscanf(tmp, "%d", &gamepadkeys[GP_JUMP]); }
        else if (strncmp(setting, "Gamepad Draw", 12)) { fscanf(tmp, "%d", &gamepadkeys[GP_DRAW]); }
        else if (strncmp(setting, "Gamepad Throw", 13)) { fscanf(tmp, "%d", &gamepadkeys[GP_THROW]); }
        else if (strncmp(setting, "Gamepad Attack", 14)) { fscanf(tmp, "%d", &gamepadkeys[GP_ATTACK]); }
        else if (strncmp(setting, "Gamepad Console", 15)) { fscanf(tmp, "%d", &gamepadkeys[GP_CONSOLE]); }
        else if (strncmp(setting, "Gamepad", 7)) { fscanf(tmp, "%d", &gamepad); }
        else if (strncmp(setting, "Damage bar", 10)) { fscanf(tmp, "%d", &showdamagebar); }
        else if (strncmp(setting, "StereoMode", 10)) { int i; fscanf(tmp, "%d", &i); stereomode=(StereoMode)i; }
        else if (strncmp(setting, "StereoSeparation", 16)) { fscanf(tmp, "%d", &stereoseparation); }
        else if (strncmp(setting, "StereoReverse", 13)) { fscanf(tmp, "%d", &stereoreverse); }
        else { fscanf(tmp, "%255s", string); fprintf(stderr, "Unknown config option '%s' with value '%s'. Ignoring.\n", setting, string); }
    }
    fclose(tmp);
    if (detail > 2) detail = 2;
    if (detail < 0) detail = 0;
    if (screenwidth < minscreenwidth || screenwidth > maxscreenwidth) screenwidth = 1024;
    if (screenheight < minscreenheight || screenheight > maxscreenheight) screenheight = 768;
    newdetail = detail;
    return true;
}
#pragma GCC diagnostic pop
// </LoadSettings>
