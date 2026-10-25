#define LUG_GL_SHIM_IMPL
#include "GLListRecorder.h"
#include <map>
#include <vector>
#include <GLES2/gl2.h>

namespace {
enum Op { OP_BEGIN, OP_END, OP_TEX, OP_V2, OP_V3, OP_TRANS };
struct Cmd { Op op; double a, b, c; unsigned e; };
std::map<GLuint, std::vector<Cmd> > g_lists;
GLuint g_next = 1, g_cur = 0, g_base = 0;
bool g_rec = false;
void rec(Op op, double a = 0, double b = 0, double c = 0, unsigned e = 0) {
    Cmd c1 = { op, a, b, c, e };
    g_lists[g_cur].push_back(c1);
}
}

extern "C" {

void lug_glBegin(GLenum m) { if (g_rec) rec(OP_BEGIN, 0, 0, 0, m); else glBegin(m); }
void lug_glEnd(void) { if (g_rec) rec(OP_END); else glEnd(); }
void lug_glTexCoord2f(GLfloat s, GLfloat t) { if (g_rec) rec(OP_TEX, s, t); else glTexCoord2f(s, t); }
void lug_glVertex2f(GLfloat x, GLfloat y) { if (g_rec) rec(OP_V2, x, y); else glVertex2f(x, y); }
void lug_glVertex2i(GLint x, GLint y) { if (g_rec) rec(OP_V2, x, y); else glVertex2i(x, y); }
void lug_glVertex3f(GLfloat x, GLfloat y, GLfloat z) { if (g_rec) rec(OP_V3, x, y, z); else glVertex3f(x, y, z); }
void lug_glTranslatef(GLfloat x, GLfloat y, GLfloat z) { if (g_rec) rec(OP_TRANS, x, y, z); else glTranslatef(x, y, z); }
void lug_glTranslated(GLdouble x, GLdouble y, GLdouble z) { if (g_rec) rec(OP_TRANS, x, y, z); else glTranslated(x, y, z); }
void lug_glGenerateMipmap(GLenum target) { ::glGenerateMipmap(target); }

GLuint glGenLists(GLsizei range) { GLuint id = g_next; g_next += (range > 0 ? range : 1); return id; }
void glDeleteLists(GLuint list, GLsizei range) { for (GLsizei i = 0; i < range; i++) g_lists.erase(list + i); }
void glNewList(GLuint list, GLenum) { g_cur = list; g_lists[list].clear(); g_rec = true; }
void glEndList(void) { g_rec = false; }
void glListBase(GLuint base) { g_base = base; }

void glCallLists(GLsizei n, GLenum type, const GLvoid* lists) {
    for (GLsizei i = 0; i < n; i++) {
        GLuint v = 0;
        if (type == GL_UNSIGNED_BYTE || type == GL_BYTE) v = ((const unsigned char*)lists)[i];
        else if (type == GL_UNSIGNED_INT || type == GL_INT) v = ((const GLuint*)lists)[i];
        else if (type == GL_UNSIGNED_SHORT || type == GL_SHORT) v = ((const unsigned short*)lists)[i];
        std::map<GLuint, std::vector<Cmd> >::iterator it = g_lists.find(g_base + v);
        if (it == g_lists.end()) continue;
        const std::vector<Cmd>& cs = it->second;
        for (size_t k = 0; k < cs.size(); k++) {
            const Cmd& c = cs[k];
            switch (c.op) {
                case OP_BEGIN: glBegin(c.e); break;
                case OP_END: glEnd(); break;
                case OP_TEX: glTexCoord2f((GLfloat)c.a, (GLfloat)c.b); break;
                case OP_V2: glVertex2f((GLfloat)c.a, (GLfloat)c.b); break;
                case OP_V3: glVertex3f((GLfloat)c.a, (GLfloat)c.b, (GLfloat)c.c); break;
                case OP_TRANS: glTranslatef((GLfloat)c.a, (GLfloat)c.b, (GLfloat)c.c); break;
            }
        }
    }
}

void glPixelTransferi(GLenum, GLint) {}

void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid* p) {
    const char* b = (const char*)p;
    GLsizei s;
    switch (format) {
        case GL_T2F_N3F_V3F:
            s = stride ? stride : 8 * sizeof(float);
            glDisableClientState(GL_COLOR_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);
            glEnableClientState(GL_VERTEX_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, s, b);
            glNormalPointer(GL_FLOAT, s, b + 2 * sizeof(float));
            glVertexPointer(3, GL_FLOAT, s, b + 5 * sizeof(float));
            break;
        case GL_T2F_V3F:
            s = stride ? stride : 5 * sizeof(float);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_VERTEX_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, s, b);
            glVertexPointer(3, GL_FLOAT, s, b + 2 * sizeof(float));
            break;
        case GL_N3F_V3F:
            s = stride ? stride : 6 * sizeof(float);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);
            glEnableClientState(GL_VERTEX_ARRAY);
            glNormalPointer(GL_FLOAT, s, b);
            glVertexPointer(3, GL_FLOAT, s, b + 3 * sizeof(float));
            break;
        case GL_V3F:
            s = stride ? stride : 3 * sizeof(float);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, s, b);
            break;
        default: break;
    }
}

void glShadeModel(GLenum) {}

}
