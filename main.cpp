#define NUM_SCENES 4
#define START_SCENE 0

#include <time.h>

#include <GL/glew.h>
#include "Camera.h"
#include "MorphTarget.h"
#include "SphericalHarmonic.h"
#include "LightProb.h"
#include <GL/freeglut.h>
#include <IL/il.h>

Camera camera;
MorphTarget morphTarget;

size_t currentScene;

size_t frame;
clock_t lastTime;

vector<const char*> morphList[NUM_SCENES] = {
	{
	"Models/Bunny/Bunny.obj"
	},
	{
	"Models/Cornell.obj"
	},
	{
		"Models/Monkey/Monkey.obj",
		"Models/Monkey/MonkeyTwist.obj"
	},
	{
		"Morphs/Head.obj",
		"Morphs/Mouth.obj",
		"Morphs/Smile.obj",
		"Morphs/Colbert.obj",
		"Morphs/HeadTilt.obj",
		"Morphs/HeadForward.obj"
	}
};

void render()
{
	// grab framerate
	clock_t currentTime = clock();
	frame++;

	if (CLOCKS_PER_SEC < currentTime - lastTime)
	{
		char title[256];
		sprintf(title, "Morph Target FPS: %4.2f",
			float(frame) * CLOCKS_PER_SEC
			/ float(currentTime - lastTime));
		lastTime = currentTime;
		glutSetWindowTitle(title);
		frame = 0;
	}

	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClearColor(0.f, 0.f, 0.f, 1.f);

	// apply camera view
	camera.apply();

	morphTarget.draw();

	glutSwapBuffers();
}

void specKeys(int key, int x, int y)
{
	camera.processSpecialKeys(key, x, y);

	glutPostRedisplay();
}

void keys(unsigned char key, int x, int y)
{
	camera.processKeys(key, x, y);
	LightProb::processKeys(key, x, y);

	switch (key)
	{
	case 't':
		MorphTarget::toggle = (MorphTarget::toggle + 1) % 4;
		break;
	case 'r':
		MorphTarget::tess = !MorphTarget::tess;
		break;
	case 'n':

		currentScene = (currentScene + 1) % NUM_SCENES;

		morphTarget.reset();
		morphTarget.load(
			(const char**)&morphList[currentScene].at(0),
			morphList[currentScene].size()
		);

		break;
	case 'y':
		MorphTarget::animate = !MorphTarget::animate;

		break;
	}
	//printf("%i\n", LDPRT::toggle);
	glutPostRedisplay();
}

void init()
{
	GLenum err = glewInit();

	if (GLEW_OK != err)
	{
		printf("ERROR %s\n", glewGetErrorString(err));
	}

	// init DevIL
	ilInit();

	// setup for OpenGL
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//glOrtho(-1.0, 1.0, -1.0, 1.0, .1, 100);
	gluPerspective(45.f, 1920.f / 1080.f, .1f, 100.f);
	//glOrtho(-1.0, 1.0, -1.0, 1.0, .1, 100);

	glMatrixMode(GL_MODELVIEW);

	// program specific stuff
	srand(time(NULL));

	SphericalHarmonic::genConstants();

	// setup scene
	currentScene = START_SCENE;

	morphTarget.load(
		(const char**)&morphList[currentScene].at(0),
		morphList[currentScene].size()
	);

	// setup camera
	camera.setAt(Vec3f(0, 0, 0));
	camera.setEye(Vec3f(0, 0, 2.f));

	// setup framerate
	frame = 0;
	lastTime = clock();
}

int main(int argc, char** argv)
{		
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE |
		GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(1920.f, 1080.f);
	glutCreateWindow("Morph Target");
	glutSpecialFunc(specKeys);
	glutKeyboardFunc(keys);
	glutDisplayFunc(render);
	glutIdleFunc(render);

	init();

	glutMainLoop();

	return 0;
}
