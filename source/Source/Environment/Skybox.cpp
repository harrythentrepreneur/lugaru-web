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

#include "Environment/Skybox.hpp"

#include "Game.hpp"

#include <iostream>

extern float viewdistance;
extern float blurness;
extern int environment;
extern bool skyboxtexture;
extern float skyboxr;
extern float skyboxg;
extern float skyboxb;

// Set to 1 to draw each face as a solid color (no textures) for debugging:
// front=red back=green up=blue down=yellow right=magenta left=cyan
#define SKY_DEBUG_COLORS 0

void SkyBox::load(const std::string& ffront, const std::string& fleft, const std::string& fback,
                  const std::string& fright, const std::string& fup, const std::string& fdown)
{
    // std::cerr << "SKYLOAD " << ffront << " | " << fleft << " | " << fback << " | "
    //           << fright << " | " << fup << " | " << fdown << std::endl;
    front.load(ffront, true);
    left.load(fleft, true);
    back.load(fback, true);
    right.load(fright, true);
    up.load(fup, true);
    down.load(fdown, true);
}

// One corner of a skybox face: position (as +-1, scaled by size later) and texture coordinate
struct SkyVert {
    float x, y, z, u, v;
};

// Face order: front, back, up, down, right, left (same geometry as the original quads)
static const SkyVert kFaces[6][4] = {
    // front
    { { -1, -1, 1, 0, 0 }, { 1, -1, 1, 1, 0 }, { 1, 1, 1, 1, 1 }, { -1, 1, 1, 0, 1 } },
    // back
    { { -1, -1, -1, 1, 0 }, { -1, 1, -1, 1, 1 }, { 1, 1, -1, 0, 1 }, { 1, -1, -1, 0, 0 } },
    // up
    { { -1, 1, -1, 0, 1 }, { -1, 1, 1, 0, 0 }, { 1, 1, 1, 1, 0 }, { 1, 1, -1, 1, 1 } },
    // down
    { { -1, -1, -1, 0, 0 }, { 1, -1, -1, 1, 0 }, { 1, -1, 1, 1, 1 }, { -1, -1, 1, 0, 1 } },
    // right
    { { 1, -1, -1, 1, 0 }, { 1, 1, -1, 1, 1 }, { 1, 1, 1, 0, 1 }, { 1, -1, 1, 0, 0 } },
    // left
    { { -1, -1, -1, 0, 0 }, { -1, -1, 1, 1, 0 }, { -1, 1, 1, 1, 1 }, { -1, 1, -1, 0, 1 } },
};

void SkyBox::draw()
{
    // Recompute every frame (was static, so it could be stuck at a bad first value)
    float size = viewdistance / 4;
    if (size < 1.0f) {
        size = 50.0f;
    }

    // Fog is applied in the emulation's shader and can flatten the sky
    bool fogWasOn = (glIsEnabled(GL_FOG) == GL_TRUE);
    glDisable(GL_FOG);

    glPushMatrix();
    static GLfloat M[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, M);
    M[12] = 0;
    M[13] = 0;
    M[14] = 0;
    glLoadMatrixf(M);
    if (environment == desertenvironment) {
        glScalef(1 + blurness / 1000, 1, 1 + blurness / 1000);
        glColor3f(1 * skyboxr, .95 * skyboxg, .95 * skyboxb);
    } else {
        glColor3f(.85 * skyboxr, .85 * skyboxg, .95 * skyboxb);
    }

    if (!skyboxtexture) {
        glDisable(GL_TEXTURE_2D);
        glColor3f(skyboxr * .8, skyboxg * .8, skyboxb * .8);
    }
    glDepthMask(0);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
    if (skyboxtexture) {
        glEnable(GL_TEXTURE_2D);
    }
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    Texture* faces[6] = { &front, &back, &up, &down, &right, &left };

    // One interleaved buffer per face (u, v, x, y, z per vertex), drawn the same way
    // the models are drawn (glInterleavedArrays), because the GL emulation handles
    // that layout reliably. Static so the pointers stay valid.
    static GLfloat buf[6][6 * 5];
    static const int tri[6] = { 0, 1, 2, 0, 2, 3 }; // quad -> two triangles

    for (int f = 0; f < 6; f++) {
        for (int i = 0; i < 6; i++) {
            const SkyVert& v = kFaces[f][tri[i]];
            buf[f][i * 5 + 0] = v.u;
            buf[f][i * 5 + 1] = v.v;
            buf[f][i * 5 + 2] = v.x * size;
            buf[f][i * 5 + 3] = v.y * size;
            buf[f][i * 5 + 4] = v.z * size;
        }

#if SKY_DEBUG_COLORS
        glDisable(GL_TEXTURE_2D);
        static const float cols[6][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 },
                                          { 1, 1, 0 }, { 1, 0, 1 }, { 0, 1, 1 } };
        glColor3f(cols[f][0], cols[f][1], cols[f][2]);
#else
        faces[f]->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
        glInterleavedArrays(GL_T2F_V3F, 0, buf[f]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glEnable(GL_CULL_FACE);
    glDepthMask(1);
    glPopMatrix();

    if (fogWasOn) {
        glEnable(GL_FOG);
    }
}
