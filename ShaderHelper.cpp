#include "ShaderHelper.h"
#include <fstream>

using namespace std;

bool loadShader(char* filename, GLuint* shader, GLenum type)
{
	ifstream file(filename, ios::in | ios::binary);

	string data;

	if (file.is_open())
		data.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	*shader = glCreateShader(type);
	GLchar* prog = (GLchar*)data.c_str();
	glShaderSource(*shader, 1, &prog, NULL);
	glCompileShader(*shader);

	GLint compiled;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		printf("Cannot compile %s\n", filename);

		GLint max_length, length;
		glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &max_length);

		GLchar* info_log = new GLchar[max_length];
		glGetShaderInfoLog(*shader, max_length, &length, info_log);
		printf("%s", info_log);
		delete info_log;

		return false;
	}

	return true;
}

void createProgram(GLuint* program, GLuint* vs, GLuint* tcs, GLuint* tes, GLuint* fs, GLuint* gs,
	char* fileVS, char* fileTCS, char* fileTES, char* fileGS, char* fileFS)
{
	*program = glCreateProgram();

	if (*program)
	{
		if (vs != nullptr)
		{
			loadShader(fileVS, vs, GL_VERTEX_SHADER);
			glAttachShader(*program, *vs);
		}

		if (tcs != nullptr)
		{
			loadShader(fileTCS, tcs, GL_TESS_CONTROL_SHADER);
			glAttachShader(*program, *tcs);
		}

		if (tes != nullptr)
		{
			loadShader(fileTES, tes, GL_TESS_EVALUATION_SHADER);
			glAttachShader(*program, *tes);
		}

		if (gs != nullptr)
		{
			loadShader(fileGS, gs, GL_GEOMETRY_SHADER);
			glAttachShader(*program, *gs);
		}

		if (fs != nullptr)
		{
			loadShader(fileFS, fs, GL_FRAGMENT_SHADER);
			glAttachShader(*program, *fs);
		}

		glLinkProgram(*program);

		GLint linked = 0;
		glGetProgramiv(*program, GL_LINK_STATUS, &linked);
		if (!linked) {
			printf("ERR Linking\n");

			GLint max_length, length;
			glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &max_length);

			GLchar* info_log = new GLchar[max_length];
			glGetProgramInfoLog(*program, max_length, &length, info_log);
			printf("%s", info_log);
			delete info_log;
		}
	}
}

Mat4f glGetMatrix(GLenum pname)
{
	GLfloat m[16];
	glGetFloatv(pname, m);
	Mat4f matCam(m[0], m[1], m[2], m[3],
		m[4], m[5], m[6], m[7],
		m[8], m[9], m[10], m[11],
		m[12], m[13], m[14], m[15]);

	// because the matrix GL returns is column major...
	return matCam.transpose();
}