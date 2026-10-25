#pragma once
#include <GL/gl.h>
#ifdef __cplusplus
extern "C" {
#endif
void lug_glBegin(GLenum m);
void lug_glEnd(void);
void lug_glTexCoord2f(GLfloat s, GLfloat t);
void lug_glVertex2f(GLfloat x, GLfloat y);
void lug_glVertex2i(GLint x, GLint y);
void lug_glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void lug_glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void lug_glTranslated(GLdouble x, GLdouble y, GLdouble z);
void lug_glGenerateMipmap(GLenum target);
#ifdef __cplusplus
}
#endif
#ifndef LUG_GL_SHIM_IMPL
#define glBegin lug_glBegin
#define glEnd lug_glEnd
#define glTexCoord2f lug_glTexCoord2f
#define glVertex2f lug_glVertex2f
#define glVertex2i lug_glVertex2i
#define glVertex3f lug_glVertex3f
#define glTranslatef lug_glTranslatef
#define glTranslated lug_glTranslated
#define glGenerateMipmap lug_glGenerateMipmap
#endif
