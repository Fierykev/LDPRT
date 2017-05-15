#ifndef __SHADER_HELPER__
#define __SHADER_HELPER__

#include <GL/glew.h>
#include "mat.h"

#define BUFFER_OFFSET(_i) (reinterpret_cast<char *>(NULL) + (_i))

void createProgram(GLuint* program, GLuint* vs, GLuint* tcs, GLuint* tes, GLuint* fs, GLuint* gs,
	char* fileVS, char* fileTCS, char* fileTES, char* fileGS, char* fileFS);

// Utility function.  Use glGetMatrix(GL_MODELVIEW_MATRIX) to retrieve
//  the current ModelView matrix.
Mat4f glGetMatrix(GLenum pname);

#endif